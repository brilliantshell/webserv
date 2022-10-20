/**
 * @file ResponseFormatter.hpp
 * @author ghan, jiskim, yongjule
 * @brief Format HTTP response
 * @date 2022-10-18
 *
 * @copyright Copyright (c) 2022
 */

#ifndef INCLUDES_RESPONSEFORMATTER_HPP_
#define INCLUDES_RESPONSEFORMATTER_HPP_

#include <sys/errno.h>

#include <sstream>

#include "HttpParser.hpp"
#include "ResourceManager.hpp"
#include "ResponseData.hpp"
#include "Types.hpp"

class ResponseFormatter {
 public:
  std::string Format(ResourceManager::Result& resource_result, uint8_t version,
                     uint8_t allowed_methods, int keep_alive);

 private:
  std::string FormatCurrentTime(void);
  std::string FormatAllowedMethods(uint8_t allowed_methods);
  std::string FormatContentType(bool is_autoindex, const std::string& ext,
                                ResponseHeaderMap& header);
  void ResolveConflicts(ResponseHeaderMap& header);
};

#endif  // INCLUDES_RESPONSEFORMATTER_HPP_
