#pragma once

#include "scheduler.h"
#include "timer.h"

namespace zdunk
{
    class IOManager : public Scheduler, public TimerManager
    {
    public:
        typedef std::shared_ptr<IOManager> ptr;
        typedef RWMutex RWMutexType;

        enum Event
        {
            NONE = 0x0,
            READ = 0x1,
            WRITE = 0x4,
        };

    public:
        IOManager(size_t threads = 1, bool use_caller = true, const std::string &name = "");
        ~IOManager();

        int addEvent(int fd, Event event, std::function<void()> cb = nullptr);
        bool delEvent(int fd, Event event);
        bool cancelEvent(int fd, Event event);
        bool cancelAll(int fd);

        static IOManager *GetThis();

    protected:
        virtual void tickle() override;
        virtual bool stopping() override;
        virtual void idle() override;
        virtual void onTimerInsertedAtFront() override;

        void contextResize(size_t size);

        bool stopping(uint64_t &timeout);

    private:
        struct FdContext
        {
            typedef Mutex MutexType;
            struct EventContext
            {
                Scheduler *scheduler = nullptr;
                Fiber::ptr fiber;
                std::function<void()> cb;
            };

            EventContext &getContext(Event event);
            void resetContext(EventContext &ctx);
            void triggerEvent(IOManager::Event event);

            EventContext read;
            EventContext write;
            int fd = 0;
            Event events = NONE;
            MutexType mutex;
        };

        int m_epfd = 0;
        int m_tickleFds[2];

        std::atomic<size_t> m_pendingEventCount;
        RWMutex m_mutex;
        std::vector<FdContext *> m_fdContexts;
    };
}