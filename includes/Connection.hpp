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

  void set_fd(int fd);
  const int get_status(void) const;

  void set_client_addr(std::string client_addr);
  void set_router(ServerRouter& router);
  void set_port(uint16_t port);
  const std::string& get_client_addr(void) const;

 private:
  int fd_;
  int status_;
  uint16_t port_;
  std::string buffer_;
  std::string client_addr_;

  HttpParser parser_;
  Router* router_;

  void Receive(void);
  void Send(const std::string& response);
};

#endif  // INCLUDES_CONNECTION_HPP_
