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
}
