#include <gtest/gtest.h>

#include <fstream>
#include <iostream>
#include <sstream>

#include "Connection.hpp"
#include "HttpParser.hpp"
#include "Types.hpp"
#include "Validator.hpp"

#define PARSER_PATH_PREFIX "../tests/HttpParser/"
#define GOINFRE_PATH "/Users/danielgyoungminhan/goinfre/"

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
  char buffer[BUFFER_SIZE + 1];

  int fd = open(file_path.c_str(), O_RDONLY);

  memset(buffer, 0, BUFFER_SIZE + 1);
  int status;
  while (read(fd, buffer, BUFFER_SIZE) > 0) {
    std::string buf_str(buffer);
    status = parser.Parse(buf_str);
    if (status >= HttpParser::kComplete) {
      break;
    }
    memset(buffer, 0, BUFFER_SIZE + 1);
  }
  EXPECT_EQ(status, parser_status) << file_path << "\n";
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
              "?where=nexearch&sm=top_hty&fbm=1&ie=utf8&query=%EC%9D%B4%\
EA%B2%83%EC%9D%80+%EA%B2%80%EC%83%89+%EC%BF%BC%EB%A6%AC%EB%AC%B8+%EC%\
9E%85%EB%8B%88%EB%8B%A4");
  }

  // Absolute Form URI
  {
    UriParser uri_parser;
    UriParser::Result result = uri_parser.ParseTarget(
        "http://naver.com/"
        "search.%6eaver?where=nexearch&sm=top_hty&fbm=1&ie=utf8&query=%EC%9D%"
        "B4%"
        "EA%B2%83%EC%9D%80+%EA%B2%80%EC%83%89+%EC%BF%BC%EB%A6%AC%EB%AC%B8+%EC%"
        "9E%85%EB%8B%88%EB%8B%A4");
    EXPECT_EQ(result.is_valid, true);
    EXPECT_EQ(result.host, "naver.com");
    EXPECT_EQ(result.path, "/search.naver");
    EXPECT_EQ(result.query,
              "?where=nexearch&sm=top_hty&fbm=1&ie=utf8&query=%EC%9D%B4%\
EA%B2%83%EC%9D%80+%EA%B2%80%EC%83%89+%EC%BF%BC%EB%A6%AC%EB%AC%B8+%EC%\
9E%85%EB%8B%88%EB%8B%A4");
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
              "?where=nexearch&sm=top_hty&fbm=1&ie=utf8&query=%EC%9D%B4%\
EA%B2%83%EC%9D%80+%EA%B2%80%EC%83%89+%EC%BF%BC%EB%A6%AC%EB%AC%B8+%EC%\
9E%85%EB%8B%88%EB%8B%A4");
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
    UriParser::Result result = uri_parser.ParseTarget(
        "http://naver.com:42424/"
        "search.naver?where=nexearch&sm=top_hty&fbm=1&ie=utf8&query=%EC%9D%B4%"
        "EA%B2%83%EC%9D%80+%EA%B2%80%EC%83%89+%EC%BF%BC%EB%A6%AC%EB%AC%B8+%EC%"
        "9E%85%EB%8B%88%EB%8B%A");
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
  char buffer[BUFFER_SIZE + 1];
  {
    // valid HTTP/1.1 DELETE request
    HttpParser parser;
    int fd = open(PARSER_PATH_PREFIX "s_00.txt", O_RDONLY);

    memset(buffer, 0, BUFFER_SIZE + 1);
    int status;
    while (read(fd, buffer, BUFFER_SIZE) > 0) {
      std::string buf_str(buffer);
      status = parser.Parse(buf_str);
      if (status >= HttpParser::kComplete) {
        break;
      }
      memset(buffer, 0, BUFFER_SIZE + 1);
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
  std::cout << "s 00 - valid HTTP/1.1 DELETE request\n";
  {
    HttpParser parser;
    int fd = open(PARSER_PATH_PREFIX "s_00.txt", O_RDONLY);

    memset(buffer, 0, BUFFER_SIZE + 1);
    int status;
    while (read(fd, buffer, BUFFER_SIZE) > 0) {
      std::string buf_str(buffer);
      status = parser.Parse(buf_str);
      if (status >= HttpParser::kComplete) {
        break;
      }
      memset(buffer, 0, BUFFER_SIZE + 1);
    }
    ASSERT_EQ(status, HttpParser::kComplete);
    HttpParser::Result& result = parser.get_result();
    EXPECT_EQ(result.request.req.method, DELETE);
    EXPECT_EQ(result.request.req.version, HttpParser::kHttp1_1);
    EXPECT_EQ(result.request.req.path, "/");
    close(fd);
  }

  // s 01 - valid HTTP/1.1 DELETE request
  std::cout << "s 01 - valid HTTP/1.1 DELETE request\n";
  {
    HttpParser parser;
    int fd = open(PARSER_PATH_PREFIX "s_01.txt", O_RDONLY);

    memset(buffer, 0, BUFFER_SIZE + 1);
    int status;
    while (read(fd, buffer, BUFFER_SIZE) > 0) {
      std::string buf_str(buffer);
      status = parser.Parse(buf_str);
      if (status == HttpParser::kComplete) {
        break;
      }
      memset(buffer, 0, BUFFER_SIZE + 1);
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
  std::cout << "s 02 - valid HTTP/1.1 GET request\n";
  {
    HttpParser parser;
    int fd = open(PARSER_PATH_PREFIX "s_02.txt", O_RDONLY);

    memset(buffer, 0, BUFFER_SIZE + 1);
    int status;
    while (read(fd, buffer, BUFFER_SIZE) > 0) {
      std::string buf_str(buffer);
      status = parser.Parse(buf_str);
      if (status >= HttpParser::kComplete) {
        break;
      }
      memset(buffer, 0, BUFFER_SIZE + 1);
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
  TestParseError(PARSER_PATH_PREFIX "f_08.txt", HttpParser::kClose, 505);

  // s 03 - case insensitive host
  std::cout << "s 03 - case insensitive host\n";
  {
    HttpParser parser;
    int fd = open(PARSER_PATH_PREFIX "s_03.txt", O_RDONLY);

    memset(buffer, 0, BUFFER_SIZE + 1);
    int status;
    while (read(fd, buffer, BUFFER_SIZE) > 0) {
      std::string buf_str(buffer);
      status = parser.Parse(buf_str);
      if (status >= HttpParser::kComplete) {
        break;
      }
      memset(buffer, 0, BUFFER_SIZE + 1);
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

  char buffer[BUFFER_SIZE + 1];
  // s 04 - singleton header field
  std::cout << "s 04 - singleton header field\n";
  {
    HttpParser parser;
    int fd = open(PARSER_PATH_PREFIX "s_04.txt", O_RDONLY);

    memset(buffer, 0, BUFFER_SIZE + 1);
    int status;
    while (read(fd, buffer, BUFFER_SIZE) > 0) {
      std::string buf_str(buffer);
      status = parser.Parse(buf_str);
      if (status >= HttpParser::kComplete) {
        break;
      }
      memset(buffer, 0, BUFFER_SIZE + 1);
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
  std::cout << "s 06 multiple header field\n";
  {
    HttpParser parser;
    int fd = open(PARSER_PATH_PREFIX "s_06.txt", O_RDONLY);

    memset(buffer, 0, BUFFER_SIZE + 1);
    int status;
    while (read(fd, buffer, BUFFER_SIZE) > 0) {
      std::string buf_str(buffer);
      status = parser.Parse(buf_str);
      if (status >= HttpParser::kComplete) {
        break;
      }
      memset(buffer, 0, BUFFER_SIZE + 1);
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
  std::cout << "s 07 so many SP / HTAB in header value\n";
  {
    HttpParser parser;
    int fd = open(PARSER_PATH_PREFIX "s_07.txt", O_RDONLY);

    memset(buffer, 0, BUFFER_SIZE + 1);
    int status;
    while (read(fd, buffer, BUFFER_SIZE) > 0) {
      std::string buf_str(buffer);
      status = parser.Parse(buf_str);
      if (status >= HttpParser::kComplete) {
        break;
      }
      memset(buffer, 0, BUFFER_SIZE + 1);
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
  std::cout << "s 08 duplicated field-name\n";
  {
    HttpParser parser;
    int fd = open(PARSER_PATH_PREFIX "s_08.txt", O_RDONLY);

    memset(buffer, 0, BUFFER_SIZE + 1);
    int status;
    while (read(fd, buffer, BUFFER_SIZE) > 0) {
      std::string buf_str(buffer);
      status = parser.Parse(buf_str);
      if (status >= HttpParser::kComplete) {
        break;
      }
      memset(buffer, 0, BUFFER_SIZE + 1);
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
  std::cout << "s 09 valid Host header field\n";
  {
    HttpParser parser;
    int fd = open(PARSER_PATH_PREFIX "s_09.txt", O_RDONLY);

    memset(buffer, 0, BUFFER_SIZE + 1);
    int status;
    while (read(fd, buffer, BUFFER_SIZE) > 0) {
      std::string buf_str(buffer);
      status = parser.Parse(buf_str);
      if (status >= HttpParser::kComplete) {
        break;
      }
      memset(buffer, 0, BUFFER_SIZE + 1);
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
  std::cout << "s 10 valid Host header field hex decoding\n";
  {
    HttpParser parser;
    int fd = open(PARSER_PATH_PREFIX "s_10.txt", O_RDONLY);

    memset(buffer, 0, BUFFER_SIZE + 1);
    int status;
    while (read(fd, buffer, BUFFER_SIZE) > 0) {
      std::string buf_str(buffer);
      status = parser.Parse(buf_str);
      if (status >= HttpParser::kComplete) {
        break;
      }
      memset(buffer, 0, BUFFER_SIZE + 1);
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

  // s 11 body length = content-length
  std::cout << "s 11 body length = content-length\n";
  {
    HttpParser parser;
    int fd = open(PARSER_PATH_PREFIX "s_11.txt", O_RDONLY);

    memset(buffer, 0, BUFFER_SIZE + 1);
    int status;
    while (read(fd, buffer, BUFFER_SIZE) > 0) {
      std::string buf_str(buffer);
      status = parser.Parse(buf_str);
      if (status >= HttpParser::kComplete) {
        break;
      }
      memset(buffer, 0, BUFFER_SIZE + 1);
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
  std::cout << "s 12 valid transfer-encoding\n";
  {
    HttpParser parser;
    int fd = open(PARSER_PATH_PREFIX "s_12.txt", O_RDONLY);

    memset(buffer, 0, BUFFER_SIZE + 1);
    int status;
    while (read(fd, buffer, BUFFER_SIZE) > 0) {
      std::string buf_str(buffer);
      status = parser.Parse(buf_str);
      if (status >= HttpParser::kComplete) {
        break;
      }
      memset(buffer, 0, BUFFER_SIZE + 1);
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
  std::cout << "s 13 valid transfer-encoding, multiple encodings\n";
  {
    HttpParser parser;
    int fd = open(PARSER_PATH_PREFIX "s_13.txt", O_RDONLY);

    memset(buffer, 0, BUFFER_SIZE + 1);
    int status;
    while (read(fd, buffer, BUFFER_SIZE) > 0) {
      std::string buf_str(buffer);
      status = parser.Parse(buf_str);
      if (status >= HttpParser::kComplete) {
        break;
      }
      memset(buffer, 0, BUFFER_SIZE + 1);
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
  TestParseError(PARSER_PATH_PREFIX "f_27.txt", HttpParser::kClose, 411);

  // f 28 && 29 transfer-encoding (quotes)
  TestParseError(PARSER_PATH_PREFIX "f_28.txt", HttpParser::kClose, 501);
  TestParseError(PARSER_PATH_PREFIX "f_29.txt", HttpParser::kClose, 501);

  // s 14 connection value case insensitive
  std::cout << "s 14 connection value case insensitive\n";
  {
    HttpParser parser;
    int fd = open(PARSER_PATH_PREFIX "s_14.txt", O_RDONLY);

    memset(buffer, 0, BUFFER_SIZE + 1);
    int status;
    while (read(fd, buffer, BUFFER_SIZE) > 0) {
      std::string buf_str(buffer);
      status = parser.Parse(buf_str);
      if (status >= HttpParser::kComplete) {
        break;
      }
      memset(buffer, 0, BUFFER_SIZE + 1);
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
  std::cout << "s 15 many connection field values\n";
  {
    HttpParser parser;
    int fd = open(PARSER_PATH_PREFIX "s_15.txt", O_RDONLY);

    memset(buffer, 0, BUFFER_SIZE + 1);
    int status;
    while (read(fd, buffer, BUFFER_SIZE) > 0) {
      std::string buf_str(buffer);
      status = parser.Parse(buf_str);
      if (status >= HttpParser::kComplete) {
        break;
      }
      memset(buffer, 0, BUFFER_SIZE + 1);
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
  std::cout << "s 16 is close? HTTP/1.1\n";
  {
    HttpParser parser;
    int fd = open(PARSER_PATH_PREFIX "s_16.txt", O_RDONLY);

    memset(buffer, 0, BUFFER_SIZE + 1);
    int status;
    while (read(fd, buffer, BUFFER_SIZE) > 0) {
      std::string buf_str(buffer);
      status = parser.Parse(buf_str);
      if (status >= HttpParser::kComplete) {
        break;
      }
      memset(buffer, 0, BUFFER_SIZE + 1);
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
  std::cout << "s 17 is keep-alive? HTTP/1.0\n";
  {
    HttpParser parser;
    int fd = open(PARSER_PATH_PREFIX "s_17.txt", O_RDONLY);

    memset(buffer, 0, BUFFER_SIZE + 1);
    int status;
    while (read(fd, buffer, BUFFER_SIZE) > 0) {
      std::string buf_str(buffer);
      status = parser.Parse(buf_str);
      if (status >= HttpParser::kComplete) {
        break;
      }
      memset(buffer, 0, BUFFER_SIZE + 1);
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
  std::cout << "s 18 is keep-alive? HTTP/1.0\n";
  {
    HttpParser parser;
    int fd = open(PARSER_PATH_PREFIX "s_18.txt", O_RDONLY);

    memset(buffer, 0, BUFFER_SIZE + 1);
    int status;
    while (read(fd, buffer, BUFFER_SIZE) > 0) {
      std::string buf_str(buffer);
      status = parser.Parse(buf_str);
      if (status >= HttpParser::kComplete) {
        break;
      }
      memset(buffer, 0, BUFFER_SIZE + 1);
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

  // f 33 Http 1.1 with no header
  TestParseError(PARSER_PATH_PREFIX "f_33.txt", HttpParser::kClose, 400);
}

TEST(HttpParserTest, ParseBody) {
  char buffer[BUFFER_SIZE + 1];
  // max_bd content exactly as long as BODY_MAX

  std::cout << "max_bd content exactly as long as BODY_MAX\n";
  {
    HttpParser parser;
    int fd = open(GOINFRE_PATH "max_bd.txt", O_RDONLY);

    memset(buffer, 0, BUFFER_SIZE + 1);
    int status;
    while (read(fd, buffer, BUFFER_SIZE) > 0) {
      std::string buf_str(buffer);
      status = parser.Parse(buf_str);
      if (status >= HttpParser::kComplete) {
        break;
      }
      memset(buffer, 0, BUFFER_SIZE + 1);
    }

    ASSERT_EQ(status, HttpParser::kComplete);
    HttpParser::Result& result = parser.get_result();
    EXPECT_EQ(result.status, 200);
    EXPECT_EQ(result.request.req.method, POST);
    EXPECT_EQ(result.request.req.version, HttpParser::kHttp1_1);
    EXPECT_EQ(result.request.header.size(), 2);

    EXPECT_EQ(result.request.header.count("content-length"), 1);
    std::list<std::string> content_length =
        result.request.header["content-length"];
    ASSERT_EQ(content_length.size(), 1);
    std::list<std::string>::iterator it = content_length.begin();
    EXPECT_EQ(*it, "134217728");

    lseek(fd, 0, SEEK_SET);
    char* body = (char*)calloc(1, 134217787);
    size_t read_size = 134217786;
    if (!body || read_size != read(fd, body, 134227787)) {
      std::cerr << "read byte does not match" << std::endl;
      exit(1);
    }
    EXPECT_EQ(result.request.content.size(), 134217728);
    EXPECT_EQ(std::equal(result.request.content.begin(),
                         result.request.content.end(), body + 58),
              true);
    free(body);
    close(fd);
  }

  // s 19
  std::cout << "s 19\n";
  {
    HttpParser parser;
    int fd = open(PARSER_PATH_PREFIX "s_19.txt", O_RDONLY);

    memset(buffer, 0, BUFFER_SIZE + 1);
    int status;
    while (read(fd, buffer, BUFFER_SIZE) > 0) {
      std::string buf_str(buffer);
      status = parser.Parse(buf_str);
      if (status >= HttpParser::kComplete) {
        break;
      }
      memset(buffer, 0, BUFFER_SIZE + 1);
    }

    ASSERT_EQ(status, HttpParser::kComplete);
    HttpParser::Result& result = parser.get_result();
    EXPECT_EQ(result.status, 200);
    EXPECT_EQ(result.request.req.method, POST);
    EXPECT_EQ(result.request.req.version, HttpParser::kHttp1_1);
    EXPECT_EQ(result.request.header.size(), 2);

    EXPECT_EQ(result.request.header.count("content-length"), 1);
    std::list<std::string> content_length =
        result.request.header["content-length"];
    ASSERT_EQ(content_length.size(), 1);
    std::list<std::string>::iterator it = content_length.begin();
    EXPECT_EQ(*it, "10000");

    lseek(fd, 0, SEEK_SET);
    char* body = (char*)calloc(1, 10055);
    size_t read_size = 10054;
    if (!body || read_size != read(fd, body, 10054)) {
      std::cerr << "read byte does not match" << std::endl;
      exit(1);
    }
    size_t crlfcrlf = std::string(body).find(CRLF CRLF);
    EXPECT_EQ(result.request.content.size(), 10000);
    EXPECT_EQ(result.request.content, std::string(body).substr(crlfcrlf + 4));
    free(body);
    close(fd);
  }

  // s 20 two requests in a row
  std::cout << "s 20 two requests in a row\n";
  {
    HttpParser parser;
    int fd = open(PARSER_PATH_PREFIX "s_20.txt", O_RDONLY);

    memset(buffer, 0, BUFFER_SIZE + 1);
    int status;
    while (read(fd, buffer, BUFFER_SIZE) > 0) {
      std::string buf_str(buffer);
      status = parser.Parse(buf_str);
      if (status >= HttpParser::kComplete) {
        break;
      }
      memset(buffer, 0, BUFFER_SIZE + 1);
    }
    off_t cursor = lseek(fd, 0, SEEK_CUR);

    ASSERT_EQ(status, HttpParser::kComplete);
    HttpParser::Result result = parser.get_result();
    EXPECT_EQ(result.status, 200);
    EXPECT_EQ(result.request.req.method, POST);
    EXPECT_EQ(result.request.req.version, HttpParser::kHttp1_1);
    EXPECT_EQ(result.request.header.size(), 2);

    EXPECT_EQ(result.request.header.count("content-length"), 1);
    std::list<std::string> content_length =
        result.request.header["content-length"];
    ASSERT_EQ(content_length.size(), 1);
    std::list<std::string>::iterator it = content_length.begin();
    EXPECT_EQ(*it, "6");

    EXPECT_EQ(result.request.content.size(), 6);
    EXPECT_EQ(result.request.content, "jiskim");

    // second request
    parser.Clear();
    memset(buffer, 0, BUFFER_SIZE + 1);
    if (cursor == lseek(fd, 0, SEEK_END)) {
      std::string buf_str;
      status = parser.Parse(buf_str);
    } else {
      lseek(fd, cursor, SEEK_SET);
      while (read(fd, buffer, BUFFER_SIZE) > 0) {
        std::string buf_str(buffer);
        status = parser.Parse(buf_str);
        if (status >= HttpParser::kComplete) {
          break;
        }
        memset(buffer, 0, BUFFER_SIZE + 1);
      }
    }

    EXPECT_EQ(status, HttpParser::kClose);
    result = parser.get_result();
    EXPECT_EQ(result.status, 200);
    EXPECT_EQ(result.request.req.method, GET);
    EXPECT_EQ(result.request.req.version, HttpParser::kHttp1_0);
    EXPECT_EQ(result.request.header.size(), 0);

    close(fd);
  }

  // s 21 zero content-length
  std::cout << "s 21 zero content-length\n";
  {
    HttpParser parser;
    int fd = open(PARSER_PATH_PREFIX "s_21.txt", O_RDONLY);

    memset(buffer, 0, BUFFER_SIZE + 1);
    int status;
    while (read(fd, buffer, BUFFER_SIZE) > 0) {
      std::string buf_str(buffer);
      status = parser.Parse(buf_str);
      if (status >= HttpParser::kComplete) {
        break;
      }
      memset(buffer, 0, BUFFER_SIZE + 1);
    }

    ASSERT_EQ(status, HttpParser::kClose);
    HttpParser::Result result = parser.get_result();
    EXPECT_EQ(result.status, 200);
    EXPECT_EQ(result.request.req.method, GET);
    EXPECT_EQ(result.request.req.version, HttpParser::kHttp1_0);
    EXPECT_EQ(result.request.header.size(), 1);

    EXPECT_EQ(result.request.header.count("content-length"), 1);
    std::list<std::string> content_length =
        result.request.header["content-length"];
    ASSERT_EQ(content_length.size(), 1);
    std::list<std::string>::iterator it = content_length.begin();
    EXPECT_EQ(*it, "0");
    EXPECT_EQ(result.request.content.size(), 0);
    close(fd);
  }

  // s 22 4096 world
  std::cout << "s 22 4096 world\n";
  {
    HttpParser parser;
    int fd = open(PARSER_PATH_PREFIX "s_22.txt", O_RDONLY);

    memset(buffer, 0, BUFFER_SIZE + 1);
    int status;
    while (read(fd, buffer, BUFFER_SIZE) > 0) {
      std::string buf_str(buffer);
      status = parser.Parse(buf_str);
      if (status >= HttpParser::kComplete) {
        break;
      }
      memset(buffer, 0, BUFFER_SIZE + 1);
    }

    ASSERT_EQ(status, HttpParser::kComplete);
    HttpParser::Result result = parser.get_result();
    EXPECT_EQ(result.status, 200);
    EXPECT_EQ(result.request.req.method, POST);
    EXPECT_EQ(result.request.req.version, HttpParser::kHttp1_1);
    EXPECT_EQ(result.request.header.size(), 2);

    EXPECT_EQ(result.request.header.count("content-length"), 1);
    std::list<std::string> content_length =
        result.request.header["content-length"];
    ASSERT_EQ(content_length.size(), 1);
    std::list<std::string>::iterator it = content_length.begin();
    EXPECT_EQ(*it, "4096");
    EXPECT_EQ(result.request.content.size(), 4096);

    lseek(fd, 0, SEEK_SET);
    char* body = (char*)calloc(1, 8193);
    size_t read_size = 8192;
    if (!body || read_size != read(fd, body, 8192)) {
      std::cerr << "read byte does not match🧐🧐🧐🧐🧐" << std::endl;
      exit(1);
    }
    size_t crlfcrlf = std::string(body).find(CRLF CRLF);
    EXPECT_EQ(result.request.content.size(), 4096);
    EXPECT_EQ(std::equal(result.request.content.begin(),
                         result.request.content.end(), body + crlfcrlf + 4),
              true);
    free(body);
    close(fd);
  }

  // f 34 looks like ok but...
  std::cout << "f 34 looks like ok but...\n";
  {
    HttpParser parser;
    int fd = open(PARSER_PATH_PREFIX "f_34.txt", O_RDONLY);

    memset(buffer, 0, BUFFER_SIZE + 1);
    int status;
    while (read(fd, buffer, BUFFER_SIZE) > 0) {
      std::string buf_str(buffer);
      status = parser.Parse(buf_str);
      if (status >= HttpParser::kComplete) {
        break;
      }
      memset(buffer, 0, BUFFER_SIZE + 1);
    }
    off_t cursor = lseek(fd, 0, SEEK_CUR);

    ASSERT_EQ(status, HttpParser::kComplete);
    HttpParser::Result result = parser.get_result();
    EXPECT_EQ(result.status, 200);
    EXPECT_EQ(result.request.req.method, POST);
    EXPECT_EQ(result.request.req.version, HttpParser::kHttp1_1);
    EXPECT_EQ(result.request.header.size(), 2);

    EXPECT_EQ(result.request.header.count("content-length"), 1);
    std::list<std::string> content_length =
        result.request.header["content-length"];
    ASSERT_EQ(content_length.size(), 1);
    std::list<std::string>::iterator it = content_length.begin();
    EXPECT_EQ(*it, "4096");
    EXPECT_EQ(result.request.content.size(), 4096);

    lseek(fd, 0, SEEK_SET);
    char* body = (char*)calloc(1, 8193);
    size_t read_size = 8192;
    if (!body || read_size != read(fd, body, 8192)) {
      std::cerr << "read byte does not match🧐🧐🧐🧐🧐" << std::endl;
      exit(1);
    }
    size_t crlfcrlf = std::string(body).find(CRLF CRLF);
    EXPECT_EQ(result.request.content.size(), 4096);
    EXPECT_EQ(std::equal(result.request.content.begin(),
                         result.request.content.end(), body + crlfcrlf + 4),
              true);
    free(body);

    parser.Clear();
    memset(buffer, 0, BUFFER_SIZE + 1);
    if (cursor == lseek(fd, 0, SEEK_END)) {
      std::string buf_str;
      status = parser.Parse(buf_str);
    } else {
      lseek(fd, cursor, SEEK_SET);
      while (read(fd, buffer, BUFFER_SIZE) > 0) {
        std::string buf_str(buffer);
        status = parser.Parse(buf_str);
        if (status >= HttpParser::kComplete) {
          break;
        }
        memset(buffer, 0, BUFFER_SIZE + 1);
      }
    }
    result = parser.get_result();

    EXPECT_EQ(status, HttpParser::kClose);
    EXPECT_EQ(result.status, 400);
    close(fd);
  }

  // s 24 transfer-encoding
  std::cout << "s 24 transfer-encoding\n";
  {
    HttpParser parser;
    int fd = open(PARSER_PATH_PREFIX "s_24.txt", O_RDONLY);

    memset(buffer, 0, BUFFER_SIZE + 1);
    int status;
    while (read(fd, buffer, BUFFER_SIZE) > 0) {
      std::string buf_str(buffer);
      status = parser.Parse(buf_str);
      if (status >= HttpParser::kComplete) {
        break;
      }
      memset(buffer, 0, BUFFER_SIZE + 1);
    }

    EXPECT_EQ(status, HttpParser::kComplete);
    HttpParser::Result result = parser.get_result();
    EXPECT_EQ(result.status, 200);
    EXPECT_EQ(result.request.req.method, POST);
    EXPECT_EQ(result.request.req.version, HttpParser::kHttp1_1);
    EXPECT_EQ(result.request.header.size(), 2);

    EXPECT_EQ(result.request.header.count("transfer-encoding"), 1);
    std::list<std::string> transfer_encoding =
        result.request.header["transfer-encoding"];
    EXPECT_EQ(transfer_encoding.size(), 1);
    std::list<std::string>::iterator it = transfer_encoding.begin();
    EXPECT_EQ(*it, "chunked");
    EXPECT_EQ(result.request.content.size(), 11);
    EXPECT_EQ(result.request.content, "ohioajiskim");

    close(fd);
  }

  // s 25 transfer-encoding : long
  std::cout << "s 25 transfer-encoding : long\n";
  {
    HttpParser parser;
    int fd = open(PARSER_PATH_PREFIX "s_25.txt", O_RDONLY);

    memset(buffer, 0, BUFFER_SIZE + 1);
    int status;
    while (read(fd, buffer, BUFFER_SIZE) > 0) {
      std::string buf_str(buffer);
      status = parser.Parse(buf_str);
      if (status >= HttpParser::kComplete) {
        break;
      }
      memset(buffer, 0, BUFFER_SIZE + 1);
    }

    EXPECT_EQ(status, HttpParser::kComplete);
    HttpParser::Result result = parser.get_result();
    EXPECT_EQ(result.status, 200);
    EXPECT_EQ(result.request.req.method, POST);
    EXPECT_EQ(result.request.req.version, HttpParser::kHttp1_1);
    EXPECT_EQ(result.request.header.size(), 2);

    EXPECT_EQ(result.request.header.count("transfer-encoding"), 1);
    std::list<std::string> transfer_encoding =
        result.request.header["transfer-encoding"];
    EXPECT_EQ(transfer_encoding.size(), 1);
    std::list<std::string>::iterator it = transfer_encoding.begin();
    EXPECT_EQ(*it, "chunked");

    EXPECT_EQ(result.request.content.size(), 12288);
    // EXPECT_EQ(result.request.content, );

    close(fd);
  }

  // f 35 missing a CRLF at the end => timeout!
  // NOTE : timeout으로 해결할 문제
  // TestParseError(PARSER_PATH_PREFIX "f_35.txt", HttpParser::kClose, 400);

  // f 36 invalid character after chunk size == 0
  TestParseError(PARSER_PATH_PREFIX "f_36.txt", HttpParser::kClose, 400);

  // f 37 invalid character before CRLF
  TestParseError(PARSER_PATH_PREFIX "f_37.txt", HttpParser::kClose, 400);

  // f 38 transfer-encoding with http 1.0
  TestParseError(PARSER_PATH_PREFIX "f_38.txt", HttpParser::kClose, 411);

  // s 26 double transfer-encoding request
  std::cout << "s 26 double transfer-encoding request\n";
  {
    HttpParser parser;
    int fd = open(PARSER_PATH_PREFIX "s_26.txt", O_RDONLY);

    memset(buffer, 0, BUFFER_SIZE + 1);
    int status;
    while (read(fd, buffer, BUFFER_SIZE) > 0) {
      std::string buf_str(buffer);
      status = parser.Parse(buf_str);
      if (status >= HttpParser::kComplete) {
        break;
      }
      memset(buffer, 0, BUFFER_SIZE + 1);
    }
    off_t cursor = lseek(fd, 0, SEEK_CUR);

    EXPECT_EQ(status, HttpParser::kComplete);
    HttpParser::Result result = parser.get_result();
    EXPECT_EQ(result.status, 200);
    EXPECT_EQ(result.request.req.method, POST);
    EXPECT_EQ(result.request.req.version, HttpParser::kHttp1_1);
    EXPECT_EQ(result.request.header.size(), 2);

    EXPECT_EQ(result.request.header.count("transfer-encoding"), 1);
    std::list<std::string> transfer_encoding =
        result.request.header["transfer-encoding"];
    EXPECT_EQ(transfer_encoding.size(), 1);
    std::list<std::string>::iterator it = transfer_encoding.begin();
    EXPECT_EQ(*it, "chunked");

    EXPECT_EQ(result.request.content.size(), 9);
    EXPECT_EQ(result.request.content, "ghanghang");

    parser.Clear();
    memset(buffer, 0, BUFFER_SIZE + 1);
    if (cursor == lseek(fd, 0, SEEK_END)) {
      std::string buf_str;
      status = parser.Parse(buf_str);
    } else {
      lseek(fd, cursor, SEEK_SET);
      while (read(fd, buffer, BUFFER_SIZE) > 0) {
        std::string buf_str(buffer);
        status = parser.Parse(buf_str);
        if (status >= HttpParser::kComplete) {
          break;
        }
        memset(buffer, 0, BUFFER_SIZE + 1);
      }
    }
    result = parser.get_result();

    EXPECT_EQ(status, HttpParser::kComplete);
    EXPECT_EQ(result.status, 200);
    EXPECT_EQ(result.request.req.method, POST);
    EXPECT_EQ(result.request.req.version, HttpParser::kHttp1_1);
    EXPECT_EQ(result.request.header.size(), 2);

    EXPECT_EQ(result.request.header.count("transfer-encoding"), 1);
    transfer_encoding = result.request.header["transfer-encoding"];
    EXPECT_EQ(transfer_encoding.size(), 1);
    EXPECT_EQ(transfer_encoding.front(), "chunked");
    EXPECT_EQ(result.request.content.size(), 9);
    EXPECT_EQ(result.request.content, "jiskimjis");

    close(fd);
  }

  // f 39 transfer-encoding data no CRLF
  TestParseError(PARSER_PATH_PREFIX "f_39.txt", HttpParser::kClose, 400);

  // f 40 transfer-encoding data no CRLF => NOTE : 나중에 다시 테스트 해야함
  // TestParseError(PARSER_PATH_PREFIX "f_40.txt", HttpParser::kClose, 400);
}
