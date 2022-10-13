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
        router.Route(parse_result.status, parse_result.request,
                     std::pair<uint16_t, std::string>(4242, "127.0.0.1"));

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
        router.Route(parse_result.status, parse_result.request,
                     std::pair<uint16_t, std::string>(4242, "127.0.0.1"));

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
        router.Route(parse_result.status, parse_result.request,
                     std::pair<uint16_t, std::string>(4242, "127.0.0.1"));

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
        router.Route(parse_result.status, parse_result.request,
                     std::pair<uint16_t, std::string>(2424, "127.0.0.1"));

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
        router.Route(parse_result.status, parse_result.request,
                     std::pair<uint16_t, std::string>(2424, "127.0.0.1"));

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
        router.Route(parse_result.status, parse_result.request,
                     std::pair<uint16_t, std::string>(4242, "127.0.0.1"));

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
        router.Route(parse_result.status, parse_result.request,
                     std::pair<uint16_t, std::string>(2424, "127.0.0.1"));

    EXPECT_EQ(route_result.status, 200);
    EXPECT_EQ(route_result.method, GET);
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
        router.Route(parse_result.status, parse_result.request,
                     std::pair<uint16_t, std::string>(2424, "127.0.0.1"));

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
        router.Route(parse_result.status, parse_result.request,
                     std::pair<uint16_t, std::string>(4242, "127.0.0.1"));

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
        router.Route(parse_result.status, parse_result.request,
                     std::pair<uint16_t, std::string>(4242, "127.0.0.1"));

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
        router.Route(parse_result.status, parse_result.request,
                     std::pair<uint16_t, std::string>(4242, "127.0.0.1"));

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
        router.Route(parse_result.status, parse_result.request,
                     std::pair<uint16_t, std::string>(4242, "127.0.0.1"));

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
        router.Route(parse_result.status, parse_result.request,
                     std::pair<uint16_t, std::string>(4242, "127.0.0.1"));

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
        router.Route(parse_result.status, parse_result.request,
                     std::pair<uint16_t, std::string>(4242, "127.0.0.1"));

    EXPECT_EQ(route_result.status, 403);  // FORBIDDEN
    EXPECT_EQ(route_result.method, GET);
    EXPECT_EQ(route_result.success_path, "./error.html");
    EXPECT_EQ(route_result.error_path, "./error.html");
  }

  // f 06 directory delete
  {
    Validator::Result result =
        TestValidatorSuccess(ROUTER_CONFIG_PATH_PREFIX "f_06");
    PortMap port_map = result.port_map;
    EXPECT_EQ(port_map.size(), 1);
    EXPECT_EQ(port_map.count(4242), 1);

    HttpParser parser;
    std::string req_buf = FileToString(ROUTER_REQ_PATH_PREFIX "f_06.txt");
    int status = parser.Parse(req_buf);
    EXPECT_EQ(status, HttpParser::kClose);
    HttpParser::Result parse_result = parser.get_result();

    Router router(port_map[4242]);
    Router::Result route_result =
        router.Route(parse_result.status, parse_result.request,
                     std::pair<uint16_t, std::string>(4242, "127.0.0.1"));

    EXPECT_EQ(route_result.status, 403);  // FORBIDDEN
    EXPECT_EQ(route_result.method, GET);
    EXPECT_EQ(route_result.success_path, "./error.html");
    EXPECT_EQ(route_result.error_path, "./error.html");
  }
}

