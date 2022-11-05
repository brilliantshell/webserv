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

#include <iostream>
#include <list>
#include <map>
#include <set>
#include <string>
#include <vector>

#define GET 0x01
#define POST 0x02
#define DELETE 0x04

#define PRINT_ERROR(msg) std::cerr << "BrilliantServer : " << msg << std::endl;
#define PRINT_OUT(msg) std::cout << "BrilliantServer : " << msg << std::endl;

struct ServerRouter;

typedef signed char int8_t;
typedef short int int16_t;
typedef int int32_t;

/* Unsigned.  */
typedef unsigned char uint8_t;
typedef unsigned short int uint16_t;
typedef unsigned int uint32_t;

// SECTION : Validator 파싱 구조체 typedef
typedef std::map<uint16_t, ServerRouter> PortMap;
typedef std::pair<uint16_t, ServerRouter> PortNode;
typedef std::set<uint16_t> PortSet;

struct ServerConfig {
  PortMap port_map;
  PortSet port_set;
};

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

// SECTION : ResponseManager 가 반환하는 응답 헤더 필드
typedef std::map<std::string, std::string> ResponseHeaderMap;

// SECTION : ResponseBufferQueue node
struct ResponseBuffer {
  enum { kHeader = 0, kContent };

  bool is_complete;
  int cur_buf;
  size_t offset;
  std::string header;
  std::string content;

  ResponseBuffer(void) : is_complete(false), cur_buf(kHeader), offset(0) {}
};

#endif  // INCLUDES_TYPES_HPP_
