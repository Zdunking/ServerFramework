#include "utils.h"

namespace zdunk
{
    pid_t GetThreadId()
    {
        return syscall(SYS_gettid);
    }

    uint32_t GetFiberID()
    {
        return 1;
    }
}