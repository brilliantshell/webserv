/**
 * @file UriParser.cpp
 * @author ghan, jiskim, yongjule
 * @brief Validate URI according to RFC 3986
 * @date 2022-09-29
 *
 * @copyright Copyright (c) 2022
 */

#include "UriParser.hpp"

UriParser::Result UriParser::ParseTarget(std::string uri) {
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

bool UriParser::ParseHost(std::string& uri) {
  size_t pos = 0;
  ValidateAuthority(uri, pos);
  std::transform(uri.begin(), uri.end(), uri.begin(), ::tolower);
  return result_.is_valid;
}

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

// RESERVED n cycle
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

std::string UriParser::GetFullPath(void) {
  if (result_.scheme.empty() == true) {
    return result_.path + result_.query;
  } else {
    return result_.scheme + "://" + result_.host + result_.port + result_.path +
           result_.query;
  }
}

// SECTION : private
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
  if (result_.is_valid == false || start >= uri.size()) {
    return;
  }
  size_t pos = start;
  while (pos < uri.size() && result_.is_valid) {
    if (IsCharSet(PCHAR "/?", false)(uri[pos])) {
      if (uri[pos] == '%' && uri.size() > pos + 2) {
        result_.is_valid = IsCharSet(HEXDIG, true)(uri[pos + 1]) &&
                           IsCharSet(HEXDIG, true)(uri[pos + 2]);
        pos += 2;
      } else {
        result_.is_valid = false;
      }
    }
    ++pos;
  }
  if (result_.is_valid == true) {
    result_.query = uri.substr(start + 1);
  }
}

// Absolute form 검증 및 파싱
void UriParser::ValidateScheme(std::string& uri, size_t& start) {
  while (start < uri.size() && uri[start] != ':' && result_.is_valid) {
    if (IsCharSet(ALPHA DIGIT "+-.", false)(uri[start])) {
      result_.is_valid = false;
      return;
    }
    ++start;
  }
  result_.scheme = uri.substr(0, start);
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
  result_.is_valid = result_.is_valid ? (pos != start) : false;
  if (result_.is_valid == false) {
    return;
  }
  result_.host = uri.substr(start, pos - start);
  if (uri[pos] == ':') {
    size_t pos_start = pos;
    while (++pos < uri.size() && IsCharSet(DIGIT, true)(uri[pos]))
      ;
    result_.port = uri.substr(pos_start, pos - pos_start);
  }
  start = pos;
}
