#include <fcntl.h>

#include <fstream>
#include <iostream>
#include <string>

#include "HttpServer.hpp"
#include "PassiveSockets.hpp"
#include "Types.hpp"
#include "Validator.hpp"

std::string FileToString(const std::string& file_path) {
  std::ifstream ifs(file_path);
  if (!ifs.good()) {
    std::cerr << "오픈 실패란다~!!~!!!~!!\nOpen failure: " << file_path
              << std::endl;
  }
  std::stringstream ss;
  ifs >> ss.rdbuf();
  return ss.str();
}

int main(int argc, char* argv[]) {
  Validator validator(FileToString(argv[1]));
  Validator::Result ret = validator.Validate();

  HttpServer server(ret.port_set);
  server.Run();
  return EXIT_SUCCESS;
}
