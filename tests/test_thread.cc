#include "../hx_sylar/hx_sylar.h"
int count = 0;
hx_sylar::Logger::ptr g_logger = HX_LOG_ROOT();

hx_sylar::RWMutex s_mutex;
void Fun1() {
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
 void Fun2() {
  int i = 1;
  while ((i --) != 0 ) {
    HX_LOG_INFO(g_logger) << "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";
}
}

[[noreturn]] void Fun3() {
  while (true) {
    HX_LOG_INFO(g_logger) << "==============================================";
}
}
auto main(int argc, char **argv) -> int {
  HX_LOG_INFO(g_logger) << "thread starts ";
  YAML::Node root = YAML::LoadFile("/home/hx/hx_sylar/bin/conf/log2.yml");
  hx_sylar::Config::LoadFromYaml(root);
  std::vector<hx_sylar::Thread::ptr> thrs;
  for (int i = 0; i < 1; ++i) {
    hx_sylar::Thread::ptr thr(
        new hx_sylar::Thread(&Fun1, "name_" + std::to_string(i * 2)));
    hx_sylar::Thread::ptr thr2(
        new hx_sylar::Thread(&Fun2, "name_" + std::to_string(i)));

    hx_sylar::Thread::ptr thr3(
        new hx_sylar::Thread(&Fun3, "name_" + std::to_string(i)));
    thrs.push_back(thr);
    thrs.push_back(thr2);
    thrs.push_back(thr3);
  }

  for (size_t i = 0; i < thrs.size(); ++i) {
    thrs[i]->join();
  }

  HX_LOG_INFO(g_logger) << "thread test end ";
  HX_LOG_INFO(g_logger) << "count = " << count;

  return 0;
}
