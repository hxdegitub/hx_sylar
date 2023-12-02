#include "hook.h"

#include <dlfcn.h>

#include <memory>

#include "config.h"
#include "fd_manager.h"
#include "fiber.h"
#include "iomanager.h"
#include "log.h"
#include "macro.h"

hx_sylar::Logger::ptr g_logger = HX_LOG_NAME("system");
namespace hx_sylar {

static hx_sylar::ConfigVar<int>::ptr g_tcp_connect_timeout =
    hx_sylar::Config::Lookup("tcp.connect.timeout", 5000,
                             "tcp connect timeout");

static thread_local bool t_hook_enable = false;

#define HOOK_FUN(XX) \
  XX(sleep)          \
  XX(usleep)         \
  XX(nanosleep)      \
  XX(socket)         \
  XX(connect)        \
  XX(accept)         \
  XX(read)           \
  XX(readv)          \
  XX(recv)           \
  XX(recvfrom)       \
  XX(recvmsg)        \
  XX(write)          \
  XX(writev)         \
  XX(send)           \
  XX(sendto)         \
  XX(sendmsg)        \
  XX(close)          \
  XX(fcntl)          \
  XX(ioctl)          \
  XX(getsockopt)     \
  XX(setsockopt)

void hook_init() {
  static bool is_inited = false;
  if (is_inited) {
    return;
  }
#define XX(name) name##_f = (name##_fun)dlsym(RTLD_NEXT, #name);
  HOOK_FUN(XX);
#undef XX
}

static uint64_t s_connect_timeout = -1;
struct _HookIniter {
  _HookIniter() {
    hook_init();
    s_connect_timeout = g_tcp_connect_timeout->getValue();

    g_tcp_connect_timeout->addListener(
        [](const int &old_value, const int &new_value) {
          HX_LOG_INFO(g_logger) << "tcp connect timeout changed from "
                                << old_value << " to " << new_value;
          s_connect_timeout = new_value;
        });
  }
};

static _HookIniter s_hook_initer;

bool is_hook_enable() { return t_hook_enable; }

void set_hook_enable(bool flag) { t_hook_enable = flag; }

}  // namespace hx_sylar

struct timer_info {
  int cancelled = 0;
};

template <typename OriginFun, typename... Args>
static auto do_io(int fd, OriginFun fun, const char *hook_fun_name,
                  uint32_t event, int timeout_so, Args &&...args) -> ssize_t {
  if (!hx_sylar::t_hook_enable) {
    return fun(fd, std::forward<Args>(args)...);
  }

  hx_sylar::FdCtx::ptr ctx = hx_sylar::FdMgr::GetInstance()->get(fd);
  if (!ctx) {
    return fun(fd, std::forward<Args>(args)...);
  }

  if (ctx->isClose()) {
    errno = EBADF;
    return -1;
  }

  if (!ctx->isSocket() || ctx->getUserNonblock()) {
    return fun(fd, std::forward<Args>(args)...);
  }

  uint64_t to = ctx->getTimeout(timeout_so);
  std::shared_ptr<timer_info> stinfo = std::make_shared<timer_info>();

retry:
  ssize_t n = fun(fd, std::forward<Args>(args)...);
  while (n == -1 && errno == EINTR) {
    n = fun(fd, std::forward<Args>(args)...);
  }
  if (n == -1 && errno == EAGAIN) {
    hx_sylar::IOManager *iom = hx_sylar::IOManager::GetThis();
    hx_sylar::Timer::ptr timer;
    std::weak_ptr<timer_info> winfo(stinfo);

    if (to != (uint64_t)-1) {
      timer = iom->addConditionTimer(
          to,
          [winfo, fd, iom, event]() {
            auto t = winfo.lock();
            if (!t || t->cancelled) {
              return;
            }
            t->cancelled = ETIMEDOUT;
            iom->cancelEvent(fd, (hx_sylar::IOManager::Event)(event));
          },
          winfo);
    }

    int rt = iom->addEvent(fd, (hx_sylar::IOManager::Event)(event));
    if (SYLAR_UNLIKELY(rt)) {
      HX_LOG_ERROR(g_logger)
          << hook_fun_name << " addEvent(" << fd << ", " << event << ")";
      if (timer) {
        timer->cancel();
      }
      return -1;
    } else {
      hx_sylar::Fiber::YieldToHold();
      if (timer) {
        timer->cancel();
      }
      if (stinfo->cancelled) {
        errno = stinfo->cancelled;
        return -1;
      }
      goto retry;
    }
  }

  return n;
}

