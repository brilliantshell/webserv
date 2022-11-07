/**
 * @file Connection.cpp
 * @author ghan, jiskim, yongjule
 * @brief Manage connection between client and server
 * @date 2022-10-27
 *
 * @copyright Copyright (c) 2022
 */

#include "Connection.hpp"

/**
 * @brief Connection 객체 생성
 *
 */
Connection::Connection(void)
    : fd_(-1),
      connection_status_(KEEP_ALIVE),
      send_status_(KEEP_SENDING),
      buffer_(BUFFER_SIZE, 0),
      router_(NULL) {}

/**
 * @brief Connection 객체 소멸 및 Router, ResponseManagerMap 정리
 *
 */
Connection::~Connection(void) {
  close(fd_);
  if (router_ != NULL) {
    delete router_;
    router_ = NULL;
  }
  response_manager_map_.Clear();
}

/**
 * @brief Connection 객체 재설정
 *
 * @param does_next_req_exist 다음 요청 존재 여부
 */
void Connection::Reset(bool does_next_req_exist) {
  if (does_next_req_exist == true) {
    connection_status_ = NEXT_REQUEST_EXISTS;
    parser_.Clear();
    buffer_.erase();
  } else {
    parser_.Reset();
    buffer_.assign(BUFFER_SIZE, 0);
  }
}

/**
 * @brief Connection 객체 초기화
 *
 */
void Connection::Clear(void) {
  close(fd_);
  fd_ = -1;
  port_ = 0;
  connection_status_ = KEEP_ALIVE;
  send_status_ = KEEP_SENDING;
  parser_.Reset();
  buffer_.assign(BUFFER_SIZE, 0);
  client_addr_.clear();
  if (router_ != NULL) {
    delete router_;
    router_ = NULL;
  }
  while (response_queue_.empty() == false) {
    response_queue_.pop();
  }
  response_manager_map_.Clear();
}

/**
 * @brief Request 파싱, 라우팅, response 생성할 response manager 생성 후 실행
 *
 * @return ResponseManager::IoFdPair I/O fd 반환 <input fd, output fd>
 */
ResponseManager::IoFdPair Connection::HandleRequest(void) {
  if (connection_status_ == CLOSE) {
    return SetConnectionError<ResponseManager::IoFdPair>("");
  }
  if (connection_status_ != NEXT_REQUEST_EXISTS) {
    if (Receive() < 0) {
      return SetConnectionError<ResponseManager::IoFdPair>(
          "Connection : recv failed");
    }
  }
  int req_status = parser_.Parse(buffer_);
  if (req_status < HttpParser::kComplete) {
    connection_status_ = KEEP_READING;
    buffer_.assign(BUFFER_SIZE, 0);
    return ResponseManager::IoFdPair();
  }
  HttpParser::Result req_data = parser_.get_result();
  Request& request = req_data.request;
  PRINT_REQ_LOG(request.req);
  Router::Result location_data = router_->Route(
      req_data.status, request, ConnectionInfo(port_, client_addr_));
  response_queue_.push(ResponseBuffer());
  ResponseManager* response_manager =
      GenerateResponseManager((req_status == HttpParser::kComplete), request,
                              location_data, response_queue_.back());
  if (response_manager == NULL) {
    return SetConnectionError<ResponseManager::IoFdPair>(
        "Connection : memory allocation failure");
  }
  ResponseManager::IoFdPair io_fds = response_manager->Execute();
  DetermineIoComplete(io_fds, response_manager);
  send_status_ = KEEP_SENDING;
  Reset(connection_status_ == KEEP_ALIVE && parser_.DoesNextReqExist() == true);
  return io_fds;
}

/**
 * @brief event 발생한 fd를 key 로 사용하여 맞는 response manager 를 찾아 요청을
 * 이어서 처리
 *
 * @param event_fd 이벤트가 발생한 I/O fd
 * @return ResponseManager::IoFdPair I/O fd 반환 <input fd, output fd>
 */
ResponseManager::IoFdPair Connection::ExecuteMethod(int event_fd) {
  ResponseManagerMap::iterator it = response_manager_map_.find(event_fd);
  if (it == response_manager_map_.end()) {
    return SetConnectionError<ResponseManager::IoFdPair>(
        "Connection : event_fd not found");
  }
  ResponseManager* manager = it->second;
  ResponseManager::IoFdPair io_fds = manager->Execute();
  ResponseManager::Result& response_result = manager->get_result();
  if (response_result.is_local_redir == true) {
    io_fds =
        HandleCgiLocalRedirection(&manager, response_result.header["location"]);
  }
  DetermineIoComplete(io_fds, manager);
  return io_fds;
}

