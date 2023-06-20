#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include "../zdunk/pch.h"
#include "../zdunk/iomanager.h"

static zdunk::Logger::ptr g_logger = LOG_ROOT();
#define LOG_TRACE LOG_INFO(g_logger)

int sock = 0;

void test_fiber()
{
    LOG_TRACE << "test_fiber";

    sock = socket(AF_INET, SOCK_STREAM, 0);
    fcntl(sock, F_SETFL, O_NONBLOCK);

    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(80);
    inet_pton(AF_INET, "39.156.66.14", &addr.sin_addr.s_addr);

    if (!connect(sock, (const sockaddr *)&addr, (socklen_t)sizeof(addr)))
    {
    }
    else if (errno == EINPROGRESS)
    {
        LOG_TRACE << "add event errno=" << errno << " " << strerror(errno);
        zdunk::IOManager::GetThis()->addEvent(sock, zdunk::IOManager::READ, []()
                                              { LOG_TRACE << "read callback"; });
        zdunk::IOManager::GetThis()->addEvent(sock, zdunk::IOManager::WRITE, []()
                                              { LOG_TRACE << "write callback";
                                              zdunk::IOManager::GetThis()->cancelEvent(sock, zdunk::IOManager::READ);
                                              close(sock); });
    }
    else
    {
        LOG_TRACE << "else" << errno << " " << strerror(errno);
    }
}

void test1()
{
    zdunk::IOManager iom;
    iom.schedule(&test_fiber);
}

int main()
{
    test1();
    return 0;
}