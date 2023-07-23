#include "scheduler.h"
#include "log.h"
#include "macro.h"
#include "hook.h"

namespace zdunk
{
    static zdunk::Logger::ptr g_logger = LOG_NAME("system");

    static thread_local Scheduler *t_scheduler = nullptr;
    static thread_local Fiber *t_fiber = nullptr;

    Scheduler::Scheduler(size_t threads /* = 1*/, bool use_caller /* = true*/, const std::string &name /* = ""*/) : m_name(name)
    {
        ZDUNK_ASSERT(threads > 0);
        if (use_caller)
        {
            zdunk::Fiber::GetThis();
            --threads;

            ZDUNK_ASSERT(GetThis() == nullptr);
            t_scheduler = this;

            m_rootFiber.reset(new Fiber(std::bind(&Scheduler::run, this), 0, true));
            zdunk::Thread::SetName(m_name);

            t_fiber = m_rootFiber.get();
            m_rootThread = zdunk::GetThreadId();
            m_threadIds.push_back(m_rootThread);
        }
        else
        {
            m_rootThread = -1;
        }
        m_threadCount = threads;
    }

    Scheduler::~Scheduler()
    {
        ZDUNK_ASSERT(m_stopping);
        if (GetThis() == this)
        {
            t_scheduler = nullptr;
        }
    }

    Scheduler *Scheduler::GetThis()
    {
        return t_scheduler;
    }

    Fiber *Scheduler::GetMainFiber()
    {
        return t_fiber;
    }

    void Scheduler::start()
    {
        MutexType::Lock lock(m_mutex);

        if (!m_stopping)
        {
            return;
        }
        m_stopping = false;
        ZDUNK_ASSERT(m_threads.empty());

        m_threads.resize(m_threadCount);
        for (size_t i = 0; i < m_threadCount; ++i)
        {
            m_threads[i].reset(new Thread(std::bind(&Scheduler::run, this), m_name + "_" + std::to_string(i)));
            m_threadIds.push_back(m_threads[i]->GetId());
        }
        lock.unlock();
        // if (m_rootFiber)
        //{
        //     m_rootFiber->call();
        //     LOG_INFO(g_logger) << "call out " << m_rootFiber->getState();
        // }
    }

    void Scheduler::stop()
    {
        m_autoStop = true;
        if (m_rootFiber && m_threadCount == 0 && (m_rootFiber->getState() == Fiber::TERM || m_rootFiber->getState() == Fiber::INIT))
        {
            LOG_INFO(g_logger) << this << " stopped";
            m_stopping = true;
            if (stopping())
                return;
        }

        // bool exit_on_this_fiber = false;
        if (m_rootThread != -1)
        {
            ZDUNK_ASSERT(GetThis() == this);
            // 插眼
        }
        else
        {
            ZDUNK_ASSERT(GetThis() != this);
        }

        m_stopping = true;
        for (size_t i = 0; i < m_threadCount; ++i)
        {
            tickle();
        }

        if (m_rootFiber)
        {
            tickle();
        }

        if (m_rootFiber)
        {
            if (!stopping())
            {
                m_rootFiber->call();
            }
        }
        std::vector<Thread::ptr> thrs;
        {
            MutexType::Lock lock(m_mutex);
            thrs.swap(m_threads);
        }

        for (auto &i : thrs)
        {
            i->join();
        }
    }

    void Scheduler::setThis()
    {
        t_scheduler = this;
    }

