#include <gtest/gtest.h>

#include "PassiveSockets.hpp"
#include "Validator.hpp"

#define SOCKET_PATH_PREFIX "../configs/tests/SocketGenerator/"

/**
 * Validator 에게 HostVector<port, host:port> 를 전달받아서
 * map<fd, port> 반환
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
    PassiveSockets passive_sockets(result.port_set);
    ListenerMap::iterator it = passive_sockets.begin();
    sockaddr_in addr;
    socklen_t len = it->second;
    memset(&addr, 0, sizeof(sockaddr_in));
    EXPECT_EQ(getsockname(it->first, (sockaddr*)&addr, &len), 0);
    close(it->first);
  }
}

TEST(SocketGeneratorTest, MultipleServers) {
  {
    Validator::Result result = TestHostVectors("s_02");
    PassiveSockets passive_sockets(result.port_set);
    EXPECT_EQ(2, passive_sockets.size());
    sockaddr_in addr;
    for (ListenerMap::iterator it = passive_sockets.begin();
         it != passive_sockets.end(); ++it) {
      socklen_t len = it->second;
      memset(&addr, 0, sizeof(sockaddr_in));
      EXPECT_EQ(getsockname(it->first, (sockaddr*)&addr, &len), 0);
      close(it->first);
    }
  }

  {
    Validator::Result result = TestHostVectors("s_03");
    PassiveSockets passive_sockets(result.port_set);
    EXPECT_EQ(8, passive_sockets.size());
    sockaddr_in addr;
    for (ListenerMap::iterator it = passive_sockets.begin();
         it != passive_sockets.end(); ++it) {
      socklen_t len = it->second;
      memset(&addr, 0, sizeof(sockaddr_in));
      EXPECT_EQ(getsockname(it->first, (sockaddr*)&addr, &len), 0);
      close(it->first);
    }
  }
}
