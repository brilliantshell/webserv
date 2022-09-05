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
        FileToString(std::string(PATH_PREFIX) + std::string("case_one.conf")));
    ASSERT_EQ(1, 2);
  } catch (const std::exception& e) {
    EXPECT_STREQ(e.what(), "this config is invalid");
  }
}

TEST(ValidatorTest, DuplicateHostPortPair) {
  Validator validator;
  EXPECT_EQ("error.html",
            validator
                .Validate(FileToString(std::string(PATH_PREFIX) +
                                       std::string("case_two.conf")))
                .error);
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}