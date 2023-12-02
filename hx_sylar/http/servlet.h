#ifndef _HX_SYALR_HTTP_SERVLET_H__
#define _HX_SYALR_HTTP_SERVLET_H__
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "../thread.h"
#include "http.h"
#include "http_session.h"
namespace hx_sylar::http {
class Servlet {
 public:
  using ptr = std::shared_ptr<Servlet>;

  Servlet(std::string name) : m_name(std::move(name)) {}

  virtual ~Servlet() = default;
  virtual auto handle(hx_sylar::http::HttpRequest::ptr request,
                      hx_sylar::http::HttpResponse::ptr response,
                      hx_sylar::http::HttpSession::ptr session) -> int32_t = 0;
  virtual auto getName() const -> const std::string& { return m_name; }

 protected:
  std::string m_name;
};
class FunctionServlet : public Servlet {
 public:
  using ptr = std::shared_ptr<FunctionServlet>;
  using callback = std::function<int32_t(hx_sylar::http::HttpRequest::ptr,
                                         hx_sylar::http::HttpResponse::ptr,
                                         hx_sylar::http::HttpSession::ptr)>;
  explicit FunctionServlet(callback cb);
  virtual auto handle(hx_sylar::http::HttpRequest::ptr request,
                      hx_sylar::http::HttpResponse::ptr response,
                      hx_sylar::http::HttpSession::ptr session)
      -> int32_t override;

 private:
  callback m_cb;
};
/*.
class IServletCreator{
  public :
    using ptr =std::shared_ptr<IServletCreator> ptr;
    virtual ~IServletCreator() {}
    virtual Servlet::ptr get() const = 0;
    virtual std::string getName() const = 0;
};

class HoldServletCreator: public IServletCreator{
  public :
    using  ptr = std::shared_ptr<HoldServletCreator>;
    HoldServletCreator(Servlet::ptr slt) :m_servlet(slt){}
    Servlet::ptr get() const override {
      return m_servlet;
    }

    std::string getName() const override {
      return m_servlet->getName();
    }
  private:
    Servlet::ptr m_servlet;
};
*/
/*
template<class T>
class ServletCreator: public IServletCreator{
  public :
    using ptr = std::shared_ptr<ServletCreator> ptr;
    ServletCreator(){}

}
*/

class ServletDispatch : public Servlet {
 public:
  using ptr = std::shared_ptr<ServletDispatch>;
  using RWMutexType = RWMutex;
  ServletDispatch();
  virtual auto handle(hx_sylar::http::HttpRequest::ptr request,
                      hx_sylar::http::HttpResponse::ptr response,
                      hx_sylar::http::HttpSession::ptr session)
      -> int32_t override;

  void addServlet(const std::string& uir, Servlet::ptr slt);
  void addServlet(const std::string& uri, FunctionServlet::callback cb);
  void addGlobServlet(const std::string& uri, Servlet::ptr sl);
  void addGlobServlet(const std::string& uri, FunctionServlet::callback cb);

  void delServlet(const std::string& uri);
  void delGlobServlet(const std::string& uri);

  Servlet::ptr getServlet(const std::string& uri);
  Servlet::ptr getGlobServlet(const std::string& uri);

  Servlet::ptr getDefault() const { return m_default; }
  void setDefault(Servlet::ptr v) { m_default = std::move(v); }

  Servlet::ptr getMatchServlet(const std::string& uri);

 private:
  RWMutexType m_mutex;
  std::unordered_map<std::string, Servlet::ptr> m_datas;
  std::vector<std::pair<std::string, Servlet::ptr> > m_globs;
  Servlet::ptr m_default;
};

class NotFoundServlet : public Servlet {
 public:
  /// 智能指针类型定义
  using ptr = std::shared_ptr<NotFoundServlet>;

  /**
   * @brief 构造函数
   */
  NotFoundServlet(const std::string& name);
  virtual int32_t handle(hx_sylar::http::HttpRequest::ptr request,
                         hx_sylar::http::HttpResponse::ptr response,
                         hx_sylar::http::HttpSession::ptr session) override;

 private:
  std::string m_name;
  std::string m_content;
};
}  // namespace hx_sylar::http
#endif
