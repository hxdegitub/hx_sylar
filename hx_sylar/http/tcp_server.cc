#include "tcp_server.h"

#include <vector>

#include "hx_sylar/address.h"
#include "hx_sylar/config.h"
#include "hx_sylar/iomanager.h"
namespace hx_sylar {
hx_sylar::Logger::ptr m_logger = HX_LOG_NAME("system");
static hx_sylar::ConfigVar<uint64_t>::ptr g_tcp_server_read_timeout =
    hx_sylar::Config::Lookup("tcp_server.read_timeout",
                             static_cast<uint64_t>(60 * 1000 * 2),
                             "tcp server read timeout");

TcpServer::TcpServer(hx_sylar::IOManager* worker,
                     hx_sylar::IOManager* io_worker,
                     hx_sylar::IOManager* accept_worker)
    : m_worker(worker),
      m_ioWorker(io_worker),
      m_acceptWorker(accept_worker),
      m_recvTimeout(g_tcp_server_read_timeout->getValue()),
      m_name("sylar/1.0.0"),
      m_isStop(true) {}
TcpServer::~TcpServer() {
  for (auto& i : m_socks) {
    i->close();
  }
  m_socks.clear();
}

auto TcpServer::bind(hx_sylar::Address::ptr addr, bool ssl) -> bool {
  std::vector<Address::ptr> addrs;
  std::vector<Address::ptr> fails;
  addrs.push_back(addr);
  return bind(addrs, fails, ssl);
}

auto TcpServer::bind(const std::vector<Address::ptr>& addrs,
                     std::vector<Address::ptr>& fails, bool ssl) -> bool {
  m_ssl = ssl;
  for (auto& addr : addrs) {
    Socket::ptr sock =
        ssl ? SSLSocket::CreateTCP(addr) : Socket::CreateTCP(addr);
    if (!sock->bind(addr)) {
      HX_LOG_ERROR(m_logger)
          << "bind fail errno=" << errno << " errstr=" << strerror(errno)
          << " addr=[" << addr->toString() << "]";
      fails.push_back(addr);
      continue;
    }
    if (!sock->listen()) {
      HX_LOG_ERROR(m_logger)
          << "listen fail errno=" << errno << " errstr=" << strerror(errno)
          << " addr=[" << addr->toString() << "]";
      fails.push_back(addr);
      continue;
    }
    m_socks.push_back(sock);
  }

  if (!fails.empty()) {
    m_socks.clear();
    return false;
  }

  for (auto& i : m_socks) {
    HX_LOG_INFO(m_logger) << "type=" << m_type << " name=" << m_name
                          << " ssl=" << m_ssl << " server bind success: " << *i;
  }
  return true;
}

auto TcpServer::start() -> bool {
  if (!m_isStop) {
    return false;
  }
  m_isStop = false;

  for (auto& sock : m_socks) {
    m_acceptWorker->schedule(
        [capture0 = shared_from_this(), sock] { capture0->startAccept(sock); });
  }
  return true;
}
void TcpServer::startAccept(Socket::ptr sock) {
  while (!m_isStop) {
    Socket::ptr client = sock->accept();
    if (client) {
      client->setRecvTimeout(m_recvTimeout);
      m_ioWorker->schedule(
          std::bind(&TcpServer::handleClient, shared_from_this(), client));
    } else {
      HX_LOG_ERROR(m_logger)
          << "accept errno=" << errno << " errstr=" << strerror(errno);
    }
  }
}
void TcpServer::stop() {
  m_isStop = true;
  auto self = shared_from_this();
  m_acceptWorker->schedule([this, self]() {
    for (auto& sock : m_socks) {
      sock->cancelAll();
      sock->close();
    }
    m_socks.clear();
  });
}
void TcpServer::handleClient(Socket::ptr& client) {
  HX_LOG_INFO(m_logger) << " handleClient " << *client;
}

auto TcpServer::loadCertificates(const std::string& cert_file,
                                 const std::string& key_file) -> bool {
  for (auto& i : m_socks) {
    auto ssl_socket = std::dynamic_pointer_cast<SSLSocket>(i);
    if (ssl_socket) {
      if (!ssl_socket->loadCertificates(cert_file, key_file)) {
        return false;
      }
    }
  }

  return true;
}
auto TcpServer::toString(const std::string& prefix) -> std::string {
  std::stringstream ss;
  ss << prefix << "[type=" << m_type << " name=" << m_name << " ssl=" << m_ssl
     << " worker=" << (m_worker != nullptr ? m_worker->getName() : "")
     << " accept="
     << (m_acceptWorker != nullptr ? m_acceptWorker->getName() : "")
     << " recv_timeout=" << m_recvTimeout << "]" << std::endl;
  std::string pfx = prefix.empty() ? "    " : prefix;
  for (auto& i : m_socks) {
    ss << pfx << pfx << *i << std::endl;
  }
  return ss.str();
}
}  // namespace hx_sylar