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

Router::Result Router::Route(int status, Request& request,
                             ConnectionInfo connection_info) {
  Result result(status);
  LocationRouter& location_router = server_router_[request.req.host];
  if (&location_router == &server_router_.default_server) {
    if (GetHostAddr(connection_info.server_name) == false) {
      result.status = 500;  // INTERNAL SERVER ERROR
      return result;
    }
  } else {
    connection_info.server_name = request.req.host;
  }
  result.error_path = "." + location_router.error.index;
  if (status >= 400) {
    result.success_path = result.error_path;
  } else {
    CgiDiscriminator cgi_discriminator =
        GetCgiLocation(location_router.cgi_vector, request.req.path);
    cgi_discriminator.second == std::string::npos
        ? RouteToLocation(result, location_router, request)
        : RouteToCgi(result, request, cgi_discriminator, connection_info);
  }
  return result;
}

// private

Router::CgiDiscriminator Router::GetCgiLocation(
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

void Router::RouteToLocation(Result& result, LocationRouter& location_router,
                             Request& request) {
  PathResolver path_resolver;
  PathResolver::Status path_status =
      path_resolver.Resolve(request.req.path, PathResolver::Purpose::kRouter);
  if (path_status == PathResolver::Status::kFailure) {
    return UpdateStatus(result, 404);  // Path Not Found
  }
  Location& location = location_router[request.req.path];
  if (location.error == true) {
    return UpdateStatus(result, 404);  // Page Not Found
  }
  if ((location.methods & request.req.method) == 0) {
    return UpdateStatus(result, 405);  // Method Not Allowed
  }
  if (location.body_max < request.content.size()) {
    return UpdateStatus(result, 413);  // Request Entity Too Large
  }
  std::string path = location.root;
  if ((location.methods & POST) == POST) {
    path += location.upload_path.substr(1);
  }
  path += request.req.path.substr(1);
  if (path_status == PathResolver::Status::kFile) {
    path += path_resolver.get_file_name();
  } else {
    if (location.methods & (POST | DELETE)) {
      return UpdateStatus(result, 403);  // Forbidden
    }
    path += ((location.index.size() == 0 && location.autoindex == false)
                 ? "index.html"
                 : location.index);
  }
  result.method = location.methods;
  result.success_path = "." + path;
}

void Router::RouteToCgi(Result& result, Request& request,
                        const CgiDiscriminator& cgi_discriminator,
                        const ConnectionInfo& connection_info) {
  const std::string& cgi_ext = cgi_discriminator.first.first;
  const Location& cgi_location = cgi_discriminator.first.second;
  if ((cgi_location.methods & request.req.method) == 0) {
    return UpdateStatus(result, 405);  // Method Not Allowed
  }
  result.success_path =
      "." + cgi_location.root +
      request.req.path.substr(1, cgi_discriminator.second + cgi_ext.size() - 1);
  result.method = cgi_location.methods;
  if (result.cgi_env.SetMetaVariables(request, cgi_location.root, cgi_ext,
                                      connection_info) == false) {
    result.status = 500;  // INTERNAL SERVER ERROR
  }
  result.is_cgi = true;
}

bool Router::GetHostAddr(std::string& server_addr) const {
  std::string h(sysconf(_SC_HOST_NAME_MAX), '\0');
  gethostname(&h[0], sysconf(_SC_HOST_NAME_MAX));
  struct hostent* host = gethostbyname(&h[0]);
  if (host == NULL) {
    return false;
  }
  server_addr = inet_ntoa(*(struct in_addr*)host->h_addr_list[0]);
  return true;
}

void Router::UpdateStatus(Result& result, int status) {
  result.success_path = result.error_path;
  result.method = GET;
  result.status = status;
}

// SECTION : Location
Location::Location(void)
    : error(false),
      autoindex(false),
      methods(GET),
      body_max(INT_MAX),
      root("/"),
      index(""),
      upload_path("/"),
      redirect_to("") {}

Location::Location(bool is_error, std::string error_path)
    : error(is_error), index(error_path) {}

// SECTION : LocationRouter
LocationRouter::LocationRouter(void) : error(true, "/error.html") {}

Location& LocationRouter::operator[](const std::string& path) {
  size_t pos = std::string::npos;
  size_t end_pos = path.rfind('/', pos);
  std::string target_path = path.substr(0, end_pos + 1);
  while (end_pos != std::string::npos) {
    if (location_map.count(target_path) == 1) {
      return location_map[target_path];
    }
    pos = end_pos;
    if (pos == 0) {
      break;
    }
    end_pos = path.rfind('/', pos - 1);
    target_path = path.substr(0, end_pos + 1);
  }
  return error;
}

// SECTION: ServerRouter
LocationRouter& ServerRouter::operator[](const std::string& host) {
  if (host.size() == 0) {
    return default_server;
  }
  std::string server_name = host.substr(0, host.find(':'));
  return (location_router_map.count(server_name) == 1)
             ? location_router_map[server_name]
             : default_server;
}
