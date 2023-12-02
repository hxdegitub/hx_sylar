#include "../hx_sylar/http/http.h"
#include "../hx_sylar/log.h"

void test_request() {
  hx_sylar::http::HttpRequest::ptr req(new hx_sylar::http::HttpRequest);
  req->setHeader("host", "www.sylar.top");
  req->setBody("hello sylar");
  req->dump(std::cout) << std::endl;
}

void test_response() {
  hx_sylar::http::HttpResponse::ptr rsp(new hx_sylar::http::HttpResponse);
  rsp->setHeader("X-X", "sylar");
  rsp->setBody("hello sylar");
  rsp->setStatus(static_cast<hx_sylar::http::HttpStatus>(400));
  rsp->setClose(false);

  rsp->dump(std::cout) << std::endl;
}

int main(int argc, char** argv) {
  test_request();
  test_response();
  return 0;
}