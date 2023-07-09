#include "servlet.h"
#include <fnmatch.h>

namespace zdunk
{
    namespace http
    {

        FunctionServlet::FunctionServlet(callback cb) : Servlet("FunctionServlet"), m_cb(cb)
        {
        }

        int32_t FunctionServlet::handle(HttpRequest::ptr request, HttpResponse::ptr response, HttpSession::ptr session)
        {
            return m_cb(request, response, session);
        }

        ServletDispatch::ServletDispatch() : Servlet("ServletDispatch")
        {
            m_default.reset(new NotFoundServlet("zdunk/1.0.0"));
        }

        int32_t ServletDispatch::handle(HttpRequest::ptr request, HttpResponse::ptr response, HttpSession::ptr session)
        {
            auto slt = getMatchedServlet(request->getPath());
            if (slt)
            {
                slt->handle(request, response, session);
            }
            return 0;
        }

        void ServletDispatch::addServlet(const std::string uri, Servlet::ptr slt)
        {
            RWMutex::WriteLock lock(m_mutex);
            m_datas[uri] = slt;
        }

        void ServletDispatch::addServlet(const std::string uri, FunctionServlet::callback cb)
        {
            RWMutex::WriteLock lock(m_mutex);
            m_datas[uri].reset(new FunctionServlet(cb));
        }

        void ServletDispatch::addGlobServlet(const std::string uri, Servlet::ptr slt)
        {

            RWMutex::WriteLock lock(m_mutex);
            for (auto it = m_globs.begin(); it != m_globs.end(); ++it)
            {
                if (it->first == uri)
                {
                    m_globs.erase(it);
                    break;
                }
            }
            m_globs.push_back(std::make_pair(uri, slt));
        }

        void ServletDispatch::addGlobServlet(const std::string uri, FunctionServlet::callback cb)
        {
            return addGlobServlet(uri, std::make_shared<FunctionServlet>(cb));
        }

        void ServletDispatch::delServlet(const std::string uri, Servlet::ptr slt)
        {
            RWMutexType::WriteLock lock(m_mutex);
            m_datas.erase(uri);
        }

        void ServletDispatch::delGlobServlet(const std::string uri, Servlet::ptr slt)
        {
            RWMutexType::WriteLock lock(m_mutex);
            for (auto it = m_globs.begin();
                 it != m_globs.end(); ++it)
            {
                if (it->first == uri)
                {
                    m_globs.erase(it);
                    break;
                }
            }
        }

        Servlet::ptr ServletDispatch::getServlet(const std::string &uri)
        {
            RWMutexType::ReadLock lock(m_mutex);
            auto it = m_datas.find(uri);
            return it == m_datas.end() ? nullptr : it->second;
        }

        Servlet::ptr ServletDispatch::getGlobServlet(const std::string &uri)
        {
            RWMutexType::ReadLock lock(m_mutex);
            for (auto it = m_globs.begin(); it != m_globs.end(); ++it)
            {
                if (it->first == uri)
                {
                    return it->second;
                }
            }
            return nullptr;
        }

        Servlet::ptr ServletDispatch::getMatchedServlet(const std::string uri)
        {
            RWMutexType::ReadLock lock(m_mutex);
            auto mit = m_datas.find(uri);
            if (mit != m_datas.end())
            {
                return mit->second;
            }
            for (auto it = m_globs.begin(); it != m_globs.end(); ++it)
            {
                if (!fnmatch(it->first.c_str(), uri.c_str(), 0))
                {
                    return it->second;
                }
            }
            return m_default;
        }

        NotFoundServlet::NotFoundServlet(const std::string &name)
            : Servlet("NotFoundServlet"), m_name(name)
        {
            m_content = "<html><head><title>404 Not Found"
                        "</title></head><body><center><h1>404 Not Found</h1></center>"
                        "<hr><center>" +
                        name + "</center></body></html>";

            // m_content = "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\">"
            //             " <html xmlns=\"http://www.w3.org/1999/xhtml\"> <head> <meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\" /> "
            //             "<title>404-对不起！您访问的页面不存在</title> <style type=\"text/css\"> "
            //             ".head404{ width:580px; height:234px; margin:50px auto 0 auto; background:url(https://www.daixiaorui.com/Public/images/head404.png) no-repeat; } "
            //             ".txtbg404{ width:499px; height:169px; margin:10px auto 0 auto; background:url(https://www.daixiaorui.com/Public/images/txtbg404.png) no-repeat;} "
            //             ".txtbg404 .txtbox{ width:390px; position:relative; top:30px; left:60px;color:#eee; font-size:13px;} .txtbg404 .txtbox p {margin:5px 0; line-height:18px;}"
            //             " .txtbg404 .txtbox .paddingbox { padding-top:15px;} .txtbg404 .txtbox p a { color:#eee; text-decoration:none;} .txtbg404 .txtbox p a:hover { color:#FC9D1D; "
            //             "text-decoration:underline;} </style> </head>  <body bgcolor=\"#494949\">    	<div class=\"head404\"></div>    	<div class=\"txtbg404\">  "
            //             " <div class=\"txtbox\">       <p>对不起，您请求的页面不存在、或已被删除、或暂时不可用</p>       <p class=\"paddingbox\">请点击以下链接继续浏览网页</p>     "
            //             "  <p>》<a style=\"cursor:pointer\" onclick=\"history.back()\">返回上一页面</a></p>       <p>》<a href=\"https://www.daixiaorui.com\">返回网站首页</a></p>   "
            //             "  </div>   </div> </body> </html>";
        }

        int32_t NotFoundServlet::handle(HttpRequest::ptr request, HttpResponse::ptr response, HttpSession::ptr session)
        {
            response->setStatus(HttpStatus::NOT_FOUND);
            response->setHeader("Server", "zdunk/1.0.0");
            response->setHeader("Content-Type", "text/html");
            response->setBody(m_content);
            return 0;
        }

    } // namespace http

} // namespace zdunk