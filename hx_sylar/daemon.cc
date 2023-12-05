#include "daemon.h"

#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include "config.h"
#include "log.h"
#include "util.h"

namespace hx_sylar {
static hx_sylar::Logger::ptr g_logger = HX_LOG_NAME("system");
static hx_sylar::ConfigVar<uint32_t>::ptr g_daemon_restart_interval =
    hx_sylar::Config::Lookup("daemon.g_daemon_restart_interval", (uint32_t)5,
                             "daemon restart interval");

auto ProcessInfo::toString() const -> std::string {
  std::stringstream ss;
  ss << "[ProcessInfo parent_id= " << parent_id << " main_id=" << main_id
     << " parent_start_time= " << hx_sylar::Time2Str(parent_start_time)
     << " main_start_time=" << hx_sylar::Time2Str(main_start_time)
     << " restart_count=" << g_daemon_restart_interval << "]";
  return ss.str();
}

static auto real_start(int argc, char** argv,
                       std::function<int(int argc, char** argv)> main_cb)
    -> int {
  ProcessInfoMgr::GetInstance()->main_id = getpid();
  ProcessInfoMgr::GetInstance()->main_start_time = time(0);
  return main_cb(argc, argv);
}

static auto real_daemon(int argc, char** argv,
                        std::function<int(int argc, char** argv)> main_cb)
    -> int {
  daemon(1, 0);
  ProcessInfoMgr::GetInstance()->parent_id = getpid();
  ProcessInfoMgr::GetInstance()->parent_start_time = time(nullptr);

  while (true) {
    pid_t pid = fork();

    if (pid == 0) {
      ProcessInfoMgr::GetInstance()->main_id = getpid();
      ProcessInfoMgr::GetInstance()->main_start_time = time(0);
      HX_LOG_INFO(g_logger) << "process start pid = " << getpid();
      return real_start(argc, argv, main_cb);
    } else if (pid < 0) {
      HX_LOG_ERROR(g_logger)
          << "fork fial return : " << pid << "errno =" << errno
          << " errstr= " << strerror(errno);
      return -1;
    } else {
      int status = 0;
      waitpid(pid, &status, 0);
      if (status != 0) {
        if (status == 9) {
          HX_LOG_INFO(g_logger) << "killed";
          break;
        }
        HX_LOG_ERROR(g_logger)
            << " child crash pid=" << pid << "status =" << status;

      } else {
        HX_LOG_INFO(g_logger) << " child finished pid = " << pid;
        break;
      }
      ProcessInfoMgr::GetInstance()->restart_count += 1;
      sleep(g_daemon_restart_interval->getValue());
    }
  }
  return 0;
}
auto start_daemon(int argc, char** argv,
                  std::function<int(int argc, char** argv)> main_cb,
                  bool is_daemon) -> int {
  if (!is_daemon) {
    ProcessInfoMgr::GetInstance()->parent_id = getpid();
    ProcessInfoMgr::GetInstance()->parent_start_time = time(0);
    return real_start(argc, argv, main_cb);
  }
  return real_daemon(argc, argv, main_cb);
}

}  // namespace hx_sylar
