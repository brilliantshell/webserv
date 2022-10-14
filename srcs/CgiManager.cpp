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

#include <iostream>

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
    // child process
    DupFds(in_fd, out_fd);
    char* argv[3] = {strdup(router_result.success_path.c_str()),
                     strdup("text/plain"), NULL};
    execve(router_result.success_path.c_str(), argv,
           const_cast<char* const*>(router_result.cgi_env.get_env()));
    free(argv[0]);
    free(argv[1]);
    exit(EXIT_FAILURE);
  } else {
    // parent process
    PassRequestContent(result, request_content, in_fd, out_fd);
    if (result.status == 500) {
      kill(pid, SIGTERM);
      return result;
    }
    int exit_status;
    waitpid(pid, &exit_status, 0);
    if (WIFSIGNALED(exit_status) ||
        (WIFEXITED(exit_status) && WEXITSTATUS(exit_status) > 0)) {
      result.status = 500;
      return result;
    }
    ReceiveCgiResponse(result.content, header, out_fd[0]);
  }
  return result;
}

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

void CgiManager::ReceiveCgiResponse(std::string& result_content,
                                    std::vector<std::string>& header,
                                    int from_cgi_fd) {
  bool is_header = true;
  ssize_t read_size;
  char buf[1024];
  std::string buf_str;

  memset(buf, 0, 1024);
  while ((read_size = read(from_cgi_fd, buf, 1024)) > 0) {
    if (is_header == true) {
      buf_str += buf;
      size_t header_end = buf_str.find("\n\n");
      if (header_end != std::string::npos) {
        size_t header_start = 0;
        size_t end_of_line = buf_str.find("\n");
        for (; end_of_line != std::string::npos;
             end_of_line = buf_str.find("\n", header_start)) {
          header.push_back(buf_str.substr(header_start, end_of_line));
          header_start = end_of_line + 1;
        }
        is_header = false;
        result_content += buf_str.substr(header_end + 2);
      }
    } else {
      result_content.append(buf, read_size);
    }
    memset(buf, 0, 1024);
  }
  close(from_cgi_fd);
}
