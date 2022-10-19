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
    Router::Result& router_result, const Request& request) {
  Result result(router_result.status);
  if (router_result.status >= 400) {
    GetErrorPage(result, router_result);
    result.ext = ParseExtension(router_result.error_path);
  } else if (router_result.is_cgi == false) {
    HandleStaticRequest(result, router_result, request);
  } else if (router_result.is_cgi == true) {
    HandleCgiRequest(result, router_result, request.content);
  }
  if (result.status >= 400) {
    GetErrorPage(result, router_result);
    result.ext = ParseExtension(router_result.error_path);
  }
  return result;
}

// SECTION: private
// Static requests
void ResourceManager::HandleStaticRequest(Result& result,
                                          Router::Result& router_result,
                                          const Request& request) {
  if (router_result.status == 301) {
    result.location = router_result.redirect_to;
    result.content = GenerateRedirectPage(router_result.redirect_to);
    result.ext = "html";
    return;
  }
  switch (request.req.method & (GET | POST | DELETE)) {
    case GET:
      Get(result, router_result);
      break;
    case POST:
      Post(result, router_result, request.content);
      break;
    case DELETE:
      Delete(result, router_result);
      break;
    default:
      break;
  }
  result.ext = (result.status == 201)
                   ? "html"
                   : ParseExtension(router_result.success_path);
}

// GET
void ResourceManager::Get(Result& result, Router::Result& router_result) {
  CheckFileMode(result, router_result);
  if (result.status < 400 && result.content.empty() == false) {
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
    result.content = ss.str();
  }
}

void ResourceManager::GetErrorPage(Result& result,
                                   Router::Result& router_result) {
  if (access(router_result.error_path.c_str(), F_OK) == -1) {
    std::stringstream ss;
    ss << result.status << " " << g_status_map[result.status];
    result.content = "<!DOCTYPE html><title>" + ss.str() +
                     "</title><body><h1>" + ss.str() + "</h1></body></html>";
    router_result.error_path = "default_error.html";
    return;
  }
  struct stat file_stat;
  if (stat(router_result.error_path.c_str(), &file_stat) == -1 ||
      (file_stat.st_mode & S_IFMT) == S_IFDIR) {
    result.status = 500;  // INTERNAL_SERVER_ERROR
    result.content = LAST_ERROR_DOCUMENT;
    return;
  }
  std::ifstream err_ifs(router_result.error_path);
  if (err_ifs.fail()) {   // bad - io operation error, fail - logical error
    result.status = 500;  // INTERNAL SERVER ERROR
    result.content = LAST_ERROR_DOCUMENT;
    return;
  }
  std::stringstream ss;
  err_ifs >> ss.rdbuf();
  result.content = ss.str();
}

// POST
void ResourceManager::Post(Result& result, Router::Result& router_result,
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
    result.content =
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
void ResourceManager::Delete(Result& result, Router::Result& router_result) {
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
  result.content = router_result.success_path.substr(1) + " is removed!\r\n";
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

void ResourceManager::CheckFileMode(Result& result,
                                    Router::Result& router_result) {
  struct stat file_stat;
  errno = 0;
  if (stat(router_result.success_path.c_str(), &file_stat) == -1) {
    result.status = errno == ENOENT ? 404 : 500;  // INTERNAL_SERVER_ERROR
    return;
  }
  if ((file_stat.st_mode & S_IFMT) == S_IFDIR) {
    if (*(router_result.success_path.rbegin()) == '/') {
      GenerateAutoindex(result, router_result.success_path);
    } else {
      result.status = 404;  // PAGE NOT FOUND
    }
  }
}

void ResourceManager::GenerateAutoindex(Result& result,
                                        const std::string& path) {
  DIR* dir = opendir(path.c_str());
  if (dir == NULL) {
    result.status = 500;  // INTERNAL_SERVER_ERROR
    return;
  }
  std::string index_of = "Index of " + path.substr(1);
  result.content = "<!DOCTYPE html><html><title>" + index_of +
                   "</title><body><h1>" + index_of + "</h1><hr><pre>\n";
  errno = 0;
  std::vector<std::string> dir_name_vector;
  std::vector<std::string> file_name_vector;
  for (dirent* ent = readdir(dir); ent != NULL; ent = readdir(dir)) {
    if (ent->d_name[0] != '.') {
      std::string file_name(ent->d_name);
      if (ent->d_type == DT_DIR) {
        dir_name_vector.push_back(file_name + "/");
      } else {
        file_name_vector.push_back(file_name);
      }
    }
  }
  if (errno != 0) {
    result.status = 500;  // INTERNAL_SERVER_ERROR
  } else {
    ListAutoindexFiles(result.content, dir_name_vector);
    ListAutoindexFiles(result.content, file_name_vector);
    result.content += "</pre><hr></body></html>";
  }
  closedir(dir);
  result.is_autoindex = true;
}

void ResourceManager::ListAutoindexFiles(std::string& content,
                                         std::vector<std::string>& files) {
  std::sort(files.begin(), files.end());
  for (size_t i = 0; i < files.size(); ++i) {
    content += "<a href='./" + files[i] + "'>" + files[i] + "</a>\n";
  }
}

// CGI

void ResourceManager::HandleCgiRequest(Result& result,
                                       Router::Result& router_result,
                                       const std::string& request_content) {
  CgiManager cgiManager;
  CgiManager::Result cgi_result = cgiManager.Execute(
      router_result, result.header, request_content, result.status);
  if (result.status < 400) {
    result.status = cgi_result.status;
    result.content = cgi_result.content;
  } else {
    result.header.clear();
  }
  result.ext = ParseExtension(router_result.success_path);
}
