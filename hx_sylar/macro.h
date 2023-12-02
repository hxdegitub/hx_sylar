#ifndef _HX_MACRO_H_
#define _HX_MACRO_H_
#include <assert.h>
#include <string.h>

#include "util.h"

#if defined __GNUC__ || defined __llvm__
/// LIKCLY 宏的封装, 告诉编译器优化,条件大概率成立
#define SYLAR_LIKELY(x) __builtin_expect(!!(x), 1)
/// LIKCLY 宏的封装, 告诉编译器优化,条件大概率不成立
#define SYLAR_UNLIKELY(x) __builtin_expect(!!(x), 0)
#else
#define SYLAR_LIKELY(x) (x)
#define SYLAR_UNLIKELY(x) (x)
#endif
#define HX_ASSERT(x)                                    \
  if (!(x)) {                                           \
    HX_LOG_ERROR(HX_LOG_ROOT())                         \
        << " ASSERTION:  " #x << "\nbacktrace : \n"     \
        << hx_sylar::BacktraceToString(100, 2, "    "); \
    assert(x);                                          \
  }

#define HX_ASSERT1(x, w)                                \
  if (!(x)) {                                           \
    HX_LOG_ERROR(HX_LOG_ROOT())                         \
        << " ASSERTION:  " #x << '\n'                   \
        << w << "\nbacktrace : \n"                      \
        << hx_sylar::BacktraceToString(100, 2, "    "); \
    assert(x);                                          \
  }
#endif
