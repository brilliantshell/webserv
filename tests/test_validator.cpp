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

void TestSyntaxException(const std::string& file_number) {
  std::cout << file_number << std::endl;
  Validator validator(FileToString(PATH_PREFIX + file_number + ".config"));
  EXPECT_THROW(validator.Validate(), Validator::SyntaxErrorException);
}

Validator::ServerMap TestValidatorSuccess(const std::string& file_number) {
  std::cout << file_number << std::endl;
  Validator validator(FileToString(PATH_PREFIX + file_number + ".config"));
  return validator.Validate();
}

TEST(ValidatorTest, BasicValidConfig) {
  TestSyntaxException("CASE_00");

  {
    Validator::ServerMap server_map = TestValidatorSuccess("CASE_01");
    Validator::ServerMap::iterator it = server_map.begin();
    ServerBlock server_block = it->first;

    EXPECT_EQ(server_block.port, 80);
    EXPECT_EQ(it->second.count("./"), 1) << "CASE 01";
  }

  {
    Validator::ServerMap server_map = TestValidatorSuccess("CASE_02");
    Validator::ServerMap::iterator it = server_map.begin();
    ServerBlock server_block = it->first;

    EXPECT_EQ(server_block.port, 8080);
    EXPECT_EQ(it->second.count("./trash"), 1) << "CASE 02";
  }

  {
    Validator::ServerMap server_map = TestValidatorSuccess("CASE_03");
    Validator::ServerMap::iterator it = server_map.begin();
    ServerBlock server_block = it->first;

    EXPECT_EQ(server_block.port, 4242);
    EXPECT_EQ(it->second.count("./hi"), 1) << "CASE 03";
  }

  {
    Validator::ServerMap server_map = TestValidatorSuccess("CASE_04");
    Validator::ServerMap::iterator it = server_map.begin();
    ServerBlock server_block = it->first;

    EXPECT_EQ(server_block.port, 4242);
    EXPECT_EQ(it->second.count("./return/42"), 1) << "CASE 04";
  }

  TestSyntaxException("CASE_05");

  {
    Validator::ServerMap server_map = TestValidatorSuccess("CASE_06");
    Validator::ServerMap::iterator it = server_map.begin();
    ServerBlock server_block = it->first;

    EXPECT_EQ(server_block.port, 80);
    EXPECT_EQ(it->second.count("./"), 1) << "CASE 06";
  }

  {
    Validator::ServerMap server_map = TestValidatorSuccess("CASE_07");
    Validator::ServerMap::iterator it = server_map.begin();
    ServerBlock server_block = it->first;

    EXPECT_EQ(server_block.port, 80);
    EXPECT_EQ(it->second.count("./"), 1) << "CASE 07";
  }

  TestSyntaxException("CASE_08");
  TestSyntaxException("CASE_09");
  TestSyntaxException("CASE_10");
  TestSyntaxException("CASE_11");

  {
    Validator::ServerMap server_map = TestValidatorSuccess("CASE_12");
    Validator::ServerMap::iterator it = server_map.begin();
    ServerBlock server_block = it->first;

    EXPECT_EQ(server_block.port, 80);
    EXPECT_EQ(server_block.server_name, "google.com");
    EXPECT_EQ(server_block.error, "404.html");
    EXPECT_EQ(it->second.count("./"), 1) << "CASE 12";
  }

  TestSyntaxException("CASE_13");
  TestSyntaxException("CASE_14");
  TestSyntaxException("CASE_15");
  TestSyntaxException("CASE_16");
  TestSyntaxException("CASE_17");

  {
    Validator::ServerMap server_map = TestValidatorSuccess("CASE_18");
    Validator::ServerMap::iterator it = server_map.begin();
    ServerBlock server_block = it->first;

    EXPECT_EQ(server_block.port, 8080);
    EXPECT_EQ(server_block.server_name, "127.0.0.1");
    EXPECT_EQ(server_block.error, "error.html");
    EXPECT_EQ(it->second.count("./default"), 1) << "CASE 18";
  }

  TestSyntaxException("CASE_19");
  TestSyntaxException("CASE_20");

  {
    Validator::ServerMap server_map = TestValidatorSuccess("CASE_21");
    Validator::ServerMap::iterator it = server_map.begin();
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

  TestSyntaxException("CASE_22");
  TestSyntaxException("CASE_23");

  // EXPECT_EQ(server_block.host, "127.0.0.1");
  // EXPECT_EQ(server_block.error, "error.html");
  // EXPECT_EQ(server_block.route.index, "index.html");
  // EXPECT_EQ(server_block.route.autoindex, false);
  // EXPECT_EQ(server_block.route.method, "GET");
}
