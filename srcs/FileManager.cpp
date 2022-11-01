/**
 * @file FileManager.cpp
 * @author ghan, jiskim, yongjule
 * @brief Execute HTTP methods on static files
 * @date 2022-10-29
 *
 * @copyright Copyright (c) 2022
 */

#include "FileManager.hpp"

FileManager::FileManager(bool is_keep_alive, ResponseBuffer& response,
                         Router::Result& router_result, Request& request)
    : ResponseManager(ResponseManager::kStatic, is_keep_alive, response,
                      router_result, request),
      in_fd_(-1),
      out_fd_(-1),
      write_offset_(0),
      response_content_(response.content) {}

FileManager::~FileManager(void) {
  close(in_fd_);
  close(out_fd_);
}

ResponseManager::IoFdPair FileManager::Execute(void) {
  if (router_result_.status >= 400) {
    return GetErrorPage();
  }
  if (router_result_.status == 301) {
    return GenerateRedirectPage();
  }
  if (io_status_ < ERROR_START) {
    if (request_.req.method == GET) {
      Get();
    } else if (request_.req.method == POST) {
      Post();
    } else {
      Delete();
    }
  }
  if (result_.status >= 400) {
    if (io_status_ < ERROR_START) {
      io_status_ = SetIoComplete(ERROR_START);
    }
    return GetErrorPage();
  }
  if (io_status_ == IO_COMPLETE) {
    return DetermineSuccessFileExt();
  }
  return (io_status_ == FILE_READ) ? ResponseManager::IoFdPair(in_fd_, -1)
                                   : ResponseManager::IoFdPair(-1, out_fd_);
}

// SECTION : private
// GET
void FileManager::Get(void) {
  if (io_status_ == IO_START) {
    CheckFileMode();
    if (result_.status < 400 && response_content_.empty() == false) {
      return;
    }
    if (result_.status < 400) {
      errno = 0;
      in_fd_ = open(router_result_.success_path.c_str(), O_RDONLY);
      if (in_fd_ == -1) {
        switch (errno) {
          case EACCES:
            result_.status = 403;  // FORBIDDEN
            return;
          case ENOENT:
            result_.status = 404;  // PAGE NOT FOUND
            return;
          case EMFILE:
            result_.status = 503;  // SERVICE UNAVAILABLE
            return;
          default:
            result_.status = 500;  // INTERNAL SERVER ERROR
            return;
        }
      }
      fcntl(in_fd_, F_SETFL, O_NONBLOCK);
      io_status_ = FILE_READ;
    }
  }
  if (io_status_ == FILE_READ) {
    ReadFile(in_fd_);
  }
}

// POST
void FileManager::Post(void) {
  if (io_status_ == IO_START) {
    FindValidOutputPath(router_result_.success_path);
    if (output_path_.empty()) {  // 403 or 500
      return;
    }
    errno = 0;
    out_fd_ = open(output_path_.c_str(), O_WRONLY | O_CREAT, 0644);
    if (out_fd_ == -1) {
      if (errno == ENOENT) {
        result_.status = 404;  // PAGE NOT FOUND
      } else if (errno == EACCES) {
        result_.status = 403;  // FORBIDDEN
      } else {
        result_.status = 500;  // INTERNAL SERVER ERROR}
      }
      return;
    }
    fcntl(out_fd_, F_SETFL, O_NONBLOCK);
    io_status_ = FILE_WRITE;
  }
  if (io_status_ == FILE_WRITE) {
    WriteFile(request_.content);
    if (result_.status == 500) {
      unlink(output_path_.c_str());
    }
  }
  if (io_status_ == IO_COMPLETE) {
    result_.status = 201;  // CREATED
    result_.location = output_path_.substr(1);
    response_content_ =
        "<!DOCTYPE html><html><title>201 Created</title><body><h1>201 "
        "Created</h1><p>YAY! The file is created at " +
        result_.location + "!</p><p>Have a nice day~</p></body></html>";
    close(out_fd_);
    out_fd_ = -1;
  }
}

void FileManager::WriteFile(const std::string& request_content) {
  ssize_t written_bytes =
      write(out_fd_, &request_content[write_offset_],
            (request_content.size() < WRITE_BUFFER_SIZE + write_offset_)
                ? request_content.size() - write_offset_
                : WRITE_BUFFER_SIZE);
  if (written_bytes < 0) {
    result_.status = 500;  // INTERNAL SERVER ERROR
    return;
  }
  write_offset_ += written_bytes;
  io_status_ =
      (write_offset_ >= request_content.size()) ? IO_COMPLETE : FILE_WRITE;
}

void FileManager::FindValidOutputPath(std::string& success_path) {
  size_t ext_start = success_path.rfind('.');
  std::string name = success_path;
  std::string ext("");
  if (ext_start > 0 && ext_start < name.size() - 1) {
    name = success_path.substr(0, ext_start);
    ext = success_path.substr(ext_start);
  }
  output_path_ = success_path;
  int i = 0;
  for (; i < 100; ++i) {
    if (access(output_path_.c_str(), F_OK) == -1) {
      if (errno != ENOENT) {
        result_.status = 500;  // INTERNAL_SERVER_ERROR
        return;
      }
      break;
    }
    std::stringstream ss;
    ss << name << "_" << i << ext;
    output_path_ = ss.str();
  }
  if (i == 100) {
    result_.status = 403;  // FORBIDDEN
    output_path_ = "";
  }
}

