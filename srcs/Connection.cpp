/**
 * @file Connection.cpp
 * @author ghan, jiskim, yongjule
 * @brief Manage connection between client and server
 * @date 2022-09-27
 *
 * @copyright Copyright (c) 2022
 */

#include "Connection.hpp"

Connection::Connection(void) : fd_(-1), status_(KEEP_ALIVE) {
  memset(buffer_, 0, sizeof(buffer_));
}

Connection::~Connection(void) { close(fd_); }

void Connection::Reset(void) {
  fd_ = -1;
  status_ = KEEP_ALIVE;
  memset(buffer_, 0, sizeof(buffer_));
}

void Connection::HandleRequest(void) {
  Receive();
  Send();
}

void Connection::set_fd(int fd) { fd_ = fd; }

const int Connection::get_status(void) const { return status_; }

// SECTION : private
void Connection::Receive(void) {
  if (recv(fd_, buffer_, BUFFER_SIZE, 0) == -1) {
    std::cerr << "Connection : recv failed for fd " << fd_ << " : "
              << strerror(errno) << '\n';
    status_ = CLOSE_RECV;
  }
}

void Connection::Send(void) {
  if (send(fd_, buffer_, BUFFER_SIZE, 0) == -1) {
    std::cerr << "Connection : send failed for fd " << fd_ << " : "
              << strerror(errno) << '\n';
    status_ = CLOSE_SEND;
  }
}
