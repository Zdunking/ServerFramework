#include "../zdunk/pch.h"
#include "../zdunk/tcp_server.h"
#include "../zdunk/iomanager.h"
#include "../zdunk/bytearray.h"
#include "../zdunk/address.h"

static zdunk::Logger::ptr g_logger = LOG_ROOT();
#define LOG_TRACE LOG_INFO(g_logger)

class EchoServer : public zdunk::TcpServer
{
public:
    typedef std::shared_ptr<EchoServer> ptr;
    EchoServer(int type);
    virtual void handleClient(zdunk::Socket::ptr client) override;

private:
    int m_type;
};

EchoServer::EchoServer(int type) : m_type(type)
{
}

void EchoServer::handleClient(zdunk::Socket::ptr client)
{
    LOG_TRACE << "handleClient: " << *client;
    zdunk::ByteArray::ptr ba(new zdunk::ByteArray);
    while (true)
    {
        ba->clear();
        std::vector<iovec> iovs;
        ba->getWriteBuffers(iovs, 1024);

        int rt = client->recv(&iovs[0], iovs.size());
        if (rt == 0)
        {
            LOG_TRACE << "client close" << *client;
            break;
        }
        else if (rt < 0)
        {
            LOG_TRACE << "client error" << rt << " errno=" << errno << " errstr=" << strerror(errno);
        }

        ba->setPosition(ba->getPosition() + rt);
        ba->setPosition(0);
        // LOG_TRACE << "recv rt = " << rt << " date : " << std::string((char *)iovs[0].iov_base, rt);

        if (m_type == 1) // TEXT
        {
            std::cout << "TEX:" << ba->toString();
        }
        else
        {
            std::cout << "HEX:" << ba->toHexString();
        }
    }
}

int type = 1;

void run()
{
    LOG_TRACE << "server type = " << type;
    EchoServer::ptr es(new EchoServer(type));
    auto addr = zdunk::Address::LookupAny("0.0.0.0:8020");
    while (!es->bind(addr))
    {
        sleep(2);
    }
    es->start();
}

int main(int argc, char **argvs)
{
    if (argc < 2)
    {
        LOG_TRACE << "used as [" << argvs[0] << "-t] or [" << argvs[0] << " -b]";
    }

    if (!strcmp(argvs[1], "-b"))
    {
        type = 2;
    }

    zdunk::IOManager iom;
    iom.schedule(run);
    return 0;
}