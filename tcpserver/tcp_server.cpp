#include <cstring>

#include <sys/socket.h> // for func socket(...)
#include <netinet/in.h> // for struct like sockaddr_in
#include <unistd.h> // for read and write
#include <thread>

#include "logger.hpp"
#include "tcp_server.h"

const size_t BUFF_SIZE = 1024;

TcpServer::TcpServer(uint16_t port) noexcept {
  struct sockaddr_in server_addr;
  memset(&server_addr, 0, sizeof(sockaddr_in));
  server_addr.sin_family = AF_INET;
  // htoxx transfer argument to network byte order
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(port);
  // AF_UNIX for two processes
  // AF_INET for
  sock_fd_ = socket(AF_INET, SOCK_STREAM, 0);
  if (sock_fd_ < 0) {
    common::Logger::t_critical("ERROR opening socket\n");
  }
  if (bind(sock_fd_, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
    common::Logger::t_critical("ERROR binding socket\n");
  }
  common::Logger::t_out("Tcp server created at port %d...\n", port);
}

TcpServer::~TcpServer() noexcept {
  close(sock_fd_);
  common::Logger::t_out("Tcp server destroyed...\n");
}

void TcpServer::serve(std::function<int(char *, int, char *)>& handler) {
  if (listen(sock_fd_, 5) < 0) {
    common::Logger::t_critical("ERROR on listen\n");
  }
  common::Logger::t_out("Tcp server start listen in thread: %08x...\n", std::this_thread::get_id());
  char buf_read[BUFF_SIZE];
  char buf_write[BUFF_SIZE];
  while (true) {
    struct sockaddr_in clientaddr;
    socklen_t clientlen;
    int conn_fd = accept(sock_fd_, (struct sockaddr *) &clientaddr, &clientlen);
    if (conn_fd < 0) {
      common::Logger::t_critical("ERROR on accept\n");
    }
    /*
    * gethostbyaddr: determine who sent the message
    */
    /*
    struct hostent *hostp = gethostbyaddr((const char *)&clientaddr.sin_addr.s_addr, sizeof(clientaddr.sin_addr.s_addr), AF_INET);
    if (hostp == NULL){
      common::Logger::t_critical("ERROR on gethostbyaddr\n");
    }
    char* hostaddrp = inet_ntoa(clientaddr.sin_addr);
    if (hostaddrp == NULL) {
      common::Logger::t_critical("ERROR on inet_ntoa\n");
    }
    common::Logger::t_out("server established connection with %s (%s)\n", hostp->h_name, hostaddrp);
    */
    memset(buf_read, '\0', sizeof(buf_read));
    memset(buf_write, '\0', sizeof(buf_write));

    // read
    int n = read(conn_fd, buf_read, sizeof(buf_read));
    if (n < 0) {
      common::Logger::t_critical("ERROR reading from socket\n");
    }
    common::Logger::t_out("server received %d bytes\n", n);

    // handle
    int send_size = handler(buf_read, n, buf_write);

    // write
    n = write(conn_fd, buf_write, send_size);
    if (n<0) {
      common::Logger::t_critical("ERROR writing to socket\n");
    }
    common::Logger::t_out("server send %d bytes\n", n);

    close(conn_fd);
  }
}
