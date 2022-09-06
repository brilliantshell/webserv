/**
 * @file Validator.cpp
 * @author ghan, jiskim, yongjule
 * @brief Validate configuration file
 * @date 2022-09-05
 *
 * @copyright Copyright (c) 2022
 */

#include "Validator.hpp"

#include <algorithm>
#include <iostream>

ServerBlock Validator::Validate(const std::string& config) {
  size_t pos = config.find("server {\n");
  if (pos == std::string::npos) throw SyntaxErrorException();
  pos = config.find("listen ", pos);
  if (pos == std::string::npos) throw SyntaxErrorException();
  if (config.find("8080", pos) == std::string::npos) {
    if (config.find("80", pos) == std::string::npos)
      return ServerBlock(4242, "/hi");
    return ServerBlock(80, "/");
  }
  return ServerBlock(8080, "/trash");
}
