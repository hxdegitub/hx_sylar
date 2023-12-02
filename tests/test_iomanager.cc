#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <sys/socket.h>

#include <unistd.h>

#include <iostream>

#include "../hx_sylar/hx_sylar.h"
#include "../hx_sylar/iomanager.h"

hx_sylar::Logger::ptr g_logger = HX_LOG_ROOT();

int sock = 0;

void TestFiber() {
  HX_LOG_INFO(g_logger) << "test_fiber sock=" << sock;


  sock = socket(AF_INET, SOCK_STREAM, 0);
  fcntl(sock, F_SETFL, O_NONBLOCK);

  sockaddr_in addr{};
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(80);
  inet_pton(AF_INET, "127.0.0,1", &addr.sin_addr.s_addr);

  if (connect(sock, reinterpret_cast<const sockaddr*>(&addr), sizeof(addr)) == 0) {
  } else if (errno == EINPROGRESS) {
    HX_LOG_INFO(g_logger) << "add event errno=" << errno << " EINPROGRESS "
                          << strerror(errno);
    hx_sylar::IOManager::GetThis()->addEvent(
        sock, hx_sylar::IOManager::READ,
        []() { HX_LOG_INFO(g_logger) << "read callback"; });
    hx_sylar::IOManager::GetThis()->addEvent(
        sock, hx_sylar::IOManager::WRITE, []() {
          HX_LOG_INFO(g_logger) << "write callback";
          // close(sock);
          hx_sylar::IOManager::GetThis()->cancelEvent(
              sock, hx_sylar::IOManager::READ);
          close(sock);
        });
  } else {
    HX_LOG_INFO(g_logger) << "else " << errno << " " << strerror(errno);
  }
}

void Test1() {
  std::cout << "EPOLL=" << EPOLLIN << " EPOLLOUT=" << EPOLLOUT << std::endl;
  hx_sylar::IOManager iom(2, false);
  iom.schedule(&TestFiber);
}

 hx_sylar::Timer::ptr s_timer;
 void TestTimer() {
   hx_sylar::IOManager iom(2);
   s_timer = iom.addTimer(
       1000,
       []() {
         static int i = 0;
         HX_LOG_INFO(g_logger) << "hello timer i=" << i;
         if (++i == 3) {
           s_timer->reset(2000, true);
//            s_timer->cancel();
         }
         if(i%5 == 0){
            s_timer->reset(5000, true);
         }
       },
       true);
 }

auto main(int argc, char** argv) -> int {
//  Test1();
   TestTimer();
  return 0;
}
