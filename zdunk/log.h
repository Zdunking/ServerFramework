#pragma once

#include <string>
#include <stdio.h>
#include <memory>
#include <list>
#include <sstream>
#include <fstream>
#include <vector>
#include <stdint.h>
#include <map>
#include "utils.h"
#include "singleton.h"

#define LOG_LEVEL(logger, level)                                                                              \
    if (logger->getlevel() <= level)                                                                          \
    zdunk::LogEventWrap(zdunk::LogEvent::ptr(new zdunk::LogEvent(logger, level,                               \
                                                                 __FILE__, __LINE__, 0, zdunk::GetThreadId(), \
                                                                 zdunk::GetFiberID(), time(0))))              \
        .getSS()

#define LOG_DEBUG(logger) LOG_LEVEL(logger, zdunk::LogLevel::Level::DEBUG)
#define LOG_INFO(logger) LOG_LEVEL(logger, zdunk::LogLevel::Level::INFO)
#define LOG_ERROR(logger) LOG_LEVEL(logger, zdunk::LogLevel::Level::ERROR)
#define LOG_WARN(logger) LOG_LEVEL(logger, zdunk::LogLevel::Level::WARN)
#define LOG_FATAL(logger) LOG_LEVEL(logger, zdunk::LogLevel::Level::FATAL)

#define LOG_FMT_LEVEL(logger, level, fmt, ...)                                                                \
    if (logger->getlevel() <= level)                                                                          \
    zdunk::LogEventWrap(zdunk::LogEvent::ptr(new zdunk::LogEvent(logger, level,                               \
                                                                 __FILE__, __LINE__, 0, zdunk::GetThreadId(), \
                                                                 zdunk::GetFiberID(), time(0))))              \
        .getEvent()                                                                                           \
        ->format(fmt, __VA_ARGS__)

#define WRITELOG_DEBUG(logger, fmt, ...) LOG_FMT_LEVEL(logger, zdunk::LogLevel::Level::DEBUG, fmt, __VA_ARGS__)
#define WRITELOG_INFO(logger, fmt, ...) LOG_FMT_LEVEL(logger, zdunk::LogLevel::Level::INFO, fmt, __VA_ARGS__)
#define WRITELOG_ERROR(logger, fmt, ...) LOG_FMT_LEVEL(logger, zdunk::LogLevel::Level::ERROR, fmt, __VA_ARGS__)
#define WRITELOG_WARN(logger, fmt, ...) LOG_FMT_LEVEL(logger, zdunk::LogLevel::Level::WARN, fmt, __VA_ARGS__)
#define WRITELOG_FATAL(logger, fmt, ...) LOG_FMT_LEVEL(logger, zdunk::LogLevel::Level::FATAL, fmt, __VA_ARGS__)

#define LOG_ROOT() zdunk::loggerMgr::GetInstance()->getRoot()

namespace zdunk
{

    class Logger;

    // 日志级别
    class LogLevel
    {
    public:
        enum Level
        {
            UNKOWN = 0,
            DEBUG = 1,
            INFO = 2,
            WARN = 3,
            ERROR = 4,
            FATAL = 5
        };
        typedef std::shared_ptr<LogLevel> ptr;
        static const char *ToString(LogLevel::Level level);
    };

    // 日志事件
    class LogEvent
    {
    public:
        LogEvent(std::shared_ptr<Logger> logger,
                 LogLevel::Level level,
                 const char *file,
                 int32_t line,
                 uint32_t elapse,
                 uint32_t threadId,
                 uint32_t fiberId,
                 time_t time);
        typedef std::shared_ptr<LogEvent> ptr;
        const char *getFile() const { return m_file; }
        int32_t getLine() const { return m_line; }
        uint32_t getElapse() const { return m_elapse; }
        uint32_t getThreadID() const { return m_threadId; }
        uint32_t getFiberId() const { return m_fiberId; }
        time_t get_time() const { return m_time; }
        std::string getContent() const { return m_ss.str(); }
        std::shared_ptr<Logger> getLogger() const { return m_logger; }
        LogLevel::Level getLevel() const { return m_level; };

