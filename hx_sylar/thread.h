#ifndef __HX_THREAD_H__
#define __HX_THREAD_H__

#include <semaphore.h>
#include <unistd.h>

#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

namespace hx_sylar {

class Semaphore {
 public:
  Semaphore(uint32_t count = 0);
  ~Semaphore();
  void wait();
  void notify();

 private:
  Semaphore(const Semaphore&) = delete;
  Semaphore(const Semaphore&&) = delete;
  Semaphore operator=(const Semaphore) = delete;

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

class Mutex {
 public:
  typedef ScopedLockImpl<Mutex> Lock;
  Mutex() { pthread_mutex_init(&m_mutex, nullptr); }
  ~Mutex() { pthread_mutex_destroy(&m_mutex); }
  void lock() { pthread_mutex_lock(&m_mutex); }
  void unlock() { pthread_mutex_unlock(&m_mutex); }

 private:
  pthread_mutex_t m_mutex;
};

template <class T>
struct ReadScopedLockImpl {
 public:
  ReadScopedLockImpl(T& mutex) : m_mutex(mutex) {
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
struct WriteScopedLockImpl {
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
class RWMutex {  // read write lock
 public:
  typedef ReadScopedLockImpl<RWMutex> ReadLock;
  typedef WriteScopedLockImpl<RWMutex> WriteLock;
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

class NullMutex {
  typedef ScopedLockImpl<Mutex> Lock;
  NullMutex() {}
  ~NullMutex() {}
  void lock() {}
  void unlock() {}
};

class NullRWMutex {
  typedef ReadScopedLockImpl<NullMutex> ReadLock;
  typedef WriteScopedLockImpl<NullMutex> WriteLock;
};

class Thread {
 public:
  typedef std::shared_ptr<Thread> ptr;
  Thread(std::function<void()> cb, const std::string& name);

  ~Thread();

  pid_t getId() const { return m_id; }

  const std::string getName() const { return m_name; }

  void join();

  static Thread* GetThis();
  static const std::string& GetName();
  static void SetName(const std::string& name);

 private:
  Thread(const Thread&) = delete;
  Thread(const Thread&&) = delete;
  Thread& operator=(const Thread&) = delete;
  static void* run(void* arg);

 private:
  pid_t m_id = -1;
  pthread_t m_thread = 0;
  std::function<void()> m_cb;
  std::string m_name;

  Semaphore m_semaphore;
};

}  // namespace hx_sylar
#endif