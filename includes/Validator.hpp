/**
 * @file Validator.hpp
 * @author ghan, jiskim, yongjule
 * @brief Validate configuration file
 * @date 2022-09-05
 *
 * @copyright Copyright (c) 2022
 */

#ifndef INCLUDES_VALIDATOR_HPP_
#define INCLUDES_VALIDATOR_HPP_

#include <algorithm>
#include <exception>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <string>

#include "Types.hpp"

struct RouteBlock {};

struct ServerBlock {
  uint16_t port;
  std::string server_name;
  std::string error;

  ServerBlock(void) : port(0), server_name("127.0.0.1"), error("error.html") {}

  std::string& operator[](const std::string& key) {
    if (key == "server_name") {
      return server_name;
    }
    return error;
  }
};

class Validator {
 private:
  struct CompareServerBlock {
    bool operator()(const ServerBlock& lhs, const ServerBlock& rhs) const {
      return lhs.port < rhs.port;
    }
  };

 public:
  typedef std::map<std::string, RouteBlock> RouteMap;
  typedef std::pair<std::string, RouteBlock> RouteNode;
  typedef std::map<ServerBlock, RouteMap, CompareServerBlock> ServerMap;
  typedef std::pair<ServerBlock, RouteMap> ServerNode;

  Validator(const std::string& config);
  ServerMap Validate(void);

  class SyntaxErrorException : public std::exception {
   public:
    virtual const char* what() const throw() { return "syntax error"; }
  };

 private:
  class IsCharSet {
   public:
    /**
     * @brief Construct a new Is Char Set object
     *
     * @param char_set : character set to find
     * @param is_true : true면 char_set을 만나면 return, false면 char_set이
     * 아닌 것을 만나면 return.
     */
    IsCharSet(const std::string char_set, const bool is_true)
        : kCharSet_(char_set), kIsTrue_(is_true) {}
    bool operator()(char c) const {
      return !((kCharSet_.find(c) != std::string::npos) ^ kIsTrue_);
    }

   private:
    const std::string kCharSet_;
    const bool kIsTrue_;
  };

  enum class ServerDirective {
    kListen = 0,
    kServerName,
    kError,
    kRoute,
  };

  typedef std::string::const_iterator ConstIterator_;
  typedef std::map<std::string, ServerDirective> ServerKeyMap_;
  typedef std::map<std::string, ServerDirective>::iterator ServerKeyIt_;

  const std::string kConfig_;

  void InitializeKeyMap(ServerKeyMap_& key_map) const;
  ServerNode ValidateServerBlock(ConstIterator_& it) const;
  RouteNode ValidateRouteBlock(ConstIterator_ it, ConstIterator_& token) const;
  ServerKeyIt_ FindDirectiveKey(ConstIterator_& it, ConstIterator_& token_end,
                                ServerKeyMap_& key_map) const;
  uint16_t TokenizePort(ConstIterator_ it, ConstIterator_& token_end) const;
  std::string TokenizeSingleString(ConstIterator_ it,
                                   ConstIterator_& token_end) const;

  std::string TokenizeRoutePath(ConstIterator_ it,
                                ConstIterator_& token_end) const;
  ConstIterator_ CheckEndOfParameter(ConstIterator_ token_end) const;
};

#endif  // INCLUDES_VALIDATOR_HPP_
