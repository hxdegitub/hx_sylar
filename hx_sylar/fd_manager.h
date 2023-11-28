//
// Created by hx on 23-11-28.
//

#ifndef hx_sylar_FD_MANAGER_H
#define hx_sylar_FD_MANAGER_H
#include "thread.h"
#include <memory>
#include <vector>
#include "singleton.h"
namespace hx_sylar{
class FdCtx:public std::enable_shared_from_this<FdCtx>{
public :
  using ptr = std::shared_ptr<FdCtx>;
  FdCtx(int fd);
  ~FdCtx();

  auto init()->bool;
  auto isInit() const ->bool {return m_isInit;}
  auto isSocket()const ->bool {return m_isSocket;}
  auto isClose()->bool{return  m_isClosed;}

  void setUserNonblock(bool v) {m_userNonblock = v;}
  auto getUserNonblock()const ->bool{return m_userNonblock;}
  void setSysNonblock(bool v) {m_sysNonblock = v;}
  auto getSysNonblock()const ->bool {return m_sysNonblock;}

  void setTimeout(int type ,  uint64_t v);
  auto  getTimeout(int type)->uint64_t;
private:
  bool m_isInit:1;
  bool m_isSocket:1;
  bool m_sysNonblock:1;
  bool m_userNonblock:1;
  bool m_isClosed:1;
  int m_fd;
  uint64_t m_recvTimeout;
  uint64_t m_sendTimeout;
};
class FdManager{
public:
using RwMutexType=RWMutex;
FdManager();
auto get(int fd,bool auto_create = false)->FdCtx::ptr;
void del(int fd);
private:
  RwMutexType  m_mutex;
  std::vector<FdCtx::ptr> m_datas;
};

using FdMgr = Singleton<FdManager>;
}

#endif // hx_sylar_FD_MANAGER_H
