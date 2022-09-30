/**
 * @file ParseUtils.hpp
 * @author ghan, jiskim, yongjule
 * @brief Parse utils
 * @date 2022-09-29
 *
 * @copyright Copyright (c) 2022
 */

#ifndef INCLUDES_PARSEUTILS_HPP_
#define INCLUDES_PARSEUTILS_HPP_

#include <string>

#define CRLF "\r\n"
#define SP " "
#define HTAB "\t"

#define UPPER_ALPHA "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
#define LOWER_ALPHA "abcdefghijklmnopqrstuvwxyz"
#define ALPHA UPPER_ALPHA LOWER_ALPHA
#define DIGIT "0123456789"
#define HEXDIG "0123456789ABCDEFabcdef"
#define UNRESERVED ALPHA DIGIT "-._~"
#define SUB_DELIMS "!$&'()*+,;="
#define PCHAR UNRESERVED SUB_DELIMS ":@"
#define TCHAR ALPHA DIGIT "!#$%&'*+-.^_`|~"
#define VCHAR ALPHA DIGIT "!#$%&'()*+,-./:;<=>?@[\\]^_`{|}~"

class IsCharSet {
 public:
  /**
   * @brief Construct a new Is Char Set object
   *
   * @param char_set : character set to find
   * @param is_true : true면 char_set을 만나면 return, false면 char_set이
   * 아닌 것을 만나면 return.
   */
  IsCharSet(const std::string& char_set, const bool is_true)
      : kCharSet_(char_set), kIsTrue_(is_true) {}
  bool operator()(char c) const {
    return !((kCharSet_.find(c) != std::string::npos) ^ kIsTrue_);
  }

 private:
  const std::string kCharSet_;
  const bool kIsTrue_;
};

#endif  // INCLUDES_PARSEUTILS_HPP_
