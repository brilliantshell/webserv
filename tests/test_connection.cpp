#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <cstring>
#include <fstream>
#include <iostream>

#include "ClientConnection.hpp"
#include "Connection.hpp"
#include "HttpParser.hpp"
#include "ResponseManager.hpp"
#include "Router.hpp"
#include "Utils.hpp"
#include "Validator.hpp"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

#define CN_CONFIG_PATH_PREFIX "../configs/tests/connection/"
#define CN_REQ_PATH_PREFIX "../tests/connection/"

std::string FileToString(const std::string& file_path);
ServerConfig TestValidatorSuccess(const std::string& case_id);

int listen_fd;

int SetUpPassiveSocket(uint16_t port) {
  sockaddr_in addr;

  memset(&addr, 0, sizeof(sockaddr_in));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr.s_addr = INADDR_ANY;
  int fd = socket(AF_INET, SOCK_STREAM, 0);
  if (fd < 0) {
    std::cerr << "Socket creation failed\n" << std::endl;
  }

  int opt = 1;
  if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
    std::cerr << "webserv : " << strerror(errno)
              << " : address cannot be reused" << '\n';
    return -1;
  }

  if (bind(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
    std::cerr << "Socket for " << port << " cannot be bound" << '\n';
    // close(fd);
    return -1;
  }
  listen(fd, 128);
  return fd;
}

void HandleClient(uint16_t port, const std::string& test_id,
                  const std::string& expected_content,
                  const std::vector<std::string>& expected_header) {
  ClientConnection client_connection;
  client_connection.Connect(port);
  client_connection.SendMessage(
      FileToString(CN_REQ_PATH_PREFIX + test_id + ".txt"));
  std::string response = client_connection.ReceiveMessage();

  size_t end_of_header = response.find(CRLF CRLF);
  if (end_of_header == std::string::npos) {
    _exit(EXIT_FAILURE);
  }
  std::string header = response.substr(0, end_of_header + 2);
  std::vector<std::string> header_lines;
  std::string token;
  size_t start = 0;
  for (size_t end = header.find(CRLF); end != std::string::npos;
       end = header.find(CRLF, start)) {
    header_lines.push_back(header.substr(start, end - start));
    start = end + 2;
  }
  if (header_lines.size() != expected_header.size()) {
    _exit(EXIT_FAILURE);
  }
  for (size_t i = 0; i < header_lines.size(); ++i) {
    if (i == 2) {
      if (header_lines[2].compare(0, 6, "date: ") != 0) {
        std::cerr << "Date field error\n\treceived: " << expected_header[i]
                  << "\n";
        _exit(EXIT_FAILURE);
      }
    } else if (header_lines[i] != expected_header[i]) {
      std::cerr << "Client expected\n\t" << header_lines[i] << " = "
                << expected_header[i] << "\nbut received " << header_lines[i]
                << "\n";
      _exit(EXIT_FAILURE);
    }
  }
  size_t delim = response.find(CRLF CRLF);
  if (delim == std::string::npos) {
    std::cerr << "No CRLF CRLF\n";
    _exit(EXIT_FAILURE);
  }
  response = response.substr(delim + 4);
  if (response != expected_content) {
    std::cerr << "content diff:\n\tresponse: " << response
              << "\n\texpected: " << expected_content << "\n";
    _exit(EXIT_FAILURE);
  }
  client_connection.~ClientConnection();
  _exit((response.compare(expected_content) == 0) ? EXIT_SUCCESS
                                                  : EXIT_FAILURE);
}

bool AcceptSetUpConnection(int listen_fd, Connection& connection,
                           HostPortMap& host_port_map, uint16_t port) {
  sockaddr_in client_addr;
  socklen_t addr_len = sizeof(sockaddr_in);
  int fd =
      accept(listen_fd, reinterpret_cast<sockaddr*>(&client_addr), &addr_len);
  if (fd == -1) {
    std::cerr << "Accept failed : " << strerror(errno) << '\n';
    return false;
  }
  connection.SetAttributes(fd, inet_ntoa(client_addr.sin_addr), port,
                           host_port_map[port]);
  if (connection.get_status() == CONNECTION_ERROR) {
    return false;
  }
  return true;
}

