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
    EXPECT_EQ(result.port_set.size(), 1);

    ASSERT_EQ(result.port_set.count(80), 1);
    ServerGate server_gate = result.port_map[80];
    ServerMap server_map = server_gate.server_map;

    ASSERT_EQ(server_map.size(), 1);
    ASSERT_EQ(server_map.count(""), 1);
    ServerBlock server_block = server_map[""];
    EXPECT_EQ(server_block.error, "error.html") << "ServerBlock/CASE_01";
  }

  {
    Validator::Result result = TestValidatorSuccess("ServerBlock/CASE_02");
    EXPECT_EQ(result.port_set.size(), 1);

    ASSERT_EQ(result.port_set.count(8080), 1);
    ServerGate server_gate = result.port_map[8080];
    ServerMap server_map = server_gate.server_map;

    ASSERT_EQ(server_map.size(), 1);
    ASSERT_EQ(server_map.count(""), 1);
    ServerBlock server_block = server_map[""];
    RouteMap route_map = server_block.route_map;
    EXPECT_EQ(route_map.count("./trash"), 1) << "ServerBlock/CASE_02";
  }

  {
    Validator::Result result = TestValidatorSuccess("ServerBlock/CASE_03");
    EXPECT_EQ(result.port_set.size(), 1);
    ASSERT_EQ(result.port_set.count(4242), 1);
    ServerGate server_gate = result.port_map[4242];
    ServerMap server_map = server_gate.server_map;

    ASSERT_EQ(server_map.size(), 1);
    ASSERT_EQ(server_map.count(""), 1);
    ServerBlock server_block = server_map[""];
    RouteMap route_map = server_block.route_map;
    EXPECT_EQ(route_map.count("./hi"), 1) << "ServerBlock/CASE_03";
  }

  {
    Validator::Result result = TestValidatorSuccess("ServerBlock/CASE_04");
    EXPECT_EQ(result.port_set.size(), 1);

    ASSERT_EQ(result.port_set.count(4242), 1);
    ServerGate server_gate = result.port_map[4242];
    ServerMap server_map = server_gate.server_map;

    ASSERT_EQ(server_map.size(), 1);
    ASSERT_EQ(server_map.count(""), 1);
    ServerBlock server_block = server_map[""];
    RouteMap route_map = server_block.route_map;
    EXPECT_EQ(route_map.count("./return/42"), 1) << "ServerBlock/CASE_04";
  }

  TestSyntaxException("ServerBlock/CASE_05");
  {
    Validator::Result result = TestValidatorSuccess("ServerBlock/CASE_06");
    EXPECT_EQ(result.port_set.size(), 1);

    ASSERT_EQ(result.port_set.count(80), 1);
    ServerGate server_gate = result.port_map[80];
    ServerMap server_map = server_gate.server_map;

    ASSERT_EQ(server_map.size(), 1);
    ASSERT_EQ(server_map.count(""), 1);
    ServerBlock server_block = server_map[""];
    RouteMap route_map = server_block.route_map;
    EXPECT_EQ(route_map.count("./"), 1) << "ServerBlock/CASE_06";
  }

  {
    Validator::Result result = TestValidatorSuccess("ServerBlock/CASE_07");
    EXPECT_EQ(result.port_set.size(), 1);

    ASSERT_EQ(result.port_set.count(80), 1);
    ServerGate server_gate = result.port_map[80];
    ServerMap server_map = server_gate.server_map;

    ASSERT_EQ(server_map.size(), 1);
    ASSERT_EQ(server_map.count(""), 1);
    ServerBlock server_block = server_map[""];
    RouteMap route_map = server_block.route_map;
    EXPECT_EQ(route_map.count("./"), 1) << "ServerBlock/CASE_07";
  }

  TestSyntaxException("ServerBlock/CASE_08");
  TestSyntaxException("ServerBlock/CASE_09");
  TestSyntaxException("ServerBlock/CASE_10");
  TestSyntaxException("ServerBlock/CASE_11");

  {
    Validator::Result result = TestValidatorSuccess("ServerBlock/CASE_12");
    EXPECT_EQ(result.port_set.size(), 1);

    ASSERT_EQ(result.port_set.count(80), 1);
    ServerGate server_gate = result.port_map[80];
    ServerMap server_map = server_gate.server_map;

    ASSERT_EQ(server_map.size(), 1);
    ASSERT_EQ(server_map.count("google.com"), 1);
    ServerBlock server_block = server_map["google.com"];
    EXPECT_EQ(server_block.error, "404.html");
    RouteMap route_map = server_block.route_map;
    EXPECT_EQ(route_map.count("./"), 1) << "ServerBlock/CASE_12";
  }

  TestSyntaxException("ServerBlock/CASE_13");
  TestSyntaxException("ServerBlock/CASE_14");
  TestSyntaxException("ServerBlock/CASE_15");
  TestSyntaxException("ServerBlock/CASE_16");
  TestSyntaxException("ServerBlock/CASE_17");

  {
    Validator::Result result = TestValidatorSuccess("ServerBlock/CASE_18");

    ASSERT_EQ(result.port_map.size(), 1);

    ASSERT_EQ(result.port_map.count(8080), 1);
    ServerGate server_gate = result.port_map[8080];
    ServerMap server_map = server_gate.server_map;

    ASSERT_EQ(server_map.size(), 1);
    ASSERT_EQ(server_map.count(""), 1);
    ServerBlock server_block = server_map[""];
    EXPECT_EQ(server_block.error, "error.html");

    RouteMap route_map = server_block.route_map;
    EXPECT_EQ(route_map.count("./default"), 1) << "ServerBlock/CASE_18";
  }

  TestSyntaxException("ServerBlock/CASE_19");
  TestSyntaxException("ServerBlock/CASE_20");

  {
    Validator::Result result = TestValidatorSuccess("ServerBlock/CASE_21");
    ASSERT_EQ(result.port_map.size(), 2);

    ASSERT_EQ(result.port_map.count(80), 1);
    ServerGate server_gate = result.port_map[80];
    ServerMap server_map = server_gate.server_map;

    ASSERT_EQ(server_map.size(), 1);
    ASSERT_EQ(server_map.count(""), 1);
    ServerBlock server_block = server_map[""];
    EXPECT_EQ(server_block.error, "error.html");

    EXPECT_EQ(result.port_map.count(4242), 1);
    server_gate = result.port_map[4242];
    server_map = server_gate.server_map;

    ASSERT_EQ(server_map.size(), 1);
    ASSERT_EQ(server_map.count(""), 1);
    server_block = server_map[""];
    EXPECT_EQ(server_block.error, "error.html");

    PortSet& port_set = result.port_set;
    ASSERT_EQ(port_set.size(), 2);
    EXPECT_EQ(port_set.count(80), 1);
    EXPECT_EQ(port_set.count(4242), 1);
  }

  TestSyntaxException("ServerBlock/CASE_22");
  TestSyntaxException("ServerBlock/CASE_23");
  TestSyntaxException("ServerBlock/CASE_24");

  {
    Validator::Result result = TestValidatorSuccess("ServerBlock/CASE_25");
    ASSERT_EQ(result.port_map.size(), 1);

    ASSERT_EQ(result.port_map.count(443), 1);
    ServerGate server_gate = result.port_map[443];
    ServerMap server_map = server_gate.server_map;
    ASSERT_EQ(server_map.size(), 1);
    ASSERT_EQ(server_map.count("blahblah"), 1);
    ServerBlock server_block = server_map["blahblah"];
    EXPECT_EQ(server_block.error, "please.html");

    PortSet& port_set = result.port_set;
    ASSERT_EQ(port_set.size(), 1);
    EXPECT_EQ(port_set.count(443), 1);
  }

  TestSyntaxException("ServerBlock/CASE_26");

  {
    Validator::Result result = TestValidatorSuccess("ServerBlock/CASE_27");
    ASSERT_EQ(result.port_map.size(), 1);

    ASSERT_EQ(result.port_map.count(4242), 1);
    ServerGate server_gate = result.port_map[4242];
    ServerMap server_map = server_gate.server_map;

    ASSERT_EQ(server_map.size(), 2);
    ASSERT_EQ(server_map.count(""), 1);
    ServerBlock server_block = server_map[""];
    EXPECT_EQ(server_block.error, "error.html");

    ASSERT_EQ(server_map.count("you_are_pro"), 1);
    server_block = server_map["you_are_pro"];
    EXPECT_EQ(server_block.error, "error.html");

    PortSet& port_set = result.port_set;
    ASSERT_EQ(port_set.size(), 1);
    EXPECT_EQ(port_set.count(4242), 1);
  }

  TestSyntaxException("ServerBlock/CASE_28");

  {
    Validator::Result result = TestValidatorSuccess("ServerBlock/CASE_29");
    ASSERT_EQ(result.port_map.size(), 2);
    ASSERT_EQ(result.port_set.size(), 2);

    ASSERT_EQ(result.port_map.count(8080), 1);
    ServerGate server_gate = result.port_map[8080];
    ServerMap server_map = server_gate.server_map;

    ASSERT_EQ(server_map.size(), 2);
    ASSERT_EQ(server_map.count(""), 1);
    ServerBlock server_block = server_map[""];
    EXPECT_EQ(server_block.error, "energetic.html");

    ASSERT_EQ(server_map.count("ghan"), 1);
    server_block = server_map["ghan"];
    EXPECT_EQ(server_block.error, "error.html");
    EXPECT_EQ(server_gate.default_server.error, "energetic.html");

    ASSERT_EQ(result.port_map.count(8081), 1);
    server_gate = result.port_map[8081];
    server_map = server_gate.server_map;

    ASSERT_EQ(server_map.count("jiskim"), 1);
    server_block = server_map["jiskim"];
    EXPECT_EQ(server_block.error, "error.html");

    PortSet& port_set = result.port_set;

    ASSERT_EQ(port_set.size(), 2);
    EXPECT_EQ(port_set.count(8080), 1);
    EXPECT_EQ(port_set.count(8081), 1);
    EXPECT_EQ(server_gate.default_server.error, "error.html");
  }

  {
    Validator::Result result = TestValidatorSuccess("ServerBlock/CASE_30");
    ASSERT_EQ(result.port_map.size(), 1);
    ASSERT_EQ(result.port_set.size(), 1);
    ASSERT_EQ(result.port_map.count(1111), 1);
    ASSERT_EQ(result.port_set.count(1111), 1);
    ServerGate server_gate = result.port_map[1111];
    ServerMap server_map = server_gate.server_map;

    ASSERT_EQ(server_map.size(), 3);
    ASSERT_EQ(server_map.count("a"), 1);
    ASSERT_EQ(server_map.count("b"), 1);
    ASSERT_EQ(server_map.count("z"), 1);
    EXPECT_EQ(server_gate.default_server.error, "go.html");
  }
}

