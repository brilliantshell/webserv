/**
 * @file Validator.cpp
 * @author ghan, jiskim, yongjule
 * @brief Validate configuration file
 * @date 2022-09-05
 *
 * @copyright Copyright (c) 2022
 */

#include "Validator.hpp"

Validator::Validator(const std::string& config) : config_(config) {
  key_set_.insert("listen");
  // key_set_.insert("server_name");
  // key_set_.insert("error");
  key_set_.insert("route");
  // key_set_.insert("root");
  // key_set_.insert("index");
  // key_set_.insert("autoindex");
  // key_set_.insert("method");
}

uint16_t Validator::TokenizePort_(ConstIterator_ it,
                                  ConstIterator_& token_end) const {
  uint16_t port;
  token_end = std::find(it, config_.end(), '\n');
  if (token_end == config_.end()) throw SyntaxErrorException();
  std::stringstream ss;
  ss.str(std::string(it, token_end));
  ss >> port;
  return port;
}

std::string Validator::TokenizeRoutePath_(ConstIterator_ it,
                                          ConstIterator_& token_end) const {
  token_end = std::find(it, config_.end(), ' ');
  if (token_end == config_.end()) throw SyntaxErrorException();
  return std::string(it, token_end);
}

ServerBlock Validator::ValidateServerBlock_(ConstIterator_& it) {
  ServerBlock server_block;
  for (; it != config_.end(); ++it) {
    it = std::find_if(it, config_.end(), IsNotWhiteSpace());
    if (*it == '}') break;
    ConstIterator_ token_end =
        std::find_if(it, config_.end(), IsHorizWhiteSpace());
    KeyIt_ key_it = key_set_.find(std::string(it, token_end));
    if (key_it == key_set_.end()) throw SyntaxErrorException();

    it = std::find_if(it, config_.end(), IsHorizWhiteSpace()) + 1;
    if (!(*key_it).compare("listen")) {
      server_block.port = TokenizePort_(it, token_end);
    } else if (!(*key_it).compare("route")) {
      server_block.route.path = TokenizeRoutePath_(it, token_end);
      token_end = std::find(token_end, config_.end(), '}');
      if (token_end == config_.end()) throw SyntaxErrorException();
      ++token_end;
    }
    it = token_end;
  }
  return server_block;
}

ServerBlock Validator::Validate(void) {
  ConstIterator_ it =
      std::find_if(config_.begin(), config_.end(), IsNotWhiteSpace());
  if (std::string(it, it + 8).compare("server {")) throw SyntaxErrorException();
  it += 8;
  it = std::find_if(it, config_.end(), IsNotHorizWhiteSpace());
  if (it == config_.end() || (*it != '\n' && *it != '{'))
    throw SyntaxErrorException();
  ServerBlock server_block = ValidateServerBlock_(++it);
  if (config_.find("}") == std::string::npos) throw SyntaxErrorException();
  return server_block;
}
