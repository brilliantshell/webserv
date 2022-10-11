/**
 * @file test_router.cpp
 * @author ghan, jiskim, yongjule
 * @brief
 * @date 2022-10-07
 *
 * @copyright Copyright (c) 2022
 */

#include <gtest/gtest.h>

#include <fstream>
#include <iostream>
//#include <sstream>

#include "Connection.hpp"
#include "HttpParser.hpp"
#include "Router.hpp"
#include "Types.hpp"
#include "Validator.hpp"

#define ROUTER_CONFIG_PATH_PREFIX "../configs/tests/router/"
#define ROUTER_REQ_PATH_PREFIX "../tests/router/"

std::string FileToString(const std::string& file_path);
Validator::Result TestValidatorSuccess(const std::string& case_id);

// Route() 결과값
/*
result{
        status
        method
        success_path
        error_path
        param
}
*/
//

// .txt (request) , .config (config)
TEST(RouteTest, ServerRouter) {
  {
    Validator::Result result =
        TestValidatorSuccess(ROUTER_CONFIG_PATH_PREFIX "s_00");
    PortMap port_map = result.port_map;
    EXPECT_EQ(port_map.size(), 1);
    EXPECT_EQ(port_map.count(4242), 1);

    HttpParser parser;
    std::string req_buf = FileToString(ROUTER_REQ_PATH_PREFIX "s_00.txt");
    int status = parser.Parse(req_buf);
    EXPECT_EQ(status, HttpParser::kComplete);
    HttpParser::Result parse_result = parser.get_result();

    Router router(port_map[4242]);
    Router::Result route_result =
        router.Route(parse_result.status, parse_result.request);

    EXPECT_EQ(route_result.status, 200);
    EXPECT_EQ(route_result.method, GET);
    EXPECT_EQ(route_result.success_path, "./normal/index.html");
    EXPECT_EQ(route_result.error_path, "./error.html");
    EXPECT_EQ(route_result.param, "");
  }

  {
    Validator::Result result =
        TestValidatorSuccess(ROUTER_CONFIG_PATH_PREFIX "s_01");
    PortMap port_map = result.port_map;
    EXPECT_EQ(port_map.size(), 1);
    EXPECT_EQ(port_map.count(4242), 1);

    HttpParser parser;
    std::string req_buf = FileToString(ROUTER_REQ_PATH_PREFIX "s_01.txt");
    int status = parser.Parse(req_buf);
    EXPECT_EQ(status, HttpParser::kClose);
    HttpParser::Result parse_result = parser.get_result();

    Router router(port_map[4242]);
    Router::Result route_result =
        router.Route(parse_result.status, parse_result.request);

    EXPECT_EQ(route_result.status, 200);
    EXPECT_EQ(route_result.method, GET);
    EXPECT_EQ(route_result.success_path,
              "./root_inside/test/index.html/index.html");
    EXPECT_EQ(route_result.error_path, "./blah/404.html");
  }

  {
    Validator::Result result =
        TestValidatorSuccess(ROUTER_CONFIG_PATH_PREFIX "f_00");
    PortMap port_map = result.port_map;
    EXPECT_EQ(port_map.size(), 1);
    EXPECT_EQ(port_map.count(4242), 1);

    HttpParser parser;
    std::string req_buf = FileToString(ROUTER_REQ_PATH_PREFIX "f_00.txt");
    int status = parser.Parse(req_buf);
    EXPECT_EQ(status, HttpParser::kClose);
    HttpParser::Result parse_result = parser.get_result();

    Router router(port_map[4242]);
    Router::Result route_result =
        router.Route(parse_result.status, parse_result.request);

    EXPECT_EQ(route_result.status, 404);
    EXPECT_EQ(route_result.method, GET);
    EXPECT_EQ(route_result.success_path, "./blah/404.html");
    EXPECT_EQ(route_result.error_path, "./blah/404.html");
  }

  {
    Validator::Result result =
        TestValidatorSuccess(ROUTER_CONFIG_PATH_PREFIX "s_02");
    PortMap port_map = result.port_map;
    EXPECT_EQ(port_map.size(), 1);
    EXPECT_EQ(port_map.count(2424), 1);

    HttpParser parser;
    std::string req_buf = FileToString(ROUTER_REQ_PATH_PREFIX "s_02.txt");
    int status = parser.Parse(req_buf);
    EXPECT_EQ(status, HttpParser::kComplete);
    HttpParser::Result parse_result = parser.get_result();

    Router router(port_map[2424]);
    Router::Result route_result =
        router.Route(parse_result.status, parse_result.request);

    EXPECT_EQ(route_result.status, 200);
    EXPECT_EQ(route_result.method, GET);
    EXPECT_EQ(route_result.success_path, "./ghan/index.html");
    EXPECT_EQ(route_result.error_path, "./ghan.error.html");
  }

  {
    Validator::Result result =
        TestValidatorSuccess(ROUTER_CONFIG_PATH_PREFIX "s_03");
    PortMap port_map = result.port_map;
    EXPECT_EQ(port_map.size(), 1);
    EXPECT_EQ(port_map.count(2424), 1);

    HttpParser parser;
    std::string req_buf = FileToString(ROUTER_REQ_PATH_PREFIX "s_03.txt");
    int status = parser.Parse(req_buf);
    EXPECT_EQ(status, HttpParser::kClose);
    HttpParser::Result parse_result = parser.get_result();

    Router router(port_map[2424]);
    Router::Result route_result =
        router.Route(parse_result.status, parse_result.request);

    EXPECT_EQ(route_result.status, 200);
    EXPECT_EQ(route_result.method, GET);
    EXPECT_EQ(route_result.success_path, "./yongjule/index.html");
    EXPECT_EQ(route_result.error_path, "./yongjule.error.html");
  }

  {
    Validator::Result result =
        TestValidatorSuccess(ROUTER_CONFIG_PATH_PREFIX "f_01");
    PortMap port_map = result.port_map;
    EXPECT_EQ(port_map.size(), 1);
    EXPECT_EQ(port_map.count(4242), 1);

    HttpParser parser;
    std::string req_buf = FileToString(ROUTER_REQ_PATH_PREFIX "f_01.txt");
    int status = parser.Parse(req_buf);
    EXPECT_EQ(status, HttpParser::kClose);
    HttpParser::Result parse_result = parser.get_result();

    Router router(port_map[4242]);
    Router::Result route_result =
        router.Route(parse_result.status, parse_result.request);

    EXPECT_EQ(route_result.status, 400);
    EXPECT_EQ(route_result.method, GET);
    EXPECT_EQ(route_result.success_path, "./f_01.html");
    EXPECT_EQ(route_result.error_path, "./f_01.html");
  }

  {
    Validator::Result result =
        TestValidatorSuccess(ROUTER_CONFIG_PATH_PREFIX "s_04");
    PortMap port_map = result.port_map;
    EXPECT_EQ(port_map.size(), 1);
    EXPECT_EQ(port_map.count(2424), 1);

    HttpParser parser;
    std::string req_buf = FileToString(ROUTER_REQ_PATH_PREFIX "s_04.txt");
    int status = parser.Parse(req_buf);
    EXPECT_EQ(status, HttpParser::kClose);
    HttpParser::Result parse_result = parser.get_result();

    Router router(port_map[2424]);
    Router::Result route_result =
        router.Route(parse_result.status, parse_result.request);

    EXPECT_EQ(route_result.status, 200);
    EXPECT_EQ(route_result.method, DELETE);
    EXPECT_EQ(route_result.success_path, "./yongjule/index.html");
    EXPECT_EQ(route_result.error_path, "./yongjule.error.html");
  }
}

