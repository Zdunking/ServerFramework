#include "../zdunk/pch.h"
#include "../zdunk/iomanager.h"
#include "../zdunk/hook.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

zdunk::Logger::ptr g_logger = LOG_ROOT();
#define LOG_TRACE LOG_INFO(g_logger)

void test_sleep()
{
    zdunk::IOManager iom(1);
    iom.schedule([]()
                 { 
                 sleep(2); 
                 LOG_TRACE << "sleep 2"; });
    iom.schedule([]()
                 { 
                 sleep(3); 
                 LOG_TRACE << "sleep 3"; });
    LOG_TRACE << "test sleep";
}

void test_sock()
{
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    // fcntl(sock, F_SETFL, O_NONBLOCK);

    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(80);
    inet_pton(AF_INET, "39.156.66.14", &addr.sin_addr.s_addr);

    int rt = connect(sock, (const sockaddr *)&addr, (socklen_t)sizeof(addr));
    LOG_TRACE << "connect rt=" << rt << " errno=" << errno;

    if (rt)
    {
        return;
    }

    const char data[] = "GET / HTTP/1.0\r\n\r\n";
    rt = send(sock, data, sizeof(data), 0);
    LOG_TRACE << "send rt=" << rt << " errno=" << errno;

    if (rt <= 0)
    {
        return;
    }

    std::string buff;
    buff.resize(4096);

    rt = recv(sock, &buff[0], buff.size(), 0);
    LOG_TRACE << "recv rt=" << rt << " errno=" << errno;

    if (rt <= 0)
    {
        return;
    }

    buff.resize(rt);
    LOG_TRACE << buff;
}

int main()
{
    // test_sock();
    // test_sleep();
    zdunk::IOManager iom;
    iom.schedule(test_sock);
    return 0;
}