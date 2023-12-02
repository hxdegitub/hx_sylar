#ifndef __HX_SYLAR_HTTP_PARSER_H__
#define __HX_SYLAR_HTTP_PARSER_H__

#include "http.h"
#include "http11_parser.h"
#include "httpclient_parser.h"
namespace hx_sylar {
namespace http {
class HttpRequestParser {
 public:
  using ptr = std::shared_ptr<HttpRequestParser>;
  HttpRequestParser();
  auto execute(char *data, size_t len) -> size_t;

  auto isFinished() -> int;

  auto getData() const -> HttpRequest::ptr { return m_data; }
  auto hasError() -> int;
  void setError(int v) { m_error = v; }
  auto getContentLength() -> uint64_t;
  auto getParser() const { return m_parser; }

 public:
  static auto GetHttpRequestBufferSize() -> uint64_t;
  static auto GetHttpRequestMaxBodySize() -> uint64_t;

 private:
  http_parser m_parser;
  HttpRequest::ptr m_data;
  int m_error;
};

class HttpResponseParser {
 public:
  using ptr = std::shared_ptr<HttpResponseParser>;
  HttpResponseParser();

  auto execute(char *data, size_t len, bool chunk) -> size_t;
  auto isFinished() -> int;

  auto hasError() -> int;

  auto getData() const -> HttpResponse::ptr { return m_data; }

  void setError(int v) { m_error = v; }
  auto getContentLength() -> uint64_t;

  auto getParser() const -> const httpclient_parser & { return m_parser; }

 public:
  static auto GetHttpResponseBufferSize() -> uint64_t;
  static auto GetHttpResponseMaxBodySize() -> uint64_t;

 private:
  httpclient_parser m_parser;
  HttpResponse::ptr m_data;
  int m_error;
};
}  // namespace http
}  // namespace hx_sylar
#endif