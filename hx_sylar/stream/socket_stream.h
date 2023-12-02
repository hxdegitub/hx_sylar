#ifndef __HX_SYLAR_SOCKET_STREAM_H__
#define __HX_SYLAR_SOCKET_STREAM_H__
#include <memory>

#include "hx_sylar/bytearray.h"
#include "hx_sylar/iomanager.h"
#include "hx_sylar/mutex.h"
#include "hx_sylar/socket.h"
#include "hx_sylar/stream.h"
namespace hx_sylar {
class SocketStream : public Stream {
 public:
  using ptr = std::shared_ptr<SocketStream>;

  explicit SocketStream(Socket::ptr sock, bool owner = true);

  ~SocketStream() override;

  virtual auto read(void* buffer, size_t length) -> int override;
  virtual auto read(ByteArray::ptr& ba, size_t length) -> int override;

  virtual auto write(const void* buffer, size_t length) -> int override;

  virtual auto write(ByteArray::ptr& ba, size_t length) -> int override;

  virtual void close() override;

  auto getSocket() const -> Socket::ptr { return m_socket; }

  /**
   * @brief 返回是否连接
   */
  auto isConnected() const -> bool;

  Address::ptr getRemoteAddress();
  Address::ptr getLocalAddress();
  std::string getRemoteAddressString();
  std::string getLocalAddressString();

 protected:
  Socket::ptr m_socket;
  bool m_owner;
};
}  // namespace hx_sylar
#endif
