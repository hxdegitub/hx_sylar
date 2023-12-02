#include "http_session.h"

#include <utility>

#include "http_parser.h"
#include "hx_sylar/hook.h"

namespace hx_sylar::http {
HttpSession::HttpSession(Socket::ptr sock, bool owner)
    : SocketStream(std::move(sock), owner) {}

auto HttpSession::recvRequest() -> HttpRequest::ptr {
  HttpRequestParser::ptr parser(new HttpRequestParser);
  uint64_t buffer_size = HttpRequestParser::GetHttpRequestBufferSize();
  std::shared_ptr<char> buffer(new char[buffer_size],
                               [](const char* ptr) { delete[] ptr; });
  char* data = buffer.get();
  int offset = 0;
  do {
    int len = read(data + offset, buffer_size - offset);
    if (len <= 0) {
      return nullptr;
    }
    len += offset;
    size_t nparse = parser->execute(data, len);
    if (parser->hasError() != 0) {
      return nullptr;
    }
    offset = len - nparse;
    //  lne += offset;
    if (offset == static_cast<int>(buffer_size)) {
      return nullptr;
    }
    if (parser->isFinished() != 0) {
      break;
    }
  } while (true);
  uint64_t length = parser->getContentLength();
  if (length > 0) {
    std::string body;
    body.reserve(length);
    int len = 0;
    if (length >= static_cast<uint64_t>(offset)) {
      memcpy(&body[0], data,offset);
      len = offset;
//      body.append(data, offset);
    } else {
      body.append(data, length);
      memcpy(&body[0],data,length);
    }
    //    body.append(data,offset);
    length -= offset;
    if (length > 0) {
      if (read(&body[len], length) <= 0) {
        return nullptr;
      }
    }
    parser->getData()->setBody(body);
  }
  return parser->getData();
}
auto HttpSession::sendResponse(HttpResponse::ptr rsp) -> int {
  std::stringstream ss;
  ss << *rsp;
  std::string data = ss.str();
  return writeFixSize(data.c_str(), data.size());
}
}  // namespace hx_sylar::http
