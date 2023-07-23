#pragma once
#include <thread>
#include <functional>
#include <memory>
#include <pthread.h>
#include <semaphore.h>
#include <list>
#include "noncopyable.h"
#include "fiber.h"

namespace zdunk
{
    class Semaphore : Noncopyable
    {
    public:
        Semaphore(uint32_t count = 0);
        ~Semaphore();

        void wait();
        void notify();

    private:
        Semaphore(const Semaphore &) = delete;
        Semaphore(const Semaphore &&) = delete;
        Semaphore operator=(const Semaphore &) = delete;

    private:
        sem_t m_semaphore;
    };

    template <class T>
    class ScopeLockImpl
    {
    public:
        ScopeLockImpl(T &mutex) : m_mutex(mutex)
        {
            m_mutex.lock();
            m_locked = true;
        }
        ~ScopeLockImpl()
        {
            unlock();
        }

        void lock()
        {
            if (!m_locked)
            {
                m_mutex.lock();
                m_locked = true;
            }
        }

        void unlock()
        {
            if (m_locked)
            {
                m_mutex.unlock();
                m_locked = false;
            }
        }

    private:
        T &m_mutex;
        bool m_locked;
    };

    class Mutex : Noncopyable
    {
    public:
        typedef ScopeLockImpl<Mutex> Lock;
        Mutex()
        {
            pthread_mutex_init(&m_mutex, nullptr);
        }
        ~Mutex()
        {
            pthread_mutex_destroy(&m_mutex);
        }

        void lock()
        {
            pthread_mutex_lock(&m_mutex);
        }

        void unlock()
        {
            pthread_mutex_unlock(&m_mutex);
        }

    private:
        pthread_mutex_t m_mutex;
    };

    class NullMutex : Noncopyable
    {
    public:
        typedef ScopeLockImpl<NullMutex> Lock;
        NullMutex() {}
        ~NullMutex() {}
        void lock() {}
        void unlock() {}
    };

    template <class T>
    class ReadScopeLockImpl
    {
    public:
        ReadScopeLockImpl(T &mutex) : m_mutex(mutex)
        {
            m_mutex.rdlock();
            m_locked = true;
        }
        ~ReadScopeLockImpl()
        {
            unlock();
        }

        void lock()
        {
            if (!m_locked)
            {
                m_mutex.rdlock();
                m_locked = true;
            }
        }

        void unlock()
        {
            if (m_locked)
            {
                m_mutex.unlock();
                m_locked = false;
            }
        }

    private:
        T &m_mutex;
        bool m_locked;
    };

    template <class T>
    class WriteScopeLockImpl
    {
    public:
        WriteScopeLockImpl(T &mutex) : m_mutex(mutex)
        {
            m_mutex.wrlock();
            m_locked = true;
        }
        ~WriteScopeLockImpl()
        {
            unlock();
        }

        void lock()
        {
            if (!m_locked)
            {
                m_mutex.wrlock();
                m_locked = true;
            }
        }

        void unlock()
        {
            if (m_locked)
            {
                m_mutex.unlock();
                m_locked = false;
            }
        }

    private:
        T &m_mutex;
        bool m_locked;
    };

    class RWMutex : Noncopyable
    {
    public:
        typedef ReadScopeLockImpl<RWMutex> ReadLock;
        typedef WriteScopeLockImpl<RWMutex> WriteLock;

        RWMutex()
        {
            pthread_rwlock_init(&m_lock, nullptr);
        }
        ~RWMutex()
        {
            pthread_rwlock_destroy(&m_lock);
        }

        void rdlock()
        {
            pthread_rwlock_rdlock(&m_lock);
        }

        void wrlock()
        {
            pthread_rwlock_wrlock(&m_lock);
        }

        void unlock()
        {
            pthread_rwlock_unlock(&m_lock);
        }

    private:
        pthread_rwlock_t m_lock;
    };

    class NUllRWMutex : Noncopyable
    {
    public:
        typedef ReadScopeLockImpl<NUllRWMutex> ReadLock;
        typedef WriteScopeLockImpl<NUllRWMutex> WriteLock;
        NUllRWMutex() {}
        ~NUllRWMutex() {}
        void rdlock() {}
        void wrlock() {}
        void unlock() {}
    };

    /**
     * @brief 自旋锁
     */
    class Spinlock : Noncopyable
    {
    public:
        /// 局部锁
        typedef ScopeLockImpl<Spinlock> Lock;

        /**
         * @brief 构造函数
         */
        Spinlock()
        {
            pthread_spin_init(&m_mutex, 0);
        }

        /**
         * @brief 析构函数
         */
        ~Spinlock()
        {
            pthread_spin_destroy(&m_mutex);
        }

        /**
         * @brief 上锁
         */
        void lock()
        {
            pthread_spin_lock(&m_mutex);
        }

        /**
         * @brief 解锁
         */
        void unlock()
        {
            pthread_spin_unlock(&m_mutex);
        }

    private:
        /// 自旋锁
        pthread_spinlock_t m_mutex;
    };

    class Scheduler;
    class FiberSemaphore : Noncopyable
    {
    public:
        typedef Spinlock MutexType;

        FiberSemaphore(size_t initial_concurrency = 0);
        ~FiberSemaphore();

        bool tryWait();
        void wait();
        void notify();

        size_t getConcurrency() const { return m_concurrency; }
        void reset() { m_concurrency = 0; }

    private:
        MutexType m_mutex;
        std::list<std::pair<Scheduler *, Fiber::ptr>> m_waiters;
        size_t m_concurrency;
    };

    class Thread
    {
    public:
        typedef std::shared_ptr<Thread> ptr;
        Thread(std::function<void()> cb, const std::string &name);
        ~Thread();

        pid_t GetId() const { return m_id; };
        const std::string &getName() const { return m_name; }

        void join();

        static Thread *GetThis();
        static const std::string &GetName();
        static void SetName(const std::string name);

    private:
        Thread(const Thread &) = delete;
        Thread(const Thread &&) = delete;
        Thread operator=(const Thread &) = delete;

        static void *run(void *param);

    private:
        pid_t m_id = -1;
        pthread_t m_thread = 0;
        std::function<void()> m_cb = nullptr;
        std::string m_name;

        Semaphore m_semaphore;
    };
}
