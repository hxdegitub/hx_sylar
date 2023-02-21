#pragma once
#include <pthread.h>
#include <stdint.h>
#include <sys/syscall.h>
#include <sys/types.h>
namespace hx_sylar {
pid_t GetThreadId();
uint32_t GetFiberId();
}  // namespace hx_sylar