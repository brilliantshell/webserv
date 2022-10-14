/**
 * @file CgiManager.hpp
 * @author ghan, jiskim, yongjule
 * @brief Execute cgi script and handle IPC between the server and the script
 * @date 2022-10-14
 *
 * @copyright Copyright (c) 2022
 */

#ifndef INCLUDES_CGIMANAGER_HPP_
#define INCLUDES_CGIMANAGER_HPP_

#include <csignal>
#include <string>

#include "Router.hpp"

class CgiManager {
 public:
  struct Result {
    int status;
    std::string content;

    Result(int status) : status(status) {}
  };

  CgiManager(void);
  ~CgiManager(void);

  Result Execute(Router::Result& router_result,
                 std::vector<std::string>& header,
                 const std::string& request_content, int status);

 private:
  int out_fd_[2];
  int in_fd_[2];

  // child
  void DupFds(int in[2], int out[2]);

  // parent
  bool OpenPipes(Result& result, int in[2], int out[2]);

  void PassRequestContent(Result& result, const std::string& request_content,
                          int in_fd[2], int out_fd[2]);

  void ReceiveCgiResponse(std::string& result_content,
                          std::vector<std::string>& header, int from_cgi_fd);
};

#endif  // INCLUDES_CGIMANAGER_HPP
