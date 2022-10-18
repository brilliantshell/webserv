#include <unistd.h>

#include <iostream>
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
  for (int i = 0; envp[i]; ++i)
    std::cout << "envp [" << i << "] : " << envp[i] << "\n";
  ssize_t read_bytes;
  char buf[1024];
  std::cout << "\n";
  while ((read_bytes = read(0, buf, sizeof(buf))) > 0) {
    std::cout.write(buf, read_bytes);
  }
  if (argc > 1) {
    std::cout << "\n";
    for (size_t i = 0; i < argc - 1; ++i) {
      std::cout << argv[i + 1] << ((i + 2 == argc) ? "" : "+");
    }
  }
  return 0;
}
