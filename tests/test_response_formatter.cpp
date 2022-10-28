#include <fstream>
#include <iostream>

#include "CgiManager.hpp"
#include "Connection.hpp"
#include "HttpParser.hpp"
#include "ResourceManager.hpp"
#include "ResponseData.hpp"
#include "ResponseFormatter.hpp"
#include "Router.hpp"
#include "Types.hpp"
#include "Validator.hpp"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

#define RF_CONFIG_PATH_PREFIX "../configs/tests/ResponseFormatter/"
#define RF_REQ_PATH_PREFIX "../tests/ResponseFormatter/"

using ::testing::HasSubstr;
using ::testing::MatchesRegex;
using ::testing::StartsWith;

std::string FileToString(const std::string& file_path);
ServerConfig TestValidatorSuccess(const std::string& case_id);

void ValidateResponse(const std::string& test_id, const std::string* expected,
                      std::string& response, size_t header_cnt,
                      const std::string& resource_str) {
  std::cout << "Validating response for " << test_id << std::endl;
  // generate response header vector
  size_t end_of_header = response.find(CRLF CRLF);
  ASSERT_NE(end_of_header, std::string::npos);
  std::string header = response.substr(0, end_of_header + 2);

  std::vector<std::string> header_lines;
  std::string token;
  size_t start = 0;
  for (size_t end = header.find(CRLF); end != std::string::npos;
       end = header.find(CRLF, start)) {
    header_lines.push_back(header.substr(start, end - start));
    start = end + 2;
  }
  for (size_t i = 0; i < header_lines.size(); i++) {
    std::cout << "Header line " << i << ": " << header_lines[i] << std::endl;
  }
  // compare response header and expected header
  ASSERT_EQ(header_lines.size(), header_cnt);
  for (size_t i = 0; i < header_lines.size(); ++i) {
    if (i == 2) {
      EXPECT_THAT(header_lines[i], StartsWith(expected[i]));
    } else {
      EXPECT_EQ(header_lines[i], expected[i]);
    }
  }
  std::string body = response.substr(end_of_header + 4);
  EXPECT_EQ(body, resource_str);
}

