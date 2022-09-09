/**
 * @file Validator.cpp
 * @author ghan, jiskim, yongjule
 * @brief Validate configuration file
 * @date 2022-09-05
 *
 * @copyright Copyright (c) 2022
 */

#include "Validator.hpp"

// SECTION : public
Validator::Validator(const std::string& config) : kConfig_(config) {}

Validator::ServerMap Validator::Validate(void) {
  ServerMap server_map;

  for (cursor_ = std::find_if(kConfig_.begin(), kConfig_.end(),
                              IsCharSet(" \n\t", false));
       cursor_ != kConfig_.end();) {
    if (std::string(cursor_, cursor_ + 8).compare("server {")) {
      throw SyntaxErrorException();
    }
    cursor_ =
        std::find_if(cursor_ + 8, kConfig_.end(), IsCharSet(" \t", false));
    if (cursor_ == kConfig_.end() || (*cursor_ != '\n' && *cursor_ != '{')) {
      throw SyntaxErrorException();
    }
    ++cursor_;
    server_map.insert(ValidateServerBlock());
    cursor_ =
        std::find_if(++cursor_, kConfig_.end(), IsCharSet(" \n\t", false));
  }
  return server_map;
}

// SECTION : private

/**
 * @brief ServerBlock 디렉티브 키맵 초기화
 *
 * @param key_map ServerBlock 디렉티브 키맵
 */
void Validator::InitializeKeyMap(ServerKeyMap_& key_map) const {
  key_map["listen"] = ServerDirective::kListen;
  key_map["server_name"] = ServerDirective::kServerName;
  key_map["error"] = ServerDirective::kError;
  key_map["route"] = ServerDirective::kRoute;
  key_map["cgi_route"] = ServerDirective::kCgiRoute;
}

/**
 * @brief RouteBlock 디렉티브 키맵 초기화
 *
 * @param key_map RouteBlock 디렉티브 키맵
 * @param is_cgi ServerDirective::kCgiRoute 인지 여부
 */
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

/**
 * @brief config 읽는 중 발견한 ServerBlock 디렉티브 판별, 유효하지 않은
 디렉티브면 throw
 *
 * @param delim ServerBlock 디렉티브 종료 위치 레퍼런스
 * @param key_map 유효한 ServerBlock 디렉티브 키맵
 * @return Validator::ServerKeyIt_ 찾은 디렉티브 가리키는 키맵 iterator
 */
Validator::ServerKeyIt_ Validator::FindDirectiveKey(ConstIterator_& delim,
                                                    ServerKeyMap_& key_map) {
  cursor_ = std::find_if(cursor_, kConfig_.end(), IsCharSet(" \n\t", false));
  if (*cursor_ == '}') {
    return key_map.end();  // NOTE : ServerBlock loop break 지점
  }
  delim = std::find_if(cursor_, kConfig_.end(), IsCharSet(" \t", true));
  ServerKeyIt_ key_it = key_map.find(std::string(cursor_, delim));
  if (key_it == key_map.end()) {
    throw SyntaxErrorException();
  }
  cursor_ = std::find_if(cursor_, kConfig_.end(), IsCharSet(" \t", true)) + 1;
  return key_it;
}

/**
 * @brief config 읽는 중 발견한 RouteBlock 디렉티브 판별, 유효하지 않은
 디렉티브면 throw
 *
 * @param delim RouteBlock 디렉티브 종료 위치 가리킬 레퍼런스
 * @param key_map 유효한 RouteBlock 디렉티브 키맵
 * @return Validator::RouteKeyIt_ 찾은 디렉티브 가리키는 키맵 iterator
 */
Validator::RouteKeyIt_ Validator::FindDirectiveKey(ConstIterator_& delim,
                                                   RouteKeyMap_& key_map) {
  cursor_ = std::find_if(cursor_, kConfig_.end(), IsCharSet(" \n\t", false));
  if (*cursor_ == '}') {
    return key_map.end();  // NOTE : RouteBlock loop break 지점
  }
  delim = std::find_if(cursor_, kConfig_.end(), IsCharSet(" \t", true));
  RouteKeyIt_ key_it = key_map.find(std::string(cursor_, delim));
  if (key_it == key_map.end()) {
    throw SyntaxErrorException();
  }
  cursor_ = std::find_if(cursor_, kConfig_.end(), IsCharSet(" \t", true)) + 1;
  return key_it;
}

