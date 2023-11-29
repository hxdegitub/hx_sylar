#ifndef __HX_SYLAR_SOCKET__
#define __HX_SYLAR_SOCKET__

#include <netinet/tcp.h>
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

  Socket(int famuly, int type, int protocol = 0);
  ~Socket();

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

  auto accpet() -> Socket::ptr;

  auto init(int sock) -> bool;
  auto bind(const Address::ptr& addr) -> bool;
  auto connect(const Address::ptr addr, uint64_t timeout_ms = -1) -> bool;
  auto listen(int backlog = SOMAXCONN) -> bool;
  auto close() -> bool;

  auto send(const void* buffer, size_t length, int flag = 0) -> int;
  auto send(const iovec* buffers, size_t length, int flags = 0) -> int;
  auto sendTo(const void* buuffer, size_t length, const Address::ptr to,
              int flags = 0) -> int;
  auto sendTo(const iovec* buffers, size_t length, const Address::ptr to,
              int flags = 0) -> int;

  auto recv(void* buffer, size_t length, int flags = 0) -> int;
  auto recv(struct iovc* buffers, size_t length, int flags = 0) -> int;
  auto recvFrom(void* buffer, size_t length, Address::ptr from, int flags = 0)
      -> int;
  auto recvFrom(iovec* buffers, size_t length, Address::ptr form, int flags = 0)
      -> int;

  auto getRemoteAddress() -> Address::ptr;
  auto getLocalAddress() -> Address::ptr;

  auto getFamily() const -> int { return m_family; }
  auto getType() const -> int { return m_type; }
  auto getProtocol() const -> int { return m_protocol; }

  auto isConnected() const -> bool { return m_isConnected; }
  auto isValid() const -> bool;

  auto getError() -> int;

  auto dump(std::ostream& os) const -> std::ostream&;

  auto getSocket() const -> int;

  auto cancelRead() -> bool;
  auto cancelaaawrite() -> bool;
  auto cancelAccept() -> bool;
  auto cancelAll() -> bool;

 private:
  void initSock();
  void newSock();

 private:
  int m_sock;
  int m_family;
  int m_type;
  int m_protocol;
  bool m_isConnected;

  Address::ptr m_localAddress;
  Address::ptr m_remoteAddress;
};
}  // namespace hx_sylar

#endif
