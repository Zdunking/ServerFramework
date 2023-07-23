#pragma once
#include <cxxabi.h>
#include <memory>
#include <functional>
#include <string>
#include <vector>
#include <unordered_map>
#include "http.h"
#include "http_session.h"
#include "../thread.h"
#include "../utils.h"

namespace zdunk
{
    namespace http
    {
        class Servlet
        {
        public:
            typedef std::shared_ptr<Servlet> ptr;

            Servlet(const std::string &name){};
            virtual ~Servlet(){};
            virtual int32_t handle(HttpRequest::ptr request, HttpResponse::ptr response, HttpSession::ptr session) = 0;

            const std::string &getName() const { return m_name; }

        protected:
            std::string m_name;
        };

        class FunctionServlet : public Servlet
        {
        public:
            typedef std::shared_ptr<FunctionServlet> ptr;
            typedef std::function<int32_t(HttpRequest::ptr request, HttpResponse::ptr response, HttpSession::ptr session)> callback;

            FunctionServlet(callback cb);

            virtual int32_t handle(HttpRequest::ptr request, HttpResponse::ptr response, HttpSession::ptr session) override;

        private:
            callback m_cb;
        };

        class IServletCreator
        {
        public:
            typedef std::shared_ptr<IServletCreator> ptr;
            virtual ~IServletCreator() {}
            virtual Servlet::ptr get() const = 0;
            virtual std::string getName() const = 0;
        };

        class HoldServletCreator : public IServletCreator
        {
        public:
            typedef std::shared_ptr<HoldServletCreator> ptr;
            HoldServletCreator(Servlet::ptr slt)
                : m_servlet(slt)
            {
            }

            Servlet::ptr get() const override
            {
                return m_servlet;
            }

            std::string getName() const override
            {
                return m_servlet->getName();
            }

        private:
            Servlet::ptr m_servlet;
        };

        template <class T>
        class ServletCreator : public IServletCreator
        {
        public:
            typedef std::shared_ptr<ServletCreator> ptr;

            ServletCreator()
            {
            }

            Servlet::ptr get() const override
            {
                return Servlet::ptr(new T);
            }

            std::string getName() const override
            {
                return TypeToName<T>();
            }
        };

        class ServletDispatch : public Servlet
        {
        public:
            typedef std::shared_ptr<ServletDispatch> ptr;
            typedef RWMutex RWMutexType;

            ServletDispatch();

            virtual int32_t handle(HttpRequest::ptr request, HttpResponse::ptr response, HttpSession::ptr session) override;

            void addServlet(const std::string uri, Servlet::ptr slt);
            void addServlet(const std::string uri, FunctionServlet::callback cb);
            void addGlobServlet(const std::string uri, Servlet::ptr slt);
            void addGlobServlet(const std::string uri, FunctionServlet::callback cb);

            void delServlet(const std::string uri, Servlet::ptr slt);
            void delGlobServlet(const std::string uri, Servlet::ptr slt);

            Servlet::ptr getDefault(const std::string &uri) const { return m_default; }
            void setDefault(Servlet::ptr v) { m_default = v; }

            Servlet::ptr getServlet(const std::string &uri);
            Servlet::ptr getGlobServlet(const std::string &uri);

            Servlet::ptr getMatchedServlet(const std::string uri);

        private:
            // uri(精准)--> servlet
            std::unordered_map<std::string, Servlet::ptr> m_datas;
            // uri(模糊)--> servlet
            std::vector<std::pair<std::string, Servlet::ptr>> m_globs;
            // 默认
            Servlet::ptr m_default;

            RWMutexType m_mutex;
        };

        class NotFoundServlet : public Servlet
        {
        public:
            typedef std::shared_ptr<NotFoundServlet> ptr;
            NotFoundServlet(const std::string &name);
            virtual int32_t handle(HttpRequest::ptr request, HttpResponse::ptr response, HttpSession::ptr session) override;

        private:
            std::string m_name;
            std::string m_content;
        };

    } // namespace http

} // namespace zdunk
