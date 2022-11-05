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
/**
 * @brief config file 검증 하는 Validator 객체 생성
 *
 * @param kConfig .config file string
 */
Validator::Validator(const std::string& kConfig) : kConfig_(kConfig) {}

/**
 * @brief config 파일 구성 요소 검증
 *
 * @return ServerConfig 파싱된 server port 정보 구조체
 */
ServerConfig Validator::Validate(void) {
  ServerConfig result;
  PortServerList_ port_server_list;

  for (cursor_ = std::find_if(kConfig_.begin(), kConfig_.end(),
                              IsCharSet(" \n\t", false));
       cursor_ != kConfig_.end();) {
    if (std::string(cursor_, cursor_ + 8).compare("server {")) {
      throw SyntaxErrorException("server block not found");
    }
    cursor_ =
        std::find_if(cursor_ + 8, kConfig_.end(), IsCharSet(" \t", false));
    if (cursor_ == kConfig_.end() || (*cursor_ != '\n' && *cursor_ != '{')) {
      throw SyntaxErrorException("invalid configuration file");
    }
    ++cursor_;
    PortServerPair port_server = ValidateLocationRouter(result.port_set);
    port_server_list.push_back(port_server);
    cursor_ =
        std::find_if(++cursor_, kConfig_.end(), IsCharSet(" \n\t", false));
  }
  if (port_server_list.empty()) {
    throw SyntaxErrorException("invalid configuration file");
  }
  GeneratePortMap(result, port_server_list);
  return result;
}

// SECTION : private
/**
 * @brief LocationRouter 디렉티브 키맵 초기화
 *
 * @param key_map LocationRouter 디렉티브 키맵
 */
void Validator::InitializeKeyMap(ServerKeyMap_& key_map) const {
  key_map["listen"] = kListen;
  key_map["server_name"] = kServerName;
  key_map["error"] = kError;
  key_map["location"] = kRoute;
  key_map["cgi"] = kCgiRoute;
}

/**
 * @brief Location 디렉티브 키맵 초기화
 *
 * @param key_map Location 디렉티브 키맵
 * @param is_cgi ServerDirective::kCgiRoute 인지 여부
 */
void Validator::InitializeKeyMap(RouteKeyMap_& key_map,
                                 ServerDirective is_cgi) const {
  if (is_cgi == kRoute) {
    key_map["autoindex"] = kAutoindex;
    key_map["redirect_to"] = kRedirectTo;
    key_map["index"] = kIndex;
  }
  key_map["methods"] = kMethods;
  key_map["body_max"] = kBodyMax;
  key_map["root"] = kRoot;
  key_map["upload_path"] = kUploadPath;
}

/**
 * @brief config 읽는 중 발견한 LocationRouter (server) 디렉티브 판별, 유효하지
 않은 디렉티브면 throw
 *
 * @param delim LocationRouter 디렉티브 종료 위치 레퍼런스
 * @param key_map 유효한 LocationRouter 디렉티브 키맵
 * @return Validator::ServerKeyIt_ 찾은 디렉티브 가리키는 키맵 iterator
 */
Validator::ServerKeyIt_ Validator::FindDirectiveKey(ConstIterator_& delim,
                                                    ServerKeyMap_& key_map) {
  cursor_ = std::find_if(cursor_, kConfig_.end(), IsCharSet(" \n\t", false));
  if (*cursor_ == '}') {
    return key_map.end();  // NOTE : LocationRouter loop break 지점
  }
  delim = std::find_if(cursor_, kConfig_.end(), IsCharSet(" \t", true));
  ServerKeyIt_ key_it = key_map.find(std::string(cursor_, delim));
  if (key_it == key_map.end()) {
    throw SyntaxErrorException(
        cursor_ == kConfig_.end()
            ? "} not found"
            : std::string(cursor_, ((*delim == '\n') ? --delim : delim)) +
                  " is invalid directive in server block");
  }
  cursor_ = delim + 1;
  if (*cursor_ == '\n') {
    throw SyntaxErrorException("invalid server directive");
  }
  return key_it;
}