        std::stringstream &getSS() { return m_ss; }
        void format(const char *fmt, ...);
        void format(const char *fmt, va_list al);

    private:
        const char *m_file = nullptr; // 文件名
        int32_t m_line = 0;           // 行号
        uint32_t m_elapse = 0;        // 程序启动到现在的毫秒数
        uint32_t m_threadId = 0;      // 线程ID
        uint32_t m_fiberId = 0;       // 协程号
        time_t m_time;                // 时间戳
        std::stringstream m_ss;

        std::shared_ptr<Logger> m_logger;
        LogLevel::Level m_level;
    };

    class LogEventWrap
    {
    public:
        LogEventWrap(LogEvent::ptr e);
        ~LogEventWrap();

        std::stringstream &getSS();
        LogEvent::ptr &getEvent() { return m_event; }

    private:
        LogEvent::ptr m_event;
    };

    // 日志格式器
    class LogFormatter
    {
    public:
        typedef std::shared_ptr<LogFormatter> ptr;
        LogFormatter(std::string pattern) : m_pattern(pattern)
        {
            init();
        }

        std::string format(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event);

    public:
        class FormatItem
        {
        public:
            typedef std::shared_ptr<FormatItem> ptr;
            FormatItem(const std::string &fmt = "") {}
            virtual ~FormatItem(){};
            virtual void format(std::ostream &os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) = 0;
        };

        void init();

    private:
        std::vector<FormatItem::ptr> m_items;
        std::string m_pattern;
    };

    // 日志输出地
    class LogAppender
    {
    public:
        typedef std::shared_ptr<LogAppender> ptr;
        virtual ~LogAppender(){};

        virtual void log(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) = 0;

        void setFomatter(LogFormatter::ptr val) { m_formatter = val; }
        LogFormatter::ptr getFormatter() const { return m_formatter; }

        void setLevel(LogLevel::Level level) { m_level = level; }
        LogLevel::Level getLevel() { return m_level; }

    protected:
        LogLevel::Level m_level = LogLevel::Level::DEBUG;
        LogFormatter::ptr m_formatter;
    };

    // 日志器
    class Logger : public std::enable_shared_from_this<Logger>
    {
    public:
        typedef std::shared_ptr<Logger> ptr;
        Logger(const std::string &name = "root");
        void log(LogLevel::Level level, LogEvent::ptr event);

        // void debug(LogEvent::ptr event);
        // void info(LogEvent::ptr event);
        // void warn(LogEvent::ptr event);
        // void error(LogEvent::ptr event);
        // void fatal(LogEvent::ptr event);

        const std::string getName() { return m_name; }

        void addAppender(LogAppender::ptr ptr);
        void delAppender(LogAppender::ptr ptr);

        LogLevel::Level getlevel()
        {
            return m_level;
        }
        void setlevel(LogLevel::Level val) { m_level = val; };

    private:
        std::string m_name;                      // 日志名称
        LogLevel::Level m_level;                 // 日志级别
        std::list<LogAppender::ptr> m_appenders; // appender是一个列表
        LogFormatter::ptr m_formatter;
    };

    // 控制台输出
    class StdoutLogAppender : public LogAppender
    {
    public:
        typedef std::shared_ptr<StdoutLogAppender> ptr;
        void log(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override;
    };

    // 输出到文件
    class FileLogAppender : public LogAppender
    {
    public:
        typedef std::shared_ptr<FileLogAppender> ptr;
        FileLogAppender(const std::string &filename);
        void log(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override;
        void reopen();

    private:
        std::string m_filename;
        std::ofstream m_filestream;
    };

    class LoggerManager
    {
    public:
        LoggerManager();
        Logger::ptr getLogger(const std::string &name);
        Logger::ptr getRoot() { return m_root; }
        void init();

    private:
        std::map<std::string, Logger::ptr> m_loggers;
        Logger::ptr m_root;
    };

    typedef zdunk::Singleton<LoggerManager> loggerMgr;
}
