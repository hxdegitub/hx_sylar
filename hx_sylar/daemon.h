#ifndef __HX_SYLAR_DAEMON_H__
#define __HX_SYLAR_DAEMON_H__
#include <unistd.h>

#include <functional>

#include "singleton.h"
namespace hx_sylar {
struct ProcessInfo {
  pid_t parent_id = 0;
  pid_t main_id = 0;
  uint64_t parent_start_time = 0;
  uint64_t main_start_time = 0;
  uint64_t restart_count = 0;
  std::string toString() const;
};

using ProcessInfoMgr = hx_sylar::Singleton<ProcessInfo>;

int start_daemon(int argc, char** argv,
                 std::function<int(int argc, char** argv)> main_cb,
                 bool is_daemon);
}  // namespace hx_sylar
#endif
