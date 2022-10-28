/**
 * @file CgiManager.cpp
 * @author ghan, jiskim, yongjule
 * @brief Execute cgi script and handle IPC between the server and the script
 * @date 2022-10-14
 *
 * @copyright Copyright (c) 2022
 */

#include "CgiManager.hpp"

CgiManager::CgiManager(void) {}

CgiManager::~CgiManager(void) {
  close(out_fd_[0]);
  close(out_fd_[1]);
  close(in_fd_[0]);
  close(in_fd_[1]);
}

CgiManager::Result CgiManager::Execute(std::string& response_content,
                                       Router::Result& router_result,
                                       ResponseHeaderMap& header,
                                       const std::string& request_content,
                                       int status) {
  Result result(status);
  if (CheckFileMode(result, router_result.success_path.c_str()) == false) {
    return result;
  }
  if (OpenPipes(result) == false) {
    result.status = 500;  // INTERNAL SERVER ERROR
    return result;
  }
  pid_t pid = fork();
  if (pid == 0) {
    ExecuteScript(router_result.success_path.c_str(),
                  const_cast<char* const*>(router_result.cgi_env.get_env()));
  } else if (pid > 0) {
    PassContent(result, request_content);
    if (result.status == 500) {
      kill(pid, SIGTERM);
    } else if (ReceiveCgiResponse(response_content, result, header) == false ||
               ParseCgiHeader(response_content, result, header) == false) {
      result.status = 500;
      result.is_local_redir = false;
      response_content.clear();
      header.clear();
    }
  } else {
    result.status = 500;  // INTERNAL SERVER ERROR
  }
  return result;
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
bool CgiManager::OpenPipes(Result& result) {
  if (pipe(out_fd_) == -1) {
    return false;
  }
  if (pipe(in_fd_) == -1) {
    close(out_fd_[0]);
    close(out_fd_[1]);
    return false;
  }
  return true;
}

bool CgiManager::CheckFileMode(Result& result, const char* path) {
  if (access(path, X_OK) == -1) {
    if (errno == ENOENT) {
      result.status = 404;  // PAGE NOT FOUND
    } else if (errno == EACCES) {
      result.status = 403;  // FORBIDDEN
    } else {
      result.status = 500;  // INTERNAL_SERVER_ERROR
    }
    return false;
  }
  return true;
}

void CgiManager::PassContent(Result& result, const std::string& content) {
  size_t offset = 0;
  for (ssize_t sent_bytes = 0; offset < content.size(); offset += sent_bytes) {
    size_t bytes = (content.size() < offset + PIPE_BUF_SIZE)
                       ? content.size() - offset
                       : PIPE_BUF_SIZE;
    sent_bytes = write(in_fd_[1], &content[offset], bytes);
    if (sent_bytes == -1) {
      result.status = 500;  // INTERNAL SERVER ERROR
      break;
    }
  }
  close(out_fd_[1]);
  close(in_fd_[0]);
  close(in_fd_[1]);
}

bool CgiManager::ReceiveCgiHeaderFields(ResponseHeaderMap& header,
                                        const std::string& header_buf) {
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

bool CgiManager::ReceiveCgiResponse(std::string& response_content,
                                    Result& result, ResponseHeaderMap& header) {
  bool is_header = true;
  ssize_t read_size;
  char buf[PIPE_BUF_SIZE + 1];
  std::string header_buf;

  memset(buf, 0, PIPE_BUF_SIZE + 1);
  while ((read_size = read(out_fd_[0], buf, PIPE_BUF_SIZE)) > 0) {
    if (is_header == true) {
      header_buf += buf;
      if (header_buf.size() > HEADER_MAX) {
        close(out_fd_[0]);
        return false;
      }
      size_t header_end = header_buf.find(CRLF CRLF);
      if (header_end != std::string::npos) {
        if (ReceiveCgiHeaderFields(
                header, header_buf.substr(0, header_end + 1)) == false) {
          close(out_fd_[0]);
          return false;
        }
        is_header = false;
        response_content.append(header_buf, header_end + 4);
      }
    } else {
      response_content.append(buf, read_size);
      if (response_content.size() > CONTENT_MAX) {
        close(out_fd_[0]);
        return false;
      }
    }
    memset(buf, 0, PIPE_BUF_SIZE + 1);
  }
  close(out_fd_[0]);
  return read_size >= 0;
}

int CgiManager::DetermineResponseType(const std::string& content,
                                      ResponseHeaderMap& header) {
  if (header.size() == 0) {
    return kError;
  }
  // redirect (local, client, client with body)
  if (header.count("location") == 1) {
    std::string& location = header["location"];
    if (location[0] == '/') {
      if (header.size() > 1 || content.size() > 0) {
        return kError;
      }
      return kLocalRedir;
    } else {
      UriParser::Result uri_result = UriParser().ParseTarget(location);
      if (uri_result.is_valid == false) {
        return kError;
      }
      if (content.size() > 0) {
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

bool CgiManager::ParseCgiHeader(std::string& response_content, Result& result,
                                ResponseHeaderMap& header) {
  int response_type = DetermineResponseType(response_content, header);
  if (response_type == kError) {
    return false;
  }
  if (header.count("status") == 1) {
    std::stringstream ss(header["status"]);
    ss >> result.status;
    if (result.status < 100 || result.status > 999) {
      return false;
    }
  }
  if (response_type == kLocalRedir) {
    result.is_local_redir = true;
  }
  if (response_type == kClientRedir || response_type == kClientRedirDoc) {
    result.status = 302;
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
