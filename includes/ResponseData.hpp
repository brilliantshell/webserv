/**
 * @file ResponseData.hpp
 * @author ghan, jiskim, yongjule
 * @brief Global Map structures which store HTTP status information, MIME type
 * @date 2022-10-18
 *
 * @copyright Copyright (c) 2022
 */

#ifndef INCLUDES_RESPONSEDATA_HPP_
#define INCLUDES_RESPONSEDATA_HPP_

#include <map>
#include <string>

/**
 * @brief 상태 코드 별 메세지 저장하는 구조체
 *
 */
struct StatusMap : public std::map<int, std::string> {
  StatusMap(void) {
    // Successful
    this->operator[](200) = "OK";
    this->operator[](201) = "Created";
    this->operator[](202) = "Accepted";
    this->operator[](203) = "Non-Authoritative Information";
    this->operator[](204) = "No Content";
    this->operator[](205) = "Reset Content";
    this->operator[](206) = "Partial Content";
    // Redirection
    this->operator[](300) = "Multiple Choices";
    this->operator[](301) = "Moved Permanently";
    this->operator[](302) = "Found";
    this->operator[](303) = "See Other";
    this->operator[](304) = "Not Modified";
    this->operator[](305) = "Use Proxy";
    this->operator[](307) = "Temporary Redirect";
    this->operator[](308) = "Permanent Redirect";
    // Client Error
    this->operator[](400) = "Bad Request";
    this->operator[](401) = "Unauthorized";
    this->operator[](402) = "Payment Required";
    this->operator[](403) = "Forbidden";
    this->operator[](404) = "Not Found";
    this->operator[](405) = "Method Not Allowed";
    this->operator[](406) = "Not Acceptable";
    this->operator[](407) = "Proxy Authentication Required";
    this->operator[](408) = "Request Timeout";
    this->operator[](409) = "Conflict";
    this->operator[](410) = "Gone";
    this->operator[](411) = "Length Required";
    this->operator[](412) = "Precondition Failed";
    this->operator[](413) = "Content Too Large";
    this->operator[](414) = "URI Too Long";
    this->operator[](415) = "Unsupported Media Type";
    this->operator[](416) = "Range Not Satisfiable";
    this->operator[](417) = "Expectation Failed";
    this->operator[](421) = "Misdirected Request";
    this->operator[](422) = "Unprocessable Content";
    this->operator[](426) = "Upgrade Required";
    // Server Error
    this->operator[](500) = "Internal Server Error";
    this->operator[](501) = "Not Implemented";
    this->operator[](502) = "Bad Gateway";
    this->operator[](503) = "Service Unavailable";
    this->operator[](504) = "Gateway Timeout";
    this->operator[](505) = "HTTP Version Not Supported";
  }
};

/**
 * @brief 확장자 별 MIME type 저장하는 구조체
 *
 */
