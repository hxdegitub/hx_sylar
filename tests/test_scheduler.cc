#include "../hx_sylar/hx_sylar.h"
#include "../hx_sylar/scheduler.h"

static hx_sylar::Logger::ptr g_logger = HX_LOG_NAME("system");

void TestFiber() {
  HX_LOG_INFO(g_logger) << "test in fiber ";
  sleep(1);
  static int s_count = 3;
  while (s_count-- >= 0) {
    hx_sylar::Scheduler::GetThis()->schedule(&TestFiber);
  }
}
auto main() -> int {
  HX_LOG_INFO(g_logger) << " main start ";
  hx_sylar::Scheduler sc(3, true, "test");
  sc.start();
  sleep(2);
  HX_LOG_INFO(g_logger) << "scheduler ";
  sc.schedule(&TestFiber);
  sc.stop();
  return 0;
}
