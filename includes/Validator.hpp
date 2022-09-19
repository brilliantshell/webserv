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
#include <sstream>
#include <string>

#include "Types.hpp"

// SECTION : Validator
class Validator {
 public:
  struct Result {
    ServerMap server_map;
    HostVector host_vector;
  };

  struct ResultPair {
    ServerNode server_node;
    HostPair host_pair;

    ResultPair(ServerNode server_node, HostPair host_pair)
        : server_node(server_node), host_pair(host_pair) {}
  };

  Validator(const std::string& config);
  Result Validate(void);

  class SyntaxErrorException : public std::exception {
   public:
    virtual const char* what() const throw() { return "syntax error"; }
  };

 private:
  class IsCharSet {
   public:
    /**
     * @brief Construct a new Is Char Set object
     *
     * @param char_set : character set to find
     * @param is_true : true면 char_set을 만나면 return, false면 char_set이
     * 아닌 것을 만나면 return.
     */
    IsCharSet(const std::string char_set, const bool is_true)
        : kCharSet_(char_set), kIsTrue_(is_true) {}
    bool operator()(char c) const {
      return !((kCharSet_.find(c) != std::string::npos) ^ kIsTrue_);
    }

   private:
    const std::string kCharSet_;
    const bool kIsTrue_;
  };

  enum class ServerDirective {
    kListen = 0,
    kServerName,
    kError,
    kRoute,
    kCgiRoute,
  };

  enum class RouteDirective {
    kAutoindex = 0,
    kMethods,
    kBodyMax,
    kRoot,
    kIndex,
    kUploadPath,
    kRedirectTo,
    kParam,
  };

  typedef std::string::const_iterator ConstIterator_;
  typedef std::map<std::string, ServerDirective> ServerKeyMap_;
  typedef std::map<std::string, ServerDirective>::iterator ServerKeyIt_;
  typedef std::map<std::string, RouteDirective> RouteKeyMap_;
  typedef std::map<std::string, RouteDirective>::iterator RouteKeyIt_;

  const std::string kConfig_;
  ConstIterator_ cursor_;

  // 디렉티브 키맵 초기화
  void InitializeKeyMap(ServerKeyMap_& key_map) const;
  void InitializeKeyMap(RouteKeyMap_& key_map, ServerDirective is_cgi) const;

  // config 읽는 중 발견한 디렉티브 판별
  ServerKeyIt_ FindDirectiveKey(ConstIterator_& delim, ServerKeyMap_& key_map);
  RouteKeyIt_ FindDirectiveKey(ConstIterator_& delim, RouteKeyMap_& key_map);

  // parameter 파싱
  uint32_t TokenizeNumber(ConstIterator_& delim);
  const std::string TokenizeSingleString(ConstIterator_& delim);
  const std::string TokenizeRoutePath(ConstIterator_& delim);
  uint8_t TokenizeMethods(ConstIterator_& delim, ServerDirective is_cgi);
  ConstIterator_ CheckEndOfParameter(ConstIterator_ delim);

  // 디렉티브별로 파싱하는 switch
  bool SwitchDirectivesToParseParam(ConstIterator_& delim,
                                    ServerBlock& server_block,
                                    HostPair& host_pair,
                                    ServerKeyMap_& key_map);
  bool SwitchDirectivesToParseParam(ConstIterator_& delim,
                                    RouteBlock& route_block,
                                    RouteKeyMap_& key_map,
                                    ServerDirective is_cgi);

  // ServerBlock, RouteBlock 파싱 및 검증
  ResultPair ValidateServerBlock(void);
  RouteNode ValidateRouteBlock(ConstIterator_& token, ServerDirective is_cgi);
};

#endif  // INCLUDES_VALIDATOR_HPP_
