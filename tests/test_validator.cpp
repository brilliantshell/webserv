#include <gtest/gtest.h>

#include <fstream>
#include <sstream>

#include "Validator.hpp"

#define PATH_PREFIX "../configs/tests/validator/"

std::string FileToString(const std::string& file_path) {
  std::ifstream ifs(file_path);
  if (!ifs.good()) {
    std::cerr << "Open failure: " << file_path << std::endl;
  }
  std::stringstream ss;
  ifs >> ss.rdbuf();
  return ss.str();
}

void TestSyntaxException(const std::string& case_id) {
  std::cout << case_id << std::endl;
  Validator validator(FileToString(PATH_PREFIX + case_id + ".config"));
  EXPECT_THROW(validator.Validate(), Validator::SyntaxErrorException);
}

Validator::Result TestValidatorSuccess(const std::string& case_id) {
  std::cout << case_id << std::endl;
  Validator validator(FileToString(PATH_PREFIX + case_id + ".config"));
  return validator.Validate();
}

TEST(ValidatorTest, ServerBlock) {
  TestSyntaxException("ServerBlock/CASE_00");

  {
    Validator::Result result = TestValidatorSuccess("ServerBlock/CASE_01");
    EXPECT_EQ(result.server_map.count("127.0.0.1"), 1);
    ServerBlock& server_block = result.server_map["127.0.0.1"];
    RouteMap& route_map = server_block.route_map;
    HostVector& host_vector = result.host_vector;

    EXPECT_EQ(host_vector[0].port, 80);
    EXPECT_EQ(route_map.count("./"), 1) << "ServerBlock/CASE_01";
  }

  {
    Validator::Result result = TestValidatorSuccess("ServerBlock/CASE_02");
    EXPECT_EQ(result.server_map.count("127.0.0.1:8080"), 1);
    ServerBlock& server_block = result.server_map["127.0.0.1:8080"];
    RouteMap& route_map = server_block.route_map;
    HostVector& host_vector = result.host_vector;

    EXPECT_EQ(host_vector[0].port, 8080);
    EXPECT_EQ(route_map.count("./trash"), 1) << "ServerBlock/CASE_02";
  }

  {
    Validator::Result result = TestValidatorSuccess("ServerBlock/CASE_03");
    EXPECT_EQ(result.server_map.count("127.0.0.1:4242"), 1);
    ServerBlock& server_block = result.server_map["127.0.0.1:4242"];
    RouteMap& route_map = server_block.route_map;
    HostVector& host_vector = result.host_vector;

    EXPECT_EQ(host_vector[0].port, 4242);
    EXPECT_EQ(route_map.count("./hi"), 1) << "ServerBlock/CASE_02";
  }

  {
    Validator::Result result = TestValidatorSuccess("ServerBlock/CASE_04");
    EXPECT_EQ(result.server_map.count("127.0.0.1:4242"), 1);
    ServerBlock& server_block = result.server_map["127.0.0.1:4242"];
    RouteMap& route_map = server_block.route_map;
    HostVector& host_vector = result.host_vector;

    EXPECT_EQ(host_vector[0].port, 4242);
    EXPECT_EQ(route_map.count("./return/42"), 1) << "ServerBlock/CASE_04";
  }

  TestSyntaxException("ServerBlock/CASE_05");

  {
    Validator::Result result = TestValidatorSuccess("ServerBlock/CASE_06");
    EXPECT_EQ(result.server_map.count("127.0.0.1"), 1);
    ServerBlock& server_block = result.server_map["127.0.0.1"];
    RouteMap& route_map = server_block.route_map;
    HostVector& host_vector = result.host_vector;

    EXPECT_EQ(host_vector[0].port, 80);
    EXPECT_EQ(route_map.count("./"), 1) << "ServerBlock/CASE_06";
  }

  {
    Validator::Result result = TestValidatorSuccess("ServerBlock/CASE_07");
    EXPECT_EQ(result.server_map.count("127.0.0.1"), 1);
    ServerBlock& server_block = result.server_map["127.0.0.1"];
    RouteMap& route_map = server_block.route_map;
    HostVector& host_vector = result.host_vector;

    EXPECT_EQ(host_vector[0].port, 80);
    EXPECT_EQ(route_map.count("./"), 1) << "ServerBlock/CASE_07";
  }

  TestSyntaxException("ServerBlock/CASE_08");
  TestSyntaxException("ServerBlock/CASE_09");
  TestSyntaxException("ServerBlock/CASE_10");
  TestSyntaxException("ServerBlock/CASE_11");

  {
    Validator::Result result = TestValidatorSuccess("ServerBlock/CASE_12");
    EXPECT_EQ(result.server_map.count("google.com"), 1);
    ServerBlock& server_block = result.server_map["google.com"];
    RouteMap& route_map = server_block.route_map;
    HostVector& host_vector = result.host_vector;

    EXPECT_EQ(host_vector[0].port, 80);
    EXPECT_EQ(host_vector[0].host, "google.com");
    EXPECT_EQ(server_block.error, "404.html");
    EXPECT_EQ(route_map.count("./"), 1) << "ServerBlock/CASE_12";
  }

  TestSyntaxException("ServerBlock/CASE_13");
  TestSyntaxException("ServerBlock/CASE_14");
  TestSyntaxException("ServerBlock/CASE_15");
  TestSyntaxException("ServerBlock/CASE_16");
  TestSyntaxException("ServerBlock/CASE_17");

  {
    Validator::Result result = TestValidatorSuccess("ServerBlock/CASE_18");
    EXPECT_EQ(result.server_map.count("127.0.0.1:8080"), 1);
    ServerBlock& server_block = result.server_map["127.0.0.1:8080"];
    RouteMap& route_map = server_block.route_map;
    HostVector& host_vector = result.host_vector;

    EXPECT_EQ(host_vector[0].port, 8080);
    EXPECT_EQ(host_vector[0].host, "127.0.0.1");
    EXPECT_EQ(server_block.error, "error.html");
    EXPECT_EQ(route_map.count("./default"), 1) << "ServerBlock/CASE_18";
  }

  TestSyntaxException("ServerBlock/CASE_19");
  TestSyntaxException("ServerBlock/CASE_20");

  {
    Validator::Result result = TestValidatorSuccess("ServerBlock/CASE_21");
    EXPECT_EQ(result.server_map.count("127.0.0.1"), 1);
    ServerBlock server_block = result.server_map["127.0.0.1"];
    RouteMap& route_map = server_block.route_map;
    HostVector& host_vector = result.host_vector;

    EXPECT_EQ(host_vector[0].port, 80);
    EXPECT_EQ(host_vector[0].host, "127.0.0.1");
    EXPECT_EQ(server_block.error, "error.html");
    EXPECT_EQ(server_block.route_map.count("./"), 1) << "ServerBlock/CASE_21";

    EXPECT_EQ(result.server_map.count("127.0.0.1:4242"), 1);
    server_block = result.server_map["127.0.0.1:4242"];

    EXPECT_EQ(host_vector[1].port, 4242);
    EXPECT_EQ(host_vector[1].host, "127.0.0.1");
    EXPECT_EQ(server_block.error, "error.html");
    EXPECT_EQ(server_block.route_map.count("./forty_two"), 1)
        << "ServerBlock/CASE_21";
  }

  TestSyntaxException("ServerBlock/CASE_22");
  TestSyntaxException("ServerBlock/CASE_23");
  TestSyntaxException("ServerBlock/CASE_24");

  {
    ServerMap server_map =
        TestValidatorSuccess("ServerBlock/CASE_25").server_map;

    EXPECT_EQ(server_map.count("blahblah:443"), 1);
    ServerBlock server_block = server_map["blahblah:443"];
    EXPECT_EQ(server_block.error, "please.html");
    EXPECT_EQ(server_block.route_map.count("./"), 1);
  }

  TestSyntaxException("ServerBlock/CASE_26");
  TestSyntaxException("ServerBlock/CASE_27");

  {
    Validator::Result result = TestValidatorSuccess("ServerBlock/CASE_28");
    EXPECT_EQ(result.server_map.count("you_are_pro:4242"), 1);
    ServerBlock server_block = result.server_map["you_are_pro:4242"];
    RouteMap& route_map = server_block.route_map;
    HostVector& host_vector = result.host_vector;

    EXPECT_EQ(host_vector[0].port, 4242);
    EXPECT_EQ(host_vector[0].host, "you_are_pro");
    EXPECT_EQ(server_block.error, "yes.html");
    EXPECT_EQ(host_vector.size(), 1);
  }

  // TestSyntaxException("ServerBlock/CASE_25");

  // {
  //   Validator::Result =
  //       TestValidatorSuccess("ServerBlock/CASE_26");

  //   EXPECT_EQ(server_map.count("127.0.0.1"), 1);
  //   ServerBlock server_block = server_map["127.0.0.1"];
  //   EXPECT_EQ(server_block.port, 80);
  //   EXPECT_EQ(server_block.server_name, "127.0.0.1");
  //   EXPECT_EQ(server_block.host, "127.0.0.1");

  //   server_block = server_map["our42vent.42cadet.kr:443"];
  //   EXPECT_EQ(server_block.port, 443);
  //   EXPECT_EQ(server_block.server_name, "our42vent.42cadet.kr");
  //   EXPECT_EQ(server_block.host, "our42vent.42cadet.kr:443");

  //   server_block = server_map["127.0.0.1:8080"];
  //   EXPECT_EQ(server_block.port, 8080);
  //   EXPECT_EQ(server_block.server_name, "127.0.0.1");
  //   EXPECT_EQ(server_block.host, "127.0.0.1:8080");
  // }

  // route block 없는 경우
  // TestSyntaxException("ServerBlock/CASE_26");
}

