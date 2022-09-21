/**
 * @file HttpServer.cpp
 * @author ghan, jiskim, yongjule
 * @brief Accept connections, receive requests, send responses
 * @date 2022-09-21
 *
 * @copyright Copyright (c) 2022
 */

#include "HttpServer.hpp"

#include <sys/event.h>
#include <sys/time.h>
#include <sys/types.h>

#include <iostream>

HttpServer::HttpServer(const ListenerMap& listener_map)
    : listener_map_(listener_map) {}

void HttpServer::Run(void) {
  //   int kq = kqueue();
  //   struct kevent event;
  //   EV_SET(&event, listener_map_.begin()->second, EVFILT_READ, EV_ADD, 0, 0,
  //          NULL);

  int fd = accept(listener_map_.begin()->first, NULL, NULL);
  if (fd == -1) {
    std::cerr << "HttpServer : accept for " << listener_map_.begin()->second
              << " failed : " << strerror(errno) << '\n';
    return;
  }

  char buf[4097];
  if (recv(fd, buf, 4097, 0) == -1) {  // fd, buf, len, flags
    std::cerr << "HttpServer : recv failed : " << strerror(errno) << '\n';
    return;
  }

  if (send(fd, buf, 4097, 0) == -1) {  // fd, buf, len, flags
    std::cerr << "HttpServer : send failed : " << strerror(errno) << '\n';
    return;
  }
  close(fd);
}
