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

#include <utility>

#include "Utils.hpp"

#define BACKLOG 128

class PassiveSockets : public ListenerMap {
 public:
  PassiveSockets(const HostPortSet& kHostPortSet);
  ~PassiveSockets(void);

 private:
  void Listen(const HostPortSet& kHostPortSet);
  void InitializeSockAddr(const HostPortPair& kHostPort, sockaddr_in* addr);
  int OpenSocket(const HostPortSet::const_iterator& kIt);
  int BindSocket(int fd, const HostPortPair& kHostPort);
};

#endif  // INCLUDES_PASSIVE_SOCKETS_HPP_
