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
#include "Utils.hpp"
#include "Validator.hpp"

#define ROUTER_CONFIG_PATH_PREFIX "../configs/tests/router/"
#define ROUTER_REQ_PATH_PREFIX "../tests/router/"

std::string FileToString(const std::string& file_path);
ServerConfig TestValidatorSuccess(const std::string& case_id);

// Route() 결과값
/*
result{
        is_cgi
        status
        method
        success_path
        error_path
}
*/
//

// .txt (request) , .config (config)
TEST(RouteTest, ServerRouter) {
  {
    ServerConfig result =
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
    Router::Result router_result =
        router.Route(parse_result.status, parse_result.request,
                     ConnectionInfo(4242, "127.0.0.1"));

    EXPECT_EQ(router_result.status, 200);
    EXPECT_TRUE((router_result.methods & parse_result.request.req.method) > 0);
    EXPECT_EQ(router_result.success_path, "./normal/index.html");
    EXPECT_EQ(router_result.error_path, "./error.html");
    EXPECT_FALSE(router_result.is_cgi);
  }

  {
    ServerConfig result =
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
    Router::Result router_result =
        router.Route(parse_result.status, parse_result.request,
                     ConnectionInfo(4242, "127.0.0.1"));

    EXPECT_EQ(router_result.status, 200);
    EXPECT_TRUE((router_result.methods & parse_result.request.req.method) > 0);
    EXPECT_EQ(router_result.success_path,
              "./root_inside/test/index.html/index.html");
    EXPECT_EQ(router_result.error_path, "./blah/404.html");
  }

  {
    ServerConfig result =
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
    Router::Result router_result =
        router.Route(parse_result.status, parse_result.request,
                     ConnectionInfo(4242, "127.0.0.1"));

    EXPECT_EQ(router_result.status, 404);
    EXPECT_TRUE((router_result.methods & parse_result.request.req.method) > 0);
    EXPECT_EQ(router_result.success_path, "./blah/404.html");
    EXPECT_EQ(router_result.error_path, "./blah/404.html");
  }

  {
    ServerConfig result =
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
    Router::Result router_result =
        router.Route(parse_result.status, parse_result.request,
                     ConnectionInfo(2424, "127.0.0.1"));

    EXPECT_EQ(router_result.status, 200);
    EXPECT_TRUE((router_result.methods & parse_result.request.req.method) > 0);
    EXPECT_EQ(router_result.success_path, "./ghan/index.html");
    EXPECT_EQ(router_result.error_path, "./ghan.error.html");
  }

  {
    ServerConfig result =
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
    Router::Result router_result =
        router.Route(parse_result.status, parse_result.request,
                     ConnectionInfo(2424, "127.0.0.1"));

    EXPECT_EQ(router_result.status, 200);
    EXPECT_TRUE((router_result.methods & parse_result.request.req.method) > 0);
    EXPECT_EQ(router_result.success_path, "./yongjule/index.html");
    EXPECT_EQ(router_result.error_path, "./yongjule.error.html");
  }

  {
    ServerConfig result =
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
    Router::Result router_result =
        router.Route(parse_result.status, parse_result.request,
                     ConnectionInfo(4242, "127.0.0.1"));

    EXPECT_EQ(router_result.status, 400);
    EXPECT_TRUE((router_result.methods & parse_result.request.req.method) > 0);
    EXPECT_EQ(router_result.success_path, "./f_01.html");
    EXPECT_EQ(router_result.error_path, "./f_01.html");
  }

  {
    ServerConfig result =
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
    Router::Result router_result =
        router.Route(parse_result.status, parse_result.request,
                     ConnectionInfo(2424, "127.0.0.1"));

    EXPECT_EQ(router_result.status, 200);
    EXPECT_TRUE((router_result.methods & parse_result.request.req.method) > 0);
    EXPECT_EQ(router_result.success_path, "./yongjule/index.html");
    EXPECT_EQ(router_result.error_path, "./yongjule.error.html");
  }
}

