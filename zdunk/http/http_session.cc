#include "http_session.h"

namespace zdunk
{
    namespace http
    {
        HttpSession::HttpSession(Socket::ptr sock, bool owner /*= true*/) : SocketStream(sock, owner)
        {
        }

        HttpRequest::ptr HttpSession::recvRequest()
        {
            HttpRequestParser::ptr parser(new HttpRequestParser);
            uint64_t buff_size = HttpRequestParser::GetHttpRequestBufferSize();
            std::shared_ptr<char> buffer(new char[buff_size], [](char *ptr)
                                         { delete[] ptr; });

            char *data = buffer.get();
            int offset = 0;
            do
            {
                int len = read(data, buff_size - offset);
                if (len <= 0)
                {
                    return nullptr;
                }
                len += offset;
                size_t nparser = parser->execute(data, buff_size + offset);
                if (parser->hasError())
                {
                    return nullptr;
                }
                offset = len - nparser;
                if (offset == (int)buff_size)
                {
                    return nullptr;
                }
                if (parser->isFinished())
                {
                    break;
                }
            } while (true);
            uint64_t length = parser->getContentLength();
            if (length > 0)
            {
                std::string body;
                body.reserve(length);

                if ((int)length >= offset)
                {
                    body.append(data, offset);
                }
                else
                {
                    body.append(data, length);
                }

                length -= offset;
                if (length > 0)
                {
                    if (readFixSize(&body[body.size()], length))
                    {
                        return nullptr;
                    }
                }
                parser->getData()->setBody(body);
            }
            return parser->getData();
        }

        int HttpSession::sendResponse(HttpResponse::ptr rsp)
        {
            std::stringstream ss;
            ss << *rsp;
            std::string data = ss.str();
            return writeFixSize(data.c_str(), data.size());
        }

    } // namespace http
} // namespace zdunk
