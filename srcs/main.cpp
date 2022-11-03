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

std::string FileToString(const std::string& kFilePath) {
  std::ifstream ifs(kFilePath);
  if (!ifs.good()) {
    throw std::runtime_error("Config open failure");
  }
  std::stringstream ss;
  ifs >> ss.rdbuf();
  return ss.str();
}

#include <execinfo.h>

void stack_trace() {
  void** buffer = new void*[15];
  int count = backtrace(buffer, 15);
  backtrace_symbols_fd(buffer, count, 2);
  delete[] buffer;

  std::exception_ptr ptr = std::current_exception();
  try {
    std::rethrow_exception(ptr);
  } catch (std::exception& p) {
    std::cerr << p.what() << std::endl;
  }
}

int main(int argc, char* argv[]) {
  // std::set_terminate(stack_trace);
  if (argc < 2) {
    std::cerr
        << "BrilliantServer : Usage: ./BrilliantServer  [config file path]\n";
    return EXIT_FAILURE;
  }
  signal(SIGPIPE, SIG_IGN);
  signal(SIGCHLD, SIG_IGN);
  // redirect error to file
  // {
  //   int fd = open("error.log", O_WRONLY | O_CREAT | O_TRUNC, 0644);
  //   dup2(fd, 2);
  //   close(fd);
  // }

  std::string config_path = argv[1];
  size_t last_dot = config_path.rfind('.');
  if (config_path.size() < 8 || last_dot == std::string::npos ||
      config_path.compare(last_dot, 7, ".config") != 0) {
    std::cerr << "BrilliantServer : Usage: config file path must end with "
                 "'.config'\n";
    return EXIT_FAILURE;
  }

  try {
    Validator validator(FileToString(config_path));
    HttpServer(validator.Validate()).Run();
  } catch (const Validator::SyntaxErrorException& e) {
    std::cerr << "BrilliantServer : Validator : " << e.what() << '\n';
  } catch (std::exception& e) {
    std::cerr << "BrilliantServer : HttpServer : " << e.what() << '\n';
  } catch (...) {
    std::cerr << "BrilliantServer : Unknown error\n";
  }
  return EXIT_FAILURE;
}
