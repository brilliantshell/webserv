/**
 * @file Connection.cpp
 * @author ghan, jiskim, yongjule
 * @brief
 * @date 2022-10-27
 *
 * @copyright Copyright (c) 2022
 */
#include "Connection.hpp"

Connection::Connection(void)
    : fd_(-1),
      connection_status_(KEEP_ALIVE),
      send_status_(KEEP_SENDING),
      buffer_(BUFFER_SIZE + 1, 0),
      router_(NULL) {}

Connection::~Connection(void) {
  close(fd_);
  if (router_ != NULL) {
    delete router_;
    router_ = NULL;
  }
}

void Connection::Reset(bool does_next_req_exist) {
  if (does_next_req_exist == kNextReq) {
    connection_status_ = NEXT_REQUEST_EXISTS;
    parser_.Clear();
  } else if (does_next_req_exist == kReset) {
    parser_.Reset();
  } else {
    close(fd_);
    fd_ = -1;
    port_ = 0;
    parser_.Reset();
    connection_status_ = KEEP_ALIVE;
    client_addr_.clear();
    if (router_ != NULL) {
      delete router_;
      router_ = NULL;
    }
    while (!response_queue_.empty()) {
      response_queue_.pop();
    }
  }
  buffer_.assign(BUFFER_SIZE + 1, 0);
}

void Connection::HandleRequest(void) {
  if (connection_status_ != NEXT_REQUEST_EXISTS) {
    Receive();
  }

  std::cerr << "요청 : " << buffer_ << "]\n\n";

  buffer_.erase(buffer_.find('\0'));
  int req_status = parser_.Parse(buffer_);
  if (req_status < HttpParser::kComplete) {
    connection_status_ = KEEP_READING;
    buffer_.assign(BUFFER_SIZE + 1, 0);
  } else {
    HttpParser::Result req_data = parser_.get_result();
    Request& request = req_data.request;
    {
      std::cerr << "\n\n======================= 요청 시작 "
                   "===========================\n\n";
      std::cerr << ">>> Request <<< \nHost : " << request.req.host
                << "\nPath : " << request.req.path << request.req.query << '\n';
    }
    GenerateResponse(req_data.status, req_status, request);
  }
}

void Connection::Send(void) {
  ResponseBuffer& response = response_queue_.front();

  struct iovec iovec[2];
  size_t iov_cnt;
  SetIov(iovec, iov_cnt, response);

  std::cerr << "\n\n";
  writev(STDERR_FILENO, iovec, iov_cnt);
  std::cerr << "\n\n======================= 요청 끝 "
               "=============================\n\n";

  response.offset += writev(fd_, iovec, iov_cnt);
  if (response.current_buf == ResponseBuffer::kHeader &&
      response.offset >= response.header.size()) {
    response.current_buf = ResponseBuffer::kContent;
  }
  send_status_ = KEEP_SENDING;
  if (response.offset >= response.total_len) {
    std::cerr << "sent bytes: " << response.total_len << " header size "
              << response.header.size() << " content size "
              << response.content.size() << '\n';
    response_queue_.pop();
    if (response_queue_.empty()) {
      send_status_ = SEND_FINISHED;
    }
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

void Connection::GenerateResponse(int status, int req_status,
                                  Request& request) {
  Router::Result location_data =
      router_->Route(status, request, ConnectionInfo(port_, client_addr_));

  response_queue_.push(ResponseBuffer());
  ResponseBuffer& res = response_queue_.back();
  ResourceManager::Result exec_result =
      resource_manager_.ExecuteMethod(res.content, location_data, request);
  if (exec_result.is_local_redir == true) {
    std::string redir_path = exec_result.header["location"];
    status = ValidateLocalRedirPath(redir_path, request.req, req_status);
    location_data =
        router_->Route(status, request, ConnectionInfo(port_, client_addr_));
    exec_result =
        resource_manager_.ExecuteMethod(res.content, location_data, request);
  }
  bool is_keep_alive =
      (exec_result.status < 500 && req_status == HttpParser::kComplete);
  res.header = response_formatter_.Format(res.content.size(), exec_result,
                                          request.req.version,
                                          location_data.methods, is_keep_alive);
  res.total_len = res.header.size() + res.content.size();
  UpdateRequestResult(is_keep_alive);
}

void Connection::SetIov(struct iovec* iov, size_t& cnt, ResponseBuffer& res) {
  std::string& header = res.header;
  std::string& content = res.content;
  cnt = (res.current_buf == ResponseBuffer::kHeader &&
         header.size() < SEND_BUFF_SIZE + res.offset)
            ? 2
            : 1;
  if (res.current_buf == ResponseBuffer::kHeader) {
    iov[0].iov_base = &header[0] + res.offset;
    iov[0].iov_len = (header.size() < SEND_BUFF_SIZE + res.offset)
                         ? header.size() - res.offset
                         : SEND_BUFF_SIZE;
    if (cnt == 2) {
      iov[1].iov_base = &content[0];
      iov[1].iov_len = (content.size() + iov[0].iov_len < SEND_BUFF_SIZE)
                           ? content.size()
                           : SEND_BUFF_SIZE - iov[0].iov_len;
    }
  } else {
    iov[0].iov_base = &content[0] + (res.offset - header.size());
    iov[0].iov_len = (res.total_len < SEND_BUFF_SIZE + res.offset)
                         ? res.total_len - res.offset
                         : SEND_BUFF_SIZE;
  }
}

void Connection::UpdateRequestResult(bool is_keep_alive) {
  connection_status_ = (is_keep_alive) ? KEEP_ALIVE : CLOSE;
  send_status_ = KEEP_SENDING;
  (connection_status_ == KEEP_ALIVE && parser_.DoesNextReqExist() == true)
      ? Reset(kNextReq)
      : Reset(kReset);
}

int Connection::ValidateLocalRedirPath(std::string& path, RequestLine& req,
                                       int& req_status) {
  UriParser::Result uri_result = UriParser().ParseTarget(path);
  if (uri_result.is_valid == false) {
    req_status = HttpParser::kClose;
    return 400;  // BAD REQUEST
  }
  PathResolver path_resolver;
  PathResolver::Status path_status =
      path_resolver.Resolve(uri_result.path, PathResolver::kHttpParser);
  if (path_status == PathResolver::kFailure) {
    req_status = HttpParser::kClose;
    return 400;
  }
  std::transform(uri_result.host.begin(), uri_result.host.end(),
                 uri_result.host.begin(), ::tolower);
  req.host = uri_result.host;
  req.path = uri_result.path + ((path_status == PathResolver::kFile)
                                    ? path_resolver.get_file_name()
                                    : "");
  req.query = uri_result.query;
  return 200;
}
