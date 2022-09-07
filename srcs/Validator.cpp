/**
 * @file Validator.cpp
 * @author ghan, jiskim, yongjule
 * @brief Validate configuration file
 * @date 2022-09-05
 *
 * @copyright Copyright (c) 2022
 */

#include "Validator.hpp"

Validator::Validator(const std::string& config) : kConfig_(config) {}

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
  return "." + std::string(it, token_end);
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

void Validator::InitializeKeyMap(ServerKeyMap_& key_map) {
  key_map["listen"] = ServerDirective::kListen;
  key_map["server_name"] = ServerDirective::kServerName;
  key_map["error"] = ServerDirective::kError;
  key_map["route"] = ServerDirective::kRoute;
}

ServerBlock Validator::ValidateServerBlock(ConstIterator_& it) {
  ServerBlock server_block;
  ServerKeyMap_ key_map;

  InitializeKeyMap(key_map);
  for (; it != kConfig_.end(); ++it) {
    it = std::find_if(it, kConfig_.end(), IsCharSet("\n\t ", false));
    if (*it == '}') {
      break;
    }
    ConstIterator_ token_end =
        std::find_if(it, kConfig_.end(), IsCharSet(" \t", true));
    ServerKeyIt_ key_it = key_map.find(std::string(it, token_end));
    if (key_it == key_map.end()) {
      throw SyntaxErrorException();
    }
    it = std::find_if(it, kConfig_.end(), IsCharSet(" \t", true)) + 1;
    switch (key_it->second) {
      case ServerDirective::kListen:
        key_map.erase(key_it->first);
        server_block.port = TokenizePort(it, token_end);
        break;
      case ServerDirective::kRoute:
        server_block.route.path = TokenizeRoutePath(it, token_end);
        token_end = std::find(token_end, kConfig_.end(), '}');
        if (token_end == kConfig_.end()) {
          throw SyntaxErrorException();
        }
        ++token_end;
        break;
      case ServerDirective::kServerName:
      case ServerDirective::kError:
        key_map.erase(key_it->first);
        server_block[key_it->first] = TokenizeSingleString(it, token_end);
        break;
      default:
        throw SyntaxErrorException();
    }
    it = token_end;
  }

  // NOTE : listen 은 필수값!
  if (key_map.count("listen")) {
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
