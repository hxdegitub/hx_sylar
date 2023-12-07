#ifndef __HX_SYLAR_SOCKET__
#define __HX_SYLAR_SOCKET__

#include <netinet/tcp.h>
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <memory>

#include "address.h"
#include "noncopyable.h"

namespace hx_sylar {
class Socket : public std::enable_shared_from_this<Socket>, public Noncopyable {
 public:
  using ptr = std::shared_ptr<Socket>;
  using weak_ptr = std::weak_ptr<Socket>;
  enum Type {
    /// TCP类型
    TCP = SOCK_STREAM,
    /// UDP类型
    UDP = SOCK_DGRAM
  };
  enum Family {
    /// IPv4 socket
    IPv4 = AF_INET,
    /// IPv6 socket
    IPv6 = AF_INET6,
    /// Unix socket
    UNIX = AF_UNIX,
  };

  static auto CreateTCP(hx_sylar::Address::ptr address) -> Socket::ptr;

  /**
   * @brief 创建UDP Socket(满足地址类型)
   * @param[in] address 地址
   */
  static auto CreateUDP(hx_sylar::Address::ptr address) -> Socket::ptr;
  static auto CreateTCPSocket() -> Socket::ptr;

  /**
   * @brief 创建IPv4的UDP Socket
   */
  static auto CreateUDPSocket() -> Socket::ptr;

  /**
   * @brief 创建IPv6的TCP Socket
   */
  static auto CreateTCPSocket6() -> Socket::ptr;

  /**
   * @brief 创建IPv6的UDP Socket
   */
  static auto CreateUDPSocket6() -> Socket::ptr;

  /**
   * @brief 创建Unix的TCP Socket
   */
  static auto CreateUnixTCPSocket() -> Socket::ptr;

  /**
   * @brief 创建Unix的UDP Socket
   */
  static auto CreateUnixUDPSocket() -> Socket::ptr;

  Socket(int famuly, int type, int protocol = 0);
  virtual ~Socket();

  auto getSendTimeout() -> uint64_t;
  void setSendTimeout(int64_t v);

  auto getRecvTimeout() -> int64_t;
  void setRecvTimeout(int64_t v);

  auto getOption(int level, int option, void* result, size_t* len) -> bool;
  template <class T>
  auto getOption(int level, int option, T& result) {
    size_t length = sizeof(T);
    return getOption(level, option, &result, &length);
  }

  auto setOption(int level, int option, const void* result, socklen_t len)
      -> bool;
  template <class T>
  auto setOption(int level, int option, const T& value) -> bool {
    return setOption(level, option, &value, sizeof(T));
  }

  virtual auto accept() -> Socket::ptr;

  virtual auto bind(const Address::ptr addr) -> bool;
  virtual auto connect(const Address::ptr addr, uint64_t timeout_ms = -1)
      -> bool;
  virtual auto reconnect(uint64_t timeout_ms = -1) -> bool;
  virtual auto listen(int backlog = SOMAXCONN) -> bool;
  virtual auto close() -> bool;

  virtual auto send(const void* buffer, size_t length, int flag = 0) -> int;
  virtual auto send(const iovec* buffers, size_t length, int flags = 0) -> int;
  virtual auto sendTo(const void* buuffer, size_t length, const Address::ptr to,
                      int flags = 0) -> int;
  virtual auto sendTo(const iovec* buffers, size_t length,
                      const Address::ptr to, int flags = 0) -> int;

  virtual auto recv(void* buffer, size_t length, int flags = 0) -> int;
  virtual auto recv(struct iovec* buffers, size_t length, int flags = 0) -> int;
  virtual auto recvFrom(void* buffer, size_t length, Address::ptr from,
                        int flags = 0) -> int;
  virtual auto recvFrom(iovec* buffers, size_t length, Address::ptr form,
                        int flags = 0) -> int;

  auto getRemoteAddress() -> Address::ptr;
  auto getLocalAddress() -> Address::ptr;

  auto getFamily() const -> int { return m_family; }
  auto getType() const -> int { return m_type; }
  auto getProtocol() const -> int { return m_protocol; }

  auto isConnected() const -> bool { return m_isConnected; }
  auto isValid() const -> bool;

  auto getError() -> int;

  virtual auto dump(std::ostream& os) const -> std::ostream&;
  virtual auto toString() const -> std::string;
  auto getSocket() const -> int;

  auto cancelRead() -> bool;
  auto cancelaaawrite() -> bool;
  auto cancelAccept() -> bool;
  auto cancelAll() -> bool;

 protected:
  void initSock();
  void newSock();
  virtual auto init(int sock) -> bool;

 protected:
  int m_sock;
  int m_family;
  int m_type;
  int m_protocol;
  bool m_isConnected;

  Address::ptr m_localAddress;
  Address::ptr m_remoteAddress;
};
class SSLSocket : public Socket {
 public:
  using ptr = std::shared_ptr<SSLSocket>;
  static auto CreateTcp(hx_sylar::Address::ptr address) -> SSLSocket::ptr;
  static auto CreateTCPSocket() -> SSLSocket::ptr;
  static auto CreateTCPSocket6() -> SSLSocket::ptr;

  SSLSocket(int famuly, int type, int protocol = 0);

  virtual auto accept() -> Socket::ptr override;
  // virtual auto bind(const Address::ptr addr) -> bool override;
  virtual auto connect(const Address::ptr addr, uint64_t timeout_ms = -1)
      -> bool override;
  virtual auto listen(int backlog = SOMAXCONN) -> bool override;
  virtual auto close() -> bool override;
  virtual auto send(const void* buffer, size_t length, int flags = 0)
      -> int override;
  virtual auto send(const iovec* buffers, size_t length, int flags = 0)
      -> int override;
  virtual auto sendTo(const void* buuffer, size_t length, const Address::ptr to,
                      int flags = 0) -> int override;
  virtual auto sendTo(const iovec* buffers, size_t length,
                      const Address::ptr to, int flags = 0) -> int override;
  virtual auto recv(void* buffer, size_t length, int flags = 0) -> int override;
  virtual auto recv(struct iovec* buffers, size_t length, int flags = 0)
      -> int override;
  virtual auto recvFrom(void* buffer, size_t length, Address::ptr from,
                        int flags = 0) -> int override;
  virtual auto recvFrom(iovec* buffers, size_t length, Address::ptr from,
                        int flags = 0) -> int override;

  auto loadCertificates(const std::string& cert_file,
                        const std::string& key_file) -> bool;

 protected:
  auto init(int sock) -> bool override;
  virtual auto dump(std::ostream& os) const -> std::ostream& override;

 private:
  std::shared_ptr<SSL_CTX> m_ctx;
  std::shared_ptr<SSL> m_ssl;
};

auto operator<<(std::ostream& os, const Socket& sock) -> std::ostream&;

}  // namespace hx_sylar

#endif
