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
  Validator validator(FileToString(case_id + ".config"));
  return validator.Validate();
}

TEST(ValidatorTest, ServerBlock) {
  TestSyntaxException("ServerBlock/CASE_00");

  {
    Validator::Result result =
        TestValidatorSuccess(PATH_PREFIX "ServerBlock/CASE_01");
    EXPECT_EQ(result.port_set.size(), 1);

    ASSERT_EQ(result.port_set.count(80), 1);
    ServerRouter server_router = result.port_map[80];
    LocationRouterMap location_router_map = server_router.location_router_map;

    ASSERT_EQ(location_router_map.size(), 1);
    ASSERT_EQ(location_router_map.count(""), 1);
    LocationRouter location_router = location_router_map[""];
    EXPECT_EQ(location_router.error.index, "./error.html")
        << "ServerBlock/CASE_01";
  }

  {
    Validator::Result result =
        TestValidatorSuccess(PATH_PREFIX "ServerBlock/CASE_02");
    EXPECT_EQ(result.port_set.size(), 1);

    ASSERT_EQ(result.port_set.count(8080), 1);
    ServerRouter server_router = result.port_map[8080];
    LocationRouterMap location_router_map = server_router.location_router_map;

    ASSERT_EQ(location_router_map.size(), 1);
    ASSERT_EQ(location_router_map.count(""), 1);
    LocationRouter location_router = location_router_map[""];
    LocationMap location_map = location_router.location_map;
    EXPECT_EQ(location_map.count("/trash/"), 1) << "ServerBlock/CASE_02";
  }

  {
    Validator::Result result =
        TestValidatorSuccess(PATH_PREFIX "ServerBlock/CASE_03");
    EXPECT_EQ(result.port_set.size(), 1);
    ASSERT_EQ(result.port_set.count(4242), 1);
    ServerRouter server_router = result.port_map[4242];
    LocationRouterMap location_router_map = server_router.location_router_map;

    ASSERT_EQ(location_router_map.size(), 1);
    ASSERT_EQ(location_router_map.count(""), 1);
    LocationRouter location_router = location_router_map[""];
    LocationMap location_map = location_router.location_map;
    EXPECT_EQ(location_map.count("/hi/"), 1) << "ServerBlock/CASE_03";
  }

  {
    Validator::Result result =
        TestValidatorSuccess(PATH_PREFIX "ServerBlock/CASE_04");
    EXPECT_EQ(result.port_set.size(), 1);

    ASSERT_EQ(result.port_set.count(4242), 1);
    ServerRouter server_router = result.port_map[4242];
    LocationRouterMap location_router_map = server_router.location_router_map;

    ASSERT_EQ(location_router_map.size(), 1);
    ASSERT_EQ(location_router_map.count(""), 1);
    LocationRouter location_router = location_router_map[""];
    LocationMap location_map = location_router.location_map;
    EXPECT_EQ(location_map.count("/return/42/"), 1) << "ServerBlock/CASE_04";
  }

  TestSyntaxException("ServerBlock/CASE_05");
  {
    Validator::Result result =
        TestValidatorSuccess(PATH_PREFIX "ServerBlock/CASE_06");
    EXPECT_EQ(result.port_set.size(), 1);

    ASSERT_EQ(result.port_set.count(80), 1);
    ServerRouter server_router = result.port_map[80];
    LocationRouterMap location_router_map = server_router.location_router_map;

    ASSERT_EQ(location_router_map.size(), 1);
    ASSERT_EQ(location_router_map.count(""), 1);
    LocationRouter location_router = location_router_map[""];
    LocationMap location_map = location_router.location_map;
    EXPECT_EQ(location_map.count("/"), 1) << "ServerBlock/CASE_06";
  }

  {
    Validator::Result result =
        TestValidatorSuccess(PATH_PREFIX "ServerBlock/CASE_07");
    EXPECT_EQ(result.port_set.size(), 1);

    ASSERT_EQ(result.port_set.count(80), 1);
    ServerRouter server_router = result.port_map[80];
    LocationRouterMap location_router_map = server_router.location_router_map;

    ASSERT_EQ(location_router_map.size(), 1);
    ASSERT_EQ(location_router_map.count(""), 1);
    LocationRouter location_router = location_router_map[""];
    LocationMap location_map = location_router.location_map;
    EXPECT_EQ(location_map.count("/"), 1) << "ServerBlock/CASE_07";
  }

  TestSyntaxException("ServerBlock/CASE_08");
  TestSyntaxException("ServerBlock/CASE_09");
  TestSyntaxException("ServerBlock/CASE_10");
  TestSyntaxException("ServerBlock/CASE_11");

  {
    Validator::Result result =
        TestValidatorSuccess(PATH_PREFIX "ServerBlock/CASE_12");
    EXPECT_EQ(result.port_set.size(), 1);

    ASSERT_EQ(result.port_set.count(80), 1);
    ServerRouter server_router = result.port_map[80];
    LocationRouterMap location_router_map = server_router.location_router_map;

    ASSERT_EQ(location_router_map.size(), 1);
    ASSERT_EQ(location_router_map.count("google.com"), 1);
    LocationRouter location_router = location_router_map["google.com"];
    EXPECT_EQ(location_router.error.index, "404.html");
    LocationMap location_map = location_router.location_map;
    EXPECT_EQ(location_map.count("/"), 1) << "ServerBlock/CASE_12";
  }

  TestSyntaxException("ServerBlock/CASE_13");
  TestSyntaxException("ServerBlock/CASE_14");
  TestSyntaxException("ServerBlock/CASE_15");
  TestSyntaxException("ServerBlock/CASE_16");
  TestSyntaxException("ServerBlock/CASE_17");

  {
    Validator::Result result =
        TestValidatorSuccess(PATH_PREFIX "ServerBlock/CASE_18");

    ASSERT_EQ(result.port_map.size(), 1);

    ASSERT_EQ(result.port_map.count(8080), 1);
    ServerRouter server_router = result.port_map[8080];
    LocationRouterMap location_router_map = server_router.location_router_map;

    ASSERT_EQ(location_router_map.size(), 1);
    ASSERT_EQ(location_router_map.count(""), 1);
    LocationRouter location_router = location_router_map[""];
    EXPECT_EQ(location_router.error.index, "./error.html");

    LocationMap location_map = location_router.location_map;
    EXPECT_EQ(location_map.count("/default/"), 1) << "ServerBlock/CASE_18";
  }

  TestSyntaxException("ServerBlock/CASE_19");
  TestSyntaxException("ServerBlock/CASE_20");

  {
    Validator::Result result =
        TestValidatorSuccess(PATH_PREFIX "ServerBlock/CASE_21");
    ASSERT_EQ(result.port_map.size(), 2);

    ASSERT_EQ(result.port_map.count(80), 1);
    ServerRouter server_router = result.port_map[80];
    LocationRouterMap location_router_map = server_router.location_router_map;

    ASSERT_EQ(location_router_map.size(), 1);
    ASSERT_EQ(location_router_map.count(""), 1);
    LocationRouter location_router = location_router_map[""];
    EXPECT_EQ(location_router.error.index, "./error.html");

    EXPECT_EQ(result.port_map.count(4242), 1);
    server_router = result.port_map[4242];
    location_router_map = server_router.location_router_map;

    ASSERT_EQ(location_router_map.size(), 1);
    ASSERT_EQ(location_router_map.count(""), 1);
    location_router = location_router_map[""];
    EXPECT_EQ(location_router.error.index, "./error.html");

    PortSet& port_set = result.port_set;
    ASSERT_EQ(port_set.size(), 2);
    EXPECT_EQ(port_set.count(80), 1);
    EXPECT_EQ(port_set.count(4242), 1);
  }

  TestSyntaxException("ServerBlock/CASE_22");
  TestSyntaxException("ServerBlock/CASE_23");
  TestSyntaxException("ServerBlock/CASE_24");

  {
    Validator::Result result =
        TestValidatorSuccess(PATH_PREFIX "ServerBlock/CASE_25");
    ASSERT_EQ(result.port_map.size(), 1);

    ASSERT_EQ(result.port_map.count(443), 1);
    ServerRouter server_router = result.port_map[443];
    LocationRouterMap location_router_map = server_router.location_router_map;
    ASSERT_EQ(location_router_map.size(), 1);
    ASSERT_EQ(location_router_map.count("blahblah"), 1);
    LocationRouter location_router = location_router_map["blahblah"];
    EXPECT_EQ(location_router.error.index, "please.html");

    PortSet& port_set = result.port_set;
    ASSERT_EQ(port_set.size(), 1);
    EXPECT_EQ(port_set.count(443), 1);
  }

  TestSyntaxException("ServerBlock/CASE_26");

  {
    Validator::Result result =
        TestValidatorSuccess(PATH_PREFIX "ServerBlock/CASE_27");
    ASSERT_EQ(result.port_map.size(), 1);

    ASSERT_EQ(result.port_map.count(4242), 1);
    ServerRouter server_router = result.port_map[4242];
    LocationRouterMap location_router_map = server_router.location_router_map;

    ASSERT_EQ(location_router_map.size(), 2);
    ASSERT_EQ(location_router_map.count(""), 1);
    LocationRouter location_router = location_router_map[""];
    EXPECT_EQ(location_router.error.index, "./error.html");

    ASSERT_EQ(location_router_map.count("you_are_pro"), 1);
    location_router = location_router_map["you_are_pro"];
    EXPECT_EQ(location_router.error.index, "./error.html");

    PortSet& port_set = result.port_set;
    ASSERT_EQ(port_set.size(), 1);
    EXPECT_EQ(port_set.count(4242), 1);
  }

  TestSyntaxException("ServerBlock/CASE_28");

  {
    Validator::Result result =
        TestValidatorSuccess(PATH_PREFIX "ServerBlock/CASE_29");
    ASSERT_EQ(result.port_map.size(), 2);
    ASSERT_EQ(result.port_set.size(), 2);

    ASSERT_EQ(result.port_map.count(8080), 1);
    ServerRouter server_router = result.port_map[8080];
    LocationRouterMap location_router_map = server_router.location_router_map;

    ASSERT_EQ(location_router_map.size(), 2);
    ASSERT_EQ(location_router_map.count(""), 1);
    LocationRouter location_router = location_router_map[""];
    EXPECT_EQ(location_router.error.index, "energetic.html");

    ASSERT_EQ(location_router_map.count("ghan"), 1);
    location_router = location_router_map["ghan"];
    EXPECT_EQ(location_router.error.index, "./error.html");
    EXPECT_EQ(server_router.default_server.error.index, "energetic.html");

    ASSERT_EQ(result.port_map.count(8081), 1);
    server_router = result.port_map[8081];
    location_router_map = server_router.location_router_map;

    ASSERT_EQ(location_router_map.count("jiskim"), 1);
    location_router = location_router_map["jiskim"];
    EXPECT_EQ(location_router.error.index, "./error.html");

    PortSet& port_set = result.port_set;

    ASSERT_EQ(port_set.size(), 2);
    EXPECT_EQ(port_set.count(8080), 1);
    EXPECT_EQ(port_set.count(8081), 1);
    EXPECT_EQ(server_router.default_server.error.index, "./error.html");
  }

  {
    Validator::Result result =
        TestValidatorSuccess(PATH_PREFIX "ServerBlock/CASE_30");
    ASSERT_EQ(result.port_map.size(), 1);
    ASSERT_EQ(result.port_set.size(), 1);
    ASSERT_EQ(result.port_map.count(1111), 1);
    ASSERT_EQ(result.port_set.count(1111), 1);
    ServerRouter server_router = result.port_map[1111];
    LocationRouterMap location_router_map = server_router.location_router_map;

    ASSERT_EQ(location_router_map.size(), 3);
    ASSERT_EQ(location_router_map.count("a"), 1);
    ASSERT_EQ(location_router_map.count("b"), 1);
    ASSERT_EQ(location_router_map.count("z"), 1);
    EXPECT_EQ(server_router.default_server.error.index, "go.html");
  }
}

