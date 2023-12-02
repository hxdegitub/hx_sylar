#include <iostream>
#include <thread>

#include "../hx_sylar/log.h"
#include "../hx_sylar/util.h"
int main() {
  hx_sylar::Logger::ptr logger(new hx_sylar::Logger);
  logger->addAppender(
      hx_sylar::LogAppender::ptr(new hx_sylar::StdoutLogAppender));

  hx_sylar::FileLogAppender::ptr file_appender(
      new hx_sylar::FileLogAppender("log.txt"));
  hx_sylar::LogFormatter::ptr fmt(new hx_sylar::LogFormatter("%d%T%p%T%m%n"));

  file_appender->setFormatter(fmt);
  file_appender->setLevel(hx_sylar::LogLevel::ERROR);

  logger->addAppender(file_appender);

  std::cout << "hello wold \n";
  HX_LOG_INFO(logger) << "test info";
  HX_LOG_FMT_ERROR(logger, "test macro fmt error %s ", "aaa");

  auto l = hx_sylar::LoggerMgr::GetInstance()->getLogger("xx");

  HX_LOG_INFO(l) << " test info 111";
  return 0;
}