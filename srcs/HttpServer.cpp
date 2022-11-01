/**
 * @file HttpServer.cpp
 * @author ghan, jiskim, yongjule
 * @brief Accept connections, receive requests, send responses
 * @date 2022-09-21
 *
 * @copyright Copyright (c) 2022
 */

#include "HttpServer.hpp"

// clang-format off
/*
 fd 맵 -> 커넥션 fd, 커넥션이 관리하는 fd맵 -> fd => connection
 connections_[fd_map[events[i].ident]] ->

 만약 커넥션이 5번 fd로 커넥트를 했고, 6번 fd로 파일을 읽는다면

    fd_map[5] = 5
    fd_map[6] = 5

    5번에 이벤트 일어남 -> connection이 recv, connection이 send
    6번에 이벤트 일어남 -> connection에서 파일 read -> EOF뜨면 close

 만약 커넥션이 5번 fd로 커넥트를 했고 cgi를 실행할때,
 			6번 fd는 write를 하고 7번 fd는 read를 해야한다면
    5번에 이벤트 일어남 -> connection이 recv, connection이 send
    6번에 이벤트 일어남 -> connection에서 data를 pipe에 write, 다썼으면... close인데?
		그냥 close 갈겨놓으면 다쓰고 close 하지 않을까? close하면 EOF 뜨는데 ㅎㅎ
		7번에 이벤트 일어남 -> EOF 뜨면 close

=============================

 ResponseManager -> static, cgi 분리

*/
// clang-format on

#define PRINT_EVENT(event)                                                 \
  std::cerr << "event: ident: [" << event.ident                            \
            << "], filter(RD: -1, WR: -2) : [" << event.filter             \
            << "], flags : [" << std::hex << event.flags << "], fflags: [" \
            << std::dec << event.fflags << "], data: [" << event.data      \
            << "], udata: [" << event.udata << "]" << std::endl;

HttpServer::HttpServer(const ServerConfig& config)
    : passive_sockets_(PassiveSockets(config.port_set)),
      port_map_(config.port_map),
      connections_(MAX_CONNECTIONS) {}

HttpServer::~HttpServer() { close(kq_); }

void HttpServer::Run(void) {
  InitKqueue();
  struct kevent events[MAX_EVENTS];

  while (true) {
    std::cerr << "\n\n ============ new cycle ================\n\n";
    int number_of_events = kevent(kq_, NULL, 0, events, MAX_EVENTS, NULL);
    if (number_of_events == -1) {
      // FIXME
      std::cerr << "HttpServer : kevent failed : " << strerror(errno) << '\n';
      return;
    }
    std::cerr << "\n";
    for (int i = 0; i < number_of_events; ++i) {
      PRINT_EVENT(events[i]);
      if (passive_sockets_.count(events[i].ident) == 1) {
        AcceptConnection(events[i].ident);
      } else if (close_io_fds_.count(events[i].ident) == 0) {
        (connections_[events[i].ident].get_fd() == -1)
            ? HandleIOEvent(events[i])
            : HandleConnectionEvent(events[i]);
      }
    }
    std::cerr << ">>>>>>>>> close io fds <<<<<<<<<<<\n";
    for (std::set<int>::iterator it = close_io_fds_.begin();
         it != close_io_fds_.end(); ++it) {
      std::cerr << "[" << *it << "], ";
    }
    close_io_fds_.clear();
  }
}

// real socket
void HttpServer::HandleConnectionEvent(struct kevent& event) {
  if (event.flags & EV_EOF) {
    close(event.ident);
    for (IoFdMap::const_iterator it = io_fd_map_.begin();
         it != io_fd_map_.end(); ++it) {
      if (it->second == event.ident) {
        close(it->first);
        close_io_fds_.insert(it->first);
        io_fd_map_[it->first] = -1;
      }
    }

    std::cerr << "connection : " << event.ident << " will be resetted"
              << std::endl;
    connections_[event.ident].Reset();
  } else if (event.filter == EVFILT_READ) {
    ReceiveRequests(event.ident);
  } else if (event.filter == EVFILT_WRITE) {
    SendResponses(event.ident);
  }
}

