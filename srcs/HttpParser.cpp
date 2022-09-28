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

HttpParser::HttpParser(void)
    : status_(CONTINUE), current_section_(HttpParser::kLeadingCRLF) {}

void HttpParser::ParseRequestLine(void) {
  size_t pos = request_line_buf_.find(SP);
  std::string method;
  if (pos != std::string::npos) {
    method = request_line_buf_.substr(0, pos);
  }
  if (method == "GET") {
    result_.request.req.method = GET;
  } else if (method == "POST") {
    result_.request.req.method = POST;
  } else if (method == "DELETE") {
    result_.request.req.method = DELETE;
  } else {
    result_.status = 501;  // NOT IMPLEMENTED
  }

  size_t pos_back = request_line_buf_.find(SP, ++pos);
  result_.request.req.path = request_line_buf_.substr(pos, pos_back - pos);

  pos = pos_back + 1;
  std::string version = request_line_buf_.substr(pos);
  if (version == "HTTP/1.1") {
    result_.request.req.version = kHttp1_1;
  } else if (version == "HTTP/1.0") {
    result_.request.req.version = kHttp1_0;
  } else {
    // FIXME
  }
}

int HttpParser::Parse(const std::string& segment) {
  size_t start = 0;
  if (current_section_ == kLeadingCRLF) {
    start = SkipLeadingCRLF(segment);
  }
  ReceiveRequestLine(start, segment);
  // if (CONTINUE) {

  // }
  return status_;
}

const HttpParser::Result& HttpParser::get_result(void) const { return result_; }

// SECTION : private

size_t HttpParser::SkipLeadingCRLF(const std::string& segment) {
  size_t pos = 0;
  if (!segment.compare(pos, 2, CRLF)) {
    pos += 2;
  }
  current_section_ = kRequestLine;
  return pos;
}

void HttpParser::ReceiveRequestLine(size_t start, const std::string& segment) {
  size_t pos = segment.find(CRLF, start);
  if (pos == std::string::npos) {
    request_line_buf_.append(segment.substr(start));
    if (request_line_buf_.size() > REQUEST_LINE_MAX) {
      status_ = RL_LEN_ERR;
      return;
    }
    status_ = CONTINUE;
  } else {
    request_line_buf_.append(segment.substr(start, pos));
    if (request_line_buf_.size() > REQUEST_LINE_MAX) {
      status_ = RL_LEN_ERR;
      return;
    }
    ParseRequestLine();
    current_section_ = kHeader;
    header_buf_.append(segment.substr(pos + 2));
    status_ = COMPLETE;
  }
}