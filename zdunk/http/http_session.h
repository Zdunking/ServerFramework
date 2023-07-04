#pragma once
#include "../stream/socket_stream.h"
#include "http.h"
#include "http_parser.h"

namespace zdunk
{
    namespace http
    {
        class HttpSession : public SocketStream
        {
        public:
            typedef std::shared_ptr<HttpSession> ptr;

            HttpSession(Socket::ptr sock, bool owner = true);
            HttpRequest::ptr recvRequest();
            int sendResponse(HttpResponse::ptr rsp);
        };
    } // namespace http
} // namespace zdunk
