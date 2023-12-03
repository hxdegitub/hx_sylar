#ifndef __HX_SYLAR_URI_H__
#define __HX_SYLAR_URI_H__
#include <memory>
#include <ostream>
#include <string>

#include "hx_sylar/address.h"

namespace hx_sylar {
class Uri {
 public:
  using ptr = std::shared_ptr<Uri>;
  Uri();
  static Uri::ptr Create(const std::string& uri);

  auto getScheme() const -> const std::string& { return m_scheme; }
  auto getUserinfo() const -> const std::string& { return m_userinfo; }
  auto getHost() const -> const std::string& { return m_host; }
  auto getPath() const -> const std::string&;
  auto getQuery() const -> const std::string& { return m_query; }
  auto getFragment() const -> const std::string& { return m_fragment; }
  auto getPort() const -> int32_t;

  void setScheme(const std::string& v) { m_scheme = v; }
  void setUserinfo(const std::string& v) { m_userinfo = v; }
  void setHost(const std::string& v) { m_host = v; }
  void setPath(const std::string& v) { m_path = v; }
  void setQuery(const std::string& v) { m_query = v; }
  void setFragment(const std::string& v) { m_fragment = v; }
  void setPort(int32_t v) { m_port = v; }
  auto dump(std::ostream& os) const -> std::ostream&;
  auto toString() const -> std::string;

  auto createAddress() const -> Address::ptr;

 private:
  bool isDefaultPort() const;

 private:
  std::string m_scheme;
  std::string m_userinfo;
  std::string m_host;
  int32_t m_port;
  std::string m_path;
  std::string m_query;
  std::string m_fragment;
};
}  // namespace hx_sylar
#endif
