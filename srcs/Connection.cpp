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
      buffer_(BUFFER_SIZE, 0),
      router_(NULL) {}

Connection::~Connection(void) {
  close(fd_);
  if (router_ != NULL) {
    delete router_;
    router_ = NULL;
  }
}

void Connection::Reset(int does_next_req_exist) {
  if (does_next_req_exist == kNextReq) {
    connection_status_ = NEXT_REQUEST_EXISTS;
    parser_.Clear();
    buffer_.erase();
  } else if (does_next_req_exist == kReset) {
    parser_.Reset();
    buffer_.assign(BUFFER_SIZE, 0);
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
    buffer_.assign(BUFFER_SIZE, 0);
    response_manager_map_.clear();
  }
}

ResponseManager::IoFdPair Connection::HandleRequest(void) {
  if (connection_status_ != NEXT_REQUEST_EXISTS) {
    Receive();
  }
  int req_status = parser_.Parse(buffer_);
  if (req_status < HttpParser::kComplete) {
    connection_status_ = KEEP_READING;
    buffer_.assign(BUFFER_SIZE, 0);
    return ResponseManager::IoFdPair();
  }
  HttpParser::Result req_data = parser_.get_result();
  Request& request = req_data.request;
  {
    std::cerr << ">>> Request <<<\n"
              << "Host: " << request.req.host << "\nPath: " << request.req.path
              << '\n';
  }
  Router::Result location_data = router_->Route(
      req_data.status, request, ConnectionInfo(port_, client_addr_));
  ResponseManager* response_manager = GenerateResponseManager(
      req_data.status, (req_status == HttpParser::kComplete), req_data.request,
      location_data);
  if (response_manager == NULL) {
    connection_status_ = CONNECTION_ERROR;
    std::cerr << "Connection: memory allocation failure\n";
    return ResponseManager::IoFdPair(-1, -1);
  }
  ResponseManager::IoFdPair io_fds = response_manager->Execute();
  bool is_keep_alive = response_manager->get_is_keep_alive();
  if (io_fds.input == -1 && io_fds.output == -1) {
    response_manager->FormatHeader();
    delete response_manager;
  } else {
    if (io_fds.input != -1) {
      response_manager_map_[io_fds.input] = response_manager;
    }
    if (io_fds.output != -1) {
      response_manager_map_[io_fds.output] = response_manager;
    }
  }
  UpdateRequestResult(is_keep_alive);
  return io_fds;
}

ResponseManager::IoFdPair Connection::ExecuteMethod(int event_fd) {
  ResponseManager* manager = response_manager_map_[event_fd];
  ResponseManager::IoFdPair io_fds = manager->Execute();
  if (io_fds.input == -1 && io_fds.output == -1) {
    manager->FormatHeader();
    UpdateRequestResult(manager->get_is_keep_alive());
    delete manager;
    if (response_manager_map_.count(event_fd) == 1) {
      response_manager_map_[event_fd] = NULL;
    }
  } else {
    if (io_fds.input != -1) {
      response_manager_map_[io_fds.input] = manager;
    }
    if (io_fds.output != -1) {
      response_manager_map_[io_fds.output] = manager;
    }
  }
  return io_fds;
}

ResponseManager::IoFdPair Connection::FormatResponse(const int event_fd) {
  ResponseManager* manager = response_manager_map_[event_fd];
  ResponseManager::IoFdPair io_fds = manager->Execute(true);

  ResponseManager::Result& response_result = manager->get_result();
  if (response_result.is_local_redir == true) {
    std::cerr << "Local redirection start\n";
    Request request = manager->get_request();
    bool is_keep_alive = manager->get_is_keep_alive();
    int status =
        ValidateLocalRedirPath(response_result.header["location"], request.req);
    Router::Result location_data =
        router_->Route(status, request, ConnectionInfo(port_, client_addr_));
    ResponseBuffer& current_response_buffer = manager->get_response_buffer();
    delete manager;
    response_manager_map_[event_fd] = NULL;
    // manager for local redirection
    manager = GenerateResponseManager(status, is_keep_alive, request,
                                      location_data, current_response_buffer);
    if (manager == NULL) {
      connection_status_ = CONNECTION_ERROR;
      std::cerr << "Connection: memory allocation failure\n";
      return ResponseManager::IoFdPair(-1, -1);
    }
    io_fds = manager->Execute();
    std::cerr << "local redir io_fds: " << io_fds.input << ", " << io_fds.output
              << '\n';
  }

  if (io_fds.input == -1 && io_fds.output == -1) {
    manager->FormatHeader();
    UpdateRequestResult(manager->get_is_keep_alive());
    delete manager;
    if (response_manager_map_.count(event_fd) == 1) {
      response_manager_map_[event_fd] = NULL;
    }
  } else {
    if (io_fds.input != -1) {
      std::cerr << "Read local redirected resource\n";
      response_manager_map_[io_fds.input] = manager;
    }
    if (io_fds.output != -1) {
      response_manager_map_[io_fds.output] = manager;
    }
  }
  return io_fds;
}

