#ifndef __HX_UTIL_H__
#define __HX_UTIL_H__

#include <cxxabi.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>

#include <cstdint>
#include <iomanip>
#include <string>
#include <vector>
namespace hx_sylar {
auto GetThreadId() -> pid_t;
uint32_t GetFiberId();
void Backtrace(std::vector<std::string>& bt, int size = 64, int skip = 1);
std::string BacktraceToString(int size = 64, int skip = 2,
                              const std::string& prefix = "");

auto GetCurrentMS() -> uint64_t;
auto GetElapsedMS() -> uint64_t;
auto GetCurrentUS() -> uint64_t;
}  // namespace hx_sylar
#endif