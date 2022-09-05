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

TEST(ValidatorTest, NoServerBlock) {
  Validator validator;
  try {
    validator.Validate(
        FileToString(std::string(PATH_PREFIX) + std::string("case_01.conf")));
    ASSERT_EQ(1, 2);
  } catch (const std::exception& e) {
    EXPECT_STREQ(e.what(), "syntax error");
  }
}

// TEST(ValidatorTest, DuplicateHostPortPair) {
//   Validator validator;
//   EXPECT_EQ("error.html",
//             validator
//                 .Validate(FileToString(std::string(PATH_PREFIX) +
//                                        std::string("case_02.conf")))
//                 .error);
// }

TEST(ValidatorTest, InvalidServerBracket) {
  Validator validator;
  try {
    validator.Validate(
        FileToString(std::string(PATH_PREFIX) + std::string("case_12.0.conf")));
    ASSERT_NE(0, 0) << "CASE 12.0";
  } catch (const std::exception& e) {
    EXPECT_STREQ(e.what(), "syntax error");
  }
  try {
    validator.Validate(
        FileToString(std::string(PATH_PREFIX) + std::string("case_12.1.conf")));
    ASSERT_NE(1, 1) << "CASE 12.1";
  } catch (const std::exception& e) {
    EXPECT_STREQ(e.what(), "syntax error");
  }
  try {
    validator.Validate(
        FileToString(std::string(PATH_PREFIX) + std::string("case_12.2.conf")));
    ASSERT_NE(2, 2) << "CASE 12.2";
  } catch (const std::exception& e) {
    EXPECT_STREQ(e.what(), "syntax error");
  }
  try {
    validator.Validate(
        FileToString(std::string(PATH_PREFIX) + std::string("case_12.3.conf")));
    ASSERT_NE(3, 3) << "CASE 12.3";
  } catch (const std::exception& e) {
    EXPECT_STREQ(e.what(), "syntax error");
  }
  try {
    validator.Validate(
        FileToString(std::string(PATH_PREFIX) + std::string("case_12.4.conf")));
    ASSERT_NE(4, 4) << "CASE 12.4";
  } catch (const std::exception& e) {
    EXPECT_STREQ(e.what(), "syntax error");
  }

  {
    ServerBlock server_block = validator.Validate(
        FileToString(std::string(PATH_PREFIX) + std::string("case_12.5.conf")));
    EXPECT_EQ(std::string(), server_block.host) << "CASE 12.5";
    EXPECT_EQ(int(), server_block.port);
    EXPECT_EQ(std::string(), server_block.error);
  }

  {
    ServerBlock server_block = validator.Validate(
        FileToString(std::string(PATH_PREFIX) + std::string("case_12.6.conf")));
    EXPECT_EQ(std::string(), server_block.host) << "CASE 12.6";
    EXPECT_EQ(int(), server_block.port);
    EXPECT_EQ(std::string(), server_block.error);
  }

  // try {
  //   validator.Validate(
  //       FileToString(std::string(PATH_PREFIX) +
  //       std::string("case_12.7.conf")));
  //   ASSERT_NE(2, 2) << "CASE 12.7";
  // } catch (const std::exception& e) {
  //   EXPECT_STREQ(e.what(), "syntax error");
  // }
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
