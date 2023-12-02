#include "http_connection.h"

#include <utility>

#include "http_parser.h"
#include "http_session.h"
#include "hx_sylar/hook.h"
namespace hx_sylar::http {
HttpConnection::HttpConnection(Socket::ptr sock, bool owner)
    : SocketStream(std::move(sock), owner) {}

auto HttpConnection::recvResponse() -> HttpResponse::ptr {
  HttpResponseParser::ptr parser(new HttpResponseParser);
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
    size_t nparse = parser->execute(data, len, false);
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
  auto& client_parser = parser->getParser();
  if (client_parser.chunked != 0) {
    std::string body;
    int len = offset;
    do {
      do {
        int rt = read(data + len, buffer_size - len);
        if (rt <= 0) {
          return nullptr;
        }
        len += rt;
        size_t nparse = parser->execute(data, len, true);
        if (parser->hasError() != 0) {
          return nullptr;
        }
        len -= nparse;
        if (len == static_cast<int>(buffer_size)) {
          return nullptr;
        }
      } while (parser->isFinished() == 0);
      if (client_parser.content_len <= len) {
        body.append(data, client_parser.content_len);
        memmove(data, data + client_parser.content_len,
                len - client_parser.content_len);
        len -= client_parser.content_len;
      } else {
        body.append(data, len);
        len = 0;
        int left = client_parser.content_len - len;
        while (left > 0) {
          int rt = read(data, left > buffer_size ? buffer_size : left);
          if (rt <= 0) {
            return nullptr;
          }
          body.append(data, rt);
          left -= rt;
        }
        len = 0;
      }
    } while ((client_parser.chunks_done) == 0);
  } else {
    uint64_t length = parser->getContentLength();

    if (length > 0) {
      std::string body;
      body.reserve(length);
      int len = 0;
      if (length >= static_cast<uint64_t>(offset)) {
        memcpy(&body[0], data, offset);
        len = offset;
        //      body.append(data, offset);
      } else {
        body.append(data, length);
        memcpy(&body[0], data, length);
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
  }
  return parser->getData();
}
auto HttpConnection::sendRequest(HttpRequest::ptr rsp) -> int {
  std::stringstream ss;
  ss << *rsp;
  std::string data = ss.str();
  return writeFixSize(data.c_str(), data.size());
}
}  // namespace hx_sylar::http
