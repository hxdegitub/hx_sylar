#include "http_parser.h"

#include <cstddef>
#include <cstdint>
#include <cstring>

#include "../config.h"
#include "../log.h"
#include "../macro.h"
#include "http.h"
#include "http11_parser.h"
#include "httpclient_parser.h"
namespace hx_sylar {
namespace http {
static hx_sylar::Logger::ptr g_logger = HX_LOG_NAME("system");
static hx_sylar::ConfigVar<uint64_t>::ptr g_http_request_buffer_size =
    hx_sylar::Config::Lookup("http.request.buffer_size",
                             static_cast<uint64_t>(4 * 1024),
                             "http request buffer size");

static hx_sylar::ConfigVar<uint64_t>::ptr g_http_request_max_body_size =
    hx_sylar::Config::Lookup("http.request.max_body_size",
                             static_cast<uint64_t>(64 * 1024 * 1024),
                             "http request max body size");

static hx_sylar::ConfigVar<uint64_t>::ptr g_http_response_buffer_size =
    hx_sylar::Config::Lookup("http.response.buffer_size",
                             static_cast<uint64_t>(4 * 1024),
                             "http response buffer size");

static hx_sylar::ConfigVar<uint64_t>::ptr g_http_response_max_body_size =
    hx_sylar::Config::Lookup("http.response.max_body_size",
                             static_cast<uint64_t>(64 * 1024 * 1024),
                             "http response max body size");
static uint64_t s_http_request_buffer_size = 0;
static uint64_t s_http_request_max_body_size = 0;
static uint64_t s_http_response_buffer_size = 0;
static uint64_t s_http_response_max_body_size = 0;
auto HttpRequestParser::GetHttpRequestBufferSize() -> uint64_t {
  return s_http_request_buffer_size;
}

auto HttpRequestParser::GetHttpRequestMaxBodySize() -> uint64_t {
  return s_http_response_max_body_size;
}

auto HttpResponseParser::GetHttpResponseBufferSize() -> uint64_t {
  return s_http_response_buffer_size;
}

auto HttpResponseParser::GetHttpResponseMaxBodySize() -> uint64_t {
  return s_http_response_max_body_size;
}
struct _RequestSizIniter {
  _RequestSizIniter() {
    s_http_request_buffer_size = g_http_request_buffer_size->getValue();
    s_http_request_max_body_size = g_http_request_max_body_size->getValue();

    g_http_request_buffer_size->addListener(
        [](const uint64_t& ov, const uint64_t& nv) {
          s_http_request_buffer_size = nv;
        });

    g_http_request_max_body_size->addListener(
        [](const uint64_t& ov, const uint64_t& nv) {
          s_http_request_max_body_size = nv;
        });

    g_http_response_buffer_size->addListener(
        [](const uint64_t& ov, const uint64_t& nv) {
          s_http_response_buffer_size = nv;
        });

    g_http_response_max_body_size->addListener(
        [](const uint64_t& ov, const uint64_t& nv) {
          s_http_response_max_body_size = nv;
        });
  }
};
static _RequestSizIniter _init;

void on_request_method(void* data, const char* at, size_t vlen) {
  auto* parser = static_cast<HttpRequestParser*>(data);
  HttpMethod m = CharsToHttpMethod(at);

  if (m == HttpMethod::INVALID_METHOD) {
    HX_LOG_WARN(g_logger) << "invalid http request method: "
                          << std::string(at, vlen);
    parser->setError(1000);
    return;
  }
  parser->getData()->setMethod(m);
}
void on_request_uri(void* data, const char* value, size_t vlen) {}
void on_request_fragment(void* data, const char* at, size_t length) {
  // SYLAR_LOG_INFO(g_logger) << "on_request_fragment:" << std::string(at,
  // length);
  auto* parser = static_cast<HttpRequestParser*>(data);
  parser->getData()->setFragment(std::string(at, length));
}

void on_request_path(void* data, const char* at, size_t length) {
  auto* parser = static_cast<HttpRequestParser*>(data);
  parser->getData()->setPath(std::string(at, length));
}

void on_request_query(void* data, const char* at, size_t length) {
  auto* parser = static_cast<HttpRequestParser*>(data);
  parser->getData()->setQuery(std::string(at, length));
}
void on_request_version(void* data, const char* value, size_t vlen) {}
void on_request_header_done(void* data, const char* value, size_t vlen) {}
void on_request_http_field(void* data, const char* value, size_t vlen,
                           const char* buffer, size_t length) {}

HttpRequestParser::HttpRequestParser() : m_error(0) {
  m_data.reset(new hx_sylar::http::HttpRequest());
  http_parser_init(&m_parser);
  m_parser.request_method = on_request_method;
  m_parser.request_uri = on_request_uri;
  m_parser.fragment = on_request_fragment;
  m_parser.request_path = on_request_path;
  m_parser.query_string = on_request_query;
  m_parser.http_version = on_request_version;
  m_parser.header_done = on_request_header_done;
  m_parser.http_field = on_request_http_field;
  m_parser.data = this;
}
auto HttpRequestParser::getContentLength() -> uint64_t {
  return m_data->getHeaderAs<uint64_t>("content-length", 0);
}

auto HttpRequestParser::execute(char* data, size_t len) -> size_t {
  size_t offset = http_parser_execute(&m_parser, data, len, 0);
  memmove(data, data + offset, (len - offset));
  return offset;
}

auto HttpRequestParser::isFinished() -> int {
  return http_parser_finish(&m_parser);
}
void on_response_reason(void* data, const char* at, size_t length) {
  HttpResponseParser* parser = static_cast<HttpResponseParser*>(data);
  parser->getData()->setReason(std::string(at, length));
}
auto HttpRequestParser::hasError() -> int {
  return (m_error != 0) || http_parser_has_error(&m_parser);
}

void on_response_status(void* data, const char* at, size_t length) {
  HttpResponseParser* parser = static_cast<HttpResponseParser*>(data);
  HttpStatus status = (HttpStatus)(atoi(at));
  parser->getData()->setStatus(status);
}

void on_response_chunk(void* data, const char* at, size_t length) {}

void on_response_version(void* data, const char* at, size_t length) {
  auto* parser = static_cast<HttpResponseParser*>(data);
  uint8_t v = 0;
  if (strncmp(at, "HTTP/1.1", length) == 0) {
    v = 0x11;
  } else if (strncmp(at, "HTTP/1.0", length) == 0) {
    v = 0x10;
  } else {
    HX_LOG_WARN(g_logger) << "invalid http response version: "
                          << std::string(at, length);
    parser->setError(1001);
    return;
  }
  parser->getData()->setVersion(v);
}

void on_response_header_done(void* data, const char* at, size_t length) {}

void on_response_last_chunk(void* data, const char* at, size_t length) {}

void on_response_http_field(void* data, const char* field, size_t flen,
                            const char* value, size_t vlen) {
  HttpResponseParser* parser = static_cast<HttpResponseParser*>(data);
  if (flen == 0) {
    HX_LOG_WARN(g_logger) << "invalid http response field length == 0";
    // parser->setError(1002);
    return;
  }
  parser->getData()->setHeader(std::string(field, flen),
                               std::string(value, vlen));
}

HttpResponseParser::HttpResponseParser() : m_error(0) {
  m_data.reset(new hx_sylar::http::HttpResponse);
  httpclient_parser_init(&m_parser);
  m_parser.reason_phrase = on_response_reason;
  m_parser.status_code = on_response_status;
  m_parser.chunk_size = on_response_chunk;
  m_parser.http_version = on_response_version;
  m_parser.header_done = on_response_header_done;
  m_parser.last_chunk = on_response_last_chunk;
  m_parser.http_field = on_response_http_field;
  m_parser.data = this;
}
auto HttpResponseParser::execute(char* data, size_t len, bool chunk) -> size_t {
  if (chunk) {
    httpclient_parser_init(&m_parser);
  }
  size_t offset = httpclient_parser_execute(&m_parser, data, len, 0);

  memmove(data, data + offset, (len - offset));
  return offset;
}

auto HttpResponseParser::isFinished() -> int {
  return httpclient_parser_finish(&m_parser);
}
int HttpResponseParser::hasError() {
  return m_error || httpclient_parser_has_error(&m_parser);
}
auto HttpResponseParser::getContentLength() -> uint64_t {
  return m_data->getHeaderAs<uint64_t>("content-length", 0);
}

}  // namespace http
}  // namespace hx_sylar