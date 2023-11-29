#include "address.h"

#include <ifaddrs.h>
#include <netdb.h>
#include <stddef.h>

#include <cstdint>
#include <memory>
#include <sstream>

#include "endian.h"
#include "log.h"

namespace hx_sylar {
static hx_sylar::Logger::ptr g_logger = HX_LOG_NAME("system");
template <class T>
static auto CreateMask(uint32_t bits) -> T {
  return (1 << (sizeof(T) * 8 - bits)) - 1;
}

template <class T>
static auto CountBytes(T value) -> uint32_t {
  uint32_t result = 0;
  for (; value; ++result) {
    value &= value - 1;
  }
  return result;
}

auto Address::LookupAny(const std::string& host, int family, int type,
                        int protocol) -> Address::ptr {
  std::vector<Address::ptr> result;
  if (Lookup(result, host, family, type, protocol)) {
    return result[0];
  }
  return nullptr;
}

auto Address::Lookup(std::vector<Address::ptr>& result, const std::string& host,
                     int family, int type, int protocol) -> bool {
  return false;
}
auto Address::getFamily() const -> int { return getAddr()->sa_family; }

auto Address::toString() const -> std::string {
  std::stringstream ss;
  insert(ss);
  return ss.str();
}
auto Address::Create(const sockaddr* addr, socklen_t addrlen) -> Address::ptr {
  if (addr == nullptr) {
    return nullptr;
  }
  Address::ptr result;
  switch (addr->sa_family) {
    case AF_INET:
      result.reset(new IPv4Address(*(const sockaddr_in*)addr));
      break;
    case AF_INET6:
      result.reset(new IPv6Address(*(const sockaddr_in6*)addr));
      break;
    default:
      result.reset(new UnknownAddress(*addr));
      break;
  }
  return result;
}
auto Address::operator<(const Address& rhs) const -> bool {
  socklen_t minlen = std::min(getAddrLen(), rhs.getAddrLen());
  int result = memcmp(getAddr(), rhs.getAddr(), minlen);
  if (result < 0) {
    return true;
  }
  if (result > 0) {
    return false;
  }
  if (getAddrLen() < rhs.getAddrLen()) {
    return true;
  }
  return false;
}

auto Address::operator==(const Address& rhs) const -> bool {
  return getAddrLen() == rhs.getAddrLen() &&
         memcmp(getAddr(), rhs.getAddr(), getAddrLen()) == 0;
}

auto Address::operator!=(const Address& rhs) const -> bool {
  return !(*this == rhs);
}

auto IPAddress::Create(const char* address, uint16_t port) -> IPAddress::ptr {
  addrinfo hints, *results;
  memset(&hints, 0, sizeof(addrinfo));

  hints.ai_flags = AI_NUMERICHOST;
  hints.ai_family = AF_UNSPEC;

  int error = getaddrinfo(address, nullptr, &hints, &results);
  if (error != 0) {
    HX_LOG_DEBUG(g_logger) << "IPAddress::Create *" << address << ", " << port
                           << ") error= " << strerror(errno);
    return nullptr;
  }
  try {
    IPAddress::ptr result = std::dynamic_pointer_cast<IPAddress>(
        Address::Create(results->ai_addr, (socklen_t)results->ai_addrlen));
    if (result) {
      result->setPort(port);
    }
    freeaddrinfo(results);
    return result;
  } catch (...) {
    freeaddrinfo(results);
    return nullptr;
  }
}
auto IPv4Address::Create(const char* address, uint16_t port)
    -> IPv4Address::ptr {
  auto rt = std::make_shared<IPv4Address>();

  rt->m_addr.sin_port = byteswapOnLittleEndian(port);
  int result = inet_pton(AF_INET, address, &rt->m_addr.sin_addr);
  if (result <= 0) {
    HX_LOG_DEBUG(g_logger) << "IPv4Address::Create(" << address << "," << port
                           << ") rt = " << result << " errno " << errno
                           << " errstr =  " << strerror(errno);
    return nullptr;
  }
  return rt;
}
IPv4Address::IPv4Address(const sockaddr_in& address) { m_addr = address; }

IPv4Address::IPv4Address(uint32_t address, uint16_t port) {
  memset(&m_addr, 0, sizeof(m_addr));
  m_addr.sin_family = AF_INET;
  m_addr.sin_port = byteswapOnLittleEndian(port);
  m_addr.sin_addr.s_addr = byteswapOnLittleEndian(address);
}
auto IPv4Address::getAddr() -> sockaddr* { return (sockaddr*)&m_addr; }

auto IPv4Address::getAddr() const -> const sockaddr* {
  return (sockaddr*)&m_addr;
}

auto IPv4Address::getAddrLen() const -> socklen_t { return sizeof(m_addr); }

auto IPv4Address::insert(std::ostream& os) const -> std::ostream& {
  uint32_t addr = byteswapOnLittleEndian(m_addr.sin_addr.s_addr);
  os << ((addr >> 24) & 0xff) << "." << ((addr >> 16) & 0xff) << "."
     << ((addr >> 8) & 0xff) << "." << (addr & 0xff);

  os << ":" << byteswapOnLittleEndian(m_addr.sin_port);
  return os;
}
auto IPv4Address::broadcastAddress(uint32_t prefix_len) -> IPAddress::ptr {
  if (prefix_len > 32) {
    return nullptr;
  }

  sockaddr_in baddr(m_addr);
  baddr.sin_addr.s_addr |=
      byteswapOnLittleEndian(CreateMask<uint32_t>(prefix_len));
  return std::make_shared<IPv4Address>(baddr);
}

auto IPv4Address::networdAddress(uint32_t prefinx_len) -> IPAddress::ptr {
  if (prefinx_len > 32) {
    return nullptr;
  }
  sockaddr_in baddr(m_addr);
  baddr.sin_addr.s_addr &=
      byteswapOnLittleEndian(CreateMask<uint32_t>(prefinx_len));
  return std::make_shared<IPv4Address>(baddr);
}

auto IPv4Address::subnetMask(uint32_t prefix_len) -> IPAddress::ptr {
  sockaddr_in subnet;
  memset(&subnet, 0, sizeof(subnet));
  subnet.sin_family = AF_INET;
  subnet.sin_addr.s_addr =
      ~byteswapOnLittleEndian(CreateMask<uint32_t>(prefix_len));
  return std::make_shared<IPv4Address>(subnet);
}

auto IPv4Address::getPort() const -> uint32_t {
  return byteswapOnLittleEndian(m_addr.sin_port);
}

void IPv4Address::setPort(uint16_t v) {
  m_addr.sin_port = byteswapOnLittleEndian(v);
}

IPv6Address::IPv6Address() {
  memset(&m_addr, 0, sizeof(m_addr));
  m_addr.sin6_family = AF_INET6;
}
auto IPv6Address::getAddr() -> sockaddr* { return (sockaddr*)&m_addr; }

auto IPv6Address::getAddr() const -> const sockaddr* {
  return (sockaddr*)&m_addr;
}
auto IPv6Address::broadcastAddress(uint32_t prefix_len) -> IPAddress::ptr {
  sockaddr_in6 baddr(m_addr);
  baddr.sin6_addr.s6_addr[prefix_len / 8] |=
      CreateMask<uint8_t>(prefix_len % 8);
  for (int i = prefix_len / 8 + 1; i < 16; ++i) {
    baddr.sin6_addr.s6_addr[i] = 0xff;
  }
  return std::make_shared<IPv6Address>(baddr);
}
auto IPv6Address::networdAddress(uint32_t prefix_len) -> IPAddress::ptr {
  sockaddr_in6 baddr(m_addr);
  baddr.sin6_addr.s6_addr[prefix_len / 8] &=
      CreateMask<uint8_t>(prefix_len % 8);
  for (int i = prefix_len / 8 + 1; i < 16; ++i) {
    baddr.sin6_addr.s6_addr[i] = 0x00;
  }
  return std::make_shared<IPv6Address>(baddr);
}
IPv6Address::IPv6Address(const sockaddr_in6& address) { m_addr = address; }

IPv6Address::IPv6Address(const uint8_t address[16], uint16_t port) {
  memset(&m_addr, 0, sizeof(m_addr));
  m_addr.sin6_family = AF_INET6;
  m_addr.sin6_port = byteswapOnLittleEndian(port);
  memcpy(&m_addr.sin6_addr.s6_addr, address, 16);
}

auto IPv6Address::subnetMask(uint32_t prefix_len) -> IPAddress::ptr {
  sockaddr_in6 subnet;
  memset(&subnet, 0, sizeof(subnet));
  subnet.sin6_family = AF_INET6;
  subnet.sin6_addr.s6_addr[prefix_len / 8] =
      ~CreateMask<uint8_t>(prefix_len % 8);

  for (uint32_t i = 0; i < prefix_len / 8; ++i) {
    subnet.sin6_addr.s6_addr[i] = 0xff;
  }
  return std::make_shared<IPv6Address>(subnet);
}

auto IPv6Address::getAddrLen() const -> socklen_t { return sizeof(m_addr); }

auto IPv6Address::insert(std::ostream& os) const -> std::ostream& {
  os << "[";
  uint16_t* addr = (uint16_t*)(m_addr.sin6_addr.s6_addr);
  bool used_zeros = false;
  for (size_t i = 0; i < 8; ++i) {
    if (addr[i] == 0 && !used_zeros) {
      continue;
    }

    if (i && addr[i - 1] == 0 && !used_zeros) {
      os << " :";
      used_zeros = true;
    }

    if (i) {
      os << ":";
    }
    os << std::hex << static_cast<int>(byteswapOnLittleEndian(addr[i]))
       << std::dec;
  }

  if (!used_zeros && addr[7] == 0) {
    os << "::";
  }
  os << "]" << byteswapOnLittleEndian(m_addr.sin6_port);
  return os;
}

auto IPv6Address::getPort() const -> uint32_t {
  return byteswapOnLittleEndian(m_addr.sin6_port);
}

void IPv6Address::setPort(uint16_t v) {
  m_addr.sin6_port = byteswapOnLittleEndian(v);
}
static const size_t MAX_PATH_LEN =
    sizeof(((sockaddr_un*)nullptr)->sun_path) - 1;
UnixAddress::UnixAddress() {
  memset(&m_addr, 0, sizeof m_addr);
  m_addr.sun_family = AF_UNIX;
  m_length = offsetof(sockaddr_un, sun_path) + MAX_PATH_LEN;
}

UnixAddress::UnixAddress(const std::string& path) {
  memset(&m_addr, 0, sizeof m_addr);
  m_addr.sun_family = AF_UNIX;
  m_length = path.size() + 1;

  if (!path.empty() && path[0] == 0) {
    --m_length;
  }
  if (m_length > sizeof(m_addr.sun_path)) {
    throw std::logic_error("path too long ");
    m_length += offsetof(sockaddr_un, sun_path);
  }
}

void UnixAddress::setAddrLen(uint32_t v) { m_length = v; }

auto UnixAddress::getAddr() -> sockaddr* { return (sockaddr*)(&m_addr); }

auto UnixAddress::getAddr() const -> const sockaddr* {
  return (sockaddr*)&m_addr;
}
auto UnixAddress::getAddrLen() const -> socklen_t { return m_length; }

auto UnixAddress::getPath() const -> std::string {
  std::stringstream ss;
  if (m_length > offsetof(sockaddr_un, sun_path) && m_addr.sun_path[0] == 0) {
    ss << "\\0"
       << std::string(m_addr.sun_path + 1,
                      m_length - offsetof(sockaddr_un, sun_path) - 1);
  } else {
    ss << m_addr.sun_path;
  }
  return ss.str();
}
auto UnixAddress::insert(std::ostream& os) const -> std::ostream& {
  std::stringstream ss;
  if (m_length > offsetof(sockaddr_un, sun_path) && m_addr.sun_path[0] == 0) {
    ss << "\\0"
       << std::string(m_addr.sun_path + 1,
                      m_length - offsetof(sockaddr_un, sun_path) - 1);
  } else {
    ss << m_addr.sun_path;
  }
  return os;
}
UnknownAddress::UnknownAddress(int family) {
  memset(&m_addr, 0, sizeof(m_addr));
  m_addr.sa_family = family;
}

UnknownAddress::UnknownAddress(const sockaddr& addr) { m_addr = addr; }

auto UnknownAddress::getAddr() -> sockaddr* { return &m_addr; }

auto UnknownAddress::getAddr() const -> const sockaddr* { return &m_addr; }

auto UnknownAddress::getAddrLen() const -> socklen_t { return sizeof(m_addr); }

auto UnknownAddress::insert(std::ostream& os) const -> std::ostream& {
  os << "[UnknownAddress family=" << m_addr.sa_family << "]";
  return os;
}

auto operator<<(std::ostream& os, const Address& addr) -> std::ostream& {
  return addr.insert(os);
}
}  // namespace hx_sylar
