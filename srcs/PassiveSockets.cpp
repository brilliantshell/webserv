/**
 * @file PassiveSockets.cpp
 * @author ghan, jiskim, yongjule
 * @brief
 * @date 2022-09-24
 *
 * @copyright Copyright (c) 2022
 */

#include "PassiveSockets.hpp"

PassiveSockets::PassiveSockets(const PortSet& port_set) { Listen(port_set); }

PassiveSockets::~PassiveSockets(void) {
  for (ListenerMap::const_iterator it = begin(); it != end(); ++it) {
    close(it->first);
  }
}

void PassiveSockets::Listen(const PortSet& port_set) {
  for (PortSet::const_iterator it = port_set.begin(); it != port_set.end();
       ++it) {
    int fd = BindSocket(OpenSocket(*it), *it);
    if (fd != -1) {
      insert(std::make_pair(fd, *it));
    }
  }
}

void PassiveSockets::InitializeSockAddr(const uint16_t port,
                                        sockaddr_in* addr) {
  memset(addr, 0, sizeof(sockaddr_in));
  addr->sin_family = AF_INET;
  addr->sin_port = htons(port);
  addr->sin_addr.s_addr = INADDR_ANY;
}

int PassiveSockets::OpenSocket(const uint16_t port) {
  sockaddr_in addr;
  InitializeSockAddr(port, &addr);
  int fd = socket(AF_INET, SOCK_STREAM, 0);  // NOTE : 0 맞나?
  if (fd < 0) {
    std::cerr << "webserv : " << strerror(errno) << '\n'
              << "socket for " << port << " cannot be opened" << '\n';
    return -1;
  }
  return fd;
}

int PassiveSockets::BindSocket(int fd, const uint16_t port) {
  sockaddr_in addr;

  if (fd != -1) {
    InitializeSockAddr(port, &addr);
    if (bind(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
      std::cerr << "webserv : " << strerror(errno) << " : "
                << "socket for " << port << " cannot be bound" << '\n';
      close(fd);
      return -1;
    }
    listen(fd, 64);
  }
  return fd;
}
