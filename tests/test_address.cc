#include "../zdunk/pch.h"
#include "../zdunk/address.h"

zdunk::Logger::ptr g_logger = LOG_ROOT();

#define LOG_TRACE LOG_INFO(g_logger)

void test()
{
    std::vector<zdunk::Address::ptr> addrs;
    bool v = zdunk::Address::Lookup(addrs, "www.baidu.com:http");

    if (!v)
    {
        LOG_TRACE << "Lookup failed";
        return;
    }

    for (size_t i = 0; i < addrs.size(); ++i)
    {
        LOG_TRACE << i << " -- " << addrs[i]->toString();
    }
}

void test_iface()
{
    std::multimap<std::string, std::pair<zdunk::Address::ptr, uint32_t>> results;
    bool v = zdunk::Address::GetInterfaceAddresses(results);
    if (!v)
    {
        LOG_TRACE << "GetInterfaceAddresses fail";
        return;
    }

    for (auto &i : results)
    {
        LOG_TRACE << i.first << " - " << i.second.first->toString() << " - "
                  << i.second.second;
    }
}

void test_ipv4()
{
    // auto addr = zdunk::IPAddress::Create("www.baidu.com");
    auto addr = zdunk::IPAddress::Create("127.0.0.8");
    if (addr)
    {
        LOG_TRACE << addr->toString();
    }
}

int main()
{
    // test();
    // test_iface();
    test_ipv4();
    return 0;
}