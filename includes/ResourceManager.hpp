/**
 * @file ResourceManager.hpp
 * @author ghan, jiskim, yongjule
 * @brief Execute methods and manage resources according to the Client's request
 * @date 2022-10-11
 *
 * @copyright Copyright (c) 2022
 */

#ifndef INCLUDES_RESOURCE_MANAGER_HPP_
#define INCLUDES_RESOURCE_MANAGER_HPP_

#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

#include <fstream>
#include <sstream>

#include "CgiManager.hpp"
#include "ResponseData.hpp"
#include "Router.hpp"
#include "Types.hpp"
#include "UriParser.hpp"

#define LAST_ERROR_DOCUMENT                                               \
  "<!DOCTYPE html><title>500 Internal Server Error</title><body><h1>500 " \
  "Internal Server Error</ h1></ body></ html> "

class ResourceManager {
 public:
  struct Result {
    bool is_autoindex;
    bool is_local_redir;
    int status;
    std::string location;  // success or error path
    std::string ext;
    ResponseHeaderMap header;

    Result(int router_result_status)
        : is_autoindex(false),
          is_local_redir(false),
          status(router_result_status),
          location("") {}
  };

  Result ExecuteMethod(std::string& response_content,
                       Router::Result& router_result,
                       const Request& request_content);

 private:
  void HandleStaticRequest(std::string& response_content, Result& result,
                           Router::Result& router_result,
                           const Request& request);

  // GET
  void Get(std::string& response_content, Result& result,
           Router::Result& router_result);
  void GetErrorPage(std::string& response_content, Result& result,
                    Router::Result& router_result);

  // POST
  void Post(std::string& response_content, Result& result,
            Router::Result& router_result, const std::string& request_content);
  std::string FindValidOutputPath(Result& result, std::string& success_path);

  // DELETE
  void Delete(std::string& response_content, Result& result,
              Router::Result& router_result);

  // Utils
  std::string ParseExtension(const std::string& success_path);
  std::string GenerateRedirectPage(const std::string& redirect_to);
  void CheckFileMode(std::string& response_content, Result& result,
                     Router::Result& router_result);
  void GenerateAutoindex(std::string& response_content, Result& result,
                         const std::string& path);
  bool DetermineFileType(const std::string& path, const dirent* ent,
                         std::vector<std::string>& dir_vector,
                         std::vector<std::string>& file_vector);
  void ListAutoindexFiles(std::string& content,
                          std::vector<std::string>& paths);

  // CGI
  void HandleCgiRequest(std::string& response_content, Result& result,
                        Router::Result& router_result,
                        const std::string& request_content);
};

#endif  // INCLUDES_RESOURCE_MANAGER_HPP_
