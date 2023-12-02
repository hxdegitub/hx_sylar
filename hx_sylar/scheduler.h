#ifndef __HX_SCHEDULER_H__
#define __HX_SCHEDULER_H__
#include <iostream>
#include <list>
#include <memory>
#include <vector>

#include "fiber.h"
#include "thread.h"

namespace hx_sylar {
class Scheduler {
 public:
  using threadId = int;
  using ptr = std::shared_ptr<Scheduler>;
  using MutexType = Mutex;
  explicit Scheduler(size_t threads = 1, bool use_caller = true,
                     std::string name = "");
  virtual ~Scheduler();
  auto getName() const -> const std::string& { return m_name_; }
  static auto GetThis() -> Scheduler*;
  static auto GetMainFiber() -> Fiber*;

  void start();
  void stop();
  template <class FiberOrCb>
  void schedule(FiberOrCb cb, int thread = -1) {
    bool need_tickle = false;
    {
      MutexType::Lock lock(m_mutex_);
      need_tickle = scheduleNoLock(cb, thread);
    }
    if (need_tickle) {
      tickle();
    }
  }

  template <class InputIterator>
  void schedule(InputIterator begin, InputIterator end) {
    bool need_tickle = false;
    {
      MutexType::Lock lock(m_mutex_);
      while (begin != end) {
        need_tickle = scheduleNoLock(&*begin, -1) || need_tickle;
        ++begin;
      }
    }
    if (need_tickle) {
      tickle();
    }
  }

  void swithcTo(int thread = -1);
  std::ostream& dump(std::ostream& os);

 protected:
  virtual void tickle();
  void run();
  virtual auto stopping() -> bool;
  virtual void idle();
  void setThis();
  auto hasIdleThreads() -> bool { return m_idle_thread_count_ > 0; }

 private:
  template <class FiberOrCb>
  auto scheduleNoLock(FiberOrCb fc, int thread) -> bool {
    bool need_tickle = m_fibers_.empty();
    FiberAndThread ft(fc, thread);
    if (ft.fiber_ || ft.cb_) {
      m_fibers_.push_back(ft);
    }
    return need_tickle;
  }

 private:
  struct FiberAndThread {
    /// 协程
    Fiber::ptr fiber_;
    /// 协程执行函数
    std::function<void()> cb_;
    /// 线程id
    threadId thread_;

    FiberAndThread(Fiber::ptr f, int thr)
        : fiber_(std::move(f)), thread_(thr) {}
    FiberAndThread(Fiber::ptr* f, int t) : thread_(t) { fiber_.swap(*f); }
    FiberAndThread(std::function<void()> f, int thr)
        : cb_(std::move(f)), thread_(thr) {}
    FiberAndThread(std::function<void()>* f, int thr) : thread_(thr) {
      cb_.swap(*f);
    }

    FiberAndThread() : thread_(-1) {}
    // clear the fiber
    void reset() {
      fiber_ = nullptr;
      cb_ = nullptr;
      thread_ = -1;
    }
  };

 private:
  MutexType m_mutex_;
  std::vector<Thread::ptr> m_threads_;
  std::list<FiberAndThread> m_fibers_;
  Fiber::ptr m_root_fiber_;
  std::string m_name_;

 protected:
  std::vector<int> m_thread_ids_;
  size_t m_thread_count_ = 0;
  std::atomic<size_t> m_active_thread_count_ = {0};
  std::atomic<size_t> m_idle_thread_count_ = {0};
  bool m_stopping_ = true;
  bool m_auto_stop_ = false;
  int m_root_thread_ = 0;
};

}  // namespace hx_sylar
#endif