TEST(ResourceFormatterTest, GETResponse) {
  // s 00 general case - GET
  {
    ServerConfig result = TestValidatorSuccess(RF_CONFIG_PATH_PREFIX "s_00");
    PortMap port_map = result.port_map;
    EXPECT_EQ(port_map.size(), 1);
    EXPECT_EQ(port_map.count(4242), 1);

    HttpParser parser;
    std::string req_buf = FileToString(RF_REQ_PATH_PREFIX "s_00.txt");
    int parse_status = parser.Parse(req_buf);
    EXPECT_EQ(parse_status, HttpParser::kComplete);
    HttpParser::Result parse_result = parser.get_result();

    Router router(port_map[4242]);
    Router::Result router_result =
        router.Route(parse_result.status, parse_result.request,
                     ConnectionInfo(4242, "127.0.0.1"));
    EXPECT_EQ(router_result.status, 200);
    EXPECT_TRUE((router_result.methods & parse_result.request.req.method) > 0);
    EXPECT_EQ(router_result.success_path, "./rf_resources/s_00.html");
    EXPECT_EQ(router_result.error_path, "./error.html");

    ResourceManager rm;
    ResourceManager::Result rm_result =
        rm.ExecuteMethod(router_result, parse_result.request);

    EXPECT_EQ(rm_result.status, 200);

    ResponseFormatter rf;
    std::string response =
        rf.Format(rm_result, parse_result.request.req.version,
                  router_result.methods, parse_status);
    std::string expected[7] = {"HTTP/1.1 200 OK",
                               "server: BrilliantServer/1.0",
                               "date: ",
                               "allow: GET",
                               "connection: keep-alive",
                               "content-length: 88",
                               "content-type: text/html;charset=utf-8"};
    ValidateResponse("s_00", expected, response, 7,
                     FileToString(RF_REQ_PATH_PREFIX "s_00.html"));
  }

  // s 01 general case - GET
  {
    ServerConfig result = TestValidatorSuccess(RF_CONFIG_PATH_PREFIX "s_01");
    PortMap port_map = result.port_map;
    EXPECT_EQ(port_map.size(), 1);
    EXPECT_EQ(port_map.count(4242), 1);

    HttpParser parser;
    std::string req_buf = FileToString(RF_REQ_PATH_PREFIX "s_01.txt");
    int parse_status = parser.Parse(req_buf);
    EXPECT_EQ(parse_status, HttpParser::kComplete);
    HttpParser::Result parse_result = parser.get_result();

    Router router(port_map[4242]);
    Router::Result router_result =
        router.Route(parse_result.status, parse_result.request,
                     ConnectionInfo(4242, "127.0.0.1"));
    EXPECT_EQ(router_result.status, 200);
    EXPECT_TRUE((router_result.methods & parse_result.request.req.method) > 0);
    EXPECT_EQ(router_result.success_path, "./rf_resources/s_01.css");
    EXPECT_EQ(router_result.error_path, "./error.html");

    ResourceManager rm;
    ResourceManager::Result rm_result =
        rm.ExecuteMethod(router_result, parse_result.request);

    EXPECT_EQ(rm_result.status, 200);

    ResponseFormatter rf;
    std::string response =
        rf.Format(rm_result, parse_result.request.req.version,
                  router_result.methods, parse_status);
    std::string expected[7] = {"HTTP/1.1 200 OK",
                               "server: BrilliantServer/1.0",
                               "date: ",
                               "allow: GET",
                               "connection: keep-alive",
                               "content-length: 38",
                               "content-type: text/css;charset=utf-8"};
    ValidateResponse("s_01", expected, response, 7,
                     FileToString(RF_REQ_PATH_PREFIX "s_01.css"));
  }

  // s 02 general case - GET
  {
    ServerConfig result = TestValidatorSuccess(RF_CONFIG_PATH_PREFIX "s_02");
    PortMap port_map = result.port_map;
    EXPECT_EQ(port_map.size(), 1);
    EXPECT_EQ(port_map.count(4242), 1);

    HttpParser parser;
    std::string req_buf = FileToString(RF_REQ_PATH_PREFIX "s_02.txt");
    int parse_status = parser.Parse(req_buf);
    EXPECT_EQ(parse_status, HttpParser::kClose);
    HttpParser::Result parse_result = parser.get_result();

    Router router(port_map[4242]);
    Router::Result router_result =
        router.Route(parse_result.status, parse_result.request,
                     ConnectionInfo(4242, "127.0.0.1"));
    EXPECT_EQ(router_result.status, 200);
    EXPECT_TRUE((router_result.methods & parse_result.request.req.method) > 0);
    EXPECT_EQ(router_result.success_path, "./rf_resources/s_02.png");
    EXPECT_EQ(router_result.error_path, "./rf_resources/error.html");

    ResourceManager rm;
    ResourceManager::Result rm_result =
        rm.ExecuteMethod(router_result, parse_result.request);

    EXPECT_EQ(rm_result.status, 200);

    ResponseFormatter rf;
    std::string response =
        rf.Format(rm_result, parse_result.request.req.version,
                  router_result.methods, parse_status);
    std::string expected[7] = {"HTTP/1.1 200 OK",
                               "server: BrilliantServer/1.0",
                               "date: ",
                               "allow: GET, POST, DELETE",
                               "connection: close",
                               "content-length: 8780",
                               "content-type: image/png"};
    ValidateResponse("s_02", expected, response, 7,
                     FileToString(RF_REQ_PATH_PREFIX "s_02.png"));
  }

  // s 03 general case - GET
  {
    ServerConfig result = TestValidatorSuccess(RF_CONFIG_PATH_PREFIX "s_03");
    PortMap port_map = result.port_map;
    EXPECT_EQ(port_map.size(), 1);
    EXPECT_EQ(port_map.count(4242), 1);

    HttpParser parser;
    std::string req_buf = FileToString(RF_REQ_PATH_PREFIX "s_03.txt");
    int parse_status = parser.Parse(req_buf);
    EXPECT_EQ(parse_status, HttpParser::kClose);
    HttpParser::Result parse_result = parser.get_result();

    Router router(port_map[4242]);
    Router::Result router_result =
        router.Route(parse_result.status, parse_result.request,
                     ConnectionInfo(4242, "127.0.0.1"));
    EXPECT_EQ(router_result.status, 200);
    EXPECT_TRUE((router_result.methods & parse_result.request.req.method) > 0);
    EXPECT_EQ(router_result.success_path, "./rf_resources/s_03.jiskim");
    EXPECT_EQ(router_result.error_path, "./rf_resources/error.html");

    ResourceManager rm;
    ResourceManager::Result rm_result =
        rm.ExecuteMethod(router_result, parse_result.request);

    EXPECT_EQ(rm_result.status, 200);

    ResponseFormatter rf;
    std::string response =
        rf.Format(rm_result, parse_result.request.req.version,
                  router_result.methods, parse_status);
    std::string expected[6] = {"HTTP/1.1 200 OK",
                               "server: BrilliantServer/1.0",
                               "date: ",
                               "allow: GET, POST, DELETE",
                               "connection: close",
                               "content-length: 8780"};
    ValidateResponse("s_03", expected, response, 6,
                     FileToString(RF_REQ_PATH_PREFIX "s_03.jiskim"));
  }

  // f 00 GET 404
  {
    ServerConfig result = TestValidatorSuccess(RF_CONFIG_PATH_PREFIX "f_00");
    PortMap port_map = result.port_map;
    EXPECT_EQ(port_map.size(), 1);
    EXPECT_EQ(port_map.count(4242), 1);

    HttpParser parser;
    std::string req_buf = FileToString(RF_REQ_PATH_PREFIX "f_00.txt");
    int parse_status = parser.Parse(req_buf);
    EXPECT_EQ(parse_status, HttpParser::kComplete);
    HttpParser::Result parse_result = parser.get_result();

    Router router(port_map[4242]);
    Router::Result router_result =
        router.Route(parse_result.status, parse_result.request,
                     ConnectionInfo(4242, "127.0.0.1"));
    EXPECT_EQ(router_result.status, 200);
    EXPECT_TRUE((router_result.methods & parse_result.request.req.method) > 0);
    EXPECT_EQ(router_result.success_path, "./rf_resources/f_00.html");
    EXPECT_EQ(router_result.error_path, "./rf_resources/error.html");

    ResourceManager rm;
    ResourceManager::Result rm_result =
        rm.ExecuteMethod(router_result, parse_result.request);

    EXPECT_EQ(rm_result.status, 404);

    ResponseFormatter rf;
    std::string response =
        rf.Format(rm_result, parse_result.request.req.version,
                  router_result.methods, parse_status);
    std::string expected[6] = {"HTTP/1.1 404 Not Found",
                               "server: BrilliantServer/1.0",
                               "date: ",
                               "connection: keep-alive",
                               "content-length: 92",
                               "content-type: text/html;charset=utf-8"};
    ValidateResponse("f_00", expected, response, 6,
                     FileToString(RF_REQ_PATH_PREFIX "error.html"));
  }

  // f 01 GET 403
  {
    ServerConfig result = TestValidatorSuccess(RF_CONFIG_PATH_PREFIX "f_01");
    PortMap port_map = result.port_map;
    EXPECT_EQ(port_map.size(), 1);
    EXPECT_EQ(port_map.count(4242), 1);

    HttpParser parser;
    std::string req_buf = FileToString(RF_REQ_PATH_PREFIX "f_01.txt");
    int parse_status = parser.Parse(req_buf);
    EXPECT_EQ(parse_status, HttpParser::kComplete);
    HttpParser::Result parse_result = parser.get_result();

    Router router(port_map[4242]);
    Router::Result router_result =
        router.Route(parse_result.status, parse_result.request,
                     ConnectionInfo(4242, "127.0.0.1"));
    EXPECT_EQ(router_result.status, 200);
    EXPECT_TRUE((router_result.methods & parse_result.request.req.method) > 0);
    EXPECT_EQ(router_result.success_path, "./rf_resources/f_01.html");
    EXPECT_EQ(router_result.error_path, "./rf_resources/error.html");

    chmod("./rf_resources/f_01.html", 0222);
    ResourceManager rm;
    ResourceManager::Result rm_result =
        rm.ExecuteMethod(router_result, parse_result.request);

    EXPECT_EQ(rm_result.status, 403);

    ResponseFormatter rf;
    std::string response =
        rf.Format(rm_result, parse_result.request.req.version,
                  router_result.methods, parse_status);
    std::string expected[7] = {"HTTP/1.1 403 Forbidden",
                               "server: BrilliantServer/1.0",
                               "date: ",
                               "allow: GET, POST, DELETE",
                               "connection: keep-alive",
                               "content-length: 92",
                               "content-type: text/html;charset=utf-8"};
    ValidateResponse("f_01", expected, response, 7,
                     FileToString(RF_REQ_PATH_PREFIX "error.html"));
    chmod("./rf_resources/f_01.html", 0777);
  }

  // f 02 GET 405
  {
    ServerConfig result = TestValidatorSuccess(RF_CONFIG_PATH_PREFIX "f_02");
    PortMap port_map = result.port_map;
    EXPECT_EQ(port_map.size(), 1);
    EXPECT_EQ(port_map.count(4242), 1);

    HttpParser parser;
    std::string req_buf = FileToString(RF_REQ_PATH_PREFIX "f_02.txt");
    int parse_status = parser.Parse(req_buf);
    EXPECT_EQ(parse_status, HttpParser::kComplete);
    HttpParser::Result parse_result = parser.get_result();

    Router router(port_map[4242]);
    Router::Result router_result =
        router.Route(parse_result.status, parse_result.request,
                     ConnectionInfo(4242, "127.0.0.1"));
    EXPECT_EQ(router_result.status, 405);
    EXPECT_TRUE((router_result.methods & parse_result.request.req.method) == 0);
    EXPECT_EQ(router_result.success_path, "./rf_resources/error.html");
    EXPECT_EQ(router_result.error_path, "./rf_resources/error.html");

    ResourceManager rm;
    ResourceManager::Result rm_result =
        rm.ExecuteMethod(router_result, parse_result.request);

    EXPECT_EQ(rm_result.status, 405);

    ResponseFormatter rf;
    std::string response =
        rf.Format(rm_result, parse_result.request.req.version,
                  router_result.methods, parse_status);

    std::string expected[7] = {
        "HTTP/1.1 405 Method Not Allowed",
        "server: BrilliantServer/1.0",
        "date: ",
        "allow: POST",
        "connection: keep-alive",
        "content-length: 92",
        "content-type: text/html;charset=utf-8",
    };
    ValidateResponse("f_02", expected, response, 7,
                     FileToString(RF_REQ_PATH_PREFIX "error.html"));
  }

  // f 03 GET 505
  {
    ServerConfig result = TestValidatorSuccess(RF_CONFIG_PATH_PREFIX "f_03");
    PortMap port_map = result.port_map;
    EXPECT_EQ(port_map.size(), 1);
    EXPECT_EQ(port_map.count(4242), 1);

    HttpParser parser;
    std::string req_buf = FileToString(RF_REQ_PATH_PREFIX "f_03.txt");
    int parse_status = parser.Parse(req_buf);
    EXPECT_EQ(parse_status, HttpParser::kClose);
    HttpParser::Result parse_result = parser.get_result();

    Router router(port_map[4242]);
    Router::Result router_result =
        router.Route(parse_result.status, parse_result.request,
                     ConnectionInfo(4242, "127.0.0.1"));
    EXPECT_EQ(router_result.status, 505);
    EXPECT_TRUE((router_result.methods & parse_result.request.req.method) > 0);
    EXPECT_EQ(router_result.success_path, "./rf_resources/error.html");
    EXPECT_EQ(router_result.error_path, "./rf_resources/error.html");

    ResourceManager rm;
    ResourceManager::Result rm_result =
        rm.ExecuteMethod(router_result, parse_result.request);

    EXPECT_EQ(rm_result.status, 505);

    ResponseFormatter rf;
    std::string response =
        rf.Format(rm_result, parse_result.request.req.version,
                  router_result.methods, parse_status);
    // split by CRLF
    std::string expected[6] = {
        "HTTP/1.1 505 HTTP Version Not Supported",
        "server: BrilliantServer/1.0",
        "date: ",
        "connection: close",
        "content-length: 92",
        "content-type: text/html;charset=utf-8",
    };
    ValidateResponse("f_03", expected, response, 6,
                     FileToString(RF_REQ_PATH_PREFIX "error.html"));
  }

  // f 04 GET 400
  {
    ServerConfig result = TestValidatorSuccess(RF_CONFIG_PATH_PREFIX "f_04");
    PortMap port_map = result.port_map;
    EXPECT_EQ(port_map.size(), 1);
    EXPECT_EQ(port_map.count(4242), 1);

    HttpParser parser;
    std::string req_buf = FileToString(RF_REQ_PATH_PREFIX "f_04.txt");
    int parse_status = parser.Parse(req_buf);
    EXPECT_EQ(parse_status, HttpParser::kClose);
    HttpParser::Result parse_result = parser.get_result();
    EXPECT_EQ(parse_result.status, 400);

    Router router(port_map[4242]);
    Router::Result router_result =
        router.Route(parse_result.status, parse_result.request,
                     ConnectionInfo(4242, "127.0.0.1"));
    EXPECT_EQ(router_result.status, 400);
    EXPECT_TRUE((router_result.methods & parse_result.request.req.method) > 0);
    EXPECT_EQ(router_result.success_path, "./rf_resources/error.html");
    EXPECT_EQ(router_result.error_path, "./rf_resources/error.html");

    ResourceManager rm;
    ResourceManager::Result rm_result =
        rm.ExecuteMethod(router_result, parse_result.request);

    EXPECT_EQ(rm_result.status, 400);

    ResponseFormatter rf;
    std::string response =
        rf.Format(rm_result, parse_result.request.req.version,
                  router_result.methods, parse_status);
    // split by CRLF
    std::string expected[6] = {
        "HTTP/1.1 400 Bad Request",
        "server: BrilliantServer/1.0",
        "date: ",
        "connection: close",
        "content-length: 92",
        "content-type: text/html;charset=utf-8",
    };
    ValidateResponse("f_04", expected, response, 6,
                     FileToString(RF_REQ_PATH_PREFIX "error.html"));
  }

  // s 04 HTTP/1.1 autoindex GET request
  {
    ServerConfig result = TestValidatorSuccess(RF_CONFIG_PATH_PREFIX "s_04");
    PortMap port_map = result.port_map;
    EXPECT_EQ(port_map.size(), 1);
    EXPECT_EQ(port_map.count(4242), 1);

    HttpParser parser;
    std::string req_buf = FileToString(RF_REQ_PATH_PREFIX "s_04.txt");
    int parse_status = parser.Parse(req_buf);
    EXPECT_EQ(parse_status, HttpParser::kComplete);
    HttpParser::Result parse_result = parser.get_result();
    EXPECT_EQ(parse_result.status, 200);

    Router router(port_map[4242]);
    Router::Result router_result =
        router.Route(parse_result.status, parse_result.request,
                     ConnectionInfo(4242, "127.0.0.1"));
    EXPECT_EQ(router_result.status, 200);
    EXPECT_TRUE((router_result.methods & parse_result.request.req.method) > 0);
    EXPECT_EQ(router_result.success_path, "./_deps/");
    EXPECT_EQ(router_result.error_path, "./rf_resources/error.html");

    ResourceManager rm;
    ResourceManager::Result rm_result =
        rm.ExecuteMethod(router_result, parse_result.request);

    EXPECT_EQ(rm_result.status, 200);

    ResponseFormatter rf;
    std::string response =
        rf.Format(rm_result, parse_result.request.req.version,
                  router_result.methods, parse_status);
    // split by CRLF
    std::string expected[7] = {
        "HTTP/1.1 200 OK",
        "server: BrilliantServer/1.0",
        "date: ",
        "allow: GET",
        "connection: keep-alive",
        "content-length: 275",
        "content-type: text/html;charset=utf-8",
    };
    ValidateResponse("s_04", expected, response, 7,
                     FileToString(RF_REQ_PATH_PREFIX "autoindex.html"));
  }

  // s 05 HTTP/1.2 GET request
  {
    ServerConfig result = TestValidatorSuccess(RF_CONFIG_PATH_PREFIX "s_05");
    PortMap port_map = result.port_map;
    EXPECT_EQ(port_map.size(), 1);
    EXPECT_EQ(port_map.count(4242), 1);

    HttpParser parser;
    std::string req_buf = FileToString(RF_REQ_PATH_PREFIX "s_05.txt");
    int parse_status = parser.Parse(req_buf);
    EXPECT_EQ(parse_status, HttpParser::kComplete);
    HttpParser::Result parse_result = parser.get_result();
    EXPECT_EQ(parse_result.status, 200);

    Router router(port_map[4242]);
    Router::Result router_result =
        router.Route(parse_result.status, parse_result.request,
                     ConnectionInfo(4242, "127.0.0.1"));
    EXPECT_EQ(router_result.status, 200);
    EXPECT_TRUE((router_result.methods & parse_result.request.req.method) > 0);
    EXPECT_EQ(router_result.success_path, "./rf_resources/s_05.html");
    EXPECT_EQ(router_result.error_path, "./rf_resources/error.html");

    ResourceManager rm;
    ResourceManager::Result rm_result =
        rm.ExecuteMethod(router_result, parse_result.request);

    EXPECT_EQ(rm_result.status, 200);

    ResponseFormatter rf;
    std::string response =
        rf.Format(rm_result, parse_result.request.req.version,
                  router_result.methods, parse_status);
    // split by CRLF
    std::string expected[7] = {
        "HTTP/1.1 200 OK",
        "server: BrilliantServer/1.0",
        "date: ",
        "allow: GET",
        "connection: keep-alive",
        "content-length: 117",
        "content-type: text/html;charset=utf-8",
    };
    ValidateResponse("s_05", expected, response, 7,
                     FileToString(RF_REQ_PATH_PREFIX "s_05.html"));
  }

  // s 06 redirection 301 (local redirection)
  {
    ServerConfig result = TestValidatorSuccess(RF_CONFIG_PATH_PREFIX "s_06");
    PortMap port_map = result.port_map;
    EXPECT_EQ(port_map.size(), 1);
    EXPECT_EQ(port_map.count(4242), 1);

    HttpParser parser;
    std::string req_buf = FileToString(RF_REQ_PATH_PREFIX "s_06.txt");
    int parse_status = parser.Parse(req_buf);
    EXPECT_EQ(parse_status, HttpParser::kComplete);
    HttpParser::Result parse_result = parser.get_result();
    EXPECT_EQ(parse_result.status, 200);

    Router router(port_map[4242]);
    Router::Result router_result =
        router.Route(parse_result.status, parse_result.request,
                     ConnectionInfo(4242, "127.0.0.1"));
    EXPECT_EQ(router_result.status, 301);
    EXPECT_EQ(router_result.redirect_to, "/resources/s_00.html");
    EXPECT_TRUE((router_result.methods & parse_result.request.req.method) > 0);
    EXPECT_EQ(router_result.error_path, "./rf_resources/error.html");

    ResourceManager rm;
    ResourceManager::Result rm_result =
        rm.ExecuteMethod(router_result, parse_result.request);

    EXPECT_EQ(rm_result.status, 301);

    ResponseFormatter rf;
    std::string response =
        rf.Format(rm_result, parse_result.request.req.version,
                  router_result.methods, parse_status);
    // split by CRLF
    std::string expected[7] = {
        "HTTP/1.1 301 Moved Permanently",
        "server: BrilliantServer/1.0",
        "date: ",
        "connection: keep-alive",
        "content-length: 191",
        "content-type: text/html;charset=utf-8",
        "location: /resources/s_00.html",
    };
    ValidateResponse("s_06", expected, response, 7,
                     "<!DOCTYPE html><html><title></title><body><h1>301 Moved \
Permanently</h1><p>The \
resource has been moved permanently to <a href='/resources/s_00.html'>\
/resources/s_00.html<a>.</p></body></html>");
  }

  // s 07 redirection 301 (redirect by absolute URI)
  {
    ServerConfig result = TestValidatorSuccess(RF_CONFIG_PATH_PREFIX "s_07");
    PortMap port_map = result.port_map;
    EXPECT_EQ(port_map.size(), 1);
    EXPECT_EQ(port_map.count(4242), 1);

    HttpParser parser;
    std::string req_buf = FileToString(RF_REQ_PATH_PREFIX "s_07.txt");
    int parse_status = parser.Parse(req_buf);
    EXPECT_EQ(parse_status, HttpParser::kComplete);
    HttpParser::Result parse_result = parser.get_result();
    EXPECT_EQ(parse_result.status, 200);

    Router router(port_map[4242]);
    Router::Result router_result =
        router.Route(parse_result.status, parse_result.request,
                     ConnectionInfo(4242, "127.0.0.1"));
    EXPECT_EQ(router_result.status, 301);
    EXPECT_EQ(router_result.redirect_to, "https://www.naver.com/");
    EXPECT_TRUE((router_result.methods & parse_result.request.req.method) > 0);
    EXPECT_EQ(router_result.error_path, "./rf_resources/error.html");

    ResourceManager rm;
    ResourceManager::Result rm_result =
        rm.ExecuteMethod(router_result, parse_result.request);

    EXPECT_EQ(rm_result.status, 301);

    ResponseFormatter rf;
    std::string response =
        rf.Format(rm_result, parse_result.request.req.version,
                  router_result.methods, parse_status);
    // split by CRLF
    std::string expected[7] = {
        "HTTP/1.1 301 Moved Permanently",
        "server: BrilliantServer/1.0",
        "date: ",
        "connection: keep-alive",
        "content-length: 195",
        "content-type: text/html;charset=utf-8",
        "location: https://www.naver.com/",
    };
    ValidateResponse("s_07", expected, response, 7,
                     "<!DOCTYPE html><html><title></title><body><h1>301 Moved \
Permanently</h1><p>The \
resource has been moved permanently to <a href='https://www.naver.com/'>\
https://www.naver.com/<a>.</p></body></html>");
  }

  // s 08 redirection 301 (redirect by absolute URI)
  {
    ServerConfig result = TestValidatorSuccess(RF_CONFIG_PATH_PREFIX "s_08");
    PortMap port_map = result.port_map;
    EXPECT_EQ(port_map.size(), 1);
    EXPECT_EQ(port_map.count(4242), 1);

    HttpParser parser;
    std::string req_buf = FileToString(RF_REQ_PATH_PREFIX "s_08.txt");
    int parse_status = parser.Parse(req_buf);
    EXPECT_EQ(parse_status, HttpParser::kComplete);
    HttpParser::Result parse_result = parser.get_result();
    EXPECT_EQ(parse_result.status, 200);

    Router router(port_map[4242]);
    Router::Result router_result =
        router.Route(parse_result.status, parse_result.request,
                     ConnectionInfo(4242, "127.0.0.1"));
    EXPECT_EQ(router_result.status, 301);
    EXPECT_EQ(router_result.redirect_to,
              "https://www.naver.com:8080/search?query=legacy");
    EXPECT_TRUE((router_result.methods & parse_result.request.req.method) > 0);
    EXPECT_EQ(router_result.error_path, "./rf_resources/error.html");

    ResourceManager rm;
    ResourceManager::Result rm_result =
        rm.ExecuteMethod(router_result, parse_result.request);

    EXPECT_EQ(rm_result.status, 301);

    ResponseFormatter rf;
    std::string response =
        rf.Format(rm_result, parse_result.request.req.version,
                  router_result.methods, parse_status);
    // split by CRLF
    std::string expected[7] = {
        "HTTP/1.1 301 Moved Permanently",
        "server: BrilliantServer/1.0",
        "date: ",
        "connection: keep-alive",
        "content-length: 243",
        "content-type: text/html;charset=utf-8",
        "location: https://www.naver.com:8080/search?query=legacy",
    };
    ValidateResponse("s_08", expected, response, 7,
                     "<!DOCTYPE html><html><title></title><body><h1>301 Moved \
Permanently</h1><p>The \
resource has been moved permanently to <a href='https://www.naver.com:8080/search?query=legacy'>\
https://www.naver.com:8080/search?query=legacy<a>.</p></body></html>");
  }

  // s 09 redirection 301 (redirect by local uri with query)
  {
    ServerConfig result = TestValidatorSuccess(RF_CONFIG_PATH_PREFIX "s_09");
    PortMap port_map = result.port_map;
    EXPECT_EQ(port_map.size(), 1);
    EXPECT_EQ(port_map.count(4242), 1);

    HttpParser parser;
    std::string req_buf = FileToString(RF_REQ_PATH_PREFIX "s_09.txt");
    int parse_status = parser.Parse(req_buf);
    EXPECT_EQ(parse_status, HttpParser::kComplete);
    HttpParser::Result parse_result = parser.get_result();
    EXPECT_EQ(parse_result.status, 200);

    Router router(port_map[4242]);
    Router::Result router_result =
        router.Route(parse_result.status, parse_result.request,
                     ConnectionInfo(4242, "127.0.0.1"));
    EXPECT_EQ(router_result.status, 301);
    EXPECT_EQ(router_result.redirect_to,
              "/resources/login?user=yongjule&password=julejule");
    EXPECT_TRUE((router_result.methods & parse_result.request.req.method) > 0);
    EXPECT_EQ(router_result.error_path, "./rf_resources/error.html");

    ResourceManager rm;
    ResourceManager::Result rm_result =
        rm.ExecuteMethod(router_result, parse_result.request);

    EXPECT_EQ(rm_result.status, 301);

    ResponseFormatter rf;
    std::string response =
        rf.Format(rm_result, parse_result.request.req.version,
                  router_result.methods, parse_status);
    // split by CRLF
    std::string expected[7] = {
        "HTTP/1.1 301 Moved Permanently",
        "server: BrilliantServer/1.0",
        "date: ",
        "connection: keep-alive",
        "content-length: 247",
        "content-type: text/html;charset=utf-8",
        "location: /resources/login?user=yongjule&password=julejule",
    };
    ValidateResponse("s_09", expected, response, 7,
                     "<!DOCTYPE html><html><title></title><body><h1>301 Moved \
Permanently</h1><p>The \
resource has been moved permanently to <a href='/resources/login?user=yongjule&password=julejule'>\
/resources/login?user=yongjule&password=julejule<a>.</p></body></html>");
  }
}

