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

#define BUFFER_SIZE 4097

#define KEEP_ALIVE 0
#define CLOSE_1_0 1
#define CLOSE_LENGTH 2
#define CLOSE_RECV 3
#define CLOSE_PARSE 4
#define CLOSE_ROUTE 5
#define CLOSE_RESOURCE 6
#define CLOSE_RESPONSE 7
#define CLOSE_SEND 8

#include <fcntl.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#include <iostream>
#include <string>

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
  char buffer_[BUFFER_SIZE];

  void Receive(void);
  void Send(void);
};

#endif  // INCLUDES_CONNECTION_HPP_
