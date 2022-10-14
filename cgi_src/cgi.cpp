#include <iostream>

int main(int argc, char **argv, char **envp) {
  if (argc < 2) {
    std::cout << "Usage: " << argv[0]
              << " <Content-Type> [<Status> <Location>]\n";

    return 1;
  }
  std::cout << "Content-Type: " << argv[1] << "\n";
  if (argc > 2) {
    std::cout << "Status: " << argv[2] << "\n";
  }
  if (argc > 3) {
    std::cout << "Location: " << argv[3] << "\n";
  }
  std::cout << "\n";
  for (int i = 0; envp[i]; ++i)
    std::cout << "envp [" << i << "] : " << envp[i] << "\n";
  return 0;
}
