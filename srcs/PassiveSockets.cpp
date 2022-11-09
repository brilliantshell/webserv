/**
 * @file PassiveSockets.cpp
 * @author ghan, jiskim, yongjule
 * @brief Open, bind passive sockets
 * @date 2022-09-24
 *
 * @copyright Copyright (c) 2022
 */

#include "PassiveSockets.hpp"

/**
 * @brief PassiveSockets 객체 생성
 *
 * @param kHostPortSet listen 할 호스트 + 포트
 */
PassiveSockets::PassiveSockets(const HostPortSet& kHostPortSet) {
  Listen(kHostPortSet);
}

/**
 * @brief PassiveSockets fd 정리 및 객체 소멸
 *
 */
PassiveSockets::~PassiveSockets(void) {
  for (ListenerMap::const_iterator it = begin(); it != end(); ++it) {
    close(it->first);
  }
}

// SECTION : private
/**
 * @brief 소켓 open 후 bind 하고 fd 와 호스트 + 포트를 매핑하여 저장
 *
 * @param kHostPortSet Config 에서 읽어온 listen 할 호스트 + 포트
 *
 */
void PassiveSockets::Listen(const HostPortSet& kHostPortSet) {
  for (HostPortSet::const_iterator it = kHostPortSet.begin();
       it != kHostPortSet.end(); ++it) {
    int fd = BindSocket(OpenSocket(it), *it);
    if (fd != -1) {
      insert(std::make_pair(fd, *it));
    }
  }
}

/**
 * @brief sockaddr_in 구조체 초기화
 *
 * @param kHostPort listen 할 호스트 + 포트
 * @param addr 초기화할 구조체 주소값
 */
void PassiveSockets::InitializeSockAddr(const HostPortPair& kHostPort,
                                        sockaddr_in* addr) {
  memset(addr, 0, sizeof(sockaddr_in));
  addr->sin_family = AF_INET;
  addr->sin_port = htons(kHostPort.port);
  addr->sin_addr.s_addr = kHostPort.host;
}

/**
 * @brief 소켓 열기
 *
 * @param kIt bind 할 호스트 + 포트
 * @return int fd, 에러 시 -1
 */
int PassiveSockets::OpenSocket(const HostPortSet::const_iterator& kIt) {
  errno = 0;
  int fd = socket(AF_INET, SOCK_STREAM, 6);
  if (fd < 0) {
    in_addr addr;
    addr.s_addr = kIt->host;
    PRINT_ERROR("socket for " << inet_ntoa(addr) << ':' << kIt->port
                              << " cannot be opened : " << strerror(errno));
  }
  return fd;
}

/**
 * @brief open 한 소켓을 포트에 bind, listen
 *
 * @param fd open 한 소켓 fd
 * @param kHostPort bind 할 호스트 + 포트
 * @return int fd, 에러 시 -1
 */
int PassiveSockets::BindSocket(int fd, const HostPortPair& kHostPort) {
  sockaddr_in addr;
  if (fd != -1) {
    InitializeSockAddr(kHostPort, &addr);
    int opt = 1;
    errno = 0;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
      PRINT_ERROR("address cannot be reused : " << strerror(errno));
      close(fd);
      return -1;
    }
    errno = 0;
    if (bind(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
      in_addr addr;
      addr.s_addr = kHostPort.host;
      PRINT_ERROR("socket for " << inet_ntoa(addr) << ':' << kHostPort.port
                                << " cannot be bound : " << strerror(errno));
      close(fd);
      return -1;
    }
    listen(fd, BACKLOG);
  }
  return fd;
}
