#include <thread>
#include <functional>
#include <mutex>
#include <condition_variable>

#include <cstring>
#include <cstdio>

#include <csignal>

#include "tcp_server.h"
// kill -l: signal 1~64
int signaled = 0;
std::mutex signal_mu;
std::condition_variable signal_cond;

void signal_handler(int sig) {
  // use signal handler to avoid unsafe quit
  std::lock_guard<std::mutex> lg(signal_mu);
  signaled = sig;
  signal_cond.notify_one();
}

// 回调
// @para1: 服务器接收的数据
// @para2: 服务器接收数据的长度
// @para3: 待写入的服务器需要发送的数据
// @return: 需要发送数据的长度
int tcp_handler(char* recv_buf, int recv_len, char* send_buf) {
  printf("Received: %s\n", recv_buf);
  const char* reply = "HAHAHHA";
  sprintf(send_buf, "%s", reply);
  return strlen(reply);
}

int main(int argc, const char *argv[]) {
  // set handler for signal
  signal(SIGINT, signal_handler);
  signal(SIGTERM, signal_handler);

  TcpServer tcpSever(10086);
  // 在子线程监听
  std::thread t([&](){
    std::function<int(char*, int, char*)> handler = tcp_handler;
    tcpSever.serve(handler);
  });
  t.detach();

  {
    std::unique_lock<std::mutex> ul(signal_mu);
    signal_cond.wait(ul, []{return signaled != 0;});
  }

  fprintf(stderr, "Terminated by signal: %d\n", signaled);
  return 0;
}
