/**
 * @file HeaderParser.cpp
 * @author ghan, jiskim, yongjule
 * @brief Parse HTTP request
 * @date 2022-10-06
 *
 * @copyright Copyright (c) 2022
 */

#include "HttpParser.hpp"

/**
 * @brief white space 를 스킵
 *
 * @param cursor 이동 할 커서
 */
void HttpParser::SkipWhiteSpace(size_t& cursor) {
  if (status_ >= kComplete) {
    return;
  }
  while (cursor < header_buf_.size() &&
         IsCharSet(SP HTAB, true)(header_buf_[cursor]) == true) {
    ++cursor;
  }
  if (cursor == header_buf_.size()) {
    UpdateStatus(400, kClose);  // BAD REQUEST
  }
}

/**
 * @brief 요청의 header 가 끝날 때 까지 버퍼에 저장 후 길이 체크
 *
 * @param segment client 로 부터 받은 데이터
 */
void HttpParser::ReceiveHeader(std::string& segment) {
  size_t ex_buf_size = header_buf_.size();
  header_buf_.append(segment, 0, segment.size());
  if (header_buf_.size() > 1 && header_buf_.compare(0, 2, CRLF) == 0) {
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
    }
    return;
  }
  header_buf_.erase(pos + 2);
  ParseHeader();
  if (header_buf_.size() > HEADER_MAX) {
    return UpdateStatus(400, kHDLenErr);  // BAD REQUEST
  }
  if (status_ < kComplete) {
    status_ = kContent;
    segment.erase(0, pos - ex_buf_size + 4);
  }
}

/**
 * @brief  헤더 파싱
 *
 */
void HttpParser::ParseHeader(void) {
  if (result_.status != 200) {
    return;
  }
  for (size_t start = 0; start < header_buf_.size(); start += 2) {
    std::string name = TokenizeFieldName(start);
    TokenizeFieldValueList(start, name);
    if (status_ >= kComplete) {
      return;
    }
  }
  if (status_ < kComplete) {
    ValidateHost();
    DetermineBodyLength();
    ValidateConnection();
  }
}

/**
 * @brief 헤더에서 필드 이름 파싱
 *
 * @param cursor 필드 이름 시작 위치
 * @return std::string 필드 이름
 */
std::string HttpParser::TokenizeFieldName(size_t& cursor) {
  if (status_ >= kComplete) {
    return "";
  }
  size_t start = cursor;
  while (cursor < header_buf_.size() &&
         IsCharSet(TCHAR, true)(header_buf_[cursor]) == true) {
    header_buf_[cursor] = ::tolower(header_buf_[cursor]);
    ++cursor;
  }
  if (header_buf_[cursor] != ':' || cursor - start > FIELD_NAME_MAX) {
    UpdateStatus(400, kClose);  // BAD REQUEST
  }
  return header_buf_.substr(start, cursor++ - start);
}

/**
 * @brief 헤더에서 필드 값 파싱, 필드가 여러 번 나올 경우 리스트 다음 요소로
 * 저장
 *
 * @param cursor value 시작 위치
 * @param name 필드 이름
 */
void HttpParser::TokenizeFieldValueList(size_t& cursor, std::string& name) {
  if (status_ >= kComplete) {
    return;
  }
  size_t value_start = cursor;
  SkipWhiteSpace(cursor);
  size_t start = cursor;
  while (cursor < header_buf_.size() &&
         (IsCharSet(VCHAR SP HTAB, true)(header_buf_[cursor]) == true ||
          static_cast<uint8_t>(header_buf_[cursor]) >= 0x80)) {
    ++cursor;
  }
  size_t value_end = cursor;
  while (value_end > start &&
         IsCharSet(SP HTAB, true)(header_buf_[value_end - 1]) == true) {
    --value_end;
  }
  result_.request.header[name].push_back(
      header_buf_.substr(start, value_end - start));

  if (cursor + 1 >= header_buf_.size() ||
      header_buf_.compare(cursor, 2, CRLF) != 0 ||
      cursor > FIELD_VALUE_MAX + value_start) {
    UpdateStatus(400, kClose);  // BAD REQUEST
  }
}

