#include "http.h"

#include <strings.h>

#include <cstdint>
#include <memory>
#include <ostream>

#include "../util.h"
namespace hx_sylar::http {
auto StringToMethod(const std::string& m) -> HttpMethod {
#define XX(num, name, string)            \
  if (strcmp(#string, m.c_str()) == 0) { \
    return HttpMethod::name;             \
  }                                      \
  HTTP_METHOD_MAP(XX);
#undef XX
  return HttpMethod::INVALID_METHOD;
}
auto CharsToHttpMethod(const char* m) -> HttpMethod {
#define XX(num, name, string)                     \
  if (strcmp(#string, m, strlen(#string)) == 0) { \
    return HttpMethod::name;                      \
  }                                               \
  HTTP_METHOD_MAP(XX);
#undef XX
  return HttpMethod::INVALID_METHOD;
}
static const char* s_method_string[] = {
#define XX(num, name, string) #string,
    HTTP_METHOD_MAP(XX)
#undef XX
};

auto HttpMethodToString(const HttpMethod& m) -> const char* {
  auto idx = static_cast<uint32_t>(m);
  if (idx >= (sizeof(s_method_string) / sizeof(s_method_string[0]))) {
    return "<unknow>";
  }
  return s_method_string[idx];
}

auto HttpStatusToString(const HttpStatus& s) -> const char* {
  switch (s) {
#define XX(code, name, msg) \
  case HttpStatus::name:    \
    return #msg;
    HTTP_STATUS_MAP(XX);
#undef XX
    default:
      return "<unknown>";
  }
}

auto CaseInsensitiveLess::operator()(const std::string& lhs,
                                     const std::string& rhs) const -> bool {
  return strcasecmp(lhs.c_str(), rhs.c_str()) < 0;
}

HttpRequest::HttpRequest(uint8_t version, bool close)
    : m_method(HttpMethod::GET),
      m_version(version),
      m_close(close),
      m_websocket(false),
      m_parserParamFlag(0),
      m_path("/") {}
auto HttpRequest::createResponse() -> std::shared_ptr<HttpResponse> {
  HttpResponse::ptr rsp =
      std::make_shared<HttpResponse>(getVersion(), isClose());

  return rsp;
}

auto HttpRequest::getParam(const std::string& key, const std::string& def)
    -> std::string {
  initQueryParam();
  initBodyParam();
  auto it = m_params.find(key);
  return it == m_params.end() ? def : it->second;
}
auto HttpRequest::getCookie(const std::string& key, const std::string& def)
    -> std::string {
  initCookies();
  auto it = m_cookies.find(key);
  return it == m_cookies.end() ? def : it->second;
}

void HttpRequest::setHeader(const std::string& key, const std::string& val) {
  m_headers[key] = val;
}

void HttpRequest::setParam(const std::string& key, const std::string& val) {
  m_params[key] = val;
}

void HttpRequest::setCookie(const std::string& key, const std::string& val) {
  m_cookies[key] = val;
}

void HttpRequest::delHeader(const std::string& key) { m_headers.erase(key); }

void HttpRequest::delParam(const std::string& key) { m_params.erase(key); }

void HttpRequest::delCookie(const std::string& key) { m_cookies.erase(key); }
auto HttpRequest::hasHeader(const std::string& key, std::string* val) -> bool {
  auto it = m_headers.find(key);
  if (it == m_headers.end()) {
    return false;
  }
  if (val != nullptr) {
    *val = it->second;
  }
  return true;
}

auto HttpRequest::hasParam(const std::string& key, std::string* val) -> bool {
  initQueryParam();
  initBodyParam();
  auto it = m_params.find(key);
  if (it == m_params.end()) {
    return false;
  }
  if (val != nullptr) {
    *val = it->second;
  }
  return true;
}

auto HttpRequest::hasCookie(const std::string& key, std::string* val) -> bool {
  initCookies();
  auto it = m_cookies.find(key);
  if (it == m_cookies.end()) {
    return false;
  }
  if (val != nullptr) {
    *val = it->second;
  }
  return true;
}
auto HttpRequest::toString() const -> std::string {
  std::stringstream ss;
  dump(ss);
  return ss.str();
}

void HttpRequest::initQueryParam() {
#define PARSE_PARAM(str, m, flag, trim)                                    \
  size_t pos = 0;                                                          \
  do {                                                                     \
    size_t last = pos;                                                     \
    pos = (str).find('=', pos);                                            \
    if (pos == std::string::npos) {                                        \
      break;                                                               \
    }                                                                      \
    size_t key = pos;                                                      \
    pos = (str).find(flag, pos);                                           \
    (m).insert(std::make_pair(trim((str).substr(last, key - last)),        \
                              hx_sylar::StringUtil::UrlDecode(             \
                                  (str).substr(key + 1, pos - key - 1)))); \
    if (pos == std::string::npos) {                                        \
      break;                                                               \
    }                                                                      \
    ++pos;                                                                 \
  } while (true);

  PARSE_PARAM(m_query, m_params, '&', );
  m_parserParamFlag |= 0x1;
}
auto HttpRequest::dump(std::ostream& os) const -> std::ostream& {
  os << HttpMethodToString(m_method) << " " << m_path
     << (m_query.empty() ? "" : "?") << m_query
     << (m_fragment.empty() ? "" : "#") << m_fragment << " HTTP/"
     << (static_cast<uint32_t>(m_version >> 4)) << "."
     << (static_cast<uint32_t>(m_version & 0x0f)) << "\r\n";

  if (!m_websocket) {
    os << "connection: " << (m_close ? "close" : "keep-alive") << "\r\n";
  }

  for (auto& i : m_headers) {
    if (!m_websocket && strcasecmp(i.first.c_str(), "connection") == 0) {
      continue;
    }
    os << i.first << ": " << i.second << "\r\n";
  }

  if (!m_body.empty()) {
    os << "content-length: " << m_body.size() << "\r\n\r\n" << m_body;
  } else {
    os << "\r\n";
  }
  return os;
}

void HttpRequest::initBodyParam() {
  if ((m_parserParamFlag & 0x2) != 0) {
    return;
  }
  std::string content_type = getHeader("content-type");
  if (strcasestr(content_type.c_str(), "application/x-www-form-urlencoded") ==
      nullptr) {
    m_parserParamFlag |= 0x2;
    return;
  }
  PARSE_PARAM(m_body, m_params, '&', );
  m_parserParamFlag |= 0x2;
}
void HttpRequest::init() {
  std::string conn = getHeader("connection");
  if (!conn.empty()) {
    m_close = strcasecmp(conn.c_str(), "keep-alive") != 0;
  }
}
auto HttpRequest::getHeader(const std::string& key,
                            const std::string& def) const -> std::string {
  auto it = m_headers.find(key);
  return it == m_headers.end() ? def : it->second;
}
void HttpRequest::initParam() {
  initQueryParam();
  initBodyParam();
  initCookies();
}

void HttpRequest::initCookies() {
  if ((m_parserParamFlag & 0x4) != 0) {
    return;
  }
  std::string cookie = getHeader("cookie");
  if (cookie.empty()) {
    m_parserParamFlag |= 0x4;
    return;
  }
  PARSE_PARAM(cookie, m_cookies, ';', hx_sylar::StringUtil::Trim);
  m_parserParamFlag |= 0x4;
}
HttpResponse::HttpResponse(uint8_t version, bool close)
    : m_status(HttpStatus::OK), m_version(version), m_close(close) {}

auto HttpResponse::getHeader(const std::string& key,
                             const std::string& def) const -> std::string {
  auto it = m_headers.find(key);
  return it == m_headers.end() ? def : it->second;
}

void HttpResponse::setHeader(const std::string& key, const std::string& val) {
  m_headers[key] = val;
}

void HttpResponse::delHeader(const std::string& key) { m_headers.erase(key); }

void HttpResponse::setRedirect(const std::string& uri) {
  m_status = HttpStatus::FOUND;
  setHeader("Location", uri);
}
void HttpResponse::setCookie(const std::string& key, const std::string& val,
                             time_t expired, const std::string& path,
                             const std::string& domain, bool secure) {
  std::stringstream ss;
  ss << key << "=" << val;
  if (expired > 0) {
    // ss << ";expires=" << hx_sylar::Time2Str(expired, "%a, %d %b %Y %H:%M:%S")
    //    << " GMT";
  }
  if (!domain.empty()) {
    ss << ";domain=" << domain;
  }
  if (!path.empty()) {
    ss << ";path=" << path;
  }
  if (secure) {
    ss << ";secure";
  }
  m_cookies.push_back(ss.str());
}
auto HttpResponse::toString() const -> std::string {
  std::stringstream ss;
  dump(ss);
  return ss.str();
}
auto HttpResponse::dump(std::ostream& os) const -> std::ostream& {
  os << "HTTP/" << (static_cast<uint32_t>(m_version >> 4)) << "."
     << (static_cast<uint32_t>(m_version & 0x0F)) << " "
     << static_cast<uint32_t>(m_status) << " "
     << (m_reason.empty() ? HttpStatusToString(m_status) : m_reason) << "\r\n";

  for (auto& i : m_headers) {
    if (!m_websocket && strcasecmp(i.first.c_str(), "connection") == 0) {
      continue;
    }
    os << i.first << ": " << i.second << "\r\n";
  }
  for (auto& i : m_cookies) {
    os << "Set-Cookie: " << i << "\r\n";
  }
  if (!m_websocket) {
    os << "connection: " << (m_close ? "close" : "keep-alive") << "\r\n";
  }
  if (!m_body.empty()) {
    os << "content-length: " << m_body.size() << "\r\n\r\n" << m_body;
  } else {
    os << "\r\n";
  }
  return os;
}

auto operator<<(std::ostream& os, const HttpRequest& req) -> std::ostream& {
  return req.dump(os);
}

auto operator<<(std::ostream& os, const HttpResponse& rsp) -> std::ostream& {
  return rsp.dump(os);
}
}  // namespace hx_sylar::http