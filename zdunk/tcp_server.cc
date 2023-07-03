#include "tcp_server.h"
#include "config.h"

namespace zdunk
{
    static zdunk::Logger::ptr g_logger = LOG_NAME("system");

    static zdunk::ConfigVar<uint64_t>::ptr g_tcp_eserver_read_timeout =
        zdunk::Config::Lookup("tcp_server.read_timeout", (uint64_t)(60 * 1000 * 2), "tcp server read timeout");

    TcpServer::TcpServer(IOManager *woker /*= IOManager::GetThis()*/, IOManager *awoker /*= IOManager::GetThis()*/)
        : m_woker(woker), m_acceptWorker(awoker),
          m_recvTimeout(g_tcp_eserver_read_timeout->getValue()), m_name("zdunk/1.0.0"), m_isStop(true)
    {
    }

    TcpServer::~TcpServer()
    {
        for (auto &i : m_socks)
        {
            i->close();
        }
        m_socks.clear();
    }

    bool TcpServer::bind(Address::ptr addr)
    {
        std::vector<Address::ptr> addrs;
        std::vector<Address::ptr> fails;
        addrs.push_back(addr);
        return bind(addrs, fails);
    }

    bool TcpServer::bind(std::vector<Address::ptr> addrs, std::vector<Address::ptr> &fails)
    {
        for (auto addr : addrs)
        {
            Socket::ptr sock = Socket::CreateTCP(addr);
            if (!sock->bind(addr))
            {
                LOG_ERROR(g_logger) << "bind failed errno=" << errno << " errstr=" << strerror(errno)
                                    << " addr=[" << addr->toString() << "]";
                fails.push_back(addr);
                continue;
            }

            if (!sock->listen())
            {

                LOG_ERROR(g_logger) << "listen failed errno=" << errno << " errstr=" << strerror(errno)
                                    << " addr=[" << addr->toString() << "]";
                fails.push_back(addr);
                continue;
            }

            m_socks.push_back(sock);
        }

        if (!fails.empty())
        {
            m_socks.clear();
            return false;
        }

        for (auto &i : m_socks)
        {
            LOG_INFO(g_logger) << "server bind sussess: " << *i;
        }
        return true;
    }

    void TcpServer::startAccept(Socket::ptr sock)
    {
        while (!m_isStop)
        {
            Socket::ptr client = sock->accept();
            if (client)
            {
                client->setRecvTimeout(m_recvTimeout);
                m_woker->schedule(std::bind(&TcpServer::handleClient, shared_from_this(), client));
            }
            else
            {
                LOG_ERROR(g_logger) << "accept error" << errno << "srrstr=" << strerror(errno);
            }
        }
    }

    bool TcpServer::start()
    {
        if (!m_isStop)
        {
            return true;
        }
        m_isStop = false;
        for (auto &sock : m_socks)
        {
            m_acceptWorker->schedule(std::bind(&TcpServer::startAccept, shared_from_this(), sock));
        }
        return true;
    }

    void TcpServer::stop()
    {
        m_isStop = true;
        auto self = shared_from_this();
        m_acceptWorker->schedule([this, self]()
                                 {
        for(auto& sock : m_socks) {
            sock->cancelAll();
            sock->close();
        }
        m_socks.clear(); });
    }

    void TcpServer::handleClient(Socket::ptr client)
    {
        LOG_INFO(g_logger) << "handleClient: " << *client;
    }

} // namespace zdunk
