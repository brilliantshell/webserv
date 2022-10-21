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

#define KEEP_ALIVE 0
#define CLOSE 1
#define CONNECTION_ERROR 2

#include <fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <cstring>
#include <iostream>
#include <string>

#include "HttpParser.hpp"
#include "ResourceManager.hpp"
#include "ResponseFormatter.hpp"
#include "Router.hpp"

class Connection {
 public:
  Connection(void);
  ~Connection(void);

  void Reset(void);
  void HandleRequest(void);

  const int get_status(void) const;

  void SetAttributes(const int fd, const std::string& client_addr,
                     const uint16_t port, ServerRouter& server_router);

 private:
  int fd_;
  int status_;
  uint16_t port_;
  std::string buffer_;
  std::string client_addr_;

  HttpParser parser_;
  Router* router_;
  ResourceManager resource_manager_;
  ResponseFormatter response_formatter_;

  void Receive(void);
  void Send(const std::string& response);
};

#endif  // INCLUDES_CONNECTION_HPP_
