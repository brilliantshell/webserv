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
#include <map>
#include <set>
#include <sstream>
#include <string>

#include "Types.hpp"

#define GET 0b00000001
#define POST 0b00000010
#define DELETE 0b00000100

// SECTION : RouteBlock
struct RouteBlock {
  bool autoindex;
  uint8_t methods;
  int32_t body_max;
  std::string root;
  std::string index;
  std::string upload_path;
  std::string redirect_to;
  std::string param;

  RouteBlock()
      : root("./"),
        index(""),
        methods(GET),
        body_max(INT_MAX),
        autoindex(false),
        upload_path(""),
        redirect_to("") {}

  std::string& operator[](const std::string& key) {
    if (key == "root") {
      return root;
    } else if (key == "index") {
      return index;
    } else if (key == "upload_path") {
      return upload_path;
    } else if (key == "redirect_to") {
      return redirect_to;
    }
    return param;
  }
};

// SECTION : ServerBlock
struct ServerBlock {
  uint16_t port;
  std::string server_name;
  std::string error;

  ServerBlock(void) : port(0), server_name("127.0.0.1"), error("error.html") {}

  std::string& operator[](const std::string& key) {
    if (key == "server_name") {
      return server_name;
    }
    return error;
  }
};

// SECTION : Validator
class Validator {
 private:
  struct CompareServerBlock {
    bool operator()(const ServerBlock& lhs, const ServerBlock& rhs) const {
      return lhs.port < rhs.port;
    }
  };

 public:
  typedef std::map<std::string, RouteBlock> RouteMap;
  typedef std::pair<std::string, RouteBlock> RouteNode;
  typedef std::map<ServerBlock, RouteMap, CompareServerBlock> ServerMap;
  typedef std::pair<ServerBlock, RouteMap> ServerNode;

  Validator(const std::string& config);
  ServerMap Validate(void);

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

  // 디렉티브 키맵 초기화
  void InitializeKeyMap(ServerKeyMap_& key_map) const;
  void InitializeKeyMap(RouteKeyMap_& key_map, ServerDirective is_cgi) const;

  // config 읽는 중 발견한 디렉티브 판별
  ServerKeyIt_ FindDirectiveKey(ConstIterator_& it, ConstIterator_& token_end,
                                ServerKeyMap_& key_map) const;
  RouteKeyIt_ FindDirectiveKey(ConstIterator_& it, ConstIterator_& token_end,
                               RouteKeyMap_& key_map) const;

  // parameter 파싱
  uint16_t TokenizePort(ConstIterator_ it, ConstIterator_& token_end) const;
  std::string TokenizeSingleString(ConstIterator_ it,
                                   ConstIterator_& token_end) const;
  std::string TokenizeRoutePath(ConstIterator_ it,
                                ConstIterator_& token_end) const;
  uint8_t TokenizeMethods(ConstIterator_ it, ConstIterator_& token_end,
                          ServerDirective is_cgi) const;
  ConstIterator_ CheckEndOfParameter(ConstIterator_ token_end) const;

  // ServerBlock, RouteBlock 파싱 및 검증
  ServerNode ValidateServerBlock(ConstIterator_& it) const;
  RouteNode ValidateRouteBlock(ConstIterator_ it, ConstIterator_& token,
                               ServerDirective is_cgi) const;
};

#endif  // INCLUDES_VALIDATOR_HPP_
