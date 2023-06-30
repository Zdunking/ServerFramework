#include "../zdunk/pch.h"
#include "../zdunk/iomanager.h"
#include "../zdunk/socket.h"

static zdunk::Logger::ptr g_logger = LOG_ROOT();
#define LOG_TRACE LOG_INFO(g_logger)

void test_socket()
{
    zdunk::IPAddress::ptr addr = zdunk::Address::LookupAnyIPAddress("www.baidu.com");
    if (!addr)
    {
        LOG_TRACE << "get address : " << addr->toString();
    }
    else
    {
        LOG_TRACE << "get address failed";
    }

    zdunk::Socket::ptr sock = zdunk::Socket::CreateTCP(addr);
    addr->setPort(80);
    if (!sock->connect(addr))
    {
        LOG_TRACE << "connect" << addr->toString() << " failed";
    }
    else
    {
        LOG_TRACE << "connect" << addr->toString() << " connect";
    }

    const char buff[] = "GET / HTTP/1.0\r\n\r\n";
    int rt = sock->send(buff, sizeof(buff));

    if (rt < 0)
    {
        LOG_TRACE << "send failed" << rt;
        return;
    }

    std::string buffs;
    buffs.resize(4096 * 2);
    rt = sock->recv(&buffs[0], buffs.size());

    if (rt < 0)
    {
        LOG_TRACE << "recv failed" << rt;
        return;
    }

        LOG_TRACE << std::endl
              << buffs;
}

int main()
{
    zdunk::IOManager iom;
    iom.schedule(test_socket);
    return 0;
}