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
  std::cout << "cwd : " << getcwd(NULL, 0);
  return 0;
}
