#include <gtest/gtest.h>

#include <fstream>
#include <iostream>
#include <sstream>

#include "Connection.hpp"
#include "HttpParser.hpp"
#include "Types.hpp"
#include "Validator.hpp"

#define PARSER_PATH_PREFIX "../tests/HttpParser/"

/**

typedef std::map<std::string, std::list<std::string> > Fields;

struct RequestLine {
        uint8_t method;
        uint8_t version;
        std::string path;
        std::string Host;
};

struct Request {
        RequestLine req;
        Fields header;
        Fields trailer; // NULLABLE
        std::string content; // NULLABLE
};

HttpParser::Result {
        int status;
        Request request;
};

 */

std::string FileToString(const std::string& file_path);

int OpenFile(const std::string& file_path) {
  int fd = open(file_path.c_str(), O_RDONLY);
  if (fd == -1) {
    std::cerr << "Failed to open file: " << file_path << std::endl;
    exit(1);
  }
  return fd;
}

TEST(HttpParserTest, ParseRequestLine) {
  {
    HttpParser parser;
    HttpParser::Result result =
        parser.Parse(FileToString(PARSER_PATH_PREFIX "s_00.txt"));
    Request& request = result.request;
    EXPECT_EQ(request.req.method, GET);
    EXPECT_EQ(request.req.version, HttpParser::kHttp1_1);
    EXPECT_EQ(request.req.path, "/");
    EXPECT_EQ(request.req.Host, "localhost:8080");
  }

  {
    HttpParser parser;
    HttpParser::Result result =
        parser.Parse(FileToString(PARSER_PATH_PREFIX "s_01.txt"));
    Request& request = result.request;
    EXPECT_EQ(request.req.method, DELETE);
    EXPECT_EQ(request.req.version, HttpParser::kHttp1_1);
    EXPECT_EQ(request.req.path, "/42");
    EXPECT_EQ(request.req.Host, "jiskim.com");
  }

  {
    HttpParser parser;
    HttpParser::Result result =
        parser.Parse(FileToString(PARSER_PATH_PREFIX "s_02.txt"));
    Request& request = result.request;
    EXPECT_EQ(request.req.method, DELETE);
    EXPECT_EQ(request.req.version, HttpParser::kHttp1_0);
    EXPECT_EQ(request.req.path, "/24");
    EXPECT_EQ(request.req.Host, "jiskim.com:4242");
  }

  // {
  //   HttpParser parser;
  //   HttpParser::Result result =
  //       parser.Parse(FileToString(PARSER_PATH_PREFIX "s_03.txt"));
  //   Request& request = result.request;
  //   EXPECT_EQ(request.req.method, DELETE);
  //   EXPECT_EQ(request.req.version, HttpParser::kHttp1_0);
  //   EXPECT_EQ(request.req.path, "/24");
  //   EXPECT_EQ(request.req.Host, "jiskim.com:4242");
  // }
}

TEST(HttpParserTest, CheckRequestLength) {
  {
    Connection connection;
    connection.set_fd(OpenFile("../../max.txt"));
    connection.Receive();
  }
}
