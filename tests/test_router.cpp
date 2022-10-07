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
#include <sstream>

#include "Connection.hpp"
#include "HttpParser.hpp"
#include "ServerRouter.hpp"
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

    ServerRouter router = port_map[4242];
    ServerRouter::Result route_result =
        router.Route(parse_result.status, parse_result.request);

    EXPECT_EQ(route_result.status, 200);
    EXPECT_EQ(route_result.method, GET);
    EXPECT_EQ(route_result.success_path, ".//normal/index.html");
    EXPECT_EQ(route_result.error_path, "./error.html");
    EXPECT_EQ(route_result.param, "");
  }
}
