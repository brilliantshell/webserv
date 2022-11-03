/**
 * @file Validator.hpp
 * @author ghan, jiskim, yongjule
 * @brief Validate configuration file
 * @date 2022-09-05
 *
 * @copyright Copyright (c) 2022
 */

#ifndef INCLUDES_VALIDATOR_HPP_
#define INCLUDES_VALIDATOR_HPP_

#include <algorithm>
#include <exception>
#include <iostream>
#include <list>
#include <sstream>
#include <string>

#include "ParseUtils.hpp"
#include "PathResolver.hpp"
#include "Router.hpp"
#include "Types.hpp"
#include "UriParser.hpp"

// SECTION : Validator
class Validator {
 public:
  Validator(const std::string& config);
  ServerConfig Validate(void);

  class SyntaxErrorException : public std::exception {
   public:
    virtual const char* what() const throw() { return "syntax error"; }
  };

 private:
  enum ServerDirective {
    kListen = 0,
    kServerName,
    kError,
    kRoute,
    kCgiRoute
  };

  enum LocationDirective {
    kAutoindex = 0,
    kMethods,
    kBodyMax,
    kRoot,
    kIndex,
    kUploadPath,
    kRedirectTo
  };

  struct PortServerPair {
    uint16_t port;
    LocationRouterNode location_router_node;

    PortServerPair(uint16_t port, LocationRouterNode location_router_node)
        : port(port), location_router_node(location_router_node) {}
  };

  typedef std::list<PortServerPair> PortServerList_;
  typedef std::string::const_iterator ConstIterator_;
  typedef std::map<std::string, ServerDirective> ServerKeyMap_;
  typedef std::map<std::string, ServerDirective>::iterator ServerKeyIt_;
  typedef std::map<std::string, LocationDirective> RouteKeyMap_;
  typedef std::map<std::string, LocationDirective>::iterator RouteKeyIt_;

  const std::string kConfig_;
  ConstIterator_ cursor_;
  PathResolver path_resolver_;

  // 디렉티브 키맵 초기화
  void InitializeKeyMap(ServerKeyMap_& key_map) const;
  void InitializeKeyMap(RouteKeyMap_& key_map, ServerDirective is_cgi) const;

  // config 읽는 중 발견한 디렉티브 판별
  ServerKeyIt_ FindDirectiveKey(ConstIterator_& delim, ServerKeyMap_& key_map);
  RouteKeyIt_ FindDirectiveKey(ConstIterator_& delim, RouteKeyMap_& key_map);

  // parameter 파싱
  uint32_t TokenizeNumber(ConstIterator_& delim);
  const std::string TokenizeSingleString(ConstIterator_& delim);
  const std::string TokenizeRoutePath(ConstIterator_& delim,
                                      ServerDirective is_cgi);
  uint8_t TokenizeMethods(ConstIterator_& delim, ServerDirective is_cgi);
  ConstIterator_ CheckEndOfParameter(ConstIterator_ delim);
  void ValidateRedirectToToken(std::string& redirect_to_token);

  // 디렉티브별로 파싱하는 switch
  bool SwitchDirectivesToParseParam(ConstIterator_& delim,
                                    LocationRouter& server_block,
                                    uint16_t& port, std::string& server_name,
                                    ServerKeyMap_& key_map);
  bool SwitchDirectivesToParseParam(ConstIterator_& delim,
                                    Location& route_block,
                                    RouteKeyMap_& key_map,
                                    ServerDirective is_cgi);

  // LocationRouter, Location 파싱 및 검증
  PortServerPair ValidateLocationRouter(PortSet& port_set);
  LocationNode ValidateLocation(ConstIterator_& token, ServerDirective is_cgi);

  // PortMap 생성
  void GeneratePortMap(ServerConfig& result,
                       PortServerList_& port_server_list) const;
};

#endif  // INCLUDES_VALIDATOR_HPP_
