/**
 * @file CgiManager.cpp
 * @author ghan, jiskim, yongjule
 * @brief Execute cgi script and handle IPC between the server and the script
 * @date 2022-10-14
 *
 * @copyright Copyright (c) 2022
 */

#include "CgiManager.hpp"

CgiManager::CgiManager(bool is_keep_alive, ResponseBuffer& response,
                       Router::Result& router_result, Request& request)
    : ResponseManager(ResponseManager::kStatic, is_keep_alive, response,
                      router_result, request),
      is_header(true),
      pid_(-1),
      write_offset_(0),
      response_content_(response.content) {}

CgiManager::~CgiManager(void) {
  close(out_fd_[0]);
  close(out_fd_[1]);
  close(in_fd_[0]);
  close(in_fd_[1]);
}

// Resource Manager Member : response_content, router_result, request
ResponseManager::IoFdPair CgiManager::Execute(bool is_eof) {
  std::cerr << "CGIMANAGER START\n";
  if (is_eof == true) {
    io_status_ = SetIoComplete(IO_COMPLETE);
  }
  switch (io_status_) {
    case IO_START:
      SetIpc();
      break;
    case PIPE_WRITE:
      PassContent();
      break;
    case PIPE_READ:
      if (ReceiveCgiResponse(result_.header) == false) {
        std::cerr << "CGI RECEIVE ERROR\n";
        SetInternalServerError();
      }
      break;
    case IO_COMPLETE:
      std::cerr << "CGI COMPLETE\n";
      if (ParseCgiHeader(result_.header) == false) {
        std::cerr << "CGI PARSE ERROR\n";
        SetInternalServerError();
      }
      break;
    default:
      break;
  }
  if (result_.status >= 400) {
    std::cerr << "CGI ERROR\n";
    return GetErrorPage();
  }
  if (io_status_ == PIPE_WRITE) {
    std::cerr << "CGI PIPE WRITE\n";
    return ResponseManager::IoFdPair(-1, in_fd_[1]);
  } else if (io_status_ == PIPE_READ) {
    std::cerr << "CGI PIPE READ, fd : " << out_fd_[0] << "\n";
    return ResponseManager::IoFdPair(out_fd_[0], -1);
  }
  result_.ext = ParseExtension(router_result_.success_path);
  return ResponseManager::IoFdPair(-1, -1);
}

// SECTION: private
// child
void CgiManager::DupFds(void) {
  if (dup2(out_fd_[1], STDOUT_FILENO) == -1) {
    exit(EXIT_FAILURE);
  }
  close(out_fd_[1]);
  close(out_fd_[0]);
  if (dup2(in_fd_[0], STDIN_FILENO) == -1) {
    exit(EXIT_FAILURE);
  }
  close(in_fd_[0]);
  close(in_fd_[1]);
}

