/**
 * @file UriParser.hpp
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

class UriParser {
 public:
  struct Result {
    bool is_valid;
    std::string port;
    std::string scheme;
    std::string path;
    std::string query;
    std::string host;

    Result(void)
        : is_valid(true),
          port(""),
          scheme(""),
          path("/"),
          query(""),
          host("") {}
  };

  Result ParseTarget(std::string uri);
  bool ParseHost(std::string& uri);
  void EncodeAsciiToHex(std::string& path);
  bool DecodeHexToAscii(std::string& uri, const size_t pos);
  std::string GetFullPath(void);

 private:
  Result result_;

  void ValidatePath(std::string& uri, size_t& start);
  void ValidateQuery(std::string& uri, size_t& start);

  void ValidateScheme(std::string& uri, size_t& start);
  void ValidateHierPart(std::string& uri, size_t& start);

  void ValidateAuthority(std::string& uri, size_t& start);
};

#endif  // INCLUDES_URIPARSER_HPP_
