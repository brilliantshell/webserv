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

TEST(ResourceManager, GETMethod) {
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
    ResourceManager::Result rm_result =
        rm.ExecuteMethod(route_result, parse_result.request.content);
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
    ResourceManager::Result rm_result =
        rm.ExecuteMethod(route_result, parse_result.request.content);
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

    chmod("./resources/f_01.html", 0111);
    Router router(port_map[80]);
    Router::Result route_result =
        router.Route(parse_result.status, parse_result.request);

    EXPECT_EQ(route_result.status, 200);
    EXPECT_EQ(route_result.method, GET);
    EXPECT_EQ(route_result.success_path, "./resources/f_01.html");
    EXPECT_EQ(route_result.error_path, "./resources/error.html");

    ResourceManager rm;
    ResourceManager::Result rm_result =
        rm.ExecuteMethod(route_result, parse_result.request.content);
    EXPECT_EQ(rm_result.status, 403);
    EXPECT_EQ(rm_result.content, FileToString(RM_REQ_PATH_PREFIX "error.html"));
    std::cout << "여기서 터지면 chmod를 생각해보렴\n";
    chmod("./resources/f_01.html", 0777);
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
    ResourceManager::Result rm_result =
        rm.ExecuteMethod(route_result, parse_result.request.content);
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
    ResourceManager::Result rm_result =
        rm.ExecuteMethod(route_result, parse_result.request.content);
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
    ResourceManager::Result rm_result =
        rm.ExecuteMethod(route_result, parse_result.request.content);
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
    ResourceManager::Result rm_result =
        rm.ExecuteMethod(route_result, parse_result.request.content);
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
    ResourceManager::Result rm_result =
        rm.ExecuteMethod(route_result, parse_result.request.content);
    EXPECT_EQ(rm_result.status, 200);
    EXPECT_EQ(rm_result.content, FileToString(RM_REQ_PATH_PREFIX "s_03.html"));
  }

  // f 04 GET file name too long
  {
    Validator::Result result =
        TestValidatorSuccess(RM_CONFIG_PATH_PREFIX "f_04");
    PortMap port_map = result.port_map;
    EXPECT_EQ(port_map.size(), 1);
    EXPECT_EQ(port_map.count(80), 1);

    HttpParser parser;
    std::string req_buf = FileToString(RM_REQ_PATH_PREFIX "f_04.txt");
    int status = parser.Parse(req_buf);
    EXPECT_EQ(status, HttpParser::kComplete);
    HttpParser::Result parse_result = parser.get_result();

    Router router(port_map[80]);
    Router::Result route_result =
        router.Route(parse_result.status, parse_result.request);

    EXPECT_EQ(route_result.status, 200);
    EXPECT_EQ(route_result.method, GET);
    EXPECT_EQ(
        route_result.success_path,
        "./resources/"
        "b5bceb60fa34881dc1edd5954020dafb83abe747b9131a99c0be79940c40e1d10031de"
        "39909cbf085e8095f46cbd056ba6c951c0972346aa0895baada73e3d20270860793ee2"
        "1ae203f8093db247201bd282d1ccdb06909cd292b34ab8b42e4485b511ba638beebb22"
        "58cc26b28ce6205eaf4380e12a6ea7944afb6fbdc93ce567");
    EXPECT_EQ(route_result.error_path, "./resources/error.html");

    ResourceManager rm;
    ResourceManager::Result rm_result =
        rm.ExecuteMethod(route_result, parse_result.request.content);
    EXPECT_EQ(rm_result.status, 500);
    EXPECT_EQ(rm_result.content, FileToString(RM_REQ_PATH_PREFIX "error.html"));
  }
}

