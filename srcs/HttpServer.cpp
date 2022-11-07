/**
 * @file HttpServer.cpp
 * @author ghan, jiskim, yongjule
 * @brief Accept connections, receive requests, send responses
 * @date 2022-09-21
 *
 * @copyright Copyright (c) 2022
 */

#include "HttpServer.hpp"

#define PRINT_EVENT(event)                                                \
  std::err << "event: ident: [" << event.ident                            \
           << "], filter(RD: -1, WR: -2) : [" << event.filter             \
           << "], flags : [" << std::hex << event.flags << "], fflags: [" \
           << std::dec << event.fflags << "], data: [" << event.data      \
           << "], udata: [" << event.udata << "]" << std::endl;

/**
 * @brief HttpServer 객체 생성, Connection vector 사이즈 max fd 개수로 설정
 *
 * @param kConfig 서버 설정값 구조체
 */
HttpServer::HttpServer(const ServerConfig& kConfig)
    : port_map_(kConfig.port_map),
      passive_sockets_(PassiveSockets(kConfig.port_set)) {
  struct rlimit fd_limit;
  getrlimit(RLIMIT_NOFILE, &fd_limit);
  connections_.resize(fd_limit.rlim_cur);
}

/**
 * @brief HttpServer 객체 소멸, kqueue 자원 정리
 *
 */
HttpServer::~HttpServer() { close(kq_); }

/**
 * @brief 서버 실행
 * kqueue 에 쌓인 이벤트를 종류 (소켓, file/PIPE I/O, timer)에 따라 처리
 *
 */
void HttpServer::Run(void) {
  InitKqueue();
  struct kevent events[MAX_EVENTS];

  while (true) {
    int number_of_events = kevent(kq_, NULL, 0, events, MAX_EVENTS, NULL);
    if (number_of_events == -1) {
      PRINT_ERROR("HttpServer : kevent failed : " << strerror(errno));
      for (ConnectionVector::iterator it = connections_.begin();
           it != connections_.end(); ++it) {
        ClearConnectionResources(it->get_fd());
      }
      sleep(1);
      continue;
    }
    for (int i = 0; i < number_of_events; ++i) {
      if (events[i].filter == EVFILT_TIMER) {
        ClearConnectionResources(events[i].ident);
      } else {
        if (passive_sockets_.count(events[i].ident) == 1) {
          AcceptConnection(events[i].ident);
        } else if (close_io_fds_.count(events[i].ident) == 0) {
          (io_fd_map_.count(events[i].ident) == 1)
              ? HandleIOEvent(events[i])
              : HandleConnectionEvent(events[i]);
        }
      }
    }
    close_io_fds_.clear();
  }
}

/**
 * @brief Connection 소켓 fd 에 발생한 recv/sen 이벤트 처리
 *
 * @param event 이벤트 구조체 (struct kevent)
 */
void HttpServer::HandleConnectionEvent(struct kevent& event) {
  int socket_fd = event.ident;
  if (event.flags & EV_EOF && event.filter == EVFILT_READ) {
    return ClearConnectionResources(socket_fd);
  }
  if (event.filter == EVFILT_READ) {
    ReceiveRequests(socket_fd);
  } else if (event.filter == EVFILT_WRITE) {
    SendResponses(socket_fd);
  }
  int connection_status = connections_[socket_fd].get_connection_status();
  if (connection_status == CONNECTION_ERROR) {
    return ClearConnectionResources(socket_fd);
  }
  UpdateTimerEvent(socket_fd, EV_ADD | EV_ONESHOT,
                   (connection_status == KEEP_READING) ? REQUEST_TIMEOUT
                                                       : CONNECTION_TIMEOUT);
}

/**
 * @brief File/PIPE I/O fd 에 발생한 이벤트 처리
 *
 * @param event 이벤트 구조체 (struct kevent)
 */
void HttpServer::HandleIOEvent(struct kevent& event) {
  int event_fd = static_cast<int>(event.ident);
  int socket_fd = io_fd_map_[event_fd];
  ResponseManager::IoFdPair io_fds =
      connections_[socket_fd].ExecuteMethod(event_fd);
  if (connections_[socket_fd].get_connection_status() == CONNECTION_ERROR) {
    return ClearConnectionResources(socket_fd);
  }
  RegisterIoEvents(io_fds, socket_fd);
  if (connections_[socket_fd].IsResponseBufferReady() == true) {
    UpdateKqueue(socket_fd, EVFILT_WRITE, EV_ADD | EV_ONESHOT);
  }
  if (event_fd != io_fds.input && event_fd != io_fds.output) {
    io_fd_map_.erase(event_fd);
  }
}

// SECTION : private
/**
 * @brief kqueue 생성 및 passive socket 등록
 *
 */
void HttpServer::InitKqueue(void) {
  kq_ = kqueue();
  if (kq_ == -1) {
    PRINT_ERROR("HttpServer : kqueue failed : " << strerror(errno));
    exit(EXIT_FAILURE);
  }
  struct kevent* sock_ev =
      new (std::nothrow) struct kevent[passive_sockets_.size()];
  if (sock_ev == NULL) {
    PRINT_ERROR("HttpServer : failed to allocate memory");
    exit(EXIT_FAILURE);
  }
  int i = 0;
  for (ListenerMap::const_iterator it = passive_sockets_.begin();
       it != passive_sockets_.end(); ++it) {
    EV_SET(&sock_ev[i++], it->first, EVFILT_READ, EV_ADD, 0, 0, NULL);
    PRINT_OUT("HttpServer : passive socket fd : " << it->first << " for "
                                                  << it->second);
  }
  if (kevent(kq_, sock_ev, passive_sockets_.size(), NULL, 0, NULL) == -1) {
    PRINT_ERROR("HttpServer : failed to listen : " << strerror(errno));
    exit(EXIT_FAILURE);
  }
  delete[] sock_ev;
}

