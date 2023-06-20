#pragma once

#include <string>
#include <cassert>
#include "utils.h"

#if defined __GNUC__ || defined __llvm__
/// LIKCLY 宏的封装, 告诉编译器优化,条件大概率成立
#define ZDUNK_LIKELY(x) __builtin_expect(!!(x), 1)
/// LIKCLY 宏的封装, 告诉编译器优化,条件大概率不成立
#define ZDUNK_UNLIKELY(x) __builtin_expect(!!(x), 0)
#else
#define ZDUNK_LIKELY(x) (x)
#define ZDUNK_UNLIKELY(x) (x)
#endif

#define ZDUNK_ASSERT(x)                                                    \
    if (!(x))                                                              \
    {                                                                      \
        LOG_ERROR(LOG_ROOT()) << "ASSERTION: " #x                          \
                              << "\nbacktrace:\n"                          \
                              << zdunk::BacktraceToString(100, 2, "    "); \
        assert(x);                                                         \
    }

#define ZDUNK_ASSERT2(x, w)                                                \
    if (!(x))                                                              \
    {                                                                      \
        LOG_ERROR(LOG_ROOT()) << "ASSERTION: " #x                          \
                              << "\nNOTICE : " << #w                       \
                              << "\nbacktrace:\n"                          \
                              << zdunk::BacktraceToString(100, 2, "    "); \
        assert(x);                                                         \
    }

namespace zdunk
{

}