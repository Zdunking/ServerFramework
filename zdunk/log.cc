#include "log.h"
#include <iostream>
#include <map>
#include <functional>
#include <time.h>
#include <string.h>
#include <stdarg.h>

namespace zdunk
{
    //%m -- 消息体
    //%p -- level
    //%r -- 启动后的时间
    //%c -- 日志名称
    //%t -- 线程ID
    //%n -- 回车换行
    //%d -- 时间
    //%f -- 文件名
    //%l -- 行号
    class MessageFormateItem : public LogFormatter::FormatItem
    {
    public:
        MessageFormateItem(const std::string &str = "") {}
        void format(std::ostream &os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event)
        {
            os << event->getContent();
        }
    };

    class LevelFormatItem : public LogFormatter::FormatItem
    {
    public:
        LevelFormatItem(const std::string &str = "") {}
        void format(std::ostream &os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event)
        {
            os << LogLevel::ToString(level);
        }
    };

    class ElapseFormatItem : public LogFormatter::FormatItem
    {
    public:
        ElapseFormatItem(const std::string &str = "") {}
        void format(std::ostream &os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event)
        {
            os << event->getElapse();
        }
    };

    class NameFormatItem : public LogFormatter::FormatItem
    {
    public:
        NameFormatItem(const std::string &str = "") {}
        void format(std::ostream &os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event)
        {
            os << logger->getName();
        }
    };

    class StringFormatItem : public LogFormatter::FormatItem
    {
    public:
        StringFormatItem(const std::string &str) : m_string(str){};
        void format(std::ostream &os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event)
        {
            os << m_string;
        }

    private:
        std::string m_string;
    };

    class ThreadIdFormatItem : public LogFormatter::FormatItem
    {
    public:
        ThreadIdFormatItem(const std::string &str = "") {}
        void format(std::ostream &os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event)
        {
            os << event->getThreadID();
        }
    };

    class FiberIdFormatItem : public LogFormatter::FormatItem
    {
    public:
        FiberIdFormatItem(const std::string &str = "") {}
        void format(std::ostream &os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event)
        {
            os << event->getFiberId();
        }
    };

    class DataTimeFormatItem : public LogFormatter::FormatItem
    {
    public:
        DataTimeFormatItem(const std::string format = "%Y-%m-%d %H:%M:%S") : m_format(format)
        {
            if (m_format.empty())
                m_format = "%Y-%m-%d %H:%M:%S";
        };
        void format(std::ostream &os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event)
        {
            struct tm tm;
            time_t time = event->get_time();
            localtime_r(&time, &tm);
            char buf[64] = "";
            strftime(buf, sizeof(buf), m_format.c_str(), &tm);
            os << buf;
        }

    private:
        std::string m_format;
    };

    class FilenameFormatItem : public LogFormatter::FormatItem
    {
    public:
        FilenameFormatItem(const std::string &str = "") {}
        void format(std::ostream &os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event)
        {
            os << event->getFile();
        }
    };

    class LineFormatItem : public LogFormatter::FormatItem
    {
    public:
        LineFormatItem(const std::string &str = "") {}
        void format(std::ostream &os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event)
        {
            os << event->getLine();
        }
    };

    class NewLineFormatItem : public LogFormatter::FormatItem
    {
    public:
        NewLineFormatItem(const std::string &str = "") {}
        void format(std::ostream &os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event)
        {
            os << std::endl;
        }
    };

    class TabFormatItem : public LogFormatter::FormatItem
    {
    public:
        TabFormatItem(const std::string &str = "") {}
        void format(std::ostream &os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event)
        {
            os << "\t";
        }
    };

    const char *LogLevel::ToString(LogLevel::Level level)
    {
        switch (level)
        {
#define XX(name)         \
    case LogLevel::name: \
        return #name;    \
        break;

            XX(DEBUG);
            XX(INFO);
            XX(WARN);
            XX(ERROR);
            XX(FATAL);
#undef XX

        default:
            return "UNKOWN";
        }
        return "UNKOWN";
    }

    LogEventWrap::LogEventWrap(LogEvent::ptr e) : m_event(e)
    {
    }