TEST(ValidatorTest, RouteBlock) {
  {
    Validator::Result result = TestValidatorSuccess("RouteBlock/CASE_00");
    HostVector host_vector = result.host_vector;
    ServerMap server_map = result.server_map;

    EXPECT_EQ(server_map.count("127.0.0.1:4242"), 1);
    ServerBlock server_block = server_map["127.0.0.1:4242"];

    EXPECT_EQ(host_vector[0].port, 4242);
    EXPECT_EQ(host_vector[0].host, "127.0.0.1");
    EXPECT_EQ(server_block.error, "error.html");
    RouteMap route_map = server_block.route_map;

    EXPECT_EQ(route_map.count("./normal"), 1) << "RouteBlock/CASE_00";
    EXPECT_EQ(route_map.count(".php"), 1) << "RouteBlock/CASE_00";
  }

  TestSyntaxException("RouteBlock/CASE_01");
  TestSyntaxException("RouteBlock/CASE_02");

  {
    Validator::Result result = TestValidatorSuccess("RouteBlock/CASE_03");
    HostVector host_vector = result.host_vector;
    ServerMap server_map = result.server_map;

    EXPECT_EQ(server_map.count("127.0.0.1"), 1);
    ServerBlock server_block = server_map["127.0.0.1"];
    RouteMap route_map = server_block.route_map;

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
    Validator::Result result = TestValidatorSuccess("RouteBlock/CASE_04");
    HostVector host_vector = result.host_vector;
    ServerMap server_map = result.server_map;

    EXPECT_EQ(server_map.count("127.0.0.1:4242"), 1);
    ServerBlock server_block = server_map["127.0.0.1:4242"];
    RouteMap route_map = server_block.route_map;
    EXPECT_EQ(route_map.count(".js"), 1) << "RouteBlock/CASE_04";

    RouteBlock route_block = route_map[".js"];
    EXPECT_EQ(route_block.root, "./");
    EXPECT_EQ(route_block.methods, GET);
    EXPECT_EQ(route_block.body_max, INT_MAX);
    EXPECT_EQ(route_block.param, "fastjs_params");
  }

  TestSyntaxException("RouteBlock/CASE_05");

  {
    Validator::Result result = TestValidatorSuccess("RouteBlock/CASE_06");
    HostVector host_vector = result.host_vector;
    ServerMap server_map = result.server_map;

    EXPECT_EQ(server_map.count("127.0.0.1:4242"), 1);
    ServerBlock server_block = server_map["127.0.0.1:4242"];
    RouteMap route_map = server_block.route_map;
    EXPECT_EQ(route_map.count("./everything"), 1) << "RouteBlock/CASE_06";

    RouteBlock route_block = route_map["./everything"];
    EXPECT_EQ(route_block.root, "/root");
    EXPECT_EQ(route_block.index, "your_fault.html");
    EXPECT_EQ(route_block.upload_path, "/upload");
  }

  {
    Validator::Result result = TestValidatorSuccess("RouteBlock/CASE_07");
    HostVector host_vector = result.host_vector;
    ServerMap server_map = result.server_map;

    EXPECT_EQ(server_map.count("127.0.0.1:4242"), 1);
    ServerBlock server_block = server_map["127.0.0.1:4242"];
    RouteMap route_map = server_block.route_map;
    EXPECT_EQ(route_map.count("./everything"), 1) << "RouteBlock/CASE_07";

    RouteBlock route_block = route_map["./everything"];
    EXPECT_EQ(route_block.methods, GET | POST | DELETE);
  }

  TestSyntaxException("RouteBlock/CASE_08");
  TestSyntaxException("RouteBlock/CASE_09");
  TestSyntaxException("RouteBlock/CASE_10");

  {
    Validator::Result result = TestValidatorSuccess("RouteBlock/CASE_11");
    HostVector host_vector = result.host_vector;
    ServerMap server_map = result.server_map;

    EXPECT_EQ(server_map.count("127.0.0.1:4242"), 1);
    ServerBlock server_block = server_map["127.0.0.1:4242"];
    RouteMap route_map = server_block.route_map;
    EXPECT_EQ(route_map.count(".php"), 1) << "RouteBlock/CASE_11";

    RouteBlock route_block = route_map[".php"];
    EXPECT_EQ(route_block.methods, GET | POST);
  }

  TestSyntaxException("RouteBlock/CASE_12");

  {
    Validator::Result result = TestValidatorSuccess("RouteBlock/CASE_13");
    HostVector host_vector = result.host_vector;
    ServerMap server_map = result.server_map;

    EXPECT_EQ(server_map.count("127.0.0.1"), 1);
    ServerBlock server_block = server_map["127.0.0.1"];
    RouteMap route_map = server_block.route_map;
    EXPECT_EQ(route_map.count("./max"), 1) << "RouteBlock/CASE_13";

    RouteBlock route_block = route_map["./max"];
    EXPECT_EQ(route_block.methods, POST);
    EXPECT_EQ(route_block.body_max, 4096);
  }

  {
    Validator::Result result = TestValidatorSuccess("RouteBlock/CASE_14");
    HostVector host_vector = result.host_vector;
    ServerMap server_map = result.server_map;

    EXPECT_EQ(server_map.count("127.0.0.1"), 1);
    ServerBlock server_block = server_map["127.0.0.1"];
    RouteMap route_map = server_block.route_map;
    EXPECT_EQ(route_map.count("./max"), 1) << "RouteBlock/CASE_14";

    RouteBlock route_block = route_map["./max"];
    EXPECT_EQ(route_block.methods, POST);
    EXPECT_EQ(route_block.body_max, 128);
  }

  TestSyntaxException("RouteBlock/CASE_15");
  TestSyntaxException("RouteBlock/CASE_16");
  TestSyntaxException("RouteBlock/CASE_17");

  {
    Validator::Result result = TestValidatorSuccess("RouteBlock/CASE_18");
    HostVector host_vector = result.host_vector;
    ServerMap server_map = result.server_map;

    EXPECT_EQ(server_map.count("127.0.0.1:5050"), 1);
    ServerBlock server_block = server_map["127.0.0.1:5050"];
    RouteMap route_map = server_block.route_map;

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
    Validator::Result result = TestValidatorSuccess("RouteBlock/CASE_20");
    HostVector host_vector = result.host_vector;
    ServerMap server_map = result.server_map;

    EXPECT_EQ(server_map.count("127.0.0.1:5252"), 1);
    ServerBlock server_block = server_map["127.0.0.1:5252"];
    RouteMap route_map = server_block.route_map;
    EXPECT_EQ(route_map.count(".php"), 1) << "RouteBlock/CASE_20";

    RouteBlock route_block = route_map[".php"];
    EXPECT_EQ(route_block.methods, GET | POST);
    EXPECT_EQ(route_block.root, "/oh_no");
    EXPECT_EQ(route_block.param, "param_param");
    EXPECT_EQ(route_block.body_max, 1234);
  }

  {
    Validator::Result result = TestValidatorSuccess("RouteBlock/CASE_21");
    HostVector host_vector = result.host_vector;
    ServerMap server_map = result.server_map;

    EXPECT_EQ(server_map.count("127.0.0.1:17"), 1);
    ServerBlock server_block = server_map["127.0.0.1:17"];
    RouteMap route_map = server_block.route_map;

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
  TestSyntaxException("RouteBlock/CASE_23");
}
