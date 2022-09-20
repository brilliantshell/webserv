/**
 * @file SocketGenerator.cpp
 * @author ghan, jiskim, yongjule
 * @brief Open, bind passive sockets
 * @date 2022-09-19
 *
 * @copyright Copyright (c) 2022
 */

#include "SocketGenerator.hpp"

static void InitializeSockAddr(const uint16_t port, sockaddr_in* addr) {
  memset(addr, 0, sizeof(sockaddr_in));
  addr->sin_family = AF_INET;
  addr->sin_port = htons(port);
  addr->sin_addr.s_addr = INADDR_ANY;
}

ListenerMap socket_generator::GenerateSocket(const PortSet& port_set) {
  ListenerMap listeners;
  sockaddr_in addr;
  int fd;

  for (PortSet::const_iterator it = port_set.begin(); it != port_set.end();
       ++it) {
    InitializeSockAddr(*it, &addr);
    fd = socket(AF_INET, SOCK_STREAM, 0);  // NOTE : 0 맞나?
    if (fd < 0) {
      std::cerr << "webserv : " << strerror(errno) << '\n'
                << "socket for " << *it << " cannot be opened" << '\n';
      continue;
    }
    if (bind(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
      std::cerr << "webserv : " << strerror(errno) << " : "
                << "socket for " << *it << " cannot be bound" << '\n';
      close(fd);
      continue;
    }
    listen(fd, 64);
    listeners[fd] = *it;
  }
  return listeners;
}
