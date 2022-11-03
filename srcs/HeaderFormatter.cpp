/**
 * @file HeaderFormatter.cpp
 * @author ghan, jiskim, yongjule
 * @brief Format HTTP response
 * @date 2022-10-18
 *
 * @copyright Copyright (c) 2022
 */

#include "HeaderFormatter.hpp"

std::string HeaderFormatter::FormatCurrentTime(void) {
  time_t now = time(0);
  char buf[80];
  struct tm gmt_time = *gmtime(&now);
  strftime(buf, sizeof(buf), "%a, %d %b %Y %H:%M:%S GMT", &gmt_time);
  return buf;
}

std::string HeaderFormatter::FormatAllowedMethods(uint8_t allowed_methods) {
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

std::string HeaderFormatter::FormatContentType(bool is_autoindex,
                                               const std::string& ext,
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
    MimeMap::iterator mime_it = g_mime_map.find(ext);
    content_type = (mime_it != g_mime_map.end()) ? mime_it->second : "";
  }
  return content_type;
}

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
