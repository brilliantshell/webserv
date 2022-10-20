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
  close(fd_);
  delete router_;
}

void Connection::Reset(void) {
  fd_ = -1;
  status_ = KEEP_ALIVE;
  buffer_.erase();
}

// 여기서 serverRouter를 아는 채로 옴
void Connection::HandleRequest(void) {
  Receive();
  int parser_status = parser_.Parse(buffer_);
  HttpParser::Result parser_result = parser_.get_result();

  Router::Result router_result =
      router_->Route(parser_result.status, parser_result.request,
                     ConnectionInfo(port_, client_addr_));
  ResourceManager::Result resource_manager_result =
      ResourceManager().ExecuteMethod(router_result, parser_result.request);

  std::string response = ResponseFormatter().Format(
      resource_manager_result, parser_result.request.req.version,
      router_result.methods, parser_status);
  status_ = (resource_manager_result.status < 500 &&
             parser_status == HttpParser::kComplete)
                ? KEEP_ALIVE
                : CLOSE;
  Send(response);
}

const int Connection::get_status(void) const { return status_; }

void Connection::set_fd(int fd) { fd_ = fd; }

void Connection::set_client_addr(std::string client_addr) {
  client_addr_ = client_addr;
}

void set_port(uint16_t port);

void Connection::set_router(ServerRouter& server_router) {
  try {
    router_ = new Router(server_router);
  } catch (std::exception& e) {
    std::cerr << e.what() << '\n';
    std::cerr << "Memory allocation error";
    // TODO : error handling
  }
}

void Connection::set_port(uint16_t port) { port_ = port; }

// SECTION : private
void Connection::Receive(void) {
  if (recv(fd_, &buffer_[0], BUFFER_SIZE - 1, 0) == -1) {
    std::cerr << "Connection : recv failed for fd " << fd_ << " : "
              << strerror(errno) << '\n';
    status_ = CLOSE;
  }
}

void Connection::Send(const std::string& response) {
  if (send(fd_, &response[0], BUFFER_SIZE - 1, 0) == -1) {
    std::cerr << "Connection : send failed for fd " << fd_ << " : "
              << strerror(errno) << '\n';
    if (status_ == KEEP_ALIVE) {
      status_ = CLOSE;
    }
  }
}