/**
 * @brief config 읽는 중 발견한 Location 디렉티브 판별, 유효하지 않은
 디렉티브면 throw
 *
 * @param delim Location 디렉티브 종료 위치 가리킬 레퍼런스
 * @param key_map 유효한 Location 디렉티브 키맵
 * @return Validator::RouteKeyIt_ 찾은 디렉티브 가리키는 키맵 iterator
 */
Validator::RouteKeyIt_ Validator::FindDirectiveKey(ConstIterator_& delim,
                                                   RouteKeyMap_& key_map) {
  cursor_ = std::find_if(cursor_, kConfig_.end(), IsCharSet(" \n\t", false));
  if (*cursor_ == '}') {
    return key_map.end();  // NOTE : Location loop break 지점
  }
  delim = std::find_if(cursor_, kConfig_.end(), IsCharSet(" \t", true));
  RouteKeyIt_ key_it = key_map.find(std::string(cursor_, delim));
  if (key_it == key_map.end()) {
    throw SyntaxErrorException(std::string(cursor_, delim) +
                               " is invalid directive in location block");
  }
  cursor_ = delim + 1;
  if (*cursor_ == '\n') {
    throw SyntaxErrorException("invalid location directive");
  }
  return key_it;
}

/**
 * @brief LocationRouter 의 listen 디렉티브의 파라미터 (PORT NUMBER) 파싱 &
 * 유효성 검사
 *
 * @param delim 파라미터 종료 위치 가리킬 레퍼런스, 파싱 후 개행 위치로 설정
 * @return uint32_t 변환된 port 값 (범위 체크 이전)
 */
uint32_t Validator::TokenizeNumber(ConstIterator_& delim) {
  uint32_t nbr;
  delim = std::find_if(cursor_, kConfig_.end(), IsCharSet("0123456789", false));
  delim = CheckEndOfParameter(delim);
  std::stringstream ss;
  ss.str(std::string(cursor_, delim));
  ss >> nbr;
  return nbr;
}

/**
 * @brief 단일 std::string 으로 받는 파라미터 파싱
 *
 * @param delim 파라미터 종료 위치 가리킬 레퍼런스, 파싱 후 개행 위치로 설정
 * @return std::string 파싱된 string 형 파라미터
 */
const std::string Validator::TokenizeSingleString(ConstIterator_& delim) {
  delim = std::find_if(cursor_, kConfig_.end(), IsCharSet(" \t\n", true));
  CheckEndOfParameter(delim);
  return std::string(cursor_, delim);
}

/**
 * @brief Location 의 path 디렉티브의 파라미터 (PATH) 파싱 & 유효성 검사
 *
 * @param delim 파라미터 종료 위치 가리킬 레퍼런스, 파싱 후 개행 위치로 설정
 * @param is_cgi cgi script 여부
 * @return std::string 라우트 경로 (cgi script 경로일 수도 있다)
 */
const std::string Validator::TokenizeRoutePath(ConstIterator_& delim,
                                               ServerDirective is_cgi) {
  delim = std::find(cursor_, kConfig_.end(), ' ');
  if (delim == kConfig_.end() || (is_cgi == kRoute && *cursor_ != '/') ||
      (is_cgi == kCgiRoute && (*cursor_ != '.' || (delim - cursor_) == 1 ||
                               std::find(cursor_, delim, '/') != delim))) {
    throw SyntaxErrorException("invalid location path");
  }
  return std::string(cursor_, delim);
}

/**
 * @brief Location 의 methods 디렉티브의 파라미터 파싱 & 유효성 검사
 *
 * @param delim 파라미터 종료 위치 가리킬 레퍼런스, 파싱 후 개행 위치로 설정
 * @param is_cgi cgi script 여부
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
    } else if (is_cgi == kRoute && method == "DELETE" && !(DELETE & flag)) {
      flag |= DELETE;
    } else {
      throw SyntaxErrorException(method + " is not a supported method");
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
    throw SyntaxErrorException(
        ("expected a newline but was [" +
         (delim == kConfig_.end() ? "EOF" : std::string(1, *delim)) + "]"));
  }
  return delim;
}

/**
 * @brief redirect_to 토큰 검증
 *
 * @param redirect_to_token
 */
