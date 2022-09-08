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

void Validator::InitializeKeyMap(ServerKeyMap_& key_map) const {
  key_map["listen"] = ServerDirective::kListen;
  key_map["server_name"] = ServerDirective::kServerName;
  key_map["error"] = ServerDirective::kError;
  key_map["route"] = ServerDirective::kRoute;
}

Validator::RouteNode Validator::ValidateRouteBlock(
    ConstIterator_ it, ConstIterator_& token_end) const {
  RouteBlock route_block;
  std::string path = TokenizeRoutePath(it, token_end);
  token_end = std::find(token_end, kConfig_.end(), '}');
  if (token_end == kConfig_.end()) {
    throw SyntaxErrorException();
  }
  ++token_end;
  return RouteNode(path, route_block);
}

Validator::ServerKeyIt_ Validator::FindDirectiveKey(
    ConstIterator_& it, ConstIterator_& token_end,
    ServerKeyMap_& key_map) const {
  it = std::find_if(it, kConfig_.end(), IsCharSet(" \n\t", false));
  if (*it == '}') {
    return key_map.end();
  }
  token_end = std::find_if(it, kConfig_.end(), IsCharSet(" \t", true));
  ServerKeyIt_ key_it = key_map.find(std::string(it, token_end));
  if (key_it == key_map.end()) {
    throw SyntaxErrorException();
  }
  it = std::find_if(it, kConfig_.end(), IsCharSet(" \t", true)) + 1;
  return key_it;
}

Validator::ServerNode Validator::ValidateServerBlock(ConstIterator_& it) const {
  ServerBlock server_block;
  RouteMap route_map;
  ServerKeyMap_ key_map;
  ConstIterator_ token_end;

  InitializeKeyMap(key_map);
  for (; it != kConfig_.end(); ++it) {
    ServerKeyIt_ key_it = FindDirectiveKey(it, token_end, key_map);
    if (key_it == key_map.end()) {
      break;
    }
    switch (key_it->second) {
      case ServerDirective::kListen:
        server_block.port = TokenizePort(it, token_end);
        key_map.erase(key_it->first);
        break;
      case ServerDirective::kRoute:
        if (!route_map.insert(ValidateRouteBlock(it, token_end)).second) {
          throw SyntaxErrorException();
        }
        break;
      case ServerDirective::kServerName:
      case ServerDirective::kError:
        server_block[key_it->first] = TokenizeSingleString(it, token_end);
        key_map.erase(key_it->first);
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
  return ServerNode(server_block, route_map);
}

Validator::ServerMap Validator::Validate(void) {
  ServerMap server_map;

  for (ConstIterator_ it = std::find_if(kConfig_.begin(), kConfig_.end(),
                                        IsCharSet(" \n\t", false));
       it != kConfig_.end();) {
    if (std::string(it, it + 8).compare("server {")) {
      throw SyntaxErrorException();
    }
    it += 8;
    it = std::find_if(it, kConfig_.end(), IsCharSet(" \t", false));
    if (it == kConfig_.end() || (*it != '\n' && *it != '{')) {
      throw SyntaxErrorException();
    }
    server_map.insert(ValidateServerBlock(++it));
    it = std::find_if(++it, kConfig_.end(), IsCharSet(" \n\t", false));
  }
  return server_map;
}
