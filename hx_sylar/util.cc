#include "util.h"

#include <unistd.h>

namespace hx_sylar {
pid_t GetThreadId() { return syscall(SYS_gettid); }
uint32_t GetFiberId() { return 0; }
}  // namespace hx_sylar