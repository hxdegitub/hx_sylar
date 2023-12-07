#include "util.h"

#include <arpa/inet.h>
#include <dirent.h>
#include <execinfo.h>
#include <ifaddrs.h>
#include <signal.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
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
auto StringUtil::Format(const char* fmt, ...) -> std::string {
  va_list ap;
  va_start(ap, fmt);
  auto v = Formatv(fmt, ap);
  va_end(ap);
  return v;
}

auto StringUtil::Formatv(const char* fmt, va_list ap) -> std::string {
  char* buf = nullptr;
  auto len = vasprintf(&buf, fmt, ap);
  if (len == -1) {
    return "";
  }
  std::string ret(buf, len);
  free(buf);
  return ret;
}

void FSUtil::ListAllFile(std::vector<std::string>& files,
                         const std::string& path, const std::string& subfix) {
  if (access(path.c_str(), 0) != 0) {
    return;
  }
  DIR* dir = opendir(path.c_str());
  if (dir == nullptr) {
    return;
  }
  struct dirent* dp = nullptr;
  while ((dp = readdir(dir)) != nullptr) {
    if (dp->d_type == DT_DIR) {
      if (!strcmp(dp->d_name, ".") || !strcmp(dp->d_name, "..")) {
        continue;
      }
      ListAllFile(files, path + "/" + dp->d_name, subfix);
    } else if (dp->d_type == DT_REG) {
      std::string filename(dp->d_name);
      if (subfix.empty()) {
        files.push_back(path + "/" + filename);
      } else {
        if (filename.size() < subfix.size()) {
          continue;
        }
        if (filename.substr(filename.length() - subfix.size()) == subfix) {
          files.push_back(path + "/" + filename);
        }
      }
    }
  }
  closedir(dir);
}

