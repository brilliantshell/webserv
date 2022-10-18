#include <unistd.h>

#include <iostream>
#include <sstream>
#include <string>

int main(int argc, char **argv, char **envp) {
  if ((argc - 1) % 3 == 0) {
    for (size_t i = 1; i < argc; i += 3) {
      std::cout << argv[i] << ": " << argv[i + 1]
                << ((strcmp(argv[i + 2], "0") == 0)
                        ? std::string("")
                        : (" " + std::string(argv[i + 2])))
                << "\n";
    }
  }
  std::cout << "\n";
  std::stringstream ss(getenv("CONTENT_LENGTH"));
  size_t len = 0;
  ss >> len;
  if (len > 0) {
    char buf[2049];
    memset(buf, 0, 2049);
    while (len > 0) {
      ssize_t read_size = read(0, buf, 2048);
      if (read_size < 0) {
        break;
      }
      std::cout << buf;
      len -= read_size;
      memset(buf, 0, 2049);
    }
  }
  return 0;
}
