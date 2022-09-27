/**
 * @file Connection.cpp
 * @author ghan, jiskim, yongjule
 * @brief Manage connection between client and server
 * @date 2022-09-27
 *
 * @copyright Copyright (c) 2022
 */

#include "Connection.hpp"

Connection::Connection(void) : fd_(-1) {}

Connection::~Connection(void) { close(fd_); }

void Connection::Close(void) {
  close(fd_);
  fd_ = -1;
}

void Connection::Receive(void) {
  if (recv(fd_, buffer_, BUFFER_SIZE, 0) == -1) {
    std::cerr << "Connection : recv failed for fd " << fd_ << " : "
              << strerror(errno) << '\n';
  }
}

void Connection::Send(void) {
  if (send(fd_, buffer_, BUFFER_SIZE, 0) == -1) {
    std::cerr << "Connection : send failed for fd " << fd_ << " : "
              << strerror(errno) << '\n';
  }
}

void Connection::set_fd(int fd) { fd_ = fd; }