/**
 * @brief ServerBlock 의 listen 디렉티브의 파라미터 (PORT NUMBER) 파싱 & 유효성
 * 검사
 *
 * @param delim 파라미터 종료 위치 가리킬 레퍼런스, 파싱 후 개행 위치로 설정
 * @return uint16_t 변환된 port 값
 */
uint16_t Validator::TokenizePort(ConstIterator_& delim) {
  uint32_t port;
  delim = std::find_if(cursor_, kConfig_.end(), IsCharSet("0123456789", false));
  delim = CheckEndOfParameter(delim);
  std::stringstream ss;
  ss.str(std::string(cursor_, delim));
  ss >> port;
  if (port == 0 || port > 65535) {
    throw SyntaxErrorException();
  }
  return port;
}

/**
 * @brief 단일 std::string 으로 받는 파라미터 파싱
 *
 * @param delim 파라미터 종료 위치 가리킬 레퍼런스, 파싱 후 개행 위치로 설정
 * @return std::string
 */
std::string Validator::TokenizeSingleString(ConstIterator_& delim) {
  delim = std::find_if(cursor_, kConfig_.end(), IsCharSet(" \t\n", true));
  return std::string(cursor_, CheckEndOfParameter(delim));
}

/**
 * @brief RouteBlock 의 path 디렉티브의 파라미터 (PATH) 파싱 & 유효성 검사
 *
 * @param delim 파라미터 종료 위치 가리킬 레퍼런스, 파싱 후 개행 위치로 설정
 * @return std::string 라우트 경로 (cgi script 경로일 수도 있다)
 */
std::string Validator::TokenizeRoutePath(ConstIterator_& delim) {
  delim = std::find(cursor_, kConfig_.end(), ' ');
  if (delim == kConfig_.end() ||
      (*cursor_ == '.' &&
       ((delim - cursor_) == 1 || std::find(cursor_, delim, '/') != delim))) {
    throw SyntaxErrorException();
  }
  return (*cursor_ == '.') ? std::string(cursor_, delim)
                           : "." + std::string(cursor_, delim);
}

/**
 * @brief RouteBlock 의 methods 디렉티브의 파라미터 파싱 & 유효성 검사
 *
 * @param delim 파라미터 종료 위치 가리킬 레퍼런스, 파싱 후 개행 위치로 설정
 * @return uint8_t bit-masked 허용 methods 플래그
 */