void Connection::Send(void) {
  ResponseBuffer& response = response_queue_.front();
  struct iovec iovec[2];
  size_t iov_cnt;
  SetIov(iovec, iov_cnt, response);
  response.offset += writev(fd_, iovec, iov_cnt);
  if (response.current_buf == ResponseBuffer::kHeader &&
      response.offset >= response.header.size()) {
    response.current_buf = ResponseBuffer::kContent;
  }
  send_status_ = KEEP_SENDING;
  if (response.offset >= response.header.size() + response.content.size()) {
    std::cerr << "response offest : " << response.offset << '\n';
    std::cerr << "sent bytes: "
              << response.header.size() + response.content.size()
              << " header size " << response.header.size() << " content size "
              << response.content.size() << '\n';
    response_queue_.pop();
    if (response_queue_.empty()) {
      send_status_ = SEND_FINISHED;
    }
  }
}

bool Connection::IsResponseBufferReady(void) const {
  if (response_queue_.empty() == true) {
    return false;
  }
  return response_queue_.front().is_complete;
}

const int Connection::get_send_status(void) const { return send_status_; }

const int Connection::get_connection_status(void) const {
  return connection_status_;
}

const int Connection::get_fd(void) const { return fd_; }

void Connection::SetAttributes(const int fd, const std::string& client_addr,
                               const uint16_t port,
                               ServerRouter& server_router) {
  fd_ = fd;
  client_addr_ = client_addr;
  port_ = port;
  router_ = new (std::nothrow) Router(server_router);
  if (router_ == NULL) {
    std::cerr << "Connection : Router memory allocation failure\n";
    connection_status_ = CONNECTION_ERROR;
  }
}

// SECTION : private
void Connection::Receive(void) {
  ssize_t recv_byte = recv(fd_, &buffer_[0], BUFFER_SIZE, 0);
  if (recv_byte <= BUFFER_SIZE) {
    if (recv_byte != -1) {  // FIXME : -1 생각
      buffer_.erase(recv_byte);
    }
  }
}

// For General Response
ResponseManager* Connection::GenerateResponseManager(
    int status, bool is_keep_alive, Request& request,
    Router::Result& router_result) {
  response_queue_.push(ResponseBuffer());
  ResponseManager::Result result(router_result.status);
  if (router_result.status >= 400 || router_result.is_cgi == false) {
    return new (std::nothrow) FileManager(is_keep_alive, response_queue_.back(),
                                          router_result, request);
  } else {  // Cgi found
    return new (std::nothrow) CgiManager(is_keep_alive, response_queue_.back(),
                                         router_result, request);
  }
  return NULL;
}

// For Local Redirection
ResponseManager* Connection::GenerateResponseManager(
    int status, bool is_keep_alive, Request& request,
    Router::Result& router_result, ResponseBuffer& response_buffer) {
  ResponseManager::Result result(router_result.status);
  if (router_result.status >= 400 || router_result.is_cgi == false) {
    return new (std::nothrow)
        FileManager(is_keep_alive, response_buffer, router_result, request);
  } else {  // Cgi found
    return new (std::nothrow)
        CgiManager(is_keep_alive, response_buffer, router_result, request);
  }
  return NULL;
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
    size_t total_len = header.size() + content.size();
    iov[0].iov_len = (total_len < SEND_BUFF_SIZE + res.offset)
                         ? total_len - res.offset
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

int Connection::ValidateLocalRedirPath(std::string& path, RequestLine& req) {
  UriParser::Result uri_result = UriParser().ParseTarget(path);
  if (uri_result.is_valid == false) {
    return 400;  // BAD REQUEST
  }
  PathResolver path_resolver;
  PathResolver::Status path_status =
      path_resolver.Resolve(uri_result.path, PathResolver::kHttpParser);
  if (path_status == PathResolver::kFailure) {
    return 400;  // BAD REQUEST
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
