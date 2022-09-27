/**
 * @file HttpParser.cpp
 * @author ghan, jiskim, yongjule
 * @brief Parse HTTP request
 * @date 2022-09-27
 *
 * @copyright Copyright (c) 2022
 */

#include "HttpParser.hpp"

#include <iostream>

int HttpParser::ParseRequestLine(RequestLine& req, const std::string& segment) {
  size_t not_rn = 0;
  while (!segment.compare(not_rn, 2, "\r\n")) {
    not_rn += 2;
  }
  size_t pos = segment.find(" ", not_rn);
  std::string method = segment.substr(not_rn, pos - not_rn);

  if (method == "GET") {
    req.method = GET;
  } else if (method == "POST") {
    req.method = POST;
  } else if (method == "DELETE") {
    req.method = DELETE;
  } else {
    return 400;  // FIXME
  }
  size_t pos_back = segment.find(" ", pos + 1);
  req.path = segment.substr(pos + 1, pos_back - (pos + 1));

  pos = pos_back + 1;
  pos_back = segment.find(CRLF, pos);
  std::string version = segment.substr(pos, pos_back - pos);
  if (version == "HTTP/1.1") {
    req.version = kHttp1_1;
  } else if (version == "HTTP/1.0") {
    req.version = kHttp1_0;
  } else {
    return 400;  // FIXME
  }

  pos = segment.find("Host: ", pos) + 6;
  pos_back = segment.find(CRLF, pos);

  req.Host = segment.substr(pos, pos_back - pos);
  return 200;
}

HttpParser::Result HttpParser::Parse(const std::string& segment) {
  Result result;

  result.status = ParseRequestLine(result.request.req, segment);
  return result;
}
