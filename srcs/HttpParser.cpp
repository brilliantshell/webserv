/**
 * @file HttpParser.cpp
 * @author ghan, jiskim, yongjule
 * @brief Parse HTTP request
 * @date 2022-09-27
 *
 * @copyright Copyright (c) 2022
 */

#include "HttpParser.hpp"

HttpParser::HttpParser(void) : status_(HttpParser::kLeadingCRLF) {}

int HttpParser::Parse(const std::string& segment) {
  size_t start = 0;
  if (status_ == kLeadingCRLF) {
    SkipLeadingCRLF(start, segment);
  }
  if (status_ == kRequestLine) {
    ReceiveRequestLine(start, segment);
  }
  if (status_ == kHeader) {
    ReceiveHeader(start, segment);
  }
  if (status_ == kComplete) {
  }
  // if (status_ == kHeader) {
  //   result_.status = 400;  // BAD REQUEST
  //   status_ = kHDLenErr;
  // }
  return status_;
}

HttpParser::Result& HttpParser::get_result(void) { return result_; }

// SECTION : private

void HttpParser::SkipLeadingCRLF(size_t& start, const std::string& segment) {
  if (!segment.compare(start, 2, CRLF)) {
    start += 2;
  }
  status_ = kRequestLine;
  if (!isupper(segment[start])) {
    status_ = kComplete;
    result_.status = 400;  // BAD REQUEST
  }
}

// Request line 파싱

void HttpParser::ReceiveRequestLine(size_t& start, const std::string& segment) {
  size_t pos = segment.find(CRLF, start);
  if (pos == std::string::npos) {
    request_line_buf_.append(segment.substr(start));
    if (request_line_buf_.size() > REQUEST_LINE_MAX) {
      status_ = kRLLenErr;
      result_.status = 400;  // BAD REQUEST
      return;
    }
  } else {
    request_line_buf_.append(segment.substr(start, pos));
    ParseRequestLine();
    if (request_line_buf_.size() > REQUEST_LINE_MAX) {
      status_ = kRLLenErr;
      return;
    }
    status_ = kHeader;
    start = pos + 2;
  }
}

void HttpParser::ParseRequestLine(void) {
  size_t pos = request_line_buf_.find(SP);

  TokenizeMethod(pos);
  TokenizePath(pos);
  TokenizeVersion(pos);
}

void HttpParser::TokenizeMethod(size_t& pos) {
  if (pos == std::string::npos) {
    result_.status = 400;  // BAD REQUEST
    status_ = kComplete;
    return;
  }
  if (pos > METHOD_MAX) {
    result_.status = 501;  // NOT IMPLEMENTED
    status_ = kComplete;
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
    status_ = kComplete;
    result_.status = (token.find_first_not_of(UPPER_ALPHA) == std::string::npos)
                         ? 501   // NOT IMPLEMENTED
                         : 400;  // BAD REQUEST
  }
}

void HttpParser::TokenizePath(size_t& pos) {
  if (status_ >= kComplete) {
    return;
  }
  size_t pos_back = request_line_buf_.find(SP, ++pos);
  if (pos_back == std::string::npos) {
    result_.status = 400;  // BAD
    status_ = kComplete;
    return;
  }
  if (pos_back - pos > REQUEST_PATH_MAX) {
    result_.status = 414;  // URI TOO LONG
    status_ = kComplete;
    return;
  }
  UriParser uri_parser;
  UriParser::Result uri_result =
      uri_parser.Parse(request_line_buf_.substr(pos, pos_back - pos));
  if (uri_result.is_valid == false) {
    result_.status = 400;  // BAD REQUEST
    status_ = kComplete;
    return;
  }
  std::transform(uri_result.host.begin(), uri_result.host.end(),
                 uri_result.host.begin(), ::tolower);
  result_.request.req.host = uri_result.host;
  result_.request.req.path = uri_result.path;
  result_.request.req.query = uri_result.query;
  pos = pos_back + 1;
}

