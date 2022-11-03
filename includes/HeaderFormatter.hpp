/**
 * @file HeaderFormatter.hpp
 * @author ghan, jiskim, yongjule
 * @brief Format HTTP response
 * @date 2022-10-18
 *
 * @copyright Copyright (c) 2022
 */

#ifndef INCLUDES_HEADERFORMATTER_HPP_
#define INCLUDES_HEADERFORMATTER_HPP_

#include <sys/errno.h>

#include <sstream>

#include "HttpParser.hpp"
#include "ResponseData.hpp"
#include "Types.hpp"

class HeaderFormatter {
 public:
  std::string FormatCurrentTime(void);
  std::string FormatAllowedMethods(uint8_t allowed_methods);
  std::string FormatContentType(bool is_autoindex, const std::string& kExt,
                                ResponseHeaderMap& header);
  void ResolveConflicts(ResponseHeaderMap& header);
};

#endif  // INCLUDES_HEADERFORMATTER_HPP_
