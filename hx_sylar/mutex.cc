#include "mutex.h"

#include <stdexcept>

namespace hx_sylar {
Semaphore::Semaphore(uint32_t count) {
  if (sem_init(&m_semaphore, 0, count) != 0) {
    throw std::logic_error("sem_inti error");
  }
}
Semaphore::~Semaphore() { sem_destroy(&m_semaphore); }
void Semaphore::wait() {
  while (true) {
    if (sem_wait(&m_semaphore) == 0) {
      return;
    }
  }
}

void Semaphore::notify() {
  if (sem_post(&m_semaphore) != 0) {
    throw std::logic_error("sem_post error");
  }
}

}  // namespace hx_sylar