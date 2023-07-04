#pragma once
#include "../tcp_server.h"
#include "http_session.h"

namespace zdunk
{
    namespace http
    {
        class HttpServer : public TcpServer
        {
        public:
            typedef std::shared_ptr<HttpServer> ptr;
            HttpServer(bool keepalived = false, zdunk::IOManager *woker = zdunk::IOManager::GetThis(), zdunk::IOManager *accept_woker = zdunk::IOManager::GetThis());

        protected:
            virtual void handleClient(Socket::ptr client) override;

        private:
            bool m_isKeepAlived;
        };

    } // namespace http
} // namespace zdunk
