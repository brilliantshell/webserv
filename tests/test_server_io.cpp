#include <fcntl.h>

#include <fstream>
#include <iostream>
#include <string>

#include "HttpServer.hpp"
#include "PassiveSockets.hpp"
#include "ResponseData.hpp"
#include "Utils.hpp"
#include "Validator.hpp"

StatusMap g_status_map;
MimeMap g_mime_map;

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
  if (argc != 2) {
    std::cerr << "Usage: ./server [config_file_path]\n";
    return 1;
  }
  Validator validator(FileToString(argv[1]));
  ServerConfig ret = validator.Validate();

  HttpServer server(ret.host_port_set);
  server.Run();
  return EXIT_SUCCESS;
}
