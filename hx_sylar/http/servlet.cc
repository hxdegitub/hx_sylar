#include "servlet.h"

namespace hx_sylar::http {
FunctionServlet::FunctionServlet(callback cb)
    : Servlet("FunctionServlet"), m_cb(cb) {}

auto FunctionServlet::handle(hx_sylar::http::HttpRequest::ptr request,
                             hx_sylar::http::HttpResponse::ptr response,
                             hx_sylar::http::HttpSession::ptr session)
    -> int32_t {
  return m_cb(request, response, session);
}

ServletDispatch::ServletDispatch() : Servlet("ServletDispatch") {
  m_default.reset(new NotFoundServlet("sylar/1.0"));
}

auto ServletDispatch::handle(hx_sylar::http::HttpRequest::ptr request,
                             hx_sylar::http::HttpResponse::ptr response,
                             hx_sylar::http::HttpSession::ptr session)
    -> int32_t {
  auto slt = getMatchServlet(request->getPath());
  if (slt) {
    return slt->handle(request, response, session);
  }
  return 0;
}

void ServletDispatch::addServlet(const std::string& uri, Servlet::ptr slt) {
  RWMutexType::WriteLock lock(m_mutex);
  m_datas[uri] = std::move(slt);
}
void ServletDispatch::addServlet(const std::string& uri,
                                 FunctionServlet::callback cb) {
  // RWMutexType::WriteLock lock(m_mutex);
  return addGlobServlet(uri, FunctionServlet::ptr(new FunctionServlet(cb)));
}
void ServletDispatch::addGlobServlet(const std::string& uri, Servlet::ptr slt) {
  RWMutexType::WriteLock lock(m_mutex);
  for (auto it = m_globs.begin(); it != m_globs.end(); ++it) {
    if (it->first == uri) {
      m_globs.erase(it);
      break;
    }
  }
  m_globs.emplace_back(uri, slt);
}
void ServletDispatch::addGlobServlet(const std::string& uri,
                                     FunctionServlet::callback cb) {
  return addGlobServlet(uri, std::make_shared<FunctionServlet>(cb));
}

void ServletDispatch::delServlet(const std::string& uri) {
  RWMutexType::WriteLock lock(m_mutex);
  m_datas.erase(uri);
}

auto ServletDispatch::getServlet(const std::string& uri) -> Servlet::ptr {
  RWMutexType::WriteLock lock(m_mutex);
  auto it = m_datas.find(uri);
  return it == m_datas.end() ? nullptr : it->second;
}

auto ServletDispatch::getGlobServlet(const std::string& uri) -> Servlet::ptr {
  RWMutexType::WriteLock lock(m_mutex);
  for (auto& m_glob : m_globs) {
    if (m_glob.first == uri) {
      return m_glob.second;
    }
  }
  return nullptr;
}
// void ServletDispatch::listAllGlobServletCreator(std::map<std::str)
auto ServletDispatch::getMatchServlet(const std::string& uri) -> Servlet::ptr {
  RWMutexType::ReadLock lock(m_mutex);
  auto mit = m_datas.find(uri);
  if (mit != m_datas.end()) {
    return mit->second;
  }
  for (auto it = m_globs.begin(); it != m_globs.end(); ++it) {
    if (it->first == uri) {
      return it->second;
    }
  }
  return m_default;
}
NotFoundServlet::NotFoundServlet(const std::string& name)
    : Servlet("NotFoundServlet"), m_name(name) {
  m_content =
      "<html><head><title>404 Not Found"
      "</title></head><body><center><h1>404 Not Found</h1></center>"
      "<hr><center>" +
      name + "</center></body></html>";
}

auto NotFoundServlet::handle(hx_sylar::http::HttpRequest::ptr request,
                             hx_sylar::http::HttpResponse::ptr response,
                             hx_sylar::http::HttpSession::ptr session)
    -> int32_t {
  response->setStatus(hx_sylar::http::HttpStatus::NOT_FOUND);
  response->setHeader("Server", "sylar/1.0.0");
  response->setHeader("Content-Type", "text/html");
  response->setBody(m_content);
  return 0;
}

}  // namespace hx_sylar::http
