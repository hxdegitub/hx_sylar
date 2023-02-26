#pragma once
#include <pthread.h>
#include <stdint.h>
#include <sys/syscall.h>
#include <sys/types.h>

#include <string>
#include <vector>
namespace hx_sylar {
pid_t GetThreadId();
uint32_t GetFiberId();
void Backtrace(std::vector<std::string>& bt, int size, int skip = 1);
std::string BacktraceToString(int size, int skip = 2,
                              const std::string& prefix = "");
}  // namespace hx_sylar