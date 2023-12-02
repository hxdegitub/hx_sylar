#include <vector>

#include "../hx_sylar/bytearray.h"
#include "../hx_sylar/hx_sylar.h"
#include "../hx_sylar/log.h"
using namespace std;
hx_sylar::Logger::ptr g_logger = HX_LOG_NAME("system");
void test() {
#define XX(type, len, write_fun, read_fun, base_len)                       \
  {                                                                        \
    std::vector<type> vec((len));                                          \
    for (int i = 0; i < (len); ++i) {                                      \
      vec.push_back(rand());                                               \
    }                                                                      \
    hx_sylar::ByteArray::ptr ba(new hx_sylar::ByteArray(base_len));        \
    for (auto& i : vec) {                                                  \
      ba->write_fun(i);                                                    \
    }                                                                      \
    ba->setPosition(0);                                                    \
    for (size_t i = 0; i < vec.size(); ++i) {                              \
      type v = ba->read_fun();                                             \
      HX_ASSERT(v == vec[i]);                                              \
    }                                                                      \
    HX_ASSERT(ba->getReadSize() == 0);                                     \
    HX_LOG_INFO(g_logger) << #write_fun "/" #read_fun " (" #type " ) len=" \
                          << (len) << " base_len=" << (base_len)           \
                          << " size=" << ba->getSize();                    \
  }
  XX(int8_t, 100, writeFint8, readFint8, 1);
  XX(int8_t, 100, writeFint8, readFint8, 1);
  XX(uint8_t, 100, writeFuint8, readFuint8, 1);
  XX(int16_t, 100, writeFint16, readFint16, 1);
  XX(uint16_t, 100, writeFuint16, readFuint16, 1);
  XX(int32_t, 100, writeFint32, readFint32, 1);
  XX(uint32_t, 100, writeFuint32, readFuint32, 1);
  XX(int64_t, 100, writeFint64, readFint64, 1);
  XX(uint64_t, 100, writeFuint64, readFuint64, 1);

  XX(int32_t, 100, writeInt32, readInt32, 1);
  XX(uint32_t, 100, writeUint32, readUint32, 1);
  XX(int64_t, 100, writeInt64, readInt64, 1);
}
int main() {
  test();
  return 0;
}