/**
 * @brief Response 를 FIFO 로 전송
 *
 */
void Connection::Send(void) {
  if (response_queue_.empty() == true) {
    return SetConnectionError<void>("");
  }
  ResponseBuffer& response = response_queue_.front();
  struct iovec iovec[2];
  size_t iov_cnt = SetIov(iovec, response);
  ssize_t sent_bytes = writev(fd_, iovec, iov_cnt);
  if (sent_bytes < 0) {
    return SetConnectionError<void>("writev error :" +
                                    std::string(strerror(errno)));
  }
  response.offset += sent_bytes;
  if (response.cur_buf == ResponseBuffer::kHeader &&
      response.offset >= response.header.size()) {
    response.cur_buf = ResponseBuffer::kContent;
  }
  send_status_ = KEEP_SENDING;
  if (response.offset >= response.header.size() + response.content.size()) {
    response_queue_.pop();
    send_status_ = SEND_NEXT;
    if (response_queue_.empty()) {
      send_status_ = SEND_FINISHED;
    }
  }
}

/**
 * @brief Connection 객체 멤버 변수 설정
 *
 * @param kFd socket fd
 * @param kClientAddr client 주소
 * @param kPort 연결된 port
 * @param server_router port 에 해당하는 서버 라우터
 */
void Connection::SetAttributes(const int kFd, const std::string& kClientAddr,
                               const uint16_t kPort,
                               ServerRouter& server_router) {
  fd_ = kFd;
  client_addr_ = kClientAddr;
  port_ = kPort;
  router_ = new (std::nothrow) Router(server_router);
  if (router_ == NULL) {
    SetConnectionError<void>("Connection : Router memory allocation failure");
  }
}

/**
 * @brief response buffer 가 send 할 준비 되었는지 확인
 *
 * @return true
 * @return false
 */
bool Connection::IsResponseBufferReady(void) const {
  if (response_queue_.empty() == true) {
    return false;
  }
  return response_queue_.front().is_complete;
}

/**
 * @brief 현재 처리중인 요청과 큐에 있는 요청이 동일한지 확인
 *
 * @return true
 * @return false
 */
bool Connection::IsHttpPairSynced(void) const {
  return (response_manager_map_.size() == response_queue_.size());
}

/**
 * @brief 응답 송신 상태 반환
 *
 * @return int
 */
int Connection::get_send_status(void) const { return send_status_; }

/**
 * @brief Connection 의 상태 반환
 *
 * @return int
 */
int Connection::get_connection_status(void) const { return connection_status_; }

/**
 * @brief 현 Connection 의 fd 반환
 *
 * @return int
 */
int Connection::get_fd(void) const { return fd_; }

// SECTION : private
/**
 * @brief 요청 수신
 *
 * @return recv_byte 한 회 수신한 바이트 수
 */
ssize_t Connection::Receive(void) {
  ssize_t recv_byte = recv(fd_, &buffer_[0], BUFFER_SIZE, 0);
  if (recv_byte <= BUFFER_SIZE && recv_byte != -1) {
    buffer_.erase(recv_byte);
  }
  return recv_byte;
}

/**
 * @brief File/PIPE I/O 완료 여부에 따라 response manager 를 삭제하거나
 * 자원 fd & ResponseManager 매핑
 *
 * @param io_fds File/PIPE I/O fd
 * @param manager 해당 응답 처리 중인 ResponseManager 객체 포인터
 */
void Connection::DetermineIoComplete(ResponseManager::IoFdPair& io_fds,
                                     ResponseManager* manager) {
  connection_status_ =
      (manager->get_is_keep_alive() == true) ? KEEP_ALIVE : CLOSE;
  if (io_fds.input == -1 && io_fds.output == -1) {
    manager->FormatHeader();
    response_manager_map_.Erase(manager);
    return delete manager;
  }
  if (io_fds.input != -1) {
    response_manager_map_[io_fds.input] = manager;
  }
  if (io_fds.output != -1) {
    response_manager_map_[io_fds.output] = manager;
  }
}

