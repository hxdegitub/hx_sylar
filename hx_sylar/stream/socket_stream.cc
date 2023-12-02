#include "socket_stream.h"

#include "../bytearray.h"
namespace hx_sylar {
SocketStream::SocketStream(Socket::ptr sock, bool owner)
    : m_socket(sock), m_owner(owner) {}
auto SocketStream::read(void* buffer, size_t length) -> int {
  if (!isConnected()) {
    return -1;
  }
  return m_socket->recv(buffer, length);
}
SocketStream::~SocketStream() {
  if (m_owner && m_socket) {
    m_socket->close();
  }
}
auto SocketStream::read(ByteArray::ptr& ba, size_t length) -> int {
  if (!isConnected()) {
    return -1;
  }

  std::vector<iovec> iovs;
  ba->getWriteBuffers(iovs, length);
  int rt = m_socket->recv(&iovs[0], iovs.size());
  if (rt > 0) {
    ba->setPosition(ba->getPosition() + rt);
  }
  return rt;
}

auto SocketStream::write(const void* buffer, size_t length) -> int {
  if (!isConnected()) {
    return -1;
  }
  return m_socket->send(buffer, length);
}
auto SocketStream::write(ByteArray::ptr& ba, size_t length) -> int {
  if (!isConnected()) {
    return -1;
  }
  std::vector<iovec> iovs;
  ba->getReadBuffers(iovs, length);
  int rt = m_socket->send(&iovs[0], iovs.size());
  if (rt > 0) {
    ba->setPosition(ba->getPosition() + rt);
  }
  return rt;
}

void SocketStream::close() {
  if (m_socket) {
    m_socket->close();
  }
}

auto SocketStream::isConnected() const -> bool {
  return m_socket && m_socket->isConnected();
}

auto SocketStream::getRemoteAddress() -> Address::ptr {
  if (m_socket) {
    return m_socket->getRemoteAddress();
  }
  return nullptr;
}

auto SocketStream::getLocalAddress() -> Address::ptr {
  if (m_socket) {
    return m_socket->getLocalAddress();
  }
  return nullptr;
}

auto SocketStream::getRemoteAddressString() -> std::string {
  auto addr = getRemoteAddress();
  if (addr) {
    return addr->toString();
  }
  return "";
}
auto SocketStream::getLocalAddressString() -> std::string {
  auto addr = getLocalAddress();
  if (addr) {
    return addr->toString();
  }
  return "";
}
}  // namespace hx_sylar