// DELETE
void FileManager::Delete(void) {
  errno = 0;
  if (access(router_result_.success_path.c_str(), W_OK) == -1) {
    if (errno == ENOENT) {
      result_.status = 404;  // PAGE NOT FOUND
    } else if (errno == EACCES) {
      result_.status = 403;  // FORBIDDEN
    } else {
      result_.status = 500;  // INTERNAL_SERVER_ERROR
    }
    return;
  }
  if (unlink(router_result_.success_path.c_str()) == -1) {
    result_.status = 500;  // INTERNAL_SERVER_ERROR
    return;
  }
  result_.status = 200;  // OK
  response_content_ =
      "<!DOCTYPE html><html><title>Deleted</title><body><h1>200 OK</h1><p>" +
      router_result_.success_path.substr(1) + " is removed!</p></body></html>";
  io_status_ = IO_COMPLETE;
}

// Utils
// Generate Redirection Message
ResponseManager::IoFdPair FileManager::GenerateRedirectPage(void) {
  result_.location = router_result_.redirect_to;
  response_content_ =
      "<!DOCTYPE html><html><title></title><body><h1>301 Moved "
      "Permanently</h1><p>The resource has been moved permanently to <a "
      "href='" +
      result_.location + "'>" + result_.location + "<a>.</p></body></html>";
  result_.ext = "html";
  io_status_ = IO_COMPLETE;
  return ResponseManager::IoFdPair(-1, -1);
}

void FileManager::CheckFileMode(void) {
  struct stat file_stat;
  errno = 0;
  if (stat(router_result_.success_path.c_str(), &file_stat) == -1) {
    result_.status =
        errno == ENOENT ? 404 : 500;  // NOT FOUND || INTERNAL_SERVER_ERROR
    return;
  }
  if ((file_stat.st_mode & S_IFMT) == S_IFDIR) {
    if (*(router_result_.success_path.rbegin()) == '/') {
      GenerateAutoindex(router_result_.success_path);
    } else {
      result_.status = 404;  // PAGE NOT FOUND
    }
  }
}

void FileManager::GenerateAutoindex(const std::string& path) {
  DIR* dir = opendir(path.c_str());
  if (dir == NULL) {
    result_.status = 500;  // INTERNAL_SERVER_ERROR
    return;
  }
  std::string index_of = "Index of " + path.substr(1);
  response_content_ = "<!DOCTYPE html><html><title>" + index_of +
                      "</title><body><h1>" + index_of + "</h1><hr><pre>\n";
  errno = 0;
  std::vector<std::string> dir_vector;
  std::vector<std::string> file_vector;
  for (dirent* ent = readdir(dir); ent != NULL; ent = readdir(dir)) {
    if (ent->d_name[0] != '.' &&
        DetermineFileType(path, ent, dir_vector, file_vector) == false) {
      break;
    }
  }
  if (errno != 0) {
    result_.status = 500;  // INTERNAL_SERVER_ERROR
  } else {
    ListAutoindexFiles(dir_vector);
    ListAutoindexFiles(file_vector);
    response_content_ += "</pre><hr></body></html>";
  }
  closedir(dir);
  result_.is_autoindex = true;
}

bool FileManager::DetermineFileType(const std::string& path, const dirent* ent,
                                    std::vector<std::string>& dir_vector,
                                    std::vector<std::string>& file_vector) {
  std::string file_name(ent->d_name);
  struct stat s_buf;
  memset(&s_buf, 0, sizeof(s_buf));
  if (ent->d_type == DT_LNK) {
    if (stat((path + file_name).c_str(), &s_buf) == -1) {
      return false;
    }
    S_ISDIR(s_buf.st_mode) ? dir_vector.push_back(file_name + "/")
                           : file_vector.push_back(file_name);
  } else {
    (ent->d_type == DT_DIR) ? dir_vector.push_back(file_name + "/")
                            : file_vector.push_back(file_name);
  }
  return true;
}

void FileManager::ListAutoindexFiles(std::vector<std::string>& paths) {
  std::sort(paths.begin(), paths.end());
  for (size_t i = 0; i < paths.size(); ++i) {
    std::string encoded_path(paths[i]);
    UriParser().EncodeAsciiToHex(encoded_path);
    response_content_ +=
        "<a href='./" + encoded_path + "'>" + paths[i] + "</a>\n";
  }
}

ResponseManager::IoFdPair FileManager::DetermineSuccessFileExt(void) {
  result_.ext =
      (result_.status == 201 ||
       ((request_.req.method & DELETE) == DELETE && result_.status == 200))
          ? "html"
          : ParseExtension(router_result_.success_path);
  return ResponseManager::IoFdPair(-1, -1);
}

int FileManager::SetIoComplete(int status) {
  close(in_fd_);
  close(out_fd_);
  in_fd_ = -1;
  out_fd_ = -1;
  return status;
}
