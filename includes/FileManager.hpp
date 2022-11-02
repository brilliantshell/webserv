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
  void WriteFile(const std::string& request_content);
  void FindValidOutputPath(std::string& success_path);

  // DELETE
  void Delete(void);

  // Utils
  ResponseManager::IoFdPair GenerateRedirectPage(void);
  void CheckFileMode(void);
  void GenerateAutoindex(const std::string& path);
  bool DetermineFileType(const std::string& path, const dirent* ent,
                         std::vector<std::string>& dir_vector,
                         std::vector<std::string>& file_vector);
  void ListAutoindexFiles(std::vector<std::string>& paths);
  ResponseManager::IoFdPair DetermineSuccessFileExt(void);
  int SetIoComplete(int status);
};

#endif  // INCLUDES_FILEMANAGER_HPP_