void HttpParser::TokenizeVersion(size_t& pos) {
  if (status_ >= kComplete) {
    return;
  }
  if (request_line_buf_.find(SP, pos) != std::string::npos) {
    result_.status = 400;  // BAD REQUEST
    status_ = kComplete;
    return;
  }
  std::string version = request_line_buf_.substr(pos);
  if (version == "HTTP/1.1") {
    result_.request.req.version = kHttp1_1;
  } else if (version == "HTTP/1.0") {
    result_.request.req.version = kHttp1_0;
  } else {
    result_.status = 505;  // HTTP VERSION NOT SUPPORTED
    status_ = kComplete;
  }
}

// Header 파싱

void HttpParser::ReceiveHeader(size_t& start, const std::string& segment) {
  size_t pos = segment.find(CRLF CRLF, start);
  if (pos == std::string::npos) {
    header_buf_.append(segment.substr(start));
    if (header_buf_.size() > HEADER_MAX) {
      status_ = kHDLenErr;
      result_.status = 400;  // BAD REQUEST
      return;
    }
  } else {
    header_buf_.append(segment.substr(start, pos + 2 - start));
    ParseHeader();
    if (header_buf_.size() > HEADER_MAX) {
      status_ = kHDLenErr;
      result_.status = 400;  // BAD REQUEST
      return;
    }
    status_ = kContent;
    start = pos + 4;

    status_ = kComplete;  // FIXME ㄱㅖㅅㄱ 내내려려야야함함
  }
}

std::string HttpParser::TokenizeFieldName(size_t& cursor) {
  if (status_ >= kComplete) {
    return "";
  }
  size_t start = cursor;
  while (cursor < header_buf_.size() &&
         IsCharSet(TCHAR, true)(header_buf_[cursor])) {
    header_buf_[cursor] = ::tolower(header_buf_[cursor]);
    ++cursor;
  }
  if (header_buf_[cursor] != ':' || cursor > FIELD_NAME_MAX) {
    result_.status = 400;  // BAD REQUEST
    status_ = kComplete;
  }
  return header_buf_.substr(start, cursor++ - start);
}

void HttpParser::SkipWhiteSpace(size_t& cursor) {
  if (status_ >= kComplete) {
    return;
  }
  while (cursor < header_buf_.size() &&
         IsCharSet(SP HTAB, true)(header_buf_[cursor])) {
    ++cursor;
  }
  if (cursor == header_buf_.size()) {
    result_.status = 400;  // BAD REQUEST
    status_ = kComplete;
  }
}

void HttpParser::TokenizeFieldValueList(size_t& cursor,
                                        std::list<std::string>& value_list) {
  if (status_ >= kComplete) {
    return;
  }

  size_t value_list_start = cursor;
  // while (cursor < header_buf_.size()) {
  SkipWhiteSpace(cursor);
  size_t start = cursor;
  while (cursor < header_buf_.size() &&
         (IsCharSet(VCHAR, true)(header_buf_[cursor]) ||
          (static_cast<uint8_t>(header_buf_[cursor]) >= 0x80 &&
           static_cast<uint8_t>(header_buf_[cursor]) <= 0xFF))) {
    ++cursor;
  }
  value_list.push_back(header_buf_.substr(start, cursor - start));
  SkipWhiteSpace(cursor);
  // }

  if (cursor + 1 >= header_buf_.size() ||
      header_buf_.compare(cursor, 2, CRLF) ||
      cursor - value_list_start > FIELD_VALUE_MAX) {
    result_.status = 400;
    status_ = kComplete;
  }
}

void HttpParser::ParseHeader(void) {
  if (result_.status != 200) {  // FIXME
    return;
  }
  size_t start = 0;
  while (start < header_buf_.size()) {
    std::list<std::string> value_list;
    std::string name = TokenizeFieldName(start);
    TokenizeFieldValueList(start, value_list);
    if (status_ >= kComplete) {
      break;
    }
    result_.request.header[name] = value_list;
    start = header_buf_.find(CRLF, start) + 2;
  }
}
