/**
 * @file HttpServer.cpp
 * @author ghan, jiskim, yongjule
 * @brief Accept connections, receive requests, send responses
 * @date 2022-09-21
 *
 * @copyright Copyright (c) 2022
 */

#include "HttpServer.hpp"

#define PRINT_EVENT(event)                                                 \
  std::cerr << "event: ident: [" << event.ident                            \
            << "], filter(RD: -1, WR: -2) : [" << event.filter             \
            << "], flags : [" << std::hex << event.flags << "], fflags: [" \
            << std::dec << event.fflags << "], data: [" << event.data      \
            << "], udata: [" << event.udata << "]" << std::endl;

HttpServer::HttpServer(const ServerConfig& kConfig)
    : port_map_(kConfig.port_map),
      passive_sockets_(PassiveSockets(kConfig.port_set)) {
  struct rlimit fd_limit;
  getrlimit(RLIMIT_NOFILE, &fd_limit);
  connections_.resize(fd_limit.rlim_cur);
}

HttpServer::~HttpServer() { close(kq_); }

void HttpServer::Run(void) {
  InitKqueue();
  struct kevent events[MAX_EVENTS];

  while (true) {
    int number_of_events = kevent(kq_, NULL, 0, events, MAX_EVENTS, NULL);
    if (number_of_events == -1) {
      std::cerr << "HttpServer : kevent failed : " << strerror(errno) << '\n';
      exit(EXIT_FAILURE);  // TODO : error handling
    }
    for (int i = 0; i < number_of_events; ++i) {
      PRINT_EVENT(events[i]);
      if (events[i].filter != EVFILT_TIMER &&
          passive_sockets_.count(events[i].ident) == 1) {
        AcceptConnection(events[i].ident);
      } else if (events[i].filter == EVFILT_TIMER) {
        ClearConnectionResources(events[i].ident);
      } else if (close_io_fds_.count(events[i].ident) == 0) {
        (io_fd_map_.count(events[i].ident) == 1)
            ? HandleIOEvent(events[i])
            : HandleConnectionEvent(events[i]);
      }
    }
    close_io_fds_.clear();
  }
}

void HttpServer::HandleConnectionEvent(struct kevent& event) {
  if (event.flags & EV_EOF && event.filter == EVFILT_READ) {
    ClearConnectionResources(event.ident);
    UpdateTimerEvent(event.ident, EV_DELETE, 0);
  } else {
    if (event.filter == EVFILT_READ) {
      ReceiveRequests(event.ident);
    } else if (event.filter == EVFILT_WRITE) {
      SendResponses(event.ident);
    }
    UpdateTimerEvent(
        event.ident, EV_ADD | EV_ONESHOT,
        (connections_[event.ident].get_connection_status() == KEEP_READING)
            ? 5
            : 30);
    if (connections_[event.ident].get_connection_status() == CONNECTION_ERROR) {
      ClearConnectionResources(event.ident);
      UpdateTimerEvent(event.ident, EV_DELETE, 0);
    }
  }
}

void HttpServer::HandleIOEvent(struct kevent& event) {
  int socket_fd = io_fd_map_[event.ident];
  if (event.filter == EVFILT_READ || event.filter == EVFILT_WRITE) {
    ResponseManager::IoFdPair io_fds =
        connections_[socket_fd].ExecuteMethod(event.ident);
    if (connections_[socket_fd].get_connection_status() == CONNECTION_ERROR) {
      ClearConnectionResources(socket_fd);
      UpdateTimerEvent(socket_fd, EV_DELETE, 0);
      return;
    }
    RegisterIoEvents(io_fds, connections_[socket_fd].get_fd());
    if (connections_[socket_fd].IsResponseBufferReady() == true) {
      UpdateKqueue(socket_fd, EVFILT_WRITE, EV_ADD | EV_ONESHOT);
    }
    if (static_cast<int>(event.ident) != io_fds.input &&
        static_cast<int>(event.ident) != io_fds.output) {
      io_fd_map_.erase(event.ident);
    }
  }
}

// SECTION: private
void HttpServer::InitKqueue(void) {
  kq_ = kqueue();
  if (kq_ == -1) {
    std::cerr << "HttpServer : kqueue failed : " << strerror(errno) << '\n';
    exit(EXIT_FAILURE);
  }
  struct kevent* sock_ev =
      new (std::nothrow) struct kevent[passive_sockets_.size()];
  if (sock_ev == NULL) {
    std::cerr << "HttpServer : Failed to allocate memory\n";
    exit(EXIT_FAILURE);
  }
  int i = 0;
  for (ListenerMap::const_iterator it = passive_sockets_.begin();
       it != passive_sockets_.end(); ++it) {
    EV_SET(&sock_ev[i++], it->first, EVFILT_READ, EV_ADD, 0, 0, NULL);
    std::cerr << "HttpServer : Added socket fd : " << it->first << '\n';
  }
  if (kevent(kq_, sock_ev, passive_sockets_.size(), NULL, 0, NULL) == -1) {
    std::cerr << " HttpServer : Failed to listen : " << strerror(errno) << '\n';
    exit(EXIT_FAILURE);
  }
  delete[] sock_ev;
}

