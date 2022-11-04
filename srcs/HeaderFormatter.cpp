/**
 * @file HeaderFormatter.cpp
 * @author ghan, jiskim, yongjule
 * @brief Format HTTP response header
 * @date 2022-10-18
 *
 * @copyright Copyright (c) 2022
 */

#include "HeaderFormatter.hpp"

/**
 * @brief Date 헤더 필드에 들어갈 값 설정, 현재 시간을 RFC 규격에 에 맞게
 * formatting
 *
 * @return std::string Formatting 된 현재 시간
 */
std::string HeaderFormatter::FormatCurrentTime(void) {
  time_t now = time(0);
  char buf[80];
  struct tm gmt_time = *gmtime(&now);
  strftime(buf, sizeof(buf), "%a, %d %b %Y %H:%M:%S GMT", &gmt_time);
  return buf;
}

/**
 * @brief Allowed 헤더 필드에 들어갈 값 설정, 라우팅 된 location 의 허가된
 * methods 출력
 *
 * @param allowed_methods 라우팅 된 location 에서 허용하는 methods
 * @return std::string GET, POST, DELETE 중 허가된 메소드 리스트
 */
std::string HeaderFormatter::FormatAllowed(uint8_t allowed_methods) {
  std::string methods;
  if (allowed_methods & GET) {
    methods += "GET";
  }
  if (allowed_methods & POST) {
    methods += (methods.size() > 0) ? ", POST" : "POST";
  }
  if (allowed_methods & DELETE) {
    methods += (methods.size() > 0) ? ", DELETE" : "DELETE";
  }
  return methods;
}

/**
 * @brief Content-Type 헤더 필드에 들어갈 값 설정, CGI 에서 설정된 값이 없다면
 * 파일 확장자에 따라 MIME 맵에서 찾아서 설정
 *
 * @param is_autoindex autoindex 여부
 * @param kExt 파일 extension
 * @param header CGI script 로 부터 받은 header fields
 * @return std::string 설정된 Content-Type
 */
std::string HeaderFormatter::FormatContentType(bool is_autoindex,
                                               const std::string& kExt,
                                               ResponseHeaderMap& header) {
  if (is_autoindex == true) {
    return "text/html;charset=utf-8";
  }
  std::string content_type = "";
  ResponseHeaderMap::iterator content_type_it = header.find("content-type");
  if (content_type_it != header.end()) {
    content_type = content_type_it->second;
    header.erase(content_type_it);
  } else {
    MimeMap::iterator mime_it = g_mime_map.find(kExt);
    content_type = (mime_it != g_mime_map.end()) ? mime_it->second : "";
  }
  return content_type;
}

/**
 * @brief CGI 와 충돌하는 헤더 필드 제거
 *
 * @param header 정리된 헤더 map
 */
void HeaderFormatter::ResolveConflicts(ResponseHeaderMap& header) {
  std::string server_fields[5] = {"server", "date", "allow", "connection",
                                  "content-length"};
  for (size_t i = 0; i < 5; ++i) {
    ResponseHeaderMap::iterator it = header.find(server_fields[i]);
    if (it != header.end()) {
      header.erase(it);
    }
  }
}
