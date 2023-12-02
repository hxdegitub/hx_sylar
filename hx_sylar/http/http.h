#ifndef __HX_SYLAR_HTTP_H__
#define __HX_SYLAR_HTTP_H__

#include <boost/lexical_cast.hpp>
#include <iostream>
#include <map>
#include <memory>
#include <ostream>
#include <sstream>
#include <string>
#include <vector>

#include "http11_parser.h"

namespace hx_sylar {
namespace http {
/* Request Methods */
#define HTTP_METHOD_MAP(XX)        \
  XX(0, DELETE, DELETE)            \
  XX(1, GET, GET)                  \
  XX(2, HEAD, HEAD)                \
  XX(3, POST, POST)                \
  XX(4, PUT, PUT)                  \
  /* pathological */               \
  XX(5, CONNECT, CONNECT)          \
  XX(6, OPTIONS, OPTIONS)          \
  XX(7, TRACE, TRACE)              \
  /* WebDAV */                     \
  XX(8, COPY, COPY)                \
  XX(9, LOCK, LOCK)                \
  XX(10, MKCOL, MKCOL)             \
  XX(11, MOVE, MOVE)               \
  XX(12, PROPFIND, PROPFIND)       \
  XX(13, PROPPATCH, PROPPATCH)     \
  XX(14, SEARCH, SEARCH)           \
  XX(15, UNLOCK, UNLOCK)           \
  XX(16, BIND, BIND)               \
  XX(17, REBIND, REBIND)           \
  XX(18, UNBIND, UNBIND)           \
  XX(19, ACL, ACL)                 \
  /* subversion */                 \
  XX(20, REPORT, REPORT)           \
  XX(21, MKACTIVITY, MKACTIVITY)   \
  XX(22, CHECKOUT, CHECKOUT)       \
  XX(23, MERGE, MERGE)             \
  /* upnp */                       \
  XX(24, MSEARCH, M - SEARCH)      \
  XX(25, NOTIFY, NOTIFY)           \
  XX(26, SUBSCRIBE, SUBSCRIBE)     \
  XX(27, UNSUBSCRIBE, UNSUBSCRIBE) \
  /* RFC-5789 */                   \
  XX(28, PATCH, PATCH)             \
  XX(29, PURGE, PURGE)             \
  /* CalDAV */                     \
  XX(30, MKCALENDAR, MKCALENDAR)   \
  /* RFC-2068, section 19.6.1.2 */ \
  XX(31, LINK, LINK)               \
  XX(32, UNLINK, UNLINK)           \
  /* icecast */                    \
  XX(33, SOURCE, SOURCE)

/* Status Codes */
#define HTTP_STATUS_MAP(XX)                                                 \
  XX(100, CONTINUE, Continue)                                               \
  XX(101, SWITCHING_PROTOCOLS, Switching Protocols)                         \
  XX(102, PROCESSING, Processing)                                           \
  XX(200, OK, OK)                                                           \
  XX(201, CREATED, Created)                                                 \
  XX(202, ACCEPTED, Accepted)                                               \
  XX(203, NON_AUTHORITATIVE_INFORMATION, Non - Authoritative Information)   \
  XX(204, NO_CONTENT, No Content)                                           \
  XX(205, RESET_CONTENT, Reset Content)                                     \
  XX(206, PARTIAL_CONTENT, Partial Content)                                 \
  XX(207, MULTI_STATUS, Multi - Status)                                     \
  XX(208, ALREADY_REPORTED, Already Reported)                               \
  XX(226, IM_USED, IM Used)                                                 \
  XX(300, MULTIPLE_CHOICES, Multiple Choices)                               \
  XX(301, MOVED_PERMANENTLY, Moved Permanently)                             \
  XX(302, FOUND, Found)                                                     \
  XX(303, SEE_OTHER, See Other)                                             \
  XX(304, NOT_MODIFIED, Not Modified)                                       \
  XX(305, USE_PROXY, Use Proxy)                                             \
  XX(307, TEMPORARY_REDIRECT, Temporary Redirect)                           \
  XX(308, PERMANENT_REDIRECT, Permanent Redirect)                           \
  XX(400, BAD_REQUEST, Bad Request)                                         \
  XX(401, UNAUTHORIZED, Unauthorized)                                       \
  XX(402, PAYMENT_REQUIRED, Payment Required)                               \
  XX(403, FORBIDDEN, Forbidden)                                             \
  XX(404, NOT_FOUND, Not Found)                                             \
  XX(405, METHOD_NOT_ALLOWED, Method Not Allowed)                           \
  XX(406, NOT_ACCEPTABLE, Not Acceptable)                                   \
  XX(407, PROXY_AUTHENTICATION_REQUIRED, Proxy Authentication Required)     \
  XX(408, REQUEST_TIMEOUT, Request Timeout)                                 \
  XX(409, CONFLICT, Conflict)                                               \
  XX(410, GONE, Gone)                                                       \
  XX(411, LENGTH_REQUIRED, Length Required)                                 \
  XX(412, PRECONDITION_FAILED, Precondition Failed)                         \
  XX(413, PAYLOAD_TOO_LARGE, Payload Too Large)                             \
  XX(414, URI_TOO_LONG, URI Too Long)                                       \
  XX(415, UNSUPPORTED_MEDIA_TYPE, Unsupported Media Type)                   \
  XX(416, RANGE_NOT_SATISFIABLE, Range Not Satisfiable)                     \
  XX(417, EXPECTATION_FAILED, Expectation Failed)                           \
  XX(421, MISDIRECTED_REQUEST, Misdirected Request)                         \
  XX(422, UNPROCESSABLE_ENTITY, Unprocessable Entity)                       \
  XX(423, LOCKED, Locked)                                                   \
  XX(424, FAILED_DEPENDENCY, Failed Dependency)                             \
  XX(426, UPGRADE_REQUIRED, Upgrade Required)                               \
  XX(428, PRECONDITION_REQUIRED, Precondition Required)                     \
  XX(429, TOO_MANY_REQUESTS, Too Many Requests)                             \
  XX(431, REQUEST_HEADER_FIELDS_TOO_LARGE, Request Header Fields Too Large) \
  XX(451, UNAVAILABLE_FOR_LEGAL_REASONS, Unavailable For Legal Reasons)     \
  XX(500, INTERNAL_SERVER_ERROR, Internal Server Error)                     \
  XX(501, NOT_IMPLEMENTED, Not Implemented)                                 \
  XX(502, BAD_GATEWAY, Bad Gateway)                                         \
  XX(503, SERVICE_UNAVAILABLE, Service Unavailable)                         \
  XX(504, GATEWAY_TIMEOUT, Gateway Timeout)                                 \
  XX(505, HTTP_VERSION_NOT_SUPPORTED, HTTP Version Not Supported)           \
  XX(506, VARIANT_ALSO_NEGOTIATES, Variant Also Negotiates)                 \
  XX(507, INSUFFICIENT_STORAGE, Insufficient Storage)                       \
  XX(508, LOOP_DETECTED, Loop Detected)                                     \
  XX(510, NOT_EXTENDED, Not Extended)                                       \
  XX(511, NETWORK_AUTHENTICATION_REQUIRED, Network Authentication Required)

/**
 * @brief HTTP方法枚举
 */
enum class HttpMethod {
#define XX(num, name, string) name = (num),
  HTTP_METHOD_MAP(XX)
#undef XX
      INVALID_METHOD
};
enum class HttpStatus {
#define XX(code, name, dessc) name = (code),
  HTTP_STATUS_MAP(XX)
#undef XX
};

auto StringToHttpMethod(const std::string& m) -> HttpMethod;
auto CharsToHttpMethod(const char* m) -> HttpMethod;
auto HttpMethodToString(const HttpMethod& m) -> const char*;
auto HttpStatusToString(const HttpStatus& s) -> const char*;
struct CaseInsensitiveLess {
  /**
   * @brief 忽略大小写比较字符串
   */
  bool operator()(const std::string& lhs, const std::string& rhs) const;
};

template <class MapType, class T>
auto checkGetAs(const MapType& m, const std::string& key, T& val,
                const T& def = T()) -> bool {
  auto it = m.find(key);
  if (it == m.end()) {
    val = def;
    return false;
  }
  try {
    val = boost::lexical_cast<T>(it->second);
    return true;
  } catch (...) {
    val = def;
  }
  return false;
}
template <class MapType, class T>
auto getAs(const MapType& m, const std::string& key, const T& def = T()) -> T {
  auto it = m.find(key);
  if (it == m.end()) {
    return def;
  }
  try {
    return boost::lexical_cast<T>(it->second);
  } catch (...) {
  }
  return def;
}
class HttpResponse;

/**
 * @brief HTTP请求结构
 */
class HttpRequest {
 public:
  /// HTTP请求的智能指针
  using ptr = std::shared_ptr<HttpRequest>;
  /// MAP结构
  using MapType = std::map<std::string, std::string, CaseInsensitiveLess>;

