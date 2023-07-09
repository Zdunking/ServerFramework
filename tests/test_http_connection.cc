#include <iostream>
#include "../zdunk/pch.h"
#include "../zdunk/http/http_connection.h"

zdunk::Logger::ptr g_logger = LOG_ROOT();
#define LOG_TRACE LOG_INFO(g_logger)

void run()
{
    zdunk::Address::ptr addr = zdunk::Address::LookupAnyIPAddress("www.sylar.top:80");
    if (!addr)
    {
        LOG_TRACE << "get addr error";
        return;
    }

    zdunk::Socket::ptr sock = zdunk::Socket::CreateTCP(addr);
    bool rt = sock->connect(addr);
    if (!rt)
    {
        LOG_TRACE << "connect " << *addr << " failed";
        return;
    }

    zdunk::http::HttpConnection::ptr conn(new zdunk::http::HttpConnection(sock));

    zdunk::http::HttpRequest::ptr req(new zdunk::http::HttpRequest);
    req->setPath("/");
    //  req->setHeader("host", "www.zdunk.top");
    LOG_TRACE << "req:" << std::endl
              << *req;

    conn->sendRequest(req);
    auto rsp = conn->recvResponse();

    if (!rsp)
    {
        LOG_TRACE << "recv response error";
        return;
    }
    LOG_TRACE << "rsp:" << std::endl
              << *rsp;

    // LOG_TRACE << "=========================";

    // auto r = zdunk::http::HttpConnection::DoGet("http://www.baidu.com/", 3000);
    // LOG_TRACE << "result=" << r->result
    //           << " error=" << r->error
    //           << " rsp=" << (r->response ? r->response->toString() : "");
}

int main()
{
    zdunk::IOManager iom;
    iom.schedule(run);
    return 0;
}