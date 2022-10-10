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
  result.error_path = "." + location_router.error.index;
  if (status >= 400) {
    result.success_path = result.error_path;
  } else {
    std::pair<LocationNode, size_t> cgi_discriminator =
        GetCgiLocation(location_router.cgi_vector, request.req.path);
    cgi_discriminator.second == std::string::npos
        ? RouteToLocation(result, location_router, request.req)
        : RouteToCgi(result, cgi_discriminator.first,
                     cgi_discriminator.first.first, request.req);
  }
  return result;
}

// private
std::pair<LocationNode, size_t> Router::GetCgiLocation(
    LocationRouter::CgiVector& cgi_vector, const std::string& path) {
  LocationNode location_node;
  size_t pos = std::string::npos;
  for (size_t i = 0; i < cgi_vector.size(); ++i) {
    size_t latest = path.find(cgi_vector[i].first);
    if (latest < pos && path[latest - 1] != '/' &&
        (latest + cgi_vector[i].first.size() == path.size() ||
         path[latest + cgi_vector[i].first.size()] == '/')) {
      pos = latest;  // update cgi extension
      location_node = cgi_vector[i];
    }
  }
  return std::make_pair(location_node, pos);
}

void Router::RouteToLocation(Router::Result& result,
                             LocationRouter& location_router,
                             const RequestLine& req) {
  // TODO PATH RESOLVE
  Location& location = location_router[req.path];
  result.method = location.methods;

  if ((location.methods & req.method) == 0) {
    result.success_path = result.error_path;
    result.method = GET;
    result.status = 405;
    return;
  }
  if (location.error == true) {
    result.success_path = result.error_path;
    result.method = GET;
    result.status = 404;
    return;
  }
  std::string path =
      location.root + req.path +
      ((location.index.size() == 0 && location.autoindex == false)
           ? "index.html"
           : location.index);
  if (path_resolver_.Resolve(path, PathResolver::kRouter) == false) {
    return;  // NOTE : ㅇㅔ러코드 분기 가르기
  }
  result.success_path = "." + path;
}

void Router::RouteToCgi(Router::Result& result, LocationNode& cgi_location_node,
                        std::string& cgi_extension, const RequestLine& req) {
  if ((cgi_location_node.second.methods & req.method) == 0) {
    result.success_path = result.error_path;
    result.method = GET;
    result.status = 405;
    return;
  }
  result.success_path = "." + cgi_location_node.second.root +
                        req.path.substr(0, cgi_extension.size());
  result.param = cgi_location_node.second.param;
  result.method = cgi_location_node.second.methods;
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
LocationRouter::LocationRouter(void) : error(true, "/error.html") {}

#include <iostream>
Location& LocationRouter::operator[](const std::string& path) {
  // std::pair<LocationMap::iterator, LocationMap::iterator> range =
  //     location_map.equal_range(path);
  // LocationMap::iterator lower_bound = --range.first;
  // LocationMap::iterator upper_bound = range.second;

  // if (lower_bound != location_map.end() && lower_bound->first == path) {
  //   return lower_bound->second;
  // }
  // if (lower_bound == upper_bound && lower_bound == location_map.end()) {
  //   std::cout << "따흑... " << location_map.size() << "\n";
  //   return error;
  // }
  // std::cout << "upper : " << upper_bound->first << ", path : " << path
  //           << std::endl;
  // if (lower_bound != location_map.end() &&
  //     path.size() > lower_bound->first.size() &&
  //     std::equal(lower_bound->first.begin(), lower_bound->first.end(),
  //                path.begin())) {
  //   return lower_bound->second;
  // } else if (upper_bound != location_map.end() &&
  //            path.size() > upper_bound->first.size() &&
  //            std::equal(upper_bound->first.begin(), upper_bound->first.end(),
  //                       path.begin())) {
  //   return upper_bound->second;
  // }
  // std::cout << "여기 오냐?\n";
  // return error;

  return (location_map.count(path) == 1) ? location_map[path] : error;
}

// SECTION: ServerRouter
LocationRouter& ServerRouter::operator[](const std::string& server_name) {
  std::string host = server_name.substr(0, server_name.find(':'));
  return (location_router_map.count(host) == 1) ? location_router_map[host]
                                                : default_server;
}