TEST(RouterTest, CgiMetaVariables) {
  // s 08 cgi meta-variable test
  {
    Validator::Result result =
        TestValidatorSuccess(ROUTER_CONFIG_PATH_PREFIX "s_08");
    PortMap port_map = result.port_map;
    EXPECT_EQ(port_map.size(), 1);
    EXPECT_EQ(port_map.count(4242), 1);

    HttpParser parser;
    std::string req_buf = FileToString(ROUTER_REQ_PATH_PREFIX "s_08.txt");
    int status = parser.Parse(req_buf);
    EXPECT_EQ(status, HttpParser::kComplete);
    HttpParser::Result parse_result = parser.get_result();

    Router router(port_map[4242]);
    Router::Result route_result =
        router.Route(parse_result.status, parse_result.request,
                     std::pair<uint16_t, std::string>(4242, "127.0.0.1"));

    EXPECT_EQ(route_result.status, 200);
    EXPECT_EQ(route_result.method, GET);
    EXPECT_EQ(route_result.success_path, "./index.php");
    EXPECT_EQ(route_result.error_path, "./error.html");

    std::vector<std::string> env = {
        "AUTH_TYPE=",
        "CONTENT_LENGTH=",
        "CONTENT_TYPE=",
        "GATEWAY_INTERFACE=CGI/1.1",
        "PATH_INFO=",
        "PATH_TRANSLATED=",
        "QUERY_STRING=",
        "REMOTE_ADDR=127.0.0.1",
        "REMOTE_HOST=127.0.0.1",
        "REMOTE_IDENT=",
        "REMOTE_USER=",
        "REQUEST_METHOD=GET",
        "SCRIPT_NAME=/index.php",
        "SERVER_NAME=checkcrlfwhensave",
        "SERVER_PORT=4242",
        "SERVER_PROTOCOL=HTTP/1.1",
        "SERVER_SOFTWARE=BrilliantServer/1.0",
    };

    EXPECT_EQ(route_result.param, "./fastphp_param");
    const char** cgi_envp = route_result.cgi_env.get_env();
    ASSERT_NE(reinterpret_cast<long>(cgi_envp), NULL);
    for (size_t i = 0; i < 17; ++i) {
      if (cgi_envp[i] != NULL) {
        EXPECT_EQ(env[i], cgi_envp[i]);
      } else {
        std::cout << "env[" << i << "] : " << env[i] << " meta variable is NULL"
                  << std::endl;
        ASSERT_TRUE(false);
        ASSERT_TRUE(true);
      }
    }
  }

  // s 09 cgi meta-variable with content-type
  {
    Validator::Result result =
        TestValidatorSuccess(ROUTER_CONFIG_PATH_PREFIX "s_09");
    PortMap port_map = result.port_map;
    EXPECT_EQ(port_map.size(), 1);
    EXPECT_EQ(port_map.count(4242), 1);

    HttpParser parser;
    std::string req_buf = FileToString(ROUTER_REQ_PATH_PREFIX "s_09.txt");
    int status = parser.Parse(req_buf);
    EXPECT_EQ(status, HttpParser::kClose);
    HttpParser::Result parse_result = parser.get_result();

    Router router(port_map[4242]);
    Router::Result route_result =
        router.Route(parse_result.status, parse_result.request,
                     std::pair<uint16_t, std::string>(4242, "127.0.0.1"));

    EXPECT_EQ(route_result.status, 200);
    EXPECT_EQ(route_result.method, POST);
    EXPECT_EQ(route_result.success_path, "./index.php");
    EXPECT_EQ(route_result.error_path, "./error.html");

    char hostname[256];
    memset(hostname, 0, sizeof(hostname));
    gethostname(hostname, 255);
    struct hostent* host = gethostbyname(hostname);
    std::string ip = inet_ntoa(*(struct in_addr*)host->h_addr_list[0]);

    std::vector<std::string> env = {
        "AUTH_TYPE=",
        "CONTENT_LENGTH=10",
        "CONTENT_TYPE=text/html; charset=\"ISO-8859-4\"",
        "GATEWAY_INTERFACE=CGI/1.1",
        "PATH_INFO=",
        "PATH_TRANSLATED=",
        "QUERY_STRING=",
        "REMOTE_ADDR=127.0.0.1",
        "REMOTE_HOST=127.0.0.1",
        "REMOTE_IDENT=",
        "REMOTE_USER=",
        "REQUEST_METHOD=POST",
        "SCRIPT_NAME=/index.php",
        "SERVER_NAME=" + ip,
        "SERVER_PORT=4242",
        "SERVER_PROTOCOL=HTTP/1.0",
        "SERVER_SOFTWARE=BrilliantServer/1.0",
    };

    EXPECT_EQ(route_result.param, "./fastphp_param");
    const char** cgi_envp = route_result.cgi_env.get_env();
    ASSERT_NE(reinterpret_cast<long>(cgi_envp), NULL);
    for (size_t i = 0; i < 17; ++i) {
      if (cgi_envp[i] != NULL) {
        EXPECT_EQ(env[i], cgi_envp[i]);
      } else {
        std::cout << "env[" << i << "] : " << env[i] << " meta variable is NULL"
                  << std::endl;
        ASSERT_TRUE(false);
        ASSERT_TRUE(true);
      }
    }
  }
}