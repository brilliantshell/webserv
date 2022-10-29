/**
 * @file ResponseManager.cpp
 * @author ghan, jiskim, yongjule
 * @brief Execute methods and manage resources according to the Client's request
 * @date 2022-10-11
 *
 * @copyright Copyright (c) 2022
 */

#include "ResponseManager.hpp"

ResponseManager::ResponseManager(int type, bool is_keep_alive,
                                 ResponseBuffer& response_buffer,
                                 Router::Result& router_result,
                                 const Request& request)
    : type_(type),
      io_status_(IO_START),
      is_keep_alive_(is_keep_alive),
      response_buffer_(response_buffer),
      router_result_(router_result),
      request_(request),
      result_(router_result.status) {}

void ResponseManager::FormatHeader(void) {
  std::stringstream ss;
  ss << (request_.req.version == HttpParser::kHttp1_1 ? "HTTP/1.1 "
                                                      : "HTTP/1.0 ")
     << result_.status << " " << g_status_map[result_.status] << CRLF
     << "server: BrilliantServer/1.0" << CRLF
     << "date: " + header_formatter_.FormatCurrentTime() << CRLF
     << ((result_.status == 301 || result_.status == 400 ||
          result_.status == 404 || result_.status >= 500)
             ? ""
             : ("allow: " +
                header_formatter_.FormatAllowedMethods(router_result_.methods) +
                CRLF))
     << "connection: "
     << ((result_.status < 500 && is_keep_alive_ == true) ? "keep-alive"
                                                          : "close")
     << CRLF << "content-length: " << response_buffer_.content.size() << CRLF;
  std::string content_type = header_formatter_.FormatContentType(
      result_.is_autoindex, result_.ext, result_.header);
  if (content_type.empty() == false) {
    ss << "content-type: " << content_type << CRLF;
  }
  if (result_.location.empty() == false) {  // 201 || 301 || 302
    ss << "location: " << result_.location << CRLF;
  }
  header_formatter_.ResolveConflicts(result_.header);
  for (ResponseHeaderMap::const_iterator it = result_.header.begin();
       it != result_.header.end(); ++it) {
    ss << it->first << ": " << it->second << CRLF;
  }
  ss << CRLF;
  response_buffer_.header = ss.str();
  response_buffer_.is_complete = true;
  std::cerr << "header: " << response_buffer_.header << std::endl;
}

int ResponseManager::get_status(void) const { return io_status_; }

bool ResponseManager::get_is_keep_alive(void) const { return is_keep_alive_; }

// SECTION: protected
// Utils
// Parse Extension for MIME type
std::string ResponseManager::ParseExtension(const std::string& path) {
  size_t last_slash = path.rfind('/');
  if (last_slash > path.size() - 3) {
    return "";
  }
  size_t last_dot = path.rfind('.');
  if (last_dot == std::string::npos || last_dot < last_slash ||
      last_dot == path.size() - 1) {
    return "";
  }
  return path.substr(last_dot + 1);
}

// Read error page
void ResponseManager::GetErrorPage(std::string& response_content,
                                   Result& result,
                                   Router::Result& router_result) {
  if (access(router_result.error_path.c_str(), F_OK) == -1) {
    std::stringstream ss;
    ss << result.status << " " << g_status_map[result.status];
    response_content = "<!DOCTYPE html><title>" + ss.str() +
                       "</title><body><h1>" + ss.str() + "</h1></body></html>";
    router_result.error_path = "default_error.html";
    return;
  }
  struct stat file_stat;
  if (stat(router_result.error_path.c_str(), &file_stat) == -1 ||
      (file_stat.st_mode & S_IFMT) == S_IFDIR) {
    result.status = 500;  // INTERNAL_SERVER_ERROR
    response_content = LAST_ERROR_DOCUMENT;
    return;
  }
  std::ifstream err_ifs(router_result.error_path);
  if (err_ifs.fail()) {   // bad - io operation error, fail - logical error
    result.status = 500;  // INTERNAL SERVER ERROR
    response_content = LAST_ERROR_DOCUMENT;
    return;
  }
  std::stringstream ss;
  err_ifs >> ss.rdbuf();
  response_content = ss.str();
}
