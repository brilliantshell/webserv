/**
 * @file test_resource_manager.cpp
 * @author ghan, jiskim, yongjule
 * @brief
 * @date 2022-10-11
 *
 * @copyright Copyright (c) 2022
 */

#include <gtest/gtest.h>

#include <fstream>
#include <iostream>

#include "Connection.hpp"
#include "HttpParser.hpp"
#include "ResourceManager.hpp"
#include "Router.hpp"
#include "Types.hpp"
#include "Validator.hpp"

#define RM_CONFIG_PATH_PREFIX "../configs/tests/ResourceManager/"
#define RM_REQ_PATH_PREFIX "../tests/ResourceManager/"

std::string FileToString(const std::string& file_path);
Validator::Result TestValidatorSuccess(const std::string& case_id);

/*
ResourceManager::Result {
        int status;
        std::string content;
};
*/

TEST(ResourceManager, GET) {
  // s 00 GET SUCCESS
  {
    Validator::Result result =
        TestValidatorSuccess(RM_CONFIG_PATH_PREFIX "s_00");
    PortMap port_map = result.port_map;
    EXPECT_EQ(port_map.size(), 1);
    EXPECT_EQ(port_map.count(80), 1);

    HttpParser parser;
    std::string req_buf = FileToString(RM_REQ_PATH_PREFIX "s_00.txt");
    int status = parser.Parse(req_buf);
    EXPECT_EQ(status, HttpParser::kComplete);
    HttpParser::Result parse_result = parser.get_result();

    Router router(port_map[80]);
    Router::Result route_result =
        router.Route(parse_result.status, parse_result.request);

    EXPECT_EQ(route_result.status, 200);
    EXPECT_EQ(route_result.method, GET);
    EXPECT_EQ(route_result.success_path, "./resources/s_00.html");
    EXPECT_EQ(route_result.error_path, "./resources/error.html");

    ResourceManager rm;
    ResourceManager::Result rm_result = rm.ExecuteMethod(route_result);
    EXPECT_EQ(rm_result.status, 200);
    EXPECT_EQ(rm_result.content, FileToString(RM_REQ_PATH_PREFIX "s_00.html"));
  }

  // f 00 GET FAIL 404
  {
    Validator::Result result =
        TestValidatorSuccess(RM_CONFIG_PATH_PREFIX "f_00");
    PortMap port_map = result.port_map;
    EXPECT_EQ(port_map.size(), 1);
    EXPECT_EQ(port_map.count(80), 1);

    HttpParser parser;
    std::string req_buf = FileToString(RM_REQ_PATH_PREFIX "f_00.txt");
    int status = parser.Parse(req_buf);
    EXPECT_EQ(status, HttpParser::kComplete);
    HttpParser::Result parse_result = parser.get_result();

    Router router(port_map[80]);
    Router::Result route_result =
        router.Route(parse_result.status, parse_result.request);

    EXPECT_EQ(route_result.status, 200);
    EXPECT_EQ(route_result.method, GET);
    EXPECT_EQ(route_result.success_path, "./resources/f_00.html");
    EXPECT_EQ(route_result.error_path, "./resources/error.html");

    ResourceManager rm;
    ResourceManager::Result rm_result = rm.ExecuteMethod(route_result);
    EXPECT_EQ(rm_result.status, 404);
    EXPECT_EQ(rm_result.content, FileToString(RM_REQ_PATH_PREFIX "error.html"));
  }

  // f 01 GET FAIL 403
  {
    Validator::Result result =
        TestValidatorSuccess(RM_CONFIG_PATH_PREFIX "f_01");
    PortMap port_map = result.port_map;
    EXPECT_EQ(port_map.size(), 1);
    EXPECT_EQ(port_map.count(80), 1);

    HttpParser parser;
    std::string req_buf = FileToString(RM_REQ_PATH_PREFIX "f_01.txt");
    int status = parser.Parse(req_buf);
    EXPECT_EQ(status, HttpParser::kComplete);
    HttpParser::Result parse_result = parser.get_result();

    Router router(port_map[80]);
    Router::Result route_result =
        router.Route(parse_result.status, parse_result.request);

    EXPECT_EQ(route_result.status, 200);
    EXPECT_EQ(route_result.method, GET);
    EXPECT_EQ(route_result.success_path, "./resources/f_01.html");
    EXPECT_EQ(route_result.error_path, "./resources/error.html");

    ResourceManager rm;
    ResourceManager::Result rm_result = rm.ExecuteMethod(route_result);
    EXPECT_EQ(rm_result.status, 403);
    EXPECT_EQ(rm_result.content, FileToString(RM_REQ_PATH_PREFIX "error.html"));
  }

  // f 02 GET FAIL 500 (ERROR, BUT NO ERROR PAGE)
  {
    Validator::Result result =
        TestValidatorSuccess(RM_CONFIG_PATH_PREFIX "f_02");
    PortMap port_map = result.port_map;
    EXPECT_EQ(port_map.size(), 1);
    EXPECT_EQ(port_map.count(80), 1);

    HttpParser parser;
    std::string req_buf = FileToString(RM_REQ_PATH_PREFIX "f_02.txt");
    int status = parser.Parse(req_buf);
    EXPECT_EQ(status, HttpParser::kComplete);
    HttpParser::Result parse_result = parser.get_result();

    Router router(port_map[80]);
    Router::Result route_result =
        router.Route(parse_result.status, parse_result.request);

    EXPECT_EQ(route_result.status, 200);
    EXPECT_EQ(route_result.method, GET);
    EXPECT_EQ(route_result.success_path, "./resources/f_02.html");
    EXPECT_EQ(route_result.error_path, "./resources/sinnarisyeos.sinna");

    ResourceManager rm;
    ResourceManager::Result rm_result = rm.ExecuteMethod(route_result);
    EXPECT_EQ(rm_result.status, 500);
  }

  // f 03 ERROR PATH IS DIRECTORY - http1.1
  {
    Validator::Result result =
        TestValidatorSuccess(RM_CONFIG_PATH_PREFIX "f_03");
    PortMap port_map = result.port_map;
    EXPECT_EQ(port_map.size(), 1);
    EXPECT_EQ(port_map.count(80), 1);

    HttpParser parser;
    std::string req_buf = FileToString(RM_REQ_PATH_PREFIX "f_03.txt");
    int status = parser.Parse(req_buf);
    EXPECT_EQ(status, HttpParser::kClose);
    HttpParser::Result parse_result = parser.get_result();

    Router router(port_map[80]);
    Router::Result route_result =
        router.Route(parse_result.status, parse_result.request);

    EXPECT_EQ(route_result.status, 200);
    EXPECT_EQ(route_result.method, GET);
    EXPECT_EQ(route_result.success_path, "./resources/f_03.html");
    EXPECT_EQ(route_result.error_path, "./resources");

    ResourceManager rm;
    ResourceManager::Result rm_result = rm.ExecuteMethod(route_result);
    EXPECT_EQ(rm_result.status, 500);
  }

  // s 01 AUTOINDEX
  {
    Validator::Result result =
        TestValidatorSuccess(RM_CONFIG_PATH_PREFIX "s_01");
    PortMap port_map = result.port_map;
    EXPECT_EQ(port_map.size(), 1);
    EXPECT_EQ(port_map.count(80), 1);

    HttpParser parser;
    std::string req_buf = FileToString(RM_REQ_PATH_PREFIX "s_01.txt");
    int status = parser.Parse(req_buf);
    EXPECT_EQ(status, HttpParser::kComplete);
    HttpParser::Result parse_result = parser.get_result();

    Router router(port_map[80]);
    Router::Result route_result =
        router.Route(parse_result.status, parse_result.request);

    EXPECT_EQ(route_result.status, 200);
    EXPECT_EQ(route_result.method, GET);
    EXPECT_EQ(route_result.success_path, "./_deps/");
    EXPECT_EQ(route_result.error_path, "./resources/error.html");

    ResourceManager rm;
    ResourceManager::Result rm_result = rm.ExecuteMethod(route_result);
    EXPECT_EQ(rm_result.status, 200);
    EXPECT_EQ(rm_result.content,
              "<!DOCTYPE html><html><title>Index of /_deps/\
</title><body><h1>Index of \
/_deps/</h1><hr><pre>\n<a \
href='./googletest-build/'>googletest-build/</a>\n<a \
href='./googletest-src/'>googletest-src/</a>\n<a \
href='./googletest-subbuild/'>googletest-subbuild/</a>\n</\
pre><hr></body></html>");
  }

  // s 02 AUTOINDEX with file
  {
    Validator::Result result =
        TestValidatorSuccess(RM_CONFIG_PATH_PREFIX "s_02");
    PortMap port_map = result.port_map;
    EXPECT_EQ(port_map.size(), 1);
    EXPECT_EQ(port_map.count(80), 1);

    HttpParser parser;
    std::string req_buf = FileToString(RM_REQ_PATH_PREFIX "s_02.txt");
    int status = parser.Parse(req_buf);
    EXPECT_EQ(status, HttpParser::kComplete);
    HttpParser::Result parse_result = parser.get_result();

    Router router(port_map[80]);
    Router::Result route_result =
        router.Route(parse_result.status, parse_result.request);

    EXPECT_EQ(route_result.status, 200);
    EXPECT_EQ(route_result.method, GET);
    EXPECT_EQ(route_result.success_path, "./_deps/googletest-build/");
    EXPECT_EQ(route_result.error_path, "./resources/error.html");

    ResourceManager rm;
    ResourceManager::Result rm_result = rm.ExecuteMethod(route_result);
    EXPECT_EQ(rm_result.status, 200);
    EXPECT_EQ(rm_result.content,
              "<!DOCTYPE html><html><title>Index of /_deps/googletest-build/\
</title><body><h1>Index of \
/_deps/googletest-build/</h1><hr><pre>\n<a \
href='./CMakeFiles/'>CMakeFiles/</a>\n<a \
href='./googlemock/'>googlemock/</a>\n<a \
href='./googletest/'>googletest/</a>\n<a \
href='./CTestTestfile.cmake'>CTestTestfile.cmake</a>\n<a \
href='./Makefile'>Makefile</a>\n<a \
href='./cmake_install.cmake'>cmake_install.cmake</a>\n</\
pre><hr></body></html>");
  }

  // s 03 AUTOINDEX off, response index file s_03.html
  {
    Validator::Result result =
        TestValidatorSuccess(RM_CONFIG_PATH_PREFIX "s_03");
    PortMap port_map = result.port_map;
    EXPECT_EQ(port_map.size(), 1);
    EXPECT_EQ(port_map.count(80), 1);

    HttpParser parser;
    std::string req_buf = FileToString(RM_REQ_PATH_PREFIX "s_03.txt");
    int status = parser.Parse(req_buf);
    EXPECT_EQ(status, HttpParser::kComplete);
    HttpParser::Result parse_result = parser.get_result();

    Router router(port_map[80]);
    Router::Result route_result =
        router.Route(parse_result.status, parse_result.request);

    EXPECT_EQ(route_result.status, 200);
    EXPECT_EQ(route_result.method, GET);
    EXPECT_EQ(route_result.success_path, "./resources/s_03.html");
    EXPECT_EQ(route_result.error_path, "./resources/error.html");

    ResourceManager rm;
    ResourceManager::Result rm_result = rm.ExecuteMethod(route_result);
    EXPECT_EQ(rm_result.status, 200);
    EXPECT_EQ(rm_result.content, FileToString(RM_REQ_PATH_PREFIX "s_03.html"));
  }
}
