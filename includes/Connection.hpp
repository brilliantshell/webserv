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
#include <unistd.h>

#include <cstring>
#include <iostream>
#include <queue>
#include <string>

#include "HttpParser.hpp"
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
#define KEEP_SENDING_HEADER 0
#define KEEP_SENDING_CONTENT 1
#define SEND_FINISHED 3

#define SND_BUFF_SIZE 32768

class Connection {
 public:
  enum {
    kReset = 0,
    kNextReq,
  };

  Connection(void);
  ~Connection(void);

  void Reset(bool does_next_req_exist = kReset);
  void HandleRequest(void);
  void Send(void);

  const int get_send_status(void) const;
  const int get_connection_status(void) const;

  void SetAttributes(const int fd, const std::string& client_addr,
                     const uint16_t port, ServerRouter& server_router);

 private:
  struct ResponseBuffer {
    std::string header;
    std::string content;
  };

  typedef std::queue<ResponseBuffer> ResponseQueue;

  int fd_;
  int connection_status_;
  int send_status_;
  uint16_t port_;
  ssize_t sent_bytes_;
  std::string client_addr_;
  std::string buffer_;
  ResponseQueue response_queue_;

  HttpParser parser_;
  Router* router_;
  ResourceManager resource_manager_;
  ResponseFormatter response_formatter_;

  void Receive(void);
};

#endif  // INCLUDES_CONNECTION_HPP_
