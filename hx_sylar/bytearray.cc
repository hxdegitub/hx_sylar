#include "bytearray.h"

#include <math.h>
#include <string.h>
#include <sys/types.h>

#include <cstddef>
#include <fstream>
#include <iomanip>
#include <sstream>

#include "endian.h"
#include "log.h"

namespace hx_sylar {
static hx_sylar::Logger::ptr g_logger = HX_LOG_NAME("system");
ByteArray::Node::Node(size_t s) : ptr(new char[s]), next(nullptr), size(s) {}
ByteArray::Node::Node() : ptr(nullptr), next(nullptr), size(0) {}

ByteArray::Node::~Node() { delete[] ptr; }

ByteArray::ByteArray(size_t base_size)
    : m_baseSize(base_size),
      m_position(0),
      m_capacity(base_size),
      m_size(0),
      m_endian(SYLAR_BIG_ENDIAN),
      m_root(new Node(base_size)),
      m_cur(m_root) {}
ByteArray::~ByteArray() {
  Node* tmp = m_root;
  while (tmp != nullptr) {
    m_cur = tmp;
    tmp = tmp->next;
    delete m_cur;
  }
}

auto ByteArray::isLittleEndian() const -> bool {
  return m_endian == SYLAR_LITTLE_ENDIAN;
}
void ByteArray::setIsLittleEndian(bool val) {
  if (val) {
    m_endian = SYLAR_LITTLE_ENDIAN;
  }
  m_endian = SYLAR_BIG_ENDIAN;
}

void ByteArray::writeFint8(int8_t value) { write(&value, sizeof value); }

void ByteArray::writeFuint8(uint8_t value) { write(&value, sizeof(value)); }

void ByteArray::writeFint16(int16_t value) {
  if (m_endian != SYLAR_BYTE_ORDER) {
    value = byteswap(value);
  }
  write(&value, sizeof(value));
}

void ByteArray::writeFuint16(uint16_t value) {
  if (m_endian != SYLAR_BYTE_ORDER) {
    value = byteswap(value);
  }
  write(&value, sizeof(value));
}

void ByteArray::writeFint32(int32_t value) {
  if (m_endian != SYLAR_BYTE_ORDER) {
    value = byteswap(value);
  }
  write(&value, sizeof(value));
}

void ByteArray::writeFuint32(uint32_t value) {
  if (m_endian != SYLAR_BYTE_ORDER) {
    value = byteswap(value);
  }
  write(&value, sizeof(value));
}

void ByteArray::writeFint64(int64_t value) {
  if (m_endian != SYLAR_BYTE_ORDER) {
    value = byteswap(value);
  }
  write(&value, sizeof(value));
}

void ByteArray::writeFuint64(uint64_t value) {
  if (m_endian != SYLAR_BYTE_ORDER) {
    value = byteswap(value);
  }
  write(&value, sizeof(value));
}
static auto EncodeZigzag32(const int32_t& v) -> uint32_t {
  if (v < 0) {
    return (static_cast<uint32_t>(-v)) * 2 - 1;
  }
  return v * 2;
}

static auto EncodeZigzag64(const int64_t& v) -> uint64_t {
  if (v < 0) {
    return (static_cast<uint64_t>(-v) * 2 - 1);
  }
  return v * 2;
}
static auto DecodeZigzag32(const uint32_t& v) -> int32_t {
  return (v >> 1) ^ -(v & 1);
}

static auto DecodeZigzag64(const uint64_t& v) -> int64_t {
  return (v >> 1) ^ -(v & 1);
}
void ByteArray::writeInt32(int32_t value) {
  writeUint32(EncodeZigzag32(value));
}

void ByteArray::writeUint32(uint32_t value) {
  uint8_t tmp[5];
  uint8_t i = 0;
  while (value >= 0x80) {
    tmp[i++] = (value & 0x7F) | 0x80;
    value >>= 7;
  }
  tmp[i++] = value;
  write(tmp, i);
}
void ByteArray::writeInt64(int64_t value) {
  writeUint64(EncodeZigzag64(value));
}

void ByteArray::writeUint64(uint64_t value) {
  uint8_t tmp[10];
  uint8_t i = 0;
  while (value >= 0x80) {
    tmp[i++] = (value & 0x7F) | 0x80;
    value >>= 7;
  }
  tmp[i++] = value;
  write(tmp, i);
}

void ByteArray::writeFloat(float value) {
  uint32_t v;
  memcpy(&v, &value, sizeof(value));
  writeFuint32(v);
}
void ByteArray::writeStringF16(const std::string& value) {
  writeFuint16(value.size());
  write(value.c_str(), value.size());
}

void ByteArray::writeStringF32(const std::string& value) {
  writeFuint32(value.size());
  write(value.c_str(), value.size());
}

void ByteArray::writeStringF64(const std::string& value) {
  writeFuint64(value.size());
  write(value.c_str(), value.size());
}

void ByteArray::writeStringVint(const std::string& value) {
  writeUint64(value.size());
  write(value.c_str(), value.size());
}

void ByteArray::writeStringWithoutLength(const std::string& value) {
  write(value.c_str(), value.size());
}

auto ByteArray::readFint8() -> int8_t {
  int8_t v;
  read(&v, sizeof(v));
  return v;
}

auto ByteArray::readFuint8() -> uint8_t {
  uint8_t v;
  read(&v, sizeof(v));
  return v;
}

#define XX(type)                      \
  type v;                             \
  read(&v, sizeof(v));                \
  if (m_endian == SYLAR_BYTE_ORDER) { \
    return v;                         \
  }                                   \
  return byteswap(v);

auto ByteArray::readFint16() -> int16_t { XX(int16_t); }
auto ByteArray::readFuint16() -> uint16_t { XX(uint16_t); }

auto ByteArray::readFint32() -> int32_t { XX(int32_t); }

auto ByteArray::readFuint32() -> uint32_t { XX(uint32_t); }

auto ByteArray::readFint64() -> int64_t { XX(int64_t); }

auto ByteArray::readFuint64() -> uint64_t { XX(uint64_t); }
#undef XX

auto ByteArray::readInt32() -> int32_t { return DecodeZigzag32(readUint32()); }

auto ByteArray::readUint32() -> uint32_t {
  uint32_t result = 0;
  for (int i = 0; i < 32; i += 7) {
    uint8_t b = readFuint8();
    if (b < 0x80) {
      result |= (static_cast<uint32_t>(b)) << i;
      break;
    }
    result |= ((static_cast<uint32_t>(b & 0x7f)) << i);
  }
  return result;
}

auto ByteArray::readInt64() -> int64_t { return DecodeZigzag64(readUint64()); }

auto ByteArray::readUint64() -> uint64_t {
  uint64_t result = 0;
  for (int i = 0; i < 64; i += 7) {
    uint8_t b = readFuint8();
    if (b < 0x80) {
      result |= (static_cast<uint64_t>(b)) << i;
      break;
    }
    result |= ((static_cast<uint64_t>(b & 0x7f)) << i);
  }
  return result;
}

auto ByteArray::readFloat() -> float {
  uint32_t v = readFuint32();
  float value;
  memcpy(&value, &v, sizeof(v));
  return value;
}

auto ByteArray::readDouble() -> double {
  uint64_t v = readFuint64();
  double value;
  memcpy(&value, &v, sizeof(v));
  return value;
}

auto ByteArray::readStringF16() -> std::string {
  uint16_t len = readFuint16();
  std::string buff;
  buff.resize(len);
  read(&buff[0], len);
  return buff;
}

auto ByteArray::readStringF32() -> std::string {
  uint32_t len = readFuint32();
  std::string buff;
  buff.resize(len);
  read(&buff[0], len);
  return buff;
}

auto ByteArray::readStringF64() -> std::string {
  uint64_t len = readFuint64();
  std::string buff;
  buff.resize(len);
  read(&buff[0], len);
  return buff;
}

auto ByteArray::readStringVint() -> std::string {
  uint64_t len = readUint64();
  std::string buff;
  buff.resize(len);
  read(&buff[0], len);
  return buff;
}

void ByteArray::clear() {
  m_position = m_size = 0;
  m_capacity = m_baseSize;
  Node* tmp = m_root->next;
  while (tmp != nullptr) {
    m_cur = tmp;
    tmp = tmp->next;
    delete m_cur;
  }
  m_cur = m_root;
  m_root->next = nullptr;
}

void ByteArray::write(const void* buf, size_t size) {
  if (size == 0) {
    return;
  }
  addCapacity(size);

  size_t npos = m_position % m_baseSize;
  size_t ncap = m_cur->size - npos;
  size_t bpos = 0;

  while (size > 0) {
    if (ncap >= size) {
      memcpy(m_cur->ptr + npos, (const char*)buf + bpos, size);
      if (m_cur->size == (npos + size)) {
        m_cur = m_cur->next;
      }

      m_position += size;
      bpos += size;
      size = 0;
    } else {
      memcpy(m_cur->ptr + npos, (const char*)buf + bpos, ncap);
      m_position += ncap;
      bpos += ncap;
      size -= ncap;
      m_cur = m_cur->next;
      ncap = m_cur->size;
      npos = 0;
    }
  }

  if (m_position > m_size) {
    m_size = m_position;
  }
}
void ByteArray::setPosition(size_t val) {
  if (val > m_size) {
    throw std::out_of_range("set_position out of range");
  }

  m_position = val;
  m_cur = m_root;
  while (val > m_cur->size) {
    val -= m_cur->size;
    m_cur = m_cur->next;
  }
  if (val == m_cur->size) {
    m_cur = m_cur->next;
  }
}

void ByteArray::read(void* buf, size_t size, size_t position) const {
  if (size > (m_size - position)) {
    throw std::out_of_range("not enough len");
  }

  size_t npos = position % m_baseSize;
  size_t ncap = m_cur->size - npos;
  size_t bpos = 0;
  Node* cur = m_cur;
  while (size > 0) {
    if (ncap >= size) {
      memcpy((char*)buf + bpos, cur->ptr + npos, size);
      if (cur->size == (npos + size)) {
        cur = cur->next;
      }
      position += size;
      bpos += size;
      size = 0;
    } else {
      memcpy((char*)buf + bpos, cur->ptr + npos, ncap);
      position += ncap;
      bpos += ncap;
      size -= ncap;
      cur = cur->next;
      ncap = cur->size;
      npos = 0;
    }
  }
}
void ByteArray::read(void* buf, size_t size) {
  if (size > getReadSize()) {
    throw std::out_of_range("not enough len");
  }

  size_t npos = m_position % m_baseSize;
  size_t ncap = m_cur->size - npos;
  size_t bpos = 0;
  while (size > 0) {
    if (ncap >= size) {
      memcpy(static_cast<char*>(buf) + bpos, m_cur->ptr + npos, size);
      if (m_cur->size == (npos + size)) {
        m_cur = m_cur->next;
      }
      m_position += size;
      bpos += size;
      size = 0;
    } else {
      memcpy(static_cast<char*>(buf) + bpos, m_cur->ptr + npos, ncap);
      m_position += ncap;
      bpos += ncap;
      size -= ncap;
      m_cur = m_cur->next;
      ncap = m_cur->size;
      npos = 0;
    }
  }
}

auto ByteArray::writeToFile(const std::string& name) const -> bool {
  std::ofstream ofs;
  ofs.open(name, std::ios::trunc | std::ios::binary);
  if (!ofs) {
    HX_LOG_ERROR(g_logger) << "writeToFile name =" << name
                           << "error , errno = " << errno
                           << " errstr =" << strerror(errno);
    return false;
  }

  int64_t read_size = getReadSize();
  int64_t pos = m_position;
  Node* cur = m_cur;
  while (read_size > 0) {
    int diff = pos % m_baseSize;
    int64_t len = (read_size > static_cast<int64_t>(m_baseSize) ? m_baseSize
                                                                : read_size) -
                  diff;

    ofs.write(cur->ptr + diff, len);
    cur = cur->next;
    pos += len;
    read_size -= len;
  }
  return true;
}

auto ByteArray::readFromFile(const std::string& name) -> bool {
  std::ifstream ifs;
  ifs.open(name, std::ios::binary);

  if (!ifs) {
    HX_LOG_ERROR(g_logger) << "readFromFile name = " << name
                           << " error , errno = " << errno
                           << "errstr = " << strerror(errno);
    return false;
  }

  std::shared_ptr<char> buff(new char[m_baseSize],
                             [](const char* cptr) { delete[] cptr; });
  while (!ifs.eof()) {
    ifs.read(buff.get(), m_baseSize);
    write(buff.get(), ifs.gcount());
  }
  return true;
}

void ByteArray::addCapacity(size_t size) {
  if (size == 0) {
    return;
  }
  size_t old_cap = getCapacity();
  if (old_cap >= size) {
    return;
  }

  size = size - old_cap;

  size_t count = ceil(1.0 * size / m_baseSize);

  Node* tmp = m_root;
  while (tmp->next != nullptr) {
    tmp = tmp->next;
  }

  Node* first = nullptr;
  for (size_t i = 0; i < count; ++i) {
    tmp->next = new Node(m_baseSize);
    if (first == nullptr) {
      first = tmp->next;
    }

    tmp = tmp->next;
    m_capacity += m_baseSize;
  }
  if (old_cap == 0) {
    m_cur = first;
  }
}

auto ByteArray::toString() const -> std::string {
  std::string str;
  str.resize(getReadSize());
  if (str.empty()) {
    return str;
  }

  read(&str[0], str.size(), m_position);
  return str;
}
auto ByteArray::toHexString() const -> std::string {
  std::string str = toString();
  std::stringstream ss;

  for (size_t i = 0; i < str.size(); ++i) {
    if (i > 0 && i % 32 == 0) {
      ss << std::endl;
    }
    ss << std::setw(2) << std::setfill('0') << std::hex
       << static_cast<int>(str[i]) << " ";
  }

  return ss.str();
}
auto ByteArray::getReadBuffers(std::vector<iovec>& buffers, uint64_t len,
                               uint64_t position) const -> uint64_t {
  len = len > getReadSize() ? getReadSize() : len;
  if (len == 0) {
    return 0;
  }

  uint64_t size = len;

  size_t npos = position % m_baseSize;
  size_t count = position / m_baseSize;
  Node* cur = m_root;
  while (count > 0) {
    cur = cur->next;
    --count;
  }

  size_t ncap = cur->size - npos;
  struct iovec iov;
  while (len > 0) {
    if (ncap >= len) {
      iov.iov_base = cur->ptr + npos;
      iov.iov_len = len;
      len = 0;
    } else {
      iov.iov_base = cur->ptr + npos;
      iov.iov_len = ncap;
      len -= ncap;
      cur = cur->next;
      ncap = cur->size;
      npos = 0;
    }
    buffers.push_back(iov);
  }
  return size;
}
auto ByteArray::getReadBuffers(std::vector<iovec>& buffers, uint64_t len) const
    -> uint64_t {
  len = len > getReadSize() ? getReadSize() : len;
  if (len == 0) {
    return 0;
  }

  uint64_t size = len;

  size_t npos = m_position % m_baseSize;
  size_t ncap = m_cur->size - npos;
  struct iovec iov;
  Node* cur = m_cur;

  while (len > 0) {
    if (ncap >= len) {
      iov.iov_base = cur->ptr + npos;
      iov.iov_len = len;
      len = 0;
    } else {
      iov.iov_base = cur->ptr + npos;
      iov.iov_len = ncap;
      len -= ncap;
      cur = cur->next;
      ncap = cur->size;
      npos = 0;
    }
    buffers.push_back(iov);
  }
  return size;
}

auto ByteArray::getWriteBuffers(std::vector<iovec>& buffers, uint64_t len)
    -> uint64_t {
  if (len == 0) {
    return 0;
  }
  addCapacity(len);
  uint64_t size = len;

  size_t npos = m_position % m_baseSize;
  size_t ncap = m_cur->size - npos;
  struct iovec iov;
  Node* cur = m_cur;
  while (len > 0) {
    if (ncap >= len) {
      iov.iov_base = cur->ptr + npos;
      iov.iov_len = len;
      len = 0;
    } else {
      iov.iov_base = cur->ptr + npos;
      iov.iov_len = ncap;

      len -= ncap;
      cur = cur->next;
      ncap = cur->size;
      npos = 0;
    }
    buffers.push_back(iov);
  }
  return size;
}
}  // namespace hx_sylar