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
    Router::Result& router_result, const std::string& request_content) {
  Result result;
  result.status = router_result.status;

  if (router_result.status >= 400) {
    GetErrorPage(result, router_result);
    return result;
  }

  switch (router_result.method & (GET | POST | DELETE)) {
    case GET:
      Get(result, router_result);
      break;
    case POST:
      Post(result, router_result, request_content);
      break;
    case DELETE:
      Delete(result, router_result);
      break;
    default:
      break;
  }
  return result;
}

// private
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
      default: {
        if (ifs.fail()) {  // bad - io operation error, fail - logical error
          result.status = 500;  // INTERNAL SERVER ERROR
        }
        break;
      }
    }
  }
  if (result.status >= 400) {
    GetErrorPage(result, router_result);
    return;
  }
  std::stringstream ss;
  ifs >> ss.rdbuf();
  result.content = ss.str();
}

void ResourceManager::GetErrorPage(Result& result,
                                   Router::Result& router_result) {
  struct stat file_stat;
  if (stat(router_result.error_path.c_str(), &file_stat) == -1 ||
      (file_stat.st_mode & S_IFMT) == S_IFDIR) {
    result.status = 500;  // INTERNAL_SERVER_ERROR
    return;
  }
  std::ifstream err_ifs(router_result.error_path);
  if (err_ifs.fail()) {   // bad - io operation error, fail - logical error
    result.status = 500;  // INTERNAL SERVER ERROR
    return;
  }
  std::stringstream ss;
  err_ifs >> ss.rdbuf();
  result.content = ss.str();
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
}

void ResourceManager::ListAutoindexFiles(std::string& content,
                                         std::vector<std::string>& files) {
  std::sort(files.begin(), files.end());
  for (size_t i = 0; i < files.size(); ++i) {
    content += "<a href='./" + files[i] + "'>" + files[i] + "</a>\n";
  }
}

// POST
void ResourceManager::Post(Result& result, Router::Result& router_result,
                           const std::string& request_content) {
  size_t ext_start = router_result.success_path.rfind('.');
  std::string name = router_result.success_path;
  std::string ext("");
  if (ext_start > 0 && ext_start < name.size() - 1) {
    name = router_result.success_path.substr(0, ext_start);
    ext = router_result.success_path.substr(ext_start);
  }

  result.location = router_result.success_path;
  int i = 0;
  for (; i < 100; ++i) {
    errno = 0;
    if (access(result.location.c_str(), F_OK) == -1) {
      if (errno != ENOENT) {
        result.status = 500;  // INTERNAL_SERVER_ERROR
        return;
      }
      break;
    }
    std::stringstream ss;
    ss << name << "_" << i << ext;
    result.location = ss.str();
  }
  if (i == 100) {
    result.status = 403;  // INTERNAL_SERVER_ERROR
    return;
  }

  errno = 0;
  std::ofstream ofs(result.location);  // fail -> failbit
  if (errno == EACCES) {
    result.status = 403;  // FORBIDDEN
  } else {
    if (ofs.fail()) {       // bad - io operation error, fail - logical error
      result.status = 500;  // INTERNAL SERVER ERROR
      return;
    }
    ofs << request_content;
    result.status = 201;  // CREATED
    result.content = "YAY! The file is created at " +
                     result.location.substr(1) + "!\r\nHave a nice day~";
  }
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
