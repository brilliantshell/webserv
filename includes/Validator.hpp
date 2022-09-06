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

#include <cstdint>
#include <exception>
#include <string>

struct Route {
  const std::string path;

  Route(const std::string path) : path(path) {}
};

struct ServerBlock {
  const uint16_t port;
  const std::string host;
  const std::string error;
  const Route route;

  ServerBlock(uint16_t port, std::string route_path,
              std::string host = "127.0.0.1", std::string error = "error.html")
      : port(port), host(host), error(error), route(route_path) {}
};

class Validator {
 public:
  ServerBlock Validate(const std::string& config);

 private:
  class SyntaxErrorException : public std::exception {
   public:
    virtual const char* what() const throw() { return "syntax error"; }
  };
};

#endif  // VALIDATOR_HPP
