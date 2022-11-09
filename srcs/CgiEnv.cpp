/**
 * @file CgiEnv.cpp
 * @author ghan, jiskim, yongjule
 * @brief CGI meta-variables holder
 * @date 2022-10-13
 *
 * @copyright Copyright (c) 2022
 */

#include "CgiEnv.hpp"

/**
 * @brief CGI meta-variables 담는 CgiEnv 객체 생성
 *
 */
CgiEnv::CgiEnv(void) { InitEnv(); }

/**
 * @brief 복사 생성자
 *
 * @param kSrc 복사할 CgiEnv 객체
 */
CgiEnv::CgiEnv(const CgiEnv &kSrc) : env_(NULL) { *this = kSrc; }

/**
 * @brief 할당 된 자원 정리 및 객체 소멸
 *
 */
CgiEnv::~CgiEnv(void) {
  if (env_ != NULL) {
    Clear();
  }
}

/**
 * @brief meta-variables 담고 있는 const char** 반환
 *
 * @return const char**
 */
const char **CgiEnv::get_env(void) const {
  return const_cast<const char **>(env_);
}

/**
 * @brief 대입 연산자
 *
 * @param kRhs 대입할 CgiEnv 객체
 * @return CgiEnv& 대입된 CgiEnv 객체
 */
CgiEnv &CgiEnv::operator=(const CgiEnv &kRhs) {
  if (env_ != NULL) {
    Clear();
  }
  InitEnv();
  for (size_t i = 0; i < 17; ++i) {
    if (kRhs.env_[i] == NULL) {
      env_[i] = NULL;
    } else {
      env_[i] = new (std::nothrow) char[strlen(kRhs.env_[i]) + 1];
      if (env_[i] == NULL) {
        Clear(i);
        return *this;
      }
      strcpy(env_[i], kRhs.env_[i]);
    }
  }
  return *this;
}

/**
 * @brief CGI meta-variables 설정
 *
 * @param request HTTP 요청 데이터
 * @param kRoot 요청이 속한 location 의 root 경로
 * @param kCgiExt CGI 확장자
 * @param kConnectionInfo 연결 정보
 * @return true
 * @return false
 */
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
      "SERVER_PORT=" + IntToString(kConnectionInfo.host_port.port),
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

// SECTION : private
/**
 * @brief PATH_INFO, PATH_TRANSLATED, SCRIPT_NAME 파싱
 *
 * @param script_uri 파싱된 결과를 담을 구조체
 * @param kReqUri HTTP 요청 URI
 * @param kRoot 요청이 속한 location 의 root 경로
 * @param kCgiExt CGI 확장자
 * @return true
 * @return false
 */
bool CgiEnv::ParseScriptUriComponents(ScriptUri &script_uri,
                                      const std::string &kReqUri,
                                      const std::string &kRoot,
                                      const std::string &kCgiExt) const {
  size_t ext_dot = kReqUri.find(kCgiExt);
  if (ext_dot == std::string::npos) {
    return false;
  }
  script_uri.path_info.assign(kReqUri, ext_dot + kCgiExt.size());
  char proc_name[PROC_PIDPATHINFO_MAXSIZE + 1];
  memset(proc_name, 0, PROC_PIDPATHINFO_MAXSIZE + 1);
  if (proc_pidpath(getpid(), proc_name, PROC_PIDPATHINFO_MAXSIZE) <= 0) {
    return false;
  }
  char *cwd = dirname(proc_name);
  if (cwd == NULL) {
    return false;
  }
  script_uri.path_translated =
      (script_uri.path_info.size() > 0)
          ? cwd + kRoot + script_uri.path_info.substr(1)
          : "";
  script_uri.script_name =
      kRoot + kReqUri.substr(1, ext_dot + kCgiExt.size() - 1);
  return true;
}

/**
 * @brief 정수를 문자열로 변환
 *
 * @tparam T 정수형 타입
 * @param value 문자열화할 정수
 * @return std::string 문자열화 된 정수
 */
template <typename T>
std::string CgiEnv::IntToString(T value) const {
  std::stringstream ss;
  ss << value;
  return ss.str();
}

/**
 * @brief env_ 배열에 CGI meta-variable 설정
 *
 * @param kIdx env_ 배열 인덱스
 * @param kKeyValue CGI meta-variable
 * @return const char* 실패시 NULL 반환
 */
const char *CgiEnv::set_env(const size_t kIdx, const std::string &kKeyValue) {
  env_[kIdx] = new (std::nothrow) char[kKeyValue.size() + 1];
  if (env_[kIdx] == NULL) {
    return NULL;
  }
  std::fill(env_[kIdx], env_[kIdx] + kKeyValue.size() + 1, '\0');
  std::copy(kKeyValue.begin(), kKeyValue.end(), env_[kIdx]);
  return const_cast<const char *>(env_[kIdx]);
}

/**
 * @brief env_ 문자열 배열 메모리 할당 및 초기화
 *
 */
void CgiEnv::InitEnv(void) {
  env_ = new (std::nothrow) char *[18];
  if (env_ != NULL) {
    for (size_t i = 0; i < 18; ++i) {
      env_[i] = NULL;
    }
  }
}

/**
 * @brief env_ 문자열 배열 메모리 해제 및 NULL 로 설정
 *
 * @param idx env_ 배열의 마지막 인덱스
 */
void CgiEnv::Clear(size_t idx) {
  for (; idx > 0; --idx) {
    if (env_[idx - 1] != NULL) {
      delete[] env_[idx - 1];
      env_[idx - 1] = NULL;
    }
  }
  delete[] env_;
  env_ = NULL;
}
