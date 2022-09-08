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

Validator::ConstIterator_ Validator::CheckEndOfParameter(
    ConstIterator_ token_end) const {
  if (*token_end != '\n') {
    token_end =
        std::find_if(token_end, kConfig_.end(), IsCharSet(" \t", false));
  }
  if (token_end == kConfig_.end() || *token_end != '\n') {
    throw SyntaxErrorException();
  }
  return token_end;
}

uint16_t Validator::TokenizePort(ConstIterator_ it,
                                 ConstIterator_& token_end) const {
  uint32_t port;
  token_end = std::find_if(it, kConfig_.end(), IsCharSet("0123456789", false));
  token_end = CheckEndOfParameter(token_end);
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
  if (token_end == kConfig_.end() ||
      (*it == '.' &&
       ((token_end - it) == 1 || std::find(it, token_end, '/') != token_end))) {
    throw SyntaxErrorException();
  }
  return (*it == '.') ? std::string(it, token_end)
                      : "." + std::string(it, token_end);
}

std::string Validator::TokenizeSingleString(ConstIterator_ it,
                                            ConstIterator_& token_end) const {
  token_end = std::find_if(it, kConfig_.end(), IsCharSet(" \t\n", true));
  return std::string(it, CheckEndOfParameter(token_end));
}

uint8_t Validator::TokenizeMethods(ConstIterator_ it, ConstIterator_& token_end,
                                   ServerDirective is_cgi) const {
  std::string method;
  uint8_t flag = 0;
  for (; it != kConfig_.end() && *it != '\n';
       it = std::find_if(token_end, kConfig_.end(), IsCharSet(" \t", false))) {
    token_end = std::find_if(it, kConfig_.end(), IsCharSet(" \t\n", true));
    method = std::string(it, token_end);
    if (method == "GET" && !(GET & flag)) {
      flag |= GET;
    } else if (method == "POST" && !(POST & flag)) {
      flag |= POST;
    } else if (is_cgi == ServerDirective::kRoute && method == "DELETE" &&
               !(DELETE & flag)) {
      flag |= DELETE;
    } else {
      throw SyntaxErrorException();
    }
  }
  return flag;
}

void Validator::InitializeKeyMap(ServerKeyMap_& key_map) const {
  key_map["listen"] = ServerDirective::kListen;
  key_map["server_name"] = ServerDirective::kServerName;
  key_map["error"] = ServerDirective::kError;
  key_map["route"] = ServerDirective::kRoute;
  key_map["cgi_route"] = ServerDirective::kCgiRoute;
}

void Validator::InitializeKeyMap(RouteKeyMap_& key_map,
                                 ServerDirective is_cgi) const {
  if (is_cgi == ServerDirective::kRoute) {
    key_map["autoindex"] = RouteDirective::kAutoindex;
    key_map["redirect_to"] = RouteDirective::kRedirectTo;
  } else {
    key_map["param"] = RouteDirective::kParam;
  }
  key_map["methods"] = RouteDirective::kMethods;
  key_map["body_max"] = RouteDirective::kBodyMax;
  key_map["root"] = RouteDirective::kRoot;
  key_map["index"] = RouteDirective::kIndex;
  key_map["upload_path"] = RouteDirective::kUploadPath;
}

Validator::ServerKeyIt_ Validator::FindDirectiveKey(
    ConstIterator_& it, ConstIterator_& token_end,
    ServerKeyMap_& key_map) const {
  it = std::find_if(it, kConfig_.end(), IsCharSet(" \n\t", false));
  if (*it == '}') {
    return key_map.end();  // NOTE : ServerBlock loop break 지점
  }
  token_end = std::find_if(it, kConfig_.end(), IsCharSet(" \t", true));
  ServerKeyIt_ key_it = key_map.find(std::string(it, token_end));
  if (key_it == key_map.end()) {
    throw SyntaxErrorException();
  }
  it = std::find_if(it, kConfig_.end(), IsCharSet(" \t", true)) + 1;
  return key_it;
}

Validator::RouteKeyIt_ Validator::FindDirectiveKey(
    ConstIterator_& it, ConstIterator_& token_end,
    RouteKeyMap_& key_map) const {
  it = std::find_if(it, kConfig_.end(), IsCharSet(" \n\t", false));
  if (*it == '}') {
    return key_map.end();  // NOTE : ServerBlock loop break 지점
  }
  token_end = std::find_if(it, kConfig_.end(), IsCharSet(" \t", true));
  RouteKeyIt_ key_it = key_map.find(std::string(it, token_end));
  if (key_it == key_map.end()) {
    throw SyntaxErrorException();
  }
  it = std::find_if(it, kConfig_.end(), IsCharSet(" \t", true)) + 1;
  return key_it;
}

Validator::RouteNode Validator::ValidateRouteBlock(
    ConstIterator_ it, ConstIterator_& token_end,
    ServerDirective is_cgi) const {
  RouteBlock route_block;
  RouteKeyMap_ key_map;
  std::string path = TokenizeRoutePath(it, token_end);
  if (*(++token_end) != '{') {
    throw SyntaxErrorException();
  }
  InitializeKeyMap(key_map, is_cgi);
  for (it = ++token_end; it != kConfig_.end(); ++it) {
    RouteKeyIt_ key_it = FindDirectiveKey(it, token_end, key_map);
    if (key_it == key_map.end()) {
      break;
    }
    switch (key_it->second) {
      case RouteDirective::kAutoindex: {
        std::string autoindex = TokenizeSingleString(it, token_end);
        if (autoindex != "on" && autoindex != "off")
          throw SyntaxErrorException();
        route_block.autoindex = (autoindex == "on");
        break;
      }
      case RouteDirective::kBodyMax:
        break;
      case RouteDirective::kParam:
      case RouteDirective::kIndex:
      case RouteDirective::kRoot:
      case RouteDirective::kUploadPath:
        route_block[key_it->first] = TokenizeSingleString(it, token_end);
        break;
      case RouteDirective::kMethods:
        route_block.methods = TokenizeMethods(it, token_end, is_cgi);
        break;
      case RouteDirective::kRedirectTo:
        break;
      default:
        throw SyntaxErrorException();
    }
    key_map.erase(key_it);
    it = token_end;
  }

  if (key_map.count("param")) {
    throw SyntaxErrorException();
  }

  token_end = std::find(token_end, kConfig_.end(), '}');
  if (++token_end == kConfig_.end()) {
    throw SyntaxErrorException();
  }
  token_end = CheckEndOfParameter(token_end);
  return RouteNode(path, route_block);
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
      case ServerDirective::kCgiRoute:
        if (!route_map.insert(ValidateRouteBlock(it, token_end, key_it->second))
                 .second) {
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
    it = std::find_if(it + 8, kConfig_.end(), IsCharSet(" \t", false));
    if (it == kConfig_.end() || (*it != '\n' && *it != '{')) {
      throw SyntaxErrorException();
    }
    server_map.insert(ValidateServerBlock(++it));
    it = std::find_if(++it, kConfig_.end(), IsCharSet(" \n\t", false));
  }
  return server_map;
}
