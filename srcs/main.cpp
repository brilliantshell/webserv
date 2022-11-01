/**
 * @file main.cpp
 * @author ghan, jiskim, yongjule
 * @brief main function of webserv
 * @date 2022-10-21
 *
 * @copyright Copyright (c) 2022
 */

#include "HttpServer.hpp"
#include "ResponseData.hpp"
#include "Validator.hpp"

StatusMap g_status_map;
MimeMap g_mime_map;

std::string FileToString(const std::string& file_path) {
  std::ifstream ifs(file_path);
  if (!ifs.good()) {
    throw std::runtime_error("Config open failure");
  }
  std::stringstream ss;
  ifs >> ss.rdbuf();
  return ss.str();
}

void sigpipe_handler(int signo) {
  std::cout << "\n\n\n\n\n\n\n>>>>>>>>>>>>SIGPIPE caught<<<<<<<<\n\n\n\n\n\n\n"
            << std::endl;
}

int main(int argc, char* argv[]) {
  if (argc < 2) {
    std::cerr << "BrilliantServer : Usage: ./webserv [config file path]\n";
    return EXIT_FAILURE;
  }
  signal(SIGPIPE, sigpipe_handler);
  signal(SIGCHLD, SIG_IGN);

  std::string config_path = argv[1];
  size_t last_dot = config_path.rfind('.');
  if (config_path.size() < 8 || last_dot == std::string::npos ||
      config_path.compare(last_dot, 7, ".config") != 0) {
    std::cerr << "BrilliantServer : Usage: config file path must end with "
                 "'.config'\n";
    return EXIT_FAILURE;
  }

  // try {
  Validator validator(FileToString(config_path));
  HttpServer(validator.Validate()).Run();
  // } catch (const std::exception& e) {
  //   std::cerr << "BrilliantServer : Validator : " << e.what() << '\n';
  // } catch (...) {
  //   std::cerr << "BrilliantServer : Unknown error\n";
  // }
  return EXIT_FAILURE;
}
