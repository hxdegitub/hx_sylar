#ifndef __HX_THREAD_H__
#define __HX_THREAD_H__

#include <semaphore.h>
#include <unistd.h>

#include <atomic>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

#include "mutex.h"
#include "noncopyable.h"
namespace hx_sylar {

class Thread {
 public:
  using ptr = std::shared_ptr<Thread>;
  Thread(std::function<void()> cb, const std::string& name);

  ~Thread();

  auto getId() const -> pid_t { return m_id; }

  auto getName() const -> const std::string& { return m_name; }

  void join();

  static auto GetThis() -> Thread*;
  static auto GetName() -> const std::string&;
  static void SetName(const std::string& name);

 private:
  Thread(const Thread&) = delete;
  Thread(const Thread&&) = delete;
  auto operator=(const Thread&) -> Thread& = delete;
  static auto run(void* arg) -> void*;

 private:
  pid_t m_id = -1;
  pthread_t m_thread = 0;
  std::function<void()> m_cb;
  std::string m_name;

  Semaphore m_semaphore;
};

}  // namespace hx_sylar
#endif