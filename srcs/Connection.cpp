/**
 * @file Connection.cpp
 * @author ghan, jiskim, yongjule
 * @brief Manage connection between client and server
 * @date 2022-09-27
 *
 * @copyright Copyright (c) 2022
 */

#include "Connection.hpp"

Connection::Connection(void)
    : fd_(-1), status_(KEEP_ALIVE), buffer_(BUFFER_SIZE, 0) {}

Connection::~Connection(void) { close(fd_); }

void Connection::Reset(void) {
  fd_ = -1;
  status_ = KEEP_ALIVE;
  buffer_.erase();
}

// 여기서 serverRouter를 아는 채로 옴
void Connection::HandleRequest(void) {
  Receive();
  // parse
  // *.route에서 진행
  // server_router[server_name] -> 찾으면 locationRouter, 못찾으면 default 리턴
  // locationRouter[path] -> 찾으면 location, 못찾으면 error_page 리턴
  //  => 최종적으로 location class 리턴
  /*
  result{
      status_code
      method
      success_path root/index.html
      error_path
  }
  */
  Send();
}

void Connection::set_fd(int fd) { fd_ = fd; }

const int Connection::get_status(void) const { return status_; }

void Connection::set_client_addr(std::string client_addr) {
  client_addr_ = client_addr;
}

// SECTION : private
void Connection::Receive(void) {
  if (recv(fd_, &buffer_[0], BUFFER_SIZE - 1, 0) == -1) {
    std::cerr << "Connection : recv failed for fd " << fd_ << " : "
              << strerror(errno) << '\n';
    status_ = CLOSE;
  }
}

void Connection::Send(void) {
  if (send(fd_, &buffer_[0], BUFFER_SIZE - 1, 0) == -1) {
    std::cerr << "Connection : send failed for fd " << fd_ << " : "
              << strerror(errno) << '\n';
    if (status_ == KEEP_ALIVE) {
      status_ = CLOSE;
    }
  }
}
