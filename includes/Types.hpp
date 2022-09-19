/**
 * @file Types.hpp
 * @author ghan, jiskim, yongjule
 * @brief fixed width integer types definition
 * @date 2022-09-07
 *
 * @copyright Copyright (c) 2022
 */

#ifndef INCLUDES_TYPES_HPP_
#define INCLUDES_TYPES_HPP_

#include <map>
#include <string>
#include <vector>

#define GET 0b00000001
#define POST 0b00000010
#define DELETE 0b00000100

typedef signed char int8_t;
typedef short int int16_t;
typedef int int32_t;

/* Unsigned.  */
typedef unsigned char uint8_t;
typedef unsigned short int uint16_t;
typedef unsigned int uint32_t;

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

// SECTION : Validator 파싱 구조체 typedef
typedef std::map<std::string, RouteBlock> RouteMap;
typedef std::pair<std::string, RouteBlock> RouteNode;

// SECTION : ServerBlock
struct ServerBlock {
  std::string error;
  RouteMap route_map;

  ServerBlock(void) : error("error.html") {}
};

// SECTION : HostVector 의 element
struct HostPair {
  uint16_t port;
  std::string host;

  HostPair(void) : port(0), host("127.0.0.1") {}
};

// SECTION : Validator 파싱 구조체 typedef
typedef std::map<std::string, ServerBlock> ServerMap;
typedef std::pair<std::string, ServerBlock> ServerNode;
typedef std::vector<HostPair> HostVector;

// SECTION : SocketGenerator 파싱 구조체 typedef
typedef std::map<int, std::string> ListenerMap;  // key: fd, value: host:port

#endif  // INCLUDES_TYPES_HPP_
