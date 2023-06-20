#pragma once
#include <stdint.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <string>
#include <vector>

namespace zdunk
{
    pid_t GetThreadId();
    uint32_t GetFiberID();

    void BackTrace(std::vector<std::string> &bt, int size, int skip = 1);
    std::string BacktraceToString(int size, int skip = 2, std::string prefix = "");

    uint64_t GetCurrentMS();
    uint64_t GetCurrentUS();
}