TEST(RouterTest, LocationRouter) {
  {
    ServerConfig result =
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
    Router::Result router_result =
        router.Route(parse_result.status, parse_result.request,
                     ConnectionInfo(2424, "127.0.0.1"));

    EXPECT_EQ(router_result.status, 405);
    EXPECT_TRUE((router_result.methods & GET) == 0);
    EXPECT_EQ(router_result.success_path, "./yongjule.error.html");
    EXPECT_EQ(router_result.error_path, "./yongjule.error.html");
  }

  {
    ServerConfig result =
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
    Router::Result router_result =
        router.Route(parse_result.status, parse_result.request,
                     ConnectionInfo(4242, "127.0.0.1"));

    EXPECT_EQ(router_result.status, 405);
    EXPECT_TRUE((router_result.methods & parse_result.request.req.method) == 0);
    EXPECT_EQ(router_result.success_path, "./merong_jiskim");
    EXPECT_EQ(router_result.error_path, "./merong_jiskim");
  }

  {
    ServerConfig result =
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
    Router::Result router_result =
        router.Route(parse_result.status, parse_result.request,
                     ConnectionInfo(4242, "127.0.0.1"));

    EXPECT_EQ(router_result.status, 413);
    EXPECT_TRUE((router_result.methods & parse_result.request.req.method) > 0);
    EXPECT_EQ(router_result.success_path, "./body_max.html");
    EXPECT_EQ(router_result.error_path, "./body_max.html");
  }

  {
    ServerConfig result =
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
    Router::Result router_result =
        router.Route(parse_result.status, parse_result.request,
                     ConnectionInfo(4242, "127.0.0.1"));

    EXPECT_EQ(router_result.status, 200);
    EXPECT_TRUE((router_result.methods & parse_result.request.req.method) > 0);
    EXPECT_EQ(router_result.success_path, "./a/b/a/b/zjj");
    EXPECT_EQ(router_result.error_path, "./f/irst/error.html");
  }

  {
    ServerConfig result =
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
    Router::Result router_result =
        router.Route(parse_result.status, parse_result.request,
                     ConnectionInfo(4242, "127.0.0.1"));

    EXPECT_EQ(router_result.status, 200);
    EXPECT_TRUE((router_result.methods & parse_result.request.req.method) > 0);
    EXPECT_EQ(router_result.success_path, "./path/to/php/index.php");
    EXPECT_EQ(router_result.error_path, "./error.html");
    EXPECT_TRUE(router_result.is_cgi);
  }

  {
    ServerConfig result =
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
    Router::Result router_result =
        router.Route(parse_result.status, parse_result.request,
                     ConnectionInfo(4242, "127.0.0.1"));

    EXPECT_EQ(router_result.status, 200);
    EXPECT_TRUE((router_result.methods & parse_result.request.req.method) > 0);
    EXPECT_EQ(router_result.success_path, "./root/upload_path/upload/file.txt");
    EXPECT_EQ(router_result.error_path, "./error.html");
  }

  {
    ServerConfig result =
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
    Router::Result router_result =
        router.Route(parse_result.status, parse_result.request,
                     ConnectionInfo(4242, "127.0.0.1"));

    EXPECT_EQ(router_result.status, 403);  // FORBIDDEN
    EXPECT_TRUE((router_result.methods & parse_result.request.req.method) > 0);
    EXPECT_EQ(router_result.success_path, "./error.html");
    EXPECT_EQ(router_result.error_path, "./error.html");
  }

  // f 06 directory delete
  {
    ServerConfig result =
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
    Router::Result router_result =
        router.Route(parse_result.status, parse_result.request,
                     ConnectionInfo(4242, "127.0.0.1"));

    EXPECT_EQ(router_result.status, 403);  // FORBIDDEN
    EXPECT_TRUE((router_result.methods & parse_result.request.req.method) > 0);
    EXPECT_EQ(router_result.success_path, "./error.html");
    EXPECT_EQ(router_result.error_path, "./error.html");
  }
}

