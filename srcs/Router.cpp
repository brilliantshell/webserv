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

// SECTION: private
Router::CgiDiscriminator Router::GetCgiLocation(
    LocationRouter::CgiVector& cgi_vector, const std::string& kPath) {
  LocationNode location_node;
  size_t pos = std::string::npos;
  for (size_t i = 0; i < cgi_vector.size(); ++i) {
    size_t latest = kPath.find(cgi_vector[i].first);
    if (latest < pos && kPath[latest - 1] != '/' &&
        (latest + cgi_vector[i].first.size() == kPath.size() ||
         kPath[latest + cgi_vector[i].first.size()] == '/')) {
      pos = latest;  // update cgi extension
      location_node = cgi_vector[i];
    }
  }
  return std::make_pair(location_node, pos);
}

void Router::RouteToLocation(Result& result, LocationRouter& location_router,
                             Request& request) {
  std::pair<Location&, size_t> location_data =
      location_router[request.req.path];
  Location& location = location_data.first;
  result.methods = location.methods;
  if (location.error == true) {
    return UpdateStatus(result, 404);  // Page Not Found
  }
  if ((location.methods & request.req.method) == 0) {
    return UpdateStatus(result, 405);  // Method Not Allowed
  }
  if (location.body_max < request.content.size()) {
    return UpdateStatus(result, 413);  // Request Entity Too Large
  }
  if (location.redirect_to.empty() == false) {
    result.redirect_to = location.redirect_to;
    result.status = 301;  // Moved Permanently
    return;
  }
  result.success_path =
      "." + location.root +
      ((request.req.method == POST) ? location.upload_path.substr(1) : "") +
      request.req.path.substr(location_data.second);
  if (*result.success_path.rbegin() == '/') {
    if (request.req.method & (POST | DELETE)) {
      return UpdateStatus(result, 403);  // Forbidden
    }
    result.success_path +=
        ((location.index.size() == 0 && location.autoindex == false)
             ? "index.html"
             : location.index);
  }
}

void Router::RouteToCgi(Result& result, Request& request,
                        const CgiDiscriminator& kCgiDiscriminator,
                        const ConnectionInfo& kConnectionInfo) {
  const std::string& kCgiExt = kCgiDiscriminator.first.first;
  const Location& kCgiLocation = kCgiDiscriminator.first.second;
  result.methods = kCgiLocation.methods;
  if ((kCgiLocation.methods & request.req.method) == 0) {
    return UpdateStatus(result, 405);  // Method Not Allowed
  }
  if (kCgiLocation.body_max < request.content.size()) {
    return UpdateStatus(result, 413);  // Request Entity Too Large
  }
  result.success_path =
      "." + kCgiLocation.root +
      request.req.path.substr(1, kCgiDiscriminator.second + kCgiExt.size() - 1);
  if (result.cgi_env.SetMetaVariables(request, kCgiLocation.root, kCgiExt,
                                      kConnectionInfo) == false) {
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
    : error(is_error), methods(GET), index(error_path) {}

// SECTION : LocationRouter
LocationRouter::LocationRouter(void) : error(true, "/error.html") {}

std::pair<Location&, size_t> LocationRouter::operator[](
    const std::string& kPath) {
  size_t pos = std::string::npos;
  size_t end_pos = kPath.rfind('/', pos);
  std::string target_path = kPath.substr(0, end_pos + 1);
  while (end_pos != std::string::npos) {
    if (location_map.count(target_path) == 1) {
      return std::pair<Location&, size_t>(location_map[target_path],
                                          target_path.size());
    }
    pos = end_pos;
    if (pos == 0) {
      break;
    }
    end_pos = kPath.rfind('/', pos - 1);
    target_path = kPath.substr(0, end_pos + 1);
  }
  return std::pair<Location&, size_t>(error, 0);
}

// SECTION: ServerRouter
LocationRouter& ServerRouter::operator[](const std::string& kHost) {
  if (kHost.size() == 0) {
    return default_server;
  }
  std::string server_name(kHost, 0, kHost.find(':'));
  return (location_router_map.count(server_name) == 1)
             ? location_router_map[server_name]
             : default_server;
}
