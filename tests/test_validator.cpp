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

void TestSyntaxException(const std::string& case_id) {
  std::cout << case_id << std::endl;
  Validator validator(FileToString(PATH_PREFIX + case_id + ".config"));
  EXPECT_THROW(validator.Validate(), Validator::SyntaxErrorException);
}

Validator::ServerMap TestValidatorSuccess(const std::string& case_id) {
  std::cout << case_id << std::endl;
  Validator validator(FileToString(PATH_PREFIX + case_id + ".config"));
  return validator.Validate();
}

TEST(ValidatorTest, ServerBlock) {
  TestSyntaxException("ServerBlock/CASE_00");

  {
    Validator::ServerMap server_map =
        TestValidatorSuccess("ServerBlock/CASE_01");
    Validator::ServerMap::iterator it = server_map.begin();
    ServerBlock server_block = it->first;

    EXPECT_EQ(server_block.port, 80);
    EXPECT_EQ(it->second.count("./"), 1) << "ServerBlock/CASE_01";
  }

  {
    Validator::ServerMap server_map =
        TestValidatorSuccess("ServerBlock/CASE_02");
    Validator::ServerMap::iterator it = server_map.begin();
    ServerBlock server_block = it->first;

    EXPECT_EQ(server_block.port, 8080);
    EXPECT_EQ(it->second.count("./trash"), 1) << "ServerBlock/CASE_02";
  }

  {
    Validator::ServerMap server_map =
        TestValidatorSuccess("ServerBlock/CASE_03");
    Validator::ServerMap::iterator it = server_map.begin();
    ServerBlock server_block = it->first;

    EXPECT_EQ(server_block.port, 4242);
    EXPECT_EQ(it->second.count("./hi"), 1) << "ServerBlock/CASE_03";
  }

  {
    Validator::ServerMap server_map =
        TestValidatorSuccess("ServerBlock/CASE_04");
    Validator::ServerMap::iterator it = server_map.begin();
    ServerBlock server_block = it->first;

    EXPECT_EQ(server_block.port, 4242);
    EXPECT_EQ(it->second.count("./return/42"), 1) << "ServerBlock/CASE_04";
  }

  TestSyntaxException("ServerBlock/CASE_05");

  {
    Validator::ServerMap server_map =
        TestValidatorSuccess("ServerBlock/CASE_06");
    Validator::ServerMap::iterator it = server_map.begin();
    ServerBlock server_block = it->first;

    EXPECT_EQ(server_block.port, 80);
    EXPECT_EQ(it->second.count("./"), 1) << "ServerBlock/CASE_06";
  }

  {
    Validator::ServerMap server_map =
        TestValidatorSuccess("ServerBlock/CASE_07");
    Validator::ServerMap::iterator it = server_map.begin();
    ServerBlock server_block = it->first;

    EXPECT_EQ(server_block.port, 80);
    EXPECT_EQ(it->second.count("./"), 1) << "ServerBlock/CASE_07";
  }

  TestSyntaxException("ServerBlock/CASE_08");
  TestSyntaxException("ServerBlock/CASE_09");
  TestSyntaxException("ServerBlock/CASE_10");
  TestSyntaxException("ServerBlock/CASE_11");

  {
    Validator::ServerMap server_map =
        TestValidatorSuccess("ServerBlock/CASE_12");
    Validator::ServerMap::iterator it = server_map.begin();
    ServerBlock server_block = it->first;

    EXPECT_EQ(server_block.port, 80);
    EXPECT_EQ(server_block.server_name, "google.com");
    EXPECT_EQ(server_block.error, "404.html");
    EXPECT_EQ(it->second.count("./"), 1) << "ServerBlock/CASE_12";
  }

  TestSyntaxException("ServerBlock/CASE_13");
  TestSyntaxException("ServerBlock/CASE_14");
  TestSyntaxException("ServerBlock/CASE_15");
  TestSyntaxException("ServerBlock/CASE_16");
  TestSyntaxException("ServerBlock/CASE_17");

  {
    Validator::ServerMap server_map =
        TestValidatorSuccess("ServerBlock/CASE_18");
    Validator::ServerMap::iterator it = server_map.begin();
    ServerBlock server_block = it->first;

    EXPECT_EQ(server_block.port, 8080);
    EXPECT_EQ(server_block.server_name, "127.0.0.1");
    EXPECT_EQ(server_block.error, "error.html");
    EXPECT_EQ(it->second.count("./default"), 1) << "ServerBlock/CASE_18";
  }

  TestSyntaxException("ServerBlock/CASE_19");
  TestSyntaxException("ServerBlock/CASE_20");

  {
    Validator::ServerMap server_map =
        TestValidatorSuccess("ServerBlock/CASE_21");
    Validator::ServerMap::iterator it = server_map.begin();
    ServerBlock server_block = it->first;

    EXPECT_EQ(server_block.port, 80);
    EXPECT_EQ(server_block.server_name, "127.0.0.1");
    EXPECT_EQ(server_block.error, "error.html");
    EXPECT_EQ(it->second.count("./"), 1) << "ServerBlock/CASE_21";

    server_block = (++it)->first;
    EXPECT_EQ(server_block.port, 4242);
    EXPECT_EQ(server_block.server_name, "127.0.0.1");
    EXPECT_EQ(server_block.error, "error.html");
    EXPECT_EQ(it->second.count("./forty_two"), 1) << "ServerBlock/CASE_21";
  }

  TestSyntaxException("ServerBlock/CASE_22");
  TestSyntaxException("ServerBlock/CASE_23");
  TestSyntaxException("ServerBlock/CASE_24");
}