void HttpServer::UpdateKqueue(int socket_fd, int16_t ev_filt,
                              uint16_t ev_flag) {
  struct kevent sock_ev;
  EV_SET(&sock_ev, socket_fd, ev_filt, ev_flag, 0, 0, NULL);
  if (kevent(kq_, &sock_ev, 1, NULL, 0, NULL) == -1) {
    std::cerr << " HttpServer : UpdateKqueue failed : " << strerror(errno)
              << '\n';
    return;  // TODO : error handling?
  }
}

// data = timeout period
void HttpServer::UpdateTimerEvent(int id, uint16_t ev_filt, intptr_t data) {
  struct kevent timer_ev;
  EV_SET(&timer_ev, id, EVFILT_TIMER, ev_filt, NOTE_SECONDS, data, NULL);
  if (kevent(kq_, &timer_ev, 1, NULL, 0, NULL) == -1) {
    std::cerr << " HttpServer : UpdateTimerEvent failed : " << strerror(errno)
              << '\n';
    return;  // TODO : error handling?
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
  int buf_size = SEND_BUFF_SIZE;
  if (setsockopt(fd, SOL_SOCKET, SO_SNDLOWAT, &buf_size, sizeof(int)) == -1) {
    std::cerr << "HttpServer : setsockopt failed : " << strerror(errno) << '\n';
    close(fd);
    return;
  }
  const uint16_t kPort = passive_sockets_[socket_fd];
  connections_[fd].SetAttributes(fd, inet_ntoa(addr.sin_addr), kPort,
                                 port_map_[kPort]);
  if (connections_[fd].get_connection_status() == CONNECTION_ERROR) {
    std::cerr << "HttpServer : connection attributes set up failed : "
              << strerror(errno) << '\n';
    connections_[fd].Reset();
    return;
  }
  UpdateKqueue(fd, EVFILT_READ, EV_ADD | EV_ONESHOT);
  UpdateTimerEvent(fd, EV_ADD | EV_ONESHOT, 30);
}

void HttpServer::ReceiveRequests(const int kSocketFd) {
  Connection& connection = connections_[kSocketFd];
  ResponseManager::IoFdPair io_fds = connection.HandleRequest();
  if (connection.get_connection_status() == CONNECTION_ERROR) {
    return;
  }
  UpdateKqueue(kSocketFd, EVFILT_READ, EV_ADD | EV_ONESHOT);
  if (connection.get_connection_status() == KEEP_READING) {
    return;
  }
  RegisterIoEvents(io_fds, kSocketFd);
  if (connection.IsResponseBufferReady() == true) {
    UpdateKqueue(kSocketFd, EVFILT_WRITE, EV_ADD | EV_ONESHOT);
  }
  while (connection.get_connection_status() == NEXT_REQUEST_EXISTS) {
    io_fds = connection.HandleRequest();
    RegisterIoEvents(io_fds, kSocketFd);
  }
}

void HttpServer::SendResponses(int socket_fd) {
  Connection& connection = connections_[socket_fd];
  if (connection.get_send_status() < SEND_FINISHED) {
    connection.Send();
    if (connection.get_connection_status() == CONNECTION_ERROR) {
      return;
    }
    std::cerr << std::boolalpha << "is conn status close? : "
              << (connection.get_connection_status() == CLOSE) << '\n'
              << "is send status send next? : "
              << (connection.get_send_status() == SEND_NEXT) << '\n'
              << "is the req/res pair synced? : "
              << connection.IsHttpPairSynced() << '\n';
    if (connection.get_connection_status() == CLOSE &&
        connection.get_send_status() > KEEP_SENDING &&
        connection.IsHttpPairSynced() == true) {
      std::cerr << "fd: " << socket_fd << "  shutdown\n ";
      shutdown(socket_fd, SHUT_WR);
    }
    if (connection.IsResponseBufferReady() == true) {
      UpdateKqueue(socket_fd, EVFILT_WRITE, EV_ADD | EV_ONESHOT);
    }
  }
}

void HttpServer::RegisterIoEvents(ResponseManager::IoFdPair io_fds,
                                  const int kSocketFd) {
  if (io_fds.input != -1) {
    if (kSocketFd > 0) {
      io_fd_map_[io_fds.input] = kSocketFd;
    }
    UpdateKqueue(io_fds.input, EVFILT_READ, EV_ADD | EV_ONESHOT);
  }
  if (io_fds.output != -1) {
    if (kSocketFd > 0) {
      io_fd_map_[io_fds.output] = kSocketFd;
    }
    UpdateKqueue(io_fds.output, EVFILT_WRITE, EV_ADD | EV_ONESHOT);
  }
}

void HttpServer::ClearConnectionResources(int socket_fd) {
  close(socket_fd);
  for (IoFdMap::const_iterator it = io_fd_map_.begin();
       it != io_fd_map_.end();) {
    IoFdMap::const_iterator tmp_it = it;
    ++it;
    if (tmp_it->second == socket_fd) {
      close(tmp_it->first);
      close_io_fds_.insert(tmp_it->first);
      io_fd_map_.erase(tmp_it->first);
    }
  }
  connections_[socket_fd].Reset();
}
