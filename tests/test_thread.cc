#include "../zdunk/pch.h"
#include "../zdunk/thread.h"
#include "vector"

zdunk::Logger::ptr g_logger = LOG_ROOT();
#define LOG_TRACE LOG_INFO(LOG_ROOT())

int count = 0;
zdunk::RWMutex s_rwmutex;
zdunk::Mutex s_mutex;

void fun1()
{
    LOG_TRACE << "name=" << zdunk::Thread::GetName()
              << " this.name=" << zdunk::Thread::GetThis()->getName()
              << " id=" << zdunk::GetThreadId()
              << " this.id=" << zdunk::Thread::GetThis()->GetId();

    for (int i = 0; i < 500000; i++)
    {
        // zdunk::RWMutex::WriteLock lock(s_rwmutex);
        zdunk::Mutex::Lock lock(s_mutex);
        count++;
    }
}

void fun2()
{
    while (1)
    {
        LOG_TRACE << "**********************************";
        sleep(1);
    }
}

void fun3()
{
    while (1)
    {
        LOG_TRACE << "----------------------------------";
        sleep(1);
    }
}

int main()
{
    LOG_TRACE << "thread test begin";
    std::vector<zdunk::Thread::ptr> thrs;
    YAML::Node root = YAML::LoadFile("/home/zdunk/workspace/zdunking/bin/conf/log2.yml");
    zdunk::Config::LoadFromYaml(root);
    // for (int i = 0; i < 2; i++)
    // {
    //     zdunk::Thread::ptr thr(new zdunk::Thread(&fun2, "name1" /*"name_" + std::to_string(i * 2)*/));
    //     zdunk::Thread::ptr thr2(new zdunk::Thread(&fun3, "name2" /*"name_" + std::to_string(i * 2 + 1)*/));
    //     thrs.push_back(thr);
    //     thrs.push_back(thr2);
    // }
    zdunk::Thread::ptr thr(new zdunk::Thread(&fun2, "name1" /*"name_" + std::to_string(i * 2)*/));
    zdunk::Thread::ptr thr2(new zdunk::Thread(&fun3, "name2" /*"name_" + std::to_string(i * 2 + 1)*/));
    thrs.push_back(thr);
    thrs.push_back(thr2);

    for (auto i : thrs)
    {
        i->join();
    }

    LOG_TRACE << "thread test end";

    LOG_TRACE << "count = " << count;

    return 0;
}
