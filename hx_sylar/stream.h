#ifndef __HX_SYLAR_STREAM_H__
#define __HX_SYLAR_STREAM_H__
#include <memory>

#include "bytearray.h"
namespace hx_sylar {
class Stream {
 public:
  using ptr = std::shared_ptr<Stream>;
  /**
   * @brief 析构函数
   */
  virtual ~Stream(){};

  /**
   * @brief 读数据
   * @param[out] buffer 接收数据的内存
   * @param[in] length 接收数据的内存大小
   * @return
   *      @retval >0 返回接收到的数据的实际大小
   *      @retval =0 被关闭
   *      @retval <0 出现流错误
   */
  virtual auto read(void* buffer, size_t length) -> int = 0;

  virtual auto read(ByteArray::ptr& ba, size_t length) -> int = 0;

  virtual auto readFixSize(void* buffer, size_t length) -> int;

  virtual auto readFixSize(ByteArray::ptr& ba, size_t length) -> int;

  virtual auto write(const void* buffer, size_t length) -> int = 0;

  virtual auto write(ByteArray::ptr& ba, size_t length) -> int = 0;

  virtual auto writeFixSize(const void* buffer, size_t length) -> int;

  virtual auto writeFixSize(ByteArray::ptr& ba, size_t length) -> int;

  /**
   * @brief 关闭流
   */
  virtual void close() = 0;
};
}  // namespace hx_sylar
#endif