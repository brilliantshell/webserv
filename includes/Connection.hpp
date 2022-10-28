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

#include "HttpParser.hpp"
#include "PathResolver.hpp"
#include "ResourceManager.hpp"
#include "ResponseFormatter.hpp"
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

  void Reset(bool does_next_req_exist = kClose);
  void HandleRequest(void);
  void Send(void);

  const int get_send_status(void) const;
  const int get_connection_status(void) const;

  void SetAttributes(const int fd, const std::string& client_addr,
                     const uint16_t port, ServerRouter& server_router);

 private:
  struct ResponseBuffer {
    enum {
      kHeader = 0,
      kContent,
    };

    int current_buf;
    size_t offset;
    size_t total_len;
    std::string header;
    std::string content;

    ResponseBuffer(void) : current_buf(kHeader), offset(0), total_len(0) {}
  };

  typedef std::queue<ResponseBuffer> ResponseQueue;

  int fd_;
  int connection_status_;
  int send_status_;
  uint16_t port_;
  std::string client_addr_;
  std::string buffer_;
  ResponseQueue response_queue_;

  HttpParser parser_;
  Router* router_;
  ResourceManager resource_manager_;
  ResponseFormatter response_formatter_;

  void Receive(void);
  void GenerateResponse(int status, int req_status, Request& request);
  void SetIov(struct iovec* iov, size_t& cnt, ResponseBuffer& res);
  void UpdateRequestResult(bool is_keep_alive);
  int ValidateLocalRedirPath(std::string& loc, RequestLine& req,
                             int& req_status);
};

#endif  // INCLUDES_CONNECTION_HPP_
