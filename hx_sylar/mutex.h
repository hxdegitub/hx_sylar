#ifndef __HX_MUTEXT_H__
#define __HX_MUTEXT_H__

#include <pthread.h>
#include <semaphore.h>
#include <stdint.h>

#include <atomic>
#include <functional>
#include <list>
#include <memory>
#include <thread>

#include "noncopyable.h"
namespace hx_sylar {
class Semaphore : public Noncopyable {
 public:
  explicit Semaphore(uint32_t count = 0);
  ~Semaphore();
  void wait();
  void notify();

 private:
  Semaphore(const Semaphore&) = delete;
  Semaphore(const Semaphore&&) = delete;
  auto operator=(const Semaphore) -> Semaphore = delete;

 private:
  sem_t m_semaphore;
};

template <class T>
struct ScopedLockImpl {
 public:
  ScopedLockImpl(T& mutex) : m_mutex(mutex) { m_mutex.lock(), m_lock = true; }
  ~ScopedLockImpl() { unlock(); }
  void lock() {
    if (!m_lock) {
      m_mutex.lock();
      m_lock = true;
    }
  }
  void unlock() {
    if (m_lock) {
      m_mutex.unlock();
      m_lock = false;
    }
  }

 private:
  T& m_mutex;
  bool m_lock;
};

class Mutex : public Noncopyable {
 public:
  using Lock = ScopedLockImpl<Mutex>;
  Mutex() { pthread_mutex_init(&m_mutex, nullptr); }
  ~Mutex() { pthread_mutex_destroy(&m_mutex); }
  void lock() { pthread_mutex_lock(&m_mutex); }
  void unlock() { pthread_mutex_unlock(&m_mutex); }

 private:
  pthread_mutex_t m_mutex;
};

template <class T>
struct ReadScopedLockImpl : public Noncopyable {
 public:
  explicit ReadScopedLockImpl(T& mutex) : m_mutex(mutex) {
    m_mutex.rdlock(), m_lock = true;
  }
  ~ReadScopedLockImpl() { unlock(); }
  void lock() {
    if (!m_lock) {
      m_mutex.rdlock();
      m_lock = true;
    }
  }
  void unlock() {
    if (m_lock) {
      m_mutex.unlock();
      m_lock = false;
    }
  }

 private:
  T& m_mutex;
  bool m_lock;
};

template <class T>
struct WriteScopedLockImpl : public Noncopyable {
 public:
  WriteScopedLockImpl(T& mutex) : m_mutex(mutex) {
    m_mutex.wrlock(), m_lock = true;
  }
  ~WriteScopedLockImpl() { unlock(); }
  void lock() {
    if (!m_lock) {
      m_mutex.wrlock();
      m_lock = true;
    }
  }
  void unlock() {
    if (m_lock) {
      m_mutex.unlock();
      m_lock = false;
    }
  }

 private:
  T& m_mutex;
  bool m_lock;
};
class RWMutex : public Noncopyable {  // read write lock
 public:
  using ReadLock = ReadScopedLockImpl<RWMutex>;
  using WriteLock = WriteScopedLockImpl<RWMutex>;
  RWMutex() {
    pthread_rwlock_init(&m_lock, nullptr);
  }  // structor init the lock
  ~RWMutex() {
    pthread_rwlock_destroy(&m_lock);
  }  // destructor destory the lock
  void rdlock() { pthread_rwlock_rdlock(&m_lock); }  // readlock
  void wrlock() { pthread_rwlock_wrlock(&m_lock); }  // wrlock
  void unlock() { pthread_rwlock_unlock(&m_lock); }  // unlock

 private:
  pthread_rwlock_t m_lock;  // lock
};

class NullMutex : public Noncopyable {
  using Lock = ScopedLockImpl<NullMutex>;
  NullMutex() {}
  ~NullMutex() = default;
  void lock() {}
  void unlock() {}
};

class NullRWMutex : public Noncopyable {
 public:
  using ReadLock = ReadScopedLockImpl<NullMutex>;
  using WriteLock = WriteScopedLockImpl<NullMutex>;

  NullRWMutex() = default;
  ~NullRWMutex() = default;
  void lock() {}
  void unlock() {}
};

class Spinlock : public Noncopyable {
 public:
  using Lock = ScopedLockImpl<Spinlock>;

  Spinlock() { pthread_spin_init(&m_mutex, 0); }
  ~Spinlock() { pthread_spin_destroy(&m_mutex); }
  void lock() { pthread_spin_lock(&m_mutex); }
  void unlock() { pthread_spin_unlock(&m_mutex); }

 private:
  pthread_spinlock_t m_mutex;
};

class CASLock : public Noncopyable {
 public:
  using Lock = ScopedLockImpl<CASLock>;
  CASLock() { m_mutex.clear(); }
  ~CASLock() = default;
  void lock() {
    while (std::atomic_flag_test_and_set_explicit(&m_mutex,
                                                  std::memory_order_acquire)) {
      ;
    }
  }
  void unlock() {
    std::atomic_flag_clear_explicit(&m_mutex, std::memory_order_release);
  }

 private:
  volatile std::atomic_flag m_mutex{};
};
}  // namespace hx_sylar
#endif