TEST(ResourceFormatterTest, POSTResponse) {
  // s 10 POST
  {
    ServerConfig result = TestValidatorSuccess(RF_CONFIG_PATH_PREFIX "s_10");
    PortMap port_map = result.port_map;
    EXPECT_EQ(port_map.size(), 1);
    EXPECT_EQ(port_map.count(4242), 1);

    HttpParser parser;
    std::string req_buf = FileToString(RF_REQ_PATH_PREFIX "s_10.txt");
    int parse_status = parser.Parse(req_buf);
    EXPECT_EQ(parse_status, HttpParser::kComplete);
    HttpParser::Result parse_result = parser.get_result();
    EXPECT_EQ(parse_result.status, 200);

    Router router(port_map[4242]);
    Router::Result router_result =
        router.Route(parse_result.status, parse_result.request,
                     ConnectionInfo(4242, "127.0.0.1"));
    EXPECT_EQ(router_result.status, 200);
    EXPECT_TRUE((router_result.methods & parse_result.request.req.method) > 0);
    EXPECT_EQ(router_result.success_path, "./rf_resources/post/s_10.txt");
    EXPECT_EQ(router_result.error_path, "./rf_resources/error.html");

    ResourceManager rm;
    ResourceManager::Result rm_result =
        rm.ExecuteMethod(router_result, parse_result.request);

    EXPECT_EQ(access(router_result.success_path.c_str(), F_OK), 0);

    EXPECT_EQ(rm_result.status, 201);

    ResponseFormatter rf;
    std::string response =
        rf.Format(rm_result, parse_result.request.req.version,
                  router_result.methods, parse_status);
    // split by CRLF
    std::string expected[8] = {
        "HTTP/1.1 201 Created",
        "server: BrilliantServer/1.0",
        "date: ",
        "allow: POST",
        "connection: keep-alive",
        "content-length: 173",
        "content-type: text/html;charset=utf-8",
        "location: /rf_resources/post/s_10.txt",
    };
    ValidateResponse(
        "s_10", expected, response, 8,
        "<!DOCTYPE html><html><title>201 Created</title><body><h1>201 \
Created</h1><p>YAY! The file is created at \
/rf_resources/post/s_10.txt!</p><p>Have a nice day~</p></body></html>");
    unlink("./rf_resources/post/s_10.txt");
  }

  // s 11 upload already existing filename
  {
    ServerConfig result = TestValidatorSuccess(RF_CONFIG_PATH_PREFIX "s_11");
    PortMap port_map = result.port_map;
    EXPECT_EQ(port_map.size(), 1);
    EXPECT_EQ(port_map.count(4242), 1);

    HttpParser parser;
    std::string req_buf = FileToString(RF_REQ_PATH_PREFIX "s_11.txt");
    int parse_status = parser.Parse(req_buf);
    EXPECT_EQ(parse_status, HttpParser::kComplete);
    HttpParser::Result parse_result = parser.get_result();
    EXPECT_EQ(parse_result.status, 200);

    Router router(port_map[4242]);
    Router::Result router_result =
        router.Route(parse_result.status, parse_result.request,
                     ConnectionInfo(4242, "127.0.0.1"));
    EXPECT_EQ(router_result.status, 200);
    EXPECT_TRUE((router_result.methods & parse_result.request.req.method) > 0);
    EXPECT_EQ(router_result.success_path, "./rf_resources/post/empty");
    EXPECT_EQ(router_result.error_path, "./rf_resources/error.html");

    ResourceManager rm;
    ResourceManager::Result rm_result =
        rm.ExecuteMethod(router_result, parse_result.request);

    EXPECT_EQ(access("./rf_resources/post/empty_0", F_OK), 0);

    EXPECT_EQ(rm_result.status, 201);

    ResponseFormatter rf;
    std::string response =
        rf.Format(rm_result, parse_result.request.req.version,
                  router_result.methods, parse_status);
    // split by CRLF
    std::string expected[8] = {
        "HTTP/1.1 201 Created",
        "server: BrilliantServer/1.0",
        "date: ",
        "allow: POST",
        "connection: keep-alive",
        "content-length: 172",
        "content-type: text/html;charset=utf-8",
        "location: /rf_resources/post/empty_0",
    };
    ValidateResponse(
        "s_11", expected, response, 8,
        "<!DOCTYPE html><html><title>201 Created</title><body><h1>201 \
Created</h1><p>YAY! The file is created at \
/rf_resources/post/empty_0!</p><p>Have a nice day~</p></body></html>");
    EXPECT_NE(unlink("./rf_resources/post/empty_0"), -1);
  }

  // f 05 post to unauthorized directory
  {
    ServerConfig result = TestValidatorSuccess(RF_CONFIG_PATH_PREFIX "f_05");
    PortMap port_map = result.port_map;
    EXPECT_EQ(port_map.size(), 1);
    EXPECT_EQ(port_map.count(4242), 1);

    HttpParser parser;
    std::string req_buf = FileToString(RF_REQ_PATH_PREFIX "f_05.txt");
    int parse_status = parser.Parse(req_buf);
    EXPECT_EQ(parse_status, HttpParser::kComplete);
    HttpParser::Result parse_result = parser.get_result();
    EXPECT_EQ(parse_result.status, 200);

    Router router(port_map[4242]);
    Router::Result router_result =
        router.Route(parse_result.status, parse_result.request,
                     ConnectionInfo(4242, "127.0.0.1"));
    EXPECT_EQ(router_result.status, 200);
    EXPECT_TRUE((router_result.methods & parse_result.request.req.method) > 0);
    EXPECT_EQ(router_result.success_path,
              "./rf_resources/post/unauthorized/empty");
    EXPECT_EQ(router_result.error_path, "./rf_resources/error.html");

    if (access("./rf_resources/post/unauthorized", F_OK) == -1) {
      mkdir("./rf_resources/post/unauthorized", 0777);
    }

    EXPECT_NE(chmod("./rf_resources/post/unauthorized", 0555), -1);

    ResourceManager rm;
    ResourceManager::Result rm_result =
        rm.ExecuteMethod(router_result, parse_result.request);
    EXPECT_EQ(rm_result.status, 403);

    ResponseFormatter rf;
    std::string response =
        rf.Format(rm_result, parse_result.request.req.version,
                  router_result.methods, parse_status);
    EXPECT_NE(chmod("./rf_resources/post/unauthorized", 0777), -1);

    // split by CRLF
    std::string expected[7] = {
        "HTTP/1.1 403 Forbidden",
        "server: BrilliantServer/1.0",
        "date: ",
        "allow: POST",
        "connection: keep-alive",
        "content-length: 92",
        "content-type: text/html;charset=utf-8",
    };
    ValidateResponse("f_05", expected, response, 7,
                     FileToString(RF_REQ_PATH_PREFIX "error.html"));
  }

  // f 06 post to out of root
  {
    ServerConfig result = TestValidatorSuccess(RF_CONFIG_PATH_PREFIX "f_06");
    PortMap port_map = result.port_map;
    EXPECT_EQ(port_map.size(), 1);
    EXPECT_EQ(port_map.count(4242), 1);

    HttpParser parser;
    std::string req_buf = FileToString(RF_REQ_PATH_PREFIX "f_06.txt");
    int parse_status = parser.Parse(req_buf);
    EXPECT_EQ(parse_status, HttpParser::kClose);
    HttpParser::Result parse_result = parser.get_result();
    EXPECT_EQ(parse_result.status, 400);

    Router router(port_map[4242]);
    Router::Result router_result =
        router.Route(parse_result.status, parse_result.request,
                     ConnectionInfo(4242, "127.0.0.1"));
    EXPECT_EQ(router_result.status, 400);
    EXPECT_EQ(router_result.success_path, "./rf_resources/error.html");
    EXPECT_EQ(router_result.error_path, "./rf_resources/error.html");

    ResourceManager rm;
    ResourceManager::Result rm_result =
        rm.ExecuteMethod(router_result, parse_result.request);
    EXPECT_EQ(rm_result.status, 400);

    ResponseFormatter rf;
    std::string response =
        rf.Format(rm_result, parse_result.request.req.version,
                  router_result.methods, parse_status);

    // split by CRLF
    std::string expected[6] = {
        "HTTP/1.1 400 Bad Request",
        "server: BrilliantServer/1.0",
        "date: ",
        "connection: close",
        "content-length: 92",
        "content-type: text/html;charset=utf-8",
    };
    ValidateResponse("f_06", expected, response, 6,
                     FileToString(RF_REQ_PATH_PREFIX "error.html"));
  }
}

