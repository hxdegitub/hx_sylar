#include "../hx_sylar/http/http.h"
#include "../hx_sylar/http/http_session.h"
#include "../hx_sylar/log.h"
#include "hx_sylar/http/http_server.h"

static hx_sylar::Logger::ptr g_logger = HX_LOG_ROOT();

#define XX(...) #__VA_ARGS__

hx_sylar::IOManager::ptr worker;
void run() {
  g_logger->setLevel(hx_sylar::LogLevel::INFO);
  // hx_sylar::http::HttpServer::ptr server(new hx_sylar::http::HttpServer(true,
  // worker.get(), hx_sylar::IOManager::GetThis()));
  hx_sylar::http::HttpServer::ptr server(new hx_sylar::http::HttpServer(true));
  hx_sylar::Address::ptr addr =
      hx_sylar::Address::LookupAnyIPAddress("0.0.0.0:8020");
  while (!server->bind(addr)) {
    sleep(2);
  }
  //   auto sd = server->getServletDispatch();
  //   sd->addServlet("/hx_sylar/xx", [](hx_sylar::http::HttpRequest::ptr req,
  //                                     hx_sylar::http::HttpResponse::ptr rsp,
  //                                     hx_sylar::http::HttpSession::ptr
  //                                     session) {
  //     rsp->setBody(req->toString());
  //     return 0;
  //   });

  //   sd->addGlobServlet("/hx_sylar/*",
  //                      [](hx_sylar::http::HttpRequest::ptr req,
  //                         hx_sylar::http::HttpResponse::ptr rsp,
  //                         hx_sylar::http::HttpSession::ptr session) {
  //                        rsp->setBody("Glob:\r\n" + req->toString());
  //                        return 0;
  //                      });

  //   sd->addGlobServlet("/sylarx/*", [](hx_sylar::http::HttpRequest::ptr req,
  //                                      hx_sylar::http::HttpResponse::ptr rsp,
  //                                      hx_sylar::http::HttpSession::ptr
  //                                      session) {
  //     rsp->setBody(
  //         XX(<html><head><title> 404 Not Found</ title></ head><body><center>
  //                    <h1> 404 Not Found</ h1></ center><hr><center>
  //                        nginx /
  //                    1.16.0 <
  //                / center > </ body></ html> < !--a padding to disable MSIE
  //                and
  //            Chrome friendly error page-- > < !--a padding to disable MSIE
  //            and Chrome friendly error page-- > < !--a padding to disable
  //            MSIE and Chrome friendly error page-- > < !--a padding to
  //            disable MSIE and Chrome friendly error page-- > < !--a padding
  //            to disable MSIE and Chrome friendly error page-- > < !--a
  //            padding to disable MSIE and Chrome friendly error page-- >));
  //     return 0;
  //   });

  server->start();
}

int main(int argc, char** argv) {
  hx_sylar::IOManager iom(1, true, "main");
  worker.reset(new hx_sylar::IOManager(3, false, "worker"));
  iom.schedule(run);
  return 0;
}
