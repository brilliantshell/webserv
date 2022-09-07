/**
 * @file Validator.cpp
 * @author ghan, jiskim, yongjule
 * @brief Validate configuration file
 * @date 2022-09-05
 *
 * @copyright Copyright (c) 2022
 */

#include "Validator.hpp"

Validator::Validator(const std::string& config) : kConfig_(config) {
  key_set_.insert("listen");
  key_set_.insert("server_name");
  key_set_.insert("error");
  key_set_.insert("route");
  // key_set_.insert("root");
  // key_set_.insert("index");
  // key_set_.insert("autoindex");
  // key_set_.insert("method");
}

uint16_t Validator::TokenizePort(ConstIterator_ it,
                                 ConstIterator_& token_end) const {
  uint32_t port;
  token_end = std::find_if(it, kConfig_.end(), IsCharSet("0123456789", false));
  if (*token_end != '\n') {
    token_end =
        std::find_if(token_end, kConfig_.end(), IsCharSet(" \t", false));
  }
  if (token_end == kConfig_.end() || *token_end != '\n') {
    throw SyntaxErrorException();
  }
  std::stringstream ss;
  ss.str(std::string(it, token_end));
  ss >> port;
  if (port == 0 || port > 65535) {
    throw SyntaxErrorException();
  }
  return port;
}

std::string Validator::TokenizeRoutePath(ConstIterator_ it,
                                         ConstIterator_& token_end) const {
  token_end = std::find(it, kConfig_.end(), ' ');
  if (token_end == kConfig_.end()) {
    throw SyntaxErrorException();
  }
  return std::string(it, token_end);
}

std::string Validator::TokenizeSingleString(ConstIterator_ it,
                                            ConstIterator_& token_end) const {
  token_end = std::find_if(it, kConfig_.end(), IsCharSet(" \t\n", true));
  if (*token_end != '\n') {
    token_end =
        std::find_if(token_end, kConfig_.end(), IsCharSet(" \t", false));
  }
  if (token_end == kConfig_.end() || *token_end != '\n') {
    throw SyntaxErrorException();
  }
  return std::string(it, token_end);
}

ServerBlock Validator::ValidateServerBlock(ConstIterator_& it) {
  ServerBlock server_block;
  for (; it != kConfig_.end(); ++it) {
    it = std::find_if(it, kConfig_.end(), IsCharSet("\n\t ", false));
    if (*it == '}') {
      break;
    }
    ConstIterator_ token_end =
        std::find_if(it, kConfig_.end(), IsCharSet(" \t", true));
    KeyIt_ key_it = key_set_.find(std::string(it, token_end));
    if (key_it == key_set_.end()) {
      throw SyntaxErrorException();
    }
    it = std::find_if(it, kConfig_.end(), IsCharSet(" \t", true)) + 1;
    if (!(*key_it).compare("listen")) {
      key_set_.erase("listen");
      server_block.port = TokenizePort(it, token_end);
    } else if (!(*key_it).compare("route")) {
      server_block.route.path = TokenizeRoutePath(it, token_end);
      token_end = std::find(token_end, kConfig_.end(), '}');
      if (token_end == kConfig_.end()) {
        throw SyntaxErrorException();
      }
      ++token_end;
    } else {
      server_block[*key_it] = TokenizeSingleString(it, token_end);
    }
    it = token_end;
  }
  if (key_set_.count("listen")) {
    throw SyntaxErrorException();
  }
  return server_block;
}

ServerBlock Validator::Validate(void) {
  ConstIterator_ it =
      std::find_if(kConfig_.begin(), kConfig_.end(), IsCharSet("\n\t ", false));
  if (std::string(it, it + 8).compare("server {")) {
    throw SyntaxErrorException();
  }
  it += 8;
  it = std::find_if(it, kConfig_.end(), IsCharSet(" \t", false));
  if (it == kConfig_.end() || (*it != '\n' && *it != '{')) {
    throw SyntaxErrorException();
  }
  ServerBlock server_block = ValidateServerBlock(++it);
  if (kConfig_.find("}") == std::string::npos) {
    throw SyntaxErrorException();
  }

  return server_block;
}
