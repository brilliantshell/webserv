#include <gtest/gtest.h>

#include <fstream>
#include <iostream>
#include <sstream>

#include "Connection.hpp"
#include "HttpParser.hpp"
#include "Types.hpp"
#include "Validator.hpp"

#define PARSER_PATH_PREFIX "../tests/HttpParser/"
#define GOINFRE_PATH "/Users/jisukim/goinfre/"

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

// #include "test_parser.hpp"

void TestParseError(const std::string& file_path, int parser_status,
                    int request_status) {
  HttpParser parser;
  char buffer[BUFFER_SIZE];

  int fd = open(file_path.c_str(), O_RDONLY);

  memset(buffer, 0, BUFFER_SIZE);
  int status;
  while (read(fd, buffer, BUFFER_SIZE - 1) > 0) {
    status = parser.Parse(buffer);
    if (status >= HttpParser::kComplete) {
      break;
    }
    memset(buffer, 0, BUFFER_SIZE);
  }
  ASSERT_EQ(status, parser_status) << file_path << "\n";
  HttpParser::Result& result = parser.get_result();
  EXPECT_EQ(result.status, request_status);
  close(fd);
}

TEST(UriParserTest, ValidateURI) {
  // Origin Form URI
  {
    UriParser uri_parser;
    UriParser::Result result = uri_parser.ParseTarget("/^");
    EXPECT_EQ(result.is_valid, false);
  }

  {
    UriParser uri_parser;
    UriParser::Result result = uri_parser.ParseTarget("/ghan");
    EXPECT_EQ(result.is_valid, true);
    EXPECT_EQ(result.path, "/ghan");
    EXPECT_EQ(result.host.size(), 0);
  }

  {
    UriParser uri_parser;
    UriParser::Result result = uri_parser.ParseTarget("//%5E");
    EXPECT_EQ(result.is_valid, true);
    EXPECT_EQ(result.path, "//^");
    EXPECT_EQ(result.host.size(), 0);
  }

  {
    UriParser uri_parser;
    UriParser::Result result = uri_parser.ParseTarget(
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
    UriParser::Result result = uri_parser.ParseTarget(
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
    UriParser::Result result = uri_parser.ParseTarget(
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
    UriParser::Result result = uri_parser.ParseTarget(
        "http://:42424/"
        "search.naver?where=nexearch&sm=top_hty&fbm=1&ie=utf8&query=%EC%9D%B4%"
        "EA%B2%83%EC%9D%80+%EA%B2%80%EC%83%89+%EC%BF%BC%EB%A6%AC%EB%AC%B8+%EC%"
        "9E%85%EB%8B%88%EB%8B%A4");
    EXPECT_EQ(result.is_valid, false);
  }

  {
    UriParser uri_parser;
    UriParser::Result result = uri_parser.ParseTarget("foo://bar:42424");
    EXPECT_EQ(result.is_valid, true);
    EXPECT_EQ(result.host, "bar");
    EXPECT_EQ(result.path, "/");
  }

  {
    UriParser uri_parser;
    UriParser::Result result = uri_parser.ParseTarget("foo://bar:42a");
    EXPECT_EQ(result.is_valid, false);
  }

  {
    UriParser uri_parser;
    UriParser::Result result = uri_parser.ParseTarget("foo://ba:/abba?bba");
    EXPECT_EQ(result.is_valid, true);
    EXPECT_EQ(result.host, "ba");
    EXPECT_EQ(result.path, "/abba");
    EXPECT_EQ(result.query, "?bba");
  }

  {
    UriParser uri_parser;
    UriParser::Result result = uri_parser.ParseTarget("f^oo://ba:/abba?bba");
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
      if (status >= HttpParser::kComplete) {
        break;
      }
      memset(buffer, 0, BUFFER_SIZE);
    }
    ASSERT_EQ(status, HttpParser::kComplete);
    HttpParser::Result& result = parser.get_result();
    EXPECT_EQ(result.request.req.method, DELETE);
    EXPECT_EQ(result.request.req.version, HttpParser::kHttp1_1);
    EXPECT_EQ(result.request.req.path, "/");
    close(fd);
  }

  // 501 Not Implemented
  TestParseError(PARSER_PATH_PREFIX "f_00.txt", HttpParser::kComplete, 501);

  // max 03 - request line 길이 초과 (414)
  TestParseError(GOINFRE_PATH "max_rl_03.txt", HttpParser::kRLLenErr, 414);

  // f 01 너무 긴 대문자 method (501)
  TestParseError(PARSER_PATH_PREFIX "f_01.txt", HttpParser::kComplete, 501);

  // f 02 소문자 method (400)
  TestParseError(PARSER_PATH_PREFIX "f_02.txt", HttpParser::kClose, 400);

  // f 03 & 04 유효하지 않은 토큰 개수 (400)
  TestParseError(PARSER_PATH_PREFIX "f_03.txt", HttpParser::kClose, 400);
  TestParseError(PARSER_PATH_PREFIX "f_04.txt", HttpParser::kClose, 400);

  // request target 에 유효하지 않은 문자 (400)
  TestParseError(PARSER_PATH_PREFIX "f_05.txt", HttpParser::kClose, 400);

  // s 00 - valid HTTP/1.1 DELETE request
  {
    HttpParser parser;
    int fd = open(PARSER_PATH_PREFIX "s_00.txt", O_RDONLY);

    memset(buffer, 0, BUFFER_SIZE);
    int status;
    while (read(fd, buffer, BUFFER_SIZE - 1) > 0) {
      status = parser.Parse(buffer);
      if (status >= HttpParser::kComplete) {
        break;
      }
      memset(buffer, 0, BUFFER_SIZE);
    }
    ASSERT_EQ(status, HttpParser::kComplete);
    HttpParser::Result& result = parser.get_result();
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
      if (status == HttpParser::kComplete) {
        break;
      }
      memset(buffer, 0, BUFFER_SIZE);
    }
    ASSERT_EQ(status, HttpParser::kComplete);
    HttpParser::Result& result = parser.get_result();
    EXPECT_EQ(result.request.req.method, DELETE);
    EXPECT_EQ(result.request.req.version, HttpParser::kHttp1_1);
    EXPECT_EQ(result.request.req.path, "/42");
    close(fd);
  }

  // f 06 & 07 - invalid HTTP/1.1 DELETE request
  TestParseError(PARSER_PATH_PREFIX "f_06.txt", HttpParser::kClose, 400);
  TestParseError(PARSER_PATH_PREFIX "f_07.txt", HttpParser::kClose, 400);

  // s 02 - valid HTTP/1.1 GET request
  {
    HttpParser parser;
    int fd = open(PARSER_PATH_PREFIX "s_02.txt", O_RDONLY);

    memset(buffer, 0, BUFFER_SIZE);
    int status;
    while (read(fd, buffer, BUFFER_SIZE - 1) > 0) {
      status = parser.Parse(buffer);
      if (status >= HttpParser::kComplete) {
        break;
      }
      memset(buffer, 0, BUFFER_SIZE);
    }
    ASSERT_EQ(status, HttpParser::kComplete);
    HttpParser::Result& result = parser.get_result();
    EXPECT_EQ(result.request.req.method, GET);
    EXPECT_EQ(result.request.req.version, HttpParser::kHttp1_1);
    EXPECT_EQ(result.request.req.host, "naver.com");
    EXPECT_EQ(result.request.req.path, "/himynameis");
    EXPECT_EQ(result.request.req.query, "?daniel");
    close(fd);
  }

  // f 08 - invalid HTTP version request
  TestParseError(PARSER_PATH_PREFIX "f_08.txt", HttpParser::kComplete, 505);

  // s 03 - case insensitive host
  {
    HttpParser parser;
    int fd = open(PARSER_PATH_PREFIX "s_03.txt", O_RDONLY);

    memset(buffer, 0, BUFFER_SIZE);
    int status;
    while (read(fd, buffer, BUFFER_SIZE - 1) > 0) {
      status = parser.Parse(buffer);
      if (status >= HttpParser::kComplete) {
        break;
      }
      memset(buffer, 0, BUFFER_SIZE);
    }
    ASSERT_EQ(status, HttpParser::kComplete);
    HttpParser::Result& result = parser.get_result();
    EXPECT_EQ(result.request.req.method, GET);
    EXPECT_EQ(result.request.req.version, HttpParser::kHttp1_1);
    EXPECT_EQ(result.request.req.host, "ghan");
    EXPECT_EQ(result.request.req.path, "/Blah");
    close(fd);
  }
}

TEST(HttpParserTest, ParseHeaderFields) {
  // max_hf 00 - header line 길이 초과 400
  TestParseError(GOINFRE_PATH "max_hf_00.txt", HttpParser::kHDLenErr, 400);

  // f 09 - header field no CRLF CRLF 400
  // TestParseError(PARSER_PATH_PREFIX "f_09.txt", HttpParser::kHDLenErr, 400);
  // NOTE: socket timeout으로 해결할 문제로 나중에 처리.

  // f 10 - header name invalid character 400
  TestParseError(PARSER_PATH_PREFIX "f_10.txt", HttpParser::kClose, 400);

  // f 11 - header field name too long 400
  TestParseError(PARSER_PATH_PREFIX "f_11.txt", HttpParser::kClose, 400);

  char buffer[BUFFER_SIZE];
  // s 04 - singleton header field
  {
    HttpParser parser;
    int fd = open(PARSER_PATH_PREFIX "s_04.txt", O_RDONLY);

    memset(buffer, 0, BUFFER_SIZE);
    int status;
    while (read(fd, buffer, BUFFER_SIZE - 1) > 0) {
      status = parser.Parse(buffer);
      if (status >= HttpParser::kComplete) {
        break;
      }
      memset(buffer, 0, BUFFER_SIZE);
    }

    ASSERT_EQ(status, HttpParser::kClose);
    HttpParser::Result& result = parser.get_result();
    EXPECT_EQ(result.status, 200);
    EXPECT_EQ(result.request.req.method, GET);
    EXPECT_EQ(result.request.req.version, HttpParser::kHttp1_0);
    EXPECT_EQ(result.request.req.path, "/");
    EXPECT_EQ(result.request.header.size(), 1);
    EXPECT_EQ(result.request.header.count("accept"), 1);
    EXPECT_EQ(result.request.header["accept"].front(), "ghan");

    close(fd);
  }

  // f 12 - header field value list too long 400
  TestParseError(PARSER_PATH_PREFIX "f_12.txt", HttpParser::kClose, 400);

  // s 06 multiple header field
  {
    HttpParser parser;
    int fd = open(PARSER_PATH_PREFIX "s_06.txt", O_RDONLY);

    memset(buffer, 0, BUFFER_SIZE);
    int status;
    while (read(fd, buffer, BUFFER_SIZE - 1) > 0) {
      status = parser.Parse(buffer);
      if (status >= HttpParser::kComplete) {
        break;
      }
      memset(buffer, 0, BUFFER_SIZE);
    }

    ASSERT_EQ(status, HttpParser::kComplete);
    HttpParser::Result& result = parser.get_result();
    EXPECT_EQ(result.status, 200);
    EXPECT_EQ(result.request.req.method, GET);
    EXPECT_EQ(result.request.req.version, HttpParser::kHttp1_1);
    EXPECT_EQ(result.request.req.path, "/");

    EXPECT_EQ(result.request.header.size(), 2);
    EXPECT_EQ(result.request.header.count("accept-encoding"), 1);
    EXPECT_EQ(result.request.header["accept-encoding"].front(),
              "gzip, deflate, br");

    close(fd);
  }

  // s 07 so many SP / HTAB in header value
  {
    HttpParser parser;
    int fd = open(PARSER_PATH_PREFIX "s_07.txt", O_RDONLY);

    memset(buffer, 0, BUFFER_SIZE);
    int status;
    while (read(fd, buffer, BUFFER_SIZE - 1) > 0) {
      status = parser.Parse(buffer);
      if (status >= HttpParser::kComplete) {
        break;
      }
      memset(buffer, 0, BUFFER_SIZE);
    }

    ASSERT_EQ(status, HttpParser::kComplete);
    HttpParser::Result& result = parser.get_result();
    EXPECT_EQ(result.status, 200);
    EXPECT_EQ(result.request.req.method, GET);
    EXPECT_EQ(result.request.req.version, HttpParser::kHttp1_1);
    EXPECT_EQ(result.request.req.path, "/");

    EXPECT_EQ(result.request.header.size(), 2);
    EXPECT_EQ(result.request.header.count("accept-encoding"), 1);
    EXPECT_EQ(result.request.header["accept-encoding"].front(),
              "gzip, deflate,\t br");

    close(fd);
  }

  // s 08 duplicated field-name
  {
    HttpParser parser;
    int fd = open(PARSER_PATH_PREFIX "s_08.txt", O_RDONLY);

    memset(buffer, 0, BUFFER_SIZE);
    int status;
    while (read(fd, buffer, BUFFER_SIZE - 1) > 0) {
      status = parser.Parse(buffer);
      if (status >= HttpParser::kComplete) {
        break;
      }
      memset(buffer, 0, BUFFER_SIZE);
    }

    ASSERT_EQ(status, HttpParser::kComplete);
    HttpParser::Result& result = parser.get_result();
    EXPECT_EQ(result.status, 200);
    EXPECT_EQ(result.request.req.method, GET);
    EXPECT_EQ(result.request.req.version, HttpParser::kHttp1_1);
    EXPECT_EQ(result.request.req.path, "/");

    EXPECT_EQ(result.request.header.size(), 2);
    EXPECT_EQ(result.request.header.count("accept"), 1);
    EXPECT_EQ(result.request.header["accept"].size(), 2);
    EXPECT_EQ(result.request.header["accept"].front(), "ghan");
    EXPECT_EQ(result.request.header["accept"].back(), "jiskim");

    close(fd);
  }

  // f 13 - invalid character in header field value 400
  TestParseError(PARSER_PATH_PREFIX "f_13.txt", HttpParser::kClose, 400);

  // s 09 valid Host header field
  {
    HttpParser parser;
    int fd = open(PARSER_PATH_PREFIX "s_09.txt", O_RDONLY);

    memset(buffer, 0, BUFFER_SIZE);
    int status;
    while (read(fd, buffer, BUFFER_SIZE - 1) > 0) {
      status = parser.Parse(buffer);
      if (status >= HttpParser::kComplete) {
        break;
      }
      memset(buffer, 0, BUFFER_SIZE);
    }

    ASSERT_EQ(status, HttpParser::kComplete);
    HttpParser::Result& result = parser.get_result();
    EXPECT_EQ(result.status, 200);
    EXPECT_EQ(result.request.req.method, GET);
    EXPECT_EQ(result.request.req.version, HttpParser::kHttp1_1);
    EXPECT_EQ(result.request.req.path, "/");
    EXPECT_EQ(result.request.req.host, "jiskim");

    EXPECT_EQ(result.request.header.size(), 1);
    EXPECT_EQ(result.request.header.count("host"), 1);
    EXPECT_EQ(result.request.header["host"].front(), "jiskim");

    close(fd);
  }

  // f 14 - invalid Host header field value 400
  TestParseError(PARSER_PATH_PREFIX "f_14.txt", HttpParser::kClose, 400);

  // s 10 valid Host header field hex decoding
  {
    HttpParser parser;
    int fd = open(PARSER_PATH_PREFIX "s_10.txt", O_RDONLY);

    memset(buffer, 0, BUFFER_SIZE);
    int status;
    while (read(fd, buffer, BUFFER_SIZE - 1) > 0) {
      status = parser.Parse(buffer);
      if (status >= HttpParser::kComplete) {
        break;
      }
      memset(buffer, 0, BUFFER_SIZE);
    }

    ASSERT_EQ(status, HttpParser::kComplete);
    HttpParser::Result& result = parser.get_result();
    EXPECT_EQ(result.status, 200);
    EXPECT_EQ(result.request.req.method, GET);
    EXPECT_EQ(result.request.req.version, HttpParser::kHttp1_1);
    EXPECT_EQ(result.request.req.path, "/");
    EXPECT_EQ(result.request.req.host, "jiskim");

    EXPECT_EQ(result.request.header.size(), 1);
    EXPECT_EQ(result.request.header.count("host"), 1);
    EXPECT_EQ(result.request.header["host"].front(), "jiskim");

    close(fd);
  }

  // f 15 content-length field value invalid
  TestParseError(PARSER_PATH_PREFIX "f_15.txt", HttpParser::kClose, 400);

  // s 11
  {
    HttpParser parser;
    int fd = open(PARSER_PATH_PREFIX "s_11.txt", O_RDONLY);

    memset(buffer, 0, BUFFER_SIZE);
    int status;
    while (read(fd, buffer, BUFFER_SIZE - 1) > 0) {
      status = parser.Parse(buffer);
      if (status >= HttpParser::kComplete) {
        break;
      }
      memset(buffer, 0, BUFFER_SIZE);
    }

    ASSERT_EQ(status, HttpParser::kComplete);
    HttpParser::Result& result = parser.get_result();
    EXPECT_EQ(result.status, 200);
    EXPECT_EQ(result.request.req.method, POST);
    EXPECT_EQ(result.request.req.version, HttpParser::kHttp1_1);
    EXPECT_EQ(result.request.req.path, "/");
    EXPECT_EQ(result.request.req.host, "jiskim");

    EXPECT_EQ(result.request.header.size(), 2);
    EXPECT_EQ(result.request.header.count("host"), 1);
    EXPECT_EQ(result.request.header["host"].front(), "jiskim");

    EXPECT_EQ(result.request.header.count("content-length"), 1);
    EXPECT_EQ(result.request.header["content-length"].front(), "42");

    close(fd);
  }

  // f 16 content-length field value invalid
  TestParseError(PARSER_PATH_PREFIX "f_16.txt", HttpParser::kClose, 400);

  // f 17 content-length too large
  TestParseError(PARSER_PATH_PREFIX "f_17.txt", HttpParser::kComplete, 413);

  // f 18 content-length overflow
  TestParseError(PARSER_PATH_PREFIX "f_18.txt", HttpParser::kComplete, 413);

  // f 19 http 1.1 with no host field
  TestParseError(PARSER_PATH_PREFIX "f_19.txt", HttpParser::kClose, 400);

  // f 20 http 1.1 both Content-Length and Transfer-Encoding
  TestParseError(PARSER_PATH_PREFIX "f_20.txt", HttpParser::kClose, 400);

  // s 12 valid transfer-encoding
  {
    HttpParser parser;
    int fd = open(PARSER_PATH_PREFIX "s_12.txt", O_RDONLY);

    memset(buffer, 0, BUFFER_SIZE);
    int status;
    while (read(fd, buffer, BUFFER_SIZE - 1) > 0) {
      status = parser.Parse(buffer);
      if (status >= HttpParser::kComplete) {
        break;
      }
      memset(buffer, 0, BUFFER_SIZE);
    }

    ASSERT_EQ(status, HttpParser::kComplete);
    HttpParser::Result& result = parser.get_result();
    EXPECT_EQ(result.status, 200);
    EXPECT_EQ(result.request.req.method, POST);
    EXPECT_EQ(result.request.req.version, HttpParser::kHttp1_1);
    EXPECT_EQ(result.request.req.path, "/");
    EXPECT_EQ(result.request.req.host, "ghan");

    EXPECT_EQ(result.request.header.size(), 2);
    EXPECT_EQ(result.request.header.count("host"), 1);
    EXPECT_EQ(result.request.header["host"].front(), "ghan");

    EXPECT_EQ(result.request.header.count("transfer-encoding"), 1);
    EXPECT_EQ(result.request.header["transfer-encoding"].front(), "chunked");

    close(fd);
  }

  // s 13 valid transfer-encoding, multiple encodings
  {
    HttpParser parser;
    int fd = open(PARSER_PATH_PREFIX "s_13.txt", O_RDONLY);

    memset(buffer, 0, BUFFER_SIZE);
    int status;
    while (read(fd, buffer, BUFFER_SIZE - 1) > 0) {
      status = parser.Parse(buffer);
      if (status >= HttpParser::kComplete) {
        break;
      }
      memset(buffer, 0, BUFFER_SIZE);
    }

    ASSERT_EQ(status, HttpParser::kComplete);
    HttpParser::Result& result = parser.get_result();
    EXPECT_EQ(result.status, 200);
    EXPECT_EQ(result.request.req.method, POST);
    EXPECT_EQ(result.request.req.version, HttpParser::kHttp1_1);
    EXPECT_EQ(result.request.req.path, "/");
    EXPECT_EQ(result.request.req.host, "ghan");

    EXPECT_EQ(result.request.header.size(), 2);
    EXPECT_EQ(result.request.header.count("host"), 1);
    EXPECT_EQ(result.request.header["host"].front(), "ghan");

    EXPECT_EQ(result.request.header.count("transfer-encoding"), 1);
    std::list<std::string> encodings =
        result.request.header["transfer-encoding"];
    EXPECT_EQ(encodings.size(), 3);
    std::list<std::string>::iterator it = encodings.begin();
    EXPECT_EQ(*it, "gzip");
    EXPECT_EQ(*(++it), "compress");
    EXPECT_EQ(*(++it), "chunked");

    close(fd);
  }

  // f 21 invalid transfer-encoding
  TestParseError(PARSER_PATH_PREFIX "f_21.txt", HttpParser::kClose, 400);

  // f 22 invalid transfer-encoding ( quote so bad)
  TestParseError(PARSER_PATH_PREFIX "f_22.txt", HttpParser::kClose, 400);

  // f 23 invalid transfer-encoding ( not in transfer coding registry )
  TestParseError(PARSER_PATH_PREFIX "f_23.txt", HttpParser::kClose, 501);

  // f 24 invalid transfer-encoding ( repeated coding )
  TestParseError(PARSER_PATH_PREFIX "f_24.txt", HttpParser::kClose, 400);

  // f 25 invalid transfer-encoding ( empty transfer-encoding )
  TestParseError(PARSER_PATH_PREFIX "f_25.txt", HttpParser::kClose, 400);

  // f 26 invalid transfer-encoding ( with parameter )
  TestParseError(PARSER_PATH_PREFIX "f_26.txt", HttpParser::kClose, 501);

  // f 27 http1.0 with transfer-encoding (no content-length)
  TestParseError(PARSER_PATH_PREFIX "f_27.txt", HttpParser::kComplete, 411);

  // f 28 && 29 transfer-encoding (quotes)
  TestParseError(PARSER_PATH_PREFIX "f_28.txt", HttpParser::kClose, 501);
  TestParseError(PARSER_PATH_PREFIX "f_29.txt", HttpParser::kClose, 501);

  // s 14 connection value case insensitive
  {
    HttpParser parser;
    int fd = open(PARSER_PATH_PREFIX "s_14.txt", O_RDONLY);

    memset(buffer, 0, BUFFER_SIZE);
    int status;
    while (read(fd, buffer, BUFFER_SIZE - 1) > 0) {
      status = parser.Parse(buffer);
      if (status >= HttpParser::kComplete) {
        break;
      }
      memset(buffer, 0, BUFFER_SIZE);
    }

    ASSERT_EQ(status, HttpParser::kComplete);
    HttpParser::Result& result = parser.get_result();
    EXPECT_EQ(result.status, 200);
    EXPECT_EQ(result.request.req.method, GET);
    EXPECT_EQ(result.request.req.version, HttpParser::kHttp1_1);
    EXPECT_EQ(result.request.header.size(), 2);

    EXPECT_EQ(result.request.header.count("connection"), 1);
    std::list<std::string> connections = result.request.header["connection"];
    EXPECT_EQ(connections.size(), 1);
    std::list<std::string>::iterator it = connections.begin();
    EXPECT_EQ(*it, "keep-alive");

    close(fd);
  }

  // s 15 many connection field values
  {
    HttpParser parser;
    int fd = open(PARSER_PATH_PREFIX "s_15.txt", O_RDONLY);

    memset(buffer, 0, BUFFER_SIZE);
    int status;
    while (read(fd, buffer, BUFFER_SIZE - 1) > 0) {
      status = parser.Parse(buffer);
      if (status >= HttpParser::kComplete) {
        break;
      }
      memset(buffer, 0, BUFFER_SIZE);
    }

    ASSERT_EQ(status, HttpParser::kClose);
    HttpParser::Result& result = parser.get_result();
    EXPECT_EQ(result.status, 200);
    EXPECT_EQ(result.request.req.method, GET);
    EXPECT_EQ(result.request.req.version, HttpParser::kHttp1_1);
    EXPECT_EQ(result.request.header.size(), 2);

    EXPECT_EQ(result.request.header.count("connection"), 1);
    std::list<std::string> connections = result.request.header["connection"];
    ASSERT_EQ(connections.size(), 3);
    std::list<std::string>::iterator it = connections.begin();
    EXPECT_EQ(*it, "keep-alive");
    EXPECT_EQ(*(++it), "close");
    EXPECT_EQ(*(++it), "host");

    close(fd);
  }

  // f 30 repeated value in connection field
  TestParseError(PARSER_PATH_PREFIX "f_30.txt", HttpParser::kClose, 400);

  // f 31 invalid value in connection field
  TestParseError(PARSER_PATH_PREFIX "f_31.txt", HttpParser::kClose, 400);

  // s 16 is close? HTTP/1.1
  {
    HttpParser parser;
    int fd = open(PARSER_PATH_PREFIX "s_16.txt", O_RDONLY);

    memset(buffer, 0, BUFFER_SIZE);
    int status;
    while (read(fd, buffer, BUFFER_SIZE - 1) > 0) {
      status = parser.Parse(buffer);
      if (status >= HttpParser::kComplete) {
        break;
      }
      memset(buffer, 0, BUFFER_SIZE);
    }

    ASSERT_EQ(status, HttpParser::kClose);
    HttpParser::Result& result = parser.get_result();
    EXPECT_EQ(result.status, 200);
    EXPECT_EQ(result.request.req.method, GET);
    EXPECT_EQ(result.request.req.version, HttpParser::kHttp1_1);
    EXPECT_EQ(result.request.header.size(), 2);

    EXPECT_EQ(result.request.header.count("connection"), 1);
    std::list<std::string> connections = result.request.header["connection"];
    ASSERT_EQ(connections.size(), 1);
    std::list<std::string>::iterator it = connections.begin();
    EXPECT_EQ(*it, "close");

    close(fd);
  }

  // s 17 is keep-alive? HTTP/1.0
  {
    HttpParser parser;
    int fd = open(PARSER_PATH_PREFIX "s_17.txt", O_RDONLY);

    memset(buffer, 0, BUFFER_SIZE);
    int status;
    while (read(fd, buffer, BUFFER_SIZE - 1) > 0) {
      status = parser.Parse(buffer);
      if (status >= HttpParser::kComplete) {
        break;
      }
      memset(buffer, 0, BUFFER_SIZE);
    }

    ASSERT_EQ(status, HttpParser::kComplete);
    HttpParser::Result& result = parser.get_result();
    EXPECT_EQ(result.status, 200);
    EXPECT_EQ(result.request.req.method, GET);
    EXPECT_EQ(result.request.req.version, HttpParser::kHttp1_0);
    EXPECT_EQ(result.request.header.size(), 1);

    EXPECT_EQ(result.request.header.count("connection"), 1);
    std::list<std::string> connections = result.request.header["connection"];
    ASSERT_EQ(connections.size(), 1);
    std::list<std::string>::iterator it = connections.begin();
    EXPECT_EQ(*it, "keep-alive");

    close(fd);
  }

  // s 18 is keep-alive? HTTP/1.0
  {
    HttpParser parser;
    int fd = open(PARSER_PATH_PREFIX "s_18.txt", O_RDONLY);

    memset(buffer, 0, BUFFER_SIZE);
    int status;
    while (read(fd, buffer, BUFFER_SIZE - 1) > 0) {
      status = parser.Parse(buffer);
      if (status >= HttpParser::kComplete) {
        break;
      }
      memset(buffer, 0, BUFFER_SIZE);
    }

    ASSERT_EQ(status, HttpParser::kComplete);
    HttpParser::Result& result = parser.get_result();
    EXPECT_EQ(result.status, 200);
    EXPECT_EQ(result.request.req.method, GET);
    EXPECT_EQ(result.request.req.version, HttpParser::kHttp1_0);
    EXPECT_EQ(result.request.header.size(), 2);

    EXPECT_EQ(result.request.header.count("connection"), 1);
    std::list<std::string> connections = result.request.header["connection"];
    ASSERT_EQ(connections.size(), 1);
    std::list<std::string>::iterator it = connections.begin();
    EXPECT_EQ(*it, "keep-alive");

    EXPECT_EQ(result.request.header.count("keep-alive"), 1);
    std::list<std::string> keep_alive = result.request.header["keep-alive"];
    ASSERT_EQ(keep_alive.size(), 1);
    it = keep_alive.begin();
    EXPECT_EQ(*it, "timeout=5, max=1000");

    close(fd);
  }

  // f 32 trailers (501)
  TestParseError(PARSER_PATH_PREFIX "f_32.txt", HttpParser::kClose, 501);
}
