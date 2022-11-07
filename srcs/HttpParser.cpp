/**
 * @file HttpParser.cpp
 * @author ghan, jiskim, yongjule
 * @brief Parse HTTP request
 * @date 2022-09-27
 *
 * @copyright Copyright (c) 2022
 */

#include "HttpParser.hpp"

/**
 * @brief 요청을 파싱하는 HttpParser 객체 생성자
 *
 */
HttpParser::HttpParser(void)
    : keep_alive_(true),
      is_data_(kChunkSize),
      status_(HttpParser::kLeadingCRLF),
      body_length_(0),
      chunk_size_(0) {}

/**
 * @brief status 에 따라 파싱할 부분을 구분하여 요청 파싱
 *
 * @param segment client 로 부터 받은 데이터
 * @return int 파싱 상태 및 connection close 여부
 */
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

/**
 * @brief 다음 리퀘스트가 있는 지 여부 반환
 *
 * @return true
 * @return false
 */
bool HttpParser::DoesNextReqExist(void) {
  return (status_ == kComplete && backup_buf_.empty() == false);
}

/**
 * @brief HttpParser 객체 초기화
 *
 */
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

/**
 * @brief HttpParser 객체 재설정
 *
 */
void HttpParser::Reset(void) {
  Clear();
  backup_buf_.clear();
}

/**
 * @brief 파싱 결과 반환
 *
 * @return HttpParser::Result& 파싱한 결과
 */
HttpParser::Result& HttpParser::get_result(void) { return result_; }

// SECTION : private
/**
 * @brief segment 에서 첫 CRLF 제거 및 유효성 검증
 *
 * @param segment 클라이언트로 부터 받은 데이터
 */
void HttpParser::SkipLeadingCRLF(std::string& segment) {
  if (segment.size() > 2 && segment.compare(0, 2, CRLF) == 0) {
    segment.erase(0, 2);
  }
  status_ = kRequestLine;
  if (isupper(segment[0]) == false) {
    UpdateStatus(400, kClose);  // BAD REQUEST
  }
}

/**
 * @brief Request Line 이 끝날 때 까지 이어서 버퍼에 receive
 *
 * @param segment client 로 부터 받은 데이터
 */
void HttpParser::ReceiveRequestLine(std::string& segment) {
  size_t ex_buf_size = request_line_buf_.size();
  request_line_buf_.append(segment);
  size_t pos = request_line_buf_.find(CRLF);
  if (pos == std::string::npos) {
    if (request_line_buf_.size() > REQUEST_LINE_MAX) {
      UpdateStatus(414, kRLLenErr);  // BAD REQUEST
    }
    return;
  }
  request_line_buf_.erase(pos);
  ParseRequestLine();
  if (status_ < kComplete) {
    status_ = kHeader;
    segment.erase(0, pos - ex_buf_size + 2);
  }
}

/**
 * @brief Request Line 파싱
 *
 */
void HttpParser::ParseRequestLine(void) {
  size_t pos = request_line_buf_.find(SP);
  TokenizeMethod(pos);
  TokenizePath(pos);
  TokenizeVersion(pos);
}

/**
 * @brief Request Line 에서 Method 파싱 및 유효성 검증
 *
 * @param pos 파싱 시작 위치
 */
