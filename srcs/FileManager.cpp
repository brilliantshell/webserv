/**
 * @file FileManager.cpp
 * @author ghan, jiskim, yongjule
 * @brief Execute HTTP methods on static files
 * @date 2022-10-29
 *
 * @copyright Copyright (c) 2022
 */

#include "FileManager.hpp"

/**
 * @brief static File 접근 요청이 왔을 때 처리하는 FileManager 객체 생성
 *
 * @param is_keep_alive 매니저가 처리할 요청의 keep-alive 여부
 * @param response 처리 후 생성해 줄 Response 객체
 * @param router_result 처리할 파일의 정보
 * @param request 요청 정보
 */
FileManager::FileManager(bool is_keep_alive, ResponseBuffer& response,
                         Router::Result& router_result, Request& request)
    : ResponseManager(is_keep_alive, response, router_result, request),
      in_fd_(-1),
      out_fd_(-1),
      write_offset_(0),
      response_content_(response.content) {}

/**
 * @brief FileManager 소멸자, 사용한 fd 정리
 *
 */
FileManager::~FileManager(void) {
  close(in_fd_);
  close(out_fd_);
}

/**
 * @brief FileManager 객체의 상태와 HTTP method에 따라 파일을 읽고 쓰는 작업
 * 수행
 *
 * @return ResponseManager::IoFdPair <in_fd, -1> | <out_fd , -1> | <-1, -1>
 */
ResponseManager::IoFdPair FileManager::Execute(void) {
  if (router_result_.status >= 400) {
    return GetErrorPage();
  }
  if (router_result_.status == 301) {
    return GenerateRedirectPage();
  }
  if (io_status_ < ERROR_START) {
    if (request_.req.method == GET) {
      Get();
    } else if (request_.req.method == POST) {
      Post();
    } else {
      Delete();
    }
  }
  if (result_.status >= 400) {
    if (io_status_ < ERROR_START) {
      io_status_ = SetIoComplete(ERROR_START);
    }
    return GetErrorPage();
  }
  if (io_status_ == IO_COMPLETE) {
    return DetermineSuccessFileExt();
  }
  return (io_status_ == FILE_READ) ? ResponseManager::IoFdPair(in_fd_, -1)
                                   : ResponseManager::IoFdPair(-1, out_fd_);
}

// SECTION : private
/**
 * @brief GET 요청이 왔을 때 파일 타입에 따라 응답 생성
 *
 */
void FileManager::Get(void) {
  if (io_status_ == IO_START) {
    CheckFileMode();
    if (result_.status >= 400 || response_content_.empty() == false) {
      return;  // file mode error || autoindex
    }
    in_fd_ = OpenResource(router_result_.success_path.c_str(), O_RDONLY);
    if (in_fd_ == -1) {
      return;
    }
    io_status_ = FILE_READ;
  }
  if (io_status_ == FILE_READ) {
    ReadFile(in_fd_);
  }
}

/**
 * @brief POST 요청이 왔을 때 요청 내용으로 파일을 생성하고 성공 시 응답 생성,
 * 실패 시 에러코드 설정
 *
 */
void FileManager::Post(void) {
  if (io_status_ == IO_START) {
    if (FindValidOutputPath(router_result_.success_path) == false) {
      return;  // 403 or 500
    }
    out_fd_ = OpenResource(output_path_.c_str(), O_WRONLY | O_CREAT);
    if (out_fd_ == -1) {
      return;
    }
    io_status_ = FILE_WRITE;
  }
  if (io_status_ == FILE_WRITE && WriteFile(request_.content) < 0) {
    unlink(output_path_.c_str());
  }
  if (io_status_ == IO_COMPLETE) {
    result_.status = 201;  // CREATED
    result_.location.assign(output_path_, 1);
    response_content_ =
        "<!DOCTYPE html><html><title>201 Created</title><body><h1>201 "
        "Created</h1><p>YAY! The file is created at " +
        result_.location + "!</p><p>Have a nice day~</p></body></html>";
    close(out_fd_);
    out_fd_ = -1;
  }
}

/**
 * @brief POST 요청 시 파일에 내용 작성 및 I/O 상태 설정
 *
 * @param kReqContent 요청 content
 * @return ssize_t 작성한 바이트 수. 에러시 -1 반환
 */
