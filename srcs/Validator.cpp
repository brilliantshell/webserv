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
 * @param it ServerBlock 디렉티브 시작 위치 레퍼런스
 * @param token_end ServerBlock 디렉티브 종료 위치 레퍼런스
 * @param key_map 유효한 ServerBlock 디렉티브 키맵
 * @return Validator::ServerKeyIt_ 찾은 디렉티브 가리키는 키맵 iterator
 */
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

/**
 * @brief config 읽는 중 발견한 RouteBlock 디렉티브 판별, 유효하지 않은
 디렉티브면 throw
 *
 * @param it RouteBlock 디렉티브 시작 위치 레퍼런스
 * @param token_end RouteBlock 디렉티브 종료 위치 가리킬 레퍼런스
 * @param key_map 유효한 RouteBlock 디렉티브 키맵
 * @return Validator::RouteKeyIt_ 찾은 디렉티브 가리키는 키맵 iterator
 */
Validator::RouteKeyIt_ Validator::FindDirectiveKey(
    ConstIterator_& it, ConstIterator_& token_end,
    RouteKeyMap_& key_map) const {
  it = std::find_if(it, kConfig_.end(), IsCharSet(" \n\t", false));
  if (*it == '}') {
    return key_map.end();  // NOTE : RouteBlock loop break 지점
  }
  token_end = std::find_if(it, kConfig_.end(), IsCharSet(" \t", true));
  RouteKeyIt_ key_it = key_map.find(std::string(it, token_end));
  if (key_it == key_map.end()) {
    throw SyntaxErrorException();
  }
  it = std::find_if(it, kConfig_.end(), IsCharSet(" \t", true)) + 1;
  return key_it;
}

/**
 * @brief ServerBlock 의 listen 디렉티브의 파라미터 (PORT NUMBER) 파싱 & 유효성
 * 검사
 *
 * @param it 파라미터 시작 위치
 * @param token_end 파라미터 종료 위치 가리킬 레퍼런스, 개행 위치로 설정
 * @return uint16_t 변환된 port 값
 */
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

/**
 * @brief 단일 std::string 으로 받는 파라미터 파싱
 *
 * @param it 파라미터 시작 위치
 * @param token_end 파라미터 종료 위치 가리킬 레퍼런스, 개행 위치로 설정
 * @return std::string
 */
std::string Validator::TokenizeSingleString(ConstIterator_ it,
                                            ConstIterator_& token_end) const {
  token_end = std::find_if(it, kConfig_.end(), IsCharSet(" \t\n", true));
  return std::string(it, CheckEndOfParameter(token_end));
}

/**
 * @brief RouteBlock 의 path 디렉티브의 파라미터 (PATH) 파싱 & 유효성 검사
 *
 * @param it 파라미터 시작 위치
 * @param token_end 파라미터 종료 위치 가리킬 레퍼런스, 개행 위치로 설정
 * @return std::string 라우트 경로 (cgi script 경로일 수도 있다)
 */
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

/**
 * @brief RouteBlock 의 methods 디렉티브의 파라미터 파싱 & 유효성 검사
 *
 * @param it 파라미터 시작 위치
 * @param token_end 파라미터 종료 위치 가리킬 레퍼런스, 개행 위치로 설정
 * @return uint8_t bit-masked 허용 methods 플래그
 */
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

/**
 * @brief 파라미터 파싱 후 token_end 를 개행 위치로 이동
 *
 * @param token_end
 * @return Validator::ConstIterator_
 */
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

/**
 * @brief loop 돌면서 ServerBlock 디렉티브 목록 검증
 *
 * @param it config 파일 커서
 * @return Validator::ServerNode 완성된 서버 노드
 */
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

/**
 * @brief loop 돌면서 RouteBlock 디렉티브 목록 검증
 *
 * @param it config 파일 커서
 * @param token_end 파라미터 종료 위치 가리킬 레퍼런스, RouteBlock 끝 위치로
 * 설정
 *
 * @param is_cgi ServerDirective::kCgiRoute 인지 여부
 * @return Validator::RouteNode 완성된 서버 노드
 */
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
