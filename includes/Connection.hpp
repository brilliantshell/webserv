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

#include <fcntl.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#include <iostream>
#include <string>

#include "HttpParser.hpp"

class Connection {
 public:
  Connection(void);
  ~Connection(void);

  void Reset(void);
  void HandleRequest(void);

  void set_fd(int fd);
  const int get_status(void) const;

 private:
  int fd_;
  int status_;
  std::string buffer_;
  HttpParser parser_;

  void Receive(void);
  void Send(void);
};

#endif  // INCLUDES_CONNECTION_HPP_
