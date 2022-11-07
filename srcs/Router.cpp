/**
 * @file Router.cpp
 * @author ghan, jiskim, yongjule
 * @brief Route a request to a path
 * @date 2022-10-07
 *
 * @copyright Copyright (c) 2022
 */

#include "Router.hpp"

// SECTION : Location
/**
 * @brief Validator 가 검증한 Config 파일에서 가져온 Location 블록 정보 담는
 * Location 객체 생성
 *
 */
Location::Location(void)
    : error(false),
      autoindex(false),
      methods(GET),
      body_max(INT_MAX),
      root("/"),
      index(""),
      upload_path("/"),
      redirect_to("") {}

/**
 * @brief 에러 Location 생성
 *
 * @param is_error
 * @param error_path
 */
Location::Location(bool is_error, std::string error_path)
    : error(is_error), methods(GET), index(error_path) {}

// SECTION : LocationRouter
/**
 * @brief Config 의 서버 블록 안에 나열된 location 블록, CGI 블록 정보들을
 * 저장하는 LocationRouter 객체 생성
 *
 */
LocationRouter::LocationRouter(void) : error(true, "/error.html") {}

/**
 * @brief LocationRouter 객체 안에 저장된 location 블록 정보 중 요청의 path와
 * 가장 일치하는 Location 객체를 반환
 *
 * @param kPath 요청 path
 * @return std::pair<Location&, size_t> 찾은 Location 객체, location 블록 path
 * 의 사이즈
 */
std::pair<Location&, size_t> LocationRouter::operator[](
    const std::string& kPath) {
  size_t end_pos = kPath.rfind('/');
  std::string target_path = kPath.substr(0, end_pos + 1);
  while (end_pos != std::string::npos) {
    LocationMap::iterator it = location_map.find(target_path);
    if (it != location_map.end()) {
      return std::pair<Location&, size_t>(it->second, target_path.size());
    }
    if (end_pos == 0) {
      break;
    }
    end_pos = kPath.rfind('/', end_pos - 1);
    target_path.assign(kPath, 0, end_pos + 1);
  }
  return std::pair<Location&, size_t>(error, 0);
}

// SECTION : ServerRouter
/**
 * @brief ServerRouter 객체 안에 저장된 서버 블록 중 요청의 host 에게 할당 된
 * LocationRouter 반환
 *
 * @param kHost HTTP 요청의 host
 * @return LocationRouter& 요청의 host 에게 할단 된 서버 블록의 LocationRouter,
 * 할당되어 있지 않은 경우 default_server 반환
 */
LocationRouter& ServerRouter::operator[](const std::string& kHost) {
  if (kHost.size() == 0) {
    return default_server;
  }
  std::string server_name(kHost, 0, kHost.find(':'));
  LocationRouterMap::iterator it = location_router_map.find(server_name);
  return (it != location_router_map.end()) ? it->second : default_server;
}

// SECTION : Router
/**
 * @brief HTTP 요청의 target URI 에 맞는 location 정보 반환하는 Router 개
 *
 * @param server_router 서버 블록들을 저장하고 있는 ServerRouter 객체
 */
Router::Router(ServerRouter& server_router) : server_router_(server_router) {}

/**
 * @brief 요청 정보를 이용하여 host 로 해당하는 서버 블록 (LocationRouter) 을
 * 찾고, 그 안에서 path 로 location 혹은 CGI 블록 (Location) 을 찾아 Result
 * 객체로 반환
 * @param status HttpParser 에서 판별한 status
 * @param request HTTP 요청 정보
 * @param connection_info Connection 정보
 * @return Router::Result 요청에 맞는 Location 정보
 */
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
    (cgi_discriminator.second == std::string::npos)
        ? RouteToLocation(result, location_router, request)
        : RouteToCgi(result, request, cgi_discriminator, connection_info);
  }
  return result;
}

// SECTION : private
/**
 * @brief CGI 요청인지 판별
 *
 * @param cgi_vector CGI 블록 정보들을 저장하고 있는
 * @param kPath 요청 path
 * @return Router::CgiDiscriminator CGI 요청이면 CGI 블록 정보와 요청 uri의 cgi
 * 확장자 위치, 아니면 pos == std::string::npos
 */
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

/**
 * @brief 정적 요청에 대한 라우팅
 *
 * @param result 라우팅 결과를 저장할 객체
 * @param location_router 요청의 host 에 해당하는 location_router 객체
 * @param request 클라이언트로부터 받은 요청
 */
void Router::RouteToLocation(Result& result, LocationRouter& location_router,
                             Request& request) {
  RequestLine& req = request.req;
  std::pair<Location&, size_t> location_data = location_router[req.path];
  Location& location = location_data.first;
  result.methods = location.methods;
  if (location.error == true) {
    return UpdateStatus(result, 404);  // Page Not Found
  }
  if ((location.methods & req.method) == 0) {
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
      ((req.method == POST) ? location.upload_path.substr(1) : "") +
      req.path.substr(location_data.second);
  if (*result.success_path.rbegin() == '/') {
    if (req.method & (POST | DELETE)) {
      return UpdateStatus(result, 403);  // Forbidden
    }
    result.success_path +=
        ((location.index.size() == 0 && location.autoindex == false)
             ? "index.html"
             : location.index);
  }
}

/**
 * @brief CGI 요청에 대한 라우팅
 *
 * @param result 라우팅 결과를 저장할 객체
 * @param request 클라이언트로부터 받은 요청
 * @param kCgiDiscriminator config 의 CGI 블록 정보와 요청 uri의 cgi 확장자 위치
 * @param kConnectionInfo 클라이언트와 서버의 정보를 담고 있는 구조체
 */
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

/**
 * @brief Default 서버에 대한 host 주소 설정
 *
 * @param server_addr host 주소를 저장할 string
 * @return true
 * @return false
 */
bool Router::GetHostAddr(std::string& server_addr) const {
  std::string host(sysconf(_SC_HOST_NAME_MAX), '\0');
  gethostname(&host[0], sysconf(_SC_HOST_NAME_MAX));
  struct hostent* host_ent = gethostbyname(&host[0]);
  if (host_ent == NULL) {
    return false;
  }
  server_addr = inet_ntoa(*(struct in_addr*)host_ent->h_addr_list[0]);
  return true;
}

/**
 * @brief 에러시 HTTP 상태 업데이트 및 에러 페이지 경로 설정
 *
 * @param result Router 의 결과
 * @param status HTTP 상태
 */
void Router::UpdateStatus(Result& result, int status) {
  result.success_path = result.error_path;
  result.status = status;
}
