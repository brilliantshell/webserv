/**
 * @file CgiManager.cpp
 * @author ghan, jiskim, yongjule
 * @brief Execute cgi script and handle IPC between the server and the script
 * @date 2022-10-14
 *
 * @copyright Copyright (c) 2022
 */

#include "CgiManager.hpp"

/**
 * @brief CGI 요청이 왔을 때 처리하는 CgiManager 객체 생성
 *
 * @param is_keep_alive 매니저가 처리할 요청의 keep-alive 여부
 * @param response 처리 후 생성해 줄 Response 객체
 * @param router_result 처리할 파일의 정보
 * @param request 요청 정보
 */
CgiManager::CgiManager(bool is_keep_alive, ResponseBuffer& response,
                       Router::Result& router_result, Request& request)
    : ResponseManager(is_keep_alive, response, router_result, request),
      is_header_(true),
      pid_(-1),
      write_offset_(0),
      response_content_(response.content) {}

/**
 * @brief CGI Manager 객체 소멸 시 파이프 fd 정리
 *
 */
CgiManager::~CgiManager(void) {
  close(out_fd_[0]);
  close(out_fd_[1]);
  close(in_fd_[0]);
  close(in_fd_[1]);
}

/**
 * @brief 요청 처리 상태에 따라 CGI 프로세스 실행 혹은 PIPE read, write 연산
 *
 * @return ResponseManager::IoFdPair read 중일 시 <read fd, -1>, write 중일 시
 * <-1, write fd>, 에러 시 <-1, -1> 반환.
 */
ResponseManager::IoFdPair CgiManager::Execute(void) {
  if (io_status_ == IO_START) {
    SetIpc();
  } else if (io_status_ == PIPE_WRITE) {
    PassContent();
  } else if (io_status_ == PIPE_READ) {
    if (ReceiveCgiResponse(result_.header) == false) {
      SetInternalServerError();
    }
    if (io_status_ == IO_COMPLETE) {
      if (ParseCgiHeader(result_.header) == false) {
        SetInternalServerError();
      }
      result_.ext = ParseExtension(router_result_.success_path);
    }
  }
  return (result_.status >= 400)
             ? GetErrorPage()
             : ResponseManager::IoFdPair(out_fd_[0], in_fd_[1]);
}

// SECTION : private

// SECTION : child

/**
 * @brief CGI 프로세스 실행을 위한 파이프 fd 리다이렉션
 *
 */
void CgiManager::DupFds(void) {
  if (dup2(out_fd_[1], STDOUT_FILENO) == -1) {
    exit(EXIT_FAILURE);
  }
  close(out_fd_[1]);
  close(out_fd_[0]);
  out_fd_[0] = -1;
  out_fd_[1] = -1;
  if (dup2(in_fd_[0], STDIN_FILENO) == -1) {
    exit(EXIT_FAILURE);
  }
  close(in_fd_[0]);
  close(in_fd_[1]);
  in_fd_[0] = -1;
  in_fd_[1] = -1;
}

/**
 * @brief Command line 형태로 query 요청이 왔을때 argv로 파싱
 *
 * @param arg_vector 파싱된 argv를 저장할 vector
 * @param query 요청 query
 */
void CgiManager::ParseScriptCommandLine(std::vector<std::string>& arg_vector,
                                        std::string query) {
  if (*(query.rbegin()) == '+') {
    return;
  }
  query.erase(0, 13);  // QUERY_STRING=
  if (query.empty() == false && query.find("=") == std::string::npos) {
    size_t start = 0;
    for (size_t plus_pos = query.find("+"); plus_pos != std::string::npos;
         plus_pos = query.find("+", start)) {
      arg_vector.push_back(query.substr(start, plus_pos - start));
      start = plus_pos + 1;
    }
    if (start < query.size()) {
      arg_vector.push_back(query.substr(start));
    }
  }
  for (size_t i = 0; i < arg_vector.size(); ++i) {
    for (size_t k = 0; k < arg_vector[i].size(); ++k) {
      if (arg_vector[i][k] == '%') {
        UriParser().DecodeHexToAscii(arg_vector[i], k);
      }
    }
  }
}

/**
 * @brief CGI 프로세스의 환경변수와 인자 설정 및 스크립트 실행
 *
 * @param kSuccessPath CGI 스크립트의 경로
 * @param kEnv CGI 스크립트 실행에 필요한 환경변수
 */