// if 중복 file_001 형식으로 생성
TEST(ResourceManager, POSTMethod) {
  // s 04 file upload
  {
    Validator::Result result =
        TestValidatorSuccess(RM_CONFIG_PATH_PREFIX "s_04");
    PortMap port_map = result.port_map;
    EXPECT_EQ(port_map.size(), 1);
    EXPECT_EQ(port_map.count(80), 1);

    HttpParser parser;
    std::string req_buf = FileToString(RM_REQ_PATH_PREFIX "s_04.txt");
    int status = parser.Parse(req_buf);
    EXPECT_EQ(status, HttpParser::kComplete);
    HttpParser::Result parse_result = parser.get_result();

    Router router(port_map[80]);
    Router::Result route_result =
        router.Route(parse_result.status, parse_result.request);

    EXPECT_EQ(route_result.status, 200);
    EXPECT_EQ(route_result.method, POST);
    EXPECT_EQ(route_result.success_path, "./resources/post/s_04.txt");
    EXPECT_EQ(route_result.error_path, "./resources/error.html");

    ResourceManager rm;
    ResourceManager::Result rm_result =
        rm.ExecuteMethod(route_result, parse_result.request.content);
    EXPECT_EQ(rm_result.status, 201);  // CREATED
    ASSERT_EQ(access("./resources/post/s_04.txt", F_OK), 0);
    EXPECT_EQ(parse_result.request.content,
              FileToString(RM_REQ_PATH_PREFIX "post/s_04.txt"));
    unlink("./resources/post/s_04.txt");
  }

  // f 05 file upload to read-only directory
  {
    Validator::Result result =
        TestValidatorSuccess(RM_CONFIG_PATH_PREFIX "f_05");
    PortMap port_map = result.port_map;
    EXPECT_EQ(port_map.size(), 1);
    EXPECT_EQ(port_map.count(80), 1);

    HttpParser parser;
    std::string req_buf = FileToString(RM_REQ_PATH_PREFIX "f_05.txt");
    int status = parser.Parse(req_buf);
    EXPECT_EQ(status, HttpParser::kComplete);
    HttpParser::Result parse_result = parser.get_result();

    Router router(port_map[80]);
    Router::Result route_result =
        router.Route(parse_result.status, parse_result.request);

    EXPECT_EQ(route_result.status, 200);
    EXPECT_EQ(route_result.method, POST);
    EXPECT_EQ(route_result.success_path,
              "./resources/post/unauthorized/f_05.html");
    EXPECT_EQ(route_result.error_path, "./resources/error.html");

    ResourceManager rm;
    ResourceManager::Result rm_result =
        rm.ExecuteMethod(route_result, parse_result.request.content);
    EXPECT_EQ(rm_result.status, 403);  // CREATED
    errno = 0;
    ASSERT_EQ(access("./resources/post/unauthorized/f_05.txt", F_OK), -1);
    ASSERT_EQ(errno, ENOENT);
  }

  // s 05 file already exist
  {
    Validator::Result result =
        TestValidatorSuccess(RM_CONFIG_PATH_PREFIX "s_05");
    PortMap port_map = result.port_map;
    EXPECT_EQ(port_map.size(), 1);
    EXPECT_EQ(port_map.count(80), 1);

    HttpParser parser;
    std::string req_buf = FileToString(RM_REQ_PATH_PREFIX "s_05.txt");
    int status = parser.Parse(req_buf);
    EXPECT_EQ(status, HttpParser::kComplete);
    HttpParser::Result parse_result = parser.get_result();

    Router router(port_map[80]);
    Router::Result route_result =
        router.Route(parse_result.status, parse_result.request);

    EXPECT_EQ(route_result.status, 200);
    EXPECT_EQ(route_result.method, POST);
    EXPECT_EQ(route_result.success_path, "./resources/post/s_05.txt");
    EXPECT_EQ(route_result.error_path, "./resources/error.html");

    ResourceManager rm;
    ResourceManager::Result rm_result =
        rm.ExecuteMethod(route_result, parse_result.request.content);
    EXPECT_EQ(rm_result.status, 201);  // CREATED
    ASSERT_EQ(access("./resources/post/s_05_0.txt", F_OK), 0);
    EXPECT_EQ(parse_result.request.content,
              FileToString(RM_REQ_PATH_PREFIX "post/s_05_0.txt"));
    std::cout << rm_result.content << '\n';
    unlink("./resources/post/s_05_0.txt");
  }

  // s 06 file already exist
  {
    Validator::Result result =
        TestValidatorSuccess(RM_CONFIG_PATH_PREFIX "s_06");
    PortMap port_map = result.port_map;
    EXPECT_EQ(port_map.size(), 1);
    EXPECT_EQ(port_map.count(80), 1);
    HttpParser parser;
    std::string req_buf = FileToString(RM_REQ_PATH_PREFIX "s_06.txt");
    int status = parser.Parse(req_buf);
    EXPECT_EQ(status, HttpParser::kComplete);
    HttpParser::Result parse_result = parser.get_result();

    Router router(port_map[80]);
    Router::Result route_result =
        router.Route(parse_result.status, parse_result.request);

    EXPECT_EQ(route_result.status, 200);
    EXPECT_EQ(route_result.method, POST);
    EXPECT_EQ(route_result.success_path, "./resources/post/s_06.txt");
    EXPECT_EQ(route_result.error_path, "./resources/error.html");

    ResourceManager rm;
    ResourceManager::Result rm_result =
        rm.ExecuteMethod(route_result, parse_result.request.content);
    EXPECT_EQ(rm_result.status, 201);  // CREATED
    ASSERT_EQ(access("./resources/post/s_06.txt", F_OK), 0);
    EXPECT_EQ(parse_result.request.content,
              FileToString(RM_REQ_PATH_PREFIX "post/s_06.txt"));
    std::cout << rm_result.content << '\n';
    unlink("./resources/post/s_06.txt");
  }
}

