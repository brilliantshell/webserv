/**
 * @file UriParser.cpp
 * @author ghan, jiskim, yongjule
 * @brief Validate URI according to RFC 3986
 * @date 2022-09-29
 *
 * @copyright Copyright (c) 2022
 */

#include "UriParser.hpp"

UriParser::Result UriParser::Parse(std::string uri) {
  size_t pos = 0;
  if (uri[0] == '/') {
    ValidatePath(uri, pos);
    ValidateQuery(uri, pos);
  } else if (isalpha(uri[0])) {
    ValidateScheme(uri, pos);
    ValidateHierPart(uri, ++pos);
  } else {
    result_.is_valid = false;
  }
  return result_;
}

// SECTION : private
bool UriParser::DecodeHexToAscii(std::string& uri, const size_t pos) {
  IsCharSet is_hexdig(HEXDIG, true);
  if (pos + 2 < uri.size() && is_hexdig(uri[pos + 1]) &&
      is_hexdig(uri[pos + 2])) {
    unsigned int x;
    std::stringstream ss;
    ss << std::hex << uri.substr(pos + 1, 2);
    ss >> x;
    uri.replace(pos, 3, 1, x);
    return true;
  }
  return false;
}

// Origin form 검증 및 파싱
void UriParser::ValidatePath(std::string& uri, size_t& start) {
  if (result_.is_valid == false) {
    return;
  }
  size_t pos = start;
  while (pos < uri.size() && uri[pos] != '?' && result_.is_valid) {
    if (IsCharSet(PCHAR "/", false)(uri[pos])) {
      result_.is_valid = (uri[pos] == '%') ? DecodeHexToAscii(uri, pos) : false;
    }
    ++pos;
  }
  if (result_.is_valid == true) {
    result_.path = uri.substr(start, pos - start);
  }
  start = pos;
}

void UriParser::ValidateQuery(std::string& uri, size_t& start) {
  if (result_.is_valid == false) {
    return;
  }
  size_t pos = start;
  while (pos < uri.size() && result_.is_valid) {
    if (IsCharSet(PCHAR "/?", false)(uri[pos])) {
      result_.is_valid = (uri[pos] == '%') ? DecodeHexToAscii(uri, pos) : false;
    }
    ++pos;
  }
  if (result_.is_valid == true) {
    result_.query = uri.substr(start);
  }
}

// Absolute form 검증 및 파싱
void UriParser::ValidateScheme(std::string& uri, size_t& start) {
  while (start < uri.size() && uri[start] != ':' && result_.is_valid) {
    if (IsCharSet(ALPHA DIGIT "+-.", false)(uri[start])) {
      result_.is_valid = false;
    }
    ++start;
  }
}

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

void UriParser::ValidateAuthority(std::string& uri, size_t& start) {
  if (result_.is_valid == false) {
    return;
  }
  size_t pos = start;
  while (pos < uri.size() && IsCharSet("/:", false)(uri[pos]) &&
         result_.is_valid) {
    if (IsCharSet(UNRESERVED SUB_DELIMS, false)(uri[pos])) {
      result_.is_valid = (uri[pos] == '%') ? DecodeHexToAscii(uri, pos) : false;
    }
    ++pos;
  }
  result_.is_valid = (pos != start);
  if (result_.is_valid == false) {
    return;
  }
  result_.host = uri.substr(start, pos - start);
  if (uri[pos] == ':') {
    while (++pos < uri.size() && IsCharSet(DIGIT, true)(uri[pos]))
      ;
  }
  start = pos;
}
