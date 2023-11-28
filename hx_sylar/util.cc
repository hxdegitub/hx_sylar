#include "util.h"

#include <execinfo.h>
#include <unistd.h>

#include "fiber.h"
#include "log.h"

namespace hx_sylar {
	hx_sylar::Logger::ptr g_logger = HX_LOG_NAME("system");
auto GetThreadId() -> pid_t { return static_cast<pid_t>(syscall(SYS_gettid)); }
auto GetFiberId() -> uint32_t { return hx_sylar::Fiber::GetFiberId(); }
void Backtrace(std::vector<std::string>& bt, int size, int skip) {
  void** array = static_cast<void**>(malloc(sizeof(void*) * size));
  size_t s = ::backtrace(array, size);
  char** strings = backtrace_symbols(array, static_cast<int>(s));
  if (strings == nullptr) {
    HX_LOG_ERROR(g_logger) << "backtrace_symbols error ";
    return;
  }
  for (size_t i = skip; i < s; ++i) {
    bt.emplace_back(strings[i]);
  }
  free(strings);
  free(array);
}
auto BacktraceToString(int size, int skip, const std::string& prefix)
    -> std::string {
  std::vector<std::string> bt;
  Backtrace(bt, size, skip);
  std::stringstream ss;
  for (const auto& i : bt) {
    ss << prefix << i << '\n';
  }
  return ss.str();
}

auto GetElapsedMS() -> uint64_t {
  struct timespec ts = {0};
  clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
  return ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}

auto GetCurrentMS() -> uint64_t {
  struct timeval tv {};
  gettimeofday(&tv, nullptr);
  return tv.tv_sec * 1000UL + tv.tv_usec / 1000;
}
auto GetCurrentUS() -> uint64_t {
  struct timeval tv {};
  gettimeofday(&tv, nullptr);
  return tv.tv_sec * 1000 * 1000UL + tv.tv_usec;
}
}  // namespace hx_sylar