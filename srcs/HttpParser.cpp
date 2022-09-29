/**
 * @file HttpParser.cpp
 * @author ghan, jiskim, yongjule
 * @brief Parse HTTP request
 * @date 2022-09-27
 *
 * @copyright Copyright (c) 2022
 */

#include "HttpParser.hpp"

HttpParser::HttpParser(void)
    : status_(CONTINUE), current_section_(HttpParser::kLeadingCRLF) {}

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
  if (!isupper(segment[pos])) {
    status_ = COMPLETE;
    result_.status = 400;  // BAD REQUEST
  }
  return pos;
}

void HttpParser::ReceiveRequestLine(size_t start, const std::string& segment) {
  if (status_ != CONTINUE) {
    return;
  }
  size_t pos = segment.find(CRLF, start);
  if (pos == std::string::npos) {
    request_line_buf_.append(segment.substr(start));
    if (request_line_buf_.size() > REQUEST_LINE_MAX) {
      status_ = RL_LEN_ERR;
      result_.status = 400;  // BAD REQUEST
      return;
    }
    status_ = CONTINUE;
  } else {
    request_line_buf_.append(segment.substr(start, pos));
    ParseRequestLine();
    if (request_line_buf_.size() > REQUEST_LINE_MAX) {
      status_ = RL_LEN_ERR;
      return;
    }
    current_section_ = kHeader;
    header_buf_.append(segment.substr(pos + 2));
    status_ = COMPLETE;  // FIXME for test
  }
}

void HttpParser::TokenizeMethod(size_t& pos) {
  if (pos == std::string::npos) {
    result_.status = 400;  // BAD REQUEST
    status_ = COMPLETE;
    return;
  }
  if (pos > METHOD_MAX) {
    result_.status = 501;  // NOT IMPLEMENTED
    status_ = COMPLETE;
    return;
  }
  std::string token = request_line_buf_.substr(0, pos);

  // 대문자 검사
  if (token == "GET") {
    result_.request.req.method = GET;
  } else if (token == "POST") {
    result_.request.req.method = POST;
  } else if (token == "DELETE") {
    result_.request.req.method = DELETE;
  } else {
    status_ = COMPLETE;
    result_.status =
        (std::find_if(token.begin(), token.end(),
                      IsCharSet(UPPER_ALPHA, false)) == token.end())
            ? 501
            : 400;  // BAD REQUEST or NOT IMPLEMENTED
  }
}

void HttpParser::TokenizePath(size_t& pos) {
  if (status_ != CONTINUE) {
    return;
  }
  size_t pos_back = request_line_buf_.find(SP, ++pos);
  if (pos_back == std::string::npos) {
    result_.status = 400;  // BAD REQUEST
    status_ = COMPLETE;
    return;
  }
  if (pos_back - pos > REQUEST_PATH_MAX) {
    result_.status = 414;  // URI TOO LONG
    status_ = COMPLETE;
    return;
  }
  UriParser uri_parser;
  UriParser::Result uri_result =
      uri_parser.Parse(request_line_buf_.substr(pos, pos_back - pos));
  if (uri_result.is_valid == false) {
    result_.status = 400;  // BAD REQUEST
    status_ = COMPLETE;
    return;
  }
  result_.request.req.host = uri_result.host;
  result_.request.req.path = uri_result.path;
  result_.request.req.query = uri_result.query;
  pos = pos_back + 1;
}

void HttpParser::TokenizeVersion(size_t& pos) {
  if (status_ != CONTINUE) {
    return;
  }
  if (request_line_buf_.find(SP, pos) != std::string::npos) {
    result_.status = 400;  // BAD REQUEST
    status_ = COMPLETE;
    return;
  }
  std::string version = request_line_buf_.substr(pos);
  if (version == "HTTP/1.1") {
    result_.request.req.version = kHttp1_1;
  } else if (version == "HTTP/1.0") {
    result_.request.req.version = kHttp1_0;
  } else {
    result_.status = 505;  // HTTP VERSION NOT SUPPORTED
    status_ = COMPLETE;
  }
}

void HttpParser::ParseRequestLine(void) {
  size_t pos = request_line_buf_.find(SP);

  TokenizeMethod(pos);
  TokenizePath(pos);
  TokenizeVersion(pos);
}
