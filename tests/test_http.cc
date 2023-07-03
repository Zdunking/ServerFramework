#include "../zdunk/pch.h"
#include "../zdunk/http/http.h"

static zdunk::Logger::ptr g_logger = LOG_ROOT();
#define LOG_TRACE = LOG_INFO(g_logger)

void test_request()
{
    zdunk::http::HttpRequest::ptr req(new zdunk::http::HttpRequest);
    req->setHeader("host", "www.baidu.com");
    req->setBody("hello zdunk");

    req->dump(std::cout) << std::endl;
}

void test_response()
{
    zdunk::http::HttpResponse::ptr rsp(new zdunk::http::HttpResponse);
    rsp->setHeader("X-X", "zdunk");
    rsp->setBody("hello zdunk");
    rsp->setStatus((zdunk::http::HttpStatus)400);
    rsp->setClose(false);

    rsp->dump(std::cout) << std::endl;
}

int main()
{
    test_request();
    test_response();
    return 0;
}
