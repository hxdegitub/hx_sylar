#include "scheduler.h"

#include "hook.h"
#include "log.h"
#include "macro.h"

namespace hx_sylar {

static hx_sylar::Logger::ptr g_logger = HX_LOG_NAME("system");

static thread_local Scheduler* t_scheduler = nullptr;
static thread_local Fiber* t_scheduler_fiber = nullptr;

Scheduler::Scheduler(size_t threads, bool use_caller, std::string name)
    : m_name_(std::move(name)) {
  HX_ASSERT(threads > 0);

  if (use_caller) {
    hx_sylar::Fiber::GetThis();
    --threads;

    HX_ASSERT(GetThis() == nullptr);
    t_scheduler = this;

    m_root_fiber_.reset(new Fiber(std::bind(&Scheduler::run, this), 0, true));
    hx_sylar::Thread::SetName(m_name_);

    t_scheduler_fiber = m_root_fiber_.get();
    m_root_thread_ = hx_sylar::GetThreadId();
    m_thread_ids_.push_back(m_root_thread_);
  } else {
    m_root_thread_ = -1;
  }
  m_thread_count_ = threads;
}

Scheduler::~Scheduler() {
  HX_ASSERT(m_stopping_);
  if (GetThis() == this) {
    t_scheduler = nullptr;
  }
}

auto Scheduler::GetThis() -> Scheduler* { return t_scheduler; }

auto Scheduler::GetMainFiber() -> Fiber* { return t_scheduler_fiber; }

void Scheduler::start() {
  MutexType::Lock lock(m_mutex_);
  if (!m_stopping_) {
    return;
  }
  m_stopping_ = false;
  HX_ASSERT(m_threads_.empty());

  m_threads_.resize(m_thread_count_);
  for (size_t i = 0; i < m_thread_count_; ++i) {
    m_threads_[i].reset(new Thread(std::bind(&Scheduler::run, this),
                                   m_name_ + "_" + std::to_string(i)));
    m_thread_ids_.push_back(m_threads_[i]->getId());
  }
  lock.unlock();
}

void Scheduler::stop() {
  m_auto_stop_ = true;
  if (m_root_fiber_ && m_thread_count_ == 0 &&
      (m_root_fiber_->getState() == Fiber::TERM ||
       m_root_fiber_->getState() == Fiber::INIT)) {
    HX_LOG_INFO(g_logger) << this << " stopped";
    m_stopping_ = true;

    if (stopping()) {
      return;
    }
  }

  // bool exit_on_this_fiber = false;
  if (m_root_thread_ != -1) {
    HX_ASSERT(GetThis() == this);
  } else {
    HX_ASSERT(GetThis() != this);
  }

  m_stopping_ = true;
  for (size_t i = 0; i < m_thread_count_; ++i) {
    tickle();
  }

  if (m_root_fiber_) {
    tickle();
  }

  if (m_root_fiber_) {
    // while(!stopping()) {
    //     if(m_root_fiber_->getState() == Fiber::TERM
    //             || m_root_fiber_->getState() == Fiber::EXCEPT) {
    //         m_root_fiber_.reset(new Fiber(std::bind(&Scheduler::run, this),
    //         0, true)); HX_LOG_INFO(g_logger) << " root fiber_ is term,
    //         reset"; t_fiber = m_root_fiber_.get();
    //     }
    //     m_root_fiber_->call();
    // }
    if (!stopping()) {
      m_root_fiber_->call();
    }
  }

  std::vector<Thread::ptr> thrs;
  {
    MutexType::Lock lock(m_mutex_);
    thrs.swap(m_threads_);
  }

  for (auto& i : thrs) {
    i->join();
  }
  // if(exit_on_this_fiber) {
  // }
}

void Scheduler::setThis() { t_scheduler = this; }

void Scheduler::run() {
  HX_LOG_DEBUG(g_logger) << m_name_ << " run";
  hx_sylar::set_hook_enable(true);
  setThis();
  if (hx_sylar::GetThreadId() != m_root_thread_) {
    t_scheduler_fiber = Fiber::GetThis().get();
  }

  Fiber::ptr idle_fiber(new Fiber(std::bind(&Scheduler::idle, this)));
  Fiber::ptr cb_fiber;

  FiberAndThread ft;
  while (true) {
    ft.reset();
    bool tickle_me = false;
    bool is_active = false;
    {
      MutexType::Lock lock(m_mutex_);
      auto it = m_fibers_.begin();
      while (it != m_fibers_.end()) {
        if (it->thread_ != -1 && it->thread_ != hx_sylar::GetThreadId()) {
          ++it;
          tickle_me = true;
          continue;
        }

        HX_ASSERT(it->fiber_ || it->cb_);
        if (it->fiber_ && it->fiber_->getState() == Fiber::EXEC) {
          ++it;
          continue;
        }

        ft = *it;
        m_fibers_.erase(it++);
        ++m_active_thread_count_;
        is_active = true;
        break;
      }
      tickle_me |= it != m_fibers_.end();
    }

    if (tickle_me) {
      tickle();
    }

    if (ft.fiber_ && (ft.fiber_->getState() != Fiber::TERM &&
                      ft.fiber_->getState() != Fiber::EXCEPT)) {
      ft.fiber_->swapIn();
      --m_active_thread_count_;

      if (ft.fiber_->getState() == Fiber::READY) {
        schedule(ft.fiber_);
      } else if (ft.fiber_->getState() != Fiber::TERM &&
                 ft.fiber_->getState() != Fiber::EXCEPT) {
        ft.fiber_->m_state = Fiber::HOLD;
      }
      ft.reset();
    } else if (ft.cb_) {
      if (cb_fiber) {
        cb_fiber->reset(ft.cb_);
      } else {
        cb_fiber.reset(new Fiber(ft.cb_));
      }
      ft.reset();
      cb_fiber->swapIn();
      --m_active_thread_count_;
      if (cb_fiber->getState() == Fiber::READY) {
        schedule(cb_fiber);
        cb_fiber.reset();
      } else if (cb_fiber->getState() == Fiber::EXCEPT ||
                 cb_fiber->getState() == Fiber::TERM) {
        cb_fiber->reset(nullptr);
      } else {
        cb_fiber->m_state = Fiber::HOLD;
        cb_fiber.reset();
      }
    } else {
      if (is_active) {
        --m_active_thread_count_;
        continue;
      }
      if (idle_fiber->getState() == Fiber::TERM) {
        HX_LOG_INFO(g_logger) << "idle fiber_ term";
        break;
      }

      ++m_idle_thread_count_;
      idle_fiber->swapIn();
      --m_idle_thread_count_;
      if (idle_fiber->getState() != Fiber::TERM &&
          idle_fiber->getState() != Fiber::EXCEPT) {
        idle_fiber->m_state = Fiber::HOLD;
      }
    }
  }
}

void Scheduler::tickle() { HX_LOG_INFO(g_logger) << "tickle"; }

auto Scheduler::stopping() -> bool {
  MutexType::Lock lock(m_mutex_);
  return m_auto_stop_ && m_stopping_ && m_fibers_.empty() &&
         m_active_thread_count_ == 0;
}

void Scheduler::idle() {
  HX_LOG_INFO(g_logger) << "idle";
  while (!stopping()) {
    hx_sylar::Fiber::YieldToHold();
  }
}

std::ostream& Scheduler::dump(std::ostream& os) {
  os << "[Scheduler name=" << m_name_ << " size=" << m_thread_count_
     << " active_count=" << m_active_thread_count_
     << " idle_count=" << m_idle_thread_count_ << " stopping=" << m_stopping_
     << " ]" << '\n'
     << "    ";
  for (size_t i = 0; i < m_thread_ids_.size(); ++i) {
    if (i != 0U) {
      os << ", ";
    }
    os << m_thread_ids_[i];
  }
  return os;
}

}  // namespace hx_sylar
