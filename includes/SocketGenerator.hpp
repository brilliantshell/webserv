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

namespace socket_generator {

ListenerMap GenerateSocket(const PortSet& port_set);

}  // namespace socket_generator

#endif  // INCLUDES_SOCKET_GENERATOR_HPP_
