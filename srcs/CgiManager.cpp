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

CgiManager::~CgiManager(void) {}

CgiManager::Result CgiManager::Execute(Router::Result& router_result,
                                       ResponseHeaderMap& header,
                                       const std::string& request_content,
                                       int status) {
  Result result(status);
  int in_fd[2];
  int out_fd[2];

  if (OpenPipes(result, in_fd, out_fd) == false) {
    result.status = 500;  // INTERNAL SERVER ERROR
    return result;
  }
  pid_t pid = fork();
  if (pid == -1) {
    result.status = 500;  // INTERNAL SERVER ERROR
    return result;
  }
  if (pid == 0) {
    ExecuteScript(in_fd, out_fd, router_result.success_path.c_str(),
                  const_cast<char* const*>(router_result.cgi_env.get_env()));
  } else {
    PassRequestContent(result, request_content, in_fd, out_fd);
    if (result.status == 500) {
      kill(pid, SIGTERM);
      return result;
    }
    // TODO : kqueue 로 자식 프로세스 상태 확인
    if (ReceiveCgiResponse(result, header, out_fd[0]) == false ||
        ParseCgiHeader(result, header) == false) {
      result.status = 500;
      result.is_local_redir = false;
      result.content.clear();
      header.clear();
    }
  }
  return result;
}

// SECTION: private
// child
void CgiManager::DupFds(int in[2], int out[2]) {
  if (dup2(out[1], STDOUT_FILENO) == -1) {
    exit(EXIT_FAILURE);
  }
  close(out[1]);
  close(out[0]);

  if (dup2(in[0], STDIN_FILENO) == -1) {
    exit(EXIT_FAILURE);
  }
  close(in[0]);
  close(in[1]);
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

void CgiManager::ExecuteScript(int in_fd[], int out_fd[],
                               const char* success_path, char* const* env) {
  try {
    // 1. ENAMETOOLONG, 2.ENOMEM(dirname)
    char* new_cwd = dirname(const_cast<char*>(success_path));
    if (new_cwd == NULL || chdir(new_cwd) == -1) {
      exit(EXIT_FAILURE);
    }
    char* script_path = basename(const_cast<char*>(success_path));
    if (script_path == NULL) {
      exit(EXIT_FAILURE);
    }
    DupFds(in_fd, out_fd);
    std::vector<std::string> arg_vector;
    ParseScriptCommandLine(arg_vector, env[6]);
    const char** argv = new const char*[arg_vector.size() + 2];
    memset(argv, 0, sizeof(char*) * (arg_vector.size() + 2));
    argv[0] = script_path;
    for (size_t i = 0; i < arg_vector.size(); ++i) {
      argv[i + 1] = arg_vector[i].c_str();
    }
    execve(script_path, const_cast<char* const*>(argv), env);
    // TODO : error handling(404, 403, 500)
    exit(EXIT_FAILURE);
  } catch (const std::exception& e) {
    exit(EXIT_FAILURE);
  }
}

// parent
bool CgiManager::OpenPipes(Result& result, int in[2], int out[2]) {
  if (pipe(out) == -1) {
    return false;
  }
  if (pipe(in) == -1) {
    close(out[0]);
    close(out[1]);
    return false;
  }
  return true;
}

void CgiManager::PassRequestContent(Result& result,
                                    const std::string& request_content,
                                    int in_fd[2], int out_fd[2]) {
  // write 실패?
  // TODO : non-blocking
  if (write(in_fd[1], request_content.c_str(), request_content.size()) < 0) {
    result.status = 500;  // INTERNAL SERVER ERROR
  }
  close(out_fd[1]);
  close(in_fd[0]);
  close(in_fd[1]);
}

bool CgiManager::ReceiveCgiHeaderFields(ResponseHeaderMap& header,
                                        const std::string& header_buf) {
  size_t start = 0;
  for (size_t end_of_line = header_buf.find("\n");
       end_of_line < header_buf.size() && end_of_line != std::string::npos;
       end_of_line = header_buf.find("\n", start)) {
    if (end_of_line - start > FIELD_LINE_MAX) {
      return false;
    }
    std::string field_line = header_buf.substr(start, end_of_line - start);
    size_t colon_pos = field_line.find(":");
    if (colon_pos == std::string::npos) {
      return false;
    }
    if (colon_pos + 1 < field_line.size()) {
      size_t value_start = field_line.find_first_not_of(" \t", colon_pos + 1);
      std::string field_name = field_line.substr(0, colon_pos);
      std::transform(field_name.begin(), field_name.begin() + colon_pos,
                     field_name.begin(), ::tolower);
      if (header
              .insert(
                  std::make_pair(field_name, field_line.substr(value_start)))
              .second == false) {
        return false;
      }
    }
    start = end_of_line + 1;
  }
  return true;
}

bool CgiManager::ReceiveCgiResponse(Result& result, ResponseHeaderMap& header,
                                    int from_cgi_fd) {
  bool is_header = true;
  ssize_t read_size;
  char buf[2049];
  std::string header_buf;

  memset(buf, 0, 2049);
  while ((read_size = read(from_cgi_fd, buf, 2048)) > 0) {
    if (is_header == true) {
      header_buf += buf;
      if (header_buf.size() > HEADER_MAX) {
        close(from_cgi_fd);
        return false;
      }
      size_t header_end = header_buf.find("\n\n");
      if (header_end != std::string::npos) {
        if (ReceiveCgiHeaderFields(
                header, header_buf.substr(0, header_end + 1)) == false) {
          close(from_cgi_fd);
          return false;
        }
        is_header = false;
        result.content += header_buf.substr(header_end + 2);
      }
    } else {
      result.content.append(buf, read_size);
      if (result.content.size() > CONTENT_MAX) {
        close(from_cgi_fd);
        return false;
      }
    }
    memset(buf, 0, 2049);
  }
  close(from_cgi_fd);
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

bool CgiManager::ParseCgiHeader(Result& result, ResponseHeaderMap& header) {
  int response_type = DetermineResponseType(result.content, header);
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
