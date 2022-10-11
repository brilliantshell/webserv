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
    Router::Result& router_result) {
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
      Post(result, router_result);
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
      // TODO : autoindex
    } else {
      result.status = 404;  // PAGE NOT FOUND
    }
  }
}

void ResourceManager::Get(Result& result, Router::Result& router_result) {
  CheckFileMode(result, router_result);
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

void ResourceManager::Post(Result& result, Router::Result& router_result) {}

void ResourceManager::Delete(Result& result, Router::Result& router_result) {}
