/**
 * @file CgiManager.hpp
 * @author ghan, jiskim, yongjule
 * @brief Execute cgi script and handle IPC between the server and the script
 * @date 2022-10-14
 *
 * @copyright Copyright (c) 2022
 */

#include "CgiManager.hpp"

#include <unistd.h>

CgiManager::CgiManager(void) {}

CgiManager::~CgiManager(void) {}

CgiManager::Result CgiManager::Execute(Router::Result& router_result,
                                       std::vector<std::string>& header,
                                       const std::string& request_content,
                                       int status) {
  Result result(status);
  int in_fd[2];
  int out_fd[2];

  if (OpenPipes(result, in_fd, out_fd) == false) {
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
    // int exit_status;
    // waitpid(pid, &exit_status, 0);
    // std::cerr << "exit status: " << exit_status << std::endl;
    // if (WIFSIGNALED(exit_status) ||
    //     (WIFEXITED(exit_status) && WEXITSTATUS(exit_status) > 0)) {
    //   result.status = 500;
    //   return result;
    // }
    ReceiveCgiResponse(result, header, out_fd[0]);
    if (result.status < 400) {
      ParseCgiHeader(result, header);
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

#include <libgen.h>
void CgiManager::ExecuteScript(int in_fd[], int out_fd[],
                               const char* success_path, char* const* env) {
  try {
    DupFds(in_fd, out_fd);
    std::vector<std::string> arg_vector;
    ParseScriptCommandLine(arg_vector, env[6]);
    const char** argv = new const char*[arg_vector.size() + 2];
    memset(argv, 0, sizeof(char*) * (arg_vector.size() + 2));
    argv[0] = success_path;
    for (size_t i = 0; i < arg_vector.size(); ++i) {
      argv[i + 1] = arg_vector[i].c_str();
    }
    // 1. ENAMETOOLONG, 2.ENOMEMop
    // if (chdir(dirname(success_path)) == -1) {
    //  exit(EXIT_FAILURE);
    //}
    execve(success_path, const_cast<char* const*>(argv), env);
    std::cerr << "execve error : " << strerror(errno) << std::endl;
    exit(EXIT_FAILURE);
  } catch (const std::exception& e) {
    exit(EXIT_FAILURE);
  }
}

// parent
bool CgiManager::OpenPipes(Result& result, int in[2], int out[2]) {
  if (pipe(out) == -1) {
    result.status = 500;  // INTERNAL SERVER ERROR
    return false;
  }
  if (pipe(in) == -1) {
    close(out[0]);
    close(out[1]);
    result.status = 500;  // INTERNAL SERVER ERROR
    return false;
  }
  return true;
}

void CgiManager::PassRequestContent(Result& result,
                                    const std::string& request_content,
                                    int in_fd[2], int out_fd[2]) {
  // write 실패?
  if (write(in_fd[1], request_content.c_str(), request_content.size()) < 0) {
    result.status = 500;  // INTERNAL SERVER ERROR
  }
  close(out_fd[1]);
  close(in_fd[0]);
  close(in_fd[1]);
}

void CgiManager::ReceiveCgiHeaderFields(Result& result,
                                        std::vector<std::string>& header,
                                        const std::string& header_buf) {
  size_t start = 0;
  for (size_t end_of_line = header_buf.find("\n");
       end_of_line < header_buf.size() && end_of_line != std::string::npos;
       end_of_line = header_buf.find("\n", start)) {
    header.push_back(header_buf.substr(start, end_of_line - start));
    if (header[header.size() - 1].size() > FIELD_LINE_MAX) {
      result.status = 500;  // INTERNAL SERVER ERROR
      header.clear();
      break;
    }
    start = end_of_line + 1;
  }
}

void CgiManager::ReceiveCgiResponse(Result& result,
                                    std::vector<std::string>& header,
                                    int from_cgi_fd) {
  bool is_header = true;
  ssize_t read_size;
  char buf[2049];
  std::string header_buf;

  memset(buf, 0, 2049);
  while ((read_size = read(from_cgi_fd, buf, 2048)) != 0) {
    if (is_header == true) {
      header_buf += buf;
      if (header_buf.size() > HEADER_MAX) {
        result.status = 500;  // INTERNAL SERVER ERROR
        header.clear();
        break;
      }
      size_t header_end = header_buf.find("\n\n");
      if (header_end != std::string::npos) {
        ReceiveCgiHeaderFields(result, header,
                               header_buf.substr(0, header_end + 1));
        if (result.status >= 400) {
          break;
        }
        is_header = false;
        result.content += header_buf.substr(header_end + 2);
      }
    } else {
      result.content.append(buf, read_size);
      if (result.content.size() > CONTENT_MAX) {
        result.status = 500;  // INTERNAL SERVER ERROR
        result.content.clear();
        break;
      }
    }
    memset(buf, 0, 2049);
  }
  close(from_cgi_fd);
}

void CgiManager::ParseCgiHeader(Result& result,
                                std::vector<std::string>& header) {
  for (size_t i = 0; i < header.size();) {
    size_t colon_pos = header[i].find(":");
    if (colon_pos == std::string::npos) {
      result.status = 500;  // INTERNAL SERVER ERROR
      return;
    }
    std::transform(header[i].begin(), header[i].begin() + colon_pos,
                   header[i].begin(), ::tolower);
    if (header[i].size() > 7 && header[i].compare(0, 7, "status:") == 0) {
      std::stringstream ss(header[i].substr(8));
      ss >> result.status;
      header.erase(header.begin() + i);
    } else if (header[i].size() > 6 && header[i].compare(0, 6, "x-cgi-") == 0) {
      header.erase(header.begin() + i);
    } else {
      ++i;
    }
  }
}
