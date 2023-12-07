#include <netinet/in.h>

#include <cstring>

#include "hx_sylar/address.h"
#include "hx_sylar/bytearray.h"
#include "hx_sylar/http/tcp_server.h"
#include "hx_sylar/iomanager.h"
#include "hx_sylar/log.h"

static hx_sylar::Logger::ptr g_logger = HX_LOG_ROOT();

class EchoServer : public hx_sylar::TcpServer {
 public:
  EchoServer(int type);
  void handleClient(hx_sylar::Socket::ptr client) override;

 private:
  int m_type = 0;
};

EchoServer::EchoServer(int type) : m_type(type) {}

void EchoServer::handleClient(hx_sylar::Socket::ptr client) {
  HX_LOG_INFO(g_logger) << "handleClient " << *client;
  hx_sylar::ByteArray::ptr ba(new hx_sylar::ByteArray);
  while (true) {
    ba->clear();
    std::vector<iovec> iovs;
    ba->getWriteBuffers(iovs, 1024);

    int rt = client->recv(&iovs[0], iovs.size());
    if (rt == 0) {
      HX_LOG_INFO(g_logger) << "client close: " << *client;
      break;
    } else if (rt < 0) {
      HX_LOG_INFO(g_logger) << "client error rt=" << rt << " errno=" << errno
                            << " errstr=" << strerror(errno);
      break;
    }
    ba->setPosition(ba->getPosition() + rt);
    ba->setPosition(0);
    HX_LOG_INFO(g_logger) << "recv rt=" << rt << " data="
                          << std::string((char*)iovs[0].iov_base, rt);
    if (m_type == 1) {              // text
      std::cout << ba->toString();  // << std::endl;
    } else {
      std::cout << ba->toHexString();  // << std::endl;
    }
    std::cout.flush();
  }
}

int type = 1;

void run() {
  HX_LOG_INFO(g_logger) << "server type=" << type;
  EchoServer::ptr es(new EchoServer(type));

  sockaddr_in server_addr;
  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(8020);  // 将端口修改为 8020
  server_addr.sin_addr.s_addr =
      inet_addr("192.168.83.123");  // 将 IP 地址修改为 192.168.83.123

  auto addr = hx_sylar::IPv4Address::Create("0.0.0.0", 8020);

  if (addr == nullptr) {
    exit(1);
  }
  while (!es->bind(addr)) {
    sleep(2);
  }
  es->start();
}

auto main(int argc, char** argv) -> int {
  if (argc < 2) {
    HX_LOG_INFO(g_logger) << "used as[" << argv[0] << " -t] or [" << argv[0]
                          << " -b]";
    return 0;
  }

  if (strcmp(argv[1], "-t")) {
    type = 2;
  }

  static hx_sylar::IOManager iom(3, false, "test_echo");
  iom.start();
  HX_LOG_INFO(g_logger) << " start ";
  iom.schedule(run);
  HX_LOG_INFO(g_logger) << " schedule run ";
  // iom.stop();
  HX_LOG_INFO(g_logger) << " stop  ";
  return 0;
}
