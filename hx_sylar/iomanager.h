#ifndef __HX_IOMANAGER_H__
#define __HX_IOMANAGER_H__

#include "scheduler.h"
#include "timer.h"

namespace hx_sylar {
class TimerManager;
class IOManager : public Scheduler, public TimerManager {
 public:
  using ptr = std::shared_ptr<IOManager>;
  using RWMutexType = RWMutex;

  enum Event {
    NONE = 0x0,
    READ = 0x1,   /// EPOLLIN
    WRITE = 0x4,  // EPOLLOUT
  };

 private:
  struct FdContext {
    using MutexType = Mutex;
    struct EventContext {
      // 事件执行的调度器
      Scheduler* scheduler = nullptr;
      ///事件协程
      Fiber::ptr fiber;
      //事件的回调函数
      std::function<void()> cb;
    };

    // member function:
    auto getContext(Event event) -> EventContext&;
    void resetContext(EventContext& eventContext);
    void triggerEvent(Event event);
    // member data
    EventContext read;
    EventContext write;
    int fd;
    Event events = NONE;
    MutexType mutex;
  };

 public:
  explicit IOManager(size_t threads = 1, bool user_call = true,
                     const std::string& name = "");
  ~IOManager() override;

  auto addEvent(int fd, Event event, std::function<void()> cb = nullptr) -> int;
  auto delEvent(int fd, Event evnet) -> bool;
  auto cancelEvent(int fd, Event event) -> bool;
  bool cancelAll(int fd);
  static auto GetThis() -> IOManager*;

 protected:
  void tickle() override;

  auto stopping() -> bool override;
  auto stopping(uint64_t& timeout) -> bool;
  void idle() override;

  void contextResize(size_t size);

  void onTimerInsertedAtFront() override;
  // bool stopping(uint64_t& timeout);

 private:
  int m_epfd = 0;
  int m_tickleFds[2]{};
  std::atomic<size_t> m_pendingEventCount = {0};
  RWMutexType m_mutex;
  std::vector<FdContext*> m_fdContexts;
};
}  // namespace hx_sylar

#endif
