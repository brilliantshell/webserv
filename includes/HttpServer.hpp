/**
 * @file HttpServer.hpp
 * @author ghan, jiskim, yongjule
 * @brief Accept connections, receive requests, send responses
 * @date 2022-09-21
 *
 * @copyright Copyright (c) 2022
 */

#ifndef INCLUDES_HTTPSERVER_HPP_
#define INCLUDES_HTTPSERVER_HPP_

#include <sys/socket.h>
#include <unistd.h>

#include "Types.hpp"

class HttpServer {
 public:
  HttpServer(const ListenerMap& listener_map);
  void Run(void);

 private:
  ListenerMap listener_map_;
};

#endif  // INCLUDES_HTTPSERVER_HPP_