static auto __lstat(const char* file, struct stat* st = nullptr) -> int {
  struct stat lst;
  int ret = lstat(file, &lst);
  if (st != nullptr) {
    *st = lst;
  }
  return ret;
}
static auto __mkdir(const char* dirname) -> int {
  if (access(dirname, F_OK) == 0) {
    return 0;
  }
  return mkdir(dirname, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
}
auto FSUtil::Mkdir(const std::string& dirname) -> bool {
  if (__lstat(dirname.c_str()) == 0) {
    return true;
  }
  char* path = strdup(dirname.c_str());
  char* ptr = strchr(path + 1, '/');
  do {
    for (; ptr != nullptr; *ptr = '/', ptr = strchr(ptr + 1, '/')) {
      *ptr = '\0';
      if (__mkdir(path) != 0) {
        break;
      }
    }
    if (ptr != nullptr) {
      break;
    }
    if (__mkdir(path) != 0) {
      break;
    }
    free(path);
    return true;
  } while (0);
  free(path);
  return false;
}

bool FSUtil::IsRunningPidfile(const std::string& pidfile) {
  if (__lstat(pidfile.c_str()) != 0) {
    return false;
  }
  std::ifstream ifs(pidfile);
  std::string line;
  if (!ifs || !std::getline(ifs, line)) {
    return false;
  }
  if (line.empty()) {
    return false;
  }
  pid_t pid = atoi(line.c_str());
  if (pid <= 1) {
    return false;
  }
  if (kill(pid, 0) != 0) {
    return false;
  }
  return true;
}

bool FSUtil::Unlink(const std::string& filename, bool exist) {
  if (!exist && __lstat(filename.c_str())) {
    return true;
  }
  return ::unlink(filename.c_str()) == 0;
}

bool FSUtil::Rm(const std::string& path) {
  struct stat st;
  if (lstat(path.c_str(), &st)) {
    return true;
  }
  if (!(st.st_mode & S_IFDIR)) {
    return Unlink(path);
  }

  DIR* dir = opendir(path.c_str());
  if (!dir) {
    return false;
  }

  bool ret = true;
  struct dirent* dp = nullptr;
  while ((dp = readdir(dir))) {
    if (!strcmp(dp->d_name, ".") || !strcmp(dp->d_name, "..")) {
      continue;
    }
    std::string dirname = path + "/" + dp->d_name;
    ret = Rm(dirname);
  }
  closedir(dir);
  if (::rmdir(path.c_str())) {
    ret = false;
  }
  return ret;
}

bool FSUtil::Mv(const std::string& from, const std::string& to) {
  if (!Rm(to)) {
    return false;
  }
  return rename(from.c_str(), to.c_str()) == 0;
}

auto FSUtil::Realpath(const std::string& path, std::string& rpath) -> bool {
  if (__lstat(path.c_str())) {
    return false;
  }
  char* ptr = ::realpath(path.c_str(), nullptr);
  if (nullptr == ptr) {
    return false;
  }
  std::string(ptr).swap(rpath);
  free(ptr);
  return true;
}

auto FSUtil::Symlink(const std::string& from, const std::string& to) -> bool {
  if (!Rm(to)) {
    return false;
  }
  return ::symlink(from.c_str(), to.c_str()) == 0;
}

auto FSUtil::Dirname(const std::string& filename) -> std::string {
  if (filename.empty()) {
    return ".";
  }
  auto pos = filename.rfind('/');
  if (pos == 0) {
    return "/";
  }
  if (pos == std::string::npos) {
    return ".";
  }
  return filename.substr(0, pos);
}

auto FSUtil::Basename(const std::string& filename) -> std::string {
  if (filename.empty()) {
    return filename;
  }
  auto pos = filename.rfind('/');
  if (pos == std::string::npos) {
    return filename;
  } else {
    return filename.substr(pos + 1);
  }
}

auto FSUtil::OpenForRead(std::ifstream& ifs, const std::string& filename,
                         std::ios_base::openmode mode) -> bool {
  ifs.open(filename.c_str(), mode);
  return ifs.is_open();
}

auto FSUtil::OpenForWrite(std::ofstream& ofs, const std::string& filename,
                          std::ios_base::openmode mode) -> bool {
  ofs.open(filename.c_str(), mode);
  if (!ofs.is_open()) {
    std::string dir = Dirname(filename);
    Mkdir(dir);
    ofs.open(filename.c_str(), mode);
  }
  return ofs.is_open();
}
static const char uri_chars[256] = {
    /* 0 */
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    1,
    1,
    0,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    0,
    0,
    0,
    1,
    0,
    0,
    /* 64 */
    0,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    0,
    0,
    0,
    0,
    1,
    0,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    0,
    0,
    0,
    1,
    0,
    /* 128 */
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    /* 192 */
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
};

static const char XDIGIT_CHARS[256] = {
    0,  0,  0,  0, 0, 0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0, 0, 0,  0,  0,
    0,  0,  0,  0, 0, 0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0, 0, 0,  0,  0,
    0,  0,  0,  0, 0, 0,  0,  0,  0,  1,  2,  3, 4, 5, 6, 7, 8, 9,  0,  0,
    0,  0,  0,  0, 0, 10, 11, 12, 13, 14, 15, 0, 0, 0, 0, 0, 0, 0,  0,  0,
    0,  0,  0,  0, 0, 0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0, 0, 10, 11, 12,
    13, 14, 15, 0, 0, 0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0, 0, 0,  0,  0,
    0,  0,  0,  0, 0, 0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0, 0, 0,  0,  0,
    0,  0,  0,  0, 0, 0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0, 0, 0,  0,  0,
    0,  0,  0,  0, 0, 0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0, 0, 0,  0,  0,
    0,  0,  0,  0, 0, 0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0, 0, 0,  0,  0,
    0,  0,  0,  0, 0, 0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0, 0, 0,  0,  0,
    0,  0,  0,  0, 0, 0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0, 0, 0,  0,  0,
    0,  0,  0,  0, 0, 0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0,
};

#define CHAR_IS_UNRESERVED(c) (uri_chars[(unsigned char)(c)])

auto StringUtil::UrlEncode(const std::string& str, bool space_as_plus)
    -> std::string {
  static const char* hexdigits = "0123456789ABCDEF";
  std::string* ss = nullptr;
  const char* end = str.c_str() + str.length();
  for (const char* c = str.c_str(); c < end; ++c) {
    if (!CHAR_IS_UNRESERVED(*c)) {
      if (ss == nullptr) {
        ss = new std::string;
        ss->reserve(str.size() * 1.2);
        ss->append(str.c_str(), c - str.c_str());
      }
      if (*c == ' ' && space_as_plus) {
        ss->append(1, '+');
      } else {
        ss->append(1, '%');
        ss->append(1, hexdigits[static_cast<uint8_t>(*c) >> 4]);
        ss->append(1, hexdigits[*c & 0xf]);
      }
    } else if (ss != nullptr) {
      ss->append(1, *c);
    }
  }
  if (ss == nullptr) {
    return str;
  }
  std::string rt = *ss;
  delete ss;
  return rt;
}

auto StringUtil::UrlDecode(const std::string& str, bool space_as_plus)
    -> std::string {
  std::string* ss = nullptr;
  const char* end = str.c_str() + str.length();
  for (const char* c = str.c_str(); c < end; ++c) {
    if (*c == '+' && space_as_plus) {
      if (ss == nullptr) {
        ss = new std::string;
        ss->append(str.c_str(), c - str.c_str());
      }
      ss->append(1, ' ');
    } else if (*c == '%' && (c + 2) < end && (isxdigit(*(c + 1)) != 0) &&
               (isxdigit(*(c + 2)) != 0)) {
      if (ss == nullptr) {
        ss = new std::string;
        ss->append(str.c_str(), c - str.c_str());
      }
      ss->append(
          1, static_cast<char>(XDIGIT_CHARS[static_cast<int>(*(c + 1))] << 4 |
                               XDIGIT_CHARS[static_cast<int>(*(c + 2))]));
      c += 2;
    } else if (ss != nullptr) {
      ss->append(1, *c);
    }
  }
  if (ss == nullptr) {
    return str;
  }
  std::string rt = *ss;
  delete ss;
  return rt;
}

auto StringUtil::Trim(const std::string& str, const std::string& delimit)
    -> std::string {
  auto begin = str.find_first_not_of(delimit);
  if (begin == std::string::npos) {
    return "";
  }
  auto end = str.find_last_not_of(delimit);
  return str.substr(begin, end - begin + 1);
}

auto StringUtil::TrimLeft(const std::string& str, const std::string& delimit)
    -> std::string {
  auto begin = str.find_first_not_of(delimit);
  if (begin == std::string::npos) {
    return "";
  }
  return str.substr(begin);
}

auto StringUtil::TrimRight(const std::string& str, const std::string& delimit)
    -> std::string {
  auto end = str.find_last_not_of(delimit);
  if (end == std::string::npos) {
    return "";
  }
  return str.substr(0, end);
}

auto StringUtil::WStringToString(const std::wstring& ws) -> std::string {
  std::string str_locale = setlocale(LC_ALL, "");
  const wchar_t* wch_src = ws.c_str();
  size_t n_dest_size = wcstombs(nullptr, wch_src, 0) + 1;
  char* ch_dest = new char[n_dest_size];
  memset(ch_dest, 0, n_dest_size);
  wcstombs(ch_dest, wch_src, n_dest_size);
  std::string str_result = ch_dest;
  delete[] ch_dest;
  setlocale(LC_ALL, str_locale.c_str());
  return str_result;
}

auto StringUtil::StringToWString(const std::string& s) -> std::wstring {
  std::string str_locale = setlocale(LC_ALL, "");
  const char* ch_src = s.c_str();
  size_t n_dest_size = mbstowcs(nullptr, ch_src, 0) + 1;
  auto* wch_dest = new wchar_t[n_dest_size];
  wmemset(wch_dest, 0, n_dest_size);
  mbstowcs(wch_dest, ch_src, n_dest_size);
  std::wstring wstr_result = wch_dest;
  delete[] wch_dest;
  setlocale(LC_ALL, str_locale.c_str());
  return wstr_result;
}

auto Time2Str(time_t ts, const std::string& format) -> std::string {
  struct tm tm;
  localtime_r(&ts, &tm);
  char buf[64];
  strftime(buf, sizeof(buf), format.c_str(), &tm);
  return buf;
}

auto Str2Time(const char* str, const char* format) -> time_t {
  struct tm t;
  memset(&t, 0, sizeof(t));
  if (strptime(str, format, &t) == nullptr) {
    return 0;
  }
  return mktime(&t);
}

}  // namespace hx_sylar
