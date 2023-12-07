#ifndef __SYLAR_ENV_H__
#define __SYLAR_ENV_H__

#include <map>
#include <vector>

#include "hx_sylar/singleton.h"
#include "hx_sylar/thread.h"

namespace hx_sylar {

class Env {
 public:
  typedef RWMutex RWMutexType;
  bool init(int argc, char** argv);

  void add(const std::string& key, const std::string& val);
  bool has(const std::string& key);
  void del(const std::string& key);
  std::string get(const std::string& key,
                  const std::string& default_value = "");

  void addHelp(const std::string& key, const std::string& desc);
  void removeHelp(const std::string& key);
  void printHelp();

  const std::string& getExe() const { return m_exe; }
  const std::string& getCwd() const { return m_cwd; }

  bool setEnv(const std::string& key, const std::string& val);
  std::string getEnv(const std::string& key,
                     const std::string& default_value = "");

  std::string getAbsolutePath(const std::string& path) const;
  std::string getAbsoluteWorkPath(const std::string& path) const;
  std::string getConfigPath();

 private:
  RWMutexType m_mutex;
  std::map<std::string, std::string> m_args;
  std::vector<std::pair<std::string, std::string> > m_helps;

  std::string m_program;
  std::string m_exe;
  std::string m_cwd;
};

using EnvMgr = hx_sylar::Singleton<Env>;

}  // namespace hx_sylar

#endif
