#include "../hx_sylar/hx_sylar.h"
int count = 0;
hx_sylar::Logger::ptr g_logger = HX_LOG_ROOT();

hx_sylar::RWMutex s_mutex;
void fun1() {
  HX_LOG_INFO(g_logger) << "name : " << hx_sylar::Thread::GetName()
                        << "this name "
                        << hx_sylar::Thread::GetThis()->getName()
                        << "id :" << hx_sylar::GetThreadId() << "this.id "
                        << hx_sylar::Thread::GetThis()->getId();
  for (int i = 0; i < 100000; ++i) {
    hx_sylar::RWMutex::WriteLock lock(s_mutex);
    ++count;
  }
}
void fun2() {}

int main(int argc, char **argv) {
  std::vector<hx_sylar::Thread::ptr> thrs;
  for (int i = 0; i < 5; ++i) {
    hx_sylar::Thread::ptr thr(
        new hx_sylar::Thread(&fun1, "name_" + std::to_string(i)));
    thrs.push_back(thr);
  }

  for (int i = 0; i < 5; ++i) {
    thrs[i]->join();
  }

  HX_LOG_INFO(g_logger) << "thread test end ";
  HX_LOG_INFO(g_logger) << "count = " << count;

  return 0;
}
