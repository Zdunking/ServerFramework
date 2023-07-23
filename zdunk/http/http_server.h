#pragma once
#include "../tcp_server.h"
#include "http_session.h"
#include "servlet.h"

namespace zdunk
{
    namespace http
    {
        class HttpServer : public TcpServer
        {
        public:
            typedef std::shared_ptr<HttpServer> ptr;
            HttpServer(bool keepalive = false, IOManager *worker = IOManager::GetThis(), IOManager *io_worker = IOManager::GetThis(), IOManager *accept_worker = IOManager::GetThis());

            ServletDispatch::ptr getServletDisPatch() const { return m_dispatch; }
            void setServletDisPatch(ServletDispatch::ptr v) { m_dispatch = v; }

        protected:
            virtual void handleClient(Socket::ptr client) override;

        private:
            bool m_isKeepAlived;
            ServletDispatch::ptr m_dispatch;
        };

    } // namespace http
} // namespace zdunk
