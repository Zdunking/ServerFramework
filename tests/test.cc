#include "../zdunk/log.h"
#include "../zdunk/utils.h"
#include <iostream>

int main()
{
    zdunk::Logger::ptr logger(new zdunk::Logger);

    zdunk::LogAppender::ptr std_append(new zdunk::StdoutLogAppender);
    std_append->setLevel(zdunk::LogLevel::Level::ERROR);
    logger->addAppender(std_append);

    zdunk::FileLogAppender::ptr file_append(new zdunk::FileLogAppender("./test.log"));
    zdunk::LogFormatter::ptr fmt(new zdunk::LogFormatter("%d%T%p%T%m%n"));
    file_append->setFomatter(fmt);
    file_append->setLevel(zdunk::LogLevel::Level::DEBUG);

    logger->addAppender(file_append);

    // zdunk::LogEvent::ptr event(new zdunk::LogEvent(__FILE__, __LINE__, 0, zdunk::GetThreadId(), zdunk::GetFiberID(), time(0))); // LogEvent被默认初始化，但是m_content和ss没有被初始化
    // event->getSS() << "hello zdunk log[1]";
    // logger->log(zdunk::LogLevel::DEBUG, event);

    LOG_INFO(logger) << "test macro!!";
    LOG_INFO(logger) << "test macro info!!";

    WRITELOG_ERROR(logger, "test macro fmt error %s", " success!");

    auto i = zdunk::loggerMgr::GetInstance()->getLogger("xx");
    LOG_INFO(i) << "xxx";
    return 0;
}
