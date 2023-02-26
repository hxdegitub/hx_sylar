#include "../hx_sylar/fiber.h"
#include "../hx_sylar/hx_sylar.h"
hx_sylar::Logger::ptr g_logger = HX_LOG_ROOT();

void run_in_fiber() {
  HX_LOG_INFO(g_logger) << "run_in_fiber start";
  hx_sylar::Fiber::YieldToHold();
  HX_LOG_INFO(g_logger) << "run_in_fiber end";
  hx_sylar::Fiber::YieldToHold();
}
void test_fiber() {
  hx_sylar::Fiber::GetThis();
  HX_LOG_INFO(g_logger) << "main  start";
  hx_sylar::Fiber::ptr fiber(new hx_sylar::Fiber(run_in_fiber));
  fiber->swapIn();
  HX_LOG_INFO(g_logger) << "main after swapin ";
  fiber->swapIn();
  HX_LOG_INFO(g_logger) << "main func  end";
}

int main(int argc, char** argv) {
  hx_sylar::Thread::SetName("Main");
  std::vector<hx_sylar::Thread::ptr> thrs;
  for (int i = 0; i < 3; ++i) {
    thrs.push_back(hx_sylar::Thread::ptr(
        new hx_sylar::Thread(&test_fiber, "name_" + std::to_string(i))));
  }

  for (auto i : thrs) {
    i->join();
  }
  return 0;
}
