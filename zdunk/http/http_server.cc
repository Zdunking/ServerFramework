#include "http_server.h"
#include "../log.h"

namespace zdunk
{
    namespace http
    {

        static zdunk::Logger::ptr g_logger = LOG_NAME("system");

        HttpServer::HttpServer(bool keepalived /*= false*/, IOManager *woker /* = IOManager::GetThis()*/, IOManager *accept_woker /* = IOManager::GetThis()*/)
            : TcpServer(woker, accept_woker), m_isKeepAlived(keepalived)
        {
        }

        void HttpServer::handleClient(Socket::ptr client)
        {
            HttpSession::ptr session(new HttpSession(client));
            do
            {
                auto req = session->recvRequest();
                if (!req)
                {
                    LOG_WARN(g_logger) << "recv http request fail, errno=" << errno << " errstr=" << strerror(errno) << " client" << *client;
                    break;
                }
                HttpResponse::ptr rsp(new HttpResponse(req->getVersion(), req->isClose() || !m_isKeepAlived));
                rsp->setBody("hello zdunk");

                // LOG_DEBUG(g_logger) << "request:" << std::endl
                //                     << *req;

                // LOG_DEBUG(g_logger) << "response:" << std::endl
                //                     << *rsp;

                session->sendResponse(rsp);
            } while (m_isKeepAlived);
            session->close();
        }

    } // namespace http
} // namespace zdunk
