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
#include "UriParser.hpp"

#define FIELD_LINE_MAX 8192    // 8KB
#define HEADER_MAX 16384       // 16KB
#define CONTENT_MAX 134217728  // 128MB

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
  // child
  void DupFds(int in[2], int out[2]);
  void ParseScriptCommandLine(std::vector<std::string>& arg_vector,
                              std::string query);
  void ExecuteScript(int in_fd[], int out_fd[], const char* success_path,
                     char* const* env);

  // parent
  bool OpenPipes(Result& result, int in[2], int out[2]);
  void PassRequestContent(Result& result, const std::string& request_content,
                          int in_fd[2], int out_fd[2]);
  void ReceiveCgiHeaderFields(Result& result, std::vector<std::string>& header,
                              const std::string& header_buf);
  void ReceiveCgiResponse(Result& result, std::vector<std::string>& header,
                          int from_cgi_fd);
  void ParseCgiHeader(Result& result, std::vector<std::string>& header);
};

#endif  // INCLUDES_CGIMANAGER_HPP
