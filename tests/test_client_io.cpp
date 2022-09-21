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

  while (true) {
    int status = 0;
    int exit_status;
    pid_t pid = waitpid(-1, &status, 0);
    if (pid == -1) {
      if (pid == -1 && errno == ECHILD) {
        std::cerr << strerror(errno) << "\n";
        return EXIT_FAILURE;
      }
      break;
    }
    if (WIFEXITED(status)) {
      exit_status = WEXITSTATUS(status);
      std::cout << "port " << pid_port_map[pid] << " : "
                << exit_msg[exit_status] << "\n";
    }
  }
  return EXIT_SUCCESS;
}

int main(int argc, char* argv[]) {
  std::map<pid_t, uint16_t> pid_port_map;
  Validator validator(FileToString(argv[1]));
  const PortSet& port_set = validator.Validate().port_set;

  for (int i = 0; i < 10; ++i) {
    for (PortSet::const_iterator it = port_set.begin(); it != port_set.end();
         ++it) {
      pid_t pid = fork();
      if (pid == 0) {
        ClientConnection connection;
        connection.Connect(*it);
        connection.SendMessage();
        connection.ReceiveMessage();
        // sleep(1);

        // exit 해야함
      } else if (pid < 0) {
        std::cerr << "fork() failed" << std::endl;
        // break;
      }
      pid_port_map[pid] = *it;
    }
  }
  return WaitChildren(pid_port_map);
}
