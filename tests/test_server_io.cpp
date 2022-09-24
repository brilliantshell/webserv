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

#include "HttpServer.hpp"
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
  HttpServer server(listener_map);
  server.Run();
  return EXIT_SUCCESS;
}