void Validator::ValidateRedirectToToken(std::string& redirect_to_token) {
  UriParser uri_parser;
  UriParser::Result uri_result = uri_parser.ParseTarget(redirect_to_token);
  if (uri_result.is_valid == false ||
      path_resolver_.Resolve(uri_result.path, PathResolver::kRedirectTo) ==
          PathResolver::kFailure) {
    throw SyntaxErrorException("invalid redirect_to token");
  }
  redirect_to_token = uri_parser.GetFullPath();
}

/**
 * @brief LocationRouter 디렉티브별로 파싱하는 switch
 *
 * @param delim 디렉티브 종료 위치 가리킬 레퍼런스, 파싱 후 개행 위치로
 * @param location_router 파싱한 파라미터 저장할 LocationRouter
 * @param port LocationRouter 의 port
 * @param server_name LocationRouter 의 server_name
 * @param key_map  서버 디렉티브 map
 * @return true
 * @return false
 */
bool Validator::SwitchDirectivesToParseParam(ConstIterator_& delim,
                                             LocationRouter& location_router,
                                             uint16_t& port,
                                             std::string& server_name,
                                             ServerKeyMap_& key_map) {
  ServerKeyIt_ key_it = FindDirectiveKey(delim, key_map);
  if (key_it == key_map.end()) {
    return false;
  }
  switch (key_it->second) {
    case kListen: {
      uint32_t num = TokenizeNumber(delim);
      if (num == 0 || num > 65535) {
        throw SyntaxErrorException(
            "the port number must be in a range, 1-65535");
      }
      port = num;
      key_map.erase(key_it->first);
      break;
    }
    case kServerName:
      server_name = TokenizeSingleString(delim);
      std::transform(server_name.begin(), server_name.end(),
                     server_name.begin(), ::tolower);
      key_map.erase(key_it->first);
      break;
    case kError: {
      std::string err_page = TokenizeSingleString(delim);
      if (path_resolver_.Resolve(err_page, PathResolver::kErrorPage) ==
          PathResolver::kFailure) {
        throw SyntaxErrorException("error page cannot be a directory");
      }
      location_router.error = Location(true, err_page);
      key_map.erase(key_it->first);
      break;
    }
    case kRoute:
      if (!location_router.location_map
               .insert(ValidateLocation(delim, key_it->second))
               .second) {
        throw SyntaxErrorException("duplicated location path");
      }
      break;
    case kCgiRoute: {
      LocationNode location_node = ValidateLocation(delim, key_it->second);
      for (size_t i = 0; i < location_router.cgi_vector.size(); ++i) {
        if (location_router.cgi_vector[i].first == location_node.first) {
          throw SyntaxErrorException("duplicated cgi extension");
        }
      }
      location_router.cgi_vector.push_back(location_node);
      break;
    }
    default:
      throw SyntaxErrorException("invalid directive in server block");
  }
  return true;
}

/**
 * @brief Location 디렉티브별로 파싱하는 switch
 *
 * @param delim 디렉티브 종료 위치 가리킬
 * @param location 파싱한 파라미터 저장할 Location
 * @param key_map Location 디렉티브 map
 * @param is_cgi cgi route 인지 여부
 * @return true
 * @return false
 */
bool Validator::SwitchDirectivesToParseParam(ConstIterator_& delim,
                                             Location& location,
                                             RouteKeyMap_& key_map,
                                             ServerDirective is_cgi) {
  RouteKeyIt_ key_it = FindDirectiveKey(delim, key_map);
  if (key_it == key_map.end()) {
    return false;
  }

  switch (key_it->second) {
    case kAutoindex: {
      std::string autoindex = TokenizeSingleString(delim);
      if (autoindex != "on" && autoindex != "off")
        throw SyntaxErrorException("autoindex must be on or off");
      location.autoindex = (autoindex == "on");
      break;
    }
    case kBodyMax: {
      uint32_t num = TokenizeNumber(delim);
      if (num > INT_MAX) {
        throw SyntaxErrorException("body_max is too large");
      }
      location.body_max = num;
      break;
    }
    case kRedirectTo:
      location[key_it->first] = TokenizeSingleString(delim);
      ValidateRedirectToToken(location[key_it->first]);
      break;
    case kIndex:
    case kRoot:
    case kUploadPath:
      location[key_it->first] = TokenizeSingleString(delim);
      if ((key_it->second == kRoot || key_it->second == kUploadPath) &&
          path_resolver_.Resolve(location[key_it->first]) ==
              PathResolver::kFailure) {
        throw SyntaxErrorException("invalid path in index root upload path");
      }
      break;
    case kMethods:
      location.methods = TokenizeMethods(delim, is_cgi);
      break;
    default:
      throw SyntaxErrorException("invalid directive in location block");
  }
  key_map.erase(key_it);
  return true;
}