void CgiManager::ParseScriptCommandLine(std::vector<std::string>& arg_vector,
                                        std::string query) {
  if (query[query.size() - 1] == '+') {
    return;
  }
  query.erase(0, 13);  // QUERY_STRING=
  if (!query.empty() && query.find("=") == std::string::npos) {
    size_t start = 1;
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

void CgiManager::ExecuteScript(const char* success_path, char* const* env) {
  // 1. ENAMETOOLONG, 2.ENOMEM(dirname)
  if (env == NULL) {
    exit(EXIT_FAILURE);
  }
  char* new_cwd = dirname(const_cast<char*>(success_path));
  if (new_cwd == NULL || chdir(new_cwd) == -1) {
    exit(EXIT_FAILURE);
  }
  char* script_path = basename(const_cast<char*>(success_path));
  if (script_path == NULL) {
    exit(EXIT_FAILURE);
  }
  DupFds();
  std::vector<std::string> arg_vector;
  ParseScriptCommandLine(arg_vector, env[6]);
  const char** argv = new (std::nothrow) const char*[arg_vector.size() + 2];
  if (argv == NULL) {
    exit(EXIT_FAILURE);
  }
  memset(argv, 0, sizeof(char*) * (arg_vector.size() + 2));
  argv[0] = script_path;
  for (size_t i = 0; i < arg_vector.size(); ++i) {
    argv[i + 1] = arg_vector[i].c_str();
  }
  alarm(5);  // CGI script timeout
  execve(script_path, const_cast<char* const*>(argv), env);
  std::cerr << "execve error" << std::endl;
  exit(EXIT_FAILURE);
}

// parent
void CgiManager::SetIpc(void) {
  if (CheckFileMode(router_result_.success_path.c_str()) == false) {
    io_status_ = SetIoComplete(ERROR_START);
  }
  if (OpenPipes() == false) {
    result_.status = 500;
    io_status_ = SetIoComplete(ERROR_START);
    return;
  }
  pid_ = fork();
  if (pid_ == 0) {
    ExecuteScript(router_result_.success_path.c_str(),
                  const_cast<char* const*>(router_result_.cgi_env.get_env()));
  } else if (pid_ > 0) {
    io_status_ = PIPE_WRITE;
  } else {
    result_.status = 500;
    io_status_ = SetIoComplete(ERROR_START);
  }
}

bool CgiManager::OpenPipes(void) {
  if (pipe(out_fd_) == -1) {
    return false;
  }
  if (pipe(in_fd_) == -1) {
    close(out_fd_[0]);
    close(out_fd_[1]);
    return false;
  }
  fcntl(out_fd_[0], F_SETFL, O_NONBLOCK);
  fcntl(in_fd_[1], F_SETFL, O_NONBLOCK);
  return true;
}

bool CgiManager::CheckFileMode(const char* path) {
  if (access(path, X_OK) == -1) {
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

void CgiManager::PassContent(void) {
  ssize_t sent_bytes =
      write(in_fd_[1], &request_.content[write_offset_],
            (request_.content.size() < write_offset_ + PIPE_BUF_SIZE)
                ? request_.content.size() - write_offset_
                : PIPE_BUF_SIZE);
  if (sent_bytes < 0) {
    result_.status = 500;  // INTERNAL SERVER ERROR
    kill(pid_, SIGTERM);
    io_status_ = SetIoComplete(ERROR_START);
  }
  write_offset_ += sent_bytes;
  io_status_ =
      (write_offset_ >= request_.content.size()) ? PIPE_READ : PIPE_WRITE;
  if (io_status_ == PIPE_READ) {
    close(out_fd_[1]);
    close(in_fd_[0]);
    close(in_fd_[1]);
  }
}

bool CgiManager::ReceiveCgiHeaderFields(ResponseHeaderMap& header,
                                        size_t header_end) {
  std::string header_buf(header_read_buf_, 0, header_end);
  size_t start = 0;
  for (size_t end_of_line = header_buf.find(CRLF);
       end_of_line < header_buf.size() && end_of_line != std::string::npos;
       end_of_line = header_buf.find(CRLF, start)) {
    if (end_of_line - start > FIELD_LINE_MAX) {
      return false;
    }
    std::string line(header_buf, start, end_of_line - start);
    size_t colon_pos = line.find(":");
    if (colon_pos == std::string::npos) {
      return false;
    }
    if (colon_pos + 1 < line.size()) {
      size_t value_start = line.find_first_not_of(" \t", colon_pos + 1);
      std::string name(line, 0, colon_pos);
      std::transform(name.begin(), name.begin() + colon_pos, name.begin(),
                     ::tolower);
      if (header.insert(std::make_pair(name, line.substr(value_start)))
              .second == false) {
        return false;
      }
    }
    start = end_of_line + 2;
  }
  return true;
}

bool CgiManager::ReceiveCgiResponse(ResponseHeaderMap& header) {
  char buf[PIPE_BUF_SIZE + 1];
  memset(buf, 0, PIPE_BUF_SIZE + 1);

  static int i = 0;
  ssize_t read_size = read(out_fd_[0], buf, PIPE_BUF_SIZE);
  std::cerr << ">>>>[ " << i << " ]<<<< cgi read buf : " << buf << "\n\n";
  if (read_size < 0) {
    return false;
  }
  // if (read_size < PIPE_BUF_SIZE) {
  //   io_status_ = SetIoComplete(IO_COMPLETE);
  // }
  if (is_header == true) {
    header_read_buf_.append(buf, read_size);
    if (header_read_buf_.size() > HEADER_MAX) {
      close(out_fd_[0]);
      return false;
    }
    size_t header_end = header_read_buf_.find(CRLF CRLF);
    if (header_end != std::string::npos) {
      if (ReceiveCgiHeaderFields(header, header_end + 2) == false) {
        std::cerr << "ReceiveCgiHeaderFields error" << std::endl;
        close(out_fd_[0]);
        return false;
      }
      is_header = false;
      response_content_.append(header_read_buf_, header_end + 4);
    }
  } else {
    response_content_.append(buf, read_size);
    if (response_content_.size() > CONTENT_MAX) {
      close(out_fd_[0]);
      return false;
    }
  }

  ++i;
  return true;
}

int CgiManager::DetermineResponseType(ResponseHeaderMap& header) {
  if (header.size() == 0) {
    return kError;
  }
  // redirect (local, client, client with body)
  if (header.count("location") == 1) {
    std::string& location = header["location"];
    if (location[0] == '/') {
      if (header.size() > 1 || response_content_.size() > 0) {
        return kError;
      }
      return kLocalRedir;
    } else {
      UriParser::Result uri_result = UriParser().ParseTarget(location);
      if (uri_result.is_valid == false) {
        return kError;
      }
      if (response_content_.size() > 0) {
        if (header.count("content-type") != 1 || header.count("status") != 1) {
          return kError;
        }
        return kClientRedirDoc;
      }
      if (header.count("content-type") == 1 || header.count("status") == 1) {
        return kError;
      }
      return kClientRedir;
    }
  } else if (header.count("content-type")) {  // document
    return kDocument;
  }
  return kError;
}

bool CgiManager::ParseCgiHeader(ResponseHeaderMap& header) {
  int response_type = DetermineResponseType(header);
  if (response_type == kError) {
    return false;
  }
  if (header.count("status") == 1) {
    std::stringstream ss(header["status"]);
    ss >> result_.status;
    if (result_.status < 100 || result_.status > 999) {
      return false;
    }
  }
  if (response_type == kLocalRedir) {
    result_.is_local_redir = true;
  }
  if (response_type == kClientRedir || response_type == kClientRedirDoc) {
    result_.status = 302;
  }
  std::vector<std::string> field_names;
  for (ResponseHeaderMap::const_iterator it = header.begin();
       it != header.end(); ++it) {
    field_names.push_back(it->first);
  }
  for (size_t i = 0; i < field_names.size(); ++i) {
    // extension header fields
    if (field_names[i].size() > 6 &&
        field_names[i].compare(0, 6, "x-cgi-") == 0) {
      header.erase(field_names[i]);
    } else if (response_type == kClientRedir && field_names[i] != "location") {
      return false;
    }
  }
  header.erase("status");
  return true;
}

void CgiManager::SetInternalServerError(void) {
  result_.status = 500;
  result_.is_local_redir = false;
  result_.header.clear();
  response_content_.clear();
  io_status_ = SetIoComplete(ERROR_START);
}

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
