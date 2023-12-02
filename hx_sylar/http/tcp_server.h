#ifndef __HX_SYLAR_TCP_SERVER_H__
#define __HX_SYLAR_TCP_SERVER_H__
#include <boost/lexical_cast.hpp>
#include <memory>
#include <vector>

#include "../address.h"
#include "../iomanager.h"
#include "../socket.h"
#include "hx_sylar/noncopyable.h"
namespace hx_sylar {
class TcpServerConf {
 public:
  using ptr = std::shared_ptr<TcpServerConf>;

  std::vector<std::string> address;
  int keepalive = 0;
  int timeout = 1000 * 2 * 60;
  int ssl = 0;
  std::string id;

  std::string type = "http";
  std::string name;
  std::string cert_file;
  std::string key_file;
  std::string accept_worker;
  std::string io_worker;
  std::string process_worker;
  std::map<std::string, std::string> args;

  auto isVaild() const -> bool { return !address.empty(); }

  auto operator==(const TcpServerConf& oth) const -> bool {
    return address == oth.address && keepalive == oth.keepalive &&
           timeout == oth.timeout && name == oth.name && ssl == oth.ssl &&
           cert_file == oth.cert_file && key_file == oth.key_file &&
           accept_worker == oth.accept_worker && io_worker == oth.io_worker &&
           process_worker == oth.process_worker && args == oth.args &&
           id == oth.id && type == oth.type;
  }
};
template <>
class LexicalCast<std::string, TcpServerConf> {
 public:
  auto operator()(const std::string& v) -> TcpServerConf {
    YAML::Node node = YAML::Load(v);
    TcpServerConf conf;
    conf.id = node["id"].as<std::string>(conf.id);
    conf.type = node["type"].as<std::string>(conf.type);
    conf.keepalive = node["keepalive"].as<int>(conf.keepalive);
    conf.timeout = node["timeout"].as<int>(conf.timeout);
    conf.name = node["name"].as<std::string>(conf.name);
    conf.ssl = node["ssl"].as<int>(conf.ssl);
    conf.cert_file = node["cert_file"].as<std::string>(conf.cert_file);
    conf.key_file = node["key_file"].as<std::string>(conf.key_file);
    conf.accept_worker = node["accept_worker"].as<std::string>();
    conf.io_worker = node["io_worker"].as<std::string>();
    conf.process_worker = node["process_worker"].as<std::string>();
    conf.args = LexicalCast<std::string, std::map<std::string, std::string> >()(
        node["args"].as<std::string>(""));
    if (node["address"].IsDefined()) {
      for (size_t i = 0; i < node["address"].size(); ++i) {
        conf.address.push_back(node["address"][i].as<std::string>());
      }
    }
    return conf;
  }
};

template <>
class LexicalCast<TcpServerConf, std::string> {
 public:
  auto operator()(const TcpServerConf& conf) -> std::string {
    YAML::Node node;
    node["id"] = conf.id;
    node["type"] = conf.type;
    node["name"] = conf.name;
    node["keepalive"] = conf.keepalive;
    node["timeout"] = conf.timeout;
    node["ssl"] = conf.ssl;
    node["cert_file"] = conf.cert_file;
    node["key_file"] = conf.key_file;
    node["accept_worker"] = conf.accept_worker;
    node["io_worker"] = conf.io_worker;
    node["process_worker"] = conf.process_worker;
    node["args"] = YAML::Load(
        LexicalCast<std::map<std::string, std::string>, std::string>()(
            conf.args));
    for (auto& i : conf.address) {
      node["address"].push_back(i);
    }
    std::stringstream ss;
    ss << node;
    return ss.str();
  }
};

class TcpServer : public std::enable_shared_from_this<TcpServer>, Noncopyable {
 public:
  using ptr = std::shared_ptr<TcpServer>;
  explicit TcpServer(
      hx_sylar::IOManager* worker = hx_sylar::IOManager::GetThis(),

      hx_sylar::IOManager* io_woker = hx_sylar::IOManager::GetThis(),
      hx_sylar::IOManager* accept_worker = hx_sylar::IOManager::GetThis());
  virtual ~TcpServer();

  virtual auto bind(hx_sylar::Address::ptr addr, bool ssl = false) -> bool;
  virtual auto bind(const std::vector<Address::ptr>& addrs,
                    std::vector<Address::ptr>& fails, bool is_ssl = false)
      -> bool;

  auto loadCertificates(const std::string& cert_file,
                        const std::string& key_file) -> bool;

  auto start() -> bool;
  void stop();

  auto getReadTimeout() const -> uint64_t { return m_recvTimeout; }
  auto getName() const -> std::string { return m_name; }

  void setName(const std::string& v) { m_name = v; }
  auto isStop() -> bool const { return m_isStop; }

  //   auto getConf()const -> TcpServerConfg::ptr;
  void setConf(const TcpServerConf& v);
  auto getSocks() const -> std::vector<Socket::ptr> { return m_socks; }
  auto toString(const std::string& prefix) -> std::string;

 protected:
  virtual void handleClient(Socket::ptr& client);
  virtual void startAccept(Socket::ptr sock);

 private:
  std::vector<Socket::ptr> m_socks;
  IOManager* m_worker;
  IOManager* m_ioWorker;
  IOManager* m_acceptWorker;
  uint64_t m_recvTimeout;
  std::string m_name;
  std::string m_type = "tcp";
  bool m_isStop;
  bool m_ssl = false;
};
}  // namespace hx_sylar
#endif