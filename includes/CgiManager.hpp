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

#include <libgen.h>
#include <unistd.h>

#include <csignal>
#include <string>

#include "ResponseManager.hpp"
#include "Router.hpp"
#include "Types.hpp"
#include "UriParser.hpp"

#define FIELD_LINE_MAX 8192    // 8KB
#define HEADER_MAX 16384       // 16KB
#define CONTENT_MAX 134217728  // 128MB

#define PIPE_BUF_SIZE 4096

class CgiManager : public ResponseManager {
 public:
  CgiManager(bool is_keep_alive, ResponseBuffer& response,
             Router::Result& router_result, const Request& request);
  virtual ~CgiManager(void);

  ResponseManager::IoFdPair Execute(bool is_eof = false);

 private:
  enum ResponseType {
    kError = 0,
    kDocument,
    kLocalRedir,
    kClientRedir,
    kClientRedirDoc,
  };

  bool is_header;
  pid_t pid_;
  int in_fd_[2];
  int out_fd_[2];
  size_t write_offset_;
  std::string header_read_buf_;
  std::string& response_content_;

  // child
  void DupFds(void);
  void ParseScriptCommandLine(std::vector<std::string>& arg_vector,
                              std::string query);
  void ExecuteScript(const char* success_path, char* const* env);

  // parent
  void SetIpc(void);
  bool OpenPipes(void);
  bool CheckFileMode(const char* path);
  void PassContent(void);
  bool ReceiveCgiHeaderFields(ResponseHeaderMap& header, size_t header_end);
  bool ReceiveCgiResponse(ResponseHeaderMap& header);
  bool ParseCgiHeader(ResponseHeaderMap& header);
  int DetermineResponseType(ResponseHeaderMap& header);
  void SetInternalServerError(void);
  int SetIoComplete(int status);
};

#endif  // INCLUDES_CGIMANAGER_HPP