void HttpParser::TokenizeMethod(size_t& pos) {
  if (pos == std::string::npos) {
    return UpdateStatus(400, kClose);  // BAD REQUEST
  }
  if (pos > METHOD_MAX) {
    return UpdateStatus(501, kComplete);  // NOT IMPLEMENTED
  }
  std::string token(request_line_buf_, 0, pos);
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

/**
 * @brief Request Line 에서 Path 파싱 및 유효성 검증 후 path 와 query 저징,
 * absolute form 일 경우 host 업데이트
 *
 * @param pos 파싱 시작 위치
 */
void HttpParser::TokenizePath(size_t& pos) {
  if (status_ >= kComplete) {
    return;
  }
  size_t pos_back = request_line_buf_.find(SP, ++pos);
  if (pos_back == std::string::npos) {
    return UpdateStatus(400, kClose);  // BAD REQUEST
  }
  if (pos_back > REQUEST_PATH_MAX + pos) {
    return UpdateStatus(414, kRLLenErr);  // URI TOO LONG
  }
  UriParser uri_parser;
  UriParser::Result uri_result =
      uri_parser.ParseTarget(request_line_buf_.substr(pos, pos_back - pos));
  if (uri_result.is_valid == false) {
    return UpdateStatus(400, kClose);  // BAD REQUEST
  }
  PathResolver path_resolver;
  PathResolver::Status path_status =
      path_resolver.Resolve(uri_result.path, PathResolver::kHttpParser);
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

/**
 * @brief Request line 에서 HTTP 버전 파싱 및 유효성 검증
 *
 * @param pos 파싱 시작 위치
 */
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

/**
 * @brief body_length_ 로 content 를 받을지 판별 후 받아야 할 경우 chunked 인지
 * 여부에 따라 content 파싱
 *
 * @param segment client 로 부터 받은 데이터
 */
void HttpParser::ReceiveContent(std::string& segment) {
  if (body_length_ == 0) {
    status_ = kComplete;
    if (segment.size() > 0) {
      backup_buf_.append(segment);
    }
    return;
  }
  if (body_length_ == CHUNKED) {
    return DecodeChunkedContent(segment);
  }
  size_t remaining_bytes = body_length_ - result_.request.content.size();
  if (segment.size() >= remaining_bytes) {
    if (segment.size() > remaining_bytes) {
      backup_buf_.assign(segment, remaining_bytes);
    }
    result_.request.content.append(segment, 0, remaining_bytes);
    status_ = kComplete;
    return;
  }
  result_.request.content.append(segment);
}

/**
 * @brief chunked content 를 디코드하여 content 에 저장
 *
 * @param segment client 로 부터 받은 데이터
 */
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

/**
 * @brief chunked data 파싱 및 유효성 검증
 *
 * @return true
 * @return false
 */
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
  chunked_buf_.erase(0, chunk_size_ + 2);
  is_data_ = kChunkSize;
  return true;
}

/**
 * @brief chunk size 파싱
 *
 * @return true
 * @return false
 */
bool HttpParser::ParseChunkSize(void) {
  size_t pos = chunked_buf_.find(CRLF);
  if (pos == std::string::npos) {
    if (chunked_buf_.size() > CHUNKED_SIZE_LINE_MAX) {
      UpdateStatus(400, kClose);  // BAD REQUEST
    }
    return false;
  }
  if (pos > CHUNKED_SIZE_LINE_MAX) {
    UpdateStatus(400, kClose);  // BAD REQUEST
    return false;
  }
  std::string chunk_size_line(chunked_buf_, 0, pos);
  chunked_buf_.erase(0, pos + 2);
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
  return true;
}

/**
 * @brief chunked 요청 끝났는지 확인, 끝났으면 상태 업데이트
 *
 */
void HttpParser::ParseChunkEnd(void) {
  if (chunked_buf_.size() >= 2) {
    if (chunked_buf_.compare(0, 2, CRLF)) {
      return UpdateStatus(400, kClose);  // BAD REQUEST
    }
    status_ = kComplete;
    backup_buf_.assign(chunked_buf_, 2);
  }
}

/**
 * @brief chunk size 라인에서 chunk extension 있으면 지우기, chunk size HEXDIG
 * 만 있는지 검증
 *
 * @param chunk_size_line chunk size 라인
 * @return true
 * @return false
 */
bool HttpParser::IgnoreChunkExtension(std::string& chunk_size_line) {
  size_t pos = chunk_size_line.find(";");
  if (pos == std::string::npos) {
    if (chunk_size_line.find_first_not_of(HEXDIG) != std::string::npos) {
      UpdateStatus(400, kClose);  // BAD REQUEST
      return false;
    }
  } else {
    chunk_size_line.erase(pos);
    pos = chunk_size_line.find_first_not_of(HEXDIG);
    if (chunk_size_line.find_first_not_of(SP HTAB, pos) != std::string::npos) {
      UpdateStatus(400, kClose);  // BAD REQUEST
      return false;
    }
  }
  return true;
}

/**
 * @brief HTTP 상태코드 및 파서 상태코드 설정
 *
 * @param http_status HTTP 상태코드
 * @param parser_status 파서 상태코드
 */
void HttpParser::UpdateStatus(int http_status, int parser_status) {
  result_.status = http_status;
  status_ = parser_status;
}
