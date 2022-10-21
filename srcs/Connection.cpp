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
    : fd_(-1), status_(KEEP_ALIVE), buffer_(BUFFER_SIZE, 0), router_(NULL) {}

Connection::~Connection(void) {
  // FIXME : 필요하냐...?
  shutdown(fd_, SHUT_RD);
  if (router_ != NULL) {
    delete router_;
    router_ = NULL;
  }
}

void Connection::Reset(void) {
  close(fd_);
  fd_ = -1;
  status_ = KEEP_ALIVE;
  uint16_t port_ = 0;
  client_addr_.clear();
  buffer_.erase();
  parser_.Reset();
  if (router_ != NULL) {
    delete router_;
    router_ = NULL;
  }
}

// 여기서 serverRouter를 아는 채로 옴
void Connection::HandleRequest() {
  if (status_ != NEXT_REQUEST_EXISTS) {
    Receive();
  }
  std::cerr << ">>> Received Buffer_ Before Parse <<<\n " << buffer_ << '\n';
  int req_status = parser_.Parse(buffer_);
  std::cerr << ">>>  parse status <<<" << (int)req_status << '\n';
  if (req_status < HttpParser::kComplete) {
    status_ = KEEP_READING;
    return;
  }
  HttpParser::Result req_data = parser_.get_result();
  Request& request = req_data.request;
  {
    std::cerr
        << ">>> Request <<< \nMethods : 1 : (GET) 2 : (POST) 4 : (DELETE)\n"
        << (int)request.req.method << "\nHost : " << request.req.host
        << "\nPath : " << request.req.path << request.req.query << '\n';
  }
  Router::Result location_data = router_->Route(
      req_data.status, request, ConnectionInfo(port_, client_addr_));
  {
    std::cerr << ">>> Internal Server <<< \nsuccess_path: "
              << location_data.success_path
              << "\nstatus: " << location_data.status << '\n';

    std::cerr << "Received Buffer_ : " << buffer_ << '\n';
  }
  ResourceManager::Result exec_result =
      resource_manager_.ExecuteMethod(location_data, request);
  std::string response = response_formatter_.Format(
      exec_result, request.req.version, location_data.methods, req_status);
  status_ = (exec_result.status < 500 && req_status == HttpParser::kComplete)
                ? KEEP_ALIVE
                : CLOSE;
  { std::cerr << ">>> Response <<< \n" << response << '\n'; }
  Send(response);
  if (status_ == KEEP_ALIVE && parser_.DoesNextReqExist() == true) {
    parser_.Clear();
    buffer_.erase();
    status_ = NEXT_REQUEST_EXISTS;
    return;
  }
  parser_.Reset();
  // shutdown(fd_, SHUT_WR);  // TODO : 테스트에 dependent
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
