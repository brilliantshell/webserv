/**
 * @file HttpServer.cpp
 * @author ghan, jiskim, yongjule
 * @brief Accept connections, receive requests, send responses
 * @date 2022-09-21
 *
 * @copyright Copyright (c) 2022
 */

#include "HttpServer.hpp"

HttpServer::HttpServer(const ServerConfig& config)
    : passive_sockets_(PassiveSockets(config.port_set)),
      port_map_(config.port_map),
      connections_(MAX_CONNECTIONS) {}

HttpServer::~HttpServer() { close(kq_); }

void HttpServer::Run(void) {
  InitKqueue();
  struct kevent events[MAX_EVENTS];

  while (true) {
    int number_of_events = kevent(kq_, NULL, 0, events, MAX_EVENTS, NULL);
    if (number_of_events == -1) {
      // FIXME
      std::cerr << "HttpServer : kevent failed : " << strerror(errno) << '\n';
      return;
    }
    for (int i = 0; i < number_of_events; ++i) {
      if (passive_sockets_.count(events[i].ident) == 1) {
        AcceptConnection(events[i].ident);
      } else if (events[i].flags & EV_EOF) {
        close(events[i].ident);
        connections_[events[i].ident].Reset();
      } else if (events[i].filter == EVFILT_READ) {
        ReceiveRequests(events[i].ident);
      } else if (events[i].filter == EVFILT_WRITE) {
        SendResponses(events[i].ident);
      }
    }
  }
}

// private
void HttpServer::InitKqueue(void) {
  kq_ = kqueue();
  if (kq_ == -1) {
    std::cerr << "HttpServer : kqueue failed : " << strerror(errno) << '\n';
  }
  try {
    struct kevent* sock_ev = new struct kevent[passive_sockets_.size()];
    int i = 0;
    for (ListenerMap::const_iterator it = passive_sockets_.begin();
         it != passive_sockets_.end(); ++it) {
      EV_SET(&sock_ev[i++], it->first, EVFILT_READ, EV_ADD, 0, 0, NULL);
    }
    if (kevent(kq_, sock_ev, passive_sockets_.size(), NULL, 0, NULL) == -1) {
      std::cerr << " HttpServer : InitKqueue failed : " << strerror(errno)
                << '\n';
      return;  // error handling
    }
    delete[] sock_ev;
  } catch (std::bad_alloc& e) {
    std::cerr << "HttpServer : " << e.what() << '\n';
  }
  // error handling
}

void HttpServer::UpdateKqueue(struct kevent* sock_ev, int socket_fd,
                              int16_t ev_filt, uint16_t ev_flag) {
  EV_SET(sock_ev, socket_fd, ev_filt, ev_flag, 0, 0, NULL);
  if (kevent(kq_, sock_ev, 1, NULL, 0, NULL) == -1) {
    std::cerr << " HttpServer : UpdateKqueue failed : " << strerror(errno)
              << '\n';
    return;  // error handling
  }
}

void HttpServer::AcceptConnection(int socket_fd) {
  sockaddr_in addr;
  socklen_t addr_len = sizeof(addr);
  int fd = accept(socket_fd, reinterpret_cast<sockaddr*>(&addr), &addr_len);
  if (fd == -1) {
    std::cerr << "HttpServer : accept failed : " << strerror(errno) << '\n';
    return;
  }
  fcntl(fd, F_SETFL, O_NONBLOCK);
  const uint16_t port = passive_sockets_[socket_fd];
  connections_[fd].SetAttributes(fd, inet_ntoa(addr.sin_addr), port,
                                 port_map_[port]);
  if (connections_[fd].get_connection_status() == CONNECTION_ERROR) {
    std::cerr << "HttpServer : connection attributes set up failed : "
              << strerror(errno) << '\n';
    connections_[fd].Reset();
    return;
  }
  int buf_size = SEND_BUFF_SIZE;
  setsockopt(fd, SOL_SOCKET, SO_SNDLOWAT, &buf_size, sizeof(int));
  struct kevent sock_ev;
  UpdateKqueue(&sock_ev, fd, EVFILT_READ, EV_ADD | EV_ONESHOT);
}

void HttpServer::ReceiveRequests(int event_fd) {
  struct kevent sock_ev;
  connections_[event_fd].HandleRequest();
  UpdateKqueue(&sock_ev, event_fd, EVFILT_READ, EV_ADD | EV_ONESHOT);
  if (connections_[event_fd].get_connection_status() == KEEP_READING) {
    return;
  }
  UpdateKqueue(&sock_ev, event_fd, EVFILT_WRITE, EV_ADD | EV_ONESHOT);
  if (connections_[event_fd].get_connection_status() == CLOSE) {
    return;
  }
  while (connections_[event_fd].get_connection_status() ==
         NEXT_REQUEST_EXISTS) {
    connections_[event_fd].HandleRequest();
  }
}

void HttpServer::SendResponses(int event_fd) {
  struct kevent sock_ev;
  if (connections_[event_fd].get_send_status() == KEEP_SENDING) {
    connections_[event_fd].Send();
    UpdateKqueue(&sock_ev, event_fd, EVFILT_WRITE, EV_ADD | EV_ONESHOT);
  }
}
