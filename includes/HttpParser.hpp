/**
 * @file HttpParser.hpp
 * @author ghan, jiskim, yongjule
 * @brief Parse HTTP request
 * @date 2022-09-27
 *
 * @copyright Copyright (c) 2022
 */

#ifndef INCLUDES_HTTPPARSER_HPP_
#define INCLUDES_HTTPPARSER_HPP_

#include <iostream>

#include "PathResolver.hpp"
#include "Types.hpp"
#include "UriParser.hpp"

#define CRLF "\r\n"
#define SP " "

#define BUFFER_SIZE 4096

// HTTP request 길이 제한
#define METHOD_MAX 6
#define REQUEST_PATH_MAX 8192
#define HTTP_VERSION_MAX 8
#define REQUEST_LINE_MAX 8208  // 8192 + 7(method max + 1) + 9(version max + 1)

#define FIELD_NAME_MAX 64
#define FIELD_VALUE_MAX 8192
#define HEADER_MAX 16384  // 16KB

#define BODY_MAX 134217728     // 128MB
#define REQUEST_MAX 134244368  // 128MB + 16KB + 10KB

#define CHUNKED std::numeric_limits<size_t>::max() - 1

#define CHUNKED_SIZE_LINE_MAX 1024
#define CHUNK_SIZE_MAX 8192

class HttpParser {
 public:
  enum { kHttp1_0 = 0, kHttp1_1 };
  enum {
    kLeadingCRLF = 0,
    kRequestLine,
    kHeader,
    kContent,
    kComplete,
    kClose,
    kRLLenErr,
    kHDLenErr,
    kBDLenErr
  };

  struct Result {
    int status;
    Request request;

    Result(void) : status(200) {}
  };

  HttpParser(void);

  int Parse(std::string& segment);
  bool DoesNextReqExist(void);
  void Clear(void);
  void Reset(void);
  Result& get_result(void);

 private:
  enum { kChunkSize = 0, kChunkData, kChunkEnd };

  bool keep_alive_;
  uint8_t is_data_;
  int status_;
  size_t body_length_;
  size_t chunk_size_;
  std::string request_line_buf_;
  std::string header_buf_;
  std::string chunked_buf_;
  std::string backup_buf_;
  Result result_;

  void SkipLeadingCRLF(std::string& segment);

  // Parse request line
  void ReceiveRequestLine(std::string& segment);
  void ParseRequestLine(void);
  void TokenizeMethod(size_t& pos);
  void TokenizePath(size_t& pos);
  void TokenizeVersion(size_t& pos);

  // Parse header
  void ReceiveHeader(std::string& segment);
  void ParseHeader(void);
  std::string TokenizeFieldName(size_t& cursor);
  void TokenizeFieldValueList(size_t& cursor, std::string& name);
  void SkipWhiteSpace(size_t& cursor);

  void ValidateHost(void);
  void DetermineBodyLength(void);
  void ParseContentLength(std::list<std::string>& content_length);
  void ParseTransferEncoding(std::list<std::string>& encodings);
  void ParseFieldValueList(std::list<std::string>& field_value_list,
                           std::map<std::string, size_t>& valid_map,
                           int no_match_status, char delim);
  void ValidateConnection(void);

  void UpdateStatus(int http_status, int parser_status);

  template <typename InputIterator>
  std::map<std::string, size_t> GenerateValidValueMap(InputIterator first,
                                                      InputIterator last);

  // parse body
  void ReceiveContent(std::string& segment);
  void DecodeChunkedContent(std::string& segment);
  bool ParseChunkData(void);
  bool ParseChunkSize(void);
  void ParseChunkEnd(void);
  bool IgnoreChunkExtension(std::string& chunk_size_line);
};

#endif  // INCLUDES_HTTPPARSER_HPP_