/**
 * @brief port와 serverNode<string, LocationRouter> 정보를 담은 PortServerPair
 * 구조체 리턴
 *
 * @param port_set port set
 * @return Validator::PortServerPair
 */
Validator::PortServerPair Validator::ValidateLocationRouter(PortSet& port_set) {
  LocationRouter location_router;
  uint16_t port;
  std::string server_name;
  ServerKeyMap_ key_map;
  ConstIterator_ delim;

  InitializeKeyMap(key_map);
  for (; cursor_ != kConfig_.end(); ++cursor_) {
    if (!SwitchDirectivesToParseParam(delim, location_router, port, server_name,
                                      key_map)) {
      break;
    }
    cursor_ = delim;
  }
  // NOTE : listen 과 location block 은 필수값!
  if (key_map.count("listen") || (location_router.location_map.empty() &&
                                  location_router.cgi_vector.empty())) {
    throw SyntaxErrorException(
        "listen directive and location block are required");
  }
  port_set.insert(port);
  return PortServerPair(port, LocationRouterNode(server_name, location_router));
}

/**
 * @brief loop 돌면서Location 디렉티브 목록 검증
 *
 * @param delim 파라미터 종료 위치 가리킬 레퍼런스, Location 끝 위치로
 * 설정
 * @param is_cgi ServerDirective::kCgiRoute 인지 여부
 * @return Validator::LocationNode 완성된 라우트 노드
 */
LocationNode Validator::ValidateLocation(ConstIterator_& delim,
                                         ServerDirective is_cgi) {
  Location location;
  RouteKeyMap_ key_map;
  std::string path = TokenizeRoutePath(delim, is_cgi);
  if (*(++delim) != '{' || (is_cgi == kRoute && path_resolver_.Resolve(path) ==
                                                    PathResolver::kFailure)) {
    throw SyntaxErrorException("invalid location block");
  }
  InitializeKeyMap(key_map, is_cgi);
  for (cursor_ = ++delim; cursor_ != kConfig_.end(); ++cursor_) {
    if (!SwitchDirectivesToParseParam(delim, location, key_map, is_cgi)) {
      break;
    }
    cursor_ = delim;
  }
  delim = std::find(delim, kConfig_.end(), '}');
  if (++delim == kConfig_.end()) {
    throw SyntaxErrorException();
  }
  delim = CheckEndOfParameter(delim);
  return LocationNode(path, location);
}

/**
 * @brief PortMap 에 port 별 ServerMap 저장
 *
 * @param result <PortMap, Portset>
 * @param port_server_list <port, LocationRouterNode> 의 리스트
 */
void Validator::GeneratePortMap(ServerConfig& result,
                                PortServerList_& port_server_list) const {
  for (PortSet::const_iterator it = result.port_set.begin();
       it != result.port_set.end(); ++it) {
    ServerRouter server_router;
    for (PortServerList_::const_iterator it2 = port_server_list.begin();
         it2 != port_server_list.end();) {
      PortServerList_::const_iterator it2_backup = it2++;
      if (it2_backup->port == *it) {
        if (server_router.location_router_map.size() == 0) {
          server_router.default_server =
              it2_backup->location_router_node.second;
        }
        if (server_router.location_router_map
                .insert(it2_backup->location_router_node)
                .second == false) {
          throw SyntaxErrorException("duplicated server name and port");
        }
        port_server_list.erase(it2_backup);
      }
    }
    result.port_map.insert(PortNode(*it, server_router));
  }
}