extern "C" {
#define XX(name) name##_fun name##_f = nullptr;
HOOK_FUN(XX);
#undef XX

auto sleep(unsigned int seconds) -> unsigned int {
  if (!hx_sylar::t_hook_enable) {
    return sleep_f(seconds);
  }

  hx_sylar::Fiber::ptr fiber = hx_sylar::Fiber::GetThis();
  hx_sylar::IOManager *iom = hx_sylar::IOManager::GetThis();
  iom->addTimer(seconds * 1000,
                std::bind((void(hx_sylar::Scheduler::*)(hx_sylar::Fiber::ptr,
                                                        int thread)) &
                              hx_sylar::IOManager::schedule,
                          iom, fiber, -1));
  hx_sylar::Fiber::YieldToHold();
  return 0;
}

auto usleep(useconds_t usec) -> int {
  if (!hx_sylar::t_hook_enable) {
    return usleep_f(usec);
  }
  hx_sylar::Fiber::ptr fiber = hx_sylar::Fiber::GetThis();
  hx_sylar::IOManager *iom = hx_sylar::IOManager::GetThis();
  iom->addTimer(usec / 1000, std::bind((void(hx_sylar::Scheduler::*)(
                                           hx_sylar::Fiber::ptr, int thread)) &
                                           hx_sylar::IOManager::schedule,
                                       iom, fiber, -1));
  hx_sylar::Fiber::YieldToHold();
  return 0;
}

auto nanosleep(const struct timespec *req, struct timespec *rem) -> int {
  if (!hx_sylar::t_hook_enable) {
    return nanosleep_f(req, rem);
  }

  int timeout_ms = req->tv_sec * 1000 + req->tv_nsec / 1000 / 1000;
  hx_sylar::Fiber::ptr fiber = hx_sylar::Fiber::GetThis();
  hx_sylar::IOManager *iom = hx_sylar::IOManager::GetThis();
  iom->addTimer(timeout_ms, std::bind((void(hx_sylar::Scheduler::*)(
                                          hx_sylar::Fiber::ptr, int thread)) &
                                          hx_sylar::IOManager::schedule,
                                      iom, fiber, -1));
  hx_sylar::Fiber::YieldToHold();
  return 0;
}

auto socket(int domain, int type, int protocol) -> int {
  if (!hx_sylar::t_hook_enable) {
    return socket_f(domain, type, protocol);
  }
  int fd = socket_f(domain, type, protocol);
  if (fd == -1) {
    return fd;
  }
  hx_sylar::FdMgr::GetInstance()->get(fd, true);
  return fd;
}

auto connect_with_timeout(int fd, const struct sockaddr *addr,
                          socklen_t addrlen, uint64_t timeout_ms) -> int {
  if (!hx_sylar::t_hook_enable) {
    return connect_f(fd, addr, addrlen);
  }
  hx_sylar::FdCtx::ptr ctx = hx_sylar::FdMgr::GetInstance()->get(fd);
  if (!ctx || ctx->isClose()) {
    errno = EBADF;
    return -1;
  }

  if (!ctx->isSocket()) {
    return connect_f(fd, addr, addrlen);
  }

  if (ctx->getUserNonblock()) {
    return connect_f(fd, addr, addrlen);
  }

  int n = connect_f(fd, addr, addrlen);
  if (n == 0) {
    return 0;
  }
  if (n != -1 || errno != EINPROGRESS) {
    return n;
  }

  hx_sylar::IOManager *iom = hx_sylar::IOManager::GetThis();
  hx_sylar::Timer::ptr timer;
  std::shared_ptr<timer_info> tinfo = std::make_shared<timer_info>();
  std::weak_ptr<timer_info> winfo(tinfo);

  if (timeout_ms != (uint64_t)-1) {
    timer = iom->addConditionTimer(
        timeout_ms,
        [winfo, fd, iom]() {
          auto t = winfo.lock();
          if (!t || t->cancelled) {
            return;
          }
          t->cancelled = ETIMEDOUT;
          iom->cancelEvent(fd, hx_sylar::IOManager::WRITE);
        },
        winfo);
  }

  int rt = iom->addEvent(fd, hx_sylar::IOManager::WRITE);
  if (rt == 0) {
    hx_sylar::Fiber::YieldToHold();
    if (timer) {
      timer->cancel();
    }
    if (tinfo->cancelled) {
      errno = tinfo->cancelled;
      return -1;
    }
  } else {
    if (timer) {
      timer->cancel();
    }
    HX_LOG_ERROR(g_logger) << "connect addEvent(" << fd << ", WRITE) error";
  }

  int error = 0;
  socklen_t len = sizeof(int);
  if (-1 == getsockopt(fd, SOL_SOCKET, SO_ERROR, &error, &len)) {
    return -1;
  }
  if (!error) {
    return 0;
  } else {
    errno = error;
    return -1;
  }
}

int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
  return connect_with_timeout(sockfd, addr, addrlen,
                              hx_sylar::s_connect_timeout);
}

