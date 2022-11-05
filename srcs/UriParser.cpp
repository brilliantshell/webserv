/**
 * @file UriParser.cpp
 * @author ghan, jiskim, yongjule
 * @brief Validate URI according to RFC 3986
 * @date 2022-09-29
 *
 * @copyright Copyright (c) 2022
 */

#include "UriParser.hpp"

/**
 * @brief HTTP 요청 target URI 파싱
 *
 * @param uri 파싱할 URI
 * @return UriParser::Result 파싱 결과
 */
UriParser::Result UriParser::ParseTarget(std::string uri) {
  size_t pos = 0;
  if (uri[0] == '/') {
    ValidatePath(uri, pos);
    ValidateQuery(uri, pos);
  } else if (isalpha(uri[0]) == true) {
    ValidateScheme(uri, pos);
    ValidateHierPart(uri, ++pos);
  } else {
    result_.is_valid = false;
  }
  return result_;
}

/**
 * @brief HTTP 요청 Host 헤더 파싱
 *
 * @param uri 파싱할 URI
 * @return true
 * @return false
 */
bool UriParser::ParseHost(std::string& uri) {
  size_t pos = 0;
  ValidateAuthority(uri, pos);
  std::transform(uri.begin(), uri.end(), uri.begin(), ::tolower);
  return result_.is_valid;
}

/**
 * @brief ASCII 문자 HEX 로 인코딩
 *
 * @param path 인코딩 적용할 경로
 */
void UriParser::EncodeAsciiToHex(std::string& path) {
  IsCharSet is_reserved(RESERVED, true);
  for (size_t i = 0; i < path.size(); ++i) {
    if (is_reserved(path[i]) == true) {
      std::stringstream ss;
      ss << std::hex << (int)path[i];
      std::string hex = ss.str();
      if (hex.size() == 1) {
        hex = "0" + hex;
      }
      path.replace(i, 1, "%" + hex);
      i += 2;
    }
  }
}

/**
 * @brief HEX 로 인코딩된 ASCII 문자 디코딩
 *
 * @param uri 디코딩 적용할 URI
 * @param kPos 문자열에서 디코딩 적용
 * @return true
 * @return false
 */
bool UriParser::DecodeHexToAscii(std::string& uri, const size_t kPos) {
  IsCharSet is_hexdig(HEXDIG, true);
  if (kPos + 2 >= uri.size() || is_hexdig(uri[kPos + 1]) == false ||
      is_hexdig(uri[kPos + 2]) == false) {
    return false;
  }
  unsigned int x;
  std::stringstream ss;
  ss << std::hex << uri.substr(kPos + 1, 2);
  ss >> x;
  uri.replace(kPos, 3, 1, x);
  return true;
}

/**
 * @brief Origin Form / Absolute Form 으로 전체 경로 반환
 *
 * @return std::string Origin Form / Absolute Form
 */
std::string UriParser::GetFullPath(void) {
  return (result_.scheme.empty() == true)
             ? result_.path + result_.query
             : result_.scheme + "://" + result_.host + result_.port +
                   result_.path + result_.query;
}

// SECTION : private
/**
 * @brief 경로 파싱 및 검증
 *
 * @param uri 파싱할 URI
 * @param start 검증 시작 위치
 */
void UriParser::ValidatePath(std::string& uri, size_t& start) {
  if (result_.is_valid == false) {
    return;
  }
  size_t pos = start;
  while (pos < uri.size() && uri[pos] != '?' && result_.is_valid) {
    if (IsCharSet(PCHAR "/", false)(uri[pos]) == true) {
      result_.is_valid = (uri[pos] == '%') ? DecodeHexToAscii(uri, pos) : false;
    }
    ++pos;
  }
  if (result_.is_valid == true) {
    result_.path.assign(uri, start, pos - start);
  }
  start = pos;
}

/**
 * @brief Query string 파싱 및 검증
 *
 * @param uri 파싱할 URI
 * @param start 검증 시작 위치
 */
void UriParser::ValidateQuery(std::string& uri, size_t& start) {
  if (result_.is_valid == false || start >= uri.size()) {
    return;
  }
  size_t pos = start;
  while (pos < uri.size() && result_.is_valid == true) {
    if (IsCharSet(PCHAR "/?", false)(uri[pos]) == true) {
      if (uri[pos] == '%' && uri.size() > pos + 2) {
        result_.is_valid = (IsCharSet(HEXDIG, true)(uri[pos + 1]) == true &&
                            IsCharSet(HEXDIG, true)(uri[pos + 2]) == true);
        pos += 2;
      } else {
        result_.is_valid = false;
      }
    }
    ++pos;
  }
  if (result_.is_valid == true) {
    result_.query.assign(uri, start + 1);
  }
}

/**
 * @brief (Absolute Form) URI scheme 파싱 및 검증
 *
 * @param uri 파싱할 URI
 * @param start 검증 시작 위치
 */
void UriParser::ValidateScheme(std::string& uri, size_t& start) {
  if (result_.is_valid == false) {
    return;
  }
  while (start < uri.size() && uri[start] != ':') {
    if (IsCharSet(ALPHA DIGIT "+-.", false)(uri[start]) == true) {
      result_.is_valid = false;
      return;
    }
    ++start;
  }
  result_.scheme.assign(uri, 0, start);
}

/**
 * @brief (Absolute Form) URI 에서 hier-part 파싱
 *
 * @param uri 파싱할 URI
 * @param start 검증 시작 위치
 */
void UriParser::ValidateHierPart(std::string& uri, size_t& start) {
  if (result_.is_valid == false) {
    return;
  }
  if (start + 1 < uri.size() && uri[start] == '/' && uri[start + 1] == '/') {
    start += 2;
    ValidateAuthority(uri, start);
    if (start == uri.size()) {
      return;
    }
    if (uri[start] != '/') {
      result_.is_valid = false;
    }
    ValidatePath(uri, start);
    ValidateQuery(uri, start);
  } else {
    result_.is_valid = false;
  }
}

/**
 * @brief URI 에서 authority 파싱
 *
 * @param uri 파싱할 URI
 * @param start 검증 시작 위치
 */
void UriParser::ValidateAuthority(std::string& uri, size_t& start) {
  if (result_.is_valid == false) {
    return;
  }
  size_t pos = start;
  while (pos < uri.size() && IsCharSet("/:", false)(uri[pos]) == true &&
         result_.is_valid == true) {
    if (IsCharSet(UNRESERVED SUB_DELIMS, false)(uri[pos]) == true) {
      result_.is_valid = (uri[pos] == '%') ? DecodeHexToAscii(uri, pos) : false;
    }
    ++pos;
  }
  result_.is_valid = (result_.is_valid && (pos != start));
  if (result_.is_valid == false) {
    return;
  }
  result_.host.assign(uri, start, pos - start);
  if (uri[pos] == ':') {
    size_t pos_start = pos;
    while (++pos < uri.size() && IsCharSet(DIGIT, true)(uri[pos]) == true)
      ;
    result_.port.assign(uri, pos_start, pos - pos_start);
  }
  start = pos;
}
