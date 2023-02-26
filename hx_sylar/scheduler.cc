#include "scheduler.h"

#include "log.h"
#include "singleton.h"
#include "thread.h"
#include "util.h"

namespace hx_sylar {
static Logger::ptr g_logger = HX_LOG_NAME("system");
static thread_local Scheduler* t_scheduler = nullptr;
static thread_local Fiber* t_fiber = nullptr;
Scheduler::Scheduler(size_t threads, bool use_caller, const std::string& name):m_name(name) {
  HX_ASSERT(threads > 0);
  if (use_caller) {
    hx_sylar::Fiber::GetThis();
    --threads;
    HX_ASSERT(GetThis() == nullptr);
    t_scheduler = this;

    m_rootFiber.reset(new Fiber(std::bind(&Scheduler::run, this)));
    hx_sylar::Thread::SetName(m_name);

    t_fiber = m_rootFiber.get();
    m_rootThread = hx_sylar::GetThreadId();
    m_threadIds.push_back(m_rootThread);
  } else {
    m_rootThread = -1;
  }
  m_threadCount = -1;
}
Scheduler::~Scheduler() {
  HX_ASSERT(m_stopping);
  if (GetThis() == this) {
    t_scheduler = nullptr;
  }
}
Scheduler* Scheduler::GetThis() { return t_scheduler; }
Fiber* Scheduler::GetMainFiber() { return t_fiber; }
void Scheduler::start() {
  MutexType::Lock lock(m_mutex);
  if (!m_stopping) {
    return;
  }
  m_stopping = false;
  HX_ASSERT(m_threads.empty());
  for (size_t i = 0; i < m_threadCount; ++i) {
    m_threads[i].reset(new Thread(std::bind(&Scheduler::run, this),
                                  m_name + " " + std::to_string(i)));
    m_threadIds.push_back(m_threads[i]->getId());
  }
}
void  Scheduler::stop(){
  m_autoStop = true;
  if (m_rootFiber && m_threadCount == 0 &&
      m_rootFiber->getState() == Fiber::INIT) {
    HX_LOG_INFO(g_logger) << this << " stopped ";
    m_stopping = true;
    if(stopping()){
      return ;
    }
  }
  bool exit_on_this_fiber = false;
  if(m_rootThread != -1){
    HX_ASSERT(GetThis() == this);
  }else{
    HX_ASSERT(GetThis() != this);
    }

    m_stopping = true;
    for(size_t i = 0 ; i < m_threadCount ; ++ i){
      tickle();
    }
     if(m_rootFiber){
        tickle();
     }
//    if(exit_on_this_fiber){}
 }
void Scheduler::run(){
   setThis();
   if(hx_sylar::GetThread() != m_rootThread){
     t_fiber = Fiber::GetThis().get();
   }
}


 Fiber::ptr idle_fiber(new Fiber());

}  // namespace hx_sylar