TEST(ValidatorTest, RouteBlock) {
  {
    Validator::Result result =
        TestValidatorSuccess(PATH_PREFIX "RouteBlock/CASE_00");
    ASSERT_EQ(result.port_map.size(), 1);
    ASSERT_EQ(result.port_set.size(), 1);
    ASSERT_EQ(result.port_map.count(4242), 1);
    ASSERT_EQ(result.port_set.count(4242), 1);
    ServerRouter server_router = result.port_map[4242];
    LocationRouterMap location_router_map = server_router.location_router_map;

    ASSERT_EQ(location_router_map.size(), 1);
    ASSERT_EQ(location_router_map.count(""), 1);
    LocationRouter location_router = location_router_map[""];

    EXPECT_EQ(location_router.error.index, "./error.html");
    LocationMap location_map = location_router.location_map;

    EXPECT_EQ(location_map.count("/normal/"), 1) << "RouteBlock/CASE_00";
    EXPECT_EQ(location_map.count(".php"), 1) << "RouteBlock/CASE_00";
  }

  TestSyntaxException("RouteBlock/CASE_01");
  TestSyntaxException("RouteBlock/CASE_02");

  {
    Validator::Result result =
        TestValidatorSuccess(PATH_PREFIX "RouteBlock/CASE_03");
    ASSERT_EQ(result.port_map.size(), 1);
    ASSERT_EQ(result.port_set.size(), 1);
    ASSERT_EQ(result.port_map.count(80), 1);
    ASSERT_EQ(result.port_set.count(80), 1);
    ServerRouter server_router = result.port_map[80];
    LocationRouterMap location_router_map = server_router.location_router_map;

    ASSERT_EQ(location_router_map.size(), 1);
    ASSERT_EQ(location_router_map.count(""), 1);
    LocationRouter location_router = location_router_map[""];

    LocationMap location_map = location_router.location_map;

    ASSERT_EQ(location_map.count("/"), 1) << "RouteBlock/CASE_03";
    Location location = location_map["/"];
    EXPECT_EQ(location.root, "/");
    EXPECT_EQ(location.index, "");
    EXPECT_EQ(location.methods, GET);
    EXPECT_EQ(location.body_max, INT_MAX);
    EXPECT_EQ(location.autoindex, false);
    EXPECT_EQ(location.upload_path, "");
  }

  {
    Validator::Result result =
        TestValidatorSuccess(PATH_PREFIX "RouteBlock/CASE_04");
    ASSERT_EQ(result.port_map.size(), 1);
    ASSERT_EQ(result.port_set.size(), 1);
    ASSERT_EQ(result.port_map.count(4242), 1);
    ASSERT_EQ(result.port_set.count(4242), 1);
    ServerRouter server_router = result.port_map[4242];
    LocationRouterMap location_router_map = server_router.location_router_map;

    ASSERT_EQ(location_router_map.size(), 1);
    ASSERT_EQ(location_router_map.count(""), 1);
    LocationRouter location_router = location_router_map[""];
    LocationMap location_map = location_router.location_map;
    ASSERT_EQ(location_map.count(".js"), 1) << "RouteBlock/CASE_04";

    Location location = location_map[".js"];
    EXPECT_EQ(location.root, "/");
    EXPECT_EQ(location.methods, GET);
    EXPECT_EQ(location.body_max, INT_MAX);
    EXPECT_EQ(location.param, "fastjs_params");
  }

  TestSyntaxException("RouteBlock/CASE_05");

  {
    Validator::Result result =
        TestValidatorSuccess(PATH_PREFIX "RouteBlock/CASE_06");
    ASSERT_EQ(result.port_map.size(), 1);
    ASSERT_EQ(result.port_set.size(), 1);
    ASSERT_EQ(result.port_map.count(4242), 1);
    ASSERT_EQ(result.port_set.count(4242), 1);
    ServerRouter server_router = result.port_map[4242];
    LocationRouterMap location_router_map = server_router.location_router_map;

    ASSERT_EQ(location_router_map.size(), 1);
    ASSERT_EQ(location_router_map.count(""), 1);
    LocationRouter location_router = location_router_map[""];
    LocationMap location_map = location_router.location_map;
    ASSERT_EQ(location_map.count("/everything/"), 1) << "RouteBlock/CASE_06";

    Location location = location_map["/everything/"];
    EXPECT_EQ(location.root, "/root");
    EXPECT_EQ(location.index, "your_fault.html");
    EXPECT_EQ(location.upload_path, "/upload");
  }

  {
    Validator::Result result =
        TestValidatorSuccess(PATH_PREFIX "RouteBlock/CASE_07");
    ASSERT_EQ(result.port_map.size(), 1);
    ASSERT_EQ(result.port_set.size(), 1);
    ASSERT_EQ(result.port_map.count(4242), 1);
    ASSERT_EQ(result.port_set.count(4242), 1);
    ServerRouter server_router = result.port_map[4242];
    LocationRouterMap location_router_map = server_router.location_router_map;

    ASSERT_EQ(location_router_map.size(), 1);
    ASSERT_EQ(location_router_map.count(""), 1);
    LocationRouter location_router = location_router_map[""];
    LocationMap location_map = location_router.location_map;
    ASSERT_EQ(location_map.count("/everything/"), 1) << "RouteBlock/CASE_07";

    Location location = location_map["/everything/"];
    EXPECT_EQ(location.methods, GET | POST | DELETE);
  }

  TestSyntaxException("RouteBlock/CASE_08");
  TestSyntaxException("RouteBlock/CASE_09");
  TestSyntaxException("RouteBlock/CASE_10");

  {
    Validator::Result result =
        TestValidatorSuccess(PATH_PREFIX "RouteBlock/CASE_11");
    ASSERT_EQ(result.port_map.size(), 1);
    ASSERT_EQ(result.port_set.size(), 1);
    ASSERT_EQ(result.port_map.count(4242), 1);
    ASSERT_EQ(result.port_set.count(4242), 1);
    ServerRouter server_router = result.port_map[4242];
    LocationRouterMap location_router_map = server_router.location_router_map;

    ASSERT_EQ(location_router_map.size(), 1);
    ASSERT_EQ(location_router_map.count(""), 1);
    LocationRouter location_router = location_router_map[""];
    LocationMap location_map = location_router.location_map;
    ASSERT_EQ(location_map.count(".php"), 1) << "RouteBlock/CASE_11";

    Location location = location_map[".php"];
    EXPECT_EQ(location.methods, GET | POST);
  }

  TestSyntaxException("RouteBlock/CASE_12");

  {
    Validator::Result result =
        TestValidatorSuccess(PATH_PREFIX "RouteBlock/CASE_13");
    ASSERT_EQ(result.port_map.size(), 1);
    ASSERT_EQ(result.port_set.size(), 1);
    ASSERT_EQ(result.port_map.count(80), 1);
    ASSERT_EQ(result.port_set.count(80), 1);
    ServerRouter server_router = result.port_map[80];
    LocationRouterMap location_router_map = server_router.location_router_map;

    ASSERT_EQ(location_router_map.size(), 1);
    ASSERT_EQ(location_router_map.count(""), 1);
    LocationRouter location_router = location_router_map[""];
    LocationMap location_map = location_router.location_map;
    ASSERT_EQ(location_map.count("/max/"), 1) << "RouteBlock/CASE_13";

    Location location = location_map["/max/"];
    EXPECT_EQ(location.methods, POST);
    EXPECT_EQ(location.body_max, 4096);
  }

  {
    Validator::Result result =
        TestValidatorSuccess(PATH_PREFIX "RouteBlock/CASE_14");
    ASSERT_EQ(result.port_map.size(), 1);
    ASSERT_EQ(result.port_set.size(), 1);
    ASSERT_EQ(result.port_map.count(80), 1);
    ASSERT_EQ(result.port_set.count(80), 1);
    ServerRouter server_router = result.port_map[80];
    LocationRouterMap location_router_map = server_router.location_router_map;

    ASSERT_EQ(location_router_map.size(), 1);
    ASSERT_EQ(location_router_map.count(""), 1);
    LocationRouter location_router = location_router_map[""];
    LocationMap location_map = location_router.location_map;
    ASSERT_EQ(location_map.count("/max/"), 1) << "RouteBlock/CASE_14";

    Location location = location_map["/max/"];
    EXPECT_EQ(location.methods, POST);
    EXPECT_EQ(location.body_max, 128);
  }

  TestSyntaxException("RouteBlock/CASE_15");
  TestSyntaxException("RouteBlock/CASE_16");
  TestSyntaxException("RouteBlock/CASE_17");

  {
    Validator::Result result =
        TestValidatorSuccess(PATH_PREFIX "RouteBlock/CASE_18");
    ASSERT_EQ(result.port_map.size(), 1);
    ASSERT_EQ(result.port_set.size(), 1);
    ASSERT_EQ(result.port_map.count(5050), 1);
    ASSERT_EQ(result.port_set.count(5050), 1);
    ServerRouter server_router = result.port_map[5050];
    LocationRouterMap location_router_map = server_router.location_router_map;

    ASSERT_EQ(location_router_map.size(), 1);
    ASSERT_EQ(location_router_map.count(""), 1);
    LocationRouter location_router = location_router_map[""];
    LocationMap location_map = location_router.location_map;
    ASSERT_EQ(location_map.size(), 4);

    ASSERT_EQ(location_map.count("/first/"), 1);
    Location location = location_map["/first/"];
    EXPECT_EQ(location.methods, GET);

    ASSERT_EQ(location_map.count("/second/"), 1);
    location = location_map["/second/"];
    EXPECT_EQ(location.methods, POST);

    ASSERT_EQ(location_map.count(".rb"), 1);
    location = location_map[".rb"];
    EXPECT_EQ(location.param, "rb_param");

    ASSERT_EQ(location_map.count("/third/"), 1);
    location = location_map["/third/"];
    EXPECT_EQ(location.methods, DELETE);
  }

  TestSyntaxException("RouteBlock/CASE_19");

  {
    Validator::Result result =
        TestValidatorSuccess(PATH_PREFIX "RouteBlock/CASE_20");
    ASSERT_EQ(result.port_map.size(), 1);
    ASSERT_EQ(result.port_set.size(), 1);
    ASSERT_EQ(result.port_map.count(5252), 1);
    ASSERT_EQ(result.port_set.count(5252), 1);
    ServerRouter server_router = result.port_map[5252];
    LocationRouterMap location_router_map = server_router.location_router_map;

    ASSERT_EQ(location_router_map.size(), 1);
    ASSERT_EQ(location_router_map.count(""), 1);
    LocationRouter location_router = location_router_map[""];
    LocationMap location_map = location_router.location_map;
    ASSERT_EQ(location_map.size(), 1);
    ASSERT_EQ(location_map.count(".php"), 1) << "RouteBlock/CASE_20";

    Location location = location_map[".php"];
    EXPECT_EQ(location.methods, GET | POST);
    EXPECT_EQ(location.root, "/oh_no");
    EXPECT_EQ(location.param, "param_param");
    EXPECT_EQ(location.body_max, 1234);
  }

  {
    Validator::Result result =
        TestValidatorSuccess(PATH_PREFIX "RouteBlock/CASE_21");
    ASSERT_EQ(result.port_map.size(), 1);
    ASSERT_EQ(result.port_set.size(), 1);
    ASSERT_EQ(result.port_map.count(17), 1);
    ASSERT_EQ(result.port_set.count(17), 1);
    ServerRouter server_router = result.port_map[17];
    LocationRouterMap location_router_map = server_router.location_router_map;

    ASSERT_EQ(location_router_map.size(), 1);
    ASSERT_EQ(location_router_map.count(""), 1);
    LocationRouter location_router = location_router_map[""];
    LocationMap location_map = location_router.location_map;
    ASSERT_EQ(location_map.size(), 6);

    ASSERT_EQ(location_map.count("/http_no_port/"), 1) << "RouteBlock/CASE_21";

    Location location = location_map["/http_no_port/"];
    EXPECT_EQ(location.redirect_to, "naver.com");

    ASSERT_EQ(location_map.count("/http_port/"), 1) << "RouteBlock/CASE_21";

    location = location_map["/http_port/"];
    EXPECT_EQ(location.redirect_to, "naver.com:8080");

    ASSERT_EQ(location_map.count("/http_protoc_no_port/"), 1)
        << "RouteBlock/CASE_21";
    location = location_map["/http_protoc_no_port/"];
    EXPECT_EQ(location.redirect_to, "http://naver.com");

    ASSERT_EQ(location_map.count("/https_protoc_no_port/"), 1)
        << "RouteBlock/CASE_21";
    location = location_map["/https_protoc_no_port/"];
    EXPECT_EQ(location.redirect_to, "https://naver.com");

    ASSERT_EQ(location_map.count("/https_protoc_port/"), 1)
        << "RouteBlock/CASE_21";
    location = location_map["/https_protoc_port/"];
    EXPECT_EQ(location.redirect_to, "https://naver.com:80");

    ASSERT_EQ(location_map.count("/https_only_port/"), 1)
        << "RouteBlock/CASE_21";
    location = location_map["/https_only_port/"];
    EXPECT_EQ(location.redirect_to, "naver.com:443");
  }

  TestSyntaxException("RouteBlock/CASE_22");
  TestSyntaxException("RouteBlock/CASE_23");
}
