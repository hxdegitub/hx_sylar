//
// Created by hx on 23-11-28.
//

#ifndef HX_SYLAR_ENDIAN_H
#define HX_SYLAR_ENDIAN_H


#include <byteswap.h>
#include <cstdint>
#include <type_traits>

namespace hx_sylar {

enum class ByteOrder {
    LittleEndian,
    BigEndian
};

template <class T>
constexpr ByteOrder getByteOrder() {
    return ByteOrder::LittleEndian;  // 根据实际情况实现
}

/**
 * @brief 字节序转化
 */
template <class T>
typename std::enable_if<std::is_integral<T>::value, T>::type
byteswap(T value) {
    if constexpr (getByteOrder<T>() == ByteOrder::LittleEndian) {
        return (T)__builtin_bswap64((uint64_t)value);
    } else {
        return value;
    }
}

template <class T>
typename std::enable_if<std::is_integral<T>::value, T>::type
byteswapOnLittleEndian(T t) {
    return t;
}

template <class T>
typename std::enable_if<std::is_integral<T>::value, T>::type
byteswapOnBigEndian(T t) {
    return byteswap(t);
}

} // namespace hx_sylar

#endif //HX_SYLAR_ENDIAN_H
