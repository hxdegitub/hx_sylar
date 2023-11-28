#include "hx_sylar/hook.h"
#include "hx_sylar/iomanager.h"
#include "hx_sylar/log.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/unistd.h>

hx_sylar::Logger ::ptr g_logger = HX_LOG_ROOT();
void test_sleep() {
  hx_sylar::IOManager iom(1);
  iom.schedule([]() {
    sleep(2);
    HX_LOG_INFO(g_logger) << "sleep 2";
  });
  iom.schedule([]() {
    sleep(3);
    HX_LOG_INFO(g_logger) << "sleep 3";
  });
  HX_LOG_INFO(g_logger) << "test_sleep end";
}


void test_sock() {
  int sock = socket(AF_INET, SOCK_STREAM, 0);

  sockaddr_in addr{};
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(8080);
  inet_pton(AF_INET, "192.168.83.5", &addr.sin_addr.s_addr);

  HX_LOG_INFO(g_logger) << "begin connect";
  int rt = connect(sock, reinterpret_cast<const sockaddr*>(&addr), sizeof(addr));
  HX_LOG_INFO(g_logger) << "connect rt=" << rt << " errno=" << errno;

  if(rt != 0) {
    return;
  }

  const char data[] = "GET / HTTP/1.0\r\n\r\n";
  rt = send(sock, data, sizeof(data), 0);
  HX_LOG_INFO(g_logger) << "send rt=" << rt << " errno=" << errno;

  if(rt <= 0) {
    return;
  }

  std::string buff;
  buff.resize(4096);

  rt = recv(sock, &buff[0], buff.size(), 0);
  HX_LOG_INFO(g_logger) << "rev rt=" << rt << " errno=" << errno;

  if(rt <= 0) {
    return;
  }

  buff.resize(rt);
  HX_LOG_INFO(g_logger) << buff;
}

int main() {
//  test_sock();
  hx_sylar::IOManager iom;
  iom.schedule(test_sock);
  return 0;
}