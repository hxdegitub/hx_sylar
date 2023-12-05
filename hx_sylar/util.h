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
auto GetFiberId() -> uint32_t;
void Backtrace(std::vector<std::string>& bt, int size = 64, int skip = 1);
auto BacktraceToString(int size = 64, int skip = 2,
                       const std::string& prefix = "") -> std::string;

auto GetCurrentMS() -> uint64_t;
auto GetElapsedMS() -> uint64_t;
auto GetCurrentUS() -> uint64_t;

class StringUtil {
 public:
  static auto Format(const char* fmt, ...) -> std::string;
  static auto Formatv(const char* fmt, va_list ap) -> std::string;

  static auto UrlEncode(const std::string& str, bool space_as_plus = true)
      -> std::string;
  static auto UrlDecode(const std::string& str, bool space_as_plus = true)
      -> std::string;

  static auto Trim(const std::string& str,
                   const std::string& delimit = " \t\r\n") -> std::string;
  static auto TrimLeft(const std::string& str,
                       const std::string& delimit = " \t\r\n") -> std::string;
  static auto TrimRight(const std::string& str,
                        const std::string& delimit = " \t\r\n") -> std::string;

  static auto WStringToString(const std::wstring& ws) -> std::string;
  static auto StringToWString(const std::string& s) -> std::wstring;

  std::string ToUpper(const std::string& name);
  std::string ToLower(const std::string& name);
};
auto Time2Str(time_t ts = time(nullptr),
              const std::string& format = "%Y-%m-%d %H:%M:%S") -> std::string;

auto Str2Time(const char* str, const char* format = "%Y-%m-%d %H:%M:%S")
    -> time_t;
}  // namespace hx_sylar
#endif