TEST(ResourceFormatterTest, DELETEResponse) {
  //   s 12 Delete a file
  {
    ServerConfig result = TestValidatorSuccess(RF_CONFIG_PATH_PREFIX "s_12");
    PortMap port_map = result.port_map;
    EXPECT_EQ(port_map.size(), 1);
    EXPECT_EQ(port_map.count(4242), 1);

    HttpParser parser;
    std::string req_buf = FileToString(RF_REQ_PATH_PREFIX "s_12.txt");
    int parse_status = parser.Parse(req_buf);
    EXPECT_EQ(parse_status, HttpParser::kComplete);
    HttpParser::Result parse_result = parser.get_result();
    EXPECT_EQ(parse_result.status, 200);

    Router router(port_map[4242]);
    Router::Result router_result =
        router.Route(parse_result.status, parse_result.request,
                     ConnectionInfo(4242, "127.0.0.1"));
    EXPECT_EQ(router_result.status, 200);
    {
      std::ofstream ofs("./rf_resources/delete/yongjule");
      ofs << "I'm gonna suicide\n";
    }
    EXPECT_EQ(access("./rf_resources/delete/yongjule", F_OK), 0);
    EXPECT_TRUE((router_result.methods & parse_result.request.req.method) > 0);
    EXPECT_EQ(router_result.success_path, "./rf_resources/delete/yongjule");
    EXPECT_EQ(router_result.error_path, "./rf_resources/error.html");

    ResourceManager rm;
    ResourceManager::Result rm_result =
        rm.ExecuteMethod(router_result, parse_result.request);
    ASSERT_EQ(access("./rf_resources/delete/yongjule", F_OK), -1);
    EXPECT_EQ(rm_result.status, 200);

    ResponseFormatter rf;
    std::string response =
        rf.Format(rm_result, parse_result.request.req.version,
                  router_result.methods, parse_status);
    // split by CRLF
    std::string expected[7] = {
        "HTTP/1.1 200 OK",
        "server: BrilliantServer/1.0",
        "date: ",
        "allow: DELETE",
        "connection: keep-alive",
        "content-length: 126",
        "content-type: text/html;charset=utf-8",
    };
    ValidateResponse(
        "s_12", expected, response, 7,
        "<!DOCTYPE html><html><title>Deleted</title><body><h1>200 OK</h1><p>" +
            router_result.success_path.substr(1) +
            " is removed!</p></body></html>");
  }

  // f 07 deleting a file that doesn't exist
  {
    ServerConfig result = TestValidatorSuccess(RF_CONFIG_PATH_PREFIX "f_07");
    PortMap port_map = result.port_map;
    EXPECT_EQ(port_map.size(), 1);
    EXPECT_EQ(port_map.count(4242), 1);

    HttpParser parser;
    std::string req_buf = FileToString(RF_REQ_PATH_PREFIX "f_07.txt");
    int parse_status = parser.Parse(req_buf);
    EXPECT_EQ(parse_status, HttpParser::kComplete);
    HttpParser::Result parse_result = parser.get_result();
    EXPECT_EQ(parse_result.status, 200);

    Router router(port_map[4242]);
    Router::Result router_result =
        router.Route(parse_result.status, parse_result.request,
                     ConnectionInfo(4242, "127.0.0.1"));
    EXPECT_EQ(router_result.status, 200);
    EXPECT_EQ(router_result.success_path, "./rf_resources/delete/undefined");
    EXPECT_EQ(router_result.error_path, "./rf_resources/error.html");

    ResourceManager rm;
    ResourceManager::Result rm_result =
        rm.ExecuteMethod(router_result, parse_result.request);
    EXPECT_EQ(rm_result.status, 404);

    ResponseFormatter rf;
    std::string response =
        rf.Format(rm_result, parse_result.request.req.version,
                  router_result.methods, parse_status);

    // split by CRLF
    std::string expected[6] = {
        "HTTP/1.1 404 Not Found",
        "server: BrilliantServer/1.0",
        "date: ",
        "connection: keep-alive",
        "content-length: 92",
        "content-type: text/html;charset=utf-8",
    };
    ValidateResponse("f_07", expected, response, 6,
                     FileToString(RF_REQ_PATH_PREFIX "error.html"));
  }

  // f 08 405 method not allowed
  {
    ServerConfig result = TestValidatorSuccess(RF_CONFIG_PATH_PREFIX "f_08");
    PortMap port_map = result.port_map;
    EXPECT_EQ(port_map.size(), 1);
    EXPECT_EQ(port_map.count(4242), 1);

    HttpParser parser;
    std::string req_buf = FileToString(RF_REQ_PATH_PREFIX "f_08.txt");
    int parse_status = parser.Parse(req_buf);
    EXPECT_EQ(parse_status, HttpParser::kComplete);
    HttpParser::Result parse_result = parser.get_result();
    EXPECT_EQ(parse_result.status, 200);

    Router router(port_map[4242]);
    Router::Result router_result =
        router.Route(parse_result.status, parse_result.request,
                     ConnectionInfo(4242, "127.0.0.1"));
    EXPECT_EQ(router_result.status, 405);
    EXPECT_EQ(router_result.success_path, "./rf_resources/error.html");
    EXPECT_EQ(router_result.error_path, "./rf_resources/error.html");

    ResourceManager rm;
    ResourceManager::Result rm_result =
        rm.ExecuteMethod(router_result, parse_result.request);
    EXPECT_EQ(rm_result.status, 405);

    ResponseFormatter rf;
    std::string response =
        rf.Format(rm_result, parse_result.request.req.version,
                  router_result.methods, parse_status);

    // split by CRLF
    std::string expected[7] = {
        "HTTP/1.1 405 Method Not Allowed",
        "server: BrilliantServer/1.0",
        "date: ",
        "allow: GET, POST",
        "connection: keep-alive",
        "content-length: 92",
        "content-type: text/html;charset=utf-8",
    };
    ValidateResponse("f_08", expected, response, 7,
                     FileToString(RF_REQ_PATH_PREFIX "error.html"));
  }

  // f 09 method not allowed
  {
    ServerConfig result = TestValidatorSuccess(RF_CONFIG_PATH_PREFIX "f_09");
    PortMap port_map = result.port_map;
    EXPECT_EQ(port_map.size(), 1);
    EXPECT_EQ(port_map.count(4242), 1);

    HttpParser parser;
    std::string req_buf = FileToString(RF_REQ_PATH_PREFIX "f_09.txt");
    int parse_status = parser.Parse(req_buf);
    EXPECT_EQ(parse_status, HttpParser::kComplete);
    HttpParser::Result parse_result = parser.get_result();
    EXPECT_EQ(parse_result.status, 200);

    Router router(port_map[4242]);
    Router::Result router_result =
        router.Route(parse_result.status, parse_result.request,
                     ConnectionInfo(4242, "127.0.0.1"));
    EXPECT_EQ(router_result.status, 200);
    EXPECT_EQ(router_result.success_path, "./rf_resources/delete/dummy");
    EXPECT_EQ(router_result.error_path, "./rf_resources/error.html");

    chmod("./rf_resources/delete/dummy", 0444);

    ResourceManager rm;
    ResourceManager::Result rm_result =
        rm.ExecuteMethod(router_result, parse_result.request);
    EXPECT_EQ(rm_result.status, 403);

    ResponseFormatter rf;
    std::string response =
        rf.Format(rm_result, parse_result.request.req.version,
                  router_result.methods, parse_status);

    // split by CRLF
    std::string expected[7] = {
        "HTTP/1.1 403 Forbidden",
        "server: BrilliantServer/1.0",
        "date: ",
        "allow: GET, POST, DELETE",
        "connection: keep-alive",
        "content-length: 92",
        "content-type: text/html;charset=utf-8",
    };
    ValidateResponse("f_09", expected, response, 7,
                     FileToString(RF_REQ_PATH_PREFIX "error.html"));
    chmod("./rf_resources/delete/dummy", 0777);
  }
}

