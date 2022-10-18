#include <gtest/gtest.h>

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

#define RF_CONFIG_PATH_PREFIX "../configs/tests/ResponseFormatter/"
#define RF_REQ_PATH_PREFIX "../tests/ResponseFormatter/"

std::string FileToString(const std::string& file_path);
Validator::Result TestValidatorSuccess(const std::string& case_id);

TEST(ResourceFormatterTest, about_blank) {
  // s 00 general case
  {
    Validator::Result result =
        TestValidatorSuccess(RF_CONFIG_PATH_PREFIX "s_00");
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
    EXPECT_EQ(router_result.method & GET, GET);
    EXPECT_EQ(router_result.success_path, "./rf_resources/s_00.html");
    EXPECT_EQ(router_result.error_path, "./error.html");

    ResourceManager rm;
    ResourceManager::Result rm_result =
        rm.ExecuteMethod(router_result, parse_result.request.content);

    EXPECT_EQ(rm_result.status, 200);

    ResponseFormatter rf;
    std::string response =
        rf.Format(rm_result, parse_result.request.req.version,
                  router_result.method, parse_status);
    size_t end_of_header = response.find(CRLF CRLF);
    ASSERT_NE(end_of_header, std::string::npos);
    std::string header = response.substr(0, end_of_header + 2);

    // split by CRLF
    std::vector<std::string> header_lines;
    std::string token;
    size_t start = 0;
    for (size_t end = header.find(CRLF); end != std::string::npos;
         end = header.find(CRLF, start)) {
      header_lines.push_back(header.substr(start, end - start));
      start = end + 2;
    }
    ASSERT_EQ(header_lines.size(), 7);
    EXPECT_EQ(header_lines[0], "HTTP/1.1 200 OK");
    EXPECT_EQ(header_lines[1], "server: BrilliantServer/1.0");
    EXPECT_TRUE(header_lines[2].compare(0, 6, "date: ") == 0);
    EXPECT_EQ(header_lines[3], "allow: GET");
    EXPECT_EQ(header_lines[4], "connection: keep-alive");
    EXPECT_EQ(header_lines[5], "content-length: 88");
    EXPECT_EQ(header_lines[6], "content-type: text/html");

    std::string body = response.substr(end_of_header + 4);
    EXPECT_EQ(body, FileToString(RF_REQ_PATH_PREFIX "s_00.html"));
  }

  // s 01 general case
  {
    Validator::Result result =
        TestValidatorSuccess(RF_CONFIG_PATH_PREFIX "s_01");
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
    EXPECT_EQ(router_result.method & GET, GET);
    EXPECT_EQ(router_result.success_path, "./rf_resources/s_01.css");
    EXPECT_EQ(router_result.error_path, "./error.html");

    ResourceManager rm;
    ResourceManager::Result rm_result =
        rm.ExecuteMethod(router_result, parse_result.request.content);

    EXPECT_EQ(rm_result.status, 200);

    ResponseFormatter rf;
    std::string response =
        rf.Format(rm_result, parse_result.request.req.version,
                  router_result.method, parse_status);
    size_t end_of_header = response.find(CRLF CRLF);
    ASSERT_NE(end_of_header, std::string::npos);
    std::string header = response.substr(0, end_of_header + 2);

    // split by CRLF
    std::vector<std::string> header_lines;
    std::string token;
    size_t start = 0;
    for (size_t end = header.find(CRLF); end != std::string::npos;
         end = header.find(CRLF, start)) {
      header_lines.push_back(header.substr(start, end - start));
      start = end + 2;
    }
    ASSERT_EQ(header_lines.size(), 7);
    EXPECT_EQ(header_lines[0], "HTTP/1.1 200 OK");
    EXPECT_EQ(header_lines[1], "server: BrilliantServer/1.0");
    EXPECT_TRUE(header_lines[2].compare(0, 6, "date: ") == 0);
    EXPECT_EQ(header_lines[3], "allow: GET");
    EXPECT_EQ(header_lines[4], "connection: keep-alive");
    EXPECT_EQ(header_lines[5], "content-length: 38");
    EXPECT_EQ(header_lines[6], "content-type: text/css");

    std::string body = response.substr(end_of_header + 4);
    EXPECT_EQ(body, FileToString(RF_REQ_PATH_PREFIX "s_01.css"));
  }
}