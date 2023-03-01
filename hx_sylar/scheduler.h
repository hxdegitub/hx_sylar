#ifndef __HX_SCHEDULER_H__
#define __HX_SCHEDULER_H__
#include <functional>
#include <memory>

#include "fiber.h"
#include "thread.h"
namespace hx_sylar {
class Scheduler {
 public:
  typedef std::shared_ptr<Scheduler> ptr;
  typedef Mutex MutexType;
  Scheduler(size_t threads = 1, bool use_caller = true,
            const std::string& name = "");
  virtual ~Scheduler();
  const std::string& getName() const { return m_name; }
  static Scheduler* GetThis();
  static Fiber* GetMainFiber();

  void start();
  void stop();
  template <class FiberOrCb>
  void schedule(FiberOrCb cb, int thread = -1) {
    bool need_tickle = false;
    {
      MutexType::Lock lock(m_mutex);
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
      MutexType::Lock lock(m_mutex);
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
  virtual bool stopping();
  virtual void idle();
  void setThis();

 private:
  template <class FiberOrCb>
  bool scheduleNoLock(FiberOrCb fc, int thread) {
    bool need_tickle = m_fibers.empty();
    FiberAndThread ft(fc, thread);
    if (ft.fiber || ft.cb) {
      m_fibers.push_back(ft);
    }
    return need_tickle;
  }

 private:
  struct FiberAndThread {
    /// 协程
    Fiber::ptr fiber;
    /// 协程执行函数
    std::function<void()> cb;
    /// 线程id
    int thread;

    FiberAndThread(Fiber::ptr f, int thr) : fiber(f), thread(thr) {}
    FiberAndThread(Fiber::ptr* f, int t) : thread(t) { fiber.swap(*f); }
    FiberAndThread(std::function<void()> f, int thr) : cb(f), thread(thr) {}
    FiberAndThread(std::function<void()>* f, int thr) : thread(thr) {
      cb.swap(*f);
    }

    FiberAndThread() : thread(-1) {}
    // clear the fiber
    void reset() {
      fiber = nullptr;
      cb = nullptr;
      thread = -1;
    }
  };

 private:
  MutexType m_mutex;
  std::vector<Thread::ptr> m_threads;
  std::list<FiberAndThread> m_fibers;
  Fiber::ptr m_rootFiber;
  std::string m_name;

 protected:
  std::vector<int> m_threadIds;
  size_t m_threadCount = 0;
  std::atomic<size_t> m_activeThreadCount = {0};
  std::atomic<size_t> m_idleThreadCount = {0};
  bool m_stopping = true;
  bool m_autoStop = false;
  int m_rootThread = 0;
};

}  // namespace hx_sylar
#endif