    LogEventWrap::~LogEventWrap()
    {
        m_event->getLogger()->log(m_event->getLevel(), m_event);
    }

    std::stringstream &LogEventWrap::getSS()
    {
        return m_event->getSS();
    }

    LogEvent::LogEvent(std::shared_ptr<Logger> logger,
                       LogLevel::Level level,
                       const char *file,
                       int32_t line,
                       uint32_t elapse,
                       uint32_t threadId,
                       uint32_t fiberId,
                       time_t time) : m_file(file),
                                      m_line(line),
                                      m_elapse(elapse),
                                      m_threadId(threadId),
                                      m_fiberId(fiberId),
                                      m_time(time),
                                      m_logger(logger),
                                      m_level(level)
    {
    }

    void LogEvent::format(const char *fmt, ...)
    {
        va_list al;
        va_start(al, fmt);
        format(fmt, al);
        va_end(al);
    }
    void LogEvent::format(const char *fmt, va_list al)
    {
        char *buf = nullptr;
        int len = vasprintf(&buf, fmt, al);
        if (len != -1)
        {
            m_ss << std::string(buf, len);
            free(buf);
        }
    }

    // 日志器的实现
    Logger::Logger(const std::string &name) : m_name(name), m_level(LogLevel::Level::DEBUG)
    {
        m_formatter.reset(new LogFormatter("%d%T%t%T%F%T[%p]%T[%c]%T[%f:%l]%T%m  %n"));
    }

    void Logger::addAppender(LogAppender::ptr appender)
    {
        if (!appender->getFormatter())
        {
            appender->setFomatter(m_formatter);
        }
        m_appenders.push_back(appender);
    }

    void Logger::delAppender(LogAppender::ptr appender)
    {
        for (auto it = m_appenders.begin(); it != m_appenders.end(); it++)
        {
            if (*it == appender)
            {
                m_appenders.erase(it);
                break;
            }
        }
    }

    void Logger::log(LogLevel::Level level, LogEvent::ptr event)
    {
        if (level >= m_level)
        {
            auto self = shared_from_this();
            for (auto i : m_appenders)
            {
                i->log(self, level, event);
            }
        }
    }

    // void Logger::debug(LogEvent::ptr event)
    // {
    //     log(LogLevel::DEBUG, event);
    // }
    // void Logger::info(LogEvent::ptr event)
    // {

    //     log(LogLevel::INFO, event);
    // }
    // void Logger::warn(LogEvent::ptr event)
    // {

    //     log(LogLevel::WARN, event);
    // }
    // void Logger::error(LogEvent::ptr event)
    // {
    //     log(LogLevel::ERROR, event);
    // }

    // void Logger::fatal(LogEvent::ptr event)
    // {
    //     log(LogLevel::FATAL, event);
    // }

    // Appender的相关实现
    FileLogAppender::FileLogAppender(const std::string &filename)
    {
        m_filename = filename;
        reopen();
    }

    void FileLogAppender::log(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event)
    {
        if (level >= m_level)
        {
            m_filestream << m_formatter->format(logger, level, event);
        }
    }

    void FileLogAppender::reopen()
    {
        if (m_filestream)
        {
            m_filestream.close();
        }
        m_filestream.open(m_filename, std::fstream::in | std::fstream::out | std::fstream::app);
        // return !!m_name;
    }
    void StdoutLogAppender::log(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event)
    {
        if (level >= m_level)
        {
            std::cout << m_formatter->format(logger, level, event);
        }
    }

    std::string LogFormatter::format(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event)
    {
        std::stringstream ss;
        for (auto &i : m_items)
        {
            i->format(ss, logger, level, event);
        }
        return ss.str();
    }

