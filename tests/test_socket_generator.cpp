#include <gtest/gtest.h>

#include "SocketGenerator.hpp"
#include "Validator.hpp"

#define SOCKET_PATH_PREFIX "../configs/tests/SocketGenerator/"

/**
 * Validator 에게 HostVector<port, host:port> 를 전달받아서
 * map<fd, host:port> 반환
 */

std::string FileToString(const std::string& file_path);

Validator::Result TestHostVectors(const std::string& case_id) {
  std::cout << case_id << std::endl;
  Validator validator(FileToString(SOCKET_PATH_PREFIX + case_id + ".config"));
  return validator.Validate();
}

TEST(SocketGeneratorTest, SingleServer) {
  {
    Validator::Result result = TestHostVectors("s_01");
    SocketGenerator sg;
    ListenerMap listeners = sg.Generate(result.host_vector);
    ListenerMap::iterator it = listeners.begin();
    sockaddr_in addr;
    socklen_t sock_len = it->second.size();
    memset(&addr, 0, sizeof(sockaddr_in));
    getsockname(it->first, (sockaddr*)&addr, &sock_len);
    EXPECT_EQ(it->second, std::string(inet_ntoa(addr.sin_addr)));
    close(it->first);
  }
}

TEST(SocketGeneratorTest, MultipleServers) {
  {
    Validator::Result result = TestHostVectors("s_02");
    SocketGenerator sg;
    ListenerMap listeners = sg.Generate(result.host_vector);
    EXPECT_EQ(2, listeners.size());
    sockaddr_in addr;
    for (ListenerMap::iterator it = listeners.begin(); it != listeners.end();
         ++it) {
      socklen_t sock_len = it->second.size();
      memset(&addr, 0, sizeof(sockaddr_in));
      getsockname(it->first, (sockaddr*)&addr, &sock_len);
      EXPECT_EQ(it->second, std::string(inet_ntoa(addr.sin_addr)));
      close(it->first);
    }
  }

  {
    Validator::Result result = TestHostVectors("s_03");
    SocketGenerator sg;
    ListenerMap listeners = sg.Generate(result.host_vector);
    EXPECT_EQ(8, listeners.size());
    sockaddr_in addr;
    for (ListenerMap::iterator it = listeners.begin(); it != listeners.end();
         ++it) {
      socklen_t sock_len = it->second.size();
      memset(&addr, 0, sizeof(sockaddr_in));
      getsockname(it->first, (sockaddr*)&addr, &sock_len);
      EXPECT_EQ(it->second, std::string(inet_ntoa(addr.sin_addr)));
      pause();
      close(it->first);
    }
  }

  // {
  //   Validator::Result result = TestHostVectors("s_04");
  //   SocketGenerator sg;
  //   ListenerMap listeners = sg.Generate(result.host_vector);
  //   EXPECT_EQ(3, listeners.size());
  //   sockaddr_in addr;
  //   for (ListenerMap::iterator it = listeners.begin(); it != listeners.end();
  //        ++it) {
  //     socklen_t sock_len = it->second.size();
  //     memset(&addr, 0, sizeof(sockaddr_in));
  //     getsockname(it->first, (sockaddr*)&addr, &sock_len);
  //     EXPECT_EQ(it->second, std::string(inet_ntoa(addr.sin_addr)));
  //     close(it->first);
  //   }
  // }
}
