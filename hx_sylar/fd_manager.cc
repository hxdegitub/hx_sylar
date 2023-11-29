#include "fd_manager.h"

#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include <memory>

#include "hook.h"

namespace hx_sylar {
FdCtx::FdCtx(int fd)
    : m_isInit(false),
      m_isSocket(false),
      m_sysNonblock(false),
      m_userNonblock(false),
      m_isClosed(false),
      m_fd(fd),
      m_recvTimeout(-1),
      m_sendTimeout(-2) {
  init();
}
FdCtx::~FdCtx() = default;
auto FdCtx::init() -> bool {
  if (m_isInit) {
    return true;
  }
  m_recvTimeout = -1;
  m_sendTimeout = -1;

  struct stat fd_stat {};
  if (-1 == fstat(m_fd, &fd_stat)) {
    m_isInit = false;
    m_isSocket = false;
  } else {
    m_isInit = true;
    m_isSocket = true;
  }

  if (m_isSocket) {
    int flags = fcntl_f(m_fd, F_GETFL, 0);
    if ((flags & O_NONBLOCK) != 0) {
      fcntl_f(m_fd, F_SETFL, flags | O_NONBLOCK);
    }
    m_sysNonblock = true;
  } else {
    m_sysNonblock = false;
  }
  m_userNonblock = false;
  m_isClosed = false;
  return m_isInit;
}

void FdCtx::setTimeout(int type, uint64_t v) {
  if (type == SO_RCVTIMEO) {
    m_recvTimeout = v;
  } else {
    m_sendTimeout = v;
  }
}

auto FdCtx::getTimeout(int type) -> uint64_t {
  if (type == SO_RCVTIMEO) {
    return m_recvTimeout;
  }
  return m_sendTimeout;
}
auto FdManager::get(int fd, bool auto_create) -> FdCtx::ptr {
  RwMutexType ::ReadLock lock(m_mutex);
  if (static_cast<int>(m_datas.size()) <= fd) {
    if (!auto_create) {
      return nullptr;
    }
  } else {
    if (m_datas[fd] || !auto_create) {
      return m_datas[fd];
    }
  }
  lock.unlock();
  RwMutexType ::WriteLock lock2(m_mutex);
  auto ctx = std::make_shared<FdCtx>(fd);
  m_datas[fd] = ctx;
  return ctx;
}
void FdManager::del(int fd) {
  RwMutexType ::WriteLock lock(m_mutex);
  if (static_cast<int>(m_datas.size()) <= fd) {
    return;
  }
  m_datas[fd].reset();
}
FdManager::FdManager() { m_datas.resize(64); }

}  // namespace hx_sylar
