#include "../hx_sylar/hx_sylar.h"
#include "../hx_sylar/scheduler.h"

static hx_sylar::Logger::ptr g_logger = HX_LOG_NAME("system");

int main() {
  hx_sylar::Scheduler sc;
  sc.start();
  sc.stop();
  return 0;
}
