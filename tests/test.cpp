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
      FAIL() << "CASE 00;";
    } catch (std::exception& e) {
      EXPECT_STREQ(e.what(), "syntax error") << "CASE 00";
    }
  }

  {
    Validator validator(FileToString(PATH_PREFIX "case_01.config"));
    ServerBlock server_block = validator.Validate();
    EXPECT_EQ(server_block.port, 80);
    EXPECT_EQ(server_block.route.path, "/") << "CASE 01";
  }

  {
    Validator validator(FileToString(PATH_PREFIX "case_02.config"));
    ServerBlock server_block2 = validator.Validate();
    EXPECT_EQ(server_block2.port, 8080);
    EXPECT_EQ(server_block2.route.path, "/trash") << "CASE 02";
  }

  {
    Validator validator(FileToString(PATH_PREFIX "case_03.config"));
    ServerBlock server_block3 = validator.Validate();
    EXPECT_EQ(server_block3.port, 4242);
    EXPECT_EQ(server_block3.route.path, "/hi") << "CASE 03";
  }

  {
    Validator validator(FileToString(PATH_PREFIX "case_04.config"));
    ServerBlock server_block4 = validator.Validate();
    EXPECT_EQ(server_block4.port, 4242);
    EXPECT_EQ(server_block4.route.path, "/return/42") << "CASE 04";
  }

  {
    Validator validator(FileToString(PATH_PREFIX "case_05.config"));
    try {
      validator.Validate();
      FAIL() << "CASE 05";
    } catch (std::exception& e) {
      EXPECT_STREQ(e.what(), "syntax error") << "CASE 05";
    }
  }

  {
    Validator validator(FileToString(PATH_PREFIX "case_06.config"));
    ServerBlock server_block4 = validator.Validate();
    EXPECT_EQ(server_block4.port, 80);
    EXPECT_EQ(server_block4.route.path, "/") << "CASE 06";
  }

  {
    Validator validator(FileToString(PATH_PREFIX "case_07.config"));
    ServerBlock server_block4 = validator.Validate();
    EXPECT_EQ(server_block4.port, 80);
    EXPECT_EQ(server_block4.route.path, "/") << "CASE 07";
  }

  {
    Validator validator(FileToString(PATH_PREFIX "case_08.config"));
    try {
      validator.Validate();
      FAIL() << "CASE 08";
    } catch (std::exception& e) {
      EXPECT_STREQ(e.what(), "syntax error") << "CASE 08";
    }
  }

  {
    Validator validator(FileToString(PATH_PREFIX "case_09.config"));
    try {
      validator.Validate();
      FAIL() << "CASE 09";
    } catch (std::exception& e) {
      EXPECT_STREQ(e.what(), "syntax error") << "CASE 09";
    }
  }

  {
    Validator validator(FileToString(PATH_PREFIX "case_10.config"));
    try {
      validator.Validate();
      FAIL() << "CASE 10";
    } catch (std::exception& e) {
      EXPECT_STREQ(e.what(), "syntax error") << "CASE 10";
    }
  }

  {
    Validator validator(FileToString(PATH_PREFIX "case_11.config"));
    try {
      validator.Validate();
      FAIL() << "CASE 11";
    } catch (std::exception& e) {
      EXPECT_STREQ(e.what(), "syntax error") << "CASE 11";
    }
  }

  {
    Validator validator(FileToString(PATH_PREFIX "case_12.config"));
    ServerBlock server_block4 = validator.Validate();
    EXPECT_EQ(server_block4.port, 80);
    EXPECT_EQ(server_block4.server_name, "google.com");
    EXPECT_EQ(server_block4.error, "404.html");
    EXPECT_EQ(server_block4.route.path, "/") << "CASE 12";
  }

  {
    Validator validator(FileToString(PATH_PREFIX "case_13.config"));
    try {
      validator.Validate();
      FAIL() << "CASE 13";
    } catch (std::exception& e) {
      EXPECT_STREQ(e.what(), "syntax error") << "CASE 13";
    }
  }

  {
    Validator validator(FileToString(PATH_PREFIX "case_14.config"));
    try {
      validator.Validate();
      FAIL() << "CASE 14";
    } catch (std::exception& e) {
      EXPECT_STREQ(e.what(), "syntax error") << "CASE 14";
    }
  }

  {
    Validator validator(FileToString(PATH_PREFIX "case_15.config"));
    try {
      validator.Validate();
      FAIL() << "CASE 15";
    } catch (std::exception& e) {
      EXPECT_STREQ(e.what(), "syntax error") << "CASE 15";
    }
  }

  {
    Validator validator(FileToString(PATH_PREFIX "case_16.config"));
    try {
      validator.Validate();
      FAIL() << "CASE 16";
    } catch (std::exception& e) {
      EXPECT_STREQ(e.what(), "syntax error") << "CASE 16";
    }
  }

  {
    Validator validator(FileToString(PATH_PREFIX "case_17.config"));
    try {
      validator.Validate();
      FAIL() << "CASE 17";
    } catch (std::exception& e) {
      EXPECT_STREQ(e.what(), "syntax error") << "CASE 17";
    }
  }

  {
    Validator validator(FileToString(PATH_PREFIX "case_18.config"));
    ServerBlock server_block4 = validator.Validate();
    EXPECT_EQ(server_block4.port, 8080);
    EXPECT_EQ(server_block4.server_name, "127.0.0.1");
    EXPECT_EQ(server_block4.error, "error.html");
    EXPECT_EQ(server_block4.route.path, "/default") << "CASE 18";
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