TEST(RouterTest, CgiMetaVariables) {
  // s 08 cgi meta-variable test
  {
    ServerConfig result =
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
    Router::Result router_result =
        router.Route(parse_result.status, parse_result.request,
                     ConnectionInfo(4242, "127.0.0.1"));

    EXPECT_EQ(router_result.status, 200);
    EXPECT_TRUE((router_result.methods & parse_result.request.req.method) > 0);
    EXPECT_EQ(router_result.success_path, "./index.php");
    EXPECT_EQ(router_result.error_path, "./error.html");

    char hostname[256];
    memset(hostname, 0, sizeof(hostname));
    gethostname(hostname, 255);
    struct hostent* host = gethostbyname(hostname);
    std::string ip = inet_ntoa(*(struct in_addr*)host->h_addr_list[0]);

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
        "SERVER_NAME=" + ip,
        "SERVER_PORT=4242",
        "SERVER_PROTOCOL=HTTP/1.1",
        "SERVER_SOFTWARE=BrilliantServer/1.0",
    };

    EXPECT_TRUE(router_result.is_cgi);
    const char** cgi_envp = router_result.cgi_env.get_env();
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
    ServerConfig result =
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
    Router::Result router_result =
        router.Route(parse_result.status, parse_result.request,
                     ConnectionInfo(4242, "127.0.0.1"));

    EXPECT_EQ(router_result.status, 200);
    EXPECT_TRUE((router_result.methods & parse_result.request.req.method) > 0);
    EXPECT_EQ(router_result.success_path, "./index.php");
    EXPECT_EQ(router_result.error_path, "./error.html");

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

    EXPECT_TRUE(router_result.is_cgi);
    const char** cgi_envp = router_result.cgi_env.get_env();
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

  // s 10 cgi meta-variable check script-uri components
  {
    ServerConfig result =
        TestValidatorSuccess(ROUTER_CONFIG_PATH_PREFIX "s_10");
    PortMap port_map = result.port_map;
    EXPECT_EQ(port_map.size(), 1);
    EXPECT_EQ(port_map.count(4242), 1);

    HttpParser parser;
    std::string req_buf = FileToString(ROUTER_REQ_PATH_PREFIX "s_10.txt");
    int status = parser.Parse(req_buf);
    EXPECT_EQ(status, HttpParser::kComplete);
    HttpParser::Result parse_result = parser.get_result();

    Router router(port_map[4242]);
    Router::Result router_result =
        router.Route(parse_result.status, parse_result.request,
                     ConnectionInfo(4242, "127.0.0.1"));

    EXPECT_EQ(router_result.status, 200);
    EXPECT_TRUE((router_result.methods & parse_result.request.req.method) > 0);
    EXPECT_EQ(router_result.success_path, "./root/after_root/script.php");
    EXPECT_EQ(router_result.error_path, "./error.html");

    char hostname[256];
    memset(hostname, 0, sizeof(hostname));
    gethostname(hostname, 255);
    struct hostent* host = gethostbyname(hostname);
    std::string ip = inet_ntoa(*(struct in_addr*)host->h_addr_list[0]);

    std::vector<std::string> env = {
        "AUTH_TYPE=",
        "CONTENT_LENGTH=8",
        "CONTENT_TYPE=application/octet-stream; charset=\"utf-8\"",
        "GATEWAY_INTERFACE=CGI/1.1",
        "PATH_INFO=/path_info/sub_path_info",
        "PATH_TRANSLATED=" + std::string(getenv("PWD")) +
            "/root/"
            "path_info/sub_path_info",
        "QUERY_STRING=?%EC%9D%B4%EA%B2%83%EC%9D%80=QStr",
        "REMOTE_ADDR=127.0.0.1",
        "REMOTE_HOST=127.0.0.1",
        "REMOTE_IDENT=",
        "REMOTE_USER=",
        "REQUEST_METHOD=POST",
        "SCRIPT_NAME=/root/after_root/script.php",
        "SERVER_NAME=jiskim:4242",
        "SERVER_PORT=4242",
        "SERVER_PROTOCOL=HTTP/1.1",
        "SERVER_SOFTWARE=BrilliantServer/1.0",
    };

    EXPECT_TRUE(router_result.is_cgi);
    const char** cgi_envp = router_result.cgi_env.get_env();
    ASSERT_NE(reinterpret_cast<long>(cgi_envp), NULL);
    for (size_t i = 0; i < 17; ++i) {
      if (cgi_envp[i] != NULL) {
        EXPECT_EQ(env[i], cgi_envp[i]);
      } else {
        std::cout << "\n\nenv[" << i << "] : " << env[i]
                  << " meta variable is NULL\n\n";
        ASSERT_TRUE(false);
      }
    }
  }

  // s 11 cgi meta-variable check script-uri components
  {
    ServerConfig result =
        TestValidatorSuccess(ROUTER_CONFIG_PATH_PREFIX "s_11");
    PortMap port_map = result.port_map;
    EXPECT_EQ(port_map.size(), 1);
    EXPECT_EQ(port_map.count(4242), 1);

    HttpParser parser;
    std::string req_buf = FileToString(ROUTER_REQ_PATH_PREFIX "s_11.txt");
    int status = parser.Parse(req_buf);
    EXPECT_EQ(status, HttpParser::kComplete);
    HttpParser::Result parse_result = parser.get_result();

    Router router(port_map[4242]);
    Router::Result router_result =
        router.Route(parse_result.status, parse_result.request,
                     ConnectionInfo(4242, "127.0.0.1"));

    EXPECT_EQ(router_result.status, 200);
    EXPECT_TRUE((router_result.methods & parse_result.request.req.method) > 0);
    EXPECT_EQ(router_result.success_path, "./script.php");
    EXPECT_EQ(router_result.error_path, "./error.html");

    char hostname[256];
    memset(hostname, 0, sizeof(hostname));
    gethostname(hostname, 255);
    struct hostent* host = gethostbyname(hostname);
    std::string ip = inet_ntoa(*(struct in_addr*)host->h_addr_list[0]);

    std::vector<std::string> env = {
        "AUTH_TYPE=",
        "CONTENT_LENGTH=8",
        "CONTENT_TYPE=application/octet-stream; charset=\"utf-8\"",
        "GATEWAY_INTERFACE=CGI/1.1",
        "PATH_INFO=/path_info",
        "PATH_TRANSLATED=" + std::string(getenv("PWD")) + "/path_info",
        "QUERY_STRING=?%EC%9D%B4%EA%B2%83%EC%9D%80=QStr",
        "REMOTE_ADDR=127.0.0.1",
        "REMOTE_HOST=127.0.0.1",
        "REMOTE_IDENT=",
        "REMOTE_USER=",
        "REQUEST_METHOD=POST",
        "SCRIPT_NAME=/script.php",
        "SERVER_NAME=" + ip,
        "SERVER_PORT=4242",
        "SERVER_PROTOCOL=HTTP/1.1",
        "SERVER_SOFTWARE=BrilliantServer/1.0",
    };

    EXPECT_TRUE(router_result.is_cgi);
    const char** cgi_envp = router_result.cgi_env.get_env();
    ASSERT_NE(reinterpret_cast<long>(cgi_envp), NULL);
    for (size_t i = 0; i < 17; ++i) {
      if (cgi_envp[i] != NULL) {
        EXPECT_EQ(env[i], cgi_envp[i]);
      } else {
        std::cout << "\n\nenv[" << i << "] : " << env[i]
                  << " meta variable is NULL\n\n";
        ASSERT_TRUE(false);
      }
    }
  }

  // s 12 cgi meta-variable check server_name (default server with
  // anonymous host name)
  {
    ServerConfig result =
        TestValidatorSuccess(ROUTER_CONFIG_PATH_PREFIX "s_12");
    PortMap port_map = result.port_map;
    EXPECT_EQ(port_map.size(), 1);
    EXPECT_EQ(port_map.count(4242), 1);

    HttpParser parser;
    std::string req_buf = FileToString(ROUTER_REQ_PATH_PREFIX "s_12.txt");
    int status = parser.Parse(req_buf);
    EXPECT_EQ(status, HttpParser::kComplete);
    HttpParser::Result parse_result = parser.get_result();

    Router router(port_map[4242]);
    Router::Result router_result =
        router.Route(parse_result.status, parse_result.request,
                     ConnectionInfo(4242, "127.0.0.1"));

    EXPECT_EQ(router_result.status, 200);
    EXPECT_TRUE((router_result.methods & parse_result.request.req.method) > 0);
    EXPECT_EQ(router_result.success_path, "./index.php");
    EXPECT_EQ(router_result.error_path, "./error.html");

    char hostname[256];
    memset(hostname, 0, sizeof(hostname));
    gethostname(hostname, 255);
    struct hostent* host = gethostbyname(hostname);
    std::string ip = inet_ntoa(*(struct in_addr*)host->h_addr_list[0]);

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
        "SERVER_NAME=" + ip,
        "SERVER_PORT=4242",
        "SERVER_PROTOCOL=HTTP/1.1",
        "SERVER_SOFTWARE=BrilliantServer/1.0",
    };

    EXPECT_TRUE(router_result.is_cgi);
    const char** cgi_envp = router_result.cgi_env.get_env();
    ASSERT_NE(reinterpret_cast<long>(cgi_envp), NULL);
    for (size_t i = 0; i < 17; ++i) {
      if (cgi_envp[i] != NULL) {
        EXPECT_EQ(env[i], cgi_envp[i]);
      } else {
        std::cout << "\n\nenv[" << i << "] : " << env[i]
                  << " meta variable is NULL\n\n";
        ASSERT_TRUE(false);
      }
    }
  }

  // s 13 cgi meta-variable check server_name (default server with
  // default's host name)
  {
    ServerConfig result =
        TestValidatorSuccess(ROUTER_CONFIG_PATH_PREFIX "s_13");
    PortMap port_map = result.port_map;
    EXPECT_EQ(port_map.size(), 1);
    EXPECT_EQ(port_map.count(4242), 1);

    HttpParser parser;
    std::string req_buf = FileToString(ROUTER_REQ_PATH_PREFIX "s_13.txt");
    int status = parser.Parse(req_buf);
    EXPECT_EQ(status, HttpParser::kComplete);
    HttpParser::Result parse_result = parser.get_result();

    Router router(port_map[4242]);
    Router::Result router_result =
        router.Route(parse_result.status, parse_result.request,
                     ConnectionInfo(4242, "127.0.0.1"));

    EXPECT_EQ(router_result.status, 200);
    EXPECT_TRUE((router_result.methods & parse_result.request.req.method) > 0);
    EXPECT_EQ(router_result.success_path, "./index.php");
    EXPECT_EQ(router_result.error_path, "./error.html");

    char hostname[256];
    memset(hostname, 0, sizeof(hostname));
    gethostname(hostname, 255);
    struct hostent* host = gethostbyname(hostname);
    std::string ip = inet_ntoa(*(struct in_addr*)host->h_addr_list[0]);

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
        "SERVER_NAME=ghan",
        "SERVER_PORT=4242",
        "SERVER_PROTOCOL=HTTP/1.1",
        "SERVER_SOFTWARE=BrilliantServer/1.0",
    };

    EXPECT_TRUE(router_result.is_cgi);
    const char** cgi_envp = router_result.cgi_env.get_env();
    ASSERT_NE(reinterpret_cast<long>(cgi_envp), NULL);
    for (size_t i = 0; i < 17; ++i) {
      if (cgi_envp[i] != NULL) {
        EXPECT_EQ(env[i], cgi_envp[i]);
      } else {
        std::cout << "\n\nenv[" << i << "] : " << env[i]
                  << " meta variable is NULL\n\n";
        ASSERT_TRUE(false);
      }
    }
  }
}
