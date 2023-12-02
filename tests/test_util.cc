#include <assert.h>

#include "../hx_sylar/hx_sylar.h"

hx_sylar::Logger::ptr g_logger = HX_LOG_ROOT();

void test_assert() {
  HX_LOG_INFO(g_logger) << hx_sylar::BacktraceToString(10, 2, "**");
  // assert(0);
  // HX_ASSERT(false);
  HX_ASSERT1(false, "abcdefg");
}
int main() {
  test_assert();
  return 0;
}