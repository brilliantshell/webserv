#include <gtest/gtest.h>

#include <fstream>
#include <iostream>

#include "CgiManager.hpp"
#include "Connection.hpp"
#include "HttpParser.hpp"
#include "ResourceManager.hpp"
#include "Router.hpp"
#include "Types.hpp"
#include "Validator.hpp"

#define CM_CONFIG_PATH_PREFIX "../configs/tests/CgiManager/"
#define CM_REQ_PATH_PREFIX "../tests/CgiManager/"

std::string FileToString(const std::string& file_path);
Validator::Result TestValidatorSuccess(const std::string& case_id);

TEST(CgiManagerTest, TestCgiManager) {
  // s 00 general case
  {
    Validator::Result result =
        TestValidatorSuccess(CM_CONFIG_PATH_PREFIX "s_00");
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
    EXPECT_EQ(router_result.method & GET, GET);
    EXPECT_EQ(router_result.success_path, "./resources/cgi/cgi.php");
    EXPECT_EQ(router_result.error_path, "./error.html");

    ResourceManager rm;
    ResourceManager::Result rm_result =
        rm.ExecuteMethod(router_result, parse_result.request.content);

    const char** cgi_env = router_result.cgi_env.get_env();
    std::stringstream ss;
    for (int i = 0; cgi_env[i]; ++i)
      ss << "envp [" << i << "] : " << cgi_env[i] << "\n";

    EXPECT_EQ(rm_result.status, 200);
    EXPECT_EQ(rm_result.content, ss.str());
    EXPECT_EQ(rm_result.header[0], "Content-Type: text/plain");
    EXPECT_EQ(rm_result.location, "");
  }
}
