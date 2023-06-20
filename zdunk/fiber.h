#pragma once
#include <memory>
#include <ucontext.h>
#include <functional>
#include "thread.h"

namespace zdunk
{
    class Scheduler;
    class Fiber : public std::enable_shared_from_this<Fiber>
    {
        friend class Scheduler;

    public:
        typedef std::shared_ptr<Fiber> ptr;
        enum State
        {
            INIT,
            READY,
            EXEC,
            HOLD,
            TERM,
            EXCEPT
        };

    public:
        Fiber(std::function<void()> cb, size_t stackszie = 0, bool use_caller = false);
        ~Fiber();

        // 重置协程状态，并重置状态
        void reset(std::function<void()> cb);
        // 切换到当前协程执行
        void swapIn();
        // 切换到后台执行
        void swapOut();

        // 将当前线程切换到执行状态
        void call();

        void back();

        uint64_t getid() const { return m_id; }

        State getState() { return m_state; }

    public:
        // 返回当前协程
        static Fiber::ptr GetThis();
        // 设置当前协程
        static void SetThis(Fiber *f);
        // 协程切换到后台，并设置为Ready状态
        static void YieldToReady();
        // 协程切换到后台，并设置为Hold状态
        static void YieldToHold();
        // 总协程数
        static uint64_t ToTalFiber();

        static void MainFunc();
        static void CallerMainFunc();
        static uint64_t GetID();

    private:
        Fiber();

    private:
        uint64_t m_id = 0;
        uint32_t m_stackSize = 0;
        State m_state = INIT;

        ucontext_t m_ctx;
        void *m_stack = nullptr;

        std::function<void()> m_cb;
    };

} // namespace zdunk
