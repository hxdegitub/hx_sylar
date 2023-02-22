
#ifndef __HX_LOG_H__
#define __HX_LOG_H__

#include <stdarg.h>
#include <stdint.h>

#include <fstream>
#include <list>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "thread.h"

//#include "config.h"
#include "singleton.h"
#include "thread.h"
#include "util.h"

#define HX_LOG_LEVEL(logger, level)                                      \
  if (logger->getLevel() <= level)                                       \
  hx_sylar::LogEventWrap(                                                \
      hx_sylar::LogEvent::ptr(new hx_sylar::LogEvent(                    \
          logger, level, __FILE__, __LINE__, 0, hx_sylar::GetThreadId(), \
          hx_sylar::GetFiberId(), time(0))))                             \
      .getSS()

#define HX_LOG_DEBUG(logger) HX_LOG_LEVEL(logger, hx_sylar::LogLevel::DEBUG)

#define HX_LOG_INFO(logger) HX_LOG_LEVEL(logger, hx_sylar::LogLevel::INFO)

#define HX_LOG_WARN(logger) HX_LOG_LEVEL(logger, hx_sylar::LogLevel::WARN)

#define HX_LOG_ERROR(logger) HX_LOG_LEVEL(logger, hx_sylar::LogLevel::ERROR)

#define HX_LOG_FATAL(logger) HX_LOG_LEVEL(logger, hx_sylar::LogLevel::FATAL)

#define HX_LOG_FMT_LEVEL(logger, level, fmt, ...)                        \
  if (logger->getLevel() <= level)                                       \
  hx_sylar::LogEventWrap(                                                \
      hx_sylar::LogEvent::ptr(new hx_sylar::LogEvent(                    \
          logger, level, __FILE__, __LINE__, 0, hx_sylar::GetThreadId(), \
          hx_sylar::GetFiberId(), time(0))))                             \
      .getEvent()                                                        \
      ->format(fmt, __VA_ARGS__)

#define HX_LOG_FMT_DEBUG(logger, fmt, ...) \
  HX_LOG_FMT_LEVEL(logger, hx_sylar::LogLevel::DEBUG, fmt, __VA_ARGS__)

#define HX_LOG_FMT_INFO(logger, fmt, ...) \
  HX_LOG_FMT_LEVEL(logger, hx_sylar::LogLevel::INFO, fmt, __VA_ARGS__)

#define HX_LOG_FMT_WARN(logger, fmt, ...) \
  HX_LOG_FMT_LEVEL(logger, hx_sylar::LogLevel::WARN, fmt, __VA_ARGS__)

#define HX_LOG_FMT_ERROR(logger, fmt, ...) \
  HX_LOG_FMT_LEVEL(logger, hx_sylar::LogLevel::ERROR, fmt, __VA_ARGS__)

#define HX_LOG_FMT_FATAL(logger, fmt, ...) \
  HX_LOG_FMT_LEVEL(logger, hx_sylar::LogLevel::FATAL, fmt, __VA_ARGS__)

#define HX_LOG_ROOT() hx_sylar::LoggerMgr::GetInstance()->getRoot()

#define HX_LOG_NAME(name) hx_sylar::LoggerMgr::GetInstance()->getLogger(name)

namespace hx_sylar {

class Logger;
class LoggerManager;

/**
 * @brief 日志级别
 */
class LogLevel {
 public:
  /**
   * @brief 日志级别枚举
   */
  enum Level {
    /// 未知级别
    UNKNOW = 0,
    /// DEBUG 级别
    DEBUG = 1,
    /// INFO 级别
    INFO = 2,
    /// WARN 级别
    WARN = 3,
    /// ERROR 级别
    ERROR = 4,
    /// FATAL 级别
    FATAL = 5
  };

  static const char* ToString(LogLevel::Level level);

