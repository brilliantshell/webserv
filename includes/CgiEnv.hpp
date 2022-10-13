/**
 * @file CgiEnv.hpp
 * @author ghan, jiskim, yongjule
 * @brief CGI metavariables holder
 * @date 2022-10-13
 *
 * @copyright Copyright (c) 2022
 */

#ifndef INCLUDES_CGIENV_HPP_
#define INCLUDES_CGIENV_HPP_

#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>

#include <algorithm>
#include <cstdlib>
#include <exception>
#include <sstream>
#include <string>

#include "HttpParser.hpp"
#include "Types.hpp"

class CgiEnv {
 public:
  CgiEnv(void);
  ~CgiEnv(void);
  const char** get_env(void) const;
  bool SetMetaVariables(
      Request& request, const std::string& root, const std::string& cgi_ext,
      const std::pair<uint16_t, std::string>& connection_info);

 private:
  struct ScriptUri {
    std::string path_info;
    std::string path_translated;
    std::string script_name;
  };

  char** env_;

  bool ParseScriptUriComponents(ScriptUri& script_uri,
                                const std::string& req_uri,
                                const std::string& root,
                                const std::string& cgi_ext) const;

  template <typename T>
  std::string IntToString(T value) const;

  bool GetHostAddr(std::string& server_addr) const;

  const char* set_env(const size_t idx, const std::string& key_value);
};

#endif  // INCLUDES_CGIENV_HPP_