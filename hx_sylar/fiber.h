#ifndef __HX_FIBER_H__
#define __HX_FIBER_H__
#include <ucontext.h>

#include <functional>
#include <memory>

#include "fiber.h"
#include "hx_sylar.h"
#include "thread.h"
namespace hx_sylar {
class Fiber : public std::enable_shared_from_this<Fiber> {
  friend class Scheduler;

 public:
  typedef std::shared_ptr<Fiber> ptr;
  enum State {
    INIT,
    HOLD,
    EXEC,
    TERM,
    READY,
    EXCEPT,
  };

 private:
  Fiber();

 public:
  Fiber(std::function<void()> cb, size_t staksize = 0);
  ~Fiber();
  // 重置协程函数
  void reset(std::function<void()> cb);

  //  切换到当前协程
  void swapIn();
  // 切换到后台执行
  void swapOut();
  void call();
  void back();
  uint64_t getId() const { return m_id; }

 public:
  // 设置当前协程
  static void SetThis(Fiber* f);
  // 返回当前协程
  static Fiber::ptr GetThis();
  // 协程切换到后台 设置为Ready
  static void YieldToReady();
  // 协程切换到后，设置为Hold
  static void YieldToHold();
  // 总协程数
  static uint64_t TotalFibers();

  static void MainFunc();

  static uint64_t GetFiberId();

  State getState() const { return m_state; }
  //  void back();
  void setState(State s);

 private:
  uint64_t m_id = 0;
  uint32_t m_stacksize = 0;
  State m_state = INIT;
  ucontext_t m_ctx;
  void* m_stack = nullptr;
  std::function<void()> m_cb;
};

}  // namespace hx_sylar
#endif
