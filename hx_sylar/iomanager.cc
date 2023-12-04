#include "iomanager.h"

#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/epoll.h>
#include <unistd.h>

#include "log.h"
#include "macro.h"
namespace hx_sylar {

auto IOManager::FdContext::getContext(IOManager::Event event)
    -> IOManager::FdContext::EventContext& {
  switch (event) {
    case IOManager::READ:
      return read;
    case IOManager::WRITE:
      return write;
    default:
      HX_ASSERT1(false, "getcontext");
  }
}
void IOManager::FdContext::resetContext(EventContext& ctx) {
  ctx.scheduler = nullptr;
  ctx.fiber.reset();
  ctx.cb = nullptr;
}
void IOManager::FdContext::triggerEvent(Event event) {
  HX_ASSERT(events & event);
  events = static_cast<Event>(events & ~event);
  EventContext& ctx = getContext(event);
  if (ctx.cb) {
    ctx.scheduler->schedule(&ctx.cb);
  } else {
    ctx.scheduler->schedule(&ctx.fiber);
  }
  ctx.scheduler = nullptr;
}

static Logger::ptr g_logger = HX_LOG_NAME("root");
IOManager::IOManager(size_t threads, bool user_call, const std::string& name)
    : Scheduler(threads, user_call, name) {
  m_epfd = epoll_create(5000);
  HX_ASSERT(m_epfd > 0);
  int rt = pipe(m_tickleFds);
  HX_ASSERT(!rt);

  epoll_event event{};
  memset(&event, 0, sizeof(epoll_event));
  event.events = EPOLLIN | EPOLLET;
  event.data.fd = m_tickleFds[0];

  rt = fcntl(m_tickleFds[0], F_SETFL, O_NONBLOCK);
  HX_ASSERT(!rt);

  rt = epoll_ctl(m_epfd, EPOLL_CTL_ADD, m_tickleFds[0], &event);

  HX_ASSERT(!rt);

  m_fdContexts.resize(64);
  start();
}

IOManager::~IOManager() {
  stop();
  close(m_epfd);
  close(m_tickleFds[0]);
  close(m_tickleFds[1]);
  for (auto& m_fd_context : m_fdContexts) {
    delete m_fd_context;
  }
}

void IOManager::contextResize(size_t size) {
  m_fdContexts.resize(size);

  for (size_t i = 0; i < m_fdContexts.size(); ++i) {
    if (m_fdContexts[i] == nullptr) {
      m_fdContexts[i] = new FdContext;
      m_fdContexts[i]->fd = i;
    }
  }
}

auto IOManager::addEvent(int fd, Event event, std::function<void()> cb) -> int {
  FdContext* fd_ctx = nullptr;
  RWMutexType::ReadLock lock(m_mutex);
  if (static_cast<int>(m_fdContexts.size()) > fd) {
    fd_ctx = m_fdContexts[fd];
    lock.unlock();
  } else {
    lock.unlock();
    RWMutexType::WriteLock lock2(m_mutex);
    contextResize(static_cast<size_t>(fd * 1.5));
    fd_ctx = m_fdContexts[fd];
  }

  FdContext::MutexType::Lock lock2(fd_ctx->mutex);
  if ((fd_ctx->events & event) != 0) {
    HX_LOG_ERROR(g_logger) << "addEvent assert fd=" << fd
                           << " event=" << static_cast<EPOLL_EVENTS>(event)
                           << " fd_ctx.event="
                           << static_cast<EPOLL_EVENTS>(fd_ctx->events);
    HX_ASSERT(!(fd_ctx->events & event));
  }

  int op = fd_ctx->events != 0U ? EPOLL_CTL_MOD : EPOLL_CTL_ADD;
  epoll_event epevent{};
  epevent.events = EPOLLET | fd_ctx->events | event;
  epevent.data.ptr = fd_ctx;

  int rt = epoll_ctl(m_epfd, op, fd, &epevent);
  if (rt != 0) {
    HX_LOG_ERROR(g_logger) << "epoll_ctl(" << m_epfd << ", " << op << ", " << fd
                           << ", " << static_cast<EPOLL_EVENTS>(epevent.events)
                           << "):" << rt << " (" << errno << ") ("
                           << strerror(errno) << ") fd_ctx->events="
                           << static_cast<EPOLL_EVENTS>(fd_ctx->events);
    return -1;
  }

  ++m_pendingEventCount;
  fd_ctx->events = static_cast<Event>(fd_ctx->events | event);
  FdContext::EventContext& event_ctx = fd_ctx->getContext(event);
  HX_ASSERT(!event_ctx.scheduler && !event_ctx.fiber && !event_ctx.cb);

  event_ctx.scheduler = Scheduler::GetThis();
  if (cb) {
    event_ctx.cb.swap(cb);
  } else {
    event_ctx.fiber = Fiber::GetThis();
    HX_ASSERT1(event_ctx.fiber->getState() == Fiber::EXEC,
               "state=" << event_ctx.fiber->getState());
  }
  return 0;
}

auto IOManager::delEvent(int fd, Event event) -> bool {
  RWMutexType ::ReadLock lock(m_mutex);
  if (m_fdContexts.empty()) {
    return false;
  }
  FdContext* fd_ctx = m_fdContexts[fd];
  lock.unlock();

  FdContext::MutexType::Lock lock1(fd_ctx->mutex);
  if ((fd_ctx->events & event) == 0) {
    return false;
  }
  auto new_events = static_cast<Event>(fd_ctx->events & ~event);
  int op = new_events != 0U ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;
  epoll_event epoll_event{};
  epoll_event.events = EPOLLET | new_events;
  epoll_event.data.ptr = fd_ctx;

  int rt = epoll_ctl(m_epfd, op, fd, &epoll_event);

  if (rt != 0) {
    HX_LOG_ERROR(g_logger) << " epoll_ctl fd (: " << m_epfd << " " << op << " ,"
                           << fd << epoll_event.events << "): " << rt << " ("
                           << errno << " )( " << strerror(errno) << " )";
    return false;
  }
  --m_pendingEventCount;
  fd_ctx->events = new_events;
  FdContext::EventContext& event_ctx = fd_ctx->getContext(event);
  fd_ctx->resetContext(event_ctx);
  return true;
}

auto IOManager::cancelEvent(int fd, Event event) -> bool {
  RWMutexType::ReadLock lock(m_mutex);
  if (static_cast<int>(m_fdContexts.size()) <= fd) {
    return false;
  }
  FdContext* fd_ctx = m_fdContexts[fd];
  lock.unlock();

  FdContext::MutexType::Lock lock2(fd_ctx->mutex);
  if (((fd_ctx->events & event) == 0)) {
    return false;
  }

  auto new_events = static_cast<Event>(fd_ctx->events & ~event);
  int op = new_events != 0U ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;
  epoll_event epoll_event{};
  epoll_event.events = EPOLLET | new_events;
  epoll_event.data.ptr = fd_ctx;

  int rt = epoll_ctl(m_epfd, op, fd, &epoll_event);
  if (rt != 0) {
    HX_LOG_ERROR(g_logger) << "epoll_ctl(" << m_epfd << ", " << op << ", " << fd
                           << ", "
                           << static_cast<EPOLL_EVENTS>(epoll_event.events)
                           << "):" << rt << " (" << errno << ") ("
                           << strerror(errno) << ")";
    return false;
  }

  fd_ctx->triggerEvent(event);
  --m_pendingEventCount;
  return true;
}

auto IOManager::cancelAll(int fd) -> bool {
  RWMutexType::ReadLock lock(m_mutex);
  if (static_cast<int>(m_fdContexts.size()) <= fd) {
    return false;
  }
  FdContext* fd_ctx = m_fdContexts[fd];
  lock.unlock();

  FdContext::MutexType::Lock lock2(fd_ctx->mutex);
  if (fd_ctx->events == 0U) {
    return false;
  }

  int op = EPOLL_CTL_DEL;
  epoll_event epevent{};
  epevent.events = NONE;
  epevent.data.ptr = fd_ctx;

  int rt = epoll_ctl(m_epfd, op, fd, &epevent);
  if (rt != 0) {
    HX_LOG_ERROR(g_logger) << "epoll_ctl(" << m_epfd << ", " << op << ", " << fd
                           << ", " << static_cast<EPOLL_EVENTS>(epevent.events)
                           << "):" << rt << " (" << errno << ") ("
                           << strerror(errno) << ")";
    return false;
  }

  if ((fd_ctx->events & READ) != 0) {
    fd_ctx->triggerEvent(READ);
    --m_pendingEventCount;
  }
  if ((fd_ctx->events & WRITE) != 0) {
    fd_ctx->triggerEvent(WRITE);
    --m_pendingEventCount;
  }

  HX_ASSERT(fd_ctx->events == 0);
  return true;
}
auto IOManager::GetThis() -> IOManager* {
  return dynamic_cast<IOManager*>(Scheduler::GetThis());
}

void IOManager::tickle() {
  if (hasIdleThreads()) {
    return;
  }
  ssize_t rt = write(m_tickleFds[1], "T", 1);
  HX_ASSERT(rt == 1);
}

auto IOManager::stopping(uint64_t& timeout) -> bool {
  timeout = getNextTimer();
  return timeout == ~0ULL && m_pendingEventCount == 0 && Scheduler::stopping();
}
auto IOManager::stopping() -> bool {
  uint64_t timeout = 0;
  return stopping(timeout);
}

void IOManager::idle() {
  const uint64_t maxevents = 256;
  auto* events = new epoll_event[maxevents]();
  std::shared_ptr<epoll_event> shared_events(
      events, [](epoll_event* ptr) { delete[] ptr; });

  while (true) {
    uint64_t next_timeout = 0;
    if ((stopping(next_timeout))) {
      HX_LOG_INFO(g_logger) << "name= : " << getName() << " idle stopping exit";
      break;
    }

    int rt = 0;
    do {
      static const int MAX_TIMEOUT = 3000;
      if (next_timeout != ~0ULL) {
        next_timeout = static_cast<int>(next_timeout) > MAX_TIMEOUT
                           ? MAX_TIMEOUT
                           : next_timeout;
      } else {
        next_timeout = MAX_TIMEOUT;
      }
      rt =
          epoll_wait(m_epfd, events, maxevents, static_cast<int>(next_timeout));
      if (rt < 0 && errno == EINTR) {
      } else {
        break;
      }
    } while (true);

    std::vector<std::function<void()> > cbs;
    listExpiredCb(cbs);
    if (!cbs.empty()) {
      schedule(cbs.begin(), cbs.end());
      cbs.clear();
    }

    for (int i = 0; i < rt; ++i) {
      epoll_event& event = events[i];
      if (event.data.fd == m_tickleFds[0]) {
        uint8_t dummy[256];
        while (read(m_tickleFds[0], dummy, sizeof(dummy)) > 0) {
          ;
        }
        continue;
      }

      auto* fd_ctx = static_cast<FdContext*>(event.data.ptr);
      FdContext::MutexType::Lock lock(fd_ctx->mutex);
      if ((event.events & (EPOLLERR | EPOLLHUP)) != 0U) {
        event.events |= (EPOLLIN | EPOLLOUT) & fd_ctx->events;
      }
      int real_events = NONE;
      if ((event.events & EPOLLIN) != NONE) {
        real_events |= READ;
      }
      if ((event.events & EPOLLOUT) != NONE) {
        real_events |= WRITE;
      }

      if ((fd_ctx->events & real_events) == NONE) {
        continue;
      }

      int left_events = (fd_ctx->events & ~real_events);
      int op = left_events != 0 ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;
      event.events = EPOLLET | left_events;

      int rt2 = epoll_ctl(m_epfd, op, fd_ctx->fd, &event);
      if (rt2 != 0) {
        HX_LOG_ERROR(g_logger)
            << "epoll_ctl(" << m_epfd << ", " << op << ", " << fd_ctx->fd
            << ", " << static_cast<EPOLL_EVENTS>(event.events) << "):" << rt2
            << " (" << errno << ") (" << strerror(errno) << ")";
        continue;
      }

      if ((real_events & READ) != 0) {
        fd_ctx->triggerEvent(READ);
        --m_pendingEventCount;
      }
      if ((real_events & WRITE) != 0) {
        fd_ctx->triggerEvent(WRITE);
        --m_pendingEventCount;
      }
    }

    Fiber::ptr cur = Fiber::GetThis();
    auto raw_ptr = cur.get();
    cur.reset();

    raw_ptr->swapOut();
  }
}

void IOManager::onTimerInsertedAtFront() { tickle(); }
}  // namespace hx_sylar