#include "http_connection.h"

#include <utility>

#include "../uri.h"
#include "http_parser.h"
#include "http_session.h"
#include "hx_sylar/hook.h"
namespace hx_sylar::http {
static hx_sylar::Logger::ptr g_logger = HX_LOG_ROOT();
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

HttpConnectionPool::ptr HttpConnectionPool::Create(const std::string& uri,
                                                   const std::string& vhost,
                                                   uint32_t max_size,
                                                   uint32_t max_alive_time,
                                                   uint32_t max_request) {
  Uri::ptr turi = Uri::Create(uri);
  if (!turi) {
    HX_LOG_ERROR(g_logger) << "invalid uri=" << uri;
  }
  return std::make_shared<HttpConnectionPool>(
      turi->getHost(), vhost, turi->getPort(), turi->getScheme() == "https",
      max_size, max_alive_time, max_request);
}

HttpConnectionPool::HttpConnectionPool(const std::string& host,
                                       const std::string& vhost, uint32_t port,
                                       bool is_https, uint32_t max_size,
                                       uint32_t max_alive_time,
                                       uint32_t max_request)
    : m_host(host),
      m_vhost(vhost),
      m_port(port ? port : (is_https ? 443 : 80)),
      m_maxSize(max_size),
      m_maxAliveTime(max_alive_time),
      m_maxRequest(max_request),
      m_isHttps(is_https) {}

HttpConnection::ptr HttpConnectionPool::getConnection() {
  uint64_t now_ms = hx_sylar::GetCurrentMS();
  std::vector<HttpConnection*> invalid_conns;
  HttpConnection* ptr = nullptr;
  MutexType::Lock lock(m_mutex);
  while (!m_conns.empty()) {
    auto conn = *m_conns.begin();
    m_conns.pop_front();
    if (!conn->isConnected()) {
      invalid_conns.push_back(conn);
      continue;
    }
    if ((conn->m_createTime + m_maxAliveTime) > now_ms) {
      invalid_conns.push_back(conn);
      continue;
    }
    ptr = conn;
    break;
  }
  lock.unlock();
  for (auto i : invalid_conns) {
    delete i;
  }
  m_total -= invalid_conns.size();

  if (!ptr) {
    IPAddress::ptr addr = Address::LookupAnyIPAddress(m_host);
    if (!addr) {
      HX_LOG_ERROR(g_logger) << "get addr fail: " << m_host;
      return nullptr;
    }
    addr->setPort(m_port);
    Socket::ptr sock =
        m_isHttps ? SSLSocket::CreateTCP(addr) : Socket::CreateTCP(addr);
    if (!sock) {
      HX_LOG_ERROR(g_logger) << "create sock fail: " << *addr;
      return nullptr;
    }
    if (!sock->connect(addr)) {
      HX_LOG_ERROR(g_logger) << "sock connect fail: " << *addr;
      return nullptr;
    }

    ptr = new HttpConnection(sock);
    ++m_total;
  }
  return {ptr, [this](auto&& PH1) {
            return HttpConnectionPool::ReleasePtr(
                std::forward<decltype(PH1)>(PH1), this);
          }};
}

void HttpConnectionPool::ReleasePtr(HttpConnection* ptr,
                                    HttpConnectionPool* pool) {
  ++ptr->m_request;
  if (!ptr->isConnected() ||
      ((ptr->m_createTime + pool->m_maxAliveTime) >=
       hx_sylar::GetCurrentMS()) ||
      (ptr->m_request >= pool->m_maxRequest)) {
    delete ptr;
    --pool->m_total;
    return;
  }
  MutexType::Lock lock(pool->m_mutex);
  pool->m_conns.push_back(ptr);
}

auto HttpConnectionPool::doGet(
    const std::string& url, uint64_t timeout_ms,
    const std::map<std::string, std::string>& headers, const std::string& body)
    -> HttpResult::ptr {
  return doRequest(HttpMethod::GET, url, timeout_ms, headers, body);
}

auto HttpConnectionPool::doGet(
    Uri::ptr uri, uint64_t timeout_ms,
    const std::map<std::string, std::string>& headers, const std::string& body)
    -> HttpResult::ptr {
  std::stringstream ss;
  ss << uri->getPath() << (uri->getQuery().empty() ? "" : "?")
     << uri->getQuery() << (uri->getFragment().empty() ? "" : "#")
     << uri->getFragment();
  return doGet(ss.str(), timeout_ms, headers, body);
}

auto HttpConnectionPool::doPost(
    const std::string& url, uint64_t timeout_ms,
    const std::map<std::string, std::string>& headers, const std::string& body)
    -> HttpResult::ptr {
  return doRequest(HttpMethod::POST, url, timeout_ms, headers, body);
}

auto HttpConnectionPool::doPost(
    Uri::ptr uri, uint64_t timeout_ms,
    const std::map<std::string, std::string>& headers, const std::string& body)
    -> HttpResult::ptr {
  std::stringstream ss;
  ss << uri->getPath() << (uri->getQuery().empty() ? "" : "?")
     << uri->getQuery() << (uri->getFragment().empty() ? "" : "#")
     << uri->getFragment();
  return doPost(ss.str(), timeout_ms, headers, body);
}

HttpResult::ptr HttpConnectionPool::doRequest(
    HttpMethod method, const std::string& url, uint64_t timeout_ms,
    const std::map<std::string, std::string>& headers,
    const std::string& body) {
  HttpRequest::ptr req = std::make_shared<HttpRequest>();
  req->setPath(url);
  req->setMethod(method);
  req->setClose(false);
  bool has_host = false;
  for (auto& i : headers) {
    if (strcasecmp(i.first.c_str(), "connection") == 0) {
      if (strcasecmp(i.second.c_str(), "keep-alive") == 0) {
        req->setClose(false);
      }
      continue;
    }

    if (!has_host && strcasecmp(i.first.c_str(), "host") == 0) {
      has_host = !i.second.empty();
    }

    req->setHeader(i.first, i.second);
  }
  if (!has_host) {
    if (m_vhost.empty()) {
      req->setHeader("Host", m_host);
    } else {
      req->setHeader("Host", m_vhost);
    }
  }
  req->setBody(body);
  return doRequest(req, timeout_ms);
}

HttpResult::ptr HttpConnectionPool::doRequest(
    HttpMethod method, Uri::ptr uri, uint64_t timeout_ms,
    const std::map<std::string, std::string>& headers,
    const std::string& body) {
  std::stringstream ss;
  ss << uri->getPath() << (uri->getQuery().empty() ? "" : "?")
     << uri->getQuery() << (uri->getFragment().empty() ? "" : "#")
     << uri->getFragment();
  return doRequest(method, ss.str(), timeout_ms, headers, body);
}

HttpResult::ptr HttpConnectionPool::doRequest(HttpRequest::ptr req,
                                              uint64_t timeout_ms) {
  auto conn = getConnection();
  if (!conn) {
    return std::make_shared<HttpResult>(
        (int)HttpResult::Error::POOL_GET_CONNECTION, nullptr,
        "pool host:" + m_host + " port:" + std::to_string(m_port));
  }
  auto sock = conn->getSocket();
  if (!sock) {
    return std::make_shared<HttpResult>(
        (int)HttpResult::Error::POOL_INVALID_CONNECTION, nullptr,
        "pool host:" + m_host + " port:" + std::to_string(m_port));
  }
  sock->setRecvTimeout(timeout_ms);
  int rt = conn->sendRequest(req);
  if (rt == 0) {
    return std::make_shared<HttpResult>(
        static_cast<int>(HttpResult::Error::SEND_CLOSE_BY_PEER), nullptr,
        "send request closed by peer: " + sock->getRemoteAddress()->toString());
  }
  if (rt < 0) {
    return std::make_shared<HttpResult>(
        (int)HttpResult::Error::SEND_SOCKET_ERROR, nullptr,
        "send request socket error errno=" + std::to_string(errno) +
            " errstr=" + std::string(strerror(errno)));
  }
  auto rsp = conn->recvResponse();
  if (!rsp) {
    return std::make_shared<HttpResult>(
        (int)HttpResult::Error::TIMEOUT, nullptr,
        "recv response timeout: " + sock->getRemoteAddress()->toString() +
            " timeout_ms:" + std::to_string(timeout_ms));
  }
  return std::make_shared<HttpResult>((int)HttpResult::Error::OK, rsp, "ok");
}

}  // namespace hx_sylar::http