TEST(RouterTest, LocationRouter) {
  {
    Validator::Result result =
        TestValidatorSuccess(ROUTER_CONFIG_PATH_PREFIX "f_02");
    PortMap port_map = result.port_map;
    EXPECT_EQ(port_map.size(), 1);
    EXPECT_EQ(port_map.count(2424), 1);

    HttpParser parser;
    std::string req_buf = FileToString(ROUTER_REQ_PATH_PREFIX "f_02.txt");
    int status = parser.Parse(req_buf);
    EXPECT_EQ(status, HttpParser::kClose);
    HttpParser::Result parse_result = parser.get_result();

    Router router(port_map[2424]);
    Router::Result route_result =
        router.Route(parse_result.status, parse_result.request);

    EXPECT_EQ(route_result.status, 405);
    EXPECT_EQ(route_result.method, GET);
    EXPECT_EQ(route_result.success_path, "./yongjule.error.html");
    EXPECT_EQ(route_result.error_path, "./yongjule.error.html");
  }

  {
    Validator::Result result =
        TestValidatorSuccess(ROUTER_CONFIG_PATH_PREFIX "f_03");
    PortMap port_map = result.port_map;
    EXPECT_EQ(port_map.size(), 1);
    EXPECT_EQ(port_map.count(4242), 1);

    HttpParser parser;
    std::string req_buf = FileToString(ROUTER_REQ_PATH_PREFIX "f_03.txt");
    int status = parser.Parse(req_buf);
    EXPECT_EQ(status, HttpParser::kClose);
    HttpParser::Result parse_result = parser.get_result();

    Router router(port_map[4242]);
    Router::Result route_result =
        router.Route(parse_result.status, parse_result.request);

    EXPECT_EQ(route_result.status, 405);
    EXPECT_EQ(route_result.method, GET);
    EXPECT_EQ(route_result.success_path, "./merong_jiskim");
    EXPECT_EQ(route_result.error_path, "./merong_jiskim");
  }

  {
    Validator::Result result =
        TestValidatorSuccess(ROUTER_CONFIG_PATH_PREFIX "f_04");
    PortMap port_map = result.port_map;
    EXPECT_EQ(port_map.size(), 1);
    EXPECT_EQ(port_map.count(4242), 1);

    HttpParser parser;
    std::string req_buf = FileToString(ROUTER_REQ_PATH_PREFIX "f_04.txt");
    int status = parser.Parse(req_buf);
    EXPECT_EQ(status, HttpParser::kComplete);
    HttpParser::Result parse_result = parser.get_result();

    Router router(port_map[4242]);
    Router::Result route_result =
        router.Route(parse_result.status, parse_result.request);

    EXPECT_EQ(route_result.status, 413);
    EXPECT_EQ(route_result.method, GET);
    EXPECT_EQ(route_result.success_path, "./body_max.html");
    EXPECT_EQ(route_result.error_path, "./body_max.html");
  }

  {
    Validator::Result result =
        TestValidatorSuccess(ROUTER_CONFIG_PATH_PREFIX "s_05");
    PortMap port_map = result.port_map;
    EXPECT_EQ(port_map.size(), 1);
    EXPECT_EQ(port_map.count(4242), 1);

    HttpParser parser;
    std::string req_buf = FileToString(ROUTER_REQ_PATH_PREFIX "s_05.txt");
    int status = parser.Parse(req_buf);
    EXPECT_EQ(status, HttpParser::kComplete);
    HttpParser::Result parse_result = parser.get_result();

    Router router(port_map[4242]);
    Router::Result route_result =
        router.Route(parse_result.status, parse_result.request);

    EXPECT_EQ(route_result.status, 200);
    EXPECT_EQ(route_result.method, GET);
    EXPECT_EQ(route_result.success_path, "./a/b/a/b/zjj");
    EXPECT_EQ(route_result.error_path, "./f/irst/error.html");
  }

  {
    Validator::Result result =
        TestValidatorSuccess(ROUTER_CONFIG_PATH_PREFIX "s_06");
    PortMap port_map = result.port_map;
    EXPECT_EQ(port_map.size(), 1);
    EXPECT_EQ(port_map.count(4242), 1);

    HttpParser parser;
    std::string req_buf = FileToString(ROUTER_REQ_PATH_PREFIX "s_06.txt");
    int status = parser.Parse(req_buf);
    EXPECT_EQ(status, HttpParser::kComplete);
    HttpParser::Result parse_result = parser.get_result();

    Router router(port_map[4242]);
    Router::Result route_result =
        router.Route(parse_result.status, parse_result.request);

    EXPECT_EQ(route_result.status, 200);
    EXPECT_EQ(route_result.method, GET);
    EXPECT_EQ(route_result.success_path, "./path/to/php/index.php");
    EXPECT_EQ(route_result.error_path, "./error.html");
    EXPECT_EQ(route_result.param, "./php_cgi");
  }

  {
    Validator::Result result =
        TestValidatorSuccess(ROUTER_CONFIG_PATH_PREFIX "s_07");
    PortMap port_map = result.port_map;
    EXPECT_EQ(port_map.size(), 1);
    EXPECT_EQ(port_map.count(4242), 1);

    HttpParser parser;
    std::string req_buf = FileToString(ROUTER_REQ_PATH_PREFIX "s_07.txt");
    int status = parser.Parse(req_buf);
    EXPECT_EQ(status, HttpParser::kClose);
    HttpParser::Result parse_result = parser.get_result();

    Router router(port_map[4242]);
    Router::Result route_result =
        router.Route(parse_result.status, parse_result.request);

    EXPECT_EQ(route_result.status, 200);
    EXPECT_EQ(route_result.method, POST);
    EXPECT_EQ(route_result.success_path, "./root/upload_path/upload/file.txt");
    EXPECT_EQ(route_result.error_path, "./error.html");
  }

  {
    Validator::Result result =
        TestValidatorSuccess(ROUTER_CONFIG_PATH_PREFIX "f_05");
    PortMap port_map = result.port_map;
    EXPECT_EQ(port_map.size(), 1);
    EXPECT_EQ(port_map.count(4242), 1);

    HttpParser parser;
    std::string req_buf = FileToString(ROUTER_REQ_PATH_PREFIX "f_05.txt");
    int status = parser.Parse(req_buf);
    EXPECT_EQ(status, HttpParser::kClose);
    HttpParser::Result parse_result = parser.get_result();

    Router router(port_map[4242]);
    Router::Result route_result =
        router.Route(parse_result.status, parse_result.request);

    EXPECT_EQ(route_result.status, 403);  // FORBIDDEN
    EXPECT_EQ(route_result.method, GET);
    EXPECT_EQ(route_result.success_path, "./error.html");
    EXPECT_EQ(route_result.error_path, "./error.html");
  }
}
