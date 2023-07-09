#include "../zdunk/http/http_server.h"
#include "../zdunk/pch.h"

static zdunk::Logger::ptr g_logger = LOG_ROOT();
#define LOG_TRACE LOG_INFO(g_logger)

void run()
{
    zdunk::http::HttpServer::ptr server(new zdunk::http::HttpServer);
    zdunk::Address::ptr addr = zdunk::Address::LookupAnyIPAddress("0.0.0.0:8020");
    while (!server->bind(addr))
    {
        sleep(2);
    }
    auto sd = server->getServletDisPatch();
    sd->addServlet("/zdunk/xxx", [](zdunk::http::HttpRequest::ptr req, zdunk::http::HttpResponse::ptr rsp, zdunk::http::HttpSession::ptr session)
                   { rsp->setBody(req->toString()); return 0; });

    sd->addGlobServlet("/zdunk/*", [](zdunk::http::HttpRequest::ptr req, zdunk::http::HttpResponse::ptr rsp, zdunk::http::HttpSession::ptr session)
                       { rsp->setBody("Glob: \r\n" + req->toString()); return 0; });
    server->start();
}

int main()
{
    zdunk::IOManager iom(2);
    iom.schedule(run);
    return 0;
}