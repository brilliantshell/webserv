/**
 * @file HttpServer.hpp
 * @author ghan, jiskim, yongjule
 * @brief Accept connections, receive requests, send responses
 * @date 2022-09-21
 *
 * @copyright Copyright (c) 2022
 */

#ifndef INCLUDES_HTTPSERVER_HPP_
#define INCLUDES_HTTPSERVER_HPP_

#include <fcntl.h>
#include <sys/event.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <iostream>

#include "PassiveSockets.hpp"
#include "Types.hpp"

#define BUFFER_SIZE 4097
#define MAX_EVENTS 64

class HttpServer {
 public:
  HttpServer(const PortSet& port_set);
  void Run(void);
  ~HttpServer(void);

 private:
  int kq_;
  PassiveSockets passive_sockets_;

  void InitKqueue(void);
  void UpdateKqueue(struct kevent* sock_ev, int socket_fd, int16_t ev_filt,
                    uint16_t ev_flag);
  void AcceptConnection(struct kevent* sock_ev, int socket_fd);
};

#endif  // INCLUDES_HTTPSERVER_HPP_
