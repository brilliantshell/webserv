/**
 * @file ServerRouter.cpp
 * @author ghan, jiskim, yongjule
 * @brief Route a request to a path
 * @date 2022-10-07
 *
 * @copyright Copyright (c) 2022
 */

#include "Router.hpp"

// SECTION : Router

Router::Router(ServerRouter& server_router) : server_router_(server_router) {}

Router::Result Router::Route(int status, const Request& request) {
  Result result(status);
  LocationRouter& location_router = server_router_[request.req.host];
  result.error_path = location_router.error.index;
  Location& location = location_router[request.req.path];
  result.method = location.methods;

  if (location.error == true) {
    result.success_path = location_router.error.index;
    result.method = GET;
    result.status = 404;
    return result;
  }
  // NOTE . 검증 필요
  std::string path =
      location.root + request.req.path +
      ((location.index.size() == 0 && location.autoindex == false)
           ? "index.html"
           : location.index);

  // TODO : cgi 판별 필요
  if (PathResolver().Resolve(path, PathResolver::kRouter) == false) {
    return result;
  }
  result.success_path = "." + path;
  result.param = location.param;
  return result;
}

// SECTION : Location
Location::Location(void)
    : error(false),
      autoindex(false),
      methods(GET),
      body_max(INT_MAX),
      root("/"),
      index(""),
      upload_path(""),
      redirect_to(""),
      param("") {}

Location::Location(bool is_error, std::string error_path)
    : error(is_error), index(error_path) {}

// SECTION : LocationRouter
LocationRouter::LocationRouter(void) : error(true, "./error.html") {}

Location& LocationRouter::operator[](const std::string& path) {
  return (location_map.count(path) == 1) ? location_map[path] : error;
}

// SECTION: ServerRouter
LocationRouter& ServerRouter::operator[](const std::string& server_name) {
  return location_router_map.count(server_name) == 1
             ? location_router_map[server_name]
             : default_server;
}
