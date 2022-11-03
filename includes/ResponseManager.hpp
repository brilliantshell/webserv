/**
 * @file ResponseManager.hpp
 * @author ghan, jiskim, yongjule
 * @brief Execute methods and manage resources according to the Client's request
 * @date 2022-10-11
 *
 * @copyright Copyright (c) 2022
 */

#ifndef INCLUDES_RESPONSEMANAGER_HPP_
#define INCLUDES_RESPONSEMANAGER_HPP_

#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include <fstream>
#include <sstream>

#include "HeaderFormatter.hpp"
#include "ResponseData.hpp"
#include "Router.hpp"
#include "Types.hpp"
#include "UriParser.hpp"

#define LAST_ERROR_DOCUMENT                                               \
  "<!DOCTYPE html><title>500 Internal Server Error</title><body><h1>500 " \
  "Internal Server Error</ h1></ body></ html> "

#define FILE_READ 0
#define PIPE_READ 1
#define FILE_WRITE 2
#define PIPE_WRITE 3
#define IO_START 4
#define IO_COMPLETE 5
#define ERROR_START 6
#define ERROR_READ 7

#define READ_BUFFER_SIZE 4096
#define WRITE_BUFFER_SIZE 4096

// Abstract Class For FileManager, CgiManager
class ResponseManager {
 public:
  struct IoFdPair {
    int input;
    int output;

    IoFdPair(void) : input(-1), output(-1) {}
    IoFdPair(int in, int out) : input(in), output(out) {}
  };

  struct Result {
    bool is_autoindex;
    bool is_local_redir;
    int status;
    std::string location;  // success or error path
    std::string ext;
    ResponseHeaderMap header;

    Result(int router_result_status)
        : is_autoindex(false),
          is_local_redir(false),
          status(router_result_status),
          location("") {}
  };

  ResponseManager(int type, bool is_keep_alive, ResponseBuffer& response_buffer,
                  Router::Result& router_result, Request& request);
  virtual ~ResponseManager(void);

  void FormatHeader(void);
  virtual IoFdPair Execute(void) = 0;

  int get_io_status(void) const;
  bool get_is_cgi(void) const;
  bool get_is_keep_alive(void) const;
  ResponseBuffer& get_response_buffer(void);
  Request& get_request(void);
  Result& get_result(void);

 protected:
  enum { kStatic = 0, kCgi };

  bool is_keep_alive_;
  int type_;
  int io_status_;
  int err_fd_;
  off_t file_size_;
  Result result_;
  Request request_;
  Router::Result router_result_;
  ResponseBuffer& response_buffer_;
  HeaderFormatter header_formatter_;

  std::string ParseExtension(const std::string& success_path);
  IoFdPair GetErrorPage(void);
  void ReadFile(int fd);
  virtual int SetIoComplete(int status);

 private:
  bool is_cgi_;
  void HandleGetErrorFailure(void);
};

#endif  // INCLUDES_RESPONSEMANAGER_HPP_
