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
#include <set>
#include <sstream>
#include <string>

#include "Types.hpp"

struct Route {
  std::string path;

  Route(void) {}

  Route(const std::string path) : path(path) {}
};

struct ServerBlock {
  uint16_t port;
  std::string server_name;
  std::string error;
  Route route;

  std::string& operator[](const std::string& key) {
    if (key == "server_name") {
      return server_name;
    }
    return error;
  }
};

class Validator {
 public:
  Validator(const std::string& config);
  ServerBlock Validate(void);

 private:
  class SyntaxErrorException : public std::exception {
   public:
    virtual const char* what() const throw() { return "syntax error"; }
  };

  class IsCharSet {
   public:
    IsCharSet(const std::string char_set, const bool is_true)
        : kCharSet_(char_set), kIsTrue_(is_true) {}
    bool operator()(char c) const {
      return !((kCharSet_.find(c) != std::string::npos) ^ kIsTrue_);
    }

   private:
    const std::string kCharSet_;
    const bool kIsTrue_;
  };

  typedef std::string::const_iterator ConstIterator_;
  typedef std::set<std::string>::iterator KeyIt_;

  std::set<std::string> key_set_;
  const std::string kConfig_;

  ServerBlock ValidateServerBlock(ConstIterator_& it);
  uint16_t TokenizePort(ConstIterator_ it, ConstIterator_& token_end) const;
  std::string TokenizeSingleString(ConstIterator_ it,
                                   ConstIterator_& token_end) const;

  std::string TokenizeRoutePath(ConstIterator_ it,
                                ConstIterator_& token_end) const;
};

#endif  // INCLUDES_VALIDATOR_HPP_
