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

#include <utility>

#include "CgiEnv.hpp"
#include "PathResolver.hpp"
#include "Types.hpp"

struct Location {
  bool error;
  bool autoindex;
  uint8_t methods;
  size_t body_max;
  std::string root;
  std::string index;
  std::string upload_path;
  std::string redirect_to;

  Location(void);
  Location(bool is_error, std::string error_path);

  std::string& operator[](const std::string& kKey) {
    if (kKey == "root") {
      return root;
    } else if (kKey == "index") {
      return index;
    } else if (kKey == "upload_path") {
      return upload_path;
    }
    return redirect_to;
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
  std::pair<Location&, size_t> operator[](const std::string& kPath);
};

typedef std::map<std::string, LocationRouter> LocationRouterMap;
typedef std::pair<std::string, LocationRouter> LocationRouterNode;

struct ServerRouter {
  LocationRouterMap location_router_map;
  LocationRouter default_server;

  LocationRouter& operator[](const std::string& kHost);
};

class Router {
 public:
  struct Result {
    bool is_cgi;
    int status;
    uint8_t methods;
    std::string success_path;
    std::string error_path;
    std::string redirect_to;
    CgiEnv cgi_env;

    Result(int parse_status)
        : is_cgi(false), status(parse_status), methods(GET) {}
  };

  Router(ServerRouter& server_router);

  Result Route(int status, Request& request, ConnectionInfo connection_info);

 private:
  typedef std::pair<LocationNode, size_t> CgiDiscriminator;
  ServerRouter& server_router_;

  CgiDiscriminator GetCgiLocation(LocationRouter::CgiVector& cgi_vector,
                                  const std::string& kPath);
  void RouteToLocation(Result& result, LocationRouter& location_router,
                       Request& request);
  void RouteToCgi(Result& result, Request& request,
                  const CgiDiscriminator& kCgiDiscriminator,
                  const ConnectionInfo& kConnectionInfo);
  bool GetHostAddr(std::string& server_addr) const;
  void UpdateStatus(Result& result, int status);
};

#endif  // INCLUDES_SERVER_ROUTER_HPP_
