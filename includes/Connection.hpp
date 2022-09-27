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

  void Close(void);
  void Receive(void);
  void Send(void);

  void set_fd(int fd);

 private:
  int fd_;
  char buffer_[BUFFER_SIZE];
};

#endif  // INCLUDES_CONNECTION_HPP_