void CgiManager::ExecuteScript(const char* kSuccessPath, char* const* kEnv) {
  // 1. ENAMETOOLONG, 2.ENOMEM(dirname)
  if (kEnv == NULL) {
    exit(EXIT_FAILURE);
  }
  char* new_cwd = dirname(const_cast<char*>(kSuccessPath));
  if (new_cwd == NULL || chdir(new_cwd) == -1) {
    exit(EXIT_FAILURE);
  }
  char* script_path = basename(const_cast<char*>(kSuccessPath));
  if (script_path == NULL) {
    exit(EXIT_FAILURE);
  }
  DupFds();
  std::vector<std::string> arg_vector;
  ParseScriptCommandLine(arg_vector, kEnv[6]);
  const char** kArgv = new (std::nothrow) const char*[arg_vector.size() + 2];
  if (kArgv == NULL) {
    exit(EXIT_FAILURE);
  }
  memset(kArgv, 0, sizeof(char*) * (arg_vector.size() + 2));
  kArgv[0] = script_path;
  for (size_t i = 0; i < arg_vector.size(); ++i) {
    kArgv[i + 1] = arg_vector[i].c_str();
  }
  alarm(5);  // CGI script timeout
  execve(script_path, const_cast<char* const*>(kArgv), kEnv);
  exit(EXIT_FAILURE);
}

// SECTION : parent

/**
 * @brief CGI 프로세스와 서버의 통신 설정 및 CGI 프로세스 실행
 *
 */
void CgiManager::SetIpc(void) {
  const char* kPath = router_result_.success_path.c_str();
  if (CheckFileMode(kPath) == false) {
    io_status_ = SetIoComplete(ERROR_START);
    return;
  }
  if (OpenPipes() == false) {
    result_.status = 500;
    io_status_ = SetIoComplete(ERROR_START);
    return;
  }
  pid_ = fork();
  if (pid_ == 0) {
    ExecuteScript(kPath,
                  const_cast<char* const*>(router_result_.cgi_env.get_env()));
  } else if (pid_ > 0) {
    io_status_ = PIPE_WRITE;
  } else {
    result_.status = 500;
    io_status_ = SetIoComplete(ERROR_START);
  }
}

/**
 * @brief CGI 프로세스와 서버의 통신을 위한 파이프 open, non-block 세팅
 *
 * @return true 파이프 open 성공
 * @return false 파이프 open 실패
 */
bool CgiManager::OpenPipes(void) {
  if (pipe(out_fd_) == -1) {
    return false;
  }
  if (pipe(in_fd_) == -1) {
    close(out_fd_[0]);
    close(out_fd_[1]);
    out_fd_[0] = -1;
    out_fd_[1] = -1;
    return false;
  }
  fcntl(out_fd_[0], F_SETFL, O_NONBLOCK);
  fcntl(in_fd_[1], F_SETFL, O_NONBLOCK);
  return true;
}

/**
 * @brief 실행할 CGI 스크립트가 실행가능 한 지 판단하고 불가능하면 에러 설정
 *
 * @param kPath CGI 스크립트 경로
 * @return true 실행 가능
 * @return false 실행 불가능
 */
bool CgiManager::CheckFileMode(const char* kPath) {
  if (access(kPath, X_OK) == -1) {
    if (errno == ENOENT) {
      result_.status = 404;  // PAGE NOT FOUND
    } else if (errno == EACCES) {
      result_.status = 403;  // FORBIDDEN
    } else {
      result_.status = 500;  // INTERNAL_SERVER_ERROR
    }
  }
  return (result_.status < 400);
}

/**
 * @brief CGI PIPE 에 content 전송
 *
 */
void CgiManager::PassContent(void) {
  size_t size = (request_.content.size() < write_offset_ + PIPE_BUF_SIZE)
                    ? request_.content.size() - write_offset_
                    : PIPE_BUF_SIZE;
  errno = 0;
  ssize_t sent_bytes = write(in_fd_[1], &request_.content[write_offset_], size);
  if (sent_bytes < 0) {
    result_.status = 500;  // INTERNAL SERVER ERROR
    kill(pid_, SIGTERM);
    io_status_ = SetIoComplete(ERROR_START);
    return;
  }
  write_offset_ += sent_bytes;
  io_status_ =
      (write_offset_ >= request_.content.size()) ? PIPE_READ : PIPE_WRITE;
  if (io_status_ == PIPE_READ) {
    close(out_fd_[1]);
    close(in_fd_[0]);
    close(in_fd_[1]);
    out_fd_[1] = -1;
    in_fd_[0] = -1;
    in_fd_[1] = -1;
  }
}

/**
 * @brief CGI 응답 헤더 라인 별로 필드 분리하여 header 에 저장
 *
 * @param header CGI 응답 헤더 필드 담을 구조체
 * @param header_end CGI 응답 헤더 끝 인덱스
 * @return true
 * @return false
 */
bool CgiManager::ReceiveCgiHeaderFields(ResponseHeaderMap& header,
                                        size_t header_end) {
  std::string buf(header_read_buf_, 0, header_end);
  size_t start = 0;
  for (size_t eol = buf.find(CRLF);
       eol < buf.size() && eol != std::string::npos;
       eol = buf.find(CRLF, start)) {
    if (eol - start > FIELD_LINE_MAX) {
      return false;
    }
    std::string line(buf, start, eol - start);
    size_t del = line.find(":");
    if (del == std::string::npos) {
      return false;
    }
    if (del + 1 < line.size()) {
      size_t value_start = line.find_first_not_of(" \t", del + 1);
      if (value_start == std::string::npos) {
        continue;
      }
      std::string name(line, 0, del);
      std::string value(line, value_start);
      std::transform(name.begin(), name.begin() + del, name.begin(), ::tolower);
      if (header.insert(std::make_pair(name, value)).second == false) {
        return false;
      }
    }
    start = eol + 2;
  }
  return true;
}

