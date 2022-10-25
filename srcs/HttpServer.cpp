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

// #include <execinfo.h>

void HttpServer::UpdateKqueue(struct kevent* sock_ev, int socket_fd,
                              int16_t ev_filt, uint16_t ev_flag) {
  EV_SET(sock_ev, socket_fd, ev_filt, ev_flag, 0, 0, NULL);
  if (kevent(kq_, sock_ev, 1, NULL, 0, NULL) == -1) {
    std::cerr << " HttpServer : UpdateKqueue failed : " << strerror(errno)
              << '\n';
    // std::cerr << "Stack Back trace:\n";
    // void* array[10];
    // size_t size;
    // size = backtrace(array, 10);
    // backtrace_symbols_fd(array, size, STDERR_FILENO);
    return;  // error handling
  }
}

bool HttpServer::AcceptConnection(struct kevent* sock_ev, int socket_fd) {
  sockaddr_in addr;
  socklen_t addr_len = sizeof(addr);
  int fd = accept(socket_fd, reinterpret_cast<sockaddr*>(&addr), &addr_len);
  if (fd == -1) {
    std::cerr << "HttpServer : accept failed : " << strerror(errno) << '\n';
    return false;
  }
  fcntl(fd, F_SETFL, O_NONBLOCK);
  const uint16_t port = passive_sockets_[socket_fd];
  connections_[fd].SetAttributes(fd, inet_ntoa(addr.sin_addr), port,
                                 port_map_[port]);
  if (connections_[fd].get_connection_status() == CONNECTION_ERROR) {
    std::cerr << "HttpServer : connection attributes set up failed : "
              << strerror(errno) << '\n';
    connections_[fd].Reset();
    return false;
  }
  int buf_size = SND_BUFF_SIZE;
  setsockopt(fd, SOL_SOCKET, SO_SNDLOWAT, &buf_size, sizeof(int));
  UpdateKqueue(sock_ev, fd, EVFILT_READ, EV_ADD | EV_ONESHOT);
  return true;
}

void HttpServer::Run(void) {
  InitKqueue();
  struct kevent events[MAX_EVENTS];
  struct kevent sock_ev;

  while (true) {
    int number_of_events = kevent(kq_, NULL, 0, events, MAX_EVENTS, NULL);
    if (number_of_events == -1) {
      // FIXME
      std::cerr << "HttpServer : kevent failed : " << strerror(errno) << '\n';
      return;
    }
    std::cerr << "number of events : " << number_of_events << '\n';
    for (int i = 0; i < number_of_events; ++i) {
      if (passive_sockets_.count(events[i].ident) == 1) {
        if (AcceptConnection(&sock_ev, events[i].ident) == true) {
          UpdateKqueue(&sock_ev, events[i].ident, EVFILT_READ,
                       EV_ADD | EV_ONESHOT);
          std::cerr << "Connection Accepted. id : [" << events[i].ident
                    << "]\n";
        }
      } else if (events[i].flags & EV_EOF) {
        std::cerr << "\n>>>Connection Closed : errno : " << events[i].fflags
                  << ", event id : [" << events[i].ident << "]<<<\n\n";
        if (events[i].filter == EVFILT_WRITE) {
          std::cerr << "ㅇㅑ write 다 야\n";
        }
        std::cerr << "EOF ㅍㅣㄹ터 : " << events[i].filter << '\n';
        close(events[i].ident);
        connections_[events[i].ident].Reset();
      } else if (events[i].filter == EVFILT_READ) {
        std::cerr << "EVFILT_READ event id : [" << events[i].ident << "]\n";
        if (connections_[events[i].ident].get_connection_status() !=
            KEEP_ALIVE) {
          continue;
        }
        std::cerr << "Request received\n";
        connections_[events[i].ident].HandleRequest();
        if (connections_[events[i].ident].get_connection_status() ==
            KEEP_READING) {
          UpdateKqueue(&sock_ev, events[i].ident, EVFILT_READ,
                       EV_ADD | EV_ONESHOT);
          continue;
        }
        UpdateKqueue(&sock_ev, events[i].ident, EVFILT_WRITE,
                     EV_ADD | EV_ONESHOT);
      } else if (events[i].filter == EVFILT_WRITE) {
        std::cerr << "EVFILT_WRITE event id : [" << events[i].ident << "]\n";
        if (connections_[events[i].ident].get_connection_status() ==
            NEXT_REQUEST_EXISTS) {
          connections_[events[i].ident].HandleRequest();

          if (connections_[events[i].ident].get_connection_status() ==
              KEEP_READING) {
            UpdateKqueue(&sock_ev, events[i].ident, EVFILT_READ,
                         EV_ADD | EV_ONESHOT);
            continue;
          }
          UpdateKqueue(&sock_ev, events[i].ident, EVFILT_WRITE,
                       EV_ADD | EV_ONESHOT);
          std::cerr << "Server sends data\n";
          continue;
        } else if (connections_[events[i].ident].get_send_status() <
                   SEND_FINISHED) {
          connections_[events[i].ident].Send();
          UpdateKqueue(&sock_ev, events[i].ident, EVFILT_WRITE,
                       EV_ADD | EV_ONESHOT);
          continue;
        }

        if (connections_[events[i].ident].get_connection_status() ==
            KEEP_ALIVE) {
          UpdateKqueue(&sock_ev, events[i].ident, EVFILT_READ,
                       EV_ADD | EV_ONESHOT);
        } else {
          std::cerr << "설마 여기 오냐?\n";
          close(events[i].ident);
          connections_[events[i].ident].Reset();
        }
      }
    }
  }
}