    //%xxx %xxx{xxx} %%
    void LogFormatter::init()
    {
        std::vector<std::tuple<std::string, std::string, int>> vec;
        std::string nstr;
        for (size_t i = 0; i < m_pattern.size(); ++i)
        {
            if (m_pattern[i] != '%')
            {
                nstr.append(1, m_pattern[i]);
                continue;
            }
            if ((i + 1) < m_pattern.size())
            {
                if (m_pattern[i + 1] == '%')
                {
                    nstr.append(1, '%');
                    continue;
                }
            }

            size_t n = i + 1;
            int fmt_status = 0;
            size_t fmt_begin = n;

            std::string str;
            std::string fmt;

            while (n < m_pattern.size())
            {
                // 解析{}里面的内容
                if (!isalpha(m_pattern[n]) && m_pattern[n] != '{' && m_pattern[n] != '}')
                {
                    break;
                }
                if (fmt_status == 0)
                {
                    if (m_pattern[n] == '{')
                    {
                        //"abcdefg" substr(i+1, n-i-1)
                        // if i == 2 substr(3, 4)
                        // str = m_pattern.substr(i + 1, n - i - 1);
                        fmt_status = 1;
                        fmt_begin = n;
                        ++n;
                        continue;
                    }
                }
                if (fmt_status == 1)
                {
                    if (m_pattern[n] == '}')
                    {
                        str = m_pattern.substr(fmt_begin + 1, n - fmt_begin - 1);
                        fmt_status = 2;
                        break;
                    }
                }
                ++n;
            }

            // 开始解析模式字符串
            if (fmt_status == 0)
            {
                if (!nstr.empty())
                {
                    vec.push_back(make_tuple(nstr, std::string(), 0));
                    nstr.clear();
                }
                str = m_pattern.substr(i + 1, n - i - 1);
                vec.push_back(std::make_tuple(str, fmt, 1));
                i = n - 1;
            }
            else if (fmt_status == 1)
            {
                // std::cout << "pattern prase error:" << m_pattern << "-" << m_pattern.substr(i) << std::endl;
                vec.push_back(std::make_tuple("<<pattern prase error>>", fmt, 1));
            }
            else if (fmt_status == 2)
            {
                if (!nstr.empty())
                {
                    vec.push_back(std::make_tuple(nstr, "", 0));
                    nstr.clear();
                }
                vec.push_back(std::make_tuple(str, fmt, 1));
                i = n - 1;
            }
        }
        if (!nstr.empty())
        {
            vec.push_back(std::make_tuple(nstr, "", 0));
        }

        static std::map<std::string, std::function<FormatItem::ptr(const std::string &str)>> s_format_items{
#define XX(str, C)                                                               \
    {                                                                            \
#str, [](const std::string &fmt) { return FormatItem::ptr(new C(fmt)); } \
    }

            XX(m, MessageFormateItem),
            XX(p, LevelFormatItem),
            XX(r, ElapseFormatItem),
            XX(c, NameFormatItem),
            XX(t, ThreadIdFormatItem),
            XX(n, NewLineFormatItem),
            XX(d, DataTimeFormatItem),
            XX(f, FilenameFormatItem),
            XX(l, LineFormatItem),
            XX(T, TabFormatItem),
            XX(F, FiberIdFormatItem),

#undef XX
        };

        for (auto &i : vec)
        {
            // get<2>(i)如果是0，说明get<0>(i)是普通字符串
            if (std::get<2>(i) == 0)
            {
                m_items.push_back(FormatItem::ptr(new StringFormatItem(std::get<0>(i))));
            }
            else
            {
                // get<2>(i)如果是1，说明get<0>(i)是模式字符
                auto it = s_format_items.find(std::get<0>(i));
                if (it == s_format_items.end())
                {
                    m_items.push_back(FormatItem::ptr(new StringFormatItem("<<error_format %" + std::get<0>(i) + ">>")));
                }
                else
                {
                    m_items.push_back(it->second(std::get<1>(i)));
                }
            }
            // std::cout << std::get<0>(i) << "-" << std::get<1>(i) << "-" << std::get<2>(i) << std::endl;
        }
    }

    LoggerManager::LoggerManager()
    {
        m_root.reset(new Logger);
        m_root->addAppender(LogAppender::ptr(new StdoutLogAppender));
    }

    void LoggerManager::init()
    {
    }

    Logger::ptr LoggerManager::getLogger(const std::string &name)
    {
        auto it = m_loggers.find(name);
        if (it != m_loggers.end())
        {
            return it->second;
        }
        return m_root;
    }

}
