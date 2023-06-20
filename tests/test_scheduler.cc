#include "../zdunk/pch.h"
#include "../zdunk/scheduler.h"

zdunk::Logger::ptr g_logger = LOG_ROOT();

#define LOG_TRACE LOG_INFO(LOG_ROOT())

void test_scheduler()
{
    LOG_TRACE << "test in fiber";

    static int s_count = 5;
    sleep(1);
    if (--s_count)
    {
        zdunk::Scheduler::GetThis()->schedule(&test_scheduler);
    }
}

void t2()
{
    LOG_TRACE << "111";
}

int main()
{
    LOG_TRACE << "mian";
    zdunk::Scheduler sc(3, true, "test");
    sc.start();
    LOG_TRACE << "schedule";
    sc.schedule(&t2);
    sc.stop();
    LOG_TRACE << "over";
    return 0;
}