// except socket
void HttpServer::HandleIOEvent(struct kevent& event) {
  struct kevent io_ev;
  int socket_fd = io_fd_map_[event.ident];
  if (event.flags & EV_EOF) {
    std::cerr << "EOF 다~~~~\n";
    ResponseManager::IoFdPair io_fds =
        connections_[socket_fd].FormatResponse(event.ident, event.filter);
    RegisterIoEvents(io_fds, connections_[socket_fd].get_fd());
    if (connections_[socket_fd].IsResponseBufferReady() == true) {
      UpdateKqueue(&io_ev, socket_fd, EVFILT_WRITE, EV_ADD | EV_ONESHOT);
    }
    if (event.ident != io_fds.input && event.ident != io_fds.output) {
      io_fd_map_.erase(event.ident);
    }
  } else if (event.filter == EVFILT_READ || event.filter == EVFILT_WRITE) {
    ResponseManager::IoFdPair io_fds =
        connections_[socket_fd].ExecuteMethod(event.ident);
    RegisterIoEvents(io_fds, connections_[socket_fd].get_fd());
    if (connections_[socket_fd].IsResponseBufferReady() == true) {
      UpdateKqueue(&io_ev, socket_fd, EVFILT_WRITE, EV_ADD | EV_ONESHOT);
    }
    if (event.ident != io_fds.input && event.ident != io_fds.output) {
      io_fd_map_.erase(event.ident);
    }
  }
}

// SECTION: private
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
  std::cerr << "accepted fd : " << fd << '\n';
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
  io_fd_map_[fd] = fd;
  int buf_size = SEND_BUFF_SIZE;
  setsockopt(fd, SOL_SOCKET, SO_SNDLOWAT, &buf_size, sizeof(int));
  // TODO 에러 핸들링
  struct kevent sock_ev;
  UpdateKqueue(&sock_ev, fd, EVFILT_READ, EV_ADD | EV_ONESHOT);
}

void HttpServer::ReceiveRequests(const int socket_fd) {
  struct kevent io_ev;
  Connection& connection = connections_[socket_fd];
  ResponseManager::IoFdPair io_fds = connection.HandleRequest();
  UpdateKqueue(&io_ev, socket_fd, EVFILT_READ, EV_ADD | EV_ONESHOT);
  if (connection.get_connection_status() == CONNECTION_ERROR) {
    close(connection.get_fd());
    connection.Reset();
    return;
  }
  if (connection.get_connection_status() == KEEP_READING) {
    return;
  }
  RegisterIoEvents(io_fds, socket_fd);
  if (connection.IsResponseBufferReady() == true) {
    UpdateKqueue(&io_ev, socket_fd, EVFILT_WRITE, EV_ADD | EV_ONESHOT);
  }
  if (connection.get_connection_status() == CONNECTION_ERROR) {
    close(connection.get_fd());
    connection.Reset();
    return;
  }
  while (connection.get_connection_status() == NEXT_REQUEST_EXISTS) {
    io_fds = connection.HandleRequest();
    RegisterIoEvents(io_fds, socket_fd);
  }
}

void HttpServer::SendResponses(int event_fd) {
  struct kevent sock_ev;
  Connection& connection = connections_[event_fd];
  if (connection.get_send_status() == KEEP_SENDING) {
    connection.Send();
    if (connection.get_connection_status() == CONNECTION_ERROR) {
      close(connection.get_fd());
      connection.Reset();
      return;
    }
    if (connection.IsResponseBufferReady() == true) {
      UpdateKqueue(&sock_ev, event_fd, EVFILT_WRITE, EV_ADD | EV_ONESHOT);
    }
  }
}

void HttpServer::RegisterIoEvents(ResponseManager::IoFdPair io_fds,
                                  int socket_fd) {
  struct kevent io_ev;
  if (io_fds.input != -1) {
    if (socket_fd > 0) {
      io_fd_map_[io_fds.input] = socket_fd;
    }
    UpdateKqueue(&io_ev, io_fds.input, EVFILT_READ, EV_ADD | EV_ONESHOT);
  }
  if (io_fds.output != -1) {
    if (socket_fd > 0) {
      io_fd_map_[io_fds.output] = socket_fd;
    }
    UpdateKqueue(&io_ev, io_fds.output, EVFILT_WRITE, EV_ADD | EV_ONESHOT);
  }
}
