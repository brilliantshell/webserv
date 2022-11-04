/**
 * @file Connection.hpp
 * @author ghan, jiskim, yongjule
 * @brief Manage connection between client and server
 * @date 2022-09-27
 *
 * @copyright Copyright (c) 2022
 */

#ifndef INCLUDES_CONNECTION_HPP_
#define INCLUDES_CONNECTION_HPP_

#include <fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

#include <cstring>
#include <queue>
#include <string>

#include "CgiManager.hpp"
#include "FileManager.hpp"
#include "HeaderFormatter.hpp"
#include "HttpParser.hpp"
#include "PathResolver.hpp"
#include "Router.hpp"

#define PRINT_REQ_LOG(host, path)                                 \
  std::cout << "----------- Request -----------\nHost : " << host \
            << "\nPath : " << path << '\n';

// connection status
#define KEEP_ALIVE 0
#define KEEP_READING 1
#define NEXT_REQUEST_EXISTS 2
#define CLOSE 3
#define CONNECTION_ERROR 4

// send status
#define KEEP_SENDING 0
#define SEND_NEXT 1
#define SEND_FINISHED 2

#define SEND_BUFF_SIZE 32768  // 32KB

class Connection {
 public:
  Connection(void);
  ~Connection(void);

  void Reset(bool does_next_req_exist);
  void Clear(void);

  ResponseManager::IoFdPair HandleRequest(void);
  ResponseManager::IoFdPair ExecuteMethod(int event_fd);

  void Send(void);

  void SetAttributes(const int kFd, const std::string& kClientAddr,
                     const uint16_t kPort, ServerRouter& server_router);

  bool IsResponseBufferReady(void) const;
  bool IsHttpPairSynced(void) const;

  int get_send_status(void) const;
  int get_connection_status(void) const;

 private:
  typedef std::queue<ResponseBuffer> ResponseQueue;

  class ResponseManagerMap : public std::map<int, ResponseManager*> {
   public:
    ~ResponseManagerMap(void) { Clear(); }

    void Clear(void) {
      std::set<ResponseManager*> manager_set;
      for (iterator it = begin(); it != end(); ++it) {
        if (manager_set.count(it->second) == 0) {
          manager_set.insert(it->second);
          delete it->second;
        }
      }
      clear();
    }

    void Erase(ResponseManager* manager) {
      for (iterator it = begin(); it != end();) {
        iterator tmp_it = it;
        ++it;
        if (tmp_it->second == manager) {
          erase(tmp_it);
        }
      }
    }
  };

  int fd_;  // connected socket fd
  int connection_status_;
  int send_status_;
  uint16_t port_;
  std::string client_addr_;
  std::string buffer_;
  ResponseQueue response_queue_;
  ResponseManagerMap response_manager_map_;

  HttpParser parser_;
  Router* router_;

  ssize_t Receive(void);
  void DetermineIoComplete(ResponseManager::IoFdPair& io_fds,
                           ResponseManager* manager);

  ResponseManager* GenerateResponseManager(bool is_keep_alive, Request& request,
                                           Router::Result& router_result,
                                           ResponseBuffer& response_buffer);

  ResponseManager::IoFdPair HandleCgiLocalRedirection(
      ResponseManager** manager, std::string& local_redir_path);
  int ValidateLocalRedirPath(std::string& path, RequestLine& req);

  size_t SetIov(struct iovec* iov, ResponseBuffer& res);

  template <typename T>
  T SetConnectionError(const std::string& kMsg);
};

#endif  // INCLUDES_CONNECTION_HPP_
