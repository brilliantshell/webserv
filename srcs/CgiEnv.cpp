/**
 * @file CgiEnv.cpp
 * @author ghan, jiskim, yongjule
 * @brief CGI metavariables holder
 * @date 2022-10-13
 *
 * @copyright Copyright (c) 2022
 */

#include "CgiEnv.hpp"

CgiEnv::CgiEnv(void) {
  try {
    env_ = new char *[18];
    for (size_t i = 0; i < 18; ++i) {
      env_[i] = NULL;
    }
  } catch (const std::exception &e) {
    env_ = NULL;
  }
}

CgiEnv::~CgiEnv(void) {
  if (env_ != NULL) {
    for (size_t i = 0; i < 18; ++i) {
      if (env_[i] != NULL) {
        delete[] env_[i];
      }
    }
    delete[] env_;
  }
}

const char **CgiEnv::get_env(void) const {
  return const_cast<const char **>(env_);
}

// private

bool CgiEnv::SetMetaVariables(Request &request, const std::string &root,
                              const std::string &cgi_ext,
                              const ConnectionInfo &connection_info) {
  ScriptUri script_uri;
  if (ParseScriptUriComponents(script_uri, request.req.path, root, cgi_ext) ==
      false) {
    return false;
  }
  const std::string key_value[17] = {
      "AUTH_TYPE=",
      "CONTENT_LENGTH=" + ((request.content.size() > 0)
                               ? IntToString(request.content.size())
                               : ""),
      "CONTENT_TYPE=" + ((request.header.count("content-type") > 0)
                             ? request.header["content-type"].front()
                             : ""),
      "GATEWAY_INTERFACE=CGI/1.1",
      "PATH_INFO=" + script_uri.path_info,
      "PATH_TRANSLATED=" + script_uri.path_translated,
      "QUERY_STRING=" + request.req.query,
      "REMOTE_ADDR=" + connection_info.client_addr,
      "REMOTE_HOST=" + connection_info.client_addr,
      "REMOTE_IDENT=",
      "REMOTE_USER=",
      "REQUEST_METHOD=" + std::string((request.req.method == GET) ? "GET"
                                      : (request.req.method == POST)
                                          ? "POST"
                                          : "DELETE"),
      "SCRIPT_NAME=" + script_uri.script_name,
      "SERVER_NAME=" + connection_info.server_name,
      "SERVER_PORT=" + IntToString(connection_info.server_port),
      "SERVER_PROTOCOL=" +
          std::string((request.req.version == HttpParser::kHttp1_0)
                          ? "HTTP/1.0"
                          : "HTTP/1.1"),
      "SERVER_SOFTWARE=BrilliantServer/1.0",
  };
  for (size_t i = 0; i < 17; ++i) {
    if (set_env(i, key_value[i]) == NULL) {
      return false;
    }
  }
  return true;
}

bool CgiEnv::ParseScriptUriComponents(ScriptUri &script_uri,
                                      const std::string &req_uri,
                                      const std::string &root,
                                      const std::string &cgi_ext) const {
  size_t ext_dot = req_uri.find(cgi_ext);
  size_t script_start = req_uri.rfind('/', ext_dot);
  script_uri.path_info = req_uri.substr(ext_dot + cgi_ext.size());
  char *cwd;
  {
    char proc_name[PROC_PIDPATHINFO_MAXSIZE + 1];
    memset(proc_name, 0, PROC_PIDPATHINFO_MAXSIZE + 1);
    if (proc_pidpath(getpid(), proc_name, PROC_PIDPATHINFO_MAXSIZE) <= 0) {
      return false;
    }
    cwd = dirname(proc_name);
    if (cwd == NULL) {
      return false;
    }
  }
  script_uri.path_translated = script_uri.path_info.size() > 0
                                   ? cwd + root + script_uri.path_info.substr(1)
                                   : "";
  script_uri.script_name =
      root + req_uri.substr(1, ext_dot + cgi_ext.size() - 1);
  return true;
}

template <typename T>
std::string CgiEnv::IntToString(T value) const {
  std::stringstream ss;
  ss << value;
  return ss.str();
}

const char *CgiEnv::set_env(const size_t idx, const std::string &key_value) {
  try {
    env_[idx] = new char[key_value.size() + 1];
    std::fill(env_[idx], env_[idx] + key_value.size() + 1, '\0');
    std::copy(key_value.begin(), key_value.end(), env_[idx]);
  } catch (const std::exception &e) {
    env_[idx] = NULL;  // fail to allocate memory
  }
  return const_cast<const char *>(env_[idx]);
}
