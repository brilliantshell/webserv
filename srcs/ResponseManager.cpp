/**
 * @file ResponseManager.cpp
 * @author ghan, jiskim, yongjule
 * @brief Execute methods and manage resources according to the Client's request
 * @date 2022-10-11
 *
 * @copyright Copyright (c) 2022
 */

#include "ResponseManager.hpp"

/**
 * @brief ResponseManager 객체 생성 (FileManager & CgiManager 의 Base 객체)
 *
 * @param is_keep_alive true면 keep-alive, false면 close
 * @param response_buffer 응답을 저장할 버퍼
 * @param router_result 라우터에서 반환한 결과
 * @param request Client의 요청
 */
ResponseManager::ResponseManager(bool is_keep_alive,
                                 ResponseBuffer& response_buffer,
                                 Router::Result& router_result,
                                 Request& request)
    : is_keep_alive_(is_keep_alive),
      io_status_(IO_START),
      err_fd_(-1),
      file_size_(0),
      result_(router_result.status),
      request_(request),
      router_result_(router_result),
      response_buffer_(response_buffer) {}

/**
 * @brief ResponseManager 객체 소멸, 에러 페이지 fd close
 *
 */
ResponseManager::~ResponseManager(void) { close(err_fd_); }

/**
 * @brief HTTP 규격에 맞게 응답 헤더 작성
 *
 */
void ResponseManager::FormatHeader(void) {
  const int kStatus = result_.status;
  std::stringstream ss;
  ss << (request_.req.version == HttpParser::kHttp1_1 ? "HTTP/1.1 "
                                                      : "HTTP/1.0 ")
     << kStatus << " " << g_status_map[kStatus] << CRLF
     << "server: BrilliantServer/1.0" << CRLF
     << "date: " + header_formatter_.FormatCurrentTime() << CRLF
     << ((kStatus == 301 || kStatus == 400 || kStatus == 404 || kStatus >= 500)
             ? ""
             : ("allow: " +
                header_formatter_.FormatAllowed(router_result_.methods) + CRLF))
     << "connection: "
     << ((kStatus < 500 && is_keep_alive_ == true) ? "keep-alive" : "close")
     << CRLF << "content-length: " << response_buffer_.content.size() << CRLF;
  std::string content_type = header_formatter_.FormatContentType(
      result_.is_autoindex, result_.ext, result_.header);
  if (content_type.empty() == false) {
    ss << "content-type: " << content_type << CRLF;
  }
  if (result_.location.empty() == false) {  // 201 || 301 || 302
    ss << "location: " << result_.location << CRLF;
  }
  header_formatter_.ResolveConflicts(result_.header);
  for (ResponseHeaderMap::const_iterator it = result_.header.begin();
       it != result_.header.end(); ++it) {
    ss << it->first << ": " << it->second << CRLF;
  }
  ss << CRLF;
  response_buffer_.header = ss.str();
  response_buffer_.is_complete = true;
}

/**
 * @brief is_keep_alive 값 반환
 *
 * @return true
 * @return false
 */
bool ResponseManager::get_is_keep_alive(void) const { return is_keep_alive_; }

/**
 * @brief response buffer 반환
 *
 * @return ResponseBuffer&
 */
ResponseBuffer& ResponseManager::get_response_buffer(void) {
  return response_buffer_;
}

/**
 * @brief request 반환
 *
 * @return Request&
 */
Request& ResponseManager::get_request(void) { return request_; }

/**
 * @brief Execute() 결과 반환
 *
 * @return ResponseManager::Result&
 */
ResponseManager::Result& ResponseManager::get_result(void) { return result_; }

// SECTION: protected
/**
 * @brief GET 요청/에러일 때 파일 읽기
 *
 * @param fd 파일 fd
 * @return ssize_t read 한 byte 수
 */