  /**
   * @brief 将文本转换成日志级别
   * @param[in] str 日志级别文本
   */
  static LogLevel::Level FromString(const std::string& str);
};

/**
 * @brief 日志事件
 */
class LogEvent {
 public:
  typedef std::shared_ptr<LogEvent> ptr;
  LogEvent();
  LogEvent(std::shared_ptr<Logger> logger, LogLevel::Level level,
           const char* file, int32_t line, uint32_t elapse, uint32_t thread_id,
           uint32_t fiber_id, uint64_t time);

  const char* getFile() const { return m_file; }

  int32_t getLine() const { return m_line; }

  uint32_t getElapse() const { return m_elapse; }

  uint32_t getThreadId() const { return m_threadId; }

  uint32_t getFiberId() const { return m_fiberId; }

  uint64_t getTime() const { return m_time; }

  const std::string& getThreadName() const { return m_threadName; }
  /**
   * @brief 返回日志内容
   */
  std::string getContent() const { return m_ss.str(); }

  /**
   * @brief 返回日志器
   */
  std::shared_ptr<Logger> getLogger() const { return m_logger; }

  /**
   * @brief 返回日志级别
   */
  LogLevel::Level getLevel() const { return m_level; }

  /**
   * @brief 返回日志内容字符串流
   */
  std::stringstream& getSS() { return m_ss; }

  /**
   * @brief 格式化写入日志内容
   */
  void format(const char* fmt, ...);

  /**
   * @brief 格式化写入日志内容
   */
  void format(const char* fmt, va_list al);

 private:
  /// 文件名
  const char* m_file = nullptr;
  /// 行号
  int32_t m_line = 0;
  /// 程序启动开始到现在的毫秒数
  uint32_t m_elapse = 0;
  /// 线程ID
  uint32_t m_threadId = 0;
  /// 协程ID
  uint32_t m_fiberId = 0;
  /// 时间戳
  uint64_t m_time = 0;
  /// 线程名称
  std::string m_threadName;
  /// 日志内容流
  std::stringstream m_ss;
  /// 日志器
  std::shared_ptr<Logger> m_logger;
  /// 日志等级
  LogLevel::Level m_level;
};

/**
 * @brief 日志事件包装器
 */
class LogEventWrap {
 public:
  LogEventWrap(LogEvent::ptr ev);

  ~LogEventWrap();

  LogEvent::ptr getEvent() const { return m_event; }

  std::stringstream& getSS();

 private:
  /**
   * @brief 日志事件
   */
  LogEvent::ptr m_event;
  std::stringstream m_ss;
};

/**
 * @brief 日志格式化
 */
class LogFormatter {
 public:
  typedef std::shared_ptr<LogFormatter> ptr;
  /**
   * @brief 构造函数
   * @param[in] pattern 格式模板
   * @details
   *  %m 消息
   *  %p 日志级别
   *  %r 累计毫秒数
   *  %c 日志名称
   *  %t 线程id
   *  %n 换行
   *  %d 时间
   *  %f 文件名
   *  %l 行号
   *  %T 制表符
   *  %F 协程id
   *  %N 线程名称
   *
   *  默认格式 "%d{%Y-%m-%d %H:%M:%S}%T%t%T%N%T%F%T[%p]%T[%c]%T%f:%l%T%m%n"
   */
  LogFormatter(const std::string& pattern);

  std::string format(std::shared_ptr<Logger> logger, LogLevel::Level level,
                     LogEvent::ptr event);
  std::ostream& format(std::ostream& ofs, std::shared_ptr<Logger> logger,
                       LogLevel::Level level, LogEvent::ptr event);

 public:
  /**
   * @brief 日志内容项格式化
   */
  class FormatItem {
   public:
    typedef std::shared_ptr<FormatItem> ptr;
    /**
     * @brief 析构函数
     */
    virtual ~FormatItem() {}
    virtual void format(std::ostream& os, std::shared_ptr<Logger> logger,
                        LogLevel::Level level, LogEvent::ptr event) = 0;
  };

  void init();

  /**
   * @brief 是否有错误
   */
  bool isError() const { return m_error; }

  /**
   * @brief 返回日志模板
   */
  const std::string getPattern() const { return m_pattern; }

