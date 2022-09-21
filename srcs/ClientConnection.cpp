/**
 * @file Client.cpp
 * @author ghan, jiskim, yongjule
 * @brief
 * @date 2022-09-21
 *
 * @copyright Copyright (c) 2022
 */

#include "ClientConnection.hpp"

ClientConnection::ClientConnection(void) : socket_fd_(-1) {}

ClientConnection::~ClientConnection(void) { close(socket_fd_); }

void ClientConnection::Connect(uint16_t port) {
  sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = inet_addr("127.0.0.1");
  addr.sin_port = htons(port);

  socket_fd_ = ::socket(AF_INET, SOCK_STREAM, 0);
  if (socket_fd_ == -1) {
    std::cerr << "socket() error " << strerror(errno) << std::endl;
    exit(2);
  }
  if (::connect(socket_fd_, (sockaddr*)&addr, sizeof(addr)) == -1) {
    std::cerr << "connect() error on port " << port << " : " << strerror(errno)
              << std::endl;
    exit(2);
  }
}

void ClientConnection::SendMessage(void) {
  if (::send(socket_fd_, RANDOM_STR, sizeof(RANDOM_STR), 0) == -1) {
    std::cerr << "send() error" << strerror(errno) << std::endl;
    exit(2);
  }
}

void ClientConnection::ReceiveMessage(void) {
  char buf[4097];

  memset(buf, 0, sizeof(buf));
  if (::recv(socket_fd_, buf, sizeof(buf), 0) == -1) {
    std::cerr << "recv() error" << strerror(errno) << std::endl;
    exit(2);
  }
  exit(strcmp(buf, RANDOM_STR) ? 1 : 0);
}