int accept(int s, struct sockaddr *addr, socklen_t *addrlen) {
  int fd = do_io(s, accept_f, "accept", hx_sylar::IOManager::READ, SO_RCVTIMEO,
                 addr, addrlen);
  if (fd >= 0) {
    hx_sylar::FdMgr::GetInstance()->get(fd, true);
  }
  return fd;
}

auto read(int fd, void *buf, size_t count) -> ssize_t {
  return do_io(fd, read_f, "read", hx_sylar::IOManager::READ, SO_RCVTIMEO, buf,
               count);
}

auto readv(int fd, const struct iovec *iov, int iovcnt) -> ssize_t {
  return do_io(fd, readv_f, "readv", hx_sylar::IOManager::READ, SO_RCVTIMEO,
               iov, iovcnt);
}

auto recv(int sockfd, void *buf, size_t len, int flags) -> ssize_t {
  return do_io(sockfd, recv_f, "recv", hx_sylar::IOManager::READ, SO_RCVTIMEO,
               buf, len, flags);
}

auto recvfrom(int sockfd, void *buf, size_t len, int flags,
              struct sockaddr *src_addr, socklen_t *addrlen) -> ssize_t {
  return do_io(sockfd, recvfrom_f, "recvfrom", hx_sylar::IOManager::READ,
               SO_RCVTIMEO, buf, len, flags, src_addr, addrlen);
}

auto recvmsg(int sockfd, struct msghdr *msg, int flags) -> ssize_t {
  return do_io(sockfd, recvmsg_f, "recvmsg", hx_sylar::IOManager::READ,
               SO_RCVTIMEO, msg, flags);
}

auto write(int fd, const void *buf, size_t count) -> ssize_t {
  return do_io(fd, write_f, "write", hx_sylar::IOManager::WRITE, SO_SNDTIMEO,
               buf, count);
}

auto writev(int fd, const struct iovec *iov, int iovcnt) -> ssize_t {
  return do_io(fd, writev_f, "writev", hx_sylar::IOManager::WRITE, SO_SNDTIMEO,
               iov, iovcnt);
}

auto send(int s, const void *msg, size_t len, int flags) -> ssize_t {
  return do_io(s, send_f, "send", hx_sylar::IOManager::WRITE, SO_SNDTIMEO, msg,
               len, flags);
}

auto sendto(int s, const void *msg, size_t len, int flags,
            const struct sockaddr *to, socklen_t tolen) -> ssize_t {
  return do_io(s, sendto_f, "sendto", hx_sylar::IOManager::WRITE, SO_SNDTIMEO,
               msg, len, flags, to, tolen);
}

auto sendmsg(int s, const struct msghdr *msg, int flags) -> ssize_t {
  return do_io(s, sendmsg_f, "sendmsg", hx_sylar::IOManager::WRITE, SO_SNDTIMEO,
               msg, flags);
}

auto close(int fd) -> int {
  if (!hx_sylar::t_hook_enable) {
    return close_f(fd);
  }

  hx_sylar::FdCtx::ptr ctx = hx_sylar::FdMgr::GetInstance()->get(fd);
  if (ctx) {
    auto iom = hx_sylar::IOManager::GetThis();
    if (iom) {
      iom->cancelAll(fd);
    }
    hx_sylar::FdMgr::GetInstance()->del(fd);
  }
  return close_f(fd);
}

