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
    : status_(HttpParser::kLeadingCRLF), body_length_(0) {}

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
    UpdateStatus(400, kComplete);  // BAD REQUEST
  }
}

// Request line 파싱

void HttpParser::ReceiveRequestLine(size_t& start, const std::string& segment) {
  size_t pos = segment.find(CRLF, start);
  if (pos == std::string::npos) {
    request_line_buf_.append(segment.substr(start));
    if (request_line_buf_.size() > REQUEST_LINE_MAX) {
      UpdateStatus(400, kRLLenErr);  // BAD REQUEST
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
    UpdateStatus(400, kComplete);  // BAD REQUEST
    return;
  }
  if (pos > METHOD_MAX) {
    UpdateStatus(501, kComplete);  // NOT IMPLEMENTED
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
    UpdateStatus((token.find_first_not_of(UPPER_ALPHA) == std::string::npos)
                     ? 501   // NOT IMPLEMENTED
                     : 400,  // BAD REQUEST
                 kComplete);
  }
}

void HttpParser::TokenizePath(size_t& pos) {
  if (status_ >= kComplete) {
    return;
  }
  size_t pos_back = request_line_buf_.find(SP, ++pos);
  if (pos_back == std::string::npos) {
    UpdateStatus(400, kComplete);  // BAD REQUEST
    return;
  }
  if (pos_back - pos > REQUEST_PATH_MAX) {
    UpdateStatus(414, kComplete);  // URI TOO LONG
    return;
  }
  UriParser uri_parser;
  UriParser::Result uri_result =
      uri_parser.ParseTarget(request_line_buf_.substr(pos, pos_back - pos));
  if (uri_result.is_valid == false) {
    UpdateStatus(400, kComplete);  // BAD REQUEST
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
    UpdateStatus(400, kComplete);  // BAD REQUEST
    return;
  }
  std::string version = request_line_buf_.substr(pos);
  if (version == "HTTP/1.1") {
    result_.request.req.version = kHttp1_1;
  } else if (version == "HTTP/1.0") {
    result_.request.req.version = kHttp1_0;
  } else {
    UpdateStatus(505, kComplete);  // HTTP VERSION NOT SUPPORTED
  }
}

// Header 파싱

void HttpParser::ReceiveHeader(size_t& start, const std::string& segment) {
  size_t pos = segment.find(CRLF CRLF, start);
  if (pos == std::string::npos) {
    header_buf_.append(segment.substr(start));
    if (header_buf_.size() > HEADER_MAX) {
      UpdateStatus(400, kHDLenErr);  // BAD REQUEST
      return;
    }
  } else {
    header_buf_.append(segment.substr(start, pos + 2 - start));
    ParseHeader();
    if (header_buf_.size() > HEADER_MAX) {
      UpdateStatus(400, kHDLenErr);  // BAD REQUEST
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
    UpdateStatus(400, kComplete);  // BAD REQUEST
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
    UpdateStatus(400, kComplete);  // BAD REQUEST
  }
}

void HttpParser::TokenizeFieldValueList(size_t& cursor, std::string& name) {
  if (status_ >= kComplete) {
    return;
  }

  size_t value_start = cursor;
  SkipWhiteSpace(cursor);
  size_t start = cursor;
  while (cursor < header_buf_.size() &&
         (IsCharSet(VCHAR SP HTAB, true)(header_buf_[cursor]) ||
          (static_cast<uint8_t>(header_buf_[cursor]) >= 0x80 &&
           static_cast<uint8_t>(header_buf_[cursor]) <= 0xFF))) {
    ++cursor;
  }
  size_t value_end = cursor;
  while (value_end > start &&
         IsCharSet(SP HTAB, true)(header_buf_[value_end - 1])) {
    --value_end;
  }
  result_.request.header[name].push_back(
      header_buf_.substr(start, value_end - start));

  if (cursor + 1 >= header_buf_.size() ||
      header_buf_.compare(cursor, 2, CRLF) ||
      cursor - value_start > FIELD_VALUE_MAX) {
    UpdateStatus(400, kComplete);  // BAD REQUEST
  }
}

void HttpParser::ValidateHost(void) {
  Request& request = result_.request;
  Fields::iterator it = request.header.find("host");
  if (it != request.header.end()) {  // yes header
    if (it->second.size() != 1) {
      UpdateStatus(400, kComplete);  // BAD REQUEST
    } else if (request.req.host.empty()) {
      if (UriParser().ParseHost(it->second.front())) {
        request.req.host = it->second.front();
      } else {
        UpdateStatus(400, kComplete);  // BAD REQUEST
      }
    }
  } else if (request.req.version == kHttp1_1) {
    UpdateStatus(400, kComplete);
  }
}

// TODO : Content-Length, Transfer-Encoding
void HttpParser::DetermineBodyLength(void) {
  Request& request = result_.request;
  Fields::iterator it = request.header.find("content-length");
  if (it != request.header.end()) {  // yes header
    if (it->second.size() != 1) {
      UpdateStatus(400, kComplete);  // BAD REQUEST
    } else {
      if (it->second.front().find_first_not_of(DIGIT) == std::string::npos) {
        std::stringstream ss(it->second.front());
        ss >> body_length_;
        if (body_length_ > BODY_MAX) {
          UpdateStatus(413, kComplete);  // CONTENT LENGTH TOO LARGE
        }
      } else {
        UpdateStatus(400, kComplete);  // BAD REQUEST
      }
    }
  }
}

void HttpParser::ParseHeader(void) {
  if (result_.status != 200) {  // FIXME
    return;
  }
  size_t start = 0;
  while (start < header_buf_.size()) {
    std::string name = TokenizeFieldName(start);
    TokenizeFieldValueList(start, name);
    if (status_ >= kComplete) {
      break;
    }
    start = header_buf_.find(CRLF, start) + 2;
  }
  if (status_ < kComplete) {
    ValidateHost();
    DetermineBodyLength();
  }
}

void HttpParser::UpdateStatus(int http_status, int parser_status) {
  result_.status = http_status;
  status_ = parser_status;
}