TEST(ResourceFormatterTest, CGIResponse) {
  // f 10 DELETE CGI request 405
  {
    ServerConfig result = TestValidatorSuccess(RF_CONFIG_PATH_PREFIX "f_10");
    PortMap port_map = result.port_map;
    EXPECT_EQ(port_map.size(), 1);
    EXPECT_EQ(port_map.count(4242), 1);

    HttpParser parser;
    std::string req_buf = FileToString(RF_REQ_PATH_PREFIX "f_10.txt");
    int parse_status = parser.Parse(req_buf);
    EXPECT_EQ(parse_status, HttpParser::kComplete);
    HttpParser::Result parse_result = parser.get_result();
    EXPECT_EQ(parse_result.status, 200);

    Router router(port_map[4242]);
    Router::Result router_result =
        router.Route(parse_result.status, parse_result.request,
                     ConnectionInfo(4242, "127.0.0.1"));
    EXPECT_EQ(router_result.status, 405);
    EXPECT_EQ(router_result.success_path, "./rf_resources/error.html");
    EXPECT_EQ(router_result.error_path, "./rf_resources/error.html");

    ResourceManager rm;
    ResourceManager::Result rm_result =
        rm.ExecuteMethod(router_result, parse_result.request);
    EXPECT_EQ(rm_result.status, 405);

    ResponseFormatter rf;
    std::string response =
        rf.Format(rm_result, parse_result.request.req.version,
                  router_result.methods, parse_status);

    // split by CRLF
    std::string expected[7] = {
        "HTTP/1.1 405 Method Not Allowed",
        "server: BrilliantServer/1.0",
        "date: ",
        "allow: GET, POST",
        "connection: keep-alive",
        "content-length: 92",
        "content-type: text/html;charset=utf-8",
    };
    ValidateResponse("f_10", expected, response, 7,
                     FileToString(RF_REQ_PATH_PREFIX "error.html"));
  }

  // s 13 CGI GET Document
  {
    ServerConfig result = TestValidatorSuccess(RF_CONFIG_PATH_PREFIX "s_13");
    PortMap port_map = result.port_map;
    EXPECT_EQ(port_map.size(), 1);
    EXPECT_EQ(port_map.count(4242), 1);

    HttpParser parser;
    std::string req_buf = FileToString(RF_REQ_PATH_PREFIX "s_13.txt");
    int parse_status = parser.Parse(req_buf);
    EXPECT_EQ(parse_status, HttpParser::kComplete);
    HttpParser::Result parse_result = parser.get_result();
    EXPECT_EQ(parse_result.status, 200);

    Router router(port_map[4242]);
    Router::Result router_result =
        router.Route(parse_result.status, parse_result.request,
                     ConnectionInfo(4242, "127.0.0.1"));
    EXPECT_EQ(router_result.status, 200);
    EXPECT_EQ(router_result.success_path, "./resources/cgi/cgi.php");
    EXPECT_EQ(router_result.error_path, "./rf_resources/error.html");

    ResourceManager rm;
    ResourceManager::Result rm_result =
        rm.ExecuteMethod(router_result, parse_result.request);
    EXPECT_EQ(rm_result.status, 200);

    const char** cgi_env = router_result.cgi_env.get_env();
    std::stringstream ss;
    for (int i = 0; cgi_env[i]; ++i)
      ss << "envp [" << i << "] : " << cgi_env[i] << "\n";
    std::string decoded_query(parse_result.request.req.query);
    UriParser uri_parser;
    for (size_t i = 0; i < decoded_query.size(); ++i) {
      if (decoded_query[i] == '%') {
        uri_parser.DecodeHexToAscii(decoded_query, i);
      }
    }

    ResponseFormatter rf;
    std::string response =
        rf.Format(rm_result, parse_result.request.req.version,
                  router_result.methods, parse_status);

    // split by CRLF
    std::string expected[8] = {
        "HTTP/1.1 200 OK",
        "server: BrilliantServer/1.0",
        "date: ",
        "allow: GET, POST",
        "connection: keep-alive",
        "content-length: 734",
        "content-type: text/plain",
        "access-control-allow-origin: *",
    };
    ValidateResponse(
        "s_13", expected, response, 8,
        ss.str() + "\n" + parse_result.request.content +
            ((!parse_result.request.req.query.empty() &&
              parse_result.request.req.query.find("=") == std::string::npos)
                 ? ("\n" + decoded_query.substr(1))
                 : ""));
  }

  // s 14 CGI GET Client Redirection 302 - no content
  {
    ServerConfig result = TestValidatorSuccess(RF_CONFIG_PATH_PREFIX "s_14");
    PortMap port_map = result.port_map;
    EXPECT_EQ(port_map.size(), 1);
    EXPECT_EQ(port_map.count(4242), 1);

    HttpParser parser;
    std::string req_buf = FileToString(RF_REQ_PATH_PREFIX "s_14.txt");
    int parse_status = parser.Parse(req_buf);
    EXPECT_EQ(parse_status, HttpParser::kComplete);
    HttpParser::Result parse_result = parser.get_result();
    EXPECT_EQ(parse_result.status, 200);

    Router router(port_map[4242]);
    Router::Result router_result =
        router.Route(parse_result.status, parse_result.request,
                     ConnectionInfo(4242, "127.0.0.1"));
    EXPECT_EQ(router_result.status, 200);
    EXPECT_EQ(router_result.success_path, "./resources/cgi/cgi_redir.php");
    EXPECT_EQ(router_result.error_path, "./rf_resources/error.html");

    ResourceManager rm;
    ResourceManager::Result rm_result =
        rm.ExecuteMethod(router_result, parse_result.request);
    EXPECT_EQ(rm_result.status, 302);

    EXPECT_EQ(rm_result.is_local_redir, false);

    ResponseFormatter rf;
    std::string response =
        rf.Format(rm_result, parse_result.request.req.version,
                  router_result.methods, parse_status);

    // split by CRLF
    std::string expected[6] = {
        "HTTP/1.1 302 Found",
        "server: BrilliantServer/1.0",
        "date: ",
        "allow: GET, POST",
        "connection: keep-alive",
        "location: https://www.naver.com/",
    };
    ValidateResponse("s_14", expected, response, 6, "");
  }

  // s 15 CGI GET Client Redirection 302 - no content
  {
    ServerConfig result = TestValidatorSuccess(RF_CONFIG_PATH_PREFIX "s_15");
    PortMap port_map = result.port_map;
    EXPECT_EQ(port_map.size(), 1);
    EXPECT_EQ(port_map.count(4242), 1);

    HttpParser parser;
    std::string req_buf = FileToString(RF_REQ_PATH_PREFIX "s_15.txt");
    int parse_status = parser.Parse(req_buf);
    EXPECT_EQ(parse_status, HttpParser::kComplete);
    HttpParser::Result parse_result = parser.get_result();
    EXPECT_EQ(parse_result.status, 200);

    Router router(port_map[4242]);
    Router::Result router_result =
        router.Route(parse_result.status, parse_result.request,
                     ConnectionInfo(4242, "127.0.0.1"));
    EXPECT_EQ(router_result.status, 200);
    EXPECT_EQ(router_result.success_path, "./resources/cgi/cgi_redir.php");
    EXPECT_EQ(router_result.error_path, "./rf_resources/error.html");

    ResourceManager rm;
    ResourceManager::Result rm_result =
        rm.ExecuteMethod(router_result, parse_result.request);
    EXPECT_EQ(rm_result.status, 302);

    EXPECT_EQ(rm_result.is_local_redir, false);

    ResponseFormatter rf;
    std::string response =
        rf.Format(rm_result, parse_result.request.req.version,
                  router_result.methods, parse_status);

    // split by CRLF
    std::string expected[] = {
        "HTTP/1.1 302 Found",
        "server: BrilliantServer/1.0",
        "date: ",
        "allow: GET, POST",
        "connection: keep-alive",
        "content-length: 10",
        "content-type: text/plain",
        "location: https://www.our42vent.42cadet.kr",
        "x-brilliantserver-a: blah",
        "x-brilliantserver-b: blahblah",
    };
    ValidateResponse("s_15", expected, response,
                     sizeof(expected) / sizeof(std::string), "aaaaaaaaaa");
  }

  // TODO : connection 구현 후 테스트
  //   // f 11 CGI no permission 403
  //   {
  //     ServerConfig result =
  //         TestValidatorSuccess(RF_CONFIG_PATH_PREFIX "f_11");
  //     PortMap port_map = result.port_map;
  //     EXPECT_EQ(port_map.size(), 1);
  //     EXPECT_EQ(port_map.count(4242), 1);

  //     HttpParser parser;
  //     std::string req_buf = FileToString(RF_REQ_PATH_PREFIX "f_11.txt");
  //     int parse_status = parser.Parse(req_buf);
  //     EXPECT_EQ(parse_status, HttpParser::kComplete);
  //     HttpParser::Result parse_result = parser.get_result();
  //     EXPECT_EQ(parse_result.status, 200);

  //     Router router(port_map[4242]);
  //     Router::Result router_result =
  //         router.Route(parse_result.status, parse_result.request,
  //                      ConnectionInfo(4242, "127.0.0.1"));
  //     EXPECT_EQ(router_result.status, 200);
  //     EXPECT_EQ(router_result.success_path, "./resources/cgi/cgi_two.php");
  //     EXPECT_EQ(router_result.error_path, "./rf_resources/error.html");

  //     std::ifstream orig("resources/cgi/cgi.php");
  //     std::ofstream copy("resources/cgi/cgi_two.php", 0666);
  //     copy << orig.rdbuf();

  //     ResourceManager rm;
  //     ResourceManager::Result rm_result =
  //         rm.ExecuteMethod(router_result, parse_result.request);
  //     EXPECT_EQ(rm_result.status, 403);

  //     EXPECT_EQ(rm_result.is_local_redir, false);

  //     ResponseFormatter rf;
  //     std::string response =
  //         rf.Format(rm_result, parse_result.request.req.version,
  //                   router_result.methods, parse_status);

  //     // split by CRLF
  //     std::string expected[] = {
  //         "HTTP/1.1 403 Internal Server Error",
  //         "server: BrilliantServer/1.0",
  //         "date: ",
  //         "connection: close",
  //         "content-length: 92",
  //         "content-type: text/html;charset=utf-8",
  //     };
  //     ValidateResponse("f_11", expected, response,
  //                      sizeof(expected) / sizeof(std::string),
  //                      FileToString(RF_REQ_PATH_PREFIX "error.html"));
  //     unlink("resources/cgi/cgi_two.php");
  //   }
}
