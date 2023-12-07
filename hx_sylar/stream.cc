#include "stream.h"

namespace hx_sylar {

auto Stream::readFixSize(void* buffer, size_t length) -> int {
  size_t offset = 0;
  int64_t left = length;
  while (left > 0) {
    int64_t len = read(static_cast<char*>(buffer) + offset, left);
    if (len <= 0) {
      return len;
    }
    offset += len;
    left -= len;
  }
  return length;
}

auto Stream::readFixSize(ByteArray::ptr ba, size_t length) -> int {
  int64_t left = length;
  while (left > 0) {
    int64_t len = read(ba, left);
    if (len <= 0) {
      return len;
    }
    left -= len;
  }
  return length;
}

auto Stream::writeFixSize(const void* buffer, size_t length) -> int {
  size_t offset = 0;
  int64_t left = length;
  while (left > 0) {
    int64_t len = write(static_cast<const char*>(buffer) + offset, left);
    if (len <= 0) {
      return len;
    }
    offset += len;
    left -= len;
  }
  return length;
}

auto Stream::writeFixSize(ByteArray::ptr ba, size_t length) -> int {
  int64_t left = length;
  while (left > 0) {
    int64_t len = write(ba, left);
    if (len <= 0) {
      return len;
    }
    left -= len;
  }
  return length;
}

}  // namespace hx_sylar
