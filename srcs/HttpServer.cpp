/**
 * @file HttpServer.cpp
 * @author ghan, jiskim, yongjule
 * @brief Accept connections, receive requests, send responses
 * @date 2022-09-21
 *
 * @copyright Copyright (c) 2022
 */

#include "HttpServer.hpp"

HttpServer::HttpServer(const PortSet& port_set)
    : passive_sockets_(PassiveSockets(port_set)),
      connections_(MAX_CONNECTIONS) {}

HttpServer::~HttpServer() { close(kq_); }

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

void HttpServer::AcceptConnection(struct kevent* sock_ev, int socket_fd) {
  int fd = accept(socket_fd, NULL, NULL);
  if (fd == -1) {
    std::cerr << "HttpServer : accept failed : " << strerror(errno) << '\n';
    return;
  }
  fcntl(fd, F_SETFL, O_NONBLOCK);
  connections_[fd].set_fd(fd);
  UpdateKqueue(sock_ev, fd, EVFILT_READ, EV_ADD);
}

void HttpServer::Run(void) {
  InitKqueue();
  struct kevent events[MAX_EVENTS];
  struct kevent sock_ev;

  while (true) {
    int number_of_events = kevent(kq_, NULL, 0, events, MAX_EVENTS, NULL);
    if (number_of_events == -1) {
      std::cerr << "HttpServer : kevent failed : " << strerror(errno) << '\n';
      return;
    }
    for (int i = 0; i < number_of_events; ++i) {
      if (passive_sockets_.count(events[i].ident) == 1) {
        AcceptConnection(&sock_ev, events[i].ident);
        UpdateKqueue(&sock_ev, events[i].ident, EVFILT_READ, EV_ADD);
      } else if (events[i].flags & EV_EOF) {
        connections_[events[i].ident].Close();
      } else if (events[i].filter == EVFILT_READ) {
        connections_[events[i].ident].Receive();
        connections_[events[i].ident].Send();
        UpdateKqueue(&sock_ev, events[i].ident, EVFILT_WRITE,
                     EV_ADD | EV_ONESHOT);
      } else if (events[i].filter == EVFILT_WRITE) {
        UpdateKqueue(&sock_ev, events[i].ident, EVFILT_READ,
                     EV_ADD | EV_ONESHOT);
      }
    }
  }
}
