#include <atomic>
#include "log.h"
#include "config.h"
#include "macro.h"
#include "fiber.h"
#include "scheduler.h"

namespace zdunk
{
    static Logger::ptr g_logger = LOG_NAME("system");
    static std::atomic<uint64_t> s_fiber_id{0};
    static std::atomic<uint64_t> s_fiber_count{0};

    static thread_local Fiber *t_fiber = nullptr;
    static thread_local Fiber::ptr t_threadFiber = nullptr;

    static ConfigVar<uint32_t>::ptr g_fiber_stack_size = Config::Lookup<uint32_t>("fiber.stack_size", 1024 * 1024, "fiber stack size");

    class MallocStackAllocator
    {
    public:
        static void *Alloc(size_t size)
        {
            return malloc(size);
        }

        static void Dealloc(void *vp, size_t size)
        {
            return free(vp);
        }
    };

    using StackAllocator = MallocStackAllocator;

    Fiber::Fiber()
    {
        m_state = EXEC;
        SetThis(this);

        if (getcontext(&m_ctx))
        {
            ZDUNK_ASSERT2(false, "getcontext error");
        }

        ++s_fiber_count;

        LOG_DEBUG(g_logger) << "Fiber::Fiber";
    }

    Fiber::Fiber(std::function<void()> cb, size_t stackszie /*= 0*/, bool use_caller /*= false*/) : m_id(++s_fiber_id), m_cb(cb)
    {
        ++s_fiber_count;
        m_stackSize = stackszie ? stackszie : g_fiber_stack_size->getValue();
        m_stack = StackAllocator::Alloc(m_stackSize);
        if (getcontext(&m_ctx))
        {
            ZDUNK_ASSERT2(false, "getcontext error");
        }
        m_ctx.uc_link = nullptr;
        m_ctx.uc_stack.ss_sp = m_stack;
        m_ctx.uc_stack.ss_size = m_stackSize;

        if (!use_caller)
        {
            makecontext(&m_ctx, &Fiber::MainFunc, 0);
        }
        else
        {
            makecontext(&m_ctx, &Fiber::CallerMainFunc, 0);
        }

        LOG_DEBUG(g_logger) << "Fiber::Fiber id = " << m_id;
    }

    Fiber::~Fiber()
    {
        --s_fiber_count;
        if (m_stack)
        {
            ZDUNK_ASSERT(m_state == TERM || m_state == INIT || m_state == EXCEPT);
            StackAllocator::Dealloc(m_stack, m_stackSize);
        }
        else
        {
            ZDUNK_ASSERT(!m_cb);
            ZDUNK_ASSERT(m_state == EXEC);

            Fiber *cur = t_fiber;
            if (cur == this)
            {
                SetThis(nullptr);
            }
        }

        LOG_DEBUG(g_logger) << "Fiber::~Fiber id = " << m_id;
    }

    void Fiber::reset(std::function<void()> cb)
    {
        ZDUNK_ASSERT(m_stack);
        ZDUNK_ASSERT(m_state == INIT || m_state == TERM || m_state == EXCEPT);
        m_cb = cb;
        if (getcontext(&m_ctx))
        {
            ZDUNK_ASSERT2(false, "getcontext error");
        }

        m_ctx.uc_link = nullptr;
        m_ctx.uc_stack.ss_sp = m_stack;
        m_ctx.uc_stack.ss_size = m_stackSize;

        makecontext(&m_ctx, &Fiber::MainFunc, 0);
        m_state = INIT;
    }

    void Fiber::call()
    {
        SetThis(this);
        m_state = EXEC;
        if (swapcontext(&t_threadFiber->m_ctx, &m_ctx))
        {
            ZDUNK_ASSERT2(false, "swapcontext");
        }
    }

    void Fiber::back()
    {
        SetThis(t_threadFiber.get());
        // m_state = HOLD;
        if (swapcontext(&m_ctx, &t_threadFiber->m_ctx))
        {
            ZDUNK_ASSERT2(false, "swapecontext error");
        }
    }

    void Fiber::swapIn()
    {
        SetThis(this);
        ZDUNK_ASSERT(m_state != EXEC);
        m_state = EXEC;

        if (swapcontext(&Scheduler::GetMainFiber()->m_ctx, &m_ctx))
        // if (swapcontext(&t_threadFiber->m_ctx, &m_ctx))
        {
            ZDUNK_ASSERT2(false, "swapecontext error");
        }
    }

    void Fiber::swapOut()
    {
        SetThis(Scheduler::GetMainFiber());
        // m_state = HOLD;
        if (swapcontext(&m_ctx, &Scheduler::GetMainFiber()->m_ctx))
        // if (swapcontext(&m_ctx, &t_threadFiber->m_ctx))
        {
            ZDUNK_ASSERT2(false, "swapecontext error");
        }
    }

    Fiber::ptr Fiber::GetThis()
    {
        if (t_fiber)
        {
            return t_fiber->shared_from_this();
        }
        Fiber::ptr main_fiber(new Fiber);
        ZDUNK_ASSERT(t_fiber == main_fiber.get());
        t_threadFiber = main_fiber;
        return t_fiber->shared_from_this();
    }

    void Fiber::SetThis(Fiber *f)
    {
        t_fiber = f;
    }

    void Fiber::YieldToReady()
    {
        Fiber::ptr cur = GetThis();
        cur->m_state = READY;
        cur->swapOut();
    }

    void Fiber::YieldToHold()
    {
        Fiber::ptr cur = GetThis();
        ZDUNK_ASSERT(cur->getState() == EXEC);
        // cur->m_state = HOLD;
        cur->swapOut();
    }

    uint64_t Fiber::ToTalFiber()
    {
        return s_fiber_count;
    }

    void Fiber::MainFunc()
    {
        Fiber::ptr cur = GetThis();
        ZDUNK_ASSERT(cur);
        try
        {
            cur->m_cb();
            cur->m_cb = nullptr;
            cur->m_state = TERM;
        }
        catch (std::exception &ex)
        {
            cur->m_state = EXCEPT;
            LOG_ERROR(g_logger) << "Fiber Exception: " << ex.what();
        }
        catch (...)
        {
            cur->m_state = EXCEPT;
            LOG_ERROR(g_logger) << "Fiber Exception: ";
        }
        // cur->swapOut();

        auto raw_ptr = cur.get();
        cur.reset();
        raw_ptr->swapOut();

        ZDUNK_ASSERT2(false, "never reach");
    }

    void Fiber::CallerMainFunc()
    {
        Fiber::ptr cur = GetThis();
        ZDUNK_ASSERT(cur);
        try
        {
            cur->m_cb();
            cur->m_cb = nullptr;
            cur->m_state = TERM;
        }
        catch (std::exception &ex)
        {
            cur->m_state = EXCEPT;
            LOG_ERROR(g_logger) << "Fiber Exception: " << ex.what();
        }
        catch (...)
        {
            cur->m_state = EXCEPT;
            LOG_ERROR(g_logger) << "Fiber Exception: ";
        }
        // cur->swapOut();

        auto raw_ptr = cur.get();
        cur.reset();
        raw_ptr->back();

        ZDUNK_ASSERT2(false, "never reach");
    }

    uint64_t Fiber::GetID()
    {
        if (t_fiber)
        {
            return t_fiber->getid();
        }
        return 0;
    }
}