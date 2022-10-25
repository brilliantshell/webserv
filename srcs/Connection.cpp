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
    : fd_(-1),
      connection_status_(KEEP_ALIVE),
      send_status_(KEEP_SENDING_HEADER),
      sent_bytes_(0),
      buffer_(BUFFER_SIZE + 1, 0),
      router_(NULL) {}

Connection::~Connection(void) {
  // FIXME : 필요하냐...?
  shutdown(fd_, SHUT_RD);
  if (router_ != NULL) {
    delete router_;
    router_ = NULL;
  }
}

void Connection::Reset(bool does_next_req_exist) {
  if (does_next_req_exist == kNextReq) {
    connection_status_ = NEXT_REQUEST_EXISTS;
    parser_.Clear();
  } else {
    close(fd_);
    fd_ = -1;
    connection_status_ = KEEP_ALIVE;
    port_ = 0;
    parser_.Reset();
    client_addr_.clear();
    if (router_ != NULL) {
      delete router_;
      router_ = NULL;
    }
    // NOTE : Queue clear or not thinking hagi
    while (!response_queue_.empty()) {
      response_queue_.pop();
    }
  }
  send_status_ = KEEP_SENDING_HEADER;
  sent_bytes_ = 0;
  buffer_.assign(BUFFER_SIZE + 1, 0);
}

void Connection::HandleRequest() {
  if (connection_status_ != NEXT_REQUEST_EXISTS) {
    Receive();
  }
  buffer_.erase(buffer_.find('\0'));
  int req_status = parser_.Parse(buffer_);
  if (req_status < HttpParser::kComplete) {
    connection_status_ = KEEP_READING;
    buffer_.assign(BUFFER_SIZE + 1, 0);
    return;
  }
  HttpParser::Result req_data = parser_.get_result();
  Request& request = req_data.request;
  {
    std::cerr << ">>> Request <<< \nHost : " << request.req.host
              << "\nPath : " << request.req.path << request.req.query << '\n';
  }
  Router::Result location_data = router_->Route(
      req_data.status, request, ConnectionInfo(port_, client_addr_));
  response_queue_.push(ResponseBuffer());
  ResponseBuffer& response = response_queue_.front();
  ResourceManager::Result exec_result =
      resource_manager_.ExecuteMethod(response.content, location_data, request);
  response.header = response_formatter_.Format(
      response.content.size(), exec_result, request.req.version,
      location_data.methods, req_status);
  connection_status_ =
      (exec_result.status < 500 && req_status == HttpParser::kComplete)
          ? KEEP_ALIVE
          : CLOSE;
  Send();
  if (connection_status_ == KEEP_ALIVE && parser_.DoesNextReqExist() == true) {
    Reset(kNextReq);
  } else {
    parser_.Reset();
    buffer_.assign(BUFFER_SIZE + 1, 0);
  }
}

void Connection::Send(void) {
  ssize_t sent;
  ResponseBuffer& response = response_queue_.front();
  if (send_status_ == KEEP_SENDING_HEADER) {
    std::cerr << "header sending... size(" << sent_bytes_ << ")\n";
    sent = send(fd_, response.header.c_str() + sent_bytes_,
                response.header.size() < SND_BUFF_SIZE + sent_bytes_
                    ? response.header.size() - sent_bytes_
                    : SND_BUFF_SIZE,
                0);
  } else {
    std::cerr << "content sending... size(" << sent_bytes_ << ")\n";
    sent = send(
        fd_, response.content.c_str() + sent_bytes_ - response.header.size(),
        response.content.size() + response.header.size() <
                SND_BUFF_SIZE + sent_bytes_
            ? response.content.size() + response.header.size() - sent_bytes_
            : SND_BUFF_SIZE,
        0);
  }
  sent_bytes_ += sent;
  if (sent_bytes_ < response.header.size()) {
    send_status_ = KEEP_SENDING_HEADER;
  } else if (sent_bytes_ < response.header.size() + response.content.size()) {
    send_status_ = KEEP_SENDING_CONTENT;
  } else {
    // end sending
    std::cerr << "sent bytes: " << sent_bytes_ << " header size "
              << response.header.size() << " content size "
              << response.content.size() << '\n';
    response_queue_.pop();
    send_status_ = SEND_FINISHED;
    sent_bytes_ = 0;
  }
}

const int Connection::get_send_status(void) const { return send_status_; }
const int Connection::get_connection_status(void) const {
  return connection_status_;
}

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
    connection_status_ = CONNECTION_ERROR;
  }
}

// SECTION : private
void Connection::Receive(void) { recv(fd_, &buffer_[0], BUFFER_SIZE, 0); }
