#ifndef __HX_CONFIG_H__
#define __HX_CONFIG_H__

#include <yaml-cpp/yaml.h>

#include <boost/lexical_cast.hpp>
#include <functional>
#include <list>
#include <map>
#include <memory>
#include <set>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "log.h"
// #include "mutex.h"
#include "thread.h"
#include "util.h"

namespace hx_sylar {

/**
 * @brief 配置变量的基类
 */
class ConfigVarBase {
 public:
  typedef std::shared_ptr<ConfigVarBase> ptr;
  /**
   * @brief 构造函数
   * @param[in] name 配置参数名称[0-9a-z_.]
   * @param[in] description 配置参数描述
   */
  ConfigVarBase(const std::string& name, const std::string& description = "")
      : m_name(name), m_description(description) {
    std::transform(m_name.begin(), m_name.end(), m_name.begin(), ::tolower);
  }

  virtual ~ConfigVarBase() {}

  const std::string& getName() const { return m_name; }

  const std::string& getDescription() const { return m_description; }

  virtual std::string toString() = 0;

  virtual bool fromString(const std::string& val) = 0;

  virtual std::string getTypeName() const = 0;

 protected:
  /// 配置参数的名称
  std::string m_name;
  /// 配置参数的描述
  std::string m_description;
};

/**
 * @brief 类型转换模板类(F 源类型, T 目标类型)
 */
template <class F, class T>
class LexicalCast {
 public:
  T operator()(const F& v) { return boost::lexical_cast<T>(v); }
};

/**
 * @brief 类型转换模板类片特化(YAML String 转换成 std::vector<T>)
 */
template <class T>
class LexicalCast<std::string, std::vector<T> > {
 public:
  std::vector<T> operator()(const std::string& v) {
    YAML::Node node = YAML::Load(v);
    typename std::vector<T> vec;
    std::stringstream ss;
    for (auto&& i : node) {
      ss.str("");
      ss << i;
      vec.push_back(LexicalCast<std::string, T>()(ss.str()));
    }
    return vec;
  }
};

template <class T>
class LexicalCast<std::vector<T>, std::string> {
 public:
  auto operator()(const std::vector<T>& v) -> std::string {
    YAML::Node node(YAML::NodeType::Sequence);
    for (auto& i : v) {
      node.push_back(YAML::Load(LexicalCast<T, std::string>()(i)));
    }
    std::stringstream ss;
    ss << node;
    return ss.str();
  }
};

/**
 * @brief 类型转换模板类片特化(YAML String 转换成 std::list<T>)
 */
template <class T>
class LexicalCast<std::string, std::list<T> > {
 public:
  auto operator()(const std::string& v) -> std::list<T> {
    YAML::Node node = YAML::Load(v);
    typename std::list<T> vec;
    std::stringstream ss;
    for (auto&& i : node) {
      ss.str("");
      ss << i;
      vec.push_back(LexicalCast<std::string, T>()(ss.str()));
    }
    return vec;
  }
};

template <class T>
class LexicalCast<std::list<T>, std::string> {
 public:
  auto operator()(const std::list<T>& v) -> std::string {
    YAML::Node node(YAML::NodeType::Sequence);
    for (auto& i : v) {
      node.push_back(YAML::Load(LexicalCast<T, std::string>()(i)));
    }
    std::stringstream ss;
    ss << node;
    return ss.str();
  }
};

/**
 * @brief 类型转换模板类片特化(YAML String 转换成 std::set<T>)
 */
template <class T>
class LexicalCast<std::string, std::set<T> > {
 public:
  auto operator()(const std::string& v) -> std::set<T> {
    YAML::Node node = YAML::Load(v);
    typename std::set<T> vec;
    std::stringstream ss;
    for (auto&& i : node) {
      ss.str("");
      ss << i;
      vec.insert(LexicalCast<std::string, T>()(ss.str()));
    }
    return vec;
  }
};

/**
 * @brief 类型转换模板类片特化(std::set<T> 转换成 YAML String)
 */
template <class T>
class LexicalCast<std::set<T>, std::string> {
 public:
  auto operator()(const std::set<T>& v) -> std::string {
    YAML::Node node(YAML::NodeType::Sequence);
    for (auto& i : v) {
      node.push_back(YAML::Load(LexicalCast<T, std::string>()(i)));
    }
    std::stringstream ss;
    ss << node;
    return ss.str();
  }
};

/**
 * @brief 类型转换模板类片特化(YAML String 转换成 std::unordered_set<T>)
 */
template <class T>
class LexicalCast<std::string, std::unordered_set<T> > {
 public:
  auto operator()(const std::string& v) -> std::unordered_set<T> {
    YAML::Node node = YAML::Load(v);
    typename std::unordered_set<T> vec;
    std::stringstream ss;
    for (auto&& i : node) {
      ss.str("");
      ss << i;
      vec.insert(LexicalCast<std::string, T>()(ss.str()));
    }
    return vec;
  }
};

/**
 * @brief 类型转换模板类片特化(std::unordered_set<T> 转换成 YAML String)
 */
template <class T>
class LexicalCast<std::unordered_set<T>, std::string> {
 public:
  auto operator()(const std::unordered_set<T>& v) -> std::string {
    YAML::Node node(YAML::NodeType::Sequence);
    for (auto& i : v) {
      node.push_back(YAML::Load(LexicalCast<T, std::string>()(i)));
    }
    std::stringstream ss;
    ss << node;
    return ss.str();
  }
};

/**
 * @brief 类型转换模板类片特化(YAML String 转换成 std::map<std::string, T>)
 */
template <class T>
class LexicalCast<std::string, std::map<std::string, T> > {
 public:
  auto operator()(const std::string& v) -> std::map<std::string, T> {
    YAML::Node node = YAML::Load(v);
    typename std::map<std::string, T> vec;
    std::stringstream ss;
    for (auto it = node.begin(); it != node.end(); ++it) {
      ss.str("");
      ss << it->second;
      vec.insert(std::make_pair(it->first.Scalar(),
                                LexicalCast<std::string, T>()(ss.str())));
    }
    return vec;
  }
};

/**
 * @brief 类型转换模板类片特化(std::map<std::string, T> 转换成 YAML String)
 */
template <class T>
class LexicalCast<std::map<std::string, T>, std::string> {
 public:
  auto operator()(const std::map<std::string, T>& v) -> std::string {
    YAML::Node node(YAML::NodeType::Map);
    for (auto& i : v) {
      node[i.first] = YAML::Load(LexicalCast<T, std::string>()(i.second));
    }
    std::stringstream ss;
    ss << node;
    return ss.str();
  }
};

/**
 * @brief 类型转换模板类片特化(YAML String 转换成
 * std::unordered_map<std::string, T>)
 */
template <class T>
class LexicalCast<std::string, std::unordered_map<std::string, T> > {
 public:
  std::unordered_map<std::string, T> operator()(const std::string& v) {
    YAML::Node node = YAML::Load(v);
    typename std::unordered_map<std::string, T> vec;
    std::stringstream ss;
    for (auto it = node.begin(); it != node.end(); ++it) {
      ss.str("");
      ss << it->second;
      vec.insert(std::make_pair(it->first.Scalar(),
                                LexicalCast<std::string, T>()(ss.str())));
    }
    return vec;
  }
};

/**
 * @brief 类型转换模板类片特化(std::unordered_map<std::string, T> 转换成 YAML
 * String)
 */
template <class T>
class LexicalCast<std::unordered_map<std::string, T>, std::string> {
 public:
  auto operator()(const std::unordered_map<std::string, T>& v) -> std::string {
    YAML::Node node(YAML::NodeType::Map);
    for (auto& i : v) {
      node[i.first] = YAML::Load(LexicalCast<T, std::string>()(i.second));
    }
    std::stringstream ss;
    ss << node;
    return ss.str();
  }
};

/**
 * @brief 配置参数模板子类,保存对应类型的参数值
 * @details T 参数的具体类型
 *          FromStr 从std::string转换成T类型的仿函数
 *          ToStr 从T转换成std::string的仿函数
 *          std::string 为YAML格式的字符串
 */
template <class T, class FromStr = LexicalCast<std::string, T>,
          class ToStr = LexicalCast<T, std::string> >
class ConfigVar : public ConfigVarBase {
 public:
  using RWMutexType = RWMutex;
  using ptr = std::shared_ptr<ConfigVar>;
  using on_change_cb = std::function<void(const T&, const T&)>;

