#include "../zdunk/pch.h"
#include "../zdunk/tcp_server.h"

static zdunk::Logger::ptr g_logger = LOG_ROOT();
#define LOG_TRACE LOG_INFO(g_logger)

void run()
{
    auto addr = zdunk::Address::LookupAny("0.0.0.0:8033");
    // auto addr2 = zdunk::UnixAddress::ptr(new zdunk::UnixAddress("tmp/unix_addr"));
    LOG_TRACE << *addr /*<< " - " << *addr2*/;
    std::vector<zdunk::Address::ptr> addrs;

    addrs.push_back(addr);
    // addrs.push_back(addr2);

    zdunk::TcpServer::ptr tcp_server(new zdunk::TcpServer);
    std::vector<zdunk::Address::ptr> fails;
    while (!tcp_server->bind(addrs, fails))
    {
        sleep(2);
    }
    tcp_server->start();
}

int main()
{
    zdunk::IOManager iom(2);
    iom.schedule(run);
    return 0;
}