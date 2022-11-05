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

#include "Types.hpp"

#define BACKLOG 128

class PassiveSockets : public ListenerMap {
 public:
  PassiveSockets(const PortSet& kPortSet);
  ~PassiveSockets(void);

 private:
  void Listen(const PortSet& kPortSet);
  void InitializeSockAddr(const uint16_t kPort, sockaddr_in* addr);
  int OpenSocket(const uint16_t kPort);
  int BindSocket(int fd, const uint16_t kPort);
};

#endif  // INCLUDES_PASSIVE_SOCKETS_HPP_
