参考
---
[英文资料](http://www.linuxhowtos.org/C_C++/socket.htm)

介绍
---
封装了一个典型的 tcpserver，特点有以下两个：

1. 实现了与 tcpserver 与业务的解耦<br/>
2. 异步收发

使用方法
---
在需要使用 tcpserver 的业务中包含头文件并创建实例。<br/>
在业务中实现回调函数
```cpp
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
```
再使用 <functional> 中的方法处理后传给 server<br/>
非成员函数：
```cpp
// 在子线程监听
std::thread t([&](){
  std::function<int(char*, int, char*)> handler = tcp_handler;
  tcpSever.serve(handler);
});
t.detach();
```
对于类成员函数，需要使用 `std::bind` 把 this 参数绑定。
```cpp
std::thread t([=]{
  // 这里不能使用 auto f = ...
  // 否则报错
  typedef std::function<int(char*, int, char*)> HANDLER;
  HANDLER f = std::bind(&QdpTraderSpi::tcp_handler, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
  server_.serve(f);
});
t.detach();
```