  explicit HttpRequest(uint8_t version = 0x11, bool close = true);

  auto createResponse() -> std::shared_ptr<HttpResponse>;

  auto getMethod() const -> HttpMethod { return m_method; }

  auto getVersion() const -> uint8_t { return m_version; }

  auto getPath() const -> const std::string& { return m_path; }

  auto getQuery() const -> const std::string& { return m_query; }

  auto getBody() const -> const std::string& { return m_body; }

  auto getHeaders() const -> const MapType& { return m_headers; }

  auto getParams() const -> const MapType& { return m_params; }

  auto getCookies() const -> const MapType& { return m_cookies; }

  void setMethod(HttpMethod v) { m_method = v; }

  void setVersion(uint8_t v) { m_version = v; }

  void setPath(const std::string& v) { m_path = v; }

  void setQuery(const std::string& v) { m_query = v; }

  /**
   * @brief 设置HTTP请求的Fragment
   * @param[in] v fragment
   */
  void setFragment(const std::string& v) { m_fragment = v; }

  /**
   * @brief 设置HTTP请求的消息体
   * @param[in] v 消息体
   */
  void setBody(const std::string& v) { m_body = v; }

  /**
   * @brief 是否自动关闭
   */
  auto isClose() const -> bool { return m_close; }

  /**
   * @brief 设置是否自动关闭
   */
  void setClose(bool v) { m_close = v; }

