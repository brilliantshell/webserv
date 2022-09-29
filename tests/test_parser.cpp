#include <gtest/gtest.h>

#include <fstream>
#include <iostream>
#include <sstream>

#include "Connection.hpp"
#include "HttpParser.hpp"
#include "Types.hpp"
#include "Validator.hpp"

#define PARSER_PATH_PREFIX "../tests/HttpParser/"
#define GOINFRE_PATH "/Users/yongjule/goinfre/"

/**

scheme
host
path

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

#include "test_parser.hpp"

void TestParseError(const std::string& file_path, int parser_status,
                    int request_status) {
  HttpParser parser;
  char buffer[BUFFER_SIZE];

  int fd = open(file_path.c_str(), O_RDONLY);

  memset(buffer, 0, BUFFER_SIZE);
  int status;
  while (read(fd, buffer, BUFFER_SIZE - 1) > 0) {
    status = parser.Parse(buffer);
    if (status == COMPLETE) {
      break;
    }
    memset(buffer, 0, BUFFER_SIZE);
  }
  ASSERT_EQ(status, parser_status) << file_path << "\n";
  const HttpParser::Result& result = parser.get_result();
  EXPECT_EQ(result.status, request_status);
  close(fd);
}

TEST(UriParserTest, ValidateURI) {
  // Origin Form URI
  {
    UriParser uri_parser;
    UriParser::Result result = uri_parser.Parse("/^");
    EXPECT_EQ(result.is_valid, false);
  }

  {
    UriParser uri_parser;
    UriParser::Result result = uri_parser.Parse("/ghan");
    EXPECT_EQ(result.is_valid, true);
    EXPECT_EQ(result.path, "/ghan");
    EXPECT_EQ(result.host.size(), 0);
  }

  {
    UriParser uri_parser;
    UriParser::Result result = uri_parser.Parse("//%5E");
    EXPECT_EQ(result.is_valid, true);
    EXPECT_EQ(result.path, "//^");
    EXPECT_EQ(result.host.size(), 0);
  }

  {
    UriParser uri_parser;
    UriParser::Result result = uri_parser.Parse(
        "/search.naver?where=nexearch&sm=top_hty&fbm=1&ie=utf8&query=%EC%9D%B4%"
        "EA%B2%83%EC%9D%80+%EA%B2%80%EC%83%89+%EC%BF%BC%EB%A6%AC%EB%AC%B8+%EC%"
        "9E%85%EB%8B%88%EB%8B%A4");
    EXPECT_EQ(result.is_valid, true);
    EXPECT_EQ(result.host.size(), 0);
    EXPECT_EQ(result.path, "/search.naver");
    EXPECT_EQ(result.query,
              "?where=nexearch&sm=top_hty&fbm=1&ie=utf8&query=이것은+검색+\
쿼리문+입니다");
  }

  // Absolute Form URI
  {
    UriParser uri_parser;
    UriParser::Result result = uri_parser.Parse(
        "http://naver.com/"
        "search.naver?where=nexearch&sm=top_hty&fbm=1&ie=utf8&query=%EC%9D%B4%"
        "EA%B2%83%EC%9D%80+%EA%B2%80%EC%83%89+%EC%BF%BC%EB%A6%AC%EB%AC%B8+%EC%"
        "9E%85%EB%8B%88%EB%8B%A4");
    EXPECT_EQ(result.is_valid, true);
    EXPECT_EQ(result.host, "naver.com");
    EXPECT_EQ(result.path, "/search.naver");
    EXPECT_EQ(result.query,
              "?where=nexearch&sm=top_hty&fbm=1&ie=utf8&query=이것은+검색+\
쿼리문+입니다");
  }

  {
    UriParser uri_parser;
    UriParser::Result result = uri_parser.Parse(
        "http://naver.com:42424/"
        "search.naver?where=nexearch&sm=top_hty&fbm=1&ie=utf8&query=%EC%9D%B4%"
        "EA%B2%83%EC%9D%80+%EA%B2%80%EC%83%89+%EC%BF%BC%EB%A6%AC%EB%AC%B8+%EC%"
        "9E%85%EB%8B%88%EB%8B%A4");
    EXPECT_EQ(result.is_valid, true);
    EXPECT_EQ(result.host, "naver.com");
    EXPECT_EQ(result.path, "/search.naver");
    EXPECT_EQ(result.query,
              "?where=nexearch&sm=top_hty&fbm=1&ie=utf8&query=이것은+검색+\
쿼리문+입니다");
  }

  {
    UriParser uri_parser;
    UriParser::Result result = uri_parser.Parse(
        "http://:42424/"
        "search.naver?where=nexearch&sm=top_hty&fbm=1&ie=utf8&query=%EC%9D%B4%"
        "EA%B2%83%EC%9D%80+%EA%B2%80%EC%83%89+%EC%BF%BC%EB%A6%AC%EB%AC%B8+%EC%"
        "9E%85%EB%8B%88%EB%8B%A4");
    EXPECT_EQ(result.is_valid, false);
  }

  {
    UriParser uri_parser;
    UriParser::Result result = uri_parser.Parse("foo://bar:42424");
    EXPECT_EQ(result.is_valid, true);
    EXPECT_EQ(result.host, "bar");
    EXPECT_EQ(result.path, "/");
  }

  {
    UriParser uri_parser;
    UriParser::Result result = uri_parser.Parse("foo://bar:42a");
    EXPECT_EQ(result.is_valid, false);
  }

  {
    UriParser uri_parser;
    UriParser::Result result = uri_parser.Parse("foo://ba:/abba?bba");
    EXPECT_EQ(result.is_valid, true);
    EXPECT_EQ(result.host, "ba");
    EXPECT_EQ(result.path, "/abba");
    EXPECT_EQ(result.query, "?bba");
  }

  {
    UriParser uri_parser;
    UriParser::Result result = uri_parser.Parse("f^oo://ba:/abba?bba");
    EXPECT_EQ(result.is_valid, false);
  }
}

TEST(HttpParserTest, ParseRequestLine) {
  char buffer[BUFFER_SIZE];
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

  // 501 Not Implemented
  TestParseError(PARSER_PATH_PREFIX "f_00.txt", COMPLETE, 501);

  // max 03 - request line 길이 초과 (414)
  TestParseError(GOINFRE_PATH "max_rl_03.txt", RL_LEN_ERR, 414);

  // f 01 너무 긴 대문자 method (501)
  TestParseError(PARSER_PATH_PREFIX "f_01.txt", COMPLETE, 501);

  // f 02 소문자 method (400)
  TestParseError(PARSER_PATH_PREFIX "f_02.txt", COMPLETE, 400);

  // f 03 & 04 유효하지 않은 토큰 개수 (400)
  TestParseError(PARSER_PATH_PREFIX "f_03.txt", COMPLETE, 400);
  TestParseError(PARSER_PATH_PREFIX "f_04.txt", COMPLETE, 400);

  // request target 에 유효하지 않은 문자 (400)
  TestParseError(PARSER_PATH_PREFIX "f_05.txt", COMPLETE, 400);

  // s 00 - valid HTTP/1.1 DELETE request
  {
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

  // s 01 - valid HTTP/1.1 DELETE request
  {
    HttpParser parser;
    int fd = open(PARSER_PATH_PREFIX "s_01.txt", O_RDONLY);

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
    EXPECT_EQ(result.request.req.path, "/42");
    close(fd);
  }

  // f 06 & 07 - invalid HTTP/1.1 DELETE request
  TestParseError(PARSER_PATH_PREFIX "f_06.txt", COMPLETE, 400);
  TestParseError(PARSER_PATH_PREFIX "f_07.txt", COMPLETE, 400);

  // s 02 - valid HTTP/1.1 GET request
  {
    HttpParser parser;
    int fd = open(PARSER_PATH_PREFIX "s_02.txt", O_RDONLY);

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
    EXPECT_EQ(result.request.req.version, HttpParser::kHttp1_1);
    EXPECT_EQ(result.request.req.host, "naver.com");
    EXPECT_EQ(result.request.req.path, "/himynameis");
    EXPECT_EQ(result.request.req.query, "?daniel");
    close(fd);
  }

  // f 08 - invalid HTTP version request
  TestParseError(PARSER_PATH_PREFIX "f_08.txt", COMPLETE, 505);
}