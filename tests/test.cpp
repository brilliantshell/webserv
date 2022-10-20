#include <chrono>

#include "ResponseData.hpp"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

StatusMap g_status_map;
MimeMap g_mime_map;

void printNow(void) {
  auto now = std::chrono::system_clock::now();
  auto in_time_t = std::chrono::system_clock::to_time_t(now);
  std::cout << "이번엔 다르다! ⏰ " << std::ctime(&in_time_t) << std::endl;
  std::string leak_str = "leaks " + std::to_string(getpid()) + " > /dev/null";
  system(leak_str.c_str());
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  ::testing::InitGoogleMock(&argc, argv);
  atexit(printNow);
  return RUN_ALL_TESTS();
}
