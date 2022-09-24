/**
 * @file PassiveSockets.hpp
 * @author ghan, jiskim, yongjule
 * @brief Open, bind passive sockets
 * @date 2022-09-24
 *
 * @copyright Copyright (c) 2022
 */

#ifndef INCLUDES_PASSIVE_SOCKETS_HPP_
#define INCLUDES_PASSIVE_SOCKETS_HPP_

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <iostream>

#include "Types.hpp"

class PassiveSockets : public ListenerMap {
 public:
  PassiveSockets(const PortSet& port_set);
  ~PassiveSockets(void);

 private:
  void Listen(const PortSet& port_set);
  int OpenSocket(const uint16_t port);
  int BindSocket(int fd, const uint16_t port);
  void InitializeSockAddr(const uint16_t port, sockaddr_in* addr);
};

#endif  // INCLUDES_PASSIVE_SOCKETS_HPP_
