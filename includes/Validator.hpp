/**
 * @file Validator.hpp
 * @author ghan, jiskim, yongjule
 * @brief Validate configuration file
 * @date 2022-09-05
 *
 * @copyright Copyright (c) 2022
 */

#ifndef VALIDATOR_HPP
#define VALIDATOR_HPP

#include <algorithm>
#include <cstdint>
#include <exception>
#include <iostream>
#include <set>
#include <sstream>
#include <string>

struct Route {
  std::string path;

  Route(void) {}

  Route(const std::string path) : path(path) {}
};

struct ServerBlock {
  uint16_t port;
  std::string host;
  std::string error;
  Route route;

  ServerBlock(void) {}

  ServerBlock(uint16_t port, std::string route_path,
              std::string host = "127.0.0.1", std::string error = "error.html")
      : port(port), host(host), error(error), route(route_path) {}
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

  struct IsNotWhiteSpace {
    bool operator()(char c) { return (c != ' ') && (c != '\t') && (c != '\n'); }
  };

  struct IsNotHorizWhiteSpace {
    bool operator()(char c) { return (c != ' ') && (c != '\t'); }
  };

  struct IsHorizWhiteSpace {
    bool operator()(char c) { return (c == ' ') || (c == '\t'); }
  };

  typedef std::string::const_iterator ConstIterator_;
  typedef std::set<std::string>::iterator KeyIt_;

  std::set<std::string> key_set_;
  const std::string config_;

  ServerBlock ValidateServerBlock_(ConstIterator_& it);
  uint16_t TokenizePort_(ConstIterator_ it, ConstIterator_& token_end) const;
  std::string TokenizeRoutePath_(ConstIterator_ it,
                                 ConstIterator_& token_end) const;
};

#endif  // VALIDATOR_HPP
