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
class FSUtil {
 public:
  static void ListAllFile(std::vector<std::string>& files,
                          const std::string& path, const std::string& subfix);
  static bool Mkdir(const std::string& dirname);
  static bool IsRunningPidfile(const std::string& pidfile);
  static bool Rm(const std::string& path);
  static bool Mv(const std::string& from, const std::string& to);
  static bool Realpath(const std::string& path, std::string& rpath);
  static bool Symlink(const std::string& frm, const std::string& to);
  static bool Unlink(const std::string& filename, bool exist = false);
  static std::string Dirname(const std::string& filename);
  static std::string Basename(const std::string& filename);
  static bool OpenForRead(std::ifstream& ifs, const std::string& filename,
                          std::ios_base::openmode mode);
  static bool OpenForWrite(std::ofstream& ofs, const std::string& filename,
                           std::ios_base::openmode mode);
};
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
