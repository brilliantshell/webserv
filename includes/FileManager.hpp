/**
 * @file FileManager.hpp
 * @author ghan, jiskim, yongjule
 * @brief Execute HTTP methods on static files
 * @date 2022-10-29
 *
 * @copyright Copyright (c) 2022
 */

#ifndef INCLUDE_FILEMANAGER_HPP_
#define INCLUDE_FILEMANAGER_HPP_

#include "ResponseManager.hpp"

class FileManager : public ResponseManager {
 public:
  FileManager(bool is_keep_alive, ResponseBuffer& response,
              Router::Result& router_result, const Request& request);
  virtual ~FileManager(void);
  ResponseManager::IoFdPair Execute(void);
  const int get_file_fd(void) const;

 private:
  int in_fd_;
  int out_fd_;
  size_t write_offset_;
  std::string output_path_;  // POST output file path
  std::string& response_content_;

  // GET
  void Get(void);
  void ReadFile(void);

  // POST
  void Post(void);
  void WriteFile(const std::string& request_content);
  void FindValidOutputPath(std::string& success_path);

  // DELETE
  void Delete(void);

  // Utils
  std::string GenerateRedirectPage(const std::string& redirect_to);
  void CheckFileMode(void);
  void GenerateAutoindex(const std::string& path);
  bool DetermineFileType(const std::string& path, const dirent* ent,
                         std::vector<std::string>& dir_vector,
                         std::vector<std::string>& file_vector);
  void ListAutoindexFiles(std::vector<std::string>& paths);
};

#endif  // INCLUDE_FILEMANAGER_HPP_
