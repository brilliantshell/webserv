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
    : keep_alive_(true),
      is_data_(false),
      status_(HttpParser::kLeadingCRLF),
      body_length_(0),
      chunk_size_(0) {}

int HttpParser::Parse(std::string& segment) {
  if (status_ == kLeadingCRLF) {
    if (backup_buf_.size()) {
      segment = backup_buf_ + segment;
      backup_buf_.clear();
    }
    SkipLeadingCRLF(segment);
  }
  if (status_ == kRequestLine) {
    ReceiveRequestLine(segment);
  }
  if (status_ == kHeader) {
    ReceiveHeader(segment);
  }
  if (status_ == kContent) {
    ReceiveContent(segment);
  }
  if (status_ == kComplete && keep_alive_ == false) {
    status_ = kClose;
  }
  return status_;
}

void HttpParser::Clear(void) {
  keep_alive_ = true;
  is_data_ = false;
  status_ = kLeadingCRLF;
  body_length_ = 0;
  chunk_size_ = 0;
  request_line_buf_.clear();
  header_buf_.clear();
  chunked_buf_.clear();
  result_ = Result();
}

HttpParser::Result& HttpParser::get_result(void) { return result_; }

// SECTION : private

void HttpParser::SkipLeadingCRLF(std::string& segment) {
  // TODO segment.size() < 2
  if (!segment.compare(0, 2, CRLF)) {
    segment = segment.substr(2);
  }
  status_ = kRequestLine;
  if (!isupper(segment[0])) {
    UpdateStatus(400, kClose);  // BAD REQUEST
  }
}

// Request line 파싱

void HttpParser::ReceiveRequestLine(std::string& segment) {
  size_t ex_size = request_line_buf_.size();
  request_line_buf_.append(segment);
  size_t pos = request_line_buf_.find(CRLF);
  if (pos == std::string::npos) {
    if (request_line_buf_.size() > REQUEST_LINE_MAX) {
      UpdateStatus(414, kRLLenErr);  // BAD REQUEST
    }
  } else {
    request_line_buf_ = request_line_buf_.substr(0, pos);
    ParseRequestLine();
    if (status_ < kComplete) {
      status_ = kHeader;
      segment = segment.substr(pos - ex_size + 2);
    }
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

// Content 파싱

#define CHUNKED_SIZE_LINE_MAX 1024
#define CHUNK_SIZE_MAX 8192
void HttpParser::DecodeChunkedContent(std::string& segment) {
  chunked_buf_.append(segment);  // segment가 \r에서 끊기면?

  while (true) {  // FIXME
    if (is_data_) {
      if (chunked_buf_.size() < chunk_size_) {
        break;
      }
      result_.request.content.append(chunked_buf_, 0, chunk_size_);
      if (result_.request.content.size() > BODY_MAX) {
        UpdateStatus(413, kClose);  // REQUEST ENTITY TOO LARGE
        return;
      }
      if (chunked_buf_.compare(chunk_size_, 2, CRLF)) {
        UpdateStatus(400, kClose);  // BAD REQUEST
        return;
      }
      chunked_buf_ = chunked_buf_.substr(chunk_size_ + 2);
      is_data_ = false;
    } else {
      size_t pos = 0;
      pos = chunked_buf_.find(CRLF);
      if (pos == std::string::npos) {
        if (chunked_buf_.size() > CHUNKED_SIZE_LINE_MAX) {
          UpdateStatus(400, kClose);  // BAD REQUEST
          return;
        }
        break;
      } else {
        std::string chunk_size_line = chunked_buf_.substr(0, pos);
        chunked_buf_ = chunked_buf_.substr(pos + 2);
        if (pos > CHUNKED_SIZE_LINE_MAX) {
          UpdateStatus(400, kClose);  // BAD REQUEST
          return;
        }
        pos = chunk_size_line.find(";");
        if (pos == std::string::npos) {
          pos = chunk_size_line.find_first_not_of(HEXDIG);
          if (pos != std::string::npos) {
            UpdateStatus(400, kClose);  // BAD REQUEST
            return;
          }
        } else {
          chunk_size_line = chunk_size_line.substr(0, pos);
          pos = chunk_size_line.find_first_not_of(HEXDIG);
          if (chunk_size_line.find_first_not_of(SP HTAB, pos) !=
              std::string::npos) {
            UpdateStatus(400, kClose);  // BAD REQUEST
            return;
          }
        }
        std::stringstream ss(chunk_size_line);
        ss >> std::hex >> chunk_size_;
        if (chunk_size_ > CHUNK_SIZE_MAX) {
          UpdateStatus(400, kClose);  // BAD REQUEST
          return;
        }
        if (chunk_size_ == 0) {
          UpdateStatus(200, kComplete);
          return;
        }
        is_data_ = true;
      }
    }
  }
}

void HttpParser::ReceiveContent(std::string& segment) {
  if (body_length_ == 0) {
    status_ = kComplete;
    return;
  }
  if (body_length_ == CHUNKED) {
    DecodeChunkedContent(segment);
    // status_ = kComplete;  // FIXME
    return;
  }

  size_t remaining_bytes = body_length_ - result_.request.content.size();
  if (segment.size() >= remaining_bytes) {
    if (segment.size() > remaining_bytes) {
      backup_buf_ = segment.substr(remaining_bytes);
    }
    result_.request.content.append(segment, 0, remaining_bytes);
    status_ = kComplete;
    return;
  } else if (segment.size() < remaining_bytes) {
    result_.request.content.append(segment);
  }
}

// Update status
void HttpParser::UpdateStatus(int http_status, int parser_status) {
  result_.status = http_status;
  status_ = parser_status;
}
