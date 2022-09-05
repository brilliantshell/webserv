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
  size_t pos = config.find("server");
  if (pos == std::string::npos) throw InvalidConfigException();
  pos = config.find("{", pos);
  if (pos == std::string::npos) throw InvalidConfigException();
  ConstIterator_ it = std::find_if(config.begin() + pos + 1, config.end(),
                                   IsNotHorizWhiteSpace());
  if (*it != '\n') throw InvalidConfigException();
  if (config.find("}", it - config.begin() + 1) == std::string::npos)
    throw InvalidConfigException();
  return ServerBlock(std::string(), int(), std::string());
}