  /**
   * @brief 是否websocket
   */
  bool isWebsocket() const { return m_websocket; }

  void setWebsocket(bool v) { m_websocket = v; }

  void setHeaders(const MapType& v) { m_headers = v; }

  void setParams(const MapType& v) { m_params = v; }

  void setCookies(const MapType& v) { m_cookies = v; }

  auto getHeader(const std::string& key, const std::string& def = "") const
      -> std::string;

  auto getParam(const std::string& key, const std::string& def = "")
      -> std::string;

  auto getCookie(const std::string& key, const std::string& def = "")
      -> std::string;

  void setHeader(const std::string& key, const std::string& val);

  void setParam(const std::string& key, const std::string& val);

  void setCookie(const std::string& key, const std::string& val);

  void delHeader(const std::string& key);

  void delParam(const std::string& key);

  void delCookie(const std::string& key);

  auto hasHeader(const std::string& key, std::string* val = nullptr) -> bool;

  auto hasParam(const std::string& key, std::string* val = nullptr) -> bool;

  auto hasCookie(const std::string& key, std::string* val = nullptr) -> bool;

  template <class T>
  auto checkGetHeaderAs(const std::string& key, T& val, const T& def = T())
      -> bool {
    return checkGetAs(m_headers, key, val, def);
  }

  template <class T>
  auto getHeaderAs(const std::string& key, const T& def = T()) -> T {
    return getAs(m_headers, key, def);
  }

  template <class T>
  auto checkGetParamAs(const std::string& key, T& val, const T& def = T())
      -> bool {
    initQueryParam();
    initBodyParam();
    return checkGetAs(m_params, key, val, def);
  }

  /**
   * @brief 获取HTTP请求的请求参数
   * @tparam T 转换类型
   * @param[in] key 关键字
   * @param[in] def 默认值
   * @return 如果存在且转换成功返回对应的值,否则返回def
   */
  template <class T>
  auto getParamAs(const std::string& key, const T& def = T()) -> T {
    initQueryParam();
    initBodyParam();
    return getAs(m_params, key, def);
  }

