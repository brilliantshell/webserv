#include <gtest/gtest.h>

#include <fstream>
#include <iostream>

#include "CgiManager.hpp"
#include "Connection.hpp"
#include "HttpParser.hpp"
#include "ResponseManager.hpp"
#include "Router.hpp"
#include "Types.hpp"
#include "Validator.hpp"

#define CM_CONFIG_PATH_PREFIX "../configs/tests/CgiManager/"
#define CM_REQ_PATH_PREFIX "../tests/CgiManager/"

std::string FileToString(const std::string& file_path);
ServerConfig TestValidatorSuccess(const std::string& case_id);

TEST(CgiManagerTest, InputOutput) {
  // s 00 general case
  {
    ServerConfig result = TestValidatorSuccess(CM_CONFIG_PATH_PREFIX "s_00");
    PortMap port_map = result.port_map;
    EXPECT_EQ(port_map.size(), 1);
    EXPECT_EQ(port_map.count(4242), 1);

    HttpParser parser;
    std::string req_buf = FileToString(CM_REQ_PATH_PREFIX "s_00.txt");
    int status = parser.Parse(req_buf);
    EXPECT_EQ(status, HttpParser::kComplete);
    HttpParser::Result parse_result = parser.get_result();

    Router router(port_map[4242]);
    Router::Result router_result =
        router.Route(parse_result.status, parse_result.request,
                     ConnectionInfo(4242, "127.0.0.1"));
    EXPECT_EQ(router_result.status, 200);
    EXPECT_TRUE((router_result.methods & parse_result.request.req.method) > 0);
    EXPECT_EQ(router_result.success_path, "./resources/cgi/cgi.php");
    EXPECT_EQ(router_result.error_path, "./error.html");

    ResponseManager rm;
    ResponseManager::Result rm_result =
        rm.ExecuteMethod(router_result, parse_result.request);

    const char** cgi_env = router_result.cgi_env.get_env();
    std::stringstream ss;
    for (int i = 0; cgi_env[i]; ++i)
      ss << "envp [" << i << "] : " << cgi_env[i] << "\n";

    EXPECT_EQ(rm_result.status, 200);
    std::string decoded_query(parse_result.request.req.query);
    UriParser uri_parser;
    for (size_t i = 0; i < decoded_query.size(); ++i) {
      if (decoded_query[i] == '%') {
        uri_parser.DecodeHexToAscii(decoded_query, i);
      }
    }
    EXPECT_EQ(
        rm_result.content,
        ss.str() + "\n" + parse_result.request.content +
            ((!parse_result.request.req.query.empty() &&
              parse_result.request.req.query.find("=") == std::string::npos)
                 ? ("\n" + decoded_query.substr(1))
                 : ""));
    EXPECT_EQ(rm_result.header.count("content-type"), 1);
    EXPECT_EQ(rm_result.header["content-type"], "text/plain");
    EXPECT_EQ(rm_result.location, "");
  }

  // s 01 general case with content
  {
    ServerConfig result = TestValidatorSuccess(CM_CONFIG_PATH_PREFIX "s_01");
    PortMap port_map = result.port_map;
    EXPECT_EQ(port_map.size(), 1);
    EXPECT_EQ(port_map.count(4242), 1);

    HttpParser parser;
    std::string req_buf = FileToString(CM_REQ_PATH_PREFIX "s_01.txt");
    int status = parser.Parse(req_buf);
    EXPECT_EQ(status, HttpParser::kComplete);
    HttpParser::Result parse_result = parser.get_result();

    Router router(port_map[4242]);
    Router::Result router_result =
        router.Route(parse_result.status, parse_result.request,
                     ConnectionInfo(4242, "127.0.0.1"));
    EXPECT_EQ(router_result.status, 200);
    EXPECT_TRUE((router_result.methods & parse_result.request.req.method) > 0);
    EXPECT_EQ(router_result.success_path, "./resources/cgi/cgi.php");
    EXPECT_EQ(router_result.error_path, "./error.html");

    ResponseManager rm;
    ResponseManager::Result rm_result =
        rm.ExecuteMethod(router_result, parse_result.request);

    const char** cgi_env = router_result.cgi_env.get_env();
    std::stringstream ss;
    for (int i = 0; cgi_env[i]; ++i)
      ss << "envp [" << i << "] : " << cgi_env[i] << "\n";

    EXPECT_EQ(rm_result.status, 200);
    std::string decoded_query(parse_result.request.req.query);
    UriParser uri_parser;
    for (size_t i = 0; i < decoded_query.size(); ++i) {
      if (decoded_query[i] == '%') {
        uri_parser.DecodeHexToAscii(decoded_query, i);
      }
    }
    EXPECT_EQ(
        rm_result.content,
        ss.str() + "\n" + parse_result.request.content +
            ((!parse_result.request.req.query.empty() &&
              parse_result.request.req.query.find("=") == std::string::npos)
                 ? ("\n" + decoded_query.substr(1))
                 : ""));
    EXPECT_EQ(rm_result.header.count("content-type"), 1);
    EXPECT_EQ(rm_result.header["content-type"], "text/plain");
    EXPECT_EQ(rm_result.location, "");
  }

  // s 02 cgi command line
  {
    ServerConfig result = TestValidatorSuccess(CM_CONFIG_PATH_PREFIX "s_02");
    PortMap port_map = result.port_map;
    EXPECT_EQ(port_map.size(), 1);
    EXPECT_EQ(port_map.count(4242), 1);

    HttpParser parser;
    std::string req_buf = FileToString(CM_REQ_PATH_PREFIX "s_02.txt");
    int status = parser.Parse(req_buf);
    EXPECT_EQ(status, HttpParser::kComplete);
    HttpParser::Result parse_result = parser.get_result();

    Router router(port_map[4242]);
    Router::Result router_result =
        router.Route(parse_result.status, parse_result.request,
                     ConnectionInfo(4242, "127.0.0.1"));
    EXPECT_EQ(router_result.status, 200);
    EXPECT_TRUE((router_result.methods & parse_result.request.req.method) > 0);
    EXPECT_EQ(router_result.success_path, "./resources/cgi/cgi.php");
    EXPECT_EQ(router_result.error_path, "./error.html");

    ResponseManager rm;
    ResponseManager::Result rm_result =
        rm.ExecuteMethod(router_result, parse_result.request);

    const char** cgi_env = router_result.cgi_env.get_env();
    std::stringstream ss;
    for (int i = 0; cgi_env[i]; ++i)
      ss << "envp [" << i << "] : " << cgi_env[i] << "\n";

    ASSERT_EQ(rm_result.status, 200);
    std::string decoded_query(parse_result.request.req.query);
    UriParser uri_parser;
    for (size_t i = 0; i < decoded_query.size(); ++i) {
      if (decoded_query[i] == '%') {
        uri_parser.DecodeHexToAscii(decoded_query, i);
      }
    }
    EXPECT_EQ(
        rm_result.content,
        ss.str() + "\n" + parse_result.request.content +
            ((!parse_result.request.req.query.empty() &&
              parse_result.request.req.query.find("=") == std::string::npos)
                 ? ("\n" + decoded_query.substr(1))
                 : ""));
    EXPECT_EQ(rm_result.header.count("content-type"), 1);
    EXPECT_EQ(rm_result.header["content-type"], "text/plain");
    EXPECT_EQ(rm_result.location, "");
  }

  // s 03 cgi command line - decoding encoded query
  {
    ServerConfig result = TestValidatorSuccess(CM_CONFIG_PATH_PREFIX "s_03");
    PortMap port_map = result.port_map;
    EXPECT_EQ(port_map.size(), 1);
    EXPECT_EQ(port_map.count(4242), 1);

    HttpParser parser;
    std::string req_buf = FileToString(CM_REQ_PATH_PREFIX "s_03.txt");
    int status = parser.Parse(req_buf);
    EXPECT_EQ(status, HttpParser::kComplete);
    HttpParser::Result parse_result = parser.get_result();

    Router router(port_map[4242]);
    Router::Result router_result =
        router.Route(parse_result.status, parse_result.request,
                     ConnectionInfo(4242, "127.0.0.1"));
    EXPECT_EQ(router_result.status, 200);
    EXPECT_TRUE((router_result.methods & parse_result.request.req.method) > 0);
    EXPECT_EQ(router_result.success_path, "./resources/cgi/cgi.php");
    EXPECT_EQ(router_result.error_path, "./error.html");

    ResponseManager rm;
    ResponseManager::Result rm_result =
        rm.ExecuteMethod(router_result, parse_result.request);

    const char** cgi_env = router_result.cgi_env.get_env();
    std::stringstream ss;
    for (int i = 0; cgi_env[i]; ++i)
      ss << "envp [" << i << "] : " << cgi_env[i] << "\n";

    ASSERT_EQ(rm_result.status, 200);
    std::string decoded_query(parse_result.request.req.query);
    UriParser uri_parser;
    for (size_t i = 0; i < decoded_query.size(); ++i) {
      if (decoded_query[i] == '%') {
        uri_parser.DecodeHexToAscii(decoded_query, i);
      }
    }
    EXPECT_EQ(
        rm_result.content,
        ss.str() + "\n" + parse_result.request.content +
            ((!parse_result.request.req.query.empty() &&
              parse_result.request.req.query.find("=") == std::string::npos)
                 ? ("\n" + decoded_query.substr(1))
                 : ""));
    EXPECT_EQ(rm_result.header.count("content-type"), 1);
    EXPECT_EQ(rm_result.header["content-type"], "text/plain");
    EXPECT_EQ(rm_result.location, "");
  }
}