/**
 * @brief PIPE 통해 CGI 프로세스로 부터 받은 응답 읽기, 헤더와 바디 구분하여
 * 저장
 *
 * @param header CGI 응답 헤더 필드 담을 구조체
 * @return true
 * @return false
 */
bool CgiManager::ReceiveCgiResponse(ResponseHeaderMap& header) {
  char buf[PIPE_BUF_SIZE + 1];
  memset(buf, 0, PIPE_BUF_SIZE + 1);

  ssize_t read_size = read(out_fd_[0], buf, PIPE_BUF_SIZE);
  if (read_size < 0) {
    return false;
  }
  if (read_size < PIPE_BUF_SIZE) {
    io_status_ = SetIoComplete(IO_COMPLETE);
  }
  if (is_header_ == true) {
    header_read_buf_.append(buf, read_size);
    if (header_read_buf_.size() > HEADER_MAX) {
      return false;
    }
    size_t header_end = header_read_buf_.find(CRLF CRLF);
    if (header_end != std::string::npos) {
      if (ReceiveCgiHeaderFields(header, header_end + 2) == false) {
        return false;
      }
      is_header_ = false;
      response_content_.append(header_read_buf_, header_end + 4);
    }
  } else {
    response_content_.append(buf, read_size);
    if (response_content_.size() > CONTENT_MAX) {
      return false;
    }
  }
  return true;
}

/**
 * @brief CGI 응답 종류 판별 (Document/Local Redirection/Client Redirection)
 *
 * @param header CGI 응답 헤더 필드
 * @return int
 */
int CgiManager::DetermineResponseType(ResponseHeaderMap& header) {
  if (header.size() == 0) {
    return kError;
  }
  bool is_content_type = header.count("content-type");
  bool is_status = header.count("status");
  ResponseHeaderMap::const_iterator it = header.find("location");
  if (it != header.end()) {
    const std::string& kLocation = it->second;
    if (kLocation[0] == '/') {
      return (header.size() > 1 || response_content_.size() > 0) ? kError
                                                                 : kLocalRedir;
    }
    if (UriParser().ParseTarget(kLocation).is_valid == false) {
      return kError;
    }
    if (response_content_.size() > 0) {
      return (is_content_type == false || is_status == false) ? kError
                                                              : kClientRedirDoc;
    }
    return (is_content_type == true || is_status == true) ? kError
                                                          : kClientRedir;
  } else if (is_content_type == true) {
    return kDocument;
  }
  return kError;
}

/**
 * @brief CGI 응답 헤더 파싱 및 검증
 *
 * @param header CGI 응답 헤더 필드
 * @return true
 * @return false
 */
bool CgiManager::ParseCgiHeader(ResponseHeaderMap& header) {
  int type = DetermineResponseType(header);
  if (type == kError) {
    return false;
  }
  ResponseHeaderMap::const_iterator it = header.find("status");
  if (it != header.end()) {
    std::stringstream ss(it->second);
    ss >> result_.status;
    if (result_.status < 100 || result_.status > 999) {
      return false;
    }
    header.erase(it);
  }
  result_.is_local_redir = (type == kLocalRedir);
  if (type == kClientRedir || type == kClientRedirDoc) {
    result_.status = 302;
  }
  std::vector<std::string> names;
  for (it = header.begin(); it != header.end(); ++it) {
    names.push_back(it->first);
  }
  for (size_t i = 0; i < names.size(); ++i) {
    if (names[i].size() > 6 && names[i].compare(0, 6, "x-cgi-") == 0) {
      header.erase(names[i]);
    } else if (type == kClientRedir && names[i] != "location") {
      return false;
    }
  }
  return true;
}

/**
 * @brief 에러 시 상태코드 500, I/O 상태 설정 및 자원 정리
 *
 */
void CgiManager::SetInternalServerError(void) {
  result_.status = 500;
  result_.is_local_redir = false;
  result_.header.clear();
  response_content_.clear();
  io_status_ = SetIoComplete(ERROR_START);
}

/**
 * @brief fd 정리 및 I/O 끝났다고 상태 설정
 *
 * @param kStatus 설정할 I/O 상태 코드 (IO_COMPLETE | ERROR_START)
 * @return int 인자로 받은 상태 코드
 */
int CgiManager::SetIoComplete(int status) {
  close(in_fd_[0]);
  close(in_fd_[1]);
  close(out_fd_[0]);
  close(out_fd_[1]);
  in_fd_[0] = -1;
  in_fd_[1] = -1;
  out_fd_[0] = -1;
  out_fd_[1] = -1;
  return status;
}