struct MimeMap : public std::map<std::string, std::string> {
  MimeMap(void) {
    this->operator[]("htm") = "text/html;charset=utf-8";
    this->operator[]("html") = "text/html;charset=utf-8";
    this->operator[]("shtml") = "text/html;charset=utf-8";
    this->operator[]("md") = "text/html;charset=utf-8";
    this->operator[]("css") = "text/css;charset=utf-8";
    this->operator[]("xml") = "text/xml;charset=utf-8";
    this->operator[]("txt") = "text/plain;charset=utf-8";
    this->operator[]("mml") = "text/mathml";
    this->operator[]("jad") = "text/vnd.sun.j2me.app-descriptor";
    this->operator[]("wml") = "text/vnd.wap.wml";
    this->operator[]("htc") = "text/x-component";
    this->operator[]("gif") = "image/gif";
    this->operator[]("jpeg") = "image/jpeg";
    this->operator[]("jpg") = "image/jpeg";
    this->operator[]("js") = "application/javascript";
    this->operator[]("atom") = "application/atom+xml";
    this->operator[]("rss") = "application/rss+xml";
    this->operator[]("avif") = "image/avif";
    this->operator[]("png") = "image/png";
    this->operator[]("svg") = "image/svg+xml";
    this->operator[]("svgz") = "image/svg+xml";
    this->operator[]("tif") = "image/tiff";
    this->operator[]("tiff") = "image/tiff";
    this->operator[]("wbmp") = "image/vnd.wap.wbmp";
    this->operator[]("webp") = "image/webp";
    this->operator[]("ico") = "image/x-icon";
    this->operator[]("jng") = "image/x-jng";
    this->operator[]("bmp") = "image/x-ms-bmp";
    this->operator[]("woff") = "font/woff";
    this->operator[]("woff2") = "font/woff2";
    this->operator[]("jar") = "application/java-archive";
    this->operator[]("war") = "application/java-archive";
    this->operator[]("ear") = "application/java-archive";
    this->operator[]("json") = "application/json";
    this->operator[]("hqx") = "application/mac-binhex40";
    this->operator[]("doc") = "application/msword";
    this->operator[]("pdf") = "application/pdf";
    this->operator[]("ps") = "application/postscript";
    this->operator[]("eps") = "application/postscript";
    this->operator[]("ai") = "application/postscript";
    this->operator[]("rtf") = "application/rtf";
    this->operator[]("m3u8") = "application/vnd.apple.mpegurl";
    this->operator[]("kml") = "application/vnd.google-earth.kml+xml";
    this->operator[]("kmz") = "application/vnd.google-earth.kmz";
    this->operator[]("xls") = "application/vnd.ms-excel";
    this->operator[]("eot") = "application/vnd.ms-fontobject";
    this->operator[]("ppt") = "application/vnd.ms-powerpoint";
    this->operator[]("odg") = "application/vnd.oasis.opendocument.graphics";
    this->operator[]("odp") = "application/vnd.oasis.opendocument.presentation";
    this->operator[]("ods") = "application/vnd.oasis.opendocument.spreadsheet";
    this->operator[]("odt") = "application/vnd.oasis.opendocument.text";
    this->operator[]("pptx") =
        "application/"
        "vnd.openxmlformats-officedocument.presentationml.presentation";
    this->operator[]("xlsx") =
        "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet";
    this->operator[]("docx") =
        "application/"
        "vnd.openxmlformats-officedocument.wordprocessingml.document";
    this->operator[]("wmlc") = "application/vnd.wap.wmlc";
    this->operator[]("wasm") = "application/wasm";
    this->operator[]("7z") = "application/x-7z-compressed";
    this->operator[]("cco") = "application/x-cocoa";
    this->operator[]("jardiff") = "application/x-java-archive-diff";
    this->operator[]("jnlp") = "application/x-java-jnlp-file";
    this->operator[]("run") = "application/x-makeself";
    this->operator[]("pl") = "application/x-perl ";
    this->operator[]("pm") = "application/x-perl";
    this->operator[]("prc") = "aplication/x-pilot";
    this->operator[]("pdb") = "application/x-";
    this->operator[]("rar") = "application/x-rar-compressed";
    this->operator[]("rpm") = "application/x-redhat-package-manager";
    this->operator[]("sea") = "application/x-sea";
    this->operator[]("swf") = "application/x-shockwave-flash";
    this->operator[]("sit") = "application/x-stuffit";
    this->operator[]("tcl") = "application/x-tcl";
    this->operator[]("tk") = "application/x-tcl";
    this->operator[]("der") = "aplication/x-x509-ca-cer";
    this->operator[]("pem") = "application/x-x509-ca-cert";
    this->operator[]("crt") = "application/x-x509-ca-cert";
    this->operator[]("xpi") = "application/x-xpinstall";
    this->operator[]("xhtml") = "application/xhtml+xml";
    this->operator[]("xspf") = "application/xspf+xml";
    this->operator[]("zip") = "application/zip";
    this->operator[]("bin") = "application/octet-stream";
    this->operator[]("exe") = "application/octet-stream";
    this->operator[]("dll") = "application/octet-stream";
    this->operator[]("deb") = "application/octet-stream";
    this->operator[]("dmg") = "application/octet-stream";
    this->operator[]("iso") = "application/octet-stream";
    this->operator[]("img") = "application/octet-stream";
    this->operator[]("msi") = "application/octet-stream";
    this->operator[]("msp") = "application/octet-stream";
    this->operator[]("msm") = "application/octet-stream";
    this->operator[]("mid") = "audio/midi";
    this->operator[]("midi") = "audio/midi";
    this->operator[]("kar") = "audio/midi";
    this->operator[]("mp3") = "audio/mpeg";
    this->operator[]("ogg") = "audio/ogg";
    this->operator[]("m4a") = "audio/x-m4a";
    this->operator[]("ra") = "audio/x-realaudio";
    this->operator[]("3gpp") = "video/3gpp";
    this->operator[]("3gp") = "video/3gpp";
    this->operator[]("ts") = "video/mp2t";
    this->operator[]("mp4") = "video/mp4";
    this->operator[]("mpeg") = "video/mpeg";
    this->operator[]("mpg") = "video/mpeg";
    this->operator[]("mov") = "video/quicktime";
    this->operator[]("webm") = "video/webm";
    this->operator[]("flv") = "video/x-flv";
    this->operator[]("m4v") = "video/x-m4v";
    this->operator[]("mng") = "video/x-mng";
    this->operator[]("asx") = "video/x-ms-asf";
    this->operator[]("asf") = "video/x-ms-asf";
    this->operator[]("wmv") = "video/x-ms-wmv";
    this->operator[]("avi") = "video/x-msvideo";
  }
};

extern StatusMap g_status_map;
extern MimeMap g_mime_map;

#endif  // INCLUDES_RESPONSEDATA_HPP_
