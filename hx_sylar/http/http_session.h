#ifndef __HX_SYLAR_HTTP_SESSION__
#define __HX_SYLAR_HTTP_SESSION__

#include "http.h"
#include "hx_sylar/stream/socket_stream.h"
namespace hx_sylar::http {
class HttpSession : public SocketStream {
 public:
  using ptr = std::shared_ptr<HttpSession>;
  explicit HttpSession(Socket::ptr sock, bool owner = true);
  auto recvRequest() -> HttpRequest::ptr;
  auto sendResponse(http::HttpResponse::ptr rsp) -> int;
};
}  // namespace hx_sylar::http
#endif