  /**
   * @brief 检查并获取HTTP请求的Cookie参数
   * @tparam T 转换类型
   * @param[in] key 关键字
   * @param[out] val 返回值
   * @param[in] def 默认值
   * @return 如果存在且转换成功返回true,否则失败val=def
   */
  template <class T>
  auto checkGetCookieAs(const std::string& key, T& val, const T& def = T())
      -> bool {
    initCookies();
    return checkGetAs(m_cookies, key, val, def);
  }

  /**
   * @brief 获取HTTP请求的Cookie参数
   * @tparam T 转换类型
   * @param[in] key 关键字
   * @param[in] def 默认值
   * @return 如果存在且转换成功返回对应的值,否则返回def
   */
  template <class T>
  auto getCookieAs(const std::string& key, const T& def = T()) -> T {
    initCookies();
    return getAs(m_cookies, key, def);
  }

  /**
   * @brief 序列化输出到流中
   * @param[in, out] os 输出流
   * @return 输出流
   */
  auto dump(std::ostream& os) const -> std::ostream&;

  /**
   * @brief 转成字符串类型
   * @return 字符串
   */
  auto toString() const -> std::string;

  void init();
  void initParam();
  void initQueryParam();
  void initBodyParam();
  void initCookies();

 private:
  /// HTTP方法
  HttpMethod m_method;
  /// HTTP版本
  uint8_t m_version;
  /// 是否自动关闭
  bool m_close;
  /// 是否为websocket
  bool m_websocket;

  uint8_t m_parserParamFlag;
  /// 请求路径
  std::string m_path;
  /// 请求参数
  std::string m_query;
  /// 请求fragment
  std::string m_fragment;
  /// 请求消息体
  std::string m_body;
  /// 请求头部MAP
  MapType m_headers;
  /// 请求参数MAP
  MapType m_params;
  /// 请求Cookie MAP
  MapType m_cookies;
};

class HttpResponse {
 public:
  using ptr = std::shared_ptr<HttpResponse>;
  using MapType = std::map<std::string, std::string, CaseInsensitiveLess>;
  HttpResponse(uint8_t version = 0x11, bool close = true);
  HttpStatus getStatus() const { return m_status; }
  uint8_t getVersion() const { return m_version; }
  auto getBody() const -> const std::string& { return m_body; }
  auto getReason() const -> const std::string& { return m_reason; }
  auto getHeaderAs() const -> const MapType& { return m_headers; }

  void setStatus(HttpStatus v) { m_status = v; }
  void setVersion(uint8_t v) { m_version = v; }
  void setBody(const std::string& v) { m_body = v; }
  void setReason(const std::string& v) { m_reason = v; }

  auto isClose() const -> bool { return m_close; }
  auto isWebsocket() const -> bool { return m_websocket; }
  void setWebsocket(bool v) { m_websocket = v; }

  auto getHeader(const std::string& key, const std::string& def = "") const
      -> std::string;
  void setHeader(const std::string& key, const std::string& val);
  void delHeader(const std::string& key);

  template <class T>
  auto checkGetHeaderAs(const std::string& key, T& val, const T& def = T())
      -> bool {
    return checkGetAs(m_headers, key, val, def);
  }

  template <class T>
  auto getHeaderAs(const std::string& key, const T& def = T()) -> T {
    return getAs(m_headers, key, def);
  }
  void setClose(bool v) { m_close = v; }

  auto dump(std::ostream& os) const -> std::ostream&;

  auto toString() const -> std::string;
  void setRedirect(const std::string& uri);
  void setCookie(const std::string& key, const std::string& val,
                 time_t expired = 0, const std::string& path = "",
                 const std::string& domain = "", bool secure = false);

 private:
  HttpStatus m_status;
  /// 版本
  uint8_t m_version;
  /// 是否自动关闭
  bool m_close;
  /// 是否为websocket
  bool m_websocket;
  /// 响应消息体
  std::string m_body;
  /// 响应原因
  std::string m_reason;
  /// 响应头部MAP
  MapType m_headers;

  std::vector<std::string> m_cookies;
};

auto operator<<(std::ostream& os, const HttpRequest& req) -> std::ostream&;
auto operator<<(std::ostream& os, const HttpResponse& rsp) -> std::ostream&;

}  // namespace http
}  // namespace hx_sylar
#endif