#include "../zdunk/pch.h"

zdunk::Logger::ptr g_logger = LOG_ROOT();
#define LOG_TRACE LOG_INFO(g_logger)

void run_in_fiber()
{
    LOG_TRACE << "run in fiber begin";
    zdunk::Fiber::YieldToHold();
    LOG_TRACE << "run in fiber end";
    zdunk::Fiber::YieldToHold();
}

void test_fiber()
{
    LOG_TRACE << "main begin -1";
    {
        // zdunk::Thread::SetName("main");
        zdunk::Fiber::GetThis();
        LOG_TRACE << "main fiber begin";
        zdunk::Fiber::ptr fiber(new zdunk::Fiber(run_in_fiber));
        fiber->swapIn();
        LOG_TRACE << "main after swapIn";
        fiber->swapIn();
        LOG_TRACE << "main fiber end";
        fiber->swapIn();
    }
    LOG_TRACE << "main after end2";
}

int main()
{

    zdunk::Thread::SetName("main");
    // std::vector<zdunk::Thread::ptr> thrs;
    // for (int i = 0; i < 3; i++)
    // {
    //     thrs.push_back(zdunk::Thread::ptr(new zdunk::Thread(&test_fiber, "name_" + std::to_string(i))));
    // }
    // for (auto i : thrs)
    // {
    //     i->join();
    // }

    auto it = zdunk::Thread::ptr(new zdunk::Thread(&test_fiber, "name_" + std::to_string(0)));
    it->join();
    return 0;
}