TEST(ResourceManager, DELETEMethod) {
  // s 07 DELETE A FILE
  {
    Validator::Result result =
        TestValidatorSuccess(RM_CONFIG_PATH_PREFIX "s_07");
    PortMap port_map = result.port_map;
    EXPECT_EQ(port_map.size(), 1);
    EXPECT_EQ(port_map.count(80), 1);
    HttpParser parser;
    std::string req_buf = FileToString(RM_REQ_PATH_PREFIX "s_07.txt");
    int status = parser.Parse(req_buf);
    EXPECT_EQ(status, HttpParser::kComplete);
    HttpParser::Result parse_result = parser.get_result();

    Router router(port_map[80]);
    Router::Result route_result =
        router.Route(parse_result.status, parse_result.request);

    EXPECT_EQ(route_result.status, 200);
    EXPECT_EQ(route_result.method, DELETE);
    EXPECT_EQ(route_result.success_path, "./resources/delete/s_07.txt");
    EXPECT_EQ(route_result.error_path, "./resources/error.html");
    {
      std::ofstream ofs("./resources/delete/s_07.txt");
      ofs << "smart";
    }
    ASSERT_EQ(access("./resources/delete/s_07.txt", F_OK), 0);
    ResourceManager rm;
    ResourceManager::Result rm_result =
        rm.ExecuteMethod(route_result, parse_result.request.content);
    EXPECT_EQ(rm_result.status, 200);  // CREATED
    ASSERT_EQ(access("./resources/delete/s_07.txt", F_OK), -1);
    std::cout << rm_result.content << '\n';
    unlink("./resources/delete/s_07.txt");
  }

  // f 06 DELETE A FILE, NO WRITE PERMISSION
  {
    Validator::Result result =
        TestValidatorSuccess(RM_CONFIG_PATH_PREFIX "f_06");
    PortMap port_map = result.port_map;
    EXPECT_EQ(port_map.size(), 1);
    EXPECT_EQ(port_map.count(80), 1);
    HttpParser parser;
    std::string req_buf = FileToString(RM_REQ_PATH_PREFIX "f_06.txt");
    int status = parser.Parse(req_buf);
    EXPECT_EQ(status, HttpParser::kComplete);
    HttpParser::Result parse_result = parser.get_result();

    Router router(port_map[80]);
    Router::Result route_result =
        router.Route(parse_result.status, parse_result.request);

    EXPECT_EQ(route_result.status, 200);
    EXPECT_EQ(route_result.method, DELETE);
    EXPECT_EQ(route_result.success_path, "./resources/delete/f_06.txt");
    EXPECT_EQ(route_result.error_path, "./resources/error.html");
    {
      std::ofstream ofs("./resources/delete/f_06.txt");
      ofs << "smart";
      if (ofs.fail())
        std::cerr << "file open failed" << strerror(errno) << "\n";
      chmod("./resources/delete/f_06.txt", 0444);
    }
    ASSERT_EQ(access("./resources/delete/f_06.txt", F_OK), 0);
    ResourceManager rm;
    ResourceManager::Result rm_result =
        rm.ExecuteMethod(route_result, parse_result.request.content);
    EXPECT_EQ(rm_result.status, 403);  // FORBIDDEN`
    ASSERT_EQ(access("./resources/delete/f_06.txt", F_OK), 0);
    chmod("./resources/delete/f_06.txt", 0777);
    unlink("./resources/delete/f_06.txt");
  }

  // f 07 DELETE file not exist
  {
    Validator::Result result =
        TestValidatorSuccess(RM_CONFIG_PATH_PREFIX "f_07");
    PortMap port_map = result.port_map;
    EXPECT_EQ(port_map.size(), 1);
    EXPECT_EQ(port_map.count(80), 1);
    HttpParser parser;
    std::string req_buf = FileToString(RM_REQ_PATH_PREFIX "f_07.txt");
    int status = parser.Parse(req_buf);
    EXPECT_EQ(status, HttpParser::kComplete);
    HttpParser::Result parse_result = parser.get_result();

    Router router(port_map[80]);
    Router::Result route_result =
        router.Route(parse_result.status, parse_result.request);

    EXPECT_EQ(route_result.status, 200);
    EXPECT_EQ(route_result.method, DELETE);
    EXPECT_EQ(route_result.success_path, "./resources/delete/f_07.txt");
    EXPECT_EQ(route_result.error_path, "./resources/error.html");
    ASSERT_EQ(access("./resources/delete/f_07.txt", F_OK), -1);
    ResourceManager rm;
    ResourceManager::Result rm_result =
        rm.ExecuteMethod(route_result, parse_result.request.content);
    EXPECT_EQ(rm_result.status, 404);  // PAGE NOT FOUND
    ASSERT_EQ(access("./resources/delete/f_07.txt", F_OK), -1);
  }
}
