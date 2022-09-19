/**
 * @file SocketGenerator.hpp
 * @author ghan, jiskim, yongjule
 * @brief Open, bind passive sockets
 * @date 2022-09-19
 *
 * @copyright Copyright (c) 2022
 */

#ifndef INCLUDES_SOCKET_GENERATOR_HPP_
#define INCLUDES_SOCKET_GENERATOR_HPP_

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <iostream>

#include "Types.hpp"

class SocketGenerator {
 public:
  ListenerMap Generate(const HostVector& host_vector);

 private:
  void InitializeSockAddr(const HostPair& host_pair, sockaddr_in* addr) const;
};

#endif  // INCLUDES_SOCKET_GENERATOR_HPP_