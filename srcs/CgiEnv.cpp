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
  env_ = new (std::nothrow) char *[18];
  if (env_ != NULL) {
    for (size_t i = 0; i < 18; ++i) {
      env_[i] = NULL;
    }
  }
}

CgiEnv::CgiEnv(const CgiEnv &kSrc) : env_(NULL) { *this = kSrc; }

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

CgiEnv &CgiEnv::operator=(const CgiEnv &kRhs) {
  if (env_ != NULL) {
    for (size_t i = 0; i < 18; ++i) {
      if (env_[i] != NULL) {
        delete[] env_[i];
        env_[i] = NULL;
      }
    }
  } else {
    env_ = new (std::nothrow) char *[18];
    if (env_ != NULL) {
      for (size_t i = 0; i < 18; ++i) {
        env_[i] = NULL;
      }
    }
  }
  for (size_t i = 0; i < 17; ++i) {
    if (kRhs.env_[i] == NULL) {
      env_[i] = NULL;
    } else {
      env_[i] = new (std::nothrow) char[strlen(kRhs.env_[i]) + 1];
      if (env_[i] == NULL) {
        for (; i > 0; --i) {
          if (env_[i - 1] != NULL) {
            delete[] env_[i - 1];
            env_[i - 1] = NULL;
          }
        }
        delete[] env_;
        env_ = NULL;
        return *this;
      }
      strcpy(env_[i], kRhs.env_[i]);
    }
  }
  return *this;
}

bool CgiEnv::SetMetaVariables(Request &request, const std::string &kRoot,
                              const std::string &kCgiExt,
                              const ConnectionInfo &kConnectionInfo) {
  ScriptUri script_uri;
  if (ParseScriptUriComponents(script_uri, request.req.path, kRoot, kCgiExt) ==
      false) {
    return false;
  }
  const std::string kKeyValue[17] = {
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
      "REMOTE_ADDR=" + kConnectionInfo.client_addr,
      "REMOTE_HOST=" + kConnectionInfo.client_addr,
      "REMOTE_IDENT=",
      "REMOTE_USER=",
      "REQUEST_METHOD=" + std::string((request.req.method == GET) ? "GET"
                                      : (request.req.method == POST)
                                          ? "POST"
                                          : "DELETE"),
      "SCRIPT_NAME=" + script_uri.script_name,
      "SERVER_NAME=" + kConnectionInfo.server_name,
      "SERVER_PORT=" + IntToString(kConnectionInfo.server_port),
      "SERVER_PROTOCOL=" +
          std::string((request.req.version == HttpParser::kHttp1_0)
                          ? "HTTP/1.0"
                          : "HTTP/1.1"),
      "SERVER_SOFTWARE=BrilliantServer/1.0",
  };
  for (size_t i = 0; i < 17; ++i) {
    if (set_env(i, kKeyValue[i]) == NULL) {
      return false;
    }
  }
  return true;
}

// SECTION: private
bool CgiEnv::ParseScriptUriComponents(ScriptUri &script_uri,
                                      const std::string &kReqUri,
                                      const std::string &kRoot,
                                      const std::string &kCgiExt) const {
  size_t ext_dot = kReqUri.find(kCgiExt);
  script_uri.path_info = kReqUri.substr(ext_dot + kCgiExt.size());
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
  script_uri.path_translated =
      script_uri.path_info.size() > 0
          ? cwd + kRoot + script_uri.path_info.substr(1)
          : "";
  script_uri.script_name =
      kRoot + kReqUri.substr(1, ext_dot + kCgiExt.size() - 1);
  return true;
}

template <typename T>
std::string CgiEnv::IntToString(T value) const {
  std::stringstream ss;
  ss << value;
  return ss.str();
}

const char *CgiEnv::set_env(const size_t kIdx, const std::string &kKeyValue) {
  env_[kIdx] = new (std::nothrow) char[kKeyValue.size() + 1];
  if (env_[kIdx] != NULL) {
    std::fill(env_[kIdx], env_[kIdx] + kKeyValue.size() + 1, '\0');
    std::copy(kKeyValue.begin(), kKeyValue.end(), env_[kIdx]);
  }
  return const_cast<const char *>(env_[kIdx]);
}
