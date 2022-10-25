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
     << ((resource_result.status == 301 || resource_result.status == 400 ||
          resource_result.status == 404 || resource_result.status >= 500)
             ? ""
             : ("allow: " + FormatAllowedMethods(allowed_methods) + CRLF))
     << "connection: "
     << ((resource_result.status < 500 && keep_alive == HttpParser::kComplete)
             ? "keep-alive"
             : "close")
     << CRLF;

  if (resource_result.content.empty() == false) {
    ss << "content-length: " << resource_result.content.size() << CRLF;
  }
  std::string content_type =
      FormatContentType(resource_result.is_autoindex, resource_result.ext,
                        resource_result.header);
  if (content_type.empty() == false) {
    ss << "content-type: " << content_type << CRLF;
  } else {
    // ss << "content-type: text/plain;charset=utf-8" << CRLF;
  }
  if (resource_result.location.empty() == false) {  // 201 || 301 || 302
    ss << "location: " << resource_result.location << CRLF;
  }
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
  char buf[80];
  struct tm gmt_time = *gmtime(&now);
  strftime(buf, sizeof(buf), "%a, %d %b %Y %H:%M:%S GMT", &gmt_time);
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

std::string ResponseFormatter::FormatContentType(bool is_autoindex,
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
