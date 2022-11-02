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

#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/event.h>
#include <sys/resource.h>

#include <iostream>

#include "Connection.hpp"
#include "PassiveSockets.hpp"
#include "Types.hpp"

#define MAX_EVENTS 64

class HttpServer {
 public:
  HttpServer(const ServerConfig& config);
  ~HttpServer(void);
  void Run(void);

 private:
  typedef std::vector<Connection> ConnectionVector;
  typedef std::map<int, int> IoFdMap;

  int kq_;
  PortMap port_map_;
  PassiveSockets passive_sockets_;
  ConnectionVector connections_;
  IoFdMap io_fd_map_;
  std::set<int> close_io_fds_;

  void InitKqueue(void);
  void UpdateKqueue(struct kevent* sock_ev, int socket_fd, int16_t ev_filt,
                    uint16_t ev_flag);
  void AcceptConnection(int socket_fd);
  void ReceiveRequests(const int socket_fd);
  void SendResponses(int socket_fd);
  void HandleIOEvent(struct kevent& event);
  void HandleConnectionEvent(struct kevent& event);
  void RegisterIoEvents(ResponseManager::IoFdPair io_fds, int socket_fd = -1);
  void ClearConnectionResources(int socket_fd);
};

#endif  // INCLUDES_HTTPSERVER_HPP_
