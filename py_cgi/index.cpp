#include <unistd.h>

#include <csignal>
#include <iostream>
#include <map>
#include <sstream>
#include <string>

int main(int argc, char **argv, char **envp) {
  std::cout << "Content-type: text/html;charset=utf-8\r\n\r\n";
  std::stringstream ss(getenv("QUERY_STRING"));
  std::string key, value;
  std::map<std::string, std::string> q;
  while (getline(ss, key, '=') && getline(ss, value, '&')) {
    q[key] = value;
  }
  std::cout << "<html><head><title>CGI</title></head><body><h1>Welcome To "
               "BrilliantServer</h1>Hello, "
            << "first name" << q["fistname"] << "\n\nlastname" << q["lastname"]
            << "!</body></html>";

  close(1);
  close(0);

  alarm(30);

  return 0;
}