/**
 * @brief 필드 값 파싱 후 delim 에 따라 리스트로 저장,
 * 유효하지 않을 경우 에러 상태 설정
 *
 * @param values 필드의 값 리스트
 * @param valid_map 유효한 필드 이름의 map
 * @param no_match_status 매칭된 값이 없을 때 설정할 HTTP 상태 (501 | 400)
 * @param delim 필드의 값들을 구분하는 구분자
 */
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
        return UpdateStatus(400, kClose);  // BAD REQUEST;
      }
      token.erase(token_end, token.end());
      std::transform(token.begin(), token.end(), token.begin(), ::tolower);
      std::map<std::string, size_t>::iterator it = valid_map.find(token);
      if (it == valid_map.end()) {
        return UpdateStatus(no_match_status, kClose);
      }
      if (it->second > 0) {
        return UpdateStatus(400, kClose);  // BAD REQUEST
      }
      values.push_back(token);
      ++(it->second);
    }
  }
}

/**
 * @brief 헤더에서 파싱한 Host 필드 값 검증
 *
 */
void HttpParser::ValidateHost(void) {
  Request& request = result_.request;
  Fields::iterator it = request.header.find("host");
  if (it != request.header.end()) {
    if (it->second.size() != 1) {
      UpdateStatus(400, kClose);  // BAD REQUEST
    } else if (request.req.host.empty() == true) {
      if (UriParser().ParseHost(it->second.front()) == true) {
        if (request.req.version == kHttp1_1) {
          request.req.host = it->second.front();
        }
        return;
      }
      UpdateStatus(400, kClose);  // BAD REQUEST
    }
  } else if (request.req.version == kHttp1_1) {
    UpdateStatus(400, kClose);
  }
}

/**
 * @brief Transfer-Encoding / Content-Length 유무 및 유효성 여부에 따라 HTTP
 *
 */
void HttpParser::DetermineBodyLength(void) {
  if (status_ >= kComplete) {
    return;
  }
  Fields& header = result_.request.header;
  Fields::iterator cl_it = header.find("content-length");
  Fields::iterator te_it = header.find("transfer-encoding");
  if (te_it != header.end() && cl_it != header.end()) {
    return UpdateStatus(400, kClose);  // BAD REQUEST
  }
  if (te_it != header.end() && result_.request.req.version == kHttp1_1) {
    ParseTransferEncoding(te_it->second);
  } else if (cl_it != header.end()) {
    ParseContentLength(cl_it->second);
  } else if (result_.request.req.method == POST) {
    UpdateStatus(411, (result_.request.req.version == kHttp1_1)
                          ? kComplete
                          : kClose);  // LENGTH REQUIRED
  }
}

/**
 * @brief Content-Length 헤더 값 파싱
 *
 * @param content_length Content-Length 헤더 값
 */
void HttpParser::ParseContentLength(std::list<std::string>& content_length) {
  if (content_length.size() != 1 ||
      content_length.front().find_first_not_of(DIGIT) != std::string::npos) {
    return UpdateStatus(400, kClose);  // BAD REQUEST
  }
  std::stringstream ss(content_length.front());
  ss >> body_length_;
  if (body_length_ > BODY_MAX) {
    UpdateStatus(413, kComplete);  // CONTENT LENGTH TOO LARGE
  }
}

/**
 * @brief Transfer-Encoding 헤더 값 파싱
 *
 * @param encodings Transfer-Encoding 헤더 값 리스트
 */
void HttpParser::ParseTransferEncoding(std::list<std::string>& encodings) {
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
    return UpdateStatus(400, kClose);  // BAD REQUEST
  }
  body_length_ = CHUNKED;
}

/**
 * @brief HTTP version 과 Connection 헤더를 통해 연결 유지 여부 결정
 * http 1.1 : close 있으면 close임 아니면 keep-alive
 * http 1.0 : keep-alive 없으면 close
 * connection 에는 header 에 있는 것만 들어갈 수 있다(keep-alive, close 제외)
 * 중복되면 400
 *
 */
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
    keep_alive_ = (result_.request.req.version == kHttp1_1)
                      ? (std::find(it->second.begin(), it->second.end(),
                                   "close") == it->second.end())
                      : (valid_value_map["keep-alive"] == 1);
  }
}

/**
 * @brief 헤더 필드 중복을 확인하기 위해 맵 second 값으로 헤더 필드 개수 셀 수
 * 있는 맵 생성
 *
 * @tparam InputIterator 이터레이터 타입
 * @param first 시작 이터레이터
 * @param last 종료 이터레이터
 * @return std::map<std::string, size_t> 중복 검사 맵
 */
template <typename InputIterator>
std::map<std::string, size_t> HttpParser::GenerateValidValueMap(
    InputIterator first, InputIterator last) {
  std::map<std::string, size_t> valid_map;
  for (; first != last; ++first) {
    valid_map[(*first).first] = 0;
  }
  return valid_map;
}
