#include <sys/wait.h>

#include <fstream>
#include <map>
#include <sstream>
#include <string>

#include "ClientConnection.hpp"
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

int WaitChildren(std::map<pid_t, uint16_t>& pid_port_map) {
  const char* exit_msg[3] = {"✅", "❌", "⚠️"};
  int exit_status = EXIT_SUCCESS;

  while (true) {
    int child_exit_status;
    int status = 0;
    pid_t pid = waitpid(-1, &status, 0);
    if (pid == -1) {
      if (pid == -1 && errno != ECHILD) {
        std::cerr << strerror(errno) << "\n";
        return EXIT_FAILURE;
      }
      break;
    }
    if (WIFEXITED(status)) {
      child_exit_status = WEXITSTATUS(status);
      if (exit_status == EXIT_SUCCESS && child_exit_status != 0) {
        exit_status = EXIT_FAILURE;
      }
      std::cout << "port " << pid_port_map[pid] << " : "
                << exit_msg[child_exit_status] << "\n";
    }
  }
  return exit_status;
}

// generate random string
static std::string RandomString(size_t length) {
  auto randchar = []() -> char {
    const char charset[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";
    const size_t max_index = (sizeof(charset) - 1);
    return charset[rand() % max_index];
  };
  std::string str(length, 0);
  std::generate_n(str.begin(), length, randchar);
  return str;
}

int main(int argc, char* argv[]) {
  std::map<pid_t, uint16_t> pid_port_map;
  Validator validator(FileToString(argv[1]));
  const PortSet& port_set = validator.Validate().port_set;
  int connection_count = atoi(argv[2]);

  for (int i = 0; i < connection_count; ++i) {
    for (PortSet::const_iterator it = port_set.begin(); it != port_set.end();
         ++it) {
      pid_t pid = fork();
      if (pid == 0) {
        ClientConnection connection;
        connection.Connect(*it);
        std::string random_str = RandomString(4096);
        connection.SendMessage(random_str);
        std::string recv_msg = connection.ReceiveMessage();
        exit(random_str != recv_msg);
        // sleep(1);

        // exit 해야함
      } else if (pid < 0) {
        std::cerr << "errno : " << errno
                  << "fork() failed : " << strerror(errno) << std::endl;
        // break;
      }
      pid_port_map[pid] = *it;
    }
  }
  return WaitChildren(pid_port_map);
}
