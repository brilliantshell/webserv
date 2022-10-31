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
      is_data_(kChunkSize),
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

bool HttpParser::DoesNextReqExist(void) {
  std::cerr << "status : " << status_ << "\nback_up buf : " << backup_buf_
            << '\n';
  return (status_ == kComplete && backup_buf_.empty() == false);
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

void HttpParser::Reset(void) {
  Clear();
  backup_buf_.clear();
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
    request_line_buf_.erase(pos);
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
    return UpdateStatus(400, kClose);  // BAD REQUEST
  }
  if (pos > METHOD_MAX) {
    return UpdateStatus(501, kComplete);  // NOT IMPLEMENTED
  }
  std::string token(request_line_buf_, 0, pos);

  // 대문자 검사
  if (token == "GET") {
    result_.request.req.method = GET;
  } else if (token == "POST") {
    result_.request.req.method = POST;
  } else if (token == "DELETE") {
    result_.request.req.method = DELETE;
  } else {
    UpdateStatus((token.find_first_not_of(UPPER_ALPHA) == std::string::npos)
                     ? 405   // NOT IMPLEMENTED
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
    return UpdateStatus(400, kClose);  // BAD REQUEST
  }
  if (pos_back - pos > REQUEST_PATH_MAX) {
    return UpdateStatus(414, kRLLenErr);  // URI TOO LONG
  }
  UriParser uri_parser;
  UriParser::Result uri_result =
      uri_parser.ParseTarget(request_line_buf_.substr(pos, pos_back - pos));
  if (uri_result.is_valid == false) {
    return UpdateStatus(400, kClose);  // BAD REQUEST
  }
  PathResolver path_resolver;
  PathResolver::Status path_status = path_resolver.Resolve(
      uri_result.path, PathResolver::Purpose::kHttpParser);
  if (path_status == PathResolver::kFailure) {
    return UpdateStatus(400, kClose);  // BAD REQUEST
  }
  std::transform(uri_result.host.begin(), uri_result.host.end(),
                 uri_result.host.begin(), ::tolower);
  result_.request.req.host = uri_result.host;
  result_.request.req.path =
      uri_result.path + ((path_status == PathResolver::kFile)
                             ? path_resolver.get_file_name()
                             : "");
  result_.request.req.query = uri_result.query;
  pos = pos_back + 1;
}

void HttpParser::TokenizeVersion(size_t& pos) {
  if (status_ >= kComplete) {
    return;
  }
  if (request_line_buf_.find(SP, pos) != std::string::npos) {
    return UpdateStatus(400, kClose);  // BAD REQUEST
  }
  std::string version(request_line_buf_, pos);
  if (version.size() != 8) {
    UpdateStatus(400, kClose);  // BAD REQUEST
  } else if (version.compare(0, 7, "HTTP/1.") == 0 && '0' <= version[7] &&
             version[7] <= '9') {
    result_.request.req.version = version[7] == '0' ? kHttp1_0 : kHttp1_1;
  } else {
    UpdateStatus(505, kClose);  // HTTP VERSION NOT SUPPORTED
  }
}

// Content 파싱
void HttpParser::ReceiveContent(std::string& segment) {
  if (body_length_ == 0) {
    status_ = kComplete;
    if (segment.size() > 0) {
      backup_buf_.append(segment);
    }
    return;
  }
  if (body_length_ == CHUNKED) {
    DecodeChunkedContent(segment);
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

void HttpParser::DecodeChunkedContent(std::string& segment) {
  chunked_buf_.append(segment);

  while (true) {
    if (is_data_ == kChunkData) {
      if (ParseChunkData() == false) {
        return;
      }
    } else if (is_data_ == kChunkSize) {
      if (ParseChunkSize() == false) {
        return;
      }
    } else if (is_data_ == kChunkEnd) {
      return ParseChunkEnd();
    }
    if (status_ >= kComplete) {
      return;
    }
  }
}

bool HttpParser::ParseChunkData() {
  if (chunked_buf_.size() < chunk_size_ + 2) {
    return false;
  }
  result_.request.content.append(chunked_buf_, 0, chunk_size_);
  if (result_.request.content.size() > BODY_MAX) {
    UpdateStatus(413, kClose);  // REQUEST ENTITY TOO LARGE
    return false;
  }
  if (chunked_buf_.compare(chunk_size_, 2, CRLF)) {
    UpdateStatus(400, kClose);  // BAD REQUEST
    return false;
  }
  chunked_buf_ = chunked_buf_.substr(chunk_size_ + 2);
  is_data_ = kChunkSize;
  return true;
}

bool HttpParser::ParseChunkSize(void) {
  size_t pos = chunked_buf_.find(CRLF);
  if (pos == std::string::npos) {
    if (chunked_buf_.size() > CHUNKED_SIZE_LINE_MAX) {
      UpdateStatus(400, kClose);  // BAD REQUEST
    }
    return false;
  } else {
    if (pos > CHUNKED_SIZE_LINE_MAX) {
      UpdateStatus(400, kClose);  // BAD REQUEST
      return false;
    }
    std::string chunk_size_line(chunked_buf_, 0, pos);
    chunked_buf_ = chunked_buf_.substr(pos + 2);
    if (IgnoreChunkExtension(chunk_size_line) == false) {
      return false;
    }
    std::stringstream ss(chunk_size_line);
    ss >> std::hex >> chunk_size_;
    if (chunk_size_ > CHUNK_SIZE_MAX) {
      UpdateStatus(400, kClose);  // BAD REQUEST
      return false;
    }
    is_data_ = (chunk_size_ == 0) ? kChunkEnd : kChunkData;
  }
  return true;
}

void HttpParser::ParseChunkEnd(void) {
  if (chunked_buf_.size() >= 2) {
    if (chunked_buf_.compare(0, 2, CRLF)) {
      return UpdateStatus(400, kClose);  // BAD REQUEST
    }
    status_ = kComplete;
    backup_buf_ = chunked_buf_.substr(2);
  }
}

bool HttpParser::IgnoreChunkExtension(std::string& chunk_size_line) {
  size_t pos = chunk_size_line.find(";");
  if (pos == std::string::npos) {
    if (chunk_size_line.find_first_not_of(HEXDIG) != std::string::npos) {
      UpdateStatus(400, kClose);  // BAD REQUEST
      return false;
    }
  } else {
    chunk_size_line = chunk_size_line.substr(0, pos);
    pos = chunk_size_line.find_first_not_of(HEXDIG);
    if (chunk_size_line.find_first_not_of(SP HTAB, pos) != std::string::npos) {
      UpdateStatus(400, kClose);  // BAD REQUEST
      return false;
    }
  }
  return true;
}

// Update status
void HttpParser::UpdateStatus(int http_status, int parser_status) {
  result_.status = http_status;
  status_ = parser_status;
}