 private:
  /// 日志格式模板
  std::string m_pattern;
  /// 日志格式解析后格式
  std::vector<FormatItem::ptr> m_items;
  /// 是否有错误
  bool m_error = false;
};

/**
 * @brief 日志输出目标
 */
class LogAppender {
  friend class Logger;

 public:
  typedef std::shared_ptr<LogAppender> ptr;
  typedef Mutex MutexType;
  virtual ~LogAppender() {}

  virtual void log(std::shared_ptr<Logger> logger, LogLevel::Level level,
                   LogEvent::ptr event) = 0;

  virtual std::string toYamlString() = 0;

  void setFormatter(LogFormatter::ptr val);

  LogFormatter::ptr getFormatter();

  /**
   * @brief 获取日志级别
   */
  LogLevel::Level getLevel() const { return m_level; }

  /**
   * @brief 设置日志级别
   */
  void setLevel(LogLevel::Level val) { m_level = val; }

 protected:
  LogLevel::Level m_level = LogLevel::DEBUG;
  bool m_hasFormatter = false;
  Mutex m_mutex;

  LogFormatter::ptr m_formatter;
};

/**
 * @brief 日志器
 */
class Logger : public std::enable_shared_from_this<Logger> {
  friend class LoggerManager;

 public:
  typedef std::shared_ptr<Logger> ptr;
  typedef Mutex MutexType;
  Logger(const std::string& name = "root");

  void log(LogLevel::Level level, LogEvent::ptr event);
  void debug(LogEvent::ptr event);
  void info(LogEvent::ptr event);
  void warn(LogEvent::ptr event);
  void error(LogEvent::ptr event);

  void fatal(LogEvent::ptr event);

  void addAppender(LogAppender::ptr appender);
  void delAppender(LogAppender::ptr appender);
  void clearAppenders();

  LogLevel::Level getLevel() const { return m_level; }

  void setLevel(LogLevel::Level val) { m_level = val; }

  const std::string& getName() const { return m_name; }

  void setFormatter(LogFormatter::ptr val);

  void setFormatter(const std::string& val);

  LogFormatter::ptr getFormatter();
  std::string toYamlString();

 private:
  /// 日志名称
  std::string m_name;
  /// 日志级别
  LogLevel::Level m_level;
  hx_sylar::Mutex m_mutex;

  /// 日志目标集合
  std::list<LogAppender::ptr> m_appenders;
  /// 日志格式器
  LogFormatter::ptr m_formatter;
  /// 主日志器
  Logger::ptr m_root;
};

/**
 * @brief 输出到控制台的Appender
 */
class StdoutLogAppender : public LogAppender {
 public:
  typedef std::shared_ptr<StdoutLogAppender> ptr;
  virtual void log(Logger::ptr logger, LogLevel::Level level,
                   LogEvent::ptr event) override;
  std::string toYamlString() override;
};

class FileLogAppender : public LogAppender {
 public:
  typedef std::shared_ptr<FileLogAppender> ptr;
  FileLogAppender(const std::string& filename);
  void log(Logger::ptr logger, LogLevel::Level level,
           LogEvent::ptr event) override;
  std::string toYamlString() override;

  bool reopen();

 private:
  /// 文件路径
  std::string m_filename;
  /// 文件流
  std::ofstream m_filestream;
  /// 上次重新打开时间
  uint64_t m_lastTime = 0;
};

/**
 * @brief 日志器管理类
 */
class LoggerManager {
 public:
  typedef Mutex MutexType;
  LoggerManager();
  Logger::ptr getLogger(const std::string& name);
  void init();
  Logger::ptr getRoot() const { return m_root; }
  std::string toYamlString();

 private:
  MutexType m_mutex;
  std::map<std::string, Logger::ptr> m_loggers;

  Logger::ptr m_root;
};

/// 日志器管理类单例模式

typedef hx_sylar::Singleton<LoggerManager> LoggerMgr;
}  // namespace hx_sylar

#endif