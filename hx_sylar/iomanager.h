#ifndef __HX_IOMANAGER_H__
#define __HX_IOMANAGER_H__
#include "scheduler.h"
namespace hx_sylar {
class IOManager : public Scheduler {
 public:
  typedef std::shared_ptr<IOManager> ptr;
  typedef RWMutex RWMutexType;

  enum Event {
    NONE = 0x0,
    READ = 0x1,   /// EPOLLIN
    WRITE = 0x4,  // EPOLLOUT
  };

 private:
  struct FdContext {
    typedef Mutex MutexType;
    struct EventContext {
      // 事件执行的调度器
      Scheduler* scheduler = nullptr;
      ///事件协程
      Fiber::ptr fiber;
      //事件的回调函数
      std::function<void()> cb;
    };

    // member function:
    EventContext& getContext(Event event);
    void resetContext(EventContext& eventContext);
    void triggerEvent(Event event);
    // member data
    EventContext read;
    EventContext write;
    int fd;  //
    Event events = NONE;
    MutexType mutex;
  };

 public:
  IOManager(size_t threads = 1, bool user_call = true,
            const std::string& name = "");
  ~IOManager();

  int addEvent(int fd, Event event, std::function<void()> cb = nullptr);
  bool delEvent(int fd, Event evnet);
  bool cancelEvent(int fd, Event event);
  bool canceAll(int fd);
  static IOManager* GetThis();

 protected:
  void tickle() override;

  bool stopping() override;
  void idle() override;

  void contextResize(size_t size);
  // bool stopping(uint64_t& timeout);

 private:
  int m_epfd = 0;
  int m_tickleFds[2];
  std::atomic<size_t> m_pendingEventCount = {0};
  RWMutexType m_mutex;
  std::vector<FdContext*> m_fdContexts;
};
}  // namespace hx_sylar

#endif
