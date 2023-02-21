#ifndef __SINGLETON_H__
#define __SINGLETON_H__

#include <memory>
namespace hx_sylar {
template <class T, class X = void, int N = 0>
class Singleton {
 public:
  static std::shared_ptr<T> GetInstance() {
    static std::shared_ptr<T> v(new T);
    return v;
  }

 private:
  Singleton();
};
}  // namespace hx_sylar
#endif