ssize_t FileManager::WriteFile(const std::string& kReqContent) {
  size_t size = (kReqContent.size() < WRITE_BUFF_SIZE + write_offset_)
                    ? kReqContent.size() - write_offset_
                    : WRITE_BUFF_SIZE;
  ssize_t written_bytes = write(out_fd_, &kReqContent[write_offset_], size);
  if (written_bytes < 0) {
    result_.status = 500;  // INTERNAL SERVER ERROR
    return -1;
  }
  write_offset_ += written_bytes;
  io_status_ = (write_offset_ >= kReqContent.size()) ? IO_COMPLETE : FILE_WRITE;
  return written_bytes;
}

/**
 * @brief POST 요청 시 생성할 파일의 이름 설정
 *
 * @param success_path 요청에서 제공해 준 경로
 * @return true
 * @return false
 */
bool FileManager::FindValidOutputPath(std::string& success_path) {
  size_t ext_start = success_path.rfind('.');
  std::string name(success_path);
  std::string ext("");
  if (ext_start > 0 && ext_start < name.size() - 1) {
    name.assign(success_path, 0, ext_start);
    ext.assign(success_path, ext_start);
  }
  output_path_ = success_path;
  int i = 0;
  for (; i < FILE_IDX_MAX; ++i) {
    if (access(output_path_.c_str(), F_OK) == -1) {
      if (errno != ENOENT) {
        result_.status = 500;  // INTERNAL_SERVER_ERROR
        return false;
      }
      break;
    }
    std::stringstream ss;
    ss << name << "_" << i << ext;
    output_path_ = ss.str();
  }
  if (i == FILE_IDX_MAX) {
    result_.status = 403;  // FORBIDDEN
    output_path_.clear();
    return false;
  }
  return true;
}

/**
 * @brief DELETE 요청이 왔을 때 파일 삭제 및 응답 생성, 실패 시 에러코드 설정
 *
 */
void FileManager::Delete(void) {
  errno = 0;
  if (access(router_result_.success_path.c_str(), W_OK) == -1) {
    if (errno == ENOENT) {
      result_.status = 404;  // PAGE NOT FOUND
    } else if (errno == EACCES) {
      result_.status = 403;  // FORBIDDEN
    } else {
      result_.status = 500;  // INTERNAL_SERVER_ERROR
    }
    return;
  }
  if (unlink(router_result_.success_path.c_str()) == -1) {
    result_.status = 500;  // INTERNAL_SERVER_ERROR
    return;
  }
  result_.status = 200;  // OK
  response_content_ =
      "<!DOCTYPE html><html><title>Deleted</title><body><h1>200 OK</h1><p>" +
      router_result_.success_path.substr(1) + " is removed!</p></body></html>";
  io_status_ = IO_COMPLETE;
}

/**
 * @brief GET 혹은 POST 요청 시 파일 open. 실패 시 에러 코드 설정
 *
 * @param kPath open 할 파일 경로
 * @param kFlags open flag (O_RDONLY, O_WRONLY | O_CREAT)
 * @return int open된 파일 디스크립터. 실패 시 -1 반환
 */
int FileManager::OpenResource(const char* kPath, const int kFlags) {
  errno = 0;
  int fd =
      (kFlags == O_RDONLY) ? open(kPath, kFlags) : open(kPath, kFlags, 0644);
  if (fd == -1) {
    if (errno == ENOENT) {
      result_.status = 404;  // PAGE NOT FOUND
    } else if (errno == EACCES) {
      result_.status = 403;  // FORBIDDEN
    } else if (errno == EMFILE) {
      result_.status = 503;  // SERVICE UNAVAILABLE
    } else {
      result_.status = 500;  // INTERNAL SERVER ERROR}
    }
  } else {
    fcntl(fd, F_SETFL, O_NONBLOCK);
  }
  return fd;
}

/**
 * @brief Redirection 설정 및 응답 생성
 *
 * @return ResponseManager::IoFdPair <-1, -1>
 */
ResponseManager::IoFdPair FileManager::GenerateRedirectPage(void) {
  result_.location = router_result_.redirect_to;
  response_content_ =
      "<!DOCTYPE html><html><title></title><body><h1>301 Moved "
      "Permanently</h1><p>The resource has been moved permanently to <a "
      "href='" +
      result_.location + "'>" + result_.location + "<a>.</p></body></html>";
  result_.ext = "html";
  io_status_ = IO_COMPLETE;
  return ResponseManager::IoFdPair();
}

/**
 * @brief GET 요청 경로가 디렉토리인지 판별
 *
 */
