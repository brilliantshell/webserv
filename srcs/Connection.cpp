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

void Connection::Reset(int does_next_req_exist) {
  if (does_next_req_exist == kNextReq) {
    connection_status_ = NEXT_REQUEST_EXISTS;
    parser_.Clear();
    buffer_.erase();
  } else if (does_next_req_exist == kReset) {
    parser_.Reset();
    buffer_.assign(BUFFER_SIZE + 1, 0);
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
    buffer_.assign(BUFFER_SIZE + 1, 0);
  }
}

ResponseManager::IoFdPair Connection::HandleRequest(void) {
  if (connection_status_ != NEXT_REQUEST_EXISTS) {
    Receive();
  }
  int req_status = parser_.Parse(buffer_);
  if (req_status < HttpParser::kComplete) {
    connection_status_ = KEEP_READING;
    buffer_.assign(BUFFER_SIZE + 1, 0);
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
      req_data.status, req_status, req_data.request, location_data);
  if (response_manager == NULL) {
    connection_status_ = CONNECTION_ERROR;
    std::cerr << "Connection: memory allocation failure\n";
    return ResponseManager::IoFdPair(-1, -1);
  }
  ResponseManager::IoFdPair io_fds = response_manager->Execute();

  if (io_fds.input == -1 && io_fds.output == -1) {
    std::cerr << "io_fds are -1\n";
    // if (location_data.is_cgi == true &&
    //     response_manager->get_result().is_local_redir == true) {
    //   std::string redir_path =
    //       response_manager->get_result().header["location"];
    //   // request resetting
    //   int status = ValidateLocalRedirPath(redir_path, request.req,
    //   req_status); location_data =
    //       router_->Route(status, request, ConnectionInfo(port_,
    //       client_addr_));
    //   delete response_manager;
    //   response_manager =
    //       GenerateResponseManager(status, req_status, request,
    //       location_data);
    //   io_fds = response_manager->Execute();
    //   if (io_fds.input == -1 && io_fds.output == -1) {
    //     response_manager->FormatHeader();
    //     UpdateRequestResult(response_manager->get_is_keep_alive());
    //   } else {
    //     if (io_fds.input != -1) {
    //       resource_manager_map_[io_fds.input] = response_manager;
    //     }
    //     if (io_fds.output != -1) {
    //       resource_manager_map_[io_fds.output] = response_manager;
    //     }
    //   }
    // }
    response_manager->FormatHeader();
  } else {
    if (io_fds.input != -1) {
      resource_manager_map_[io_fds.input] = response_manager;
      std::cerr << "input fd : " << io_fds.input
                << ", RM address : " << response_manager << std::endl;
    }
    if (io_fds.output != -1) {
      resource_manager_map_[io_fds.output] = response_manager;
      std::cerr << "output fd : " << io_fds.output
                << ", RM address : " << response_manager << std::endl;
    }
  }
  UpdateRequestResult(response_manager->get_is_keep_alive());
  std::cerr << "In Execute : resource manager map size : "
            << resource_manager_map_.size() << '\n';
  return io_fds;
}

ResponseManager::IoFdPair Connection::ExecuteMethod(int event_fd) {
  std::cerr << "event fd : " << event_fd << '\n';
  std::cerr << "IN Execute Method: resource manager map size : "
            << resource_manager_map_.size() << '\n';
  ResponseManager* manager = resource_manager_map_[event_fd];
  std::cerr << "manager : " << manager << '\n';
  ResponseManager::IoFdPair io_fds = manager->Execute();
  if (io_fds.input == -1 && io_fds.output == -1) {
    std::cerr << "io_fds are -1\n";
    manager->FormatHeader();
    UpdateRequestResult(manager->get_is_keep_alive());
  } else {
    if (io_fds.input != -1) {
      resource_manager_map_[io_fds.input] = manager;
      std::cerr << "input fd : " << io_fds.input << ", RM address : " << manager
                << std::endl;
    }
    if (io_fds.output != -1) {
      resource_manager_map_[io_fds.output] = manager;
      std::cerr << "output fd : " << io_fds.output
                << ", RM address : " << manager << std::endl;
    }
    std::cerr << "In Execute Method 2 : resource manager map size : "
              << resource_manager_map_.size() << '\n';
  }

  // if (manager->get_status() == IO_COMPLETE) {
  //   resource_manager_map_[event_fd] = NULL;
  //   delete manager;
  //   return ResponseManager::IoFdPair(-1, -1);
  // }
  // local redir?
  return io_fds;
}

// {
//   // 여기까지 할 일 이라고 생각하는데 ,,,,,,,,
//   // 이건 어쩌지?
//   ExecuteMethod(res.content, router_result, request);
//   // ResponseManager::Result exec_result =
//   //     resource_manager_.ExecuteMethod(res.content, router_result,
//   request);
//  if (exec_result.is_local_redir == true) {
//     std::string redir_path = exec_result.header["location"];
//     status = ValidateLocalRedirPath(redir_path, request.req, req_status);
//     router_result =
//         router_->Route(status, request, ConnectionInfo(port_, client_addr_));
//     exec_result =
//         resource_manager_.ExecuteMethod(res.content, router_result, request);
//   }
//   bool is_keep_alive =
//       (exec_result.status < 500 && req_status == HttpParser::kComplete);
//   res.header = response_formatter_.Format(res.content.size(), exec_result,
//                                           request.req.version,
//                                           router_result.methods,
//                                           is_keep_alive);
//   res.total_len = res.header.size() + res.content.size();
//   UpdateRequestResult(is_keep_alive);
// }

void Connection::FormatResponse(const int event_fd) {
  std::cerr << "connection : FormatResponse\n";
  ResponseManager* manager = resource_manager_map_[event_fd];
  manager->Execute(true);
  manager->FormatHeader();
  UpdateRequestResult(manager->get_is_keep_alive());
  delete manager;
  resource_manager_map_[event_fd] = NULL;
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
  std::cerr << "sent so far : " << response.offset << "\n";
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
  if (recv_byte < BUFFER_SIZE) {
    if (recv_byte != -1) {  // -1 뜨면 운명임.
      buffer_.erase(recv_byte);
    }
  }
}

ResponseManager* Connection::GenerateResponseManager(
    int status, int req_status, Request& request,
    Router::Result& router_result) {
  response_queue_.push(ResponseBuffer());
  ResponseManager::Result result(router_result.status);
  if (router_result.status >= 400 || router_result.is_cgi == false) {
    return new (std::nothrow)
        FileManager((req_status == HttpParser::kComplete),
                    response_queue_.back(), router_result, request);
  } else {  // Cgi found
    return new (std::nothrow)
        CgiManager((req_status == HttpParser::kComplete),
                   response_queue_.back(), router_result, request);
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
