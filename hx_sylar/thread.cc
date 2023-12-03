#include "thread.h"

#include <utility>

#include "log.h"
#include "util.h"

namespace hx_sylar {

static thread_local Thread* t_thread = nullptr;
static thread_local std::string t_thread_name = "UNKNOWN";

static hx_sylar::Logger::ptr g_logger = HX_LOG_NAME("system");

auto Thread::GetThis() -> Thread* { return t_thread; }

auto Thread::GetName() -> const std::string& { return t_thread_name; }

void Thread::SetName(const std::string& name) {
  if (name.empty()) {
    return;
  }
  if (t_thread != nullptr) {
    t_thread->m_name = name;
  }
  t_thread_name = name;
}

Thread::Thread(std::function<void()> cb, const std::string& name)
    : m_cb(std::move(cb)), m_name(name) {
  if (name.empty()) {
    m_name = "UNKNOWN";
  }
  int rt = pthread_create(&m_thread, nullptr, &Thread::run, this);
  if (rt != 0) {
    HX_LOG_ERROR(g_logger) << "pthread_create thread fail, rt=" << rt
                           << " name=" << name;
    throw std::logic_error("pthread_create error");
  }
  m_semaphore.wait();
}

Thread::~Thread() {
  if (m_thread != 0U) {
    pthread_detach(m_thread);
  }
}

void Thread::join() {
  if (m_thread != 0U) {
    int rt = pthread_join(m_thread, nullptr);
    if (rt != 0) {
      HX_LOG_ERROR(g_logger)
          << "pthread_join thread fail, rt=" << rt << " name=" << m_name;
      throw std::logic_error("pthread_join error");
    }
    m_thread = 0;
  }
}

auto Thread::run(void* arg) -> void* {
  auto* thread = static_cast<Thread*>(arg);
  t_thread = thread;
  t_thread_name = thread->m_name;
  thread->m_id = hx_sylar::GetThreadId();
  pthread_setname_np(pthread_self(), thread->m_name.substr(0, 15).c_str());

  std::function<void()> cb;
  cb.swap(thread->m_cb);

  thread->m_semaphore.notify();

  cb();
  return nullptr;
}

}  // namespace hx_sylar
