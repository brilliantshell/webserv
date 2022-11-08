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
 * @param kPortSet listen 할 포트 번호들
 */
PassiveSockets::PassiveSockets(const PortSet& kPortSet) { Listen(kPortSet); }

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
 * @brief 소켓 open 후 bind 하고 fd 와 포트 번호를 매핑하여 저장
 *
 * @param kPortSet Config 에서 읽어온 listen 할 포트 번호들
 *
 */
void PassiveSockets::Listen(const PortSet& kPortSet) {
  for (PortSet::const_iterator it = kPortSet.begin(); it != kPortSet.end();
       ++it) {
    int fd = BindSocket(OpenSocket(*it), *it);
    if (fd != -1) {
      insert(std::make_pair(fd, *it));
    }
  }
}

/**
 * @brief sockaddr_in 구조체 초기화
 *
 * @param kPort listen 할 포트 번호
 * @param addr 초기화할 구조체 주소값
 */
void PassiveSockets::InitializeSockAddr(const uint16_t kPort,
                                        sockaddr_in* addr) {
  memset(addr, 0, sizeof(sockaddr_in));
  addr->sin_family = AF_INET;
  addr->sin_port = htons(kPort);
  addr->sin_addr.s_addr = INADDR_ANY;
}

/**
 * @brief 소켓 열기
 *
 * @param kPort bind 할 포트 번호
 * @return int fd, 에러 시 -1
 */
int PassiveSockets::OpenSocket(const uint16_t kPort) {
  errno = 0;
  int fd = socket(AF_INET, SOCK_STREAM, 6);
  if (fd < 0) {
    PRINT_ERROR("socket for " << kPort
                              << " cannot be opened : " << strerror(errno));
  }
  return fd;
}

/**
 * @brief open 한 소켓을 포트에 bind, listen
 *
 * @param fd open 한 소켓 fd
 * @param kPort bind 할 포트 번호
 * @return int fd, 에러 시 -1
 */
int PassiveSockets::BindSocket(int fd, const uint16_t kPort) {
  sockaddr_in addr;
  if (fd != -1) {
    InitializeSockAddr(kPort, &addr);
    int opt = 1;
    errno = 0;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
      PRINT_ERROR("address cannot be reused : " << strerror(errno));
      close(fd);
      return -1;
    }
    errno = 0;
    if (bind(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
      PRINT_ERROR("socket for " << kPort
                                << " cannot be bound : " << strerror(errno));
      close(fd);
      return -1;
    }
    listen(fd, BACKLOG);
  }
  return fd;
}
