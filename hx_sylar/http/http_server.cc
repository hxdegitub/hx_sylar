#include "http_server.h"

#include <memory>

#include "hx_sylar/http/http.h"
#include "hx_sylar/http/http_session.h"
#include "hx_sylar/http/tcp_server.h"
#include "hx_sylar/iomanager.h"
#include "hx_sylar/log.h"
#include "hx_sylar/socket.h"
namespace hx_sylar::http {
static hx_sylar::Logger::ptr g_logger = HX_LOG_NAME("system");

// HttpServer::HttpServer(bool keepalive, hx_sylar::IOManager* worker,
//                        hx_sylar::IOManager* io_worker,
//                        hx_sylar::IOManager* accept_worker)
//     : TcpServer(worker, accept_worker), m_isKeepalive(keepalive) {}
HttpServer::HttpServer(bool keepalive, hx_sylar::IOManager* worker,
                       hx_sylar::IOManager* io_worker,
                       hx_sylar::IOManager* accept_worker)
    : TcpServer(worker, accept_worker) {}
void HttpServer::handleClient(Socket::ptr& client) {
  HttpSession::ptr session(new HttpSession(client));

  do {
    auto req = session->recvRequest();
    if (!req) {
      HX_LOG_WARN(g_logger)
          << "recv http request fail, errno =" << errno
          << " errstr=" << strerror(errno) << " client: " << *client;
      break;
    }
    HttpResponse::ptr rsp = std::make_shared<HttpResponse>(
        req->getVersion(), req->isClose() || !m_isKeepalive);

    rsp->setBody("hello sylar");

    session->sendResponse(rsp);
  } while (m_isKeepalive);
  session->close();
}
// void HttpServer::setName(const std::string& v) {
//   TcpServer::setName(v);
//   //   m_dispatch->setDefault(std::make_shared<NotFoundServlet>(v));
// }

}  // namespace hx_sylar::http