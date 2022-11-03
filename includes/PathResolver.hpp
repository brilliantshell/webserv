/**
 * @file PathResolver.hpp
 * @author ghan, jiskim, yongjule
 * @brief Resolve path
 * @date 2022-10-08
 *
 * @copyright Copyright (c) 2022
 */

#ifndef INCLUDES_PATH_RESOLVER_HPP_
#define INCLUDES_PATH_RESOLVER_HPP_

#include "ParseUtils.hpp"
#include "Types.hpp"

class PathResolver {
 public:
  enum Purpose { kLocation = 0, kRedirectTo, kErrorPage, kHttpParser };

  enum Status { kFailure = 0, kDirectory, kFile };

  PathResolver(void);

  Status Resolve(std::string &path, Purpose = kLocation);
  const std::string &get_file_name(void) const;

 private:
  std::string file_name_;

  bool ReserveFileName(std::string &path, Purpose);
  bool NormalizeDirPath(std::string &path);
};

#endif  // INCLUDES_PATH_RESOLVER_HPP_
