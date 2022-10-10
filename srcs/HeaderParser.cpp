/**
 * @file HeaderParser.cpp
 * @author ghan, jiskim, yongjule
 * @brief Parse HTTP request
 * @date 2022-10-06
 *
 * @copyright Copyright (c) 2022
 */

#include "HttpParser.hpp"

// Header 파싱

void HttpParser::ReceiveHeader(std::string& segment) {
  size_t ex_size = header_buf_.size();
  header_buf_.append(segment);
  if (header_buf_.size() > 1 && !header_buf_.compare(0, 2, CRLF)) {
    if (result_.request.req.version == kHttp1_1) {
      UpdateStatus(400, kClose);  // BAD REQUEST
    }
    status_ = kClose;
    return;
  }
  size_t pos = header_buf_.find(CRLF CRLF);
  if (pos == std::string::npos) {
    if (header_buf_.size() > HEADER_MAX) {
      UpdateStatus(400, kHDLenErr);  // BAD REQUEST
      return;
    }
  } else {
    header_buf_ = header_buf_.substr(0, pos + 2);
    ParseHeader();
    if (header_buf_.size() > HEADER_MAX) {
      UpdateStatus(400, kHDLenErr);  // BAD REQUEST
      return;
    }
    if (status_ < kComplete) {
      status_ = kContent;
      segment = segment.substr(pos - ex_size + 4);
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
  if (header_buf_[cursor] != ':' || cursor - start > FIELD_NAME_MAX) {
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
        if (request.req.version == kHttp1_1) {
          request.req.host = it->second.front();
        }
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
  std::pair<std::string, size_t> valid_codings[7] = {
      std::make_pair("chunked", 0),   std::make_pair("compress", 0),
      std::make_pair("deflate", 0),   std::make_pair("gzip", 0),
      std::make_pair("identity", 0),  std::make_pair("x-gzip", 0),
      std::make_pair("x-compress", 0)};
  std::map<std::string, size_t> valid_codings_map(valid_codings,
                                                  valid_codings + 7);
  ParseFieldValueList(encodings, valid_codings_map, 501, ',');
  if (status_ >= kComplete) {
    return;
  }
  if (encodings.empty() || encodings.back() != "chunked") {
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
    UpdateStatus(411, (result_.request.req.version == kHttp1_1)
                          ? kComplete
                          : kClose);  // LENGTH REQUIRED
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

// http 1.1 : close 있으면 close임 아니면 keep-alive
// http 1.0 : keep-alive 없으면 close
// connection 에는 header 에 있는 것만 들어갈 수 있다(keep-alive, close 제외)
// 중복되면 400
void HttpParser::ValidateConnection(void) {
  if (status_ >= kComplete) {
    return;
  }
  keep_alive_ = result_.request.req.version;
  Fields& header = result_.request.header;
  Fields::iterator it = header.find("connection");
  if (it != header.end()) {
    std::map<std::string, size_t> valid_value_map =
        GenerateValidValueMap(header.begin(), header.end());
    valid_value_map["keep-alive"] = 0;
    valid_value_map["close"] = 0;
    ParseFieldValueList(it->second, valid_value_map, 400, ',');
    if (status_ >= kComplete) {
      return;
    }
    keep_alive_ = result_.request.req.version == kHttp1_1
                      ? (std::find(it->second.begin(), it->second.end(),
                                   "close") == it->second.end())
                      : (valid_value_map["keep-alive"] == 1);
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
    start += 2;
  }
  if (status_ < kComplete) {
    ValidateHost();
    DetermineBodyLength();
    ValidateConnection();
  }
}
