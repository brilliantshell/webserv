/**
 * @file ResponseManager.cpp
 * @author ghan, jiskim, yongjule
 * @brief Execute methods and manage resources according to the Client's request
 * @date 2022-10-11
 *
 * @copyright Copyright (c) 2022
 */

#include "ResponseManager.hpp"

ResponseManager::ResponseManager(int type, bool is_keep_alive,
                                 ResponseBuffer& response_buffer,
                                 Router::Result& router_result,
                                 Request& request)
    : is_keep_alive_(is_keep_alive),
      type_(type),
      io_status_(IO_START),
      err_fd_(-1),
      file_size_(0),
      result_(router_result.status),
      request_(request),
      router_result_(router_result),
      response_buffer_(response_buffer) {
  is_cgi_ = router_result_.is_cgi;
}

ResponseManager::~ResponseManager(void) { close(err_fd_); }

void ResponseManager::FormatHeader(void) {
  std::stringstream ss;
  ss << (request_.req.version == HttpParser::kHttp1_1 ? "HTTP/1.1 "
                                                      : "HTTP/1.0 ")
     << result_.status << " " << g_status_map[result_.status] << CRLF
     << "server: BrilliantServer/1.0" << CRLF
     << "date: " + header_formatter_.FormatCurrentTime() << CRLF
     << ((result_.status == 301 || result_.status == 400 ||
          result_.status == 404 || result_.status >= 500)
             ? ""
             : ("allow: " +
                header_formatter_.FormatAllowedMethods(router_result_.methods) +
                CRLF))
     << "connection: "
     << ((result_.status < 500 && is_keep_alive_ == true) ? "keep-alive"
                                                          : "close")
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

int ResponseManager::get_io_status(void) const { return io_status_; }

bool ResponseManager::get_is_cgi(void) const { return is_cgi_; }

bool ResponseManager::get_is_keep_alive(void) const { return is_keep_alive_; }

ResponseBuffer& ResponseManager::get_response_buffer(void) {
  return response_buffer_;
}

Request& ResponseManager::get_request(void) { return request_; }

ResponseManager::Result& ResponseManager::get_result(void) { return result_; }

// SECTION: protected
// Utils
// Parse Extension for MIME type
std::string ResponseManager::ParseExtension(const std::string& path) {
  size_t last_slash = path.rfind('/');
  if (last_slash > path.size() - 3) {
    return "";
  }
  size_t last_dot = path.rfind('.');
  if (last_dot == std::string::npos || last_dot < last_slash ||
      last_dot == path.size() - 1) {
    return "";
  }
  return path.substr(last_dot + 1);
}

// Read error page
ResponseManager::IoFdPair ResponseManager::GetErrorPage(void) {
  std::cerr << "error path : " << router_result_.error_path << '\n';
  if (access(router_result_.error_path.c_str(), F_OK) == -1) {
    // no error page
    std::stringstream ss;
    ss << result_.status << " " << g_status_map[result_.status];
    response_buffer_.content = "<!DOCTYPE html><title>" + ss.str() +
                               "</title><body><h1>" + ss.str() +
                               "</h1></body></html>";
    router_result_.error_path = "default_error.html";
    io_status_ = SetIoComplete(IO_COMPLETE);
  } else {
    struct stat file_stat;
    if (stat(router_result_.error_path.c_str(), &file_stat) == -1 ||
        (file_stat.st_mode & S_IFMT) == S_IFDIR) {
      HandleGetErrorFailure();
    }
    file_size_ = file_stat.st_size;
    if (io_status_ == ERROR_START || io_status_ == IO_START) {
      err_fd_ = open(router_result_.error_path.c_str(), O_RDONLY);
      if (err_fd_ == -1) {
        HandleGetErrorFailure();
      } else {
        fcntl(err_fd_, F_SETFL, O_NONBLOCK);
        io_status_ = ERROR_READ;
      }
    }
    if (io_status_ == ERROR_READ) {
      ReadFile(err_fd_);
    }
  }
  if (io_status_ == ERROR_READ) {
    return IoFdPair(err_fd_, -1);
  }
  result_.ext = ParseExtension(router_result_.error_path);
  return IoFdPair();
}

void ResponseManager::ReadFile(int fd) {
  char read_buf[READ_BUFFER_SIZE + 1];
  memset(read_buf, 0, READ_BUFFER_SIZE + 1);
  ssize_t read_bytes = read(fd, read_buf, READ_BUFFER_SIZE);
  if (read_bytes > 0) {
    response_buffer_.content.append(read_buf, read_bytes);
    if (read_bytes < READ_BUFFER_SIZE ||
        response_buffer_.content.size() == static_cast<size_t>(file_size_)) {
      io_status_ = SetIoComplete(IO_COMPLETE);
    } else if (io_status_ < ERROR_START) {
      io_status_ = FILE_READ;
    } else {
      io_status_ = ERROR_READ;
    }
  } else {
    if (read_bytes == -1) {
      response_buffer_.content.clear();
      result_.status = 500;  // INTERNAL SERVER ERROR
    }
    io_status_ = SetIoComplete(IO_COMPLETE);
  }
}

int ResponseManager::SetIoComplete(int status) {
  close(err_fd_);
  err_fd_ = -1;
  return status;
}

// SECTION: private
void ResponseManager::HandleGetErrorFailure(void) {
  result_.status = 500;  // INTERNAL_SERVER_ERROR
  response_buffer_.content = LAST_ERROR_DOCUMENT;
  io_status_ = SetIoComplete(IO_COMPLETE);
}
