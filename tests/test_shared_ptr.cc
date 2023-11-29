#include <memory.h>

#include <memory>
using namespace std;
#include <iostream>
class A {
 public:
  A() : x(3), y(3) {}
  void print() { cout << " x : " << x << "y : " << y << endl; }

 private:
  int x, y;
};

int main() {
  auto ptr = std::make_shared<A>();
  ptr->print();
  return 0;
}