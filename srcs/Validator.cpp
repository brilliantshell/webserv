/**
 * @file Validator.cpp
 * @author ghan, jiskim, yongjule
 * @brief Validate configuration file
 * @date 2022-09-05
 *
 * @copyright Copyright (c) 2022
 */

#include "Validator.hpp"

ServerBlock Validator::Validate(const std::string& config) {
  if (config.find("server") == std::string::npos)
    throw InvalidConfigException();
  return ServerBlock("example.com", 80, "error.html");
}
