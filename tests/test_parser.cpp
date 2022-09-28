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

// TEST(HttpParserTest, ParseRequestLine) {
// {
//   HttpParser parser;
//   HttpParser::Result result =
//       parser.Parse(FileToString(PARSER_PATH_PREFIX "s_00.txt"));
//   Request& request = result.request;
//   EXPECT_EQ(request.req.method, GET);
//   EXPECT_EQ(request.req.version, HttpParser::kHttp1_1);
//   EXPECT_EQ(request.req.path, "/");
//   EXPECT_EQ(request.req.Host, "localhost:8080");
// }

// {
//   HttpParser parser;
//   HttpParser::Result result =
//       parser.Parse(FileToString(PARSER_PATH_PREFIX "s_01.txt"));
//   Request& request = result.request;
//   EXPECT_EQ(request.req.method, DELETE);
//   EXPECT_EQ(request.req.version, HttpParser::kHttp1_1);
//   EXPECT_EQ(request.req.path, "/42");
//   EXPECT_EQ(request.req.Host, "jiskim.com");
// }

// {
//   HttpParser parser;
//   HttpParser::Result result =
//       parser.Parse(FileToString(PARSER_PATH_PREFIX "s_02.txt"));
//   Request& request = result.request;
//   EXPECT_EQ(request.req.method, DELETE);
//   EXPECT_EQ(request.req.version, HttpParser::kHttp1_0);
//   EXPECT_EQ(request.req.path, "/24");
//   EXPECT_EQ(request.req.Host, "jiskim.com:4242");
// }

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
//}

#include "test_parser.hpp"

TEST(HttpParserTest, ParseRequestLine) {
  char buffer[BUFFER_SIZE];
  {
    // request line length limit exceeded
    HttpParser parser;
    int fd = open("/Users/ghan/goinfre/max_rl.txt", O_RDONLY);

    memset(buffer, 0, BUFFER_SIZE);
    int status;
    while (read(fd, buffer, BUFFER_SIZE - 1) > 0) {
      status = parser.Parse(buffer);
      if (status == RL_LEN_ERR) {
        break;
      }
      memset(buffer, 0, BUFFER_SIZE);
    }
    EXPECT_EQ(status, RL_LEN_ERR);
    close(fd);
  }

  {
    // valid HTTP/1.0 request
    HttpParser parser;
    int fd = open("/Users/ghan/goinfre/max_rl_01.txt", O_RDONLY);

    memset(buffer, 0, BUFFER_SIZE);
    int status;
    while (read(fd, buffer, BUFFER_SIZE - 1) > 0) {
      status = parser.Parse(buffer);
      if (status == COMPLETE) {
        break;
      }
      memset(buffer, 0, BUFFER_SIZE);
    }
    ASSERT_EQ(status, COMPLETE);
    const HttpParser::Result& result = parser.get_result();
    EXPECT_EQ(result.request.req.method, GET);
    EXPECT_EQ(result.request.req.version, HttpParser::kHttp1_0);
    EXPECT_EQ(result.request.req.path, RL_MAX_PATH);
    close(fd);
  }

  {
    // valid HTTP/1.0 request
    HttpParser parser;
    int fd = open("/Users/ghan/goinfre/max_rl_02.txt", O_RDONLY);

    memset(buffer, 0, BUFFER_SIZE);
    int status;
    while (read(fd, buffer, BUFFER_SIZE - 1) > 0) {
      status = parser.Parse(buffer);
      if (status == COMPLETE) {
        break;
      }
      memset(buffer, 0, BUFFER_SIZE);
    }
    ASSERT_EQ(status, COMPLETE);
    const HttpParser::Result& result = parser.get_result();
    EXPECT_EQ(result.request.req.method, GET);
    EXPECT_EQ(result.request.req.version, HttpParser::kHttp1_0);
    EXPECT_EQ(result.request.req.path, RL_MAX_PATH);
    close(fd);
  }

  {
    // valid HTTP/1.1 DELETE request
    HttpParser parser;
    int fd = open(PARSER_PATH_PREFIX "s_00.txt", O_RDONLY);

    memset(buffer, 0, BUFFER_SIZE);
    int status;
    while (read(fd, buffer, BUFFER_SIZE - 1) > 0) {
      status = parser.Parse(buffer);
      if (status == COMPLETE) {
        break;
      }
      memset(buffer, 0, BUFFER_SIZE);
    }
    ASSERT_EQ(status, COMPLETE);
    const HttpParser::Result& result = parser.get_result();
    EXPECT_EQ(result.request.req.method, DELETE);
    EXPECT_EQ(result.request.req.version, HttpParser::kHttp1_1);
    EXPECT_EQ(result.request.req.path, "/");
    close(fd);
  }

  {
    // 501 Not Implemented
    HttpParser parser;
    int fd = open(PARSER_PATH_PREFIX "f_00.txt", O_RDONLY);

    memset(buffer, 0, BUFFER_SIZE);
    int status;
    while (read(fd, buffer, BUFFER_SIZE - 1) > 0) {
      status = parser.Parse(buffer);
      if (status == COMPLETE) {
        break;
      }
      memset(buffer, 0, BUFFER_SIZE);
    }
    ASSERT_EQ(status, COMPLETE);
    const HttpParser::Result& result = parser.get_result();
    EXPECT_EQ(result.status, 501);
    close(fd);
  }
}