ssize_t ResponseManager::ReadFile(int fd) {
  char read_buf[READ_BUFF_SIZE + 1];
  memset(read_buf, 0, READ_BUFF_SIZE + 1);
  ssize_t read_bytes = read(fd, read_buf, READ_BUFF_SIZE);
  if (read_bytes > 0) {
    response_buffer_.content.append(read_buf, read_bytes);
    if (read_bytes < READ_BUFF_SIZE ||
        response_buffer_.content.size() == static_cast<size_t>(file_size_)) {
      io_status_ = SetIoComplete(IO_COMPLETE);
    } else {
      io_status_ = (io_status_ < ERROR_START) ? FILE_READ : ERROR_READ;
    }
    return read_bytes;
  }
  if (read_bytes == -1) {
    response_buffer_.content.clear();
    result_.status = 500;  // INTERNAL SERVER ERROR
    return -1;
  }
  io_status_ = SetIoComplete(IO_COMPLETE);
  return read_bytes;
}

/**
 * @brief 상태 코드에 따라 에러 페이지 응답 생성
 *
 * @return ResponseManager::IoFdPair <에러 페이지 fd, -1> / <-1, -1>
 */
ResponseManager::IoFdPair ResponseManager::GetErrorPage(void) {
  is_keep_alive_ = (result_.status < 500);
  const char* kErrorPath = router_result_.error_path.c_str();
  if (access(kErrorPath, F_OK) == -1) {
    return GenerateDefaultError();
  }
  struct stat st;
  if (stat(kErrorPath, &st) == -1 || S_ISDIR(st.st_mode) == true) {
    return HandleGetErrorFailure();
  }
  file_size_ = st.st_size;
  if (io_status_ == ERROR_START || io_status_ == IO_START) {
    err_fd_ = open(kErrorPath, O_RDONLY);
    if (err_fd_ == -1) {
      return HandleGetErrorFailure();
    }
    fcntl(err_fd_, F_SETFL, O_NONBLOCK);
    io_status_ = ERROR_READ;
  }
  if (io_status_ == ERROR_READ && ReadFile(err_fd_) == -1) {
    return HandleGetErrorFailure();
  }
  if (io_status_ == ERROR_READ) {
    return IoFdPair(err_fd_, -1);
  }
  result_.ext = ParseExtension(router_result_.error_path);
  return IoFdPair();
}

/**
 * @brief Content-Type 헤더 작성을 위한 파일 확장자 추출
 *
 * @param kPath 클라이언트가 요청한 파일 경로
 * @return std::string 파일 확장자
 */
std::string ResponseManager::ParseExtension(const std::string& kPath) {
  size_t last_slash = kPath.rfind('/');
  if (last_slash > kPath.size() - 3) {
    return "";
  }
  size_t last_dot = kPath.rfind('.');
  if (last_dot == std::string::npos || last_dot < last_slash ||
      last_dot == kPath.size() - 1) {
    return "";
  }
  return kPath.substr(last_dot + 1);
}

/**
 * @brief  I/O 작업 완료 상태 설정 및 fd close
 *
 * @param status I/O 작업 상태
 * @return int 설정하는 io_status_
 */
int ResponseManager::SetIoComplete(int status) {
  close(err_fd_);
  err_fd_ = -1;
  return status;
}

// SECTION: private
/**
 * @brief 에러 페이지 I/O 실패시 에러 코드 설정 및 에러 페이지 작성
 *
 * @return IoFdPair <-1, -1>
 */
ResponseManager::IoFdPair ResponseManager::HandleGetErrorFailure(void) {
  result_.status = 500;  // INTERNAL_SERVER_ERROR
  is_keep_alive_ = false;
  response_buffer_.content = LAST_ERROR_DOCUMENT;
  io_status_ = SetIoComplete(IO_COMPLETE);
  return IoFdPair();
}

/**
 * @brief 에러 페이지가 없을 경우 기본 에러 페이지 작성
 *
 * @return IoFdPair <-1, -1>
 */
ResponseManager::IoFdPair ResponseManager::GenerateDefaultError(void) {
  std::stringstream ss;
  ss << result_.status << " " << g_status_map[result_.status];
  response_buffer_.content = "<!DOCTYPE html><title>" + ss.str() +
                             "</title><body><h1>" + ss.str() +
                             "</h1></body></html>";
  router_result_.error_path = "default_error.html";
  io_status_ = SetIoComplete(IO_COMPLETE);
  result_.ext = "html";
  return IoFdPair();
}
