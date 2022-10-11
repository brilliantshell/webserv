/**
 * @file ServerRouter.hpp
 * @author ghan, jiskim, yongjule
 * @brief Route a request to a path
 * @date 2022-10-07
 *
 * @copyright Copyright (c) 2022
 */

#ifndef INCLUDES_SERVER_ROUTER_HPP_
#define INCLUDES_SERVER_ROUTER_HPP_

#include "PathResolver.hpp"
#include "Types.hpp"

struct Location {
  bool error;
  bool autoindex;
  uint8_t methods;
  int32_t body_max;
  std::string root;
  std::string index;
  std::string upload_path;
  std::string redirect_to;
  std::string param;

  Location(void);
  Location(bool is_error, std::string error_path);

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

typedef std::map<std::string, Location> LocationMap;
typedef std::pair<std::string, Location> LocationNode;

struct LocationRouter {
  typedef std::vector<LocationNode> CgiVector;

  Location error;
  CgiVector cgi_vector;
  LocationMap location_map;

  LocationRouter(void);
  Location& operator[](const std::string& path);
};

typedef std::map<std::string, LocationRouter> LocationRouterMap;
typedef std::pair<std::string, LocationRouter> LocationRouterNode;

struct ServerRouter {
  LocationRouterMap location_router_map;
  LocationRouter default_server;

  LocationRouter& operator[](const std::string& server_name);
};

class Router {
 public:
  struct Result {
    int status;
    uint8_t method;
    std::string success_path;
    std::string error_path;
    std::string param;

    Result(int parse_status)
        : status(parse_status),
          method(GET),
          success_path(""),
          error_path(""),
          param("") {}
  };

  Router(ServerRouter& server_router);

  Result Route(int status, Request& request);

 private:
  typedef std::pair<LocationNode, size_t> CgiDiscriminator;
  ServerRouter& server_router_;

  CgiDiscriminator GetCgiLocation(LocationRouter::CgiVector& cgi_vector,
                                  const std::string& path);
  void RouteToLocation(Result& result, LocationRouter& location_router,
                       Request& request);
  void RouteToCgi(Result& result, CgiDiscriminator& cgi_discriminator,
                  std::string& cgi_extension, const Request& request);
  void UpdateStatus(Result& result, int status);
};

typedef std::map<uint16_t, ServerRouter> PortMap;
typedef std::pair<uint16_t, ServerRouter> PortNode;

#endif  // INCLUDES_SERVER_ROUTER_HPP_
