/**
 * @file PathResolver.cpp
 * @author ghan, jiskim, yongjule
 * @brief Resolve path
 * @date 2022-10-08
 *
 * @copyright Copyright (c) 2022
 */

#include "PathResolver.hpp"

PathResolver::PathResolver(void) : file_name_("") {}

bool PathResolver::Resolve(std::string &path, int purpose) {
  if (ReserveFileName(path, purpose) == false) {
    return false;
  }
  if (NormalizeDirPath(path) == false) {
    return false;
  }
  if (file_name_.size() > 0) {
    path += file_name_;
    file_name_.clear();
  }
  return true;
}

bool PathResolver::ReserveFileName(std::string &path, int purpose) {
  if (path[path.size() - 1] != '/') {
    size_t not_dot = path.find_last_not_of(".");
    if (purpose == kConfigValidator || not_dot == path.size() - 2 ||
        not_dot == path.size() - 3) {
      path += '/';
    } else {
      size_t last_slash_pos = path.rfind('/');
      if (last_slash_pos == std::string::npos) {
        return false;  // NOTE : cgi 일 수 있는데 판별 어떻게?
      }
      file_name_ = path.substr(last_slash_pos + 1);
      path.erase(last_slash_pos + 1);
    }
  }
  return true;
}

bool PathResolver::NormalizeDirPath(std::string &path) {
  std::string valid_chars(path.begin(), path.end());
  std::string::iterator first_idx = valid_chars.begin();

  for (size_t i = 0; i < path.size();) {
    if (path[i] == '/') {
      size_t k = 1;
      for (; i + k < path.size(); ++k) {
        if (path[i + k] == '/') {
          valid_chars[i + k] = '\0';
        } else if (i + k + 1 < path.size() &&
                   path.compare(i + k, 2, "./") == 0) {
          std::fill_n(first_idx + i + k, 2, '\0');
          ++k;
        } else if (i + k + 2 < path.size() &&
                   path.compare(i + k, 3, "../") == 0) {
          if (i == 0) {
            return false;
          }
          size_t erase_start = valid_chars.rfind('/', i - 1);
          if (erase_start == std::string::npos) {
            return false;
          }
          std::fill_n(first_idx + erase_start, i + k + 2 - erase_start, '\0');
          k += 2;
        } else {
          break;
        }
      }
      i += k;
    } else {
      ++i;
    }
  }

  path.resize(valid_chars.size() -
              std::count(valid_chars.begin(), valid_chars.end(), '\0'));
  std::remove_copy(valid_chars.begin(), valid_chars.end(), path.begin(), '\0');
  return true;
}
