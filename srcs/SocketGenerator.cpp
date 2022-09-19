/**
 * @file SocketGenerator.cpp
 * @author ghan, jiskim, yongjule
 * @brief Open, bind passive sockets
 * @date 2022-09-19
 *
 * @copyright Copyright (c) 2022
 */

#include "SocketGenerator.hpp"

#include <netdb.h>

void SocketGenerator::InitializeSockAddr(const HostPair& host_pair,
                                         sockaddr_in* addr) const {
  memset(addr, 0, sizeof(sockaddr_in));
  addr->sin_family = AF_INET;
  addr->sin_port = htons(host_pair.port);
  struct hostent* ent = gethostbyname(host_pair.host.c_str());
  memcpy(&(addr->sin_addr.s_addr), ent->h_addr_list[0], ent->h_length);
  // addr->sin_addr.s_addr = inet_addr(host_pair.host.c_str());
}

ListenerMap SocketGenerator::Generate(const HostVector& host_vector) {
  ListenerMap listeners;
  sockaddr_in addr;
  int fd;

  for (HostVector::const_iterator it = host_vector.begin();
       it != host_vector.end(); ++it) {
    InitializeSockAddr(*it, &addr);
    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
      std::cerr << strerror(errno) << '\n'
                << "socket for " << it->host << ":" << it->port
                << " cannot be opened" << '\n';
      continue;
    }
    if (bind(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
      std::cerr << strerror(errno) << '\n'
                << "socket for " << it->host << ":" << it->port
                << " cannot be bound" << '\n';
      close(fd);
      continue;
    }
    listen(fd, 64);
    listeners[fd] = it->host;
  }
  return listeners;
}
