/**
 * @file Connection.hpp
 * @author ghan, jiskim, yongjule
 * @brief Manage connection between client and server
 * @date 2022-09-27
 *
 * @copyright Copyright (c) 2022
 */

#ifndef INCLUDES_CONNECTION_HPP_
#define INCLUDES_CONNECTION_HPP_

#include <fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

#include <cstring>
#include <iostream>
#include <queue>
#include <string>

#include "CgiManager.hpp"
#include "FileManager.hpp"
#include "HeaderFormatter.hpp"
#include "HttpParser.hpp"
#include "PathResolver.hpp"
#include "Router.hpp"

// connection status
#define KEEP_ALIVE 0
#define CLOSE 1
#define CONNECTION_ERROR 2
#define KEEP_READING 3
#define NEXT_REQUEST_EXISTS 4

// send status
#define KEEP_SENDING 0
#define SEND_FINISHED 1
#define SEND_BUFF_SIZE 32768

class Connection {
 public:
  enum {
    kClose = 0,
    kReset,
    kNextReq,
  };

  Connection(void);
  ~Connection(void);

  void Reset(int does_next_req_exist = kClose);
  ResponseManager::IoFdPair HandleRequest(void);
  ResponseManager::IoFdPair ExecuteMethod(int event_fd);
  void FormatResponse(const int event_fd);
  void Send(void);

  bool IsResponseBufferReady(void) const;

  const int get_send_status(void) const;
  const int get_connection_status(void) const;

  const int get_fd(void) const { return fd_; }

  void SetAttributes(const int fd, const std::string& client_addr,
                     const uint16_t port, ServerRouter& server_router);

 private:
  typedef std::queue<ResponseBuffer> ResponseQueue;
  typedef std::map<int, ResponseManager*> ResponseManagerMap;

  int fd_;  // connected socket fd
  int connection_status_;
  int send_status_;
  uint16_t port_;
  std::string client_addr_;
  std::string buffer_;
  ResponseQueue response_queue_;
  ResponseManagerMap resource_manager_map_;

  HttpParser parser_;
  Router* router_;
  HeaderFormatter response_formatter_;

  void Receive(void);
  ResponseManager* GenerateResponseManager(int status, int req_status,
                                           Request& request,
                                           Router::Result& router_result);
  void SetIov(struct iovec* iov, size_t& cnt, ResponseBuffer& res);
  void UpdateRequestResult(bool is_keep_alive);
  int ValidateLocalRedirPath(std::string& loc, RequestLine& req,
                             int& req_status);
};

#endif  // INCLUDES_CONNECTION_HPP_