void TestConnection(const std::string& test_id, const uint16_t port,
                    std::vector<std::string>& expected_header,
                    const std::string& expected_response) {
  ServerConfig result = TestValidatorSuccess(CN_CONFIG_PATH_PREFIX + test_id);
  HostPortMap host_port_map = result.host_port_map;
  if (listen_fd > 0) {
    pid_t client_pid = fork();
    if (client_pid == 0) {
      HandleClient(port, test_id, expected_response, expected_header);
    } else if (client_pid == -1) {
      std::cerr << "Failed to fork child process :" << strerror(errno) << "\n";
      return;
    }
    Connection connection;
    if (AcceptSetUpConnection(listen_fd, connection, host_port_map, port) ==
        false) {
      kill(client_pid, SIGTERM);
    } else {
      connection.HandleRequest();
      int status = 0;
      ASSERT_EQ(waitpid(client_pid, &status, 0), client_pid);
      ASSERT_TRUE(WIFEXITED(status));
      EXPECT_EQ(static_cast<int>(WEXITSTATUS(status)), 0)
          << " >>> EXIT STATUS <<< \n";
    }
  } else {
    std::cerr << "Listen fd is not valid\n";
  }
}

TEST(ConnectionTest, SingleRequestViaConnection) {
  listen_fd = SetUpPassiveSocket(4242);
  // s_00 Connection general GET test
  {
    std::vector<std::string> expected_header = {
        "HTTP/1.1 200 OK",
        "server: BrilliantServer/1.0",
        "date: ",
        "allow: GET",
        "connection: keep-alive",
        "content-length: 88",
        "content-type: text/html;charset=utf-8",
    };
    std::string expected_response =
        FileToString(CN_REQ_PATH_PREFIX "s_00.html");
    TestConnection("s_00", 4242, expected_header, expected_response);
  }

  // s_01 general case - GET
  {
    std::vector<std::string> expected_header = {
        "HTTP/1.1 200 OK",
        "server: BrilliantServer/1.0",
        "date: ",
        "allow: GET",
        "connection: keep-alive",
        "content-length: 38",
        "content-type: text/css;charset=utf-8",
    };
    TestConnection("s_01", 4242, expected_header,
                   FileToString(CN_REQ_PATH_PREFIX "s_01.css"));
  }

  // NOTE : 0203 imagefile (too large)
  //// s 02 general case - GET
  //{
  //  std::vector<std::string> expected_header = {"HTTP/1.1 200 OK",
  //                                              "server:
  //                                              BrilliantServer/1.0", "date:
  //                                              ", "allow: GET, POST,
  //                                              DELETE", "connection:
  //                                              close", "content-length:
  //                                              8780", "content-type:
  //                                              image/png",};
  //  TestConnection("s_02", 4242, expected_header,
  //                 FileToString(CN_REQ_PATH_PREFIX "s_02.png"));
  //}

  //// s 03 general case - GET
  //{
  //  std::vector<std::string> expected_header = {"HTTP/1.1 200 OK",
  //                                              "server:
  //                                              BrilliantServer/1.0", "date:
  //                                              ", "allow: GET, POST,
  //                                              DELETE", "connection:
  //                                              close", "content-length:
  //                                              8780",};
  //  TestConnection("s_03", 4242, expected_header,
  //                 FileToString(CN_REQ_PATH_PREFIX "s_03.jiskim"));
  //}

  // f 00 GET 404
  {
    std::vector<std::string> expected_header = {
        "HTTP/1.1 404 Not Found",
        "server: BrilliantServer/1.0",
        "date: ",
        "connection: keep-alive",
        "content-length: 92",
        "content-type: text/html;charset=utf-8",
    };
    TestConnection("f_00", 4242, expected_header,
                   FileToString(CN_REQ_PATH_PREFIX "error.html"));
  }

  // f_01 GET 403
  {
    chmod("./rf_resources/f_01.html", 0222);
    std::vector<std::string> expected_header = {
        "HTTP/1.1 403 Forbidden",
        "server: BrilliantServer/1.0",
        "date: ",
        "allow: GET, POST, DELETE",
        "connection: keep-alive",
        "content-length: 92",
        "content-type: text/html;charset=utf-8",
    };
    TestConnection("f_01", 4242, expected_header,
                   FileToString(CN_REQ_PATH_PREFIX "error.html"));
    chmod("./rf_resources/f_01.html", 0777);
  }

  // f_02 GET 405
  {
    std::vector<std::string> expected_header = {
        "HTTP/1.1 405 Method Not Allowed",
        "server: BrilliantServer/1.0",
        "date: ",
        "allow: POST",
        "connection: keep-alive",
        "content-length: 92",
        "content-type: text/html;charset=utf-8",
    };
    TestConnection("f_02", 4242, expected_header,
                   FileToString(CN_REQ_PATH_PREFIX "error.html"));
  }

  // f_03 GET 505
  {
    std::vector<std::string> expected_header = {
        "HTTP/1.1 505 HTTP Version Not Supported",
        "server: BrilliantServer/1.0",
        "date: ",
        "connection: close",
        "content-length: 92",
        "content-type: text/html;charset=utf-8",
    };
    TestConnection("f_03", 4242, expected_header,
                   FileToString(CN_REQ_PATH_PREFIX "error.html"));
  }

  // f_04 GET 400
  {
    std::vector<std::string> expected_header = {
        "HTTP/1.1 400 Bad Request",
        "server: BrilliantServer/1.0",
        "date: ",
        "connection: close",
        "content-length: 92",
        "content-type: text/html;charset=utf-8",
    };
    TestConnection("f_04", 4242, expected_header,
                   FileToString(CN_REQ_PATH_PREFIX "error.html"));
  }

  // s 04 HTTP/1.1 autoindex GET request
  {
    std::vector<std::string> expected_header = {
        "HTTP/1.1 200 OK",
        "server: BrilliantServer/1.0",
        "date: ",
        "allow: GET",
        "connection: keep-alive",
        "content-length: 275",
        "content-type: text/html;charset=utf-8",
    };
    TestConnection("s_04", 4242, expected_header,
                   FileToString(CN_REQ_PATH_PREFIX "autoindex.html"));
  }

  // s_05 HTTP/1.2 GET request 200
  {
    std::vector<std::string> expected_header = {
        "HTTP/1.1 200 OK",
        "server: BrilliantServer/1.0",
        "date: ",
        "allow: GET",
        "connection: keep-alive",
        "content-length: 117",
        "content-type: text/html;charset=utf-8",
    };
    TestConnection("s_05", 4242, expected_header,
                   FileToString(CN_REQ_PATH_PREFIX "s_05.html"));
  }

  // s 06 redirection 301 (local redirection)
  {
    std::vector<std::string> expected_header = {
        "HTTP/1.1 301 Moved Permanently",
        "server: BrilliantServer/1.0",
        "date: ",
        "connection: keep-alive",
        "content-length: 191",
        "content-type: text/html;charset=utf-8",
        "location: /resources/s_00.html",
    };
    TestConnection("s_06", 4242, expected_header,
                   "<!DOCTYPE html><html><title></title><body><h1>301 Moved \
Permanently</h1><p>The \
resource has been moved permanently to <a href='/resources/s_00.html'>\
/resources/s_00.html<a>.</p></body></html>");
  }

  // s 07 redirection 301 (redirect by absolute URI)
  {
    std::vector<std::string> expected_header = {
        "HTTP/1.1 301 Moved Permanently",
        "server: BrilliantServer/1.0",
        "date: ",
        "connection: keep-alive",
        "content-length: 195",
        "content-type: text/html;charset=utf-8",
        "location: https://www.naver.com/",
    };
    TestConnection("s_07", 4242, expected_header,
                   "<!DOCTYPE html><html><title></title><body><h1>301 Moved \
Permanently</h1><p>The \
resource has been moved permanently to <a href='https://www.naver.com/'>\
https://www.naver.com/<a>.</p></body></html>");
  }

  // s 08 redirection 301 (redirect by absolute URI)
  {
    std::vector<std::string> expected_header = {
        "HTTP/1.1 301 Moved Permanently",
        "server: BrilliantServer/1.0",
        "date: ",
        "connection: keep-alive",
        "content-length: 243",
        "content-type: text/html;charset=utf-8",
        "location: https://www.naver.com:8080/search?query=legacy",
    };
    TestConnection("s_08", 4242, expected_header,
                   "<!DOCTYPE html><html><title></title><body><h1>301 Moved \
Permanently</h1><p>The \
resource has been moved permanently to <a href='https://www.naver.com:8080/search?query=legacy'>\
https://www.naver.com:8080/search?query=legacy<a>.</p></body></html>");
  }

  // s 09 redirection 301 (redirect by local uri with query)
  {
    std::vector<std::string> expected_header = {
        "HTTP/1.1 301 Moved Permanently",
        "server: BrilliantServer/1.0",
        "date: ",
        "connection: keep-alive",
        "content-length: 247",
        "content-type: text/html;charset=utf-8",
        "location: /resources/login?user=yongjule&password=julejule",
    };
    TestConnection("s_09", 4242, expected_header,
                   "<!DOCTYPE html><html><title></title><body><h1>301 Moved \
Permanently</h1><p>The \
resource has been moved permanently to <a href='/resources/login?user=yongjule&password=julejule'>\
/resources/login?user=yongjule&password=julejule<a>.</p></body></html>");
  }

  // s 10 POST
  {
    std::vector<std::string> expected_header = {
        "HTTP/1.1 201 Created",
        "server: BrilliantServer/1.0",
        "date: ",
        "allow: POST",
        "connection: keep-alive",
        "content-length: 173",
        "content-type: text/html;charset=utf-8",
        "location: /rf_resources/post/s_10.txt",
    };
    TestConnection(
        "s_10", 4242, expected_header,
        "<!DOCTYPE html><html><title>201 Created</title><body><h1>201 \
Created</h1><p>YAY! The file is created at \
/rf_resources/post/s_10.txt!</p><p>Have a nice day~</p></body></html>");
    unlink("./rf_resources/post/s_10.txt");
  }

  // s 11 upload already existing filename
  {
    std::vector<std::string> expected_header = {
        "HTTP/1.1 201 Created",
        "server: BrilliantServer/1.0",
        "date: ",
        "allow: POST",
        "connection: keep-alive",
        "content-length: 172",
        "content-type: text/html;charset=utf-8",
        "location: /rf_resources/post/empty_0",
    };
    TestConnection(
        "s_11", 4242, expected_header,
        "<!DOCTYPE html><html><title>201 Created</title><body><h1>201 \
Created</h1><p>YAY! The file is created at \
/rf_resources/post/empty_0!</p><p>Have a nice day~</p></body></html>");
    EXPECT_EQ(access("./cn_resources/post/empty_0", F_OK), 0);
    EXPECT_NE(unlink("./cn_resources/post/empty_0"), -1);
  }

  // f 05 post to unauthorized directory
  {
    if (access("./cn_resources/post/unauthorized", F_OK) == -1) {
      mkdir("./rf_resources/post/unauthorized", 0777);
    }
    EXPECT_NE(chmod("./rf_resources/post/unauthorized", 0555), -1);
    std::vector<std::string> expected_header = {
        "HTTP/1.1 403 Forbidden",
        "server: BrilliantServer/1.0",
        "date: ",
        "allow: POST",
        "connection: keep-alive",
        "content-length: 92",
        "content-type: text/html;charset=utf-8",
    };
    TestConnection("f_05", 4242, expected_header,
                   FileToString(CN_REQ_PATH_PREFIX "error.html"));
    EXPECT_NE(chmod("./cn_resources/post/unauthorized", 0777), -1);
  }

  // f 06 post to out of root
  {
    std::vector<std::string> expected_header = {
        "HTTP/1.1 400 Bad Request",
        "server: BrilliantServer/1.0",
        "date: ",
        "connection: close",
        "content-length: 92",
        "content-type: text/html;charset=utf-8",
    };
    TestConnection("f_06", 4242, expected_header,
                   FileToString(CN_REQ_PATH_PREFIX "error.html"));
  }

  // s 12 Delete a file
  {
    {
      std::ofstream ofs("./rf_resources/delete/yongjule");
      ofs << "I'm gonna suicide\n";
    }
    std::vector<std::string> expected_header = {
        "HTTP/1.1 200 OK",
        "server: BrilliantServer/1.0",
        "date: ",
        "allow: DELETE",
        "connection: keep-alive",
        "content-length: 126",
        "content-type: text/html;charset=utf-8",
    };
    TestConnection(
        "s_12", 4242, expected_header,
        "<!DOCTYPE html><html><title>Deleted</title><body><h1>200 OK</h1><p>"
        "/rf_resources/delete/yongjule"
        " is removed!</p></body></html>");
    ASSERT_EQ(access("./cn_resources/delete/yongjule", F_OK), -1);
  }

  // f 07 deleting a file that doesn't exist
  {
    std::vector<std::string> expected_header = {
        "HTTP/1.1 404 Not Found",
        "server: BrilliantServer/1.0",
        "date: ",
        "connection: keep-alive",
        "content-length: 92",
        "content-type: text/html;charset=utf-8",
    };
    TestConnection("f_07", 4242, expected_header,
                   FileToString(CN_REQ_PATH_PREFIX "error.html"));
  }

  // f 08 405 method not allowed
  {
    std::vector<std::string> expected_header = {
        "HTTP/1.1 405 Method Not Allowed",
        "server: BrilliantServer/1.0",
        "date: ",
        "allow: GET, POST",
        "connection: keep-alive",
        "content-length: 92",
        "content-type: text/html;charset=utf-8",
    };
    TestConnection("f_08", 4242, expected_header,
                   FileToString(CN_REQ_PATH_PREFIX "error.html"));
  }
  // f 09 method not allowed
  {
    chmod("./rf_resources/delete/dummy", 0444);
    std::vector<std::string> expected_header = {
        "HTTP/1.1 403 Forbidden",
        "server: BrilliantServer/1.0",
        "date: ",
        "allow: GET, POST, DELETE",
        "connection: keep-alive",
        "content-length: 92",
        "content-type: text/html;charset=utf-8",
    };
    TestConnection("f_09", 4242, expected_header,
                   FileToString(CN_REQ_PATH_PREFIX "error.html"));
    chmod("./rf_resources/delete/dummy", 0777);
  }

  // f 10 DELETE CGI request 405
  {
    std::vector<std::string> expected_header = {
        "HTTP/1.1 405 Method Not Allowed",
        "server: BrilliantServer/1.0",
        "date: ",
        "allow: GET, POST",
        "connection: keep-alive",
        "content-length: 92",
        "content-type: text/html;charset=utf-8",
    };
    TestConnection("f_10", 4242, expected_header,
                   FileToString(CN_REQ_PATH_PREFIX "error.html"));
  }

  // s 13 CGI GET Document
  {
    std::vector<std::string> expected_header = {
        "HTTP/1.1 200 OK",
        "server: BrilliantServer/1.0",
        "date: ",
        "allow: GET, POST",
        "connection: keep-alive",
        "content-length: 734",
        "content-type: text/plain",
        "access-control-allow-origin: *",
    };
    TestConnection("s_13", 4242, expected_header,
                   "envp [0] : AUTH_TYPE=\n\
envp [1] : CONTENT_LENGTH=\n\
envp [2] : CONTENT_TYPE=\n\
envp [3] : GATEWAY_INTERFACE=CGI/1.1\n\
envp [4] : PATH_INFO=\n\
envp [5] : PATH_TRANSLATED=\n\
envp [6] : QUERY_STRING=?Content-type+text/plain+0+Connection+upgrade+0+X-cgi-yongjule+yongjule+0+Access-Control-Allow-Origin+*+0\n\
envp [7] : REMOTE_ADDR=127.0.0.1\n\
envp [8] : REMOTE_HOST=127.0.0.1\n\
envp [9] : REMOTE_IDENT=\n\
envp [10] : REMOTE_USER=\n\
envp [11] : REQUEST_METHOD=GET\n\
envp [12] : SCRIPT_NAME=/resources/cgi/cgi.php\n\
envp [13] : SERVER_NAME=ghan\n\
envp [14] : SERVER_PORT=4242\n\
envp [15] : SERVER_PROTOCOL=HTTP/1.1\n\
envp [16] : SERVER_SOFTWARE=BrilliantServer/1.0\n\
\n\
\n\
Content-type+text/plain+0+Connection+upgrade+0+X-cgi-yongjule+yongjule+0+Access-Control-Allow-Origin+*+0");
  }

  // s 14 CGI GET Client Redirection 302 - no content
  {
    std::vector<std::string> expected_header = {
        "HTTP/1.1 302 Found",
        "server: BrilliantServer/1.0",
        "date: ",
        "allow: GET, POST",
        "connection: keep-alive",
        "location: https://www.naver.com/",
    };
    TestConnection("s_14", 4242, expected_header, "");
  }

  // s 15 CGI GET Client Redirection 302 - no content
  {
    std::vector<std::string> expected_header = {
        "HTTP/1.1 302 Found",
        "server: BrilliantServer/1.0",
        "date: ",
        "allow: GET, POST",
        "connection: keep-alive",
        "content-length: 10",
        "content-type: text/plain",
        "location: https://www.our42vent.42cadet.kr",
        "x-brilliantserver-a: blah",
        "x-brilliantserver-b: blahblah",
    };
    std::string expected_response = "aaaaaaaaaa";
    TestConnection("s_15", 4242, expected_header, expected_response);
  }

  close(listen_fd);
}
