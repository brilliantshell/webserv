#include <gtest/gtest.h>

#include <chrono>

#include "ResponseData.hpp"

StatusMap g_status_map;
MimeMap g_mime_map;

void printNow(void) {
  auto now = std::chrono::system_clock::now();
  auto in_time_t = std::chrono::system_clock::to_time_t(now);
  std::cout << "이번엔 다르다! ⏰ " << std::ctime(&in_time_t) << std::endl;
  system("leaks test_webserv");
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  atexit(printNow);
  return RUN_ALL_TESTS();
}
