/**
 * @file ResourceManager.hpp
 * @author ghan, jiskim, yongjule
 * @brief Execute methods and manage resources according to the Client's request
 * @date 2022-10-11
 *
 * @copyright Copyright (c) 2022
 */

#include "ResourceManager.hpp"

ResourceManager::Result ResourceManager::ExecuteMethod(
    std::string& response_content, Router::Result& router_result,
    const Request& request) {
  Result result(router_result.status);
  if (request.req.method == HEAD) {
    return result;
  }
  if (router_result.status >= 400) {
    GetErrorPage(response_content, result, router_result);
    result.ext = ParseExtension(router_result.error_path);
  } else if (router_result.is_cgi == false) {
    HandleStaticRequest(response_content, result, router_result, request);
  } else if (router_result.is_cgi == true) {
    HandleCgiRequest(response_content, result, router_result, request.content);
  }
  if (result.status >= 400) {
    GetErrorPage(response_content, result, router_result);
    result.ext = ParseExtension(router_result.error_path);
  }
  return result;
}

// SECTION: private
// Static requests
void ResourceManager::HandleStaticRequest(std::string& response_content,
                                          Result& result,
                                          Router::Result& router_result,
                                          const Request& request) {
  if (router_result.status == 301) {
    result.location = router_result.redirect_to;
    response_content = GenerateRedirectPage(router_result.redirect_to);
    result.ext = "html";
    return;
  }
  switch (request.req.method & (GET | POST | DELETE)) {
    case GET:
      Get(response_content, result, router_result);
      break;
    case POST:
      Post(response_content, result, router_result, request.content);
      break;
    case DELETE:
      Delete(response_content, result, router_result);
      break;
    default:
      break;
  }
  result.ext =
      (result.status == 201 ||
       ((request.req.method & DELETE) == DELETE && result.status == 200))
          ? "html"
          : ParseExtension(router_result.success_path);
}

// GET
void ResourceManager::Get(std::string& response_content, Result& result,
                          Router::Result& router_result) {
  CheckFileMode(response_content, result, router_result);
  if (result.status < 400 && response_content.empty() == false) {
    return;
  }
  errno = 0;
  std::ifstream ifs;
  if (result.status < 400) {
    ifs.open(router_result.success_path, std::ios::in);
    switch (errno) {
      case EACCES:
        result.status = 403;  // FORBIDDEN
        break;
      case ENOENT:
        result.status = 404;  // PAGE NOT FOUND
        break;
      case EMFILE:
        result.status = 503;  // SERVICE UNAVAILABLE
        break;
      default:
        if (ifs.fail()) {  // bad - io operation error, fail - logical error
          result.status = 500;  // INTERNAL SERVER ERROR
        }
        break;
    }
    std::stringstream ss;
    ifs >> ss.rdbuf();
    response_content = ss.str();
  }
}

