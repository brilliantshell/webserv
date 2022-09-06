#include <gtest/gtest.h>

#include <fstream>
#include <sstream>

#include "Validator.hpp"

#define PATH_PREFIX "../configs/tests/validator/"

std::string FileToString(const std::string& file_path) {
  std::ifstream ifs(file_path);
  std::stringstream ss;
  ifs >> ss.rdbuf();
  return ss.str();
}

TEST(ValidatorTest, BasicValidConfig) {
  Validator validator;
  try {
    validator.Validate(FileToString(PATH_PREFIX "case_00.config"));
    ASSERT_NE(1, 1);
  } catch (std::exception& e) {
    EXPECT_STREQ(e.what(), "syntax error") << "CASE 00";
  }

  ServerBlock server_block =
      validator.Validate(FileToString(PATH_PREFIX "case_01.config"));
  EXPECT_EQ(server_block.port, 80);
  EXPECT_EQ(server_block.route.path, "/") << "CASE 01";

  ServerBlock server_block2 =
      validator.Validate(FileToString(PATH_PREFIX "case_02.config"));
  EXPECT_EQ(server_block2.port, 8080);
  EXPECT_EQ(server_block2.route.path, "/trash") << "CASE 02";

  ServerBlock server_block3 =
      validator.Validate(FileToString(PATH_PREFIX "case_03.config"));
  EXPECT_EQ(server_block3.port, 4242);
  EXPECT_EQ(server_block3.route.path, "/hi") << "CASE 03";

  // EXPECT_EQ(server_block.host, "127.0.0.1");
  // EXPECT_EQ(server_block.error, "error.html");
  // EXPECT_EQ(server_block.route.index, "index.html");
  // EXPECT_EQ(server_block.route.autoindex, false);
  // EXPECT_EQ(server_block.route.method, "GET");
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