TEST(CgiManagerTest, ParseCgiResponse) {
  // s 04 successful document response
  {
    ServerConfig result = TestValidatorSuccess(CM_CONFIG_PATH_PREFIX "s_04");
    PortMap port_map = result.port_map;
    EXPECT_EQ(port_map.size(), 1);
    EXPECT_EQ(port_map.count(4242), 1);

    HttpParser parser;
    std::string req_buf = FileToString(CM_REQ_PATH_PREFIX "s_04.txt");
    int status = parser.Parse(req_buf);
    EXPECT_EQ(status, HttpParser::kComplete);
    HttpParser::Result parse_result = parser.get_result();

    Router router(port_map[4242]);
    Router::Result router_result =
        router.Route(parse_result.status, parse_result.request,
                     ConnectionInfo(4242, "127.0.0.1"));
    EXPECT_EQ(router_result.status, 200);
    EXPECT_TRUE((router_result.methods & parse_result.request.req.method) > 0);
    EXPECT_EQ(router_result.success_path, "./resources/cgi/cgi.php");
    EXPECT_EQ(router_result.error_path, "./error.html");

    ResponseManager rm;

    ResponseManager::Result rm_result =
        rm.ExecuteMethod(router_result, parse_result.request);

    const char** cgi_env = router_result.cgi_env.get_env();
    std::stringstream ss;
    for (int i = 0; cgi_env[i]; ++i)
      ss << "envp [" << i << "] : " << cgi_env[i] << "\n";

    ASSERT_EQ(rm_result.status, 400);
    std::string decoded_query(parse_result.request.req.query);
    UriParser uri_parser;
    for (size_t i = 0; i < decoded_query.size(); ++i) {
      if (decoded_query[i] == '%') {
        uri_parser.DecodeHexToAscii(decoded_query, i);
      }
    }
    EXPECT_EQ(rm_result.content,
              "<!DOCTYPE html><title>400 Bad Request</title><body><h1>400 Bad \
Request</h1></body></html>");
  }

  // s 05 successful document response
  {
    ServerConfig result = TestValidatorSuccess(CM_CONFIG_PATH_PREFIX "s_05");
    PortMap port_map = result.port_map;
    EXPECT_EQ(port_map.size(), 1);
    EXPECT_EQ(port_map.count(4242), 1);

    HttpParser parser;
    std::string req_buf = FileToString(CM_REQ_PATH_PREFIX "s_05.txt");
    int status = parser.Parse(req_buf);
    EXPECT_EQ(status, HttpParser::kComplete);
    HttpParser::Result parse_result = parser.get_result();

    Router router(port_map[4242]);
    Router::Result router_result =
        router.Route(parse_result.status, parse_result.request,
                     ConnectionInfo(4242, "127.0.0.1"));
    EXPECT_EQ(router_result.status, 200);
    EXPECT_TRUE((router_result.methods & parse_result.request.req.method) > 0);
    EXPECT_EQ(router_result.success_path, "./resources/cgi/cgi.php");
    EXPECT_EQ(router_result.error_path, "./error.html");

    ResponseManager rm;
    ResponseManager::Result rm_result =
        rm.ExecuteMethod(router_result, parse_result.request);

    const char** cgi_env = router_result.cgi_env.get_env();
    std::stringstream ss;
    for (int i = 0; cgi_env[i]; ++i)
      ss << "envp [" << i << "] : " << cgi_env[i] << "\n";

    ASSERT_EQ(rm_result.status, 200);
    std::string decoded_query(parse_result.request.req.query);
    UriParser uri_parser;
    for (size_t i = 0; i < decoded_query.size(); ++i) {
      if (decoded_query[i] == '%') {
        uri_parser.DecodeHexToAscii(decoded_query, i);
      }
    }
    EXPECT_EQ(
        rm_result.content,
        ss.str() + "\n" + parse_result.request.content +
            ((!parse_result.request.req.query.empty() &&
              parse_result.request.req.query.find("=") == std::string::npos)
                 ? ("\n" + decoded_query.substr(1))
                 : ""));
    EXPECT_EQ(rm_result.header.size(), 1);
    EXPECT_EQ(rm_result.header.count("content-type"), 1);
    EXPECT_EQ(rm_result.header["content-type"], "text/plain");
    EXPECT_EQ(rm_result.location, "");
  }

  // f 00 cgi max content size exceed
  {
    ServerConfig result = TestValidatorSuccess(CM_CONFIG_PATH_PREFIX "f_00");
    PortMap port_map = result.port_map;
    EXPECT_EQ(port_map.size(), 1);
    EXPECT_EQ(port_map.count(4242), 1);

    HttpParser parser;
    std::string req_buf = FileToString(CM_REQ_PATH_PREFIX "f_00.txt");
    int status = parser.Parse(req_buf);
    EXPECT_EQ(status, HttpParser::kClose);
    HttpParser::Result parse_result = parser.get_result();

    Router router(port_map[4242]);
    Router::Result router_result =
        router.Route(parse_result.status, parse_result.request,
                     ConnectionInfo(4242, "127.0.0.1"));
    EXPECT_EQ(router_result.status, 200);
    EXPECT_TRUE((router_result.methods & parse_result.request.req.method) > 0);
    EXPECT_EQ(router_result.success_path, "./resources/cgi/cgi_max.php");
    EXPECT_EQ(router_result.error_path, "./error.html");

    ResponseManager rm;
    ResponseManager::Result rm_result =
        rm.ExecuteMethod(router_result, parse_result.request);

    ASSERT_EQ(rm_result.status, 500);
  }

  // f 01 cgi max header size exceed
  {
    ServerConfig result = TestValidatorSuccess(CM_CONFIG_PATH_PREFIX "f_01");
    PortMap port_map = result.port_map;
    EXPECT_EQ(port_map.size(), 1);
    EXPECT_EQ(port_map.count(4242), 1);

    HttpParser parser;
    std::string req_buf = FileToString(CM_REQ_PATH_PREFIX "f_01.txt");
    int status = parser.Parse(req_buf);
    EXPECT_EQ(status, HttpParser::kClose);
    HttpParser::Result parse_result = parser.get_result();

    Router router(port_map[4242]);
    Router::Result router_result =
        router.Route(parse_result.status, parse_result.request,
                     ConnectionInfo(4242, "127.0.0.1"));
    EXPECT_EQ(router_result.status, 200);
    EXPECT_TRUE((router_result.methods & parse_result.request.req.method) > 0);
    EXPECT_EQ(router_result.success_path, "./resources/cgi/cgi_max.php");
    EXPECT_EQ(router_result.error_path, "./error.html");

    ResponseManager rm;
    ResponseManager::Result rm_result =
        rm.ExecuteMethod(router_result, parse_result.request);

    ASSERT_EQ(rm_result.status, 500);
  }

  // f 02 cgi max field size exceed
  {
    ServerConfig result = TestValidatorSuccess(CM_CONFIG_PATH_PREFIX "f_02");
    PortMap port_map = result.port_map;
    EXPECT_EQ(port_map.size(), 1);
    EXPECT_EQ(port_map.count(4242), 1);

    HttpParser parser;
    std::string req_buf = FileToString(CM_REQ_PATH_PREFIX "f_02.txt");
    int status = parser.Parse(req_buf);
    EXPECT_EQ(status, HttpParser::kClose);
    HttpParser::Result parse_result = parser.get_result();

    Router router(port_map[4242]);
    Router::Result router_result =
        router.Route(parse_result.status, parse_result.request,
                     ConnectionInfo(4242, "127.0.0.1"));
    EXPECT_EQ(router_result.status, 200);
    EXPECT_TRUE((router_result.methods & parse_result.request.req.method) > 0);
    EXPECT_EQ(router_result.success_path, "./resources/cgi/cgi_max.php");
    EXPECT_EQ(router_result.error_path, "./error.html");

    ResponseManager rm;
    ResponseManager::Result rm_result =
        rm.ExecuteMethod(router_result, parse_result.request);

    ASSERT_EQ(rm_result.status, 500);
  }

  // f 02 cgi max field size exceed
  {
    ServerConfig result = TestValidatorSuccess(CM_CONFIG_PATH_PREFIX "f_02");
    PortMap port_map = result.port_map;
    EXPECT_EQ(port_map.size(), 1);
    EXPECT_EQ(port_map.count(4242), 1);

    HttpParser parser;
    std::string req_buf = FileToString(CM_REQ_PATH_PREFIX "f_02.txt");
    int status = parser.Parse(req_buf);
    EXPECT_EQ(status, HttpParser::kClose);
    HttpParser::Result parse_result = parser.get_result();

    Router router(port_map[4242]);
    Router::Result router_result =
        router.Route(parse_result.status, parse_result.request,
                     ConnectionInfo(4242, "127.0.0.1"));
    EXPECT_EQ(router_result.status, 200);
    EXPECT_TRUE((router_result.methods & parse_result.request.req.method) > 0);
    EXPECT_EQ(router_result.success_path, "./resources/cgi/cgi_max.php");
    EXPECT_EQ(router_result.error_path, "./error.html");

    ResponseManager rm;
    ResponseManager::Result rm_result =
        rm.ExecuteMethod(router_result, parse_result.request);

    ASSERT_EQ(rm_result.status, 500);
  }

  // f 03 document response, first header != content-type
  {
    ServerConfig result = TestValidatorSuccess(CM_CONFIG_PATH_PREFIX "f_03");
    PortMap port_map = result.port_map;
    EXPECT_EQ(port_map.size(), 1);
    EXPECT_EQ(port_map.count(4242), 1);

    HttpParser parser;
    std::string req_buf = FileToString(CM_REQ_PATH_PREFIX "f_03.txt");
    int status = parser.Parse(req_buf);
    EXPECT_EQ(status, HttpParser::kComplete);
    HttpParser::Result parse_result = parser.get_result();

    Router router(port_map[4242]);
    Router::Result router_result =
        router.Route(parse_result.status, parse_result.request,
                     ConnectionInfo(4242, "127.0.0.1"));
    EXPECT_EQ(router_result.status, 200);
    EXPECT_TRUE((router_result.methods & parse_result.request.req.method) > 0);
    EXPECT_EQ(router_result.success_path, "./resources/cgi/cgi.php");
    EXPECT_EQ(router_result.error_path, "./error.html");

    ResponseManager rm;
    ResponseManager::Result rm_result =
        rm.ExecuteMethod(router_result, parse_result.request);

    ASSERT_EQ(rm_result.status, 500);
  }

  // s 06 child process correct cwd
  {
    ServerConfig result = TestValidatorSuccess(CM_CONFIG_PATH_PREFIX "s_06");
    PortMap port_map = result.port_map;
    EXPECT_EQ(port_map.size(), 1);
    EXPECT_EQ(port_map.count(4242), 1);

    HttpParser parser;
    std::string req_buf = FileToString(CM_REQ_PATH_PREFIX "s_06.txt");
    int status = parser.Parse(req_buf);
    EXPECT_EQ(status, HttpParser::kComplete);
    HttpParser::Result parse_result = parser.get_result();

    Router router(port_map[4242]);
    Router::Result router_result =
        router.Route(parse_result.status, parse_result.request,
                     ConnectionInfo(4242, "127.0.0.1"));
    EXPECT_EQ(router_result.status, 200);
    EXPECT_TRUE((router_result.methods & parse_result.request.req.method) > 0);
    EXPECT_EQ(router_result.success_path, "./resources/cgi/cgi_cwd.php");
    EXPECT_EQ(router_result.error_path, "./error.html");

    ResponseManager rm;
    ResponseManager::Result rm_result =
        rm.ExecuteMethod(router_result, parse_result.request);

    const char** cgi_env = router_result.cgi_env.get_env();
    std::stringstream ss;
    for (int i = 0; cgi_env[i]; ++i)
      ss << "envp [" << i << "] : " << cgi_env[i] << "\n";

    char proc_name[PROC_PIDPATHINFO_MAXSIZE + 1];
    memset(proc_name, 0, PROC_PIDPATHINFO_MAXSIZE + 1);
    proc_pidpath(getpid(), proc_name, PROC_PIDPATHINFO_MAXSIZE);
    char* cwd = dirname(proc_name);
    char cwd_abs[PROC_PIDPATHINFO_MAXSIZE + 1];
    memset(cwd_abs, 0, PROC_PIDPATHINFO_MAXSIZE + 1);
    strcpy(cwd_abs, cwd);
    strcat(cwd_abs, "/resources/cgi");
    char* resolved = realpath(cwd_abs, NULL);
    ASSERT_EQ(rm_result.status, 200);
    EXPECT_EQ(rm_result.content, "cwd : " + std::string(resolved));
    EXPECT_EQ(rm_result.header.count("content-type"), 1);
    EXPECT_EQ(rm_result.header["content-type"], "text/plain");
    EXPECT_EQ(rm_result.header.count("allow"), 1);
    EXPECT_EQ(rm_result.header["allow"], "GET");
    free(resolved);
  }

  // s 07 cgi local redirection
  {
    ServerConfig result = TestValidatorSuccess(CM_CONFIG_PATH_PREFIX "s_07");
    PortMap port_map = result.port_map;
    EXPECT_EQ(port_map.size(), 1);
    EXPECT_EQ(port_map.count(4242), 1);

    HttpParser parser;
    std::string req_buf = FileToString(CM_REQ_PATH_PREFIX "s_07.txt");
    int status = parser.Parse(req_buf);
    EXPECT_EQ(status, HttpParser::kClose);
    HttpParser::Result parse_result = parser.get_result();

    Router router(port_map[4242]);
    Router::Result router_result =
        router.Route(parse_result.status, parse_result.request,
                     ConnectionInfo(4242, "127.0.0.1"));
    EXPECT_EQ(router_result.status, 200);
    EXPECT_TRUE((router_result.methods & parse_result.request.req.method) > 0);
    EXPECT_EQ(router_result.success_path, "./resources/cgi/cgi_redir.php");
    EXPECT_EQ(router_result.error_path, "./error.html");

    ResponseManager rm;
    ResponseManager::Result rm_result =
        rm.ExecuteMethod(router_result, parse_result.request);

    ASSERT_EQ(rm_result.status, 200);
    EXPECT_EQ(rm_result.header.size(), 1);
    EXPECT_EQ(rm_result.header.count("location"), 1);
    EXPECT_EQ(rm_result.header["location"], "/ghan");
  }

  // f 04 cgi local redirection
  {
    ServerConfig result = TestValidatorSuccess(CM_CONFIG_PATH_PREFIX "f_04");
    PortMap port_map = result.port_map;
    EXPECT_EQ(port_map.size(), 1);
    EXPECT_EQ(port_map.count(4242), 1);

    HttpParser parser;
    std::string req_buf = FileToString(CM_REQ_PATH_PREFIX "f_04.txt");
    int status = parser.Parse(req_buf);
    EXPECT_EQ(status, HttpParser::kClose);
    HttpParser::Result parse_result = parser.get_result();

    Router router(port_map[4242]);
    Router::Result router_result =
        router.Route(parse_result.status, parse_result.request,
                     ConnectionInfo(4242, "127.0.0.1"));
    EXPECT_EQ(router_result.status, 200);
    EXPECT_TRUE((router_result.methods & parse_result.request.req.method) > 0);
    EXPECT_EQ(router_result.success_path, "./resources/cgi/cgi_redir.php");
    EXPECT_EQ(router_result.error_path, "./error.html");

    ResponseManager rm;
    ResponseManager::Result rm_result =
        rm.ExecuteMethod(router_result, parse_result.request);

    ASSERT_EQ(rm_result.status, 500);
  }

  // s 08 cgi local redirection
  {
    ServerConfig result = TestValidatorSuccess(CM_CONFIG_PATH_PREFIX "s_08");
    PortMap port_map = result.port_map;
    EXPECT_EQ(port_map.size(), 1);
    EXPECT_EQ(port_map.count(4242), 1);

    HttpParser parser;
    std::string req_buf = FileToString(CM_REQ_PATH_PREFIX "s_08.txt");
    int status = parser.Parse(req_buf);
    EXPECT_EQ(status, HttpParser::kClose);
    HttpParser::Result parse_result = parser.get_result();

    Router router(port_map[4242]);
    Router::Result router_result =
        router.Route(parse_result.status, parse_result.request,
                     ConnectionInfo(4242, "127.0.0.1"));
    EXPECT_EQ(router_result.status, 200);
    EXPECT_TRUE((router_result.methods & parse_result.request.req.method) > 0);
    EXPECT_EQ(router_result.success_path, "./resources/cgi/cgi_redir.php");
    EXPECT_EQ(router_result.error_path, "./error.html");

    ResponseManager rm;
    ResponseManager::Result rm_result =
        rm.ExecuteMethod(router_result, parse_result.request);

    ASSERT_EQ(rm_result.status, 200);
    EXPECT_EQ(rm_result.header.size(), 1);
    EXPECT_EQ(rm_result.header.count("location"), 1);
    EXPECT_EQ(rm_result.header["location"], "/jiskim");
  }

  // f 05 cgi local redirection
  {
    ServerConfig result = TestValidatorSuccess(CM_CONFIG_PATH_PREFIX "f_05");
    PortMap port_map = result.port_map;
    EXPECT_EQ(port_map.size(), 1);
    EXPECT_EQ(port_map.count(4242), 1);

    HttpParser parser;
    std::string req_buf = FileToString(CM_REQ_PATH_PREFIX "f_05.txt");
    int status = parser.Parse(req_buf);
    EXPECT_EQ(status, HttpParser::kClose);
    HttpParser::Result parse_result = parser.get_result();

    Router router(port_map[4242]);
    Router::Result router_result =
        router.Route(parse_result.status, parse_result.request,
                     ConnectionInfo(4242, "127.0.0.1"));
    EXPECT_EQ(router_result.status, 200);
    EXPECT_TRUE((router_result.methods & parse_result.request.req.method) > 0);
    EXPECT_EQ(router_result.success_path, "./resources/cgi/cgi_redir.php");
    EXPECT_EQ(router_result.error_path, "./error.html");

    ResponseManager rm;
    ResponseManager::Result rm_result =
        rm.ExecuteMethod(router_result, parse_result.request);

    ASSERT_EQ(rm_result.status, 500);
  }

  // s 09 cgi client redirection no body
  {
    ServerConfig result = TestValidatorSuccess(CM_CONFIG_PATH_PREFIX "s_09");
    PortMap port_map = result.port_map;
    EXPECT_EQ(port_map.size(), 1);
    EXPECT_EQ(port_map.count(4242), 1);

    HttpParser parser;
    std::string req_buf = FileToString(CM_REQ_PATH_PREFIX "s_09.txt");
    int status = parser.Parse(req_buf);
    EXPECT_EQ(status, HttpParser::kClose);
    HttpParser::Result parse_result = parser.get_result();

    Router router(port_map[4242]);
    Router::Result router_result =
        router.Route(parse_result.status, parse_result.request,
                     ConnectionInfo(4242, "127.0.0.1"));
    EXPECT_EQ(router_result.status, 200);
    EXPECT_TRUE((router_result.methods & parse_result.request.req.method) > 0);
    EXPECT_EQ(router_result.success_path, "./resources/cgi/cgi_redir.php");
    EXPECT_EQ(router_result.error_path, "./error.html");

    ResponseManager rm;
    ResponseManager::Result rm_result =
        rm.ExecuteMethod(router_result, parse_result.request);

    ASSERT_EQ(rm_result.status, 302);
    EXPECT_EQ(rm_result.header.size(), 1);
    EXPECT_EQ(rm_result.header.count("location"), 1);
    EXPECT_EQ(rm_result.header["location"],
              "http://www.our42vent.42cadet.trashservice.io");
  }

  // s 10 cgi client redirection no body
  {
    ServerConfig result = TestValidatorSuccess(CM_CONFIG_PATH_PREFIX "s_10");
    PortMap port_map = result.port_map;
    EXPECT_EQ(port_map.size(), 1);
    EXPECT_EQ(port_map.count(4242), 1);

    HttpParser parser;
    std::string req_buf = FileToString(CM_REQ_PATH_PREFIX "s_10.txt");
    int status = parser.Parse(req_buf);
    EXPECT_EQ(status, HttpParser::kClose);
    HttpParser::Result parse_result = parser.get_result();

    Router router(port_map[4242]);
    Router::Result router_result =
        router.Route(parse_result.status, parse_result.request,
                     ConnectionInfo(4242, "127.0.0.1"));
    EXPECT_EQ(router_result.status, 200);
    EXPECT_TRUE((router_result.methods & parse_result.request.req.method) > 0);
    EXPECT_EQ(router_result.success_path, "./resources/cgi/cgi_redir.php");
    EXPECT_EQ(router_result.error_path, "./error.html");

    ResponseManager rm;
    ResponseManager::Result rm_result =
        rm.ExecuteMethod(router_result, parse_result.request);

    ASSERT_EQ(rm_result.status, 302);
    EXPECT_EQ(rm_result.header.size(), 1);
    EXPECT_EQ(rm_result.header.count("location"), 1);
    EXPECT_EQ(rm_result.header["location"],
              "http://www.our42vent.42cadet.trashservice.io");
  }

  // f 06 cgi client redirection invalid field
  {
    ServerConfig result = TestValidatorSuccess(CM_CONFIG_PATH_PREFIX "f_06");
    PortMap port_map = result.port_map;
    EXPECT_EQ(port_map.size(), 1);
    EXPECT_EQ(port_map.count(4242), 1);

    HttpParser parser;
    std::string req_buf = FileToString(CM_REQ_PATH_PREFIX "f_06.txt");
    int status = parser.Parse(req_buf);
    EXPECT_EQ(status, HttpParser::kClose);
    HttpParser::Result parse_result = parser.get_result();

    Router router(port_map[4242]);
    Router::Result router_result =
        router.Route(parse_result.status, parse_result.request,
                     ConnectionInfo(4242, "127.0.0.1"));
    EXPECT_EQ(router_result.status, 200);
    EXPECT_TRUE((router_result.methods & parse_result.request.req.method) > 0);
    EXPECT_EQ(router_result.success_path, "./resources/cgi/cgi_redir.php");
    EXPECT_EQ(router_result.error_path, "./error.html");

    ResponseManager rm;
    ResponseManager::Result rm_result =
        rm.ExecuteMethod(router_result, parse_result.request);

    ASSERT_EQ(rm_result.status, 500);
  }

  // f 07 cgi client redirection with body, lacking mandatory fields
  {
    ServerConfig result = TestValidatorSuccess(CM_CONFIG_PATH_PREFIX "f_07");
    PortMap port_map = result.port_map;
    EXPECT_EQ(port_map.size(), 1);
    EXPECT_EQ(port_map.count(4242), 1);

    HttpParser parser;
    std::string req_buf = FileToString(CM_REQ_PATH_PREFIX "f_07.txt");
    int status = parser.Parse(req_buf);
    EXPECT_EQ(status, HttpParser::kClose);
    HttpParser::Result parse_result = parser.get_result();

    Router router(port_map[4242]);
    Router::Result router_result =
        router.Route(parse_result.status, parse_result.request,
                     ConnectionInfo(4242, "127.0.0.1"));
    EXPECT_EQ(router_result.status, 200);
    EXPECT_TRUE((router_result.methods & parse_result.request.req.method) > 0);
    EXPECT_EQ(router_result.success_path, "./resources/cgi/cgi_redir.php");
    EXPECT_EQ(router_result.error_path, "./error.html");

    ResponseManager rm;
    ResponseManager::Result rm_result =
        rm.ExecuteMethod(router_result, parse_result.request);

    ASSERT_EQ(rm_result.status, 500);
  }

  // f 08 cgi response with a repeated field name
  {
    ServerConfig result = TestValidatorSuccess(CM_CONFIG_PATH_PREFIX "f_08");
    PortMap port_map = result.port_map;
    EXPECT_EQ(port_map.size(), 1);
    EXPECT_EQ(port_map.count(4242), 1);

    HttpParser parser;
    std::string req_buf = FileToString(CM_REQ_PATH_PREFIX "f_08.txt");
    int status = parser.Parse(req_buf);
    EXPECT_EQ(status, HttpParser::kClose);
    HttpParser::Result parse_result = parser.get_result();

    Router router(port_map[4242]);
    Router::Result router_result =
        router.Route(parse_result.status, parse_result.request,
                     ConnectionInfo(4242, "127.0.0.1"));
    EXPECT_EQ(router_result.status, 200);
    EXPECT_TRUE((router_result.methods & parse_result.request.req.method) > 0);
    EXPECT_EQ(router_result.success_path, "./resources/cgi/cgi_redir.php");
    EXPECT_EQ(router_result.error_path, "./error.html");

    ResponseManager rm;
    ResponseManager::Result rm_result =
        rm.ExecuteMethod(router_result, parse_result.request);

    ASSERT_EQ(rm_result.status, 500);
  }

  // s 11 cgi client redirection with body
  {
    ServerConfig result = TestValidatorSuccess(CM_CONFIG_PATH_PREFIX "s_11");
    PortMap port_map = result.port_map;
    EXPECT_EQ(port_map.size(), 1);
    EXPECT_EQ(port_map.count(4242), 1);

    HttpParser parser;
    std::string req_buf = FileToString(CM_REQ_PATH_PREFIX "s_11.txt");
    int status = parser.Parse(req_buf);
    EXPECT_EQ(status, HttpParser::kClose);
    HttpParser::Result parse_result = parser.get_result();

    Router router(port_map[4242]);
    Router::Result router_result =
        router.Route(parse_result.status, parse_result.request,
                     ConnectionInfo(4242, "127.0.0.1"));
    EXPECT_EQ(router_result.status, 200);
    EXPECT_TRUE((router_result.methods & parse_result.request.req.method) > 0);
    EXPECT_EQ(router_result.success_path, "./resources/cgi/cgi_redir.php");
    EXPECT_EQ(router_result.error_path, "./error.html");

    ResponseManager rm;
    ResponseManager::Result rm_result =
        rm.ExecuteMethod(router_result, parse_result.request);

    ASSERT_EQ(rm_result.status, 302);
    EXPECT_EQ(rm_result.header.size(), 3);
    EXPECT_EQ(rm_result.header.count("location"), 1);
    EXPECT_EQ(rm_result.header["location"], "https://naver.com");
    EXPECT_EQ(rm_result.header.count("content-type"), 1);
    EXPECT_EQ(rm_result.header["content-type"], "text/html");
    EXPECT_EQ(rm_result.header.count("allow"), 1);
    EXPECT_EQ(rm_result.header["allow"], "GET, POST");

    EXPECT_EQ(rm_result.content, "abc");
  }
}
