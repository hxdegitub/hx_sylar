#include "fiber.h"

#include <atomic>

#include "hx_sylar.h"
#include "scheduler.h"
#include "util.h"

namespace hx_sylar {

static Logger::ptr g_logger = HX_LOG_NAME("system");
static std::atomic<uint64_t> s_fiber_id(0);
static std::atomic<uint32_t> s_fiber_count(0);
static thread_local Fiber* t_fiber = nullptr;
static thread_local Fiber::ptr t_threadFiber = nullptr;
static ConfigVar<uint32_t>::ptr g_fiber_stack_size = Config::Lookup<uint32_t>(
    "fiber.stack_size", 1024 * 1024, "fiber stack size");
class MallocStackAllocator {
 public:
  static void* Alloc(size_t size) { return malloc(size); }

  static void Dealloc(void* vp, size_t size) { return free(vp); }
};

using StackAllocator = MallocStackAllocator;

uint64_t Fiber::GetFiberId() {
  if (t_fiber) {
    return t_fiber->getId();
  }
  return 0;
}
Fiber::Fiber() {
  m_state = EXEC;
  SetThis(this);
  if (getcontext(&m_ctx)) {
    HX_ASSERT1(false, "getcontext");
  }

  ++s_fiber_count;

  HX_LOG_DEBUG(g_logger) << " Fiber create ";
}

Fiber::Fiber(std::function<void()> cb, size_t stacksize, bool use_caller)
    : m_id(++s_fiber_id), m_cb(cb) {
  ++s_fiber_count;
  m_stacksize = stacksize ? stacksize : g_fiber_stack_size->getValue();

  m_stack = StackAllocator::Alloc(m_stacksize);
  if (getcontext(&m_ctx)) {
    HX_ASSERT1(false, "getContext");
  }
  m_ctx.uc_link = nullptr;
  m_ctx.uc_stack.ss_sp = m_stack;
  m_ctx.uc_stack.ss_size = m_stacksize;

  if (!use_caller) {
    makecontext(&m_ctx, &Fiber::MainFunc, 0);
  } else {
    makecontext(&m_ctx, &Fiber::CallerMainFunc, 0);
  }

  HX_LOG_INFO(g_logger) << " create fiber ";
}

Fiber::~Fiber() {
  --s_fiber_count;
  if (m_stack) {
    HX_ASSERT(m_state == TERM || m_state == INIT || m_state == EXCEPT);
    StackAllocator::Dealloc(m_stack, m_stacksize);
  } else {
    HX_ASSERT(!m_cb);
    HX_ASSERT(m_state == EXEC);
    Fiber* cur = t_fiber;
    if (cur == this) {
      SetThis(nullptr);
    }
  }
  HX_LOG_DEBUG(g_logger) << "Fiber::~Fiber id=" << m_id
                         << " total=" << s_fiber_count;
}

void Fiber::reset(std::function<void()> cb) {
  HX_ASSERT(m_stack);
  HX_ASSERT(m_state == TERM || m_state == INIT || m_state == EXCEPT);
  m_cb = cb;
  if (getcontext(&m_ctx)) {
    HX_ASSERT(false);
  }
  m_ctx.uc_link = nullptr;
  m_ctx.uc_stack.ss_sp = m_stack;
  m_ctx.uc_stack.ss_size = m_stacksize;
  makecontext(&m_ctx, &Fiber::MainFunc, 0);
  m_state = INIT;
}

void Fiber::call() {
  SetThis(this);
  m_state = EXEC;
  if (swapcontext(&(t_threadFiber->m_ctx), &m_ctx)) {
    HX_ASSERT1(false, "swapcontext");
  }
  // HX_LOG_INFO(g_logger) << "swapin call";
}

void Fiber::back() {
  SetThis(t_threadFiber.get());
  m_state = EXEC;
  if (swapcontext(&m_ctx, &(t_threadFiber->m_ctx))) {
    HX_ASSERT1(false, "swapcontext");
  }
}

void Fiber::swapIn() {
  SetThis(this);
  HX_ASSERT1(m_state != EXEC, "state error ");
  m_state = EXEC;
  if (swapcontext(&Scheduler::GetMainFiber()->m_ctx, &m_ctx)) {
    HX_ASSERT1(false, "swapcontext");
  }
}

void Fiber::swapOut() {
  SetThis(Scheduler::GetMainFiber());
  if (swapcontext(&m_ctx, &Scheduler::GetMainFiber()->m_ctx)) {
    HX_ASSERT1(false, "swapcontext");
  }
}

void Fiber::SetThis(Fiber* f) { t_fiber = f; }

//协程切换到后台，并且设置为Ready状态
Fiber::ptr Fiber::GetThis() {
  if (t_fiber) {
    return t_fiber->shared_from_this();
  }
  Fiber::ptr main_fiber(new Fiber);
  HX_ASSERT(t_fiber == main_fiber.get());
  t_threadFiber = main_fiber;
  return t_fiber->shared_from_this();
}

void Fiber::YieldToReady() {
  Fiber::ptr cur = GetThis();
  HX_ASSERT(cur->m_state == EXEC);
  cur->m_state = READY;
  cur->swapOut();
}

//协程切换到后台，并且设置为Hold状态
void Fiber::YieldToHold() {
  // HX_LOG_DEBUG(g_logger) << "YieldToHold";
  Fiber::ptr cur = GetThis();
  HX_ASSERT(cur->m_state == EXEC);
  // cur->m_state = HOLD;
  cur->swapOut();
}

uint64_t Fiber::TotalFibers() { return s_fiber_count; }

void Fiber::MainFunc() {
  Fiber::ptr cur = GetThis();
  HX_ASSERT(cur);
  try {
    cur->m_cb();
    cur->m_cb = nullptr;
    cur->m_state = TERM;
  } catch (std::exception& ex) {
    cur->m_state = EXCEPT;
    HX_LOG_ERROR(g_logger) << "Fiber Except: " << ex.what()
                           << " fiber_id= " << cur->getId() << std::endl
                           << hx_sylar::BacktraceToString(10);
  } catch (...) {
    cur->m_state = EXCEPT;
    HX_LOG_ERROR(g_logger) << "Fiber Except"
                           << " fiber_id=" << cur->getId() << std::endl
                           << hx_sylar::BacktraceToString(10);
  }

  auto raw_ptr = cur.get();
  cur.reset();
  raw_ptr->swapOut();
  HX_ASSERT1(false, "never reach");
}

void Fiber::CallerMainFunc() {
  Fiber::ptr cur = GetThis();
  HX_ASSERT(cur);
  try {
    cur->m_cb();
    cur->m_cb = nullptr;
    cur->m_state = TERM;
  } catch (std::exception& ex) {
    cur->m_state = EXCEPT;
    HX_LOG_ERROR(g_logger) << "Fiber Except: " << ex.what()
                           << " fiber_id= " << cur->getId() << std::endl
                           << hx_sylar::BacktraceToString(10);
  } catch (...) {
    cur->m_state = EXCEPT;
    HX_LOG_ERROR(g_logger) << "Fiber Except"
                           << " fiber_id=" << cur->getId() << std::endl
                           << hx_sylar::BacktraceToString(10);
  }

  auto raw_ptr = cur.get();
  cur.reset();
  raw_ptr->back();
  HX_ASSERT1(false, "never reach" + std::to_string(raw_ptr->getId()));
}

}  // namespace hx_sylar
