#include "log.h"
#include "thread.h"
#include "macro.h"
#include "scheduler.h"

namespace zdunk
{
    static thread_local Thread *t_thread = nullptr;
    static thread_local std::string t_thread_name = "UNKNOW";

    static zdunk::Logger::ptr g_logger = LOG_NAME("system");

    Semaphore::Semaphore(uint32_t count /*= 0*/)
    {
        if (sem_init(&m_semaphore, 0, count))
        {
            throw std::logic_error("sem_init error");
        }
    }

    Semaphore::~Semaphore()
    {
        sem_destroy(&m_semaphore);
    }

    void Semaphore::wait()
    {

        if (sem_wait(&m_semaphore))
        {
            throw std::logic_error("sem_wait error");
        }
    }

    void Semaphore::notify()
    {
        if (sem_post(&m_semaphore))
        {
            throw std::logic_error("sem_post error");
        }
    }

    FiberSemaphore::FiberSemaphore(size_t initial_concurrency)
        : m_concurrency(initial_concurrency)
    {
    }

    FiberSemaphore::~FiberSemaphore()
    {
        ZDUNK_ASSERT(m_waiters.empty());
    }

    bool FiberSemaphore::tryWait()
    {
        ZDUNK_ASSERT(Scheduler::GetThis());
        {
            MutexType::Lock lock(m_mutex);
            if (m_concurrency > 0u)
            {
                --m_concurrency;
                return true;
            }
            return false;
        }
    }

    void FiberSemaphore::wait()
    {
        ZDUNK_ASSERT(Scheduler::GetThis());
        {
            MutexType::Lock lock(m_mutex);
            if (m_concurrency > 0u)
            {
                --m_concurrency;
                return;
            }
            m_waiters.push_back(std::make_pair(Scheduler::GetThis(), Fiber::GetThis()));
        }
        Fiber::YieldToHold();
    }

    void FiberSemaphore::notify()
    {
        MutexType::Lock lock(m_mutex);
        if (!m_waiters.empty())
        {
            auto next = m_waiters.front();
            m_waiters.pop_front();
            next.first->schedule(next.second);
        }
        else
        {
            ++m_concurrency;
        }
    }

    Thread::Thread(std::function<void()> cb, const std::string &name) : m_cb(cb), m_name(name)
    {
        if (name.empty())
        {
            m_name = "UNKNOW";
        }
        int rt = pthread_create(&m_thread, nullptr, &Thread::run, this);
        if (rt)
        {
            LOG_ERROR(g_logger) << "pthread_create thread failed, rt=" << rt << " name=" << m_name;
            throw std::logic_error("pthread_create error");
        }

        m_semaphore.wait();
    }

    Thread::~Thread()
    {
        if (m_thread)
        {
            pthread_detach(m_thread);
        }
    }

    void Thread::join()
    {
        if (m_thread)
        {
            int rt = pthread_join(m_thread, nullptr);
            if (rt)
            {
                LOG_ERROR(g_logger) << "pthread_join thread failed, rt=" << rt << " name=" << m_name;
                throw std::logic_error("pthread_join error");
            }
            m_thread = 0;
        }
    }

    Thread *Thread::GetThis()
    {
        return t_thread;
    }

    const std::string &Thread::GetName()
    {
        return t_thread_name;
    }

    void Thread::SetName(const std::string name)
    {
        if (name.empty())
        {
            return;
        }
        if (t_thread)
        {
            t_thread->m_name = name;
        }
        t_thread_name = name;
    }

    void *Thread::run(void *param)
    {
        Thread *thread = (Thread *)param;
        t_thread = thread;
        t_thread_name = thread->m_name;
        thread->m_id = zdunk::GetThreadId();
        pthread_setname_np(pthread_self(), thread->m_name.substr(0, 15).c_str());

        std::function<void()> cb;
        cb.swap(thread->m_cb);

        thread->m_semaphore.notify();

        cb();
        return 0;
    }
}
