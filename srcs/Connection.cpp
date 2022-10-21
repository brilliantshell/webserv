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

Connection::~Connection(void) {
  shutdown(fd_, SHUT_RD);
  if (router_ != NULL) {
    delete router_;
  }
}

void Connection::Reset(void) {
  fd_ = -1;
  status_ = KEEP_ALIVE;
  buffer_.erase();
}

// 여기서 serverRouter를 아는 채로 옴
void Connection::HandleRequest(void) {
  Receive();
  int req_status = parser_.Parse(buffer_);
  HttpParser::Result req_data = parser_.get_result();
  Request& request = req_data.request;
  Router::Result location_data = router_->Route(
      req_data.status, request, ConnectionInfo(port_, client_addr_));
  ResourceManager::Result exec_result =
      resource_manager_.ExecuteMethod(location_data, request);
  std::string response = response_formatter_.Format(
      exec_result, request.req.version, location_data.methods, req_status);
  status_ = (exec_result.status < 500 && req_status == HttpParser::kComplete)
                ? KEEP_ALIVE
                : CLOSE;
  Send(response);
  shutdown(fd_, SHUT_WR);  // TODO : 테스트에 dependent
}

const int Connection::get_status(void) const { return status_; }

void Connection::SetAttributes(const int fd, const std::string& client_addr,
                               const uint16_t port,
                               ServerRouter& server_router) {
  fd_ = fd;
  client_addr_ = client_addr;
  port_ = port;
  try {
    router_ = new Router(server_router);
  } catch (std::exception& e) {
    std::cerr << "Connection : Router memory allocation failure\n";
    status_ = CONNECTION_ERROR;
  }
}

// SECTION : private
void Connection::Receive(void) {
  if (recv(fd_, &buffer_[0], BUFFER_SIZE - 1, 0) == -1) {
    std::cerr << "Connection : recv failed for fd " << fd_ << " : "
              << strerror(errno) << '\n';
    status_ = CLOSE;
  }
}

void Connection::Send(const std::string& response) {
  if (send(fd_, response.c_str(), BUFFER_SIZE - 1, 0) == -1) {
    std::cerr << "Connection : send failed for fd " << fd_ << " : "
              << strerror(errno) << '\n';
    if (status_ == KEEP_ALIVE) {
      status_ = CLOSE;
    }
  }
}
