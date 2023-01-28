#pragma once
#include <stdint.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <unistd.h>

namespace zdunk
{
    pid_t GetThreadId();
    uint32_t GetFiberID();
}