TEST(ValidatorTest, RouteBlock) {
  {
    Validator::ServerMap server_map =
        TestValidatorSuccess("RouteBlock/CASE_00");
    Validator::ServerMap::iterator it = server_map.begin();
    ServerBlock server_block = it->first;

    EXPECT_EQ(server_block.port, 4242);
    EXPECT_EQ(server_block.server_name, "127.0.0.1");
    EXPECT_EQ(server_block.error, "error.html");
    Validator::RouteMap route_map = it->second;

    EXPECT_EQ(route_map.count("./normal"), 1) << "RouteBlock/CASE_00";
    EXPECT_EQ(route_map.count(".php"), 1) << "RouteBlock/CASE_00";
  }

  TestSyntaxException("RouteBlock/CASE_01");
  TestSyntaxException("RouteBlock/CASE_02");

  {
    Validator::ServerMap server_map =
        TestValidatorSuccess("RouteBlock/CASE_03");
    Validator::ServerMap::iterator it = server_map.begin();
    Validator::RouteMap route_map = it->second;
    EXPECT_EQ(route_map.count("./"), 1) << "RouteBlock/CASE_03";

    RouteBlock route_block = route_map["./"];
    EXPECT_EQ(route_block.root, "./");
    EXPECT_EQ(route_block.index, "");
    EXPECT_EQ(route_block.methods, GET);
    EXPECT_EQ(route_block.body_max, INT_MAX);
    EXPECT_EQ(route_block.autoindex, false);
    EXPECT_EQ(route_block.upload_path, "");
  }

  {
    Validator::ServerMap server_map =
        TestValidatorSuccess("RouteBlock/CASE_04");
    Validator::ServerMap::iterator it = server_map.begin();
    Validator::RouteMap route_map = it->second;
    EXPECT_EQ(route_map.count(".js"), 1) << "RouteBlock/CASE_04";

    RouteBlock route_block = route_map[".js"];
    EXPECT_EQ(route_block.root, "./");
    EXPECT_EQ(route_block.methods, GET);
    EXPECT_EQ(route_block.body_max, INT_MAX);
    EXPECT_EQ(route_block.param, "fastjs_params");
  }

  TestSyntaxException("RouteBlock/CASE_05");

  {
    Validator::ServerMap server_map =
        TestValidatorSuccess("RouteBlock/CASE_06");
    Validator::ServerMap::iterator it = server_map.begin();
    Validator::RouteMap route_map = it->second;
    EXPECT_EQ(route_map.count("./everything"), 1) << "RouteBlock/CASE_06";

    RouteBlock route_block = route_map["./everything"];
    EXPECT_EQ(route_block.root, "/root");
    EXPECT_EQ(route_block.index, "your_fault.html");
    EXPECT_EQ(route_block.upload_path, "/upload");
  }

  {
    Validator::ServerMap server_map =
        TestValidatorSuccess("RouteBlock/CASE_07");
    Validator::ServerMap::iterator it = server_map.begin();
    Validator::RouteMap route_map = it->second;
    EXPECT_EQ(route_map.count("./everything"), 1) << "RouteBlock/CASE_07";

    RouteBlock route_block = route_map["./everything"];
    EXPECT_EQ(route_block.methods, GET | POST | DELETE);
  }

  TestSyntaxException("RouteBlock/CASE_08");
  TestSyntaxException("RouteBlock/CASE_09");
  TestSyntaxException("RouteBlock/CASE_10");

  {
    Validator::ServerMap server_map =
        TestValidatorSuccess("RouteBlock/CASE_11");
    Validator::ServerMap::iterator it = server_map.begin();
    Validator::RouteMap route_map = it->second;
    EXPECT_EQ(route_map.count(".php"), 1) << "RouteBlock/CASE_11";

    RouteBlock route_block = route_map[".php"];
    EXPECT_EQ(route_block.methods, GET | POST);
  }

  TestSyntaxException("RouteBlock/CASE_12");

  {
    Validator::ServerMap server_map =
        TestValidatorSuccess("RouteBlock/CASE_13");
    Validator::ServerMap::iterator it = server_map.begin();
    Validator::RouteMap route_map = it->second;
    EXPECT_EQ(route_map.count("./max"), 1) << "RouteBlock/CASE_13";

    RouteBlock route_block = route_map["./max"];
    EXPECT_EQ(route_block.methods, POST);
    EXPECT_EQ(route_block.body_max, 4096);
  }

  {
    Validator::ServerMap server_map =
        TestValidatorSuccess("RouteBlock/CASE_14");
    Validator::ServerMap::iterator it = server_map.begin();
    Validator::RouteMap route_map = it->second;
    EXPECT_EQ(route_map.count("./max"), 1) << "RouteBlock/CASE_14";

    RouteBlock route_block = route_map["./max"];
    EXPECT_EQ(route_block.methods, POST);
    EXPECT_EQ(route_block.body_max, 128);
  }

  TestSyntaxException("RouteBlock/CASE_15");
  TestSyntaxException("RouteBlock/CASE_16");
  TestSyntaxException("RouteBlock/CASE_17");

  {
    Validator::ServerMap server_map =
        TestValidatorSuccess("RouteBlock/CASE_18");
    Validator::ServerMap::iterator it = server_map.begin();
    Validator::RouteMap route_map = it->second;

    EXPECT_EQ(route_map.count("./first"), 1);
    RouteBlock route_block = route_map["./first"];
    EXPECT_EQ(route_block.methods, GET);

    EXPECT_EQ(route_map.count("./second"), 1);
    route_block = route_map["./second"];
    EXPECT_EQ(route_block.methods, POST);

    EXPECT_EQ(route_map.count(".rb"), 1);
    route_block = route_map[".rb"];
    EXPECT_EQ(route_block.param, "rb_param");

    EXPECT_EQ(route_map.count("./third"), 1);
    route_block = route_map["./third"];
    EXPECT_EQ(route_block.methods, DELETE);
  }

  TestSyntaxException("RouteBlock/CASE_19");

  {
    Validator::ServerMap server_map =
        TestValidatorSuccess("RouteBlock/CASE_20");
    Validator::ServerMap::iterator it = server_map.begin();
    Validator::RouteMap route_map = it->second;
    EXPECT_EQ(route_map.count(".php"), 1) << "RouteBlock/CASE_20";

    RouteBlock route_block = route_map[".php"];
    EXPECT_EQ(route_block.methods, GET | POST);
    EXPECT_EQ(route_block.root, "/oh_no");
    EXPECT_EQ(route_block.param, "param_param");
    EXPECT_EQ(route_block.body_max, 1234);
  }

  {
    Validator::ServerMap server_map =
        TestValidatorSuccess("RouteBlock/CASE_21");
    Validator::ServerMap::iterator it = server_map.begin();
    Validator::RouteMap route_map = it->second;
    EXPECT_EQ(route_map.count("./http_no_port"), 1) << "RouteBlock/CASE_21";

    RouteBlock route_block = route_map["./http_no_port"];
    EXPECT_EQ(route_block.redirect_to, "naver.com");

    EXPECT_EQ(route_map.count("./http_port"), 1) << "RouteBlock/CASE_21";

    route_block = route_map["./http_port"];
    EXPECT_EQ(route_block.redirect_to, "naver.com:8080");

    EXPECT_EQ(route_map.count("./http_protoc_no_port"), 1)
        << "RouteBlock/CASE_21";
    route_block = route_map["./http_protoc_no_port"];
    EXPECT_EQ(route_block.redirect_to, "http://naver.com");

    EXPECT_EQ(route_map.count("./https_protoc_no_port"), 1)
        << "RouteBlock/CASE_21";
    route_block = route_map["./https_protoc_no_port"];
    EXPECT_EQ(route_block.redirect_to, "https://naver.com");

    EXPECT_EQ(route_map.count("./https_protoc_port"), 1)
        << "RouteBlock/CASE_21";
    route_block = route_map["./https_protoc_port"];
    EXPECT_EQ(route_block.redirect_to, "https://naver.com:80");

    EXPECT_EQ(route_map.count("./https_only_port"), 1) << "RouteBlock/CASE_21";
    route_block = route_map["./https_only_port"];
    EXPECT_EQ(route_block.redirect_to, "naver.com:443");
  }

  TestSyntaxException("RouteBlock/CASE_22");
}
