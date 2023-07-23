#include <iostream>
#include <vector>
#include "../zdunk/module.h"
#include "../zdunk/application.h"

int32_t testHandle(zdunk::http::HttpRequest::ptr request, zdunk::http::HttpResponse::ptr response, zdunk::http::HttpSession::ptr session)
{
    response->setBody("http server zdunk");
    return 0;
}

class MyModule : public zdunk::Module
{
public:
    MyModule() : Module("TestModule", "1.0", "") {}

    void onBeforeArgsParse(int argc, char **argv) override
    {
    }

    void onAfterArgsParse(int argc, char **argv) override
    {
    }

    bool onLoad() override
    {
        return true;
    }

    bool onUnload() override
    {
        return true;
    }

    bool onConnect(zdunk::Stream::ptr stream) override
    {
        return true;
    }

    bool onDisconnect(zdunk::Stream::ptr stream) override
    {
        return true;
    }

    bool onServerReady() override
    {
        std::vector<zdunk::TcpServer::ptr> servers;
        zdunk::Application::GetInstance()->getServer("http", servers);
        for (auto i : servers)
        {
            auto sd = std::dynamic_pointer_cast<zdunk::http::HttpServer>(i)->getServletDisPatch();
            sd->addServlet("/zdunk", testHandle);
        }
        return true;
    }

    bool onServerUp() override
    {
        return true;
    }
};

extern "C"
{
    zdunk::Module *CreateModule()
    {
        std::cout << "************CreateModule*************" << std::endl;
        return new MyModule;
    }

    void DestoryModule(zdunk::Module *ptr)
    {
        std::cout << "************DestoryModule*************" << std::endl;
        delete ptr;
    }
}