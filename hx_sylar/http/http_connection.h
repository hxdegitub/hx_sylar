#ifndef __HX_SYLAR_HTTP_CONNECTION__
#define __HX_SYLAR_HTTP_CONNECTION__

#include "http.h"
#include "hx_sylar/http/http_session.h"
#include "hx_sylar/socket.h"
#include "hx_sylar/stream/socket_stream.h"
namespace hx_sylar::http {
class HttpConnection : public SocketStream {
 public:
  using ptr = std::shared_ptr<HttpConnection>;
  explicit HttpConnection(Socket::ptr sock, bool owner = true);
  auto recvResponse() -> HttpResponse::ptr;
  auto sendRequest(http::HttpRequest::ptr rsp) -> int;
};
}  // namespace hx_sylar::http
#endif