uint8_t Validator::TokenizeMethods(ConstIterator_& delim,
                                   ServerDirective is_cgi) {
  std::string method;
  uint8_t flag = 0;
  for (; cursor_ != kConfig_.end() && *cursor_ != '\n';
       cursor_ = std::find_if(delim, kConfig_.end(), IsCharSet(" \t", false))) {
    delim = std::find_if(cursor_, kConfig_.end(), IsCharSet(" \t\n", true));
    method = std::string(cursor_, delim);
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

/**
 * @brief 파라미터 파싱 후 delim 를 개행 위치로 이동
 *
 * @param delim
 * @return Validator::ConstIterator_
 */
Validator::ConstIterator_ Validator::CheckEndOfParameter(ConstIterator_ delim) {
  if (*delim != '\n') {
    delim = std::find_if(delim, kConfig_.end(), IsCharSet(" \t", false));
  }
  if (delim == kConfig_.end() || *delim != '\n') {
    throw SyntaxErrorException();
  }
  return delim;
}

/**
 * @brief ServerBlock 디렉티브별로 파싱하는 switch
 *
 * @param delim 디렉티브 종료 위치 가리킬 레퍼런스, 파싱 후 개행 위치로
 * @param server_block 파싱한 파라미터 저장할 ServerBlock
 * @param key_map 서버 디렉티브 map
 * @param route_map 라우트 디렉티브 map
 * @return true  파싱 성공
 * @return false 서버 블록이 끝난 경우
 */
bool Validator::SwitchDirectivesToParseParam(ConstIterator_& delim,
                                             ServerBlock& server_block,
                                             ServerKeyMap_& key_map,
                                             RouteMap& route_map) {
  ServerKeyIt_ key_it = FindDirectiveKey(delim, key_map);
  if (key_it == key_map.end()) {
    return false;
  }
  switch (key_it->second) {
    case ServerDirective::kListen:
      server_block.port = TokenizePort(delim);
      key_map.erase(key_it->first);
      break;
    case ServerDirective::kRoute:
    case ServerDirective::kCgiRoute:
      if (!route_map.insert(ValidateRouteBlock(delim, key_it->second)).second) {
        throw SyntaxErrorException();
      }
      break;
    case ServerDirective::kServerName:
    case ServerDirective::kError:
      server_block[key_it->first] = TokenizeSingleString(delim);
      key_map.erase(key_it->first);
      break;
    default:
      throw SyntaxErrorException();
  }
  return true;
}

/**
 * @brief RouteBlock 디렉티브별로 파싱하는 switch
 *
 * @param delim 디렉티브 종료 위치 가리킬
 * @param route_block 파싱한 파라미터 저장할 RouteBlock
 * @param key_map RouteBlock 디렉티브 map
 * @param is_cgi cgi route 인지 여부
 * @return true 파싱 성공
 * @return false 서버 블록이 끝난 경우
 */
bool Validator::SwitchDirectivesToParseParam(ConstIterator_& delim,
                                             RouteBlock& route_block,
                                             RouteKeyMap_& key_map,
                                             ServerDirective is_cgi) {
  RouteKeyIt_ key_it = FindDirectiveKey(delim, key_map);
  if (key_it == key_map.end()) {
    return false;
  }
  switch (key_it->second) {
    case RouteDirective::kAutoindex: {
      std::string autoindex = TokenizeSingleString(delim);
      if (autoindex != "on" && autoindex != "off") throw SyntaxErrorException();
      route_block.autoindex = (autoindex == "on");
      break;
    }
    case RouteDirective::kBodyMax:
      break;
    case RouteDirective::kParam:
    case RouteDirective::kIndex:
    case RouteDirective::kRoot:
    case RouteDirective::kUploadPath:
      route_block[key_it->first] = TokenizeSingleString(delim);
      break;
    case RouteDirective::kMethods:
      route_block.methods = TokenizeMethods(delim, is_cgi);
      break;
    case RouteDirective::kRedirectTo:
      break;
    default:
      throw SyntaxErrorException();
  }
  key_map.erase(key_it);
  return true;
}

/**
 * @brief loop 돌면서 ServerBlock 디렉티브 목록 검증
 *
 * @return Validator::ServerNode 완성된 서버 노드
 */
Validator::ServerNode Validator::ValidateServerBlock(void) {
  ServerBlock server_block;
  RouteMap route_map;
  ServerKeyMap_ key_map;
  ConstIterator_ delim;

  InitializeKeyMap(key_map);
  for (; cursor_ != kConfig_.end(); ++cursor_) {
    if (!SwitchDirectivesToParseParam(delim, server_block, key_map,
                                      route_map)) {
      break;
    }
    cursor_ = delim;
  }

  // NOTE : listen 은 필수값!
  if (key_map.count("listen")) {
    throw SyntaxErrorException();
  }
  return ServerNode(server_block, route_map);
}

/**
 * @brief loop 돌면서RouteBlock 디렉티브 목록 검증
 *
 * @param delim 파라미터 종료 위치 가리킬 레퍼런스, RouteBlock 끝 위치로
 * 설정
 * @param is_cgi ServerDirective::kCgiRoute 인지 여부
 * @return Validator::RouteNode 완성된 서버 노드
 */
Validator::RouteNode Validator::ValidateRouteBlock(ConstIterator_& delim,
                                                   ServerDirective is_cgi) {
  RouteBlock route_block;
  RouteKeyMap_ key_map;
  std::string path = TokenizeRoutePath(delim);
  if (*(++delim) != '{') {
    throw SyntaxErrorException();
  }
  InitializeKeyMap(key_map, is_cgi);
  for (cursor_ = ++delim; cursor_ != kConfig_.end(); ++cursor_) {
    if (!SwitchDirectivesToParseParam(delim, route_block, key_map, is_cgi)) {
      break;
    }
    cursor_ = delim;
  }

  if (key_map.count("param")) {
    throw SyntaxErrorException();
  }

  delim = std::find(delim, kConfig_.end(), '}');
  if (++delim == kConfig_.end()) {
    throw SyntaxErrorException();
  }
  delim = CheckEndOfParameter(delim);
  return RouteNode(path, route_block);
}
