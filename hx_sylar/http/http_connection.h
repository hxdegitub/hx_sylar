#ifndef __HX_SYLAR_HTTP_CONNECTION__
#define __HX_SYLAR_HTTP_CONNECTION__

#include "../uri.h"
#include "http.h"
#include "hx_sylar/http/http_session.h"
#include "hx_sylar/socket.h"
#include "hx_sylar/stream/socket_stream.h"
namespace hx_sylar::http {

struct HttpResult {
  /// 智能指针类型定义
  typedef std::shared_ptr<HttpResult> ptr;
  /**
   * @brief 错误码定义
   */
  enum class Error {
    /// 正常
    OK = 0,
    /// 非法URL
    INVALID_URL = 1,
    /// 无法解析HOST
    INVALID_HOST = 2,
    /// 连接失败
    CONNECT_FAIL = 3,
    /// 连接被对端关闭
    SEND_CLOSE_BY_PEER = 4,
    /// 发送请求产生Socket错误
    SEND_SOCKET_ERROR = 5,
    /// 超时
    TIMEOUT = 6,
    /// 创建Socket失败
    CREATE_SOCKET_ERROR = 7,
    /// 从连接池中取连接失败
    POOL_GET_CONNECTION = 8,
    /// 无效的连接
    POOL_INVALID_CONNECTION = 9,
  };

  /**
   * @brief 构造函数
   * @param[in] _result 错误码
   * @param[in] _response HTTP响应结构体
   * @param[in] _error 错误描述
   */
  HttpResult(int _result, HttpResponse::ptr _response,
             const std::string& _error)
      : result(_result), response(_response), error(_error) {}

  /// 错误码
  int result;
  /// HTTP响应结构体
  HttpResponse::ptr response;
  /// 错误描述
  std::string error;

  std::string toString() const;
};
class HttpConnection : public SocketStream {
  friend class HttpConnectionPool;

 public:
  using ptr = std::shared_ptr<HttpConnection>;
  explicit HttpConnection(Socket::ptr sock, bool owner = true);
  auto recvResponse() -> HttpResponse::ptr;
  auto sendRequest(http::HttpRequest::ptr rsp) -> int;
  static HttpResult::ptr DoGet(
      const std::string& url, uint64_t timeout_ms,
      const std::map<std::string, std::string>& headers = {},
      const std::string& body = "");

  /**
   * @brief 发送HTTP的GET请求
   * @param[in] uri URI结构体
   * @param[in] timeout_ms 超时时间(毫秒)
   * @param[in] headers HTTP请求头部参数
   * @param[in] body 请求消息体
   * @return 返回HTTP结果结构体
   */
  static HttpResult::ptr DoGet(
      Uri::ptr uri, uint64_t timeout_ms,
      const std::map<std::string, std::string>& headers = {},
      const std::string& body = "");
  static auto DoPost(const std::string& url, uint64_t timeout_ms,
                     const std::map<std::string, std::string>& headers = {},
                     const std::string& body = "") -> HttpResult::ptr;

  /**
   * @brief 发送HTTP的POST请求
   * @param[in] uri URI结构体
   * @param[in] timeout_ms 超时时间(毫秒)
   * @param[in] headers HTTP请求头部参数
   * @param[in] body 请求消息体
   * @return 返回HTTP结果结构体
   */
  static auto DoPost(Uri::ptr uri, uint64_t timeout_ms,
                     const std::map<std::string, std::string>& headers = {},
                     const std::string& body = "") -> HttpResult::ptr;

  /**
   * @brief 发送HTTP请求
   * @param[in] method 请求类型
   * @param[in] uri 请求的url
   * @param[in] timeout_ms 超时时间(毫秒)
   * @param[in] headers HTTP请求头部参数
   * @param[in] body 请求消息体
   * @return 返回HTTP结果结构体
   */
  static auto DoRequest(HttpMethod method, const std::string& url,
                        uint64_t timeout_ms,
                        const std::map<std::string, std::string>& headers = {},
                        const std::string& body = "") -> HttpResult::ptr;

  /**
   * @brief 发送HTTP请求
   * @param[in] method 请求类型
   * @param[in] uri URI结构体
   * @param[in] timeout_ms 超时时间(毫秒)
   * @param[in] headers HTTP请求头部参数
   * @param[in] body 请求消息体
   * @return 返回HTTP结果结构体
   */
  static auto DoRequest(HttpMethod method, Uri::ptr uri, uint64_t timeout_ms,
                        const std::map<std::string, std::string>& headers = {},
                        const std::string& body = "") -> HttpResult::ptr;

 private:
  uint64_t m_createTime = 0;
  uint64_t m_request = 0;
};

class HttpConnectionPool {
 public:
  typedef std::shared_ptr<HttpConnectionPool> ptr;
  typedef Mutex MutexType;

  static HttpConnectionPool::ptr Create(const std::string& uri,
                                        const std::string& vhost,
                                        uint32_t max_size,
                                        uint32_t max_alive_time,
                                        uint32_t max_request);

  HttpConnectionPool(const std::string& host, const std::string& vhost,
                     uint32_t port, bool is_https, uint32_t max_size,
                     uint32_t max_alive_time, uint32_t max_request);

  HttpConnection::ptr getConnection();

  HttpResult::ptr doGet(const std::string& url, uint64_t timeout_ms,
                        const std::map<std::string, std::string>& headers = {},
                        const std::string& body = "");

  HttpResult::ptr doGet(Uri::ptr uri, uint64_t timeout_ms,
                        const std::map<std::string, std::string>& headers = {},
                        const std::string& body = "");

  HttpResult::ptr doPost(const std::string& url, uint64_t timeout_ms,
                         const std::map<std::string, std::string>& headers = {},
                         const std::string& body = "");

  HttpResult::ptr doPost(Uri::ptr uri, uint64_t timeout_ms,
                         const std::map<std::string, std::string>& headers = {},
                         const std::string& body = "");

  HttpResult::ptr doRequest(
      HttpMethod method, const std::string& url, uint64_t timeout_ms,
      const std::map<std::string, std::string>& headers = {},
      const std::string& body = "");

  /**
   * @brief 发送HTTP请求
   * @param[in] method 请求类型
   * @param[in] uri URI结构体
   * @param[in] timeout_ms 超时时间(毫秒)
   * @param[in] headers HTTP请求头部参数
   * @param[in] body 请求消息体
   * @return 返回HTTP结果结构体
   */
  auto doRequest(HttpMethod method, Uri::ptr uri, uint64_t timeout_ms,
                 const std::map<std::string, std::string>& headers = {},
                 const std::string& body = "") -> HttpResult::ptr;

  /**
   * @brief 发送HTTP请求
   * @param[in] req 请求结构体
   * @param[in] timeout_ms 超时时间(毫秒)
   * @return 返回HTTP结果结构体
   */
  auto doRequest(HttpRequest::ptr req, uint64_t timeout_ms) -> HttpResult::ptr;
  auto getMaxSize() -> uint32_t { return m_maxSize; }

 private:
  static void ReleasePtr(HttpConnection* ptr, HttpConnectionPool* pool);

 private:
  std::string m_host;
  std::string m_vhost;
  uint32_t m_port;
  uint32_t m_maxSize;
  uint32_t m_maxAliveTime;
  uint32_t m_maxRequest;
  bool m_isHttps;

  MutexType m_mutex;
  std::list<HttpConnection*> m_conns;
  std::atomic<int32_t> m_total = {0};
};
}  // namespace hx_sylar::http
#endif
