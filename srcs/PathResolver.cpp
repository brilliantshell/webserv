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

PathResolver::Status PathResolver::Resolve(std::string &path, Purpose purpose) {
  if (ReserveFileName(path, purpose) == false) {
    return kFailure;
  }
  if (NormalizeDirPath(path) == false) {
    return kFailure;
  }
  if (file_name_.size() > 0) {
    if (purpose != kHttpParser) {
      path += file_name_;
      file_name_.clear();
    }
    return kFile;
  }
  return kDirectory;
}

const std::string &PathResolver::get_file_name(void) const {
  return file_name_;
}

// SECTION : private
bool PathResolver::ReserveFileName(std::string &path, Purpose purpose) {
  if ((purpose == kErrorPage) && path[0] != '/') {
    path.insert(0, "/");
  }
  if (*path.rbegin() != '/') {
    if (purpose == kLocation ||
        (path.size() > 2 && path.compare(path.size() - 2, 2, "/.") == 0) ||
        (path.size() > 3 && path.compare(path.size() - 3, 3, "/..") == 0)) {
      path += '/';
    } else {
      size_t last_slash_pos = path.rfind('/');
      if (last_slash_pos == std::string::npos) {
        return false;
      }
      file_name_ = path.substr(last_slash_pos + 1);
      path.erase(last_slash_pos + 1);
    }
  }
  return !((purpose == kErrorPage) && file_name_.size() == 0);
}

bool PathResolver::NormalizeDirPath(std::string &path) {
  std::string valid_chars(path.begin(), path.end());
  std::string::iterator first_idx = valid_chars.begin();

  for (size_t i = 0; i < path.size();) {
    if (path[i] == '/') {
      size_t k = i + 1;
      for (; k < path.size(); ++k) {
        if (path[k] == '/') {
          valid_chars[k] = '\0';
        } else if (k + 1 < path.size() && path.compare(k, 2, "./") == 0) {
          std::fill_n(first_idx + k, 2, '\0');
          ++k;
        } else if (k + 2 < path.size() && path.compare(k, 3, "../") == 0) {
          if (i == 0) {
            return false;
          }
          size_t erase_start = valid_chars.rfind('/', i - 1);
          if (erase_start == std::string::npos) {
            return false;
          }
          std::fill_n(first_idx + erase_start, k + 2 - erase_start, '\0');
          k += 2;
        } else {
          break;
        }
      }
      i = k;
    } else {
      ++i;
    }
  }

  path.resize(valid_chars.size() -
              std::count(valid_chars.begin(), valid_chars.end(), '\0'));
  std::remove_copy(valid_chars.begin(), valid_chars.end(), path.begin(), '\0');
  return true;
}
