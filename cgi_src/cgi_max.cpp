#include <unistd.h>

#include <exception>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#define CGI_MAX 134217728  // 2 ^ 27

#define GOINFRE_PATH "/Users/jiskim/goinfre/"

int main(int argc, char **argv, char **envp) {
  try {
    std::string str;
    std::string file_name(getenv("QUERY_STRING"));
    file_name.erase(0, 1);
    std::ifstream ifs(GOINFRE_PATH + file_name);
    if (ifs.bad()) std::cerr << "ifs error : " << strerror(errno) << std::endl;
    std::stringstream ss;
    ss << ifs.rdbuf();
    str = ss.str();
    for (ssize_t i = 0; i < str.size();) {
      i += write(STDOUT_FILENO, str.c_str() + i, 2048);
    }
  } catch (const std::exception &e) {
    std::cerr << "exception : " << e.what() << std::endl;
  } catch (...) {
    std::cerr << "another err\n";
  }
  return 0;
}
