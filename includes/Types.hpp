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

#include <list>
#include <map>
#include <set>
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

// SECTION : Validator 파싱 구조체 typedef
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

typedef std::map<std::string, RouteBlock> RouteMap;
typedef std::pair<std::string, RouteBlock> RouteNode;

struct ServerBlock {
  std::string error;
  RouteMap route_map;

  ServerBlock(void) : error("error.html") {}
};

typedef std::map<std::string, ServerBlock> ServerMap;
typedef std::pair<std::string, ServerBlock> ServerNode;

struct ServerGate {
  ServerBlock default_server;
  ServerMap server_map;
};

typedef std::map<uint16_t, ServerGate> PortMap;
typedef std::pair<uint16_t, ServerGate> PortNode;
typedef std::set<uint16_t> PortSet;

// SECTION : GenerateSocket 파싱 구조체 typedef
typedef std::map<int, uint16_t> ListenerMap;  // key: fd, value: port

// SECTION : Http request 파싱 구조체
typedef std::map<std::string, std::list<std::string> > Fields;

struct RequestLine {
  uint8_t method;
  uint8_t version;
  std::string path;
  std::string Host;

  RequestLine(void) : method(GET), version(1), path("/"), Host("") {}
};

struct Request {
  RequestLine req;
  Fields header;
  Fields trailer;       // NULLABLE
  std::string content;  // NULLABLE
};

#endif  // INCLUDES_TYPES_HPP_