void FileManager::CheckFileMode(void) {
  struct stat file_stat;
  errno = 0;
  if (stat(router_result_.success_path.c_str(), &file_stat) == -1) {
    result_.status =
        errno == ENOENT ? 404 : 500;  // NOT FOUND || INTERNAL_SERVER_ERROR
    return;
  }
  file_size_ = file_stat.st_size;
  if (S_ISDIR(file_stat.st_mode)) {
    (*(router_result_.success_path.rbegin()) == '/')
        ? result_.status = GenerateAutoindex(router_result_.success_path)
        : result_.status = 404;  // PAGE NOT FOUND
  }
}

/**
 * @brief GET 요청이 디렉토리로 시도되었을 경우 디렉토리 내 파일 리스트 생성
 *
 * @param kPath 디렉토리 경로
 * @return int 실패 시 500, 성공 시 기존 status code 리턴
 */
int FileManager::GenerateAutoindex(const std::string& kPath) {
  DIR* dir = opendir(kPath.c_str());
  if (dir == NULL) {
    return 500;  // INTERNAL_SERVER_ERROR
  }
  std::string index_of = "Index of " + kPath.substr(1);
  response_content_ = "<!DOCTYPE html><html><title>" + index_of +
                      "</title><body><h1>" + index_of + "</h1><hr><pre>\n";
  errno = 0;
  std::vector<std::string> dir_vector;
  std::vector<std::string> file_vector;
  for (dirent* ent = readdir(dir); ent != NULL; ent = readdir(dir)) {
    if (ent->d_name[0] != '.' &&
        DetermineFileType(kPath, ent, dir_vector, file_vector) == false) {
      break;
    }
  }
  closedir(dir);
  if (errno != 0) {
    return 500;  // INTERNAL_SERVER_ERROR
  }
  ListAutoindexFiles(dir_vector);
  ListAutoindexFiles(file_vector);
  response_content_ += "</pre><hr></body></html>";
  result_.is_autoindex = true;
  return result_.status;
}

/**
 * @brief autoindex 페이지에 나열할 파일 타입 판별
 *
 * @param kPath autoindex root 디렉토리 경로
 * @param kEnt 디렉토리 엔트리
 * @param dir_vector 디렉토리 경로 벡터
 * @param file_vector 파일 경로 벡터
 * @return true
 * @return false
 */
bool FileManager::DetermineFileType(const std::string& kPath,
                                    const dirent* kEnt,
                                    std::vector<std::string>& dir_vector,
                                    std::vector<std::string>& file_vector) {
  std::string file_name(kEnt->d_name);
  if (kEnt->d_type == DT_LNK) {
    struct stat s_buf;
    if (stat((kPath + file_name).c_str(), &s_buf) == -1) {
      return false;
    }
    S_ISDIR(s_buf.st_mode) ? dir_vector.push_back(file_name + "/")
                           : file_vector.push_back(file_name);
  } else {
    (kEnt->d_type == DT_DIR) ? dir_vector.push_back(file_name + "/")
                             : file_vector.push_back(file_name);
  }
  return true;
}

/**
 * @brief autoindex 경로 리스트 html <a> 태그 달아서 생성
 *
 * @param paths 디렉토리/파일 경로 벡터
 */
void FileManager::ListAutoindexFiles(std::vector<std::string>& paths) {
  std::sort(paths.begin(), paths.end());
  for (size_t i = 0; i < paths.size(); ++i) {
    std::string encoded_path(paths[i]);
    UriParser().EncodeAsciiToHex(encoded_path);
    response_content_ +=
        "<a href='./" + encoded_path + "'>" + paths[i] + "</a>\n";
  }
}

/**
 * @brief GET || POST || DELETE 후 응답 content 의 확장자 설정
 *
 * @return ResponseManager::IoFdPair <-1, -1>
 */
ResponseManager::IoFdPair FileManager::DetermineSuccessFileExt(void) {
  result_.ext = (result_.status == 201 ||
                 (request_.req.method == DELETE && result_.status == 200))
                    ? "html"
                    : ParseExtension(router_result_.success_path);
  return ResponseManager::IoFdPair();
}

/**
 * @brief fd 정리 및 I/O 끝났다고 상태 설정
 *
 * @param kStatus 설정할 I/O 상태 코드 (IO_COMPLETE | ERROR_START)
 * @return int 인자로 받은 상태 코드
 */
int FileManager::SetIoComplete(const int kStatus) {
  close(in_fd_);
  close(out_fd_);
  in_fd_ = -1;
  out_fd_ = -1;
  return kStatus;
}
