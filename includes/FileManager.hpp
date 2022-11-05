/**
 * @file FileManager.hpp
 * @author ghan, jiskim, yongjule
 * @brief Execute HTTP methods on static files
 * @date 2022-10-29
 *
 * @copyright Copyright (c) 2022
 */

#ifndef INCLUDES_FILEMANAGER_HPP_
#define INCLUDES_FILEMANAGER_HPP_

#include "ResponseManager.hpp"

#define FILE_IDX_MAX 100

class FileManager : public ResponseManager {
 public:
  FileManager(bool is_keep_alive, ResponseBuffer& response,
              Router::Result& router_result, Request& request);
  virtual ~FileManager(void);
  ResponseManager::IoFdPair Execute(void);

 private:
  int in_fd_;
  int out_fd_;
  size_t write_offset_;
  std::string output_path_;  // POST output file path
  std::string& response_content_;

  // GET
  void Get(void);

  // POST
  void Post(void);
  ssize_t WriteFile(const std::string& kReqContent);
  bool FindValidOutputPath(std::string& success_path);

  // DELETE
  void Delete(void);

  // Utils
  int OpenResource(const char* kPath, const int kFlags);
  ResponseManager::IoFdPair GenerateRedirectPage(void);
  void CheckFileMode(void);
  int GenerateAutoindex(const std::string& kPath);
  bool DetermineFileType(const std::string& kPath, const dirent* kEnt,
                         std::vector<std::string>& dir_vector,
                         std::vector<std::string>& file_vector);
  void ListAutoindexFiles(std::vector<std::string>& paths);
  ResponseManager::IoFdPair DetermineSuccessFileExt(void);
  int SetIoComplete(const int kStatus);
};

#endif  // INCLUDES_FILEMANAGER_HPP_