void ResourceManager::GetErrorPage(std::string& response_content,
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

// POST
void ResourceManager::Post(std::string& response_content, Result& result,
                           Router::Result& router_result,
                           const std::string& request_content) {
  std::string output_path =
      FindValidOutputPath(result, router_result.success_path);
  if (output_path.empty()) {  // 403 or 500
    return;
  }
  errno = 0;
  std::ofstream ofs(output_path);  // fail -> failbit
  if (errno == EACCES) {
    result.status = 403;  // FORBIDDEN
  } else {
    if (ofs.fail()) {       // bad - io operation error, fail - logical error
      result.status = 500;  // INTERNAL SERVER ERROR
      return;
    }
    ofs << request_content;
    result.status = 201;  // CREATED
    response_content =
        "<!DOCTYPE html><html><title>201 Created</title><body><h1>201 "
        "Created</h1><p>YAY! The file is created at " +
        output_path.substr(1) + "!</p><p>Have a nice day~</p></body></html>";
    result.location = output_path.substr(1);
  }
}

std::string ResourceManager::FindValidOutputPath(Result& result,
                                                 std::string& success_path) {
  size_t ext_start = success_path.rfind('.');
  std::string name = success_path;
  std::string ext("");
  if (ext_start > 0 && ext_start < name.size() - 1) {
    name = success_path.substr(0, ext_start);
    ext = success_path.substr(ext_start);
  }
  std::string output_path = success_path;
  int i = 0;
  for (; i < 100; ++i) {
    if (access(output_path.c_str(), F_OK) == -1) {
      if (errno != ENOENT) {
        result.status = 500;  // INTERNAL_SERVER_ERROR
        return "";
      }
      break;
    }
    std::stringstream ss;
    ss << name << "_" << i << ext;
    output_path = ss.str();
  }
  if (i == 100) {
    result.status = 403;  // FORBIDDEN
    return "";
  }
  return output_path;
}

// DELETE
void ResourceManager::Delete(std::string& response_content, Result& result,
                             Router::Result& router_result) {
  errno = 0;
  if (access(router_result.success_path.c_str(), W_OK) == -1) {
    if (errno == ENOENT) {
      result.status = 404;  // PAGE NOT FOUND
    } else if (errno == EACCES) {
      result.status = 403;  // FORBIDDEN
    } else {
      result.status = 500;  // INTERNAL_SERVER_ERROR
    }
    return;
  }
  if (unlink(router_result.success_path.c_str()) == -1) {
    result.status = 500;  // INTERNAL_SERVER_ERROR
    return;
  }
  result.status = 200;  // OK
  response_content =
      "<!DOCTYPE html><html><title>Deleted</title><body><h1>200 OK</h1><p>" +
      router_result.success_path.substr(1) + " is removed!</p></body></html>";
}

// Utils
// Parse Extension for MIME type
std::string ResourceManager::ParseExtension(const std::string& path) {
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

// Generate Redirection Message
std::string ResourceManager::GenerateRedirectPage(
    const std::string& redirect_to) {
  std::string href;
  return "<!DOCTYPE html><html><title></title><body><h1>301 Moved "
         "Permanently</h1><p>The resource has been moved permanently to <a "
         "href='" +
         redirect_to + "'>" + redirect_to + "<a>.</p></body></html>";
}

void ResourceManager::CheckFileMode(std::string& response_content,
                                    Result& result,
                                    Router::Result& router_result) {
  struct stat file_stat;
  errno = 0;
  if (stat(router_result.success_path.c_str(), &file_stat) == -1) {
    result.status = errno == ENOENT ? 404 : 500;  // INTERNAL_SERVER_ERROR
    return;
  }
  if ((file_stat.st_mode & S_IFMT) == S_IFDIR) {
    if (*(router_result.success_path.rbegin()) == '/') {
      GenerateAutoindex(response_content, result, router_result.success_path);
    } else {
      result.status = 404;  // PAGE NOT FOUND
    }
  }
}

void ResourceManager::GenerateAutoindex(std::string& response_content,
                                        Result& result,
                                        const std::string& path) {
  DIR* dir = opendir(path.c_str());
  if (dir == NULL) {
    result.status = 500;  // INTERNAL_SERVER_ERROR
    return;
  }
  std::string index_of = "Index of " + path.substr(1);
  response_content = "<!DOCTYPE html><html><title>" + index_of +
                     "</title><body><h1>" + index_of + "</h1><hr><pre>\n";
  errno = 0;
  std::vector<std::string> dir_vector;
  std::vector<std::string> file_vector;
  for (dirent* ent = readdir(dir); ent != NULL; ent = readdir(dir)) {
    if (ent->d_name[0] != '.' &&
        DetermineFileType(path, ent, dir_vector, file_vector) == false) {
      break;
    }
  }
  if (errno != 0) {
    result.status = 500;  // INTERNAL_SERVER_ERROR
  } else {
    ListAutoindexFiles(response_content, dir_vector);
    ListAutoindexFiles(response_content, file_vector);
    response_content += "</pre><hr></body></html>";
  }
  closedir(dir);
  result.is_autoindex = true;
}

bool ResourceManager::DetermineFileType(const std::string& path,
                                        const dirent* ent,
                                        std::vector<std::string>& dir_vector,
                                        std::vector<std::string>& file_vector) {
  std::string file_name(ent->d_name);
  struct stat s_buf;
  memset(&s_buf, 0, sizeof(s_buf));
  if (ent->d_type == DT_LNK) {
    if (stat((path + file_name).c_str(), &s_buf) == -1) {
      return false;
    }
    S_ISDIR(s_buf.st_mode) ? dir_vector.push_back(file_name + "/")
                           : file_vector.push_back(file_name);
  } else {
    (ent->d_type == DT_DIR) ? dir_vector.push_back(file_name + "/")
                            : file_vector.push_back(file_name);
  }
  return true;
}

void ResourceManager::ListAutoindexFiles(std::string& content,
                                         std::vector<std::string>& paths) {
  std::sort(paths.begin(), paths.end());
  for (size_t i = 0; i < paths.size(); ++i) {
    std::string encoded_path(paths[i]);
    UriParser().EncodeAsciiToHex(encoded_path);
    content += "<a href='./" + encoded_path + "'>" + paths[i] + "</a>\n";
  }
}

// CGI
void ResourceManager::HandleCgiRequest(std::string& response_content,
                                       Result& result,
                                       Router::Result& router_result,
                                       const std::string& request_content) {
  CgiManager::Result cgi_result =
      CgiManager().Execute(response_content, router_result, result.header,
                           request_content, result.status);
  if (result.status < 400) {
    result.status = cgi_result.status;
    result.is_local_redir = cgi_result.is_local_redir;
  } else {
    result.header.clear();
  }
  result.ext = ParseExtension(router_result.success_path);
}
