#include "../zdunk/pch.h"
#include <cassert>

zdunk::Logger::ptr g_logger = LOG_ROOT();
#define LOG_TRACE LOG_INFO(g_logger)

void test_assert()
{
    LOG_TRACE << zdunk::BacktraceToString(10);
    // assert(0);
    ZDUNK_ASSERT2(0 == 1, "test");
}

int main()
{
    test_assert();
    // std::cout << "I'm here!" << std::endl;
    return 0;
}