    void Scheduler::run()
    {
        LOG_INFO(g_logger) << "run";
        set_hook_enable(true);
        setThis();
        if (zdunk::GetThreadId() != m_rootThread)
        {
            t_fiber = Fiber::GetThis().get();
        }

        Fiber::ptr idle_fiber(new Fiber(std::bind(&Scheduler::idle, this)));
        Fiber::ptr cb_fiber;

        FiberAndThread ft;
        while (true)
        {
            ft.reset();
            bool tickle_me = false;
            bool is_active = false;
            {
                MutexType::Lock lock(m_mutex);
                auto it = m_fibers.begin();
                while (it != m_fibers.end())
                {
                    if (it->thread != -1 && it->thread != zdunk::GetThreadId())
                    {
                        ++it;
                        tickle_me = true;
                        continue;
                    }

                    ZDUNK_ASSERT(it->fiber || it->cb);
                    if (it->fiber && it->fiber->getState() == Fiber::EXEC)
                    {
                        ++it;
                        continue;
                    }

                    ft = *it;
                    m_fibers.erase(it++);
                    ++m_activeThreadCount;
                    is_active = true;
                    break;
                }
                tickle_me |= it != m_fibers.end();
            }

            if (tickle_me)
                tickle();

            if (ft.fiber && (ft.fiber->getState() != Fiber::TERM && ft.fiber->getState() != Fiber::EXCEPT))
            {
                ft.fiber->swapIn();
                --m_activeThreadCount;

                if (ft.fiber->getState() == Fiber::READY)
                {
                    schedule(ft.fiber);
                }
                else if (ft.fiber->getState() != Fiber::TERM && ft.fiber->getState() != Fiber::EXCEPT)
                {
                    ft.fiber->m_state = Fiber::HOLD;
                }
                ft.reset();
            }
            else if (ft.cb)
            {
                if (cb_fiber)
                {
                    cb_fiber->reset(ft.cb);
                }
                else
                {
                    cb_fiber.reset(new Fiber(ft.cb));
                }
                ft.reset();
                cb_fiber->swapIn();
                --m_activeThreadCount;

                if (cb_fiber->getState() == Fiber::READY)
                {
                    schedule(cb_fiber);
                    cb_fiber.reset();
                }
                else if (cb_fiber->getState() == Fiber::EXCEPT || cb_fiber->getState() == Fiber::TERM)
                {
                    cb_fiber->reset(nullptr);
                }
                else
                {
                    cb_fiber->m_state = Fiber::HOLD;
                    cb_fiber.reset();
                }
            }
            else
            {
                if (is_active)
                {
                    --m_activeThreadCount;
                    continue;
                }
                if (idle_fiber->getState() == Fiber::TERM)
                {
                    LOG_INFO(g_logger) << "idle fiber term";
                    break;
                }
                ++m_idleThreadCount;
                idle_fiber->swapIn();
                --m_idleThreadCount;
                if (idle_fiber->getState() != Fiber::TERM && idle_fiber->getState() != Fiber::EXCEPT)
                {
                    idle_fiber->m_state = Fiber::HOLD;
                }
            }
        }
    }

    void Scheduler::tickle()
    {
        LOG_INFO(g_logger) << "tickle";
        return;
    }
    bool Scheduler::stopping()
    {
        MutexType::Lock lock(m_mutex);
        return m_autoStop && m_stopping && m_fibers.empty() && m_activeThreadCount == 0;
    }
    void Scheduler::idle()
    {
        LOG_INFO(g_logger) << "idle";
        while (!stopping())
        {
            zdunk::Fiber::YieldToHold();
        }
    }

    void Scheduler::switchTo(int thread)
    {
        ZDUNK_ASSERT(Scheduler::GetThis() != nullptr);
        if (Scheduler::GetThis() == this)
        {
            if (thread == -1 || thread == zdunk::GetThreadId())
            {
                return;
            }
        }
        schedule(Fiber::GetThis(), thread);
        Fiber::YieldToHold();
    }

    std::ostream &Scheduler::dump(std::ostream &os)
    {
        os << "[Scheduler name=" << m_name
           << " size=" << m_threadCount
           << " active_count=" << m_activeThreadCount
           << " idle_count=" << m_idleThreadCount
           << " stopping=" << m_stopping
           << " ]" << std::endl
           << "    ";
        for (size_t i = 0; i < m_threadIds.size(); ++i)
        {
            if (i)
            {
                os << ", ";
            }
            os << m_threadIds[i];
        }
        return os;
    }

}