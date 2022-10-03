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
    : keep_alive_(true), status_(HttpParser::kLeadingCRLF), body_length_(0) {}

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
  if (status_ == kComplete && keep_alive_ == false) {
    return kClose;
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
    UpdateStatus(400, kClose);  // BAD REQUEST
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
    if (status_ >= kComplete) {
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
    UpdateStatus(400, kClose);  // BAD REQUEST
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
    UpdateStatus(400, kClose);  // BAD REQUEST
    return;
  }
  if (pos_back - pos > REQUEST_PATH_MAX) {
    UpdateStatus(414, kRLLenErr);  // URI TOO LONG
    return;
  }
  UriParser uri_parser;
  UriParser::Result uri_result =
      uri_parser.ParseTarget(request_line_buf_.substr(pos, pos_back - pos));
  if (uri_result.is_valid == false) {
    UpdateStatus(400, kClose);  // BAD REQUEST
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
    UpdateStatus(400, kClose);  // BAD REQUEST
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
    if (status_ < kClose) {
      status_ = kContent;
      start = pos + 4;
      status_ = kComplete;  // FIXME ㄱㅖㅅㄱ 내내려려야야함함
    }
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
    UpdateStatus(400, kClose);  // BAD REQUEST
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
    UpdateStatus(400, kClose);  // BAD REQUEST
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
    UpdateStatus(400, kClose);  // BAD REQUEST
  }
}

void HttpParser::ValidateHost(void) {
  Request& request = result_.request;
  Fields::iterator it = request.header.find("host");
  if (it != request.header.end()) {  // yes header
    if (it->second.size() != 1) {
      UpdateStatus(400, kClose);  // BAD REQUEST
    } else if (request.req.host.empty()) {
      if (UriParser().ParseHost(it->second.front())) {
        request.req.host = it->second.front();
      } else {
        UpdateStatus(400, kClose);  // BAD REQUEST
      }
    }
  } else if (request.req.version == kHttp1_1) {  // no host
    UpdateStatus(400, kClose);
  }
}

void HttpParser::ParseFieldValueList(std::list<std::string>& values,
                                     std::map<std::string, size_t>& valid_map,
                                     int no_match_status, char delim) {
  size_t list_size = values.size();
  for (size_t i = 0; i < list_size; ++i) {
    std::istringstream ss(values.front());
    values.pop_front();
    std::string token;
    while (std::getline(ss, token, delim)) {
      std::string::iterator token_end =
          std::remove_if(token.begin(), token.end(), IsCharSet(SP HTAB, true));
      if (token_end == token.begin()) {
        UpdateStatus(400, kClose);  // BAD REQUEST
        return;
      }
      token.erase(token_end, token.end());
      std::transform(token.begin(), token.end(), token.begin(), ::tolower);
      if (valid_map.count(token) == 0) {
        UpdateStatus(no_match_status, kClose);
        return;
      }
      if (valid_map[token] > 0) {
        UpdateStatus(400, kClose);  // BAD REQUEST
        return;
      }
      values.push_back(token);
      ++valid_map[token];
    }
  }
}

void HttpParser::ParseTransferEncoding(std::list<std::string>& encodings) {
  size_t list_size = encodings.size();
  std::pair<std::string, size_t> valid_codings[8] = {
      std::make_pair("chunked", 0),    std::make_pair("compress", 0),
      std::make_pair("deflate", 0),    std::make_pair("gzip", 0),
      std::make_pair("identity", 0),   std::make_pair("x-gzip", 0),
      std::make_pair("x-compress", 0), std::make_pair("trailers", 0),
  };
  std::map<std::string, size_t> valid_codings_map(valid_codings,
                                                  valid_codings + 8);
  ParseFieldValueList(encodings, valid_codings_map, 501, ',');
  if (status_ >= kComplete) {
    return;
  }
  if (encodings.back() != "chunked") {
    UpdateStatus(400, kClose);  // BAD REQUEST
    return;
  }
  body_length_ = CHUNKED;
}

void HttpParser::ParseContentLength(std::list<std::string>& content_length) {
  if (content_length.size() != 1) {
    UpdateStatus(400, kClose);  // BAD REQUEST
  } else {
    if (content_length.front().find_first_not_of(DIGIT) == std::string::npos) {
      std::stringstream ss(content_length.front());
      ss >> body_length_;
      if (body_length_ > BODY_MAX) {
        UpdateStatus(413, kComplete);  // CONTENT LENGTH TOO LARGE
      }
    } else {
      UpdateStatus(400, kClose);  // BAD REQUEST
    }
  }
}

// TE & CL : error (400) - kClose
// TE :
//  if chunked is not the last token, 400
//  else, chunked parsing
// CL : if not valid number 400
// !TE || !CL : body length == 0
//
void HttpParser::DetermineBodyLength(void) {
  if (status_ >= kComplete) {
    return;
  }
  Fields& header = result_.request.header;
  Fields::iterator cl_it = header.find("content-length");
  Fields::iterator te_it = header.find("transfer-encoding");
  if (te_it != header.end() && cl_it != header.end()) {
    UpdateStatus(400, kClose);  // BAD REQUEST
    return;
  }
  if (te_it != header.end() && result_.request.req.version == kHttp1_1) {
    ParseTransferEncoding(te_it->second);
  } else if (cl_it != header.end()) {
    ParseContentLength(cl_it->second);
  } else if (result_.request.req.method == POST) {
    UpdateStatus(411, kComplete);  // LENGTH REQUIRED
    return;
  }
}

template <typename InputIterator>
std::map<std::string, size_t> HttpParser::GenerateValidValueMap(
    InputIterator first, InputIterator last) {
  std::map<std::string, size_t> valid_map;
  for (; first != last; ++first) {
    valid_map[(*first).first] = 0;
  }
  return valid_map;
}

void HttpParser::ValidateConnection(void) {
  if (status_ >= kComplete) {
    return;
  }
  // map ㅁㅏㄴㄷㄹ기

  Fields& header = result_.request.header;
  Fields::iterator it = header.find("connection");
  keep_alive_ = result_.request.req.version;  // 1.0 close 1.1 keep-alive
  if (it != header.end()) {
    std::map<std::string, size_t> valid_value_map =
        GenerateValidValueMap(header.begin(), header.end());
    valid_value_map["keep-alive"] = 0;
    valid_value_map["close"] = 0;
    ParseFieldValueList(it->second, valid_value_map, 400, ',');
    if (it->second.front() == "keep-alive") {
      keep_alive_ = kHttp1_1;
    }
    // it->second.push_back();
    // ParseFieldValueList(encodings, valid_codings_map, 501, ',');
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
    ValidateConnection();
  }
}

void HttpParser::UpdateStatus(int http_status, int parser_status) {
  result_.status = http_status;
  status_ = parser_status;
}
