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

#include <sys/stat.h>

#include <fstream>
#include <sstream>

#include "Router.hpp"

class ResourceManager {
 public:
  struct Result {
    int status;
    std::string content;
  };

  Result ExecuteMethod(Router::Result& route_result);

 private:
  void Get(Result& result, Router::Result& router_result);
  void GetErrorPage(Result& result, Router::Result& router_result);
  void CheckFileMode(Result& result, Router::Result& router_result);

  void Post(Result& result, Router::Result& router_result);

  void Delete(Result& result, Router::Result& router_result);
};

#endif  // INCLUDES_RESOURCE_MANAGER_HPP_
