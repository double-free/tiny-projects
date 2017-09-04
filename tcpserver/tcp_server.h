#pragma once
#include <functional>
class TcpServer {
public:
  explicit TcpServer(uint16_t port) noexcept;
  TcpServer &operator=(const TcpServer& s) = delete;
  TcpServer(const TcpServer& s) = delete;
  ~TcpServer() noexcept;
  // handler 返回要发送的有效数据长度
  void serve(std::function<int(char *, int, char *)>& handler);
protected:
private:
  int sock_fd_;
};
