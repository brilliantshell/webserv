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

#include "Types.hpp"

class HttpParser {
 public:
  enum { kHttp1_0 = 0, kHttp1_1 };

  struct Result {
    int status;
    Request request;

    Result(void) : status(200) {}
  };

  Result Parse(const std::string& segment);
  int ParseRequestLine(RequestLine& req, const std::string& segment);
};

#endif  // INCLUDES_HTTPPARSER_HPP_
