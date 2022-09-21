/**
 * @file test_server_io.cpp
 * @author ghan, jiskim, yongjule
 * @brief
 * @date 2022-09-21
 *
 * @copyright Copyright (c) 2022
 */

#include <fcntl.h>

#include <fstream>
#include <iostream>
#include <string>

#include "SocketGenerator.hpp"
#include "Types.hpp"
#include "Validator.hpp"

std::string FileToString(const std::string& file_path) {
  std::ifstream ifs(file_path);
  if (!ifs.good()) {
    std::cerr << "Open failure: " << file_path << std::endl;
  }
  std::stringstream ss;
  ifs >> ss.rdbuf();
  return ss.str();
}

int main(int argc, char* argv[]) {
  Validator validator(FileToString(argv[1]));
  Validator::Result ret = validator.Validate();

  ListenerMap listener_map = socket_generator::GenerateSocket(ret.port_set);
  for (ListenerMap::iterator it = listener_map.begin();
       it != listener_map.end(); ++it) {
    sockaddr_in client_addr;
    memset(&client_addr, 0, sizeof(client_addr));
    int client_addr_len = sizeof(client_addr);
    int client_fd = accept(it->first, (sockaddr*)&client_addr,
                           (socklen_t*)&client_addr_len);
    if (client_fd < 0) {
      std::cerr << "server: " << strerror(errno) << "accept failed"
                << std::endl;
      close(it->first);
      continue;
    }
    char buf[4097];
    recv(client_fd, buf, sizeof(buf), 0);
    send(client_fd, buf, sizeof(buf), 0);
    recv(client_fd, buf, sizeof(buf), 0);
    close(client_fd);
  }
  for (ListenerMap::iterator it = listener_map.begin();
       it != listener_map.end(); ++it) {
    close(it->first);
  }
  return EXIT_SUCCESS;
}
