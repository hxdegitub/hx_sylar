#ifndef __HX_SYLAR_HTTP_SERVER__
#define __HX_SYLAR_HTTP_SERVER__
#include <memory>
#include <utility>

#include "http_session.h"
#include "hx_sylar/iomanager.h"
#include "servlet.h"
#include "tcp_server.h"
namespace hx_sylar::http {

class HttpServer : public TcpServer {
 public:
  using ptr = std::shared_ptr<HttpServer>;
  explicit HttpServer(
      bool keepalve = false,
      hx_sylar::IOManager* worker = hx_sylar::IOManager::GetThis(),
      hx_sylar::IOManager* io_worker = hx_sylar::IOManager::GetThis(),
      hx_sylar::IOManager* accept_worker = hx_sylar::IOManager::GetThis());

  // ServletDispatch::ptr getServletDispatch() const { return m_dispatch; }

  /**
   * @brief 设置ServletDispatch
   */
  // void setServletDispatch(ServletDispatch::ptr v) { m_dispatch = v; }

  // virtual void setName(const std::string& v) override;
  void setServletDispatch(ServletDispatch::ptr v) { m_dispatch = std::move(v); }
  auto getServletDispatch() const -> ServletDispatch::ptr { return m_dispatch; }

 protected:
  virtual void handleClient(Socket::ptr& client) override;

 private:
  /// 是否支持长连接
  bool m_isKeepalive;
  /// Servlet分发器
  ServletDispatch::ptr m_dispatch;
};
}  // namespace hx_sylar::http
#endif