  ConfigVar(const std::string& name, const T& default_value,
            const std::string& description = "")
      : ConfigVarBase(name, description), m_val(default_value) {}

  /**
   * @brief 将参数值转换成YAML String
   * @exception 当转换失败抛出异常
   */
  auto toString() -> std::string override {
    try {
      RWMutexType ::ReadLock lock(m_mutex);
      return ToStr()(m_val);
    } catch (std::exception& e) {
      HX_LOG_ERROR(HX_LOG_ROOT())
          << "ConfigVar::toString exception " << e.what()
          << " convert: " << typeid(m_val).name() << " to string"
          << " name=" << m_name;
    }
    return "";
  }

  auto fromString(const std::string& val) -> bool override {
    try {
      setValue(FromStr()(val));
    } catch (std::exception& e) {
      HX_LOG_ERROR(HX_LOG_ROOT())
          << "ConfigVar::fromString exception " << e.what()
          << " convert: string to " << typeid(m_val).name()
          << " name=" << m_name << " - " << val;
    }
    return false;
  }
  auto getValue() -> const T {
    RWMutexType::ReadLock lock(m_mutex);
    return m_val;
  }

  void setValue(const T& v) {
    {
      RWMutexType::ReadLock lock(m_mutex);
      if (v == m_val) {
        return;
      }
      for (auto& i : m_cbs) {
        i.second(m_val, v);
      }
    }
    RWMutexType::WriteLock lock(m_mutex);
    m_val = v;
  }

  auto getTypeName() const -> std::string override {
    return typeid(m_val).name();
  }

  auto addListener(on_change_cb cb) -> uint64_t {
    static uint64_t s_fun_id = 0;
    RWMutexType::WriteLock lock(m_mutex);
    ++s_fun_id;
    m_cbs[s_fun_id] = cb;
    return s_fun_id;
  }

  /**
   * @brief 删除回调函数
   * @param[in] key 回调函数的唯一id
   */
  void delListener(uint64_t key) {
    RWMutexType::WriteLock lock(m_mutex);
    m_cbs.erase(key);
  }

  auto getListener(uint64_t key) -> on_change_cb {
    RWMutexType::ReadLock lock(m_mutex);
    auto it = m_cbs.find(key);
    return it == m_cbs.end() ? nullptr : it->second;
  }

  void clearListener() {
    RWMutexType::WriteLock lock(m_mutex);
    m_cbs.clear();
  }

 private:
  RWMutexType m_mutex;
  T m_val;
  //变更回调函数组, uint64_t key,要求唯一，一般可以用hash
  std::map<uint64_t, on_change_cb> m_cbs;
};

class Config {
 public:
  using ConfigVarMap = std::unordered_map<std::string, ConfigVarBase::ptr>;
  using RWMutexType = RWMutex;
  template <class T>
  static auto Lookup(const std::string& name, const T& default_value,
                     const std::string& description = "") ->
      typename ConfigVar<T>::ptr {
    RWMutexType::WriteLock lock(GetMutex());
    auto it = GetDatas().find(name);
    if (it != GetDatas().end()) {
      auto tmp = std::dynamic_pointer_cast<ConfigVar<T> >(it->second);
      if (tmp) {
        HX_LOG_INFO(HX_LOG_ROOT()) << "Lookup name= " << name << " exists";
        return tmp;
      }
      HX_LOG_ERROR(HX_LOG_ROOT())
          << "Lookup name=" << name << " exists but type not "
          << " real_type=" << it->second->getTypeName() << " "
          << it->second->toString();
      return nullptr;
    }

    if (name.find_first_not_of("abcdefghikjlmnopqrstuvwxyz._012345678") !=
        std::string::npos) {
      HX_LOG_ERROR(HX_LOG_ROOT()) << "Lookup name invalid " << name;
      throw std::invalid_argument(name);
    }

    typename ConfigVar<T>::ptr v(
        new ConfigVar<T>(name, default_value, description));
    GetDatas()[name] = v;
    return v;
  }

  /**
   * @brief 查找配置参数
   * @param[in] name 配置参数名称
   * @return 返回配置参数名为name的配置参数
   */
  template <class T>
  static auto Lookup(const std::string& name) -> typename ConfigVar<T>::ptr {
    RWMutexType::ReadLock lock(GetMutex());
    auto it = GetDatas().find(name);
    if (it == GetDatas().end()) {
      return nullptr;
    }
    return std::dynamic_pointer_cast<ConfigVar<T> >(it->second);
  }

  static void LoadFromYaml(const YAML::Node& root);

  static void LoadFromConfDir(const std::string& path, bool force = false);

  static auto LookupBase(const std::string& name) -> ConfigVarBase::ptr;

  static void Visit(std::function<void(ConfigVarBase::ptr)> cb);

 private:
  static auto GetDatas() -> ConfigVarMap& {
    static ConfigVarMap s_datas;
    return s_datas;
  }
  static auto GetMutex() -> RWMutexType& {
    static RWMutexType s_mutex;
    return s_mutex;
  }
};

}  // namespace hx_sylar

#endif
