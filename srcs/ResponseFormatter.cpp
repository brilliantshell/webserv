/**
 * @file ResponseFormatter.cpp
 * @author ghan, jiskim, yongjule
 * @brief Format HTTP response
 * @date 2022-10-18
 *
 * @copyright Copyright (c) 2022
 */

#include "ResponseFormatter.hpp"

std::string ResponseFormatter::Format(ResourceManager::Result& resource_result,
                                      uint8_t version, uint8_t allowed_methods,
                                      int keep_alive) {
  std::stringstream ss;
  ss << (version == HttpParser::kHttp1_1 ? "HTTP/1.1 " : "HTTP/1.0 ")
     << resource_result.status << " " << g_status_map[resource_result.status]
     << CRLF << "server: BrilliantServer/1.0" << CRLF
     << "date: " + FormatCurrentTime() << CRLF
     << "allow: " << FormatAllowedMethods(allowed_methods) << CRLF
     << "connection: "
     << ((keep_alive == HttpParser::kComplete) ? "keep-alive" : "close") << CRLF
     << "content-length: " << resource_result.content.size() << CRLF
     << "content-type: "
     << FormatContentType(resource_result.ext, resource_result.header) << CRLF;
  ResolveConflicts(resource_result.header);
  for (ResponseHeaderMap::const_iterator it = resource_result.header.begin();
       it != resource_result.header.end(); ++it) {
    ss << it->first << ": " << it->second << CRLF;
  }
  ss << CRLF << resource_result.content;
  return ss.str();
}

std::string ResponseFormatter::FormatCurrentTime(void) {
  time_t now = time(0);
  struct tm tstruct;
  char buf[80];
  tstruct = *gmtime(&now);
  strftime(buf, sizeof(buf), "%a, %d %b %Y %H:%M:%S GMT", &tstruct);
  return buf;
}

std::string ResponseFormatter::FormatAllowedMethods(uint8_t allowed_methods) {
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

std::string ResponseFormatter::FormatContentType(const std::string& ext,
                                                 ResponseHeaderMap& header) {
  std::string content_type;
  ResponseHeaderMap::iterator content_type_it = header.find("content-type");
  if (content_type_it != header.end()) {
    content_type = content_type_it->second;
    header.erase(content_type_it);
  } else {
    MimeMap::iterator mime_it = g_mime_map.find(ext);
    content_type = (mime_it != g_mime_map.end()) ? mime_it->second
                                                 : "application/octet-stream";
  }
  return content_type;
}

void ResponseFormatter::ResolveConflicts(ResponseHeaderMap& header) {
  std::string server_fields[5] = {"server", "date", "allow", "connection",
                                  "content-length"};
  for (size_t i = 0; i < 5; ++i) {
    ResponseHeaderMap::iterator it = header.find(server_fields[i]);
    if (it != header.end()) {
      header.erase(it);
    }
  }
}