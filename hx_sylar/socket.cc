#include "socket.h"

#include <asm-generic/socket.h>
#include <sys/ioctl.h>
#include <sys/socket.h>

#include <cerrno>
#include <cstring>
#include <memory>

#include "address.h"
#include "fd_manager.h"
#include "hook.h"
#include "iomanager.h"
#include "log.h"
#include "macro.h"
namespace hx_sylar {
static hx_sylar::Logger::ptr g_logger = HX_LOG_NAME("system");
Socket::Socket(int family, int type, int protocol)
    : m_sock(-1),
      m_family(family),
      m_type(type),
      m_protocol(protocol),
      m_isConnected(false) {}
Socket::~Socket() { close(); }

auto Socket::getSendTimeout() -> uint64_t {
  FdCtx::ptr ctx = FdMgr::GetInstance()->get(m_sock);
  if (ctx) {
    return ctx->getTimeout(SO_SNDTIMEO);
  }
  return -1;
}
void Socket::setSendTimeout(int64_t v) {
  struct timeval tv {
    int(v / 1000), int(v % 1000 * 1000)
  };
  setOption(SOL_SOCKET, SO_SNDTIMEO, tv);
}

auto Socket::getRecvTimeout() -> int64_t {
  FdCtx::ptr ctx = FdMgr::GetInstance()->get(m_sock);
  if (ctx) {
    return ctx->getTimeout(SO_RCVTIMEO);
  }
  return -1;
}
void Socket::setRecvTimeout(int64_t v) {
  struct timeval tv {
    int(v / 1000), int(v % 1000 * 1000)
  };
  setOption(SOL_SOCKET, SO_RCVTIMEO, tv);
}
auto Socket::getOption(int level, int option, void* result, size_t* len)
    -> bool {
  int rt = getsockopt(m_sock, level, option, result,
                      reinterpret_cast<socklen_t*>(len));
  if (rt != 0) {
    HX_LOG_DEBUG(g_logger) << "getOption sock = " << m_sock
                           << " level=" << level << "option " << option
                           << "errno" << errno
                           << " errstr = " << strerror(errno);
    return false;
  }
  return true;
}

bool Socket::setOption(int level, int option, const void* result,
                       socklen_t len) {
  if (setsockopt(m_sock, level, option, result, (socklen_t)len)) {
    HX_LOG_DEBUG(g_logger) << "setOption sock=" << m_sock << " level=" << level
                           << " option=" << option << " errno=" << errno
                           << " errstr=" << strerror(errno);
    return false;
  }
  return true;
}

auto Socket::accpet() -> Socket::ptr {
  Socket::ptr sock = std::make_shared<Socket>(m_family, m_type, m_protocol);
  int newsock = ::accept(m_sock, nullptr, nullptr);
  if (newsock == -1) {
    HX_LOG_ERROR(g_logger) << "accept(" << m_sock << "( errno = " << errno
                           << "errstr = " << strerror(errno);
    return nullptr;
  }
  if (sock->init(newsock)) {
    return sock;
  }
  return nullptr;
}

auto Socket::init(int sock) -> bool {
  auto ctx = FdMgr::GetInstance()->get(sock);
  if (ctx && ctx->isSocket() && !ctx->isClose()) {
    m_sock = sock;
    m_isConnected = true;
    initSock();
    getLocalAddress();
    getRemoteAddress();
    return true;
  }
  return false;
}
auto Socket::bind(const Address::ptr& addr) -> bool {
  // FdCtx::ptr ctx = FdMgr::GetInstance()->get(sock);
  // if (ctx && ctx->isSocket() && !ctx->isClose()) {
  // }

  if (!isValid()) {
    newSock();
    if (SYLAR_UNLIKELY(!isValid())) {
      return false;
    }
  }

  if (SYLAR_UNLIKELY(addr->getFamily() != m_family)) {
    HX_LOG_ERROR(g_logger) << " bind sock.family (" << m_family
                           << ") adddr addr.family ()" << addr->getFamily()
                           << ") not equal , addr = " << addr->toString();
    return false;
  }
  if (::bind(m_sock, addr->getAddr(), addr->getFamily())) {
    HX_LOG_ERROR(g_logger) << "bind sock.family (" << m_family
                           << "0 addr.family ()" << addr->getFamily()
                           << ") noe equal addr = " << addr->toString();
    return false;
  }
  getLocalAddress();
  return true;
}
auto Socket::connect(const Address::ptr addr, uint64_t timeout_ms) -> bool {
  if (!isValid()) {
    newSock();
    if (SYLAR_UNLIKELY(!isValid())) {
      return false;
    }
  }
  if (SYLAR_UNLIKELY(addr->getFamily() != m_family)) {
    HX_LOG_ERROR(g_logger) << " connect sock.family (" << m_family
                           << ") adddr addr.family ()" << addr->getFamily()
                           << ") not equal , addr = " << addr->toString();
    return false;
  }
  if (timeout_ms == static_cast<uint64_t>(-1)) {
    if (::connect(m_sock, addr->getAddr(), addr->getAddrLen()) != 0) {
      HX_LOG_ERROR(g_logger)
          << "sock = " << m_sock << "connect ( )" << addr->toString()
          << ") errno: " << errno << "errstr = " << strerror(errno);
      return false;
    }
  } else {
    if (::connect_with_timeout(m_sock, addr->getAddr(), addr->getAddrLen(),
                               timeout_ms)) {
      HX_LOG_ERROR(g_logger)
          << "sock = " << m_sock << " connect ()" << addr->toString()
          << ") timeout = " << timeout_ms;
    }
  }

  return true;
}
auto Socket::listen(int backlog) -> bool {
  if (!isValid()) {
    HX_LOG_ERROR(g_logger) << "listen error sock = -1";
    return false;
  }
  if (::listen(m_sock, backlog)) {
    HX_LOG_ERROR(g_logger) << "listen error errno = " << errno
                           << " errstr = " << strerror(errno);
  }
  return true;
}
auto Socket::close() -> bool {
  if (!m_isConnected && m_sock == -1) {
    return true;
  }

  m_isConnected = false;

  if (m_sock != -1) {
    ::close(m_sock);
    m_sock = -1;
  }
  return false;
}
auto Socket::send(const void* buffer, size_t length, int flags) -> int {
  if (!m_isConnected) {
    return -1;
  }
  return ::send(m_sock, buffer, length, flags);
}
auto Socket::send(const iovec* buffers, size_t length, int flags) -> int {
  if (m_isConnected) {
    msghdr msg;
    memset(&msg, 0, sizeof(msg));
    msg.msg_iov = (iovec*)buffers;
    msg.msg_iovlen = length;

    return ::sendmsg(m_sock, &msg, flags);
  }
  return -1;
}
auto Socket::sendTo(const void* buffer, size_t length, const Address::ptr to,
                    int flags) -> int {
  if (m_isConnected) {
    return ::sendto(m_sock, buffer, length, flags, to->getAddr(),
                    to->getAddrLen());
  }
  return -1;
}
auto Socket::sendTo(const iovec* buffers, size_t length, const Address::ptr to,
                    int flags) -> int {
  if (m_isConnected) {
    msghdr msg;
    memset(&msg, 0, sizeof msg);
    msg.msg_iov = (iovec*)buffers;
    msg.msg_iovlen = length;
    msg.msg_name = to->getAddr();
    msg.msg_namelen = to->getAddrLen();

    return ::sendmsg(m_sock, &msg, flags);
  }
  return -1;
}
auto Socket::recv(void* buffer, size_t length, int flags) -> int {
  if (m_isConnected) {
    return ::recv(m_sock, buffer, length, flags);
  }
  return -1;
}
auto Socket::recv(iovc* buffers, size_t length, int flags) -> int {
  if (m_isConnected) {
    msghdr msg;
    memset(&msg, 0, sizeof msg);
    msg.msg_iov = (iovec*)buffers;
    msg.msg_iovlen = length;
    return ::recvmsg(m_sock, &msg, flags);
  }
  return -1;
}
auto Socket::recvFrom(void* buffer, size_t length, Address::ptr from, int flags)
    -> int {
  if (m_isConnected) {
    socklen_t len = from->getAddrLen();
    return ::recvfrom(m_sock, buffer, length, flags, from->getAddr(), &len);
  }
  return -1;
}
auto Socket::recvFrom(iovec* buffers, size_t length, Address::ptr from,
                      int flags) -> int {
  if (m_isConnected) {
    msghdr msg;
    memset(&msg, 0, sizeof msg);
    msg.msg_iov = (iovec*)buffers;
    msg.msg_iovlen = length;
    msg.msg_name = from->getAddr();
    msg.msg_namelen = from->getAddrLen();

    return ::recvmsg(m_sock, &msg, flags);
  }
  return -1;
}
auto Socket::getRemoteAddress() -> Address::ptr {
  if (m_remoteAddress) {
    return m_remoteAddress;
  }
  Address::ptr result;
  switch (m_family) {
    case AF_INET:
      result.reset(new IPv4Address());
      break;
    case AF_INET6:
      result.reset(new IPv6Address());
      break;
    case AF_UNIX:
      result.reset(new UnixAddress());
      break;
    default:
      result.reset(new UnknownAddress(m_family));
      break;
  }
  socklen_t addrlen = result->getAddrLen();
  if (getpeername(m_sock, result->getAddr(), &addrlen)) {
    HX_LOG_ERROR(g_logger) << "getpeername error sock" << m_sock
                           << " errno = " << errno
                           << "errnostr = " << strerror(errno);
    return Address::ptr(new UnknownAddress(m_family));
  }
  if (m_family == AF_UNIX) {
    UnixAddress::ptr addr = std::dynamic_pointer_cast<UnixAddress>(result);
    addr->setAddrLen(addrlen);
  }
  m_remoteAddress = result;
  return m_remoteAddress;
}
auto Socket::getLocalAddress() -> Address::ptr {
  if (m_localAddress) {
    return m_localAddress;
  }
  Address::ptr result;
  switch (m_family) {
    case AF_INET:
      result.reset(new IPv4Address());
      break;
    case AF_INET6:
      result.reset(new IPv6Address());
      break;
    case AF_UNIX:
      result.reset(new UnixAddress());
      break;
    default:
      result.reset(new UnknownAddress(m_family));
      break;
  }
  socklen_t addrlen = result->getAddrLen();
  if (getsockname(m_sock, result->getAddr(), &addrlen)) {
    HX_LOG_ERROR(g_logger) << "getsockname error sock" << m_sock
                           << " errno = " << errno
                           << "errnostr = " << strerror(errno);
    return Address::ptr(new UnknownAddress(m_family));
  }
  if (m_family == AF_UNIX) {
    UnixAddress::ptr addr = std::dynamic_pointer_cast<UnixAddress>(result);
    addr->setAddrLen(addrlen);
  }
  m_localAddress = result;
  return m_localAddress;
}

auto Socket::isValid() const -> bool { return m_sock != -1; }
auto Socket::getError() -> int {
  int error = 0;
  size_t len = sizeof error;
  if (!getOption(SOL_SOCKET, SO_ERROR, &error, &len)) {
    return -1;
  }
  return error;
}
auto Socket::dump(std::ostream& os) const -> std::ostream& {
  os << "[Sock sock= " << m_sock << "is_connected= " << m_isConnected
     << "family= " << m_family << "type=" << m_type
     << "protocol=" << m_protocol;
  if (m_localAddress) {
    os << " local_address = " << m_localAddress->toString();
  }
  if (m_remoteAddress) {
    os << " remote_address=" << m_remoteAddress->toString();
  }

  os << "]";
  return os;
}
// auto Socket::getSocket() const -> int {}
auto Socket::cancelRead() -> bool {
  return IOManager::GetThis()->cancelEvent(m_sock, IOManager::READ);
}
auto Socket::cancelaaawrite() -> bool {
  return IOManager::GetThis()->cancelEvent(m_sock, IOManager::WRITE);
}
auto Socket::cancelAccept() -> bool {
  return IOManager::GetThis()->cancelEvent(m_sock, IOManager::READ);
}
auto Socket::cancelAll() -> bool {
  return IOManager::GetThis()->cancelAll(m_sock);
}
void Socket::initSock() {
  int val = 1;
  setOption(SOL_SOCKET, SO_REUSEADDR, val);
  if (m_type == SOCK_STREAM) {
    setOption(IPPROTO_TCP, TCP_NODELAY, val);
  }
}
void Socket::newSock() {
  m_sock = socket(m_family, m_type, m_protocol);
  if (SYLAR_LIKELY(m_sock != -1)) {
    initSock();
  } else {
    HX_LOG_ERROR(g_logger) << "socket( )" << m_family << ", " << m_type << ", "
                           << m_protocol << ") errno = " << errno
                           << "errstr = " << strerror(errno);
  }
}

}  // namespace hx_sylar
