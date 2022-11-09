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
#include <sys/event.h>
#include <sys/resource.h>

#include "Connection.hpp"
#include "PassiveSockets.hpp"
#include "Utils.hpp"

#define MAX_EVENTS 64

#define CONNECTION_TIMEOUT 30
#define REQUEST_TIMEOUT 5

class HttpServer {
 public:
  HttpServer(const ServerConfig& kConfig);
  ~HttpServer(void);
  void Run(void);

 private:
  typedef std::vector<Connection> ConnectionVector;
  typedef std::map<int, int> IoFdMap;
  typedef std::set<int> CloseIoFdSet;

  int kq_;
  HostPortMap host_port_map_;
  PassiveSockets passive_sockets_;
  ConnectionVector connections_;
  IoFdMap io_fd_map_;
  CloseIoFdSet close_io_fds_;

  void HandleIOEvent(struct kevent& event);
  void HandleConnectionEvent(struct kevent& event);

  void InitKqueue(void);
  void UpdateKqueue(int socket_fd, int16_t ev_filt, uint16_t ev_flag);
  void UpdateTimerEvent(int id, uint16_t ev_filt, intptr_t data);

  void AcceptConnection(int socket_fd);
  void ReceiveRequests(const int kSocketFd);
  void SendResponses(int socket_fd);

  void RegisterIoEvents(ResponseManager::IoFdPair io_fds,
                        const int kSocketFd = -1);
  void ClearConnectionResources(int socket_fd);
};

#endif  // INCLUDES_HTTPSERVER_HPP_