/**
 * @brief CGI 요청이면 CgiManager 객체, 아니면 FileManager 객체 생성
 *
 * @param is_keep_alive keep-alive 여부
 * @param request 요청 정보 구조체
 * @param router_result 라우팅 결과 구조체
 * @param response_buffer 해당 요청에 할당된 ResponseBuffer
 * @return ResponseManager* 동적 할당 된 CgiManager/FileManager 객체
 */
ResponseManager* Connection::GenerateResponseManager(
    bool is_keep_alive, Request& request, Router::Result& router_result,
    ResponseBuffer& response_buffer) {
  if (router_result.status >= 400 || router_result.is_cgi == false) {
    return new (std::nothrow)
        FileManager(is_keep_alive, response_buffer, router_result, request);
  }
  return new (std::nothrow)
      CgiManager(is_keep_alive, response_buffer, router_result, request);
}

/**
 * @brief CGI 로컬 리다이렉션 처리
 *
 * @param manager 현재 요청 처리 중인 ResponseManager
 * @param local_redir_path CGI 로컬 리다이렉션 경로
 */
ResponseManager::IoFdPair Connection::HandleCgiLocalRedirection(
    ResponseManager** manager, std::string& local_redir_path) {
  Request request = (*manager)->get_request();
  bool is_keep_alive = (*manager)->get_is_keep_alive();
  int status = ValidateLocalRedirPath(local_redir_path, request.req);
  Router::Result location_data =
      router_->Route(status, request, ConnectionInfo(port_, client_addr_));
  ResponseBuffer& current_response_buffer = (*manager)->get_response_buffer();
  response_manager_map_.Erase(*manager);
  delete *manager;
  *manager = GenerateResponseManager(is_keep_alive, request, location_data,
                                     current_response_buffer);
  return (*manager != NULL) ? (*manager)->Execute()
                            : SetConnectionError<ResponseManager::IoFdPair>(
                                  "Connection : memory allocation failure");
}

/**
 * @brief CGI 로컬 리다이렉션 시 Location 헤더 필드 값 유효성 검사 후
 * 적절한 상태코드 설정
 *
 * @param path Location 헤더 필드값
 * @param req 업데이트 되어야할 RequestLine 구조체
 * @return int 상태 코드
 */
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
  req.path = uri_result.path + path_resolver.get_file_name();
  req.query = uri_result.query;
  return 200;
}

/**
 * @brief writev 호출 전 송신할 버퍼 offset 맞춰서 iovec 구조체에 설정
 *
 * @param iov 외부에서 선언 된 iovec 구조체 배열
 * @param res 처리 중인 요청의 Responsebuffer 구조체
 * @return size_t iovec 배열 크기
 */
size_t Connection::SetIov(struct iovec* iov, ResponseBuffer& res) {
  std::string& header = res.header;
  std::string& content = res.content;
  size_t offset = res.offset;
  size_t cnt = (res.cur_buf == ResponseBuffer::kHeader &&
                header.size() < SEND_BUFF_SIZE + offset)
                   ? 2
                   : 1;
  if (res.cur_buf == ResponseBuffer::kHeader) {
    iov[0].iov_base = &header[0] + offset;
    iov[0].iov_len = (header.size() < SEND_BUFF_SIZE + offset)
                         ? header.size() - offset
                         : SEND_BUFF_SIZE;
    if (cnt == 2) {
      iov[1].iov_base = &content[0];
      iov[1].iov_len = (content.size() + iov[0].iov_len < SEND_BUFF_SIZE)
                           ? content.size()
                           : SEND_BUFF_SIZE - iov[0].iov_len;
    }
  } else {
    iov[0].iov_base = &content[0] + (offset - header.size());
    size_t total_len = header.size() + content.size();
    iov[0].iov_len = (total_len < SEND_BUFF_SIZE + offset) ? total_len - offset
                                                           : SEND_BUFF_SIZE;
  }
  return cnt;
}

/**
 * @brief connection_status_ CONNECTION 에러로 설정, 에러 메시지 출력
 *
 * @tparam T ResponseMananger::IoFdPair/void
 * @param kMsg 에러 메시지
 * @return T ResponseMananger::IoFdPair/void
 */
template <typename T>
T Connection::SetConnectionError(const std::string& kMsg) {
  if (kMsg.size() > 0) {
    PRINT_ERROR(kMsg);
  }
  connection_status_ = CONNECTION_ERROR;
  return T();
}
