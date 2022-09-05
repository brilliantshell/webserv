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

#include <exception>
#include <string>

struct ServerBlock {
  const std::string host;
  const int port;
  const std::string error;

  ServerBlock(std::string host, int port, std::string error)
      : host(host), port(port), error(error) {}
};

struct IsNotHorizWhiteSpace {
  bool operator()(const char& c) { return (c != ' ') && (c != '\t'); }
};

class Validator {
 private:
  class InvalidConfigException : public std::exception {
    virtual const char* what() const throw() { return "syntax error"; }
  };

  typedef std::string::const_iterator ConstIterator_;

 public:
  ServerBlock Validate(const std::string& config);
};

#endif  // VALIDATOR_HPP
