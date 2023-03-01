#include "iomanager.h"

#include <error.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <unistd.h>

#include "log.h"
#include "macro.h"
namespace hx_sylar {

IOManager::FdContext::EventContext& IOManager::FdContext::getContext(
    IOManager::Event event) {
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
  events = (Event)(event & event);
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
  HX_ASSERT(rt);

  epoll_event event;
  memset(&event, 0, sizeof(epoll_event));
  event.events = EPOLLIN | EPOLLET;
  event.data.fd = m_tickleFds[0];

  rt = fcntl(m_tickleFds[0], F_SETFL, O_NONBLOCK);
  HX_ASSERT(rt);

  rt = epoll_ctl(m_epfd, EPOLL_CTL_ADD, m_tickleFds[0], &event);

  HX_ASSERT(rt);

  m_fdContexts.resize(64);
  start();
}

IOManager::~IOManager() {
  stop();
  close(m_epfd);
  close(m_tickleFds[0]);
  close(m_tickleFds[1]);
  for (size_t i = 0; i < m_fdContexts.size(); ++i) {
    if (m_fdContexts[i]) {
      delete m_fdContexts[i];
    }
  }
}

void IOManager::contextResize(size_t size) {}

int IOManager::addEvent(int fd, Event event, std::function<void()> cb) {
  FdContext* fd_ctx = nullptr;
  RWMutexType::ReadLock lock(m_mutex);
  if ((int)m_fdContexts.size() > fd) {
    fd_ctx = m_fdContexts[fd];
    lock.unlock();
  } else {
    lock.unlock();
    RWMutexType::WriteLock lock2(m_mutex);
    contextResize(fd * 1.5);
    fd_ctx = m_fdContexts[fd];
  }

  FdContext::MutexType::Lock lock2(fd_ctx->mutex);
  if ((fd_ctx->events & event)) {
    HX_LOG_ERROR(g_logger) << "addEvent assert fd=" << fd
                           << " event=" << (EPOLL_EVENTS)event
                           << " fd_ctx.event=" << (EPOLL_EVENTS)fd_ctx->events;
    HX_ASSERT(!(fd_ctx->events & event));
  }

  int op = fd_ctx->events ? EPOLL_CTL_MOD : EPOLL_CTL_ADD;
  epoll_event epevent;
  epevent.events = EPOLLET | fd_ctx->events | event;
  epevent.data.ptr = fd_ctx;

  int rt = epoll_ctl(m_epfd, op, fd, &epevent);
  if (rt) {
    HX_LOG_ERROR(g_logger) << "epoll_ctl(" << m_epfd << ", " << op << ", " << fd
                           << ", " << (EPOLL_EVENTS)epevent.events << "):" << rt
                           << " (" << errno << ") (" << strerror(errno)
                           << ") fd_ctx->events="
                           << (EPOLL_EVENTS)fd_ctx->events;
    return -1;
  }

  ++m_pendingEventCount;
  fd_ctx->events = (Event)(fd_ctx->events | event);
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

bool IOManager::delEvent(int fd, Event event) {
  RWMutexType ::ReadLock lock(m_mutex);
  if (m_fdContexts.size() < 0) {
    return false;
  }
  FdContext* fd_ctx = m_fdContexts[fd];
  lock.unlock();

  FdContext::MutexType::Lock lock1(fd_ctx->mutex);
  if (!(fd_ctx->events & event)) {
    return false;
  }
  Event new_events = (Event)(fd_ctx->events & ~event);
  int op = new_events ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;
  epoll_event epevent;
  epevent.events = EPOLLET | new_events;
  epevent.data.ptr = fd_ctx;

  int rt = epoll_ctl(m_epfd, op, fd, &epevent);

  if (rt) {
    HX_LOG_ERROR(g_logger) << " epoll_ctl fd (: " << m_epfd << " " << op << " ,"
                           << fd << epevent.events << "): " << rt << " ("
                           << errno << " )( " << strerror(errno) << " )";
    return false;
  }
  --m_pendingEventCount;
  fd_ctx->events = new_events;
  FdContext::EventContext& event_ctx = fd_ctx->getContext(event);
  fd_ctx->resetContext(event_ctx);
  return true;
}

bool IOManager::cancelEvent(int fd, Event event) {
  RWMutexType::ReadLock lock(m_mutex);
  if ((int)m_fdContexts.size() <= fd) {
    return false;
  }
  FdContext* fd_ctx = m_fdContexts[fd];
  lock.unlock();

  FdContext::MutexType::Lock lock2(fd_ctx->mutex);
  if ((!(fd_ctx->events & event))) {
    return false;
  }

  Event new_events = (Event)(fd_ctx->events & ~event);
  int op = new_events ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;
  epoll_event epevent;
  epevent.events = EPOLLET | new_events;
  epevent.data.ptr = fd_ctx;

  int rt = epoll_ctl(m_epfd, op, fd, &epevent);
  if (rt) {
    HX_LOG_ERROR(g_logger) << "epoll_ctl(" << m_epfd << ", " << op << ", " << fd
                           << ", " << (EPOLL_EVENTS)epevent.events << "):" << rt
                           << " (" << errno << ") (" << strerror(errno) << ")";
    return false;
  }

  fd_ctx->triggerEvent(event);
  --m_pendingEventCount;
  return true;
}

bool IOManager::canceAll(int fd) {
  RWMutexType::ReadLock lock(m_mutex);
  if ((int)m_fdContexts.size() <= fd) {
    return false;
  }
  FdContext* fd_ctx = m_fdContexts[fd];
  lock.unlock();

  FdContext::MutexType::Lock lock2(fd_ctx->mutex);
  if (!fd_ctx->events) {
    return false;
  }

  int op = EPOLL_CTL_DEL;
  epoll_event epevent;
  epevent.events = 0;
  epevent.data.ptr = fd_ctx;

  int rt = epoll_ctl(m_epfd, op, fd, &epevent);
  if (rt) {
    HX_LOG_ERROR(g_logger) << "epoll_ctl(" << m_epfd << ", " << op << ", " << fd
                           << ", " << (EPOLL_EVENTS)epevent.events << "):" << rt
                           << " (" << errno << ") (" << strerror(errno) << ")";
    return false;
  }

  if (fd_ctx->events & READ) {
    fd_ctx->triggerEvent(READ);
    --m_pendingEventCount;
  }
  if (fd_ctx->events & WRITE) {
    fd_ctx->triggerEvent(WRITE);
    --m_pendingEventCount;
  }

  HX_ASSERT(fd_ctx->events == 0);
  return true;
}
static IOManager* GetThis() {
  return dynamic_cast<IOManager*>(Scheduler::GetThis());
}

}  // namespace hx_sylar