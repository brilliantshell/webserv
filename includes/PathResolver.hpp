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
  enum {
    kConfigValidator = 0,
    kRouter,
  };

  PathResolver(void);

  bool Resolve(std::string &path, int purpose = PathResolver::kConfigValidator);

 private:
  std::string file_name_;

  bool ReserveFileName(std::string &path, int purpose);
  bool NormalizeDirPath(std::string &path);
};

#endif  // INCLUDES_PATH_RESOLVER_HPP_
