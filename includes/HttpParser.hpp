/**
 * @file HttpParser.hpp
 * @author ghan, jiskim, yongjule
 * @brief Parse HTTP request
 * @date 2022-09-27
 *
 * @copyright Copyright (c) 2022
 */

#ifndef INCLUDES_HTTPPARSER_HPP_
#define INCLUDES_HTTPPARSER_HPP_

#include <iostream>

#include "ParseUtils.hpp"
#include "Types.hpp"
#include "UriParser.hpp"

#define CRLF "\r\n"
#define SP " "

#define BUFFER_SIZE 4097

// HTTP request 길이 제한
#define METHOD_MAX 6
#define REQUEST_PATH_MAX 8192
#define HTTP_VERSION_MAX 8
#define REQUEST_LINE_MAX 8208  // 8192 + 7(method max + 1) + 9(version max + 1)
#define HEADER_MAX 16000       // 16KB
#define BODY_MAX 128000000     // 128MB
#define REQUEST_MAX 128026000  // 128MB + 16KB + 10KB

// HTTP parser 상태값 (DEBUG)
#define CONTINUE 0
#define COMPLETE 1
#define RL_LEN_ERR 2
#define HD_LEN_ERR 3
#define BD_LEN_ERR 4

class HttpParser {
 public:
  enum { kHttp1_0 = 0, kHttp1_1 };

  struct Result {
    int status;
    Request request;

    Result(void) : status(200) {}
  };

  HttpParser(void);

  int Parse(const std::string& segment);

  const Result& get_result(void) const;

 private:
  enum { kLeadingCRLF = 0, kRequestLine, kHeader, kBody, kTrailer };

  int status_;
  int current_section_;
  std::string request_line_buf_;
  std::string header_buf_;
  std::string body_buf_;
  Result result_;

  size_t SkipLeadingCRLF(const std::string& segment);
  void ReceiveRequestLine(size_t start, const std::string& segment);

  // Parse request line
  void ParseRequestLine(void);
  void TokenizeMethod(size_t& pos);
  void TokenizePath(size_t& pos);
  void TokenizeVersion(size_t& pos);
};

#endif  // INCLUDES_HTTPPARSER_HPP_
