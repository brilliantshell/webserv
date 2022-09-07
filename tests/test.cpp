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
  {
    Validator validator(FileToString(PATH_PREFIX "case_00.config"));
    try {
      validator.Validate();
      ASSERT_NE(1, 1);
    } catch (std::exception& e) {
      EXPECT_STREQ(e.what(), "syntax error") << "CASE 00";
    }
  }

  {
    Validator validator(FileToString(PATH_PREFIX "case_01.config"));
    std::cout << "CASE 01\n";
    ServerBlock server_block = validator.Validate();
    EXPECT_EQ(server_block.port, 80);
    EXPECT_EQ(server_block.route.path, "/") << "CASE 01";
  }

  {
    Validator validator(FileToString(PATH_PREFIX "case_02.config"));
    std::cout << "CASE 02\n";
    ServerBlock server_block2 = validator.Validate();
    EXPECT_EQ(server_block2.port, 8080);
    EXPECT_EQ(server_block2.route.path, "/trash") << "CASE 02";
  }

  {
    Validator validator(FileToString(PATH_PREFIX "case_03.config"));
    std::cout << "CASE 03\n";
    ServerBlock server_block3 = validator.Validate();
    EXPECT_EQ(server_block3.port, 4242);
    EXPECT_EQ(server_block3.route.path, "/hi") << "CASE 03";
  }

  {
    Validator validator(FileToString(PATH_PREFIX "case_04.config"));
    std::cout << "CASE 04\n";
    ServerBlock server_block4 = validator.Validate();
    EXPECT_EQ(server_block4.port, 4242);
    EXPECT_EQ(server_block4.route.path, "/return/42") << "CASE 04";
  }
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