TEST(ValidatorTest, RouteBlock) {
  {
    Validator::Result result = TestValidatorSuccess("RouteBlock/CASE_00");
    ASSERT_EQ(result.port_map.size(), 1);
    ASSERT_EQ(result.port_set.size(), 1);
    ASSERT_EQ(result.port_map.count(4242), 1);
    ASSERT_EQ(result.port_set.count(4242), 1);
    ServerGate server_gate = result.port_map[4242];
    ServerMap server_map = server_gate.server_map;

    ASSERT_EQ(server_map.size(), 1);
    ASSERT_EQ(server_map.count(""), 1);
    ServerBlock server_block = server_map[""];

    EXPECT_EQ(server_block.error, "error.html");
    RouteMap route_map = server_block.route_map;

    EXPECT_EQ(route_map.count("./normal"), 1) << "RouteBlock/CASE_00";
    EXPECT_EQ(route_map.count(".php"), 1) << "RouteBlock/CASE_00";
  }

  TestSyntaxException("RouteBlock/CASE_01");
  TestSyntaxException("RouteBlock/CASE_02");

  {
    Validator::Result result = TestValidatorSuccess("RouteBlock/CASE_03");
    ASSERT_EQ(result.port_map.size(), 1);
    ASSERT_EQ(result.port_set.size(), 1);
    ASSERT_EQ(result.port_map.count(80), 1);
    ASSERT_EQ(result.port_set.count(80), 1);
    ServerGate server_gate = result.port_map[80];
    ServerMap server_map = server_gate.server_map;

    ASSERT_EQ(server_map.size(), 1);
    ASSERT_EQ(server_map.count(""), 1);
    ServerBlock server_block = server_map[""];

    RouteMap route_map = server_block.route_map;

    ASSERT_EQ(route_map.count("./"), 1) << "RouteBlock/CASE_03";
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
    ASSERT_EQ(result.port_map.size(), 1);
    ASSERT_EQ(result.port_set.size(), 1);
    ASSERT_EQ(result.port_map.count(4242), 1);
    ASSERT_EQ(result.port_set.count(4242), 1);
    ServerGate server_gate = result.port_map[4242];
    ServerMap server_map = server_gate.server_map;

    ASSERT_EQ(server_map.size(), 1);
    ASSERT_EQ(server_map.count(""), 1);
    ServerBlock server_block = server_map[""];
    RouteMap route_map = server_block.route_map;
    ASSERT_EQ(route_map.count(".js"), 1) << "RouteBlock/CASE_04";

    RouteBlock route_block = route_map[".js"];
    EXPECT_EQ(route_block.root, "./");
    EXPECT_EQ(route_block.methods, GET);
    EXPECT_EQ(route_block.body_max, INT_MAX);
    EXPECT_EQ(route_block.param, "fastjs_params");
  }

  TestSyntaxException("RouteBlock/CASE_05");

  {
    Validator::Result result = TestValidatorSuccess("RouteBlock/CASE_06");
    ASSERT_EQ(result.port_map.size(), 1);
    ASSERT_EQ(result.port_set.size(), 1);
    ASSERT_EQ(result.port_map.count(4242), 1);
    ASSERT_EQ(result.port_set.count(4242), 1);
    ServerGate server_gate = result.port_map[4242];
    ServerMap server_map = server_gate.server_map;

    ASSERT_EQ(server_map.size(), 1);
    ASSERT_EQ(server_map.count(""), 1);
    ServerBlock server_block = server_map[""];
    RouteMap route_map = server_block.route_map;
    ASSERT_EQ(route_map.count("./everything"), 1) << "RouteBlock/CASE_06";

    RouteBlock route_block = route_map["./everything"];
    EXPECT_EQ(route_block.root, "/root");
    EXPECT_EQ(route_block.index, "your_fault.html");
    EXPECT_EQ(route_block.upload_path, "/upload");
  }

  {
    Validator::Result result = TestValidatorSuccess("RouteBlock/CASE_07");
    ASSERT_EQ(result.port_map.size(), 1);
    ASSERT_EQ(result.port_set.size(), 1);
    ASSERT_EQ(result.port_map.count(4242), 1);
    ASSERT_EQ(result.port_set.count(4242), 1);
    ServerGate server_gate = result.port_map[4242];
    ServerMap server_map = server_gate.server_map;

    ASSERT_EQ(server_map.size(), 1);
    ASSERT_EQ(server_map.count(""), 1);
    ServerBlock server_block = server_map[""];
    RouteMap route_map = server_block.route_map;
    ASSERT_EQ(route_map.count("./everything"), 1) << "RouteBlock/CASE_07";

    RouteBlock route_block = route_map["./everything"];
    EXPECT_EQ(route_block.methods, GET | POST | DELETE);
  }

  TestSyntaxException("RouteBlock/CASE_08");
  TestSyntaxException("RouteBlock/CASE_09");
  TestSyntaxException("RouteBlock/CASE_10");

  {
    Validator::Result result = TestValidatorSuccess("RouteBlock/CASE_11");
    ASSERT_EQ(result.port_map.size(), 1);
    ASSERT_EQ(result.port_set.size(), 1);
    ASSERT_EQ(result.port_map.count(4242), 1);
    ASSERT_EQ(result.port_set.count(4242), 1);
    ServerGate server_gate = result.port_map[4242];
    ServerMap server_map = server_gate.server_map;

    ASSERT_EQ(server_map.size(), 1);
    ASSERT_EQ(server_map.count(""), 1);
    ServerBlock server_block = server_map[""];
    RouteMap route_map = server_block.route_map;
    ASSERT_EQ(route_map.count(".php"), 1) << "RouteBlock/CASE_11";

    RouteBlock route_block = route_map[".php"];
    EXPECT_EQ(route_block.methods, GET | POST);
  }

  TestSyntaxException("RouteBlock/CASE_12");

  {
    Validator::Result result = TestValidatorSuccess("RouteBlock/CASE_13");
    ASSERT_EQ(result.port_map.size(), 1);
    ASSERT_EQ(result.port_set.size(), 1);
    ASSERT_EQ(result.port_map.count(80), 1);
    ASSERT_EQ(result.port_set.count(80), 1);
    ServerGate server_gate = result.port_map[80];
    ServerMap server_map = server_gate.server_map;

    ASSERT_EQ(server_map.size(), 1);
    ASSERT_EQ(server_map.count(""), 1);
    ServerBlock server_block = server_map[""];
    RouteMap route_map = server_block.route_map;
    ASSERT_EQ(route_map.count("./max"), 1) << "RouteBlock/CASE_13";

    RouteBlock route_block = route_map["./max"];
    EXPECT_EQ(route_block.methods, POST);
    EXPECT_EQ(route_block.body_max, 4096);
  }

  {
    Validator::Result result = TestValidatorSuccess("RouteBlock/CASE_14");
    ASSERT_EQ(result.port_map.size(), 1);
    ASSERT_EQ(result.port_set.size(), 1);
    ASSERT_EQ(result.port_map.count(80), 1);
    ASSERT_EQ(result.port_set.count(80), 1);
    ServerGate server_gate = result.port_map[80];
    ServerMap server_map = server_gate.server_map;

    ASSERT_EQ(server_map.size(), 1);
    ASSERT_EQ(server_map.count(""), 1);
    ServerBlock server_block = server_map[""];
    RouteMap route_map = server_block.route_map;
    ASSERT_EQ(route_map.count("./max"), 1) << "RouteBlock/CASE_14";

    RouteBlock route_block = route_map["./max"];
    EXPECT_EQ(route_block.methods, POST);
    EXPECT_EQ(route_block.body_max, 128);
  }

  TestSyntaxException("RouteBlock/CASE_15");
  TestSyntaxException("RouteBlock/CASE_16");
  TestSyntaxException("RouteBlock/CASE_17");

  {
    Validator::Result result = TestValidatorSuccess("RouteBlock/CASE_18");
    ASSERT_EQ(result.port_map.size(), 1);
    ASSERT_EQ(result.port_set.size(), 1);
    ASSERT_EQ(result.port_map.count(5050), 1);
    ASSERT_EQ(result.port_set.count(5050), 1);
    ServerGate server_gate = result.port_map[5050];
    ServerMap server_map = server_gate.server_map;

    ASSERT_EQ(server_map.size(), 1);
    ASSERT_EQ(server_map.count(""), 1);
    ServerBlock server_block = server_map[""];
    RouteMap route_map = server_block.route_map;
    ASSERT_EQ(route_map.size(), 4);

    ASSERT_EQ(route_map.count("./first"), 1);
    RouteBlock route_block = route_map["./first"];
    EXPECT_EQ(route_block.methods, GET);

    ASSERT_EQ(route_map.count("./second"), 1);
    route_block = route_map["./second"];
    EXPECT_EQ(route_block.methods, POST);

    ASSERT_EQ(route_map.count(".rb"), 1);
    route_block = route_map[".rb"];
    EXPECT_EQ(route_block.param, "rb_param");

    ASSERT_EQ(route_map.count("./third"), 1);
    route_block = route_map["./third"];
    EXPECT_EQ(route_block.methods, DELETE);
  }

  TestSyntaxException("RouteBlock/CASE_19");

  {
    Validator::Result result = TestValidatorSuccess("RouteBlock/CASE_20");
    ASSERT_EQ(result.port_map.size(), 1);
    ASSERT_EQ(result.port_set.size(), 1);
    ASSERT_EQ(result.port_map.count(5252), 1);
    ASSERT_EQ(result.port_set.count(5252), 1);
    ServerGate server_gate = result.port_map[5252];
    ServerMap server_map = server_gate.server_map;

    ASSERT_EQ(server_map.size(), 1);
    ASSERT_EQ(server_map.count(""), 1);
    ServerBlock server_block = server_map[""];
    RouteMap route_map = server_block.route_map;
    ASSERT_EQ(route_map.size(), 1);
    ASSERT_EQ(route_map.count(".php"), 1) << "RouteBlock/CASE_20";

    RouteBlock route_block = route_map[".php"];
    EXPECT_EQ(route_block.methods, GET | POST);
    EXPECT_EQ(route_block.root, "/oh_no");
    EXPECT_EQ(route_block.param, "param_param");
    EXPECT_EQ(route_block.body_max, 1234);
  }

  {
    Validator::Result result = TestValidatorSuccess("RouteBlock/CASE_21");
    ASSERT_EQ(result.port_map.size(), 1);
    ASSERT_EQ(result.port_set.size(), 1);
    ASSERT_EQ(result.port_map.count(17), 1);
    ASSERT_EQ(result.port_set.count(17), 1);
    ServerGate server_gate = result.port_map[17];
    ServerMap server_map = server_gate.server_map;

    ASSERT_EQ(server_map.size(), 1);
    ASSERT_EQ(server_map.count(""), 1);
    ServerBlock server_block = server_map[""];
    RouteMap route_map = server_block.route_map;
    ASSERT_EQ(route_map.size(), 6);

    ASSERT_EQ(route_map.count("./http_no_port"), 1) << "RouteBlock/CASE_21";

    RouteBlock route_block = route_map["./http_no_port"];
    EXPECT_EQ(route_block.redirect_to, "naver.com");

    ASSERT_EQ(route_map.count("./http_port"), 1) << "RouteBlock/CASE_21";

    route_block = route_map["./http_port"];
    EXPECT_EQ(route_block.redirect_to, "naver.com:8080");

    ASSERT_EQ(route_map.count("./http_protoc_no_port"), 1)
        << "RouteBlock/CASE_21";
    route_block = route_map["./http_protoc_no_port"];
    EXPECT_EQ(route_block.redirect_to, "http://naver.com");

    ASSERT_EQ(route_map.count("./https_protoc_no_port"), 1)
        << "RouteBlock/CASE_21";
    route_block = route_map["./https_protoc_no_port"];
    EXPECT_EQ(route_block.redirect_to, "https://naver.com");

    ASSERT_EQ(route_map.count("./https_protoc_port"), 1)
        << "RouteBlock/CASE_21";
    route_block = route_map["./https_protoc_port"];
    EXPECT_EQ(route_block.redirect_to, "https://naver.com:80");

    ASSERT_EQ(route_map.count("./https_only_port"), 1) << "RouteBlock/CASE_21";
    route_block = route_map["./https_only_port"];
    EXPECT_EQ(route_block.redirect_to, "naver.com:443");
  }

  TestSyntaxException("RouteBlock/CASE_22");
  TestSyntaxException("RouteBlock/CASE_23");
}
