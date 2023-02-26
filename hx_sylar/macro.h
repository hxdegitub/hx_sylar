#ifndef _HX_MACRO_H_
#define _HX_MACRO_H_
#include <assert.h>
#include <string.h>

#include <boost/contract_macro.hpp>

#include "util.h"
#define HX_ASSERT(x)                                    \
  if (!(x)) {                                           \
    HX_LOG_ERROR(HX_LOG_ROOT())                         \
        << " ASSERTION:  " #x << "\nbacktrace : \n"     \
        << hx_sylar::BacktraceToString(100, 2, "    "); \
    assert(x);                                          \
  }

#define HX_ASSERT1(x, w)                                                   \
  if (!(x)) {                                                              \
    HX_LOG_ERROR(HX_LOG_ROOT()) << " ASSERTION:  " #x << "\n"              \
                                << w                                       \
        "\nbacktrace : \n" << hx_sylar::BacktraceToString(100, 2, "    "); \
    assert(x);                                                             \
  }
#endif
