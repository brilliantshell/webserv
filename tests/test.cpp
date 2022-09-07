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
    std::cout << "CASE 00" << std::endl;
    Validator validator(FileToString(PATH_PREFIX "case_00.config"));
    try {
      validator.Validate();
      FAIL() << "CASE 00;";
    } catch (std::exception& e) {
      EXPECT_STREQ(e.what(), "syntax error") << "CASE 00";
    }
  }

  {
    std::cout << "CASE 01" << std::endl;
    Validator validator(FileToString(PATH_PREFIX "case_01.config"));
    Validator::ServerMap server_block_map = validator.Validate();
    Validator::ServerMap::iterator it = server_block_map.begin();
    ServerBlock server_block = it->first;

    EXPECT_EQ(server_block.port, 80);
    EXPECT_EQ(it->second.count("./"), 1) << "CASE 01";
  }

  {
    std::cout << "CASE 02" << std::endl;
    Validator validator(FileToString(PATH_PREFIX "case_02.config"));
    Validator::ServerMap server_block_map = validator.Validate();
    Validator::ServerMap::iterator it = server_block_map.begin();
    ServerBlock server_block = it->first;

    EXPECT_EQ(server_block.port, 8080);
    EXPECT_EQ(it->second.count("./trash"), 1) << "CASE 02";
  }

  {
    std::cout << "CASE 03" << std::endl;
    Validator validator(FileToString(PATH_PREFIX "case_03.config"));
    Validator::ServerMap server_block_map = validator.Validate();
    Validator::ServerMap::iterator it = server_block_map.begin();
    ServerBlock server_block = it->first;

    EXPECT_EQ(server_block.port, 4242);
    EXPECT_EQ(it->second.count("./hi"), 1) << "CASE 03";
  }

  {
    std::cout << "CASE 04" << std::endl;
    Validator validator(FileToString(PATH_PREFIX "case_04.config"));
    Validator::ServerMap server_block_map = validator.Validate();
    Validator::ServerMap::iterator it = server_block_map.begin();
    ServerBlock server_block = it->first;

    EXPECT_EQ(server_block.port, 4242);
    EXPECT_EQ(it->second.count("./return/42"), 1) << "CASE 04";
  }

  {
    std::cout << "CASE 05" << std::endl;
    Validator validator(FileToString(PATH_PREFIX "case_05.config"));
    try {
      validator.Validate();
      FAIL() << "CASE 05";
    } catch (std::exception& e) {
      EXPECT_STREQ(e.what(), "syntax error") << "CASE 05";
    }
  }

  {
    std::cout << "CASE 06" << std::endl;
    Validator validator(FileToString(PATH_PREFIX "case_06.config"));

    Validator::ServerMap server_block_map = validator.Validate();
    Validator::ServerMap::iterator it = server_block_map.begin();
    ServerBlock server_block = it->first;

    EXPECT_EQ(server_block.port, 80);
    EXPECT_EQ(it->second.count("./"), 1) << "CASE 06";
  }

  {
    std::cout << "CASE 07" << std::endl;
    Validator validator(FileToString(PATH_PREFIX "case_07.config"));
    Validator::ServerMap server_block_map = validator.Validate();
    Validator::ServerMap::iterator it = server_block_map.begin();
    ServerBlock server_block = it->first;

    EXPECT_EQ(server_block.port, 80);
    EXPECT_EQ(it->second.count("./"), 1) << "CASE 07";
  }

  {
    std::cout << "CASE 08" << std::endl;
    Validator validator(FileToString(PATH_PREFIX "case_08.config"));
    try {
      validator.Validate();
      FAIL() << "CASE 08";
    } catch (std::exception& e) {
      EXPECT_STREQ(e.what(), "syntax error") << "CASE 08";
    }
  }

  {
    std::cout << "CASE 09" << std::endl;
    Validator validator(FileToString(PATH_PREFIX "case_09.config"));
    try {
      validator.Validate();
      FAIL() << "CASE 09";
    } catch (std::exception& e) {
      EXPECT_STREQ(e.what(), "syntax error") << "CASE 09";
    }
  }

  {
    std::cout << "CASE 10" << std::endl;
    Validator validator(FileToString(PATH_PREFIX "case_10.config"));
    try {
      validator.Validate();
      FAIL() << "CASE 10";
    } catch (std::exception& e) {
      EXPECT_STREQ(e.what(), "syntax error") << "CASE 10";
    }
  }

  {
    std::cout << "CASE 11" << std::endl;
    Validator validator(FileToString(PATH_PREFIX "case_11.config"));
    try {
      validator.Validate();
      FAIL() << "CASE 11";
    } catch (std::exception& e) {
      EXPECT_STREQ(e.what(), "syntax error") << "CASE 11";
    }
  }

  {
    std::cout << "CASE 12" << std::endl;
    Validator validator(FileToString(PATH_PREFIX "case_12.config"));
    Validator::ServerMap server_block_map = validator.Validate();
    Validator::ServerMap::iterator it = server_block_map.begin();
    ServerBlock server_block = it->first;

    EXPECT_EQ(server_block.port, 80);
    EXPECT_EQ(server_block.server_name, "google.com");
    EXPECT_EQ(server_block.error, "404.html");
    EXPECT_EQ(it->second.count("./"), 1) << "CASE 12";
  }

  {
    std::cout << "CASE 13" << std::endl;
    Validator validator(FileToString(PATH_PREFIX "case_13.config"));
    try {
      validator.Validate();
      FAIL() << "CASE 13";
    } catch (std::exception& e) {
      EXPECT_STREQ(e.what(), "syntax error") << "CASE 13";
    }
  }

  {
    std::cout << "CASE 14" << std::endl;
    Validator validator(FileToString(PATH_PREFIX "case_14.config"));
    try {
      validator.Validate();
      FAIL() << "CASE 14";
    } catch (std::exception& e) {
      EXPECT_STREQ(e.what(), "syntax error") << "CASE 14";
    }
  }

  {
    std::cout << "CASE 15" << std::endl;
    Validator validator(FileToString(PATH_PREFIX "case_15.config"));
    try {
      validator.Validate();
      FAIL() << "CASE 15";
    } catch (std::exception& e) {
      EXPECT_STREQ(e.what(), "syntax error") << "CASE 15";
    }
  }

  {
    std::cout << "CASE 16" << std::endl;
    Validator validator(FileToString(PATH_PREFIX "case_16.config"));
    try {
      validator.Validate();
      FAIL() << "CASE 16";
    } catch (std::exception& e) {
      EXPECT_STREQ(e.what(), "syntax error") << "CASE 16";
    }
  }

  {
    std::cout << "CASE 17" << std::endl;
    Validator validator(FileToString(PATH_PREFIX "case_17.config"));
    try {
      validator.Validate();
      FAIL() << "CASE 17";
    } catch (std::exception& e) {
      EXPECT_STREQ(e.what(), "syntax error") << "CASE 17";
    }
  }

  {
    std::cout << "CASE 18" << std::endl;
    Validator validator(FileToString(PATH_PREFIX "case_18.config"));
    Validator::ServerMap server_block_map = validator.Validate();
    Validator::ServerMap::iterator it = server_block_map.begin();
    ServerBlock server_block = it->first;

    EXPECT_EQ(server_block.port, 8080);
    EXPECT_EQ(server_block.server_name, "127.0.0.1");
    EXPECT_EQ(server_block.error, "error.html");
    EXPECT_EQ(it->second.count("./default"), 1) << "CASE 18";
  }

  {
    std::cout << "CASE 19" << std::endl;
    Validator validator(FileToString(PATH_PREFIX "case_19.config"));
    try {
      validator.Validate();
      FAIL() << "CASE 19";
    } catch (std::exception& e) {
      EXPECT_STREQ(e.what(), "syntax error") << "CASE 19";
    }
  }

  {
    std::cout << "CASE 20" << std::endl;
    Validator validator(FileToString(PATH_PREFIX "case_20.config"));
    try {
      validator.Validate();
      FAIL() << "CASE 20";
    } catch (std::exception& e) {
      EXPECT_STREQ(e.what(), "syntax error") << "CASE 20";
    }
  }

  {
    std::cout << "CASE 21" << std::endl;
    Validator validator(FileToString(PATH_PREFIX "case_21.config"));
    Validator::ServerMap server_block_map = validator.Validate();
    Validator::ServerMap::iterator it = server_block_map.begin();
    ServerBlock server_block = it->first;

    EXPECT_EQ(server_block.port, 80);
    EXPECT_EQ(server_block.server_name, "127.0.0.1");
    EXPECT_EQ(server_block.error, "error.html");
    EXPECT_EQ(it->second.count("./"), 1) << "CASE 21";

    server_block = (++it)->first;
    EXPECT_EQ(server_block.port, 4242);
    EXPECT_EQ(server_block.server_name, "127.0.0.1");
    EXPECT_EQ(server_block.error, "error.html");
    EXPECT_EQ(it->second.count("./forty_two"), 1) << "CASE 21";
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