auto fcntl(int fd, int cmd, ... /* arg */) -> int {
  va_list va;
  va_start(va, cmd);
  switch (cmd) {
    case F_SETFL: {
      int arg = va_arg(va, int);
      va_end(va);
      hx_sylar::FdCtx::ptr ctx = hx_sylar::FdMgr::GetInstance()->get(fd);
      if (!ctx || ctx->isClose() || !ctx->isSocket()) {
        return fcntl_f(fd, cmd, arg);
      }
      ctx->setUserNonblock(arg & O_NONBLOCK);
      if (ctx->getSysNonblock()) {
        arg |= O_NONBLOCK;
      } else {
        arg &= ~O_NONBLOCK;
      }
      return fcntl_f(fd, cmd, arg);
    } break;
    case F_GETFL: {
      va_end(va);
      int arg = fcntl_f(fd, cmd);
      hx_sylar::FdCtx::ptr ctx = hx_sylar::FdMgr::GetInstance()->get(fd);
      if (!ctx || ctx->isClose() || !ctx->isSocket()) {
        return arg;
      }
      if (ctx->getUserNonblock()) {
        return arg | O_NONBLOCK;
      } else {
        return arg & ~O_NONBLOCK;
      }
    } break;
    case F_DUPFD:
    case F_DUPFD_CLOEXEC:
    case F_SETFD:
    case F_SETOWN:
    case F_SETSIG:
    case F_SETLEASE:
    case F_NOTIFY:
#ifdef F_SETPIPE_SZ
    case F_SETPIPE_SZ:
#endif
    {
      int arg = va_arg(va, int);
      va_end(va);
      return fcntl_f(fd, cmd, arg);
    } break;
    case F_GETFD:
    case F_GETOWN:
    case F_GETSIG:
    case F_GETLEASE:
#ifdef F_GETPIPE_SZ
    case F_GETPIPE_SZ:
#endif
    {
      va_end(va);
      return fcntl_f(fd, cmd);
    } break;
    case F_SETLK:
    case F_SETLKW:
    case F_GETLK: {
      struct flock *arg = va_arg(va, struct flock *);
      va_end(va);
      return fcntl_f(fd, cmd, arg);
    } break;
    case F_GETOWN_EX:
    case F_SETOWN_EX: {
      struct f_owner_exlock *arg = va_arg(va, struct f_owner_exlock *);
      va_end(va);
      return fcntl_f(fd, cmd, arg);
    } break;
    default:
      va_end(va);
      return fcntl_f(fd, cmd);
  }
}

auto ioctl(int d, unsigned long int request, ...) -> int {
  va_list va;
  va_start(va, request);
  void *arg = va_arg(va, void *);
  va_end(va);

  if (FIONBIO == request) {
    bool user_nonblock = !!*(int *)arg;
    hx_sylar::FdCtx::ptr ctx = hx_sylar::FdMgr::GetInstance()->get(d);
    if (!ctx || ctx->isClose() || !ctx->isSocket()) {
      return ioctl_f(d, request, arg);
    }
    ctx->setUserNonblock(user_nonblock);
  }
  return ioctl_f(d, request, arg);
}

auto getsockopt(int sockfd, int level, int optname, void *optval,
                socklen_t *optlen) -> int {
  return getsockopt_f(sockfd, level, optname, optval, optlen);
}

auto setsockopt(int sockfd, int level, int optname, const void *optval,
                socklen_t optlen) -> int {
  if (!hx_sylar::t_hook_enable) {
    return setsockopt_f(sockfd, level, optname, optval, optlen);
  }
  if (level == SOL_SOCKET) {
    if (optname == SO_RCVTIMEO || optname == SO_SNDTIMEO) {
      hx_sylar::FdCtx::ptr ctx = hx_sylar::FdMgr::GetInstance()->get(sockfd);
      if (ctx) {
        const timeval *v = (const timeval *)optval;
        ctx->setTimeout(optname, v->tv_sec * 1000 + v->tv_usec / 1000);
      }
    }
  }
  return setsockopt_f(sockfd, level, optname, optval, optlen);
}
}
