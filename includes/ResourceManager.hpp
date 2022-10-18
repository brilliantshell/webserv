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
#include "Router.hpp"
#include "Types.hpp"

class ResourceManager {
 public:
  struct Result {
    int status;
    std::string content;
    std::string location;
    ResponseHeaderMap header;
  };

  Result ExecuteMethod(Router::Result& route_result,
                       const std::string& request_content);

 private:
  void Get(Result& result, Router::Result& router_result);
  void GetErrorPage(Result& result, Router::Result& router_result);
  void CheckFileMode(Result& result, Router::Result& router_result);
  void GenerateAutoindex(Result& result, const std::string& path);
  void ListAutoindexFiles(std::string& content,
                          std::vector<std::string>& files);

  void Post(Result& result, Router::Result& router_result,
            const std::string& request_content);

  void Delete(Result& result, Router::Result& router_result);
};

#endif  // INCLUDES_RESOURCE_MANAGER_HPP_
