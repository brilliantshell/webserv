/**
 * @file URIParser.hpp
 * @author ghan, jiskim, yongjule
 * @brief Validate URI according to RFC 3986
 * @date 2022-09-29
 *
 * @copyright Copyright (c) 2022
 */

#ifndef INCLUDES_URIPARSER_HPP_
#define INCLUDES_URIPARSER_HPP_

#include <sstream>
#include <string>

#include "ParseUtils.hpp"

class URIParser {
 public:
  struct Result {
    bool is_valid;
    std::string path;
    std::string query;
    std::string host;

    Result(void) : is_valid(true), path("/") {}
  };

  Result Parse(std::string uri);

 private:
  Result result_;

  bool DecodeHexToAscii(std::string& uri, const size_t pos);

  void ValidatePath(std::string& uri, size_t& start);
  void ValidateQuery(std::string& uri, size_t& start);

  void ValidateScheme(std::string& uri, size_t& start);
  void ValidateHierPart(std::string& uri, size_t& start);
  void ValidateAuthority(std::string& uri, size_t& start);
};

#endif  // INCLUDES_URIPARSER_HPP_
