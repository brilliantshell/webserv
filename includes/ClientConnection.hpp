/**
 * @file ClientConnection.hpp
 * @author ghan, jiskim, yongjule
 * @brief simple client
 * @date 2022-09-21
 *
 * @copyright Copyright (c) 2022
 */

#ifndef INCLUDES_CLIENT_CONNECTION_HPP_
#define INCLUDES_CLIENT_CONNECTION_HPP_

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <iostream>

#define RANDOM_STR                                                             \
  "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" \
  "3MRhk8VQf6Th3Wo9fJRhBUMjdi"

class ClientConnection {
 public:
  ClientConnection(void);
  ~ClientConnection(void);

  void Connect(uint16_t port);
  void SendMessage(const std::string& message);
  std::string ReceiveMessage(void);

 private:
  int socket_fd_;
};

#endif  // INCLUDES_CLIENT_CONNECTION_HPP_
