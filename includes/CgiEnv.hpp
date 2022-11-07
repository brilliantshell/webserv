/**
 * @file CgiEnv.hpp
 * @author ghan, jiskim, yongjule
 * @brief CGI meta-variables holder
 * @date 2022-10-13
 *
 * @copyright Copyright (c) 2022
 */

#ifndef INCLUDES_CGIENV_HPP_
#define INCLUDES_CGIENV_HPP_

#include <arpa/inet.h>
#include <libgen.h>
#include <libproc.h>
#include <netdb.h>
#include <unistd.h>

#include <algorithm>

#include "HttpParser.hpp"
#include "Utils.hpp"

class CgiEnv {
 public:
  CgiEnv(void);
  CgiEnv(const CgiEnv& kSrc);
  ~CgiEnv(void);

  const char** get_env(void) const;
  CgiEnv& operator=(const CgiEnv& kRhs);
  bool SetMetaVariables(Request& request, const std::string& kRoot,
                        const std::string& kCgiExt,
                        const ConnectionInfo& kConnectionInfo);

 private:
  struct ScriptUri {
    std::string path_info;
    std::string path_translated;
    std::string script_name;
  };

  char** env_;

  bool ParseScriptUriComponents(ScriptUri& script_uri,
                                const std::string& kReqUri,
                                const std::string& kRoot,
                                const std::string& kCgiExt) const;

  template <typename T>
  std::string IntToString(T value) const;

  const char* set_env(const size_t kIdx, const std::string& kKeyValue);
  void InitEnv(void);
  void Clear(size_t idx = 18);
};

#endif  // INCLUDES_CGIENV_HPP_
