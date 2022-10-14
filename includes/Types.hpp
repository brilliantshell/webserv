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
typedef std::set<uint16_t> PortSet;

// SECTION : GenerateSocket 파싱 구조체 typedef
typedef std::map<int, uint16_t> ListenerMap;  // key: fd, value: port

// SECTION : Http request 파싱 구조체
typedef std::map<std::string, std::list<std::string> > Fields;

struct RequestLine {
  uint8_t method;
  uint8_t version;
  std::string path;
  std::string query;
  std::string host;

  RequestLine(void) : method(GET), version(1), path("/"), query(""), host("") {}
};

struct Request {
  RequestLine req;
  Fields header;
  std::string content;  // NULLABLE
};

// SECTION : Router 가 필요한 server & client 연결 정보
struct ConnectionInfo {
  uint16_t server_port;
  std::string server_name;
  std::string client_addr;

  ConnectionInfo(uint16_t port, std::string addr)
      : server_port(port), client_addr(addr) {}
};

#endif  // INCLUDES_TYPES_HPP_