/**
 * @brief I/O 이벤트 등록
 *
 * @param socket_fd 발생한 이벤트 fd
 * @param ev_filt 이벤트 종류
 * @param ev_flag 이벤트 플래그
 */
void HttpServer::UpdateKqueue(int socket_fd, int16_t ev_filt,
                              uint16_t ev_flag) {
  struct kevent sock_ev;
  EV_SET(&sock_ev, socket_fd, ev_filt, ev_flag, 0, 0, NULL);
  if (kevent(kq_, &sock_ev, 1, NULL, 0, NULL) == -1) {
    PRINT_ERROR(
        "HttpServer : failed to update an I/O event : " << strerror(errno));
  }
}

/**
 * @brief timer 이벤트 등록
 *
 * @param id timer id
 * @param ev_filt 이벤트 종류
 * @param data timeout 주기
 */
void HttpServer::UpdateTimerEvent(int id, uint16_t ev_filt, intptr_t data) {
  struct kevent timer_ev;
  EV_SET(&timer_ev, id, EVFILT_TIMER, ev_filt, NOTE_SECONDS, data, NULL);
  if (kevent(kq_, &timer_ev, 1, NULL, 0, NULL) == -1) {
    if (ev_filt != EV_DELETE) {
      PRINT_ERROR("HttpServer : failed to update connection timeout : "
                  << strerror(errno));
      ClearConnectionResources(id);
    }
  }
}

/**
 * @brief 연결 요청 허가, Connection 객체 초기화
 *
 * @param socket_fd 연결 요청이 발생한 passive 소켓 fd
 */
void HttpServer::AcceptConnection(int socket_fd) {
  sockaddr_in addr;
  socklen_t addr_len = sizeof(addr);
  int fd = accept(socket_fd, reinterpret_cast<sockaddr*>(&addr), &addr_len);
  if (fd == -1) {
    PRINT_ERROR("HttpServer : failed to accept request via "
                << passive_sockets_[socket_fd] << " : " << strerror(errno));
    return;
  }
  fcntl(fd, F_SETFL, O_NONBLOCK);
  int buf_size = SEND_BUFF_SIZE + SEND_BUFF_SIZE / 2;
  if (setsockopt(fd, SOL_SOCKET, SO_SNDLOWAT, &buf_size, sizeof(int)) == -1) {
    PRINT_ERROR(
        "HttpServer: setting send low watermark failed : " << strerror(errno));
    close(fd);
    return;
  }
  const uint16_t kPort = passive_sockets_[socket_fd];
  connections_[fd].SetAttributes(fd, inet_ntoa(addr.sin_addr), kPort,
                                 port_map_[kPort]);
  if (connections_[fd].get_connection_status() == CONNECTION_ERROR) {
    PRINT_ERROR("HttpServer : connection attributes set up failed : "
                << strerror(errno));
    return connections_[fd].Clear();
  }
  UpdateKqueue(fd, EVFILT_READ, EV_ADD | EV_ONESHOT);
  UpdateTimerEvent(fd, EV_ADD | EV_ONESHOT, CONNECTION_TIMEOUT);
}

/**
 * @brief Connection 소켓 fd 에 요청이 입력됐을 때 처리 및 소켓 I/O 이벤트 등록
 *
 * @param kSocketFd 요청이 발생한 소켓 fd
 */
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

/**
 * @brief Connection 소켓 fd 에 응답 송신이 준비 됐을 때 출력 및
 * 소켓 I/O 이벤트 등록
 *
 * @param socket_fd 요청 처리 중인 Connection 소켓 fd
 */
void HttpServer::SendResponses(int socket_fd) {
  Connection& connection = connections_[socket_fd];
  if (connection.get_send_status() < SEND_FINISHED) {
    connection.Send();
    if (connection.get_connection_status() == CONNECTION_ERROR) {
      return;
    }
    if (connection.get_connection_status() == CLOSE &&
        connection.get_send_status() > KEEP_SENDING &&
        connection.IsHttpPairSynced() == true) {
      shutdown(socket_fd, SHUT_WR);
      return;
    }
    if (connection.IsResponseBufferReady() == true) {
      UpdateKqueue(socket_fd, EVFILT_WRITE, EV_ADD | EV_ONESHOT);
    }
  }
}

/**
 * @brief File/PIPE I/O 이벤트 발생 시 kqueue 에 기대되는 이벤트 등록 및
 * 자원 fd & Connection 소켓 fd 매핑
 *
 * @param io_fds File/PIPE I/O 이벤트 중인 fd 담는 pair
 * @param kSocketFd 요청 처리 중인 Connection 소켓 fd
 */
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

/**
 * @brief Client 와 연결 해제 시 자원 정리
 *
 * @param socket_fd 연결 해제 된 소켓 fd
 */
void HttpServer::ClearConnectionResources(int socket_fd) {
  if (socket_fd == -1) {
    return;
  }
  close(socket_fd);
  for (IoFdMap::const_iterator it = io_fd_map_.begin();
       it != io_fd_map_.end();) {
    IoFdMap::const_iterator io_fds_node = it;
    ++it;
    if (io_fds_node->second == socket_fd) {
      close(io_fds_node->first);
      close_io_fds_.insert(io_fds_node->first);
      io_fd_map_.erase(io_fds_node->first);
    }
  }
  connections_[socket_fd].Clear();
  UpdateTimerEvent(socket_fd, EV_DELETE, 0);
}
