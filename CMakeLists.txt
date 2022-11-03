cmake_minimum_required(VERSION 3.24)
project(
	webserv
	LANGUAGES CXX
)

# GoogleTest requires at least C++14
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "-pedantic -std=c++98 -O0")
set(CMAKE_CXX_FLAGS_DEBUG "-g3 -fsanitize=address")

include(FetchContent)
FetchContent_Declare(
	googletest
	googlemock
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG release-1.12.1
)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(
	test_webserv
	tests/test.cpp
	tests/test_validator.cpp
	#tests/test_socket_generator.cpp
	#tests/test_parser.cpp
	#tests/test_router.cpp
	#tests/test_path_resolver.cpp
	#tests/test_resource_manager.cpp
	#tests/test_cgi_manager.cpp
	#tests/test_response_formatter.cpp
	#tests/test_connection.cpp
	srcs/Validator.cpp
	# srcs/PassiveSockets.cpp
	# srcs/Connection.cpp
	# srcs/HttpParser.cpp
	srcs/UriParser.cpp
	# srcs/HeaderParser.cpp
	srcs/Router.cpp
	srcs/PathResolver.cpp
	# srcs/ResponseManager.cpp
	srcs/CgiEnv.cpp
	# srcs/CgiManager.cpp
	# srcs/ResponseFormatter.cpp
	# srcs/ClientConnection.cpp
)

# add_executable(
# 	client
# 	tests/test_client_io.cpp
# 	srcs/Validator.cpp
# 	srcs/Router.cpp
# 	srcs/ClientConnection.cpp
# 	srcs/PathResolver.cpp
# 	srcs/CgiEnv.cpp
# )

# add_executable(
# 	server
# 	tests/test_server_io.cpp
# 	srcs/Validator.cpp
# 	srcs/PassiveSockets.cpp
# 	srcs/HttpServer.cpp
# 	srcs/Connection.cpp
# 	srcs/HttpParser.cpp
# 	srcs/UriParser.cpp
# 	srcs/HeaderParser.cpp
# 	srcs/Router.cpp
# 	srcs/PathResolver.cpp
# 	srcs/ResponseManager.cpp
# 	srcs/CgiEnv.cpp
# 	srcs/CgiManager.cpp
# 	srcs/ResponseFormatter.cpp
# )

add_executable(
	cgi.php
	cgi_src/cgi.cpp
)

add_executable(
	cgi_max.php
	cgi_src/cgi_max.cpp
)

add_executable(
	cgi_cwd.php
	cgi_src/cgi_cwd.cpp
)

add_executable(
	cgi_redir.php
	cgi_src/cgi_redir.cpp
)

target_include_directories(
	test_webserv
	PUBLIC ${CMAKE_SOURCE_DIR}/includes
)

# target_include_directories(
# 	client
# 	PUBLIC ${CMAKE_SOURCE_DIR}/includes
# )

# target_include_directories(
# 	server
# 	PUBLIC ${CMAKE_SOURCE_DIR}/includes
# )

target_link_libraries(
	test_webserv
	GTest::gtest
	GTest::gmock
)

execute_process(
	# symlink
	COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_SOURCE_DIR}/tests/ResponseManager ${CMAKE_SOURCE_DIR}/build/resources
	COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_SOURCE_DIR}/tests/ResponseFormatter ${CMAKE_SOURCE_DIR}/build/rf_resources
	COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_SOURCE_DIR}/tests/Connection ${CMAKE_SOURCE_DIR}/build/cn_resources

	COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_SOURCE_DIR}/build/cgi.php ${CMAKE_SOURCE_DIR}/build/resources/cgi/cgi.php
	COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_SOURCE_DIR}/build/cgi_max.php ${CMAKE_SOURCE_DIR}/build/resources/cgi/cgi_max.php
	COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_SOURCE_DIR}/build/cgi_cwd.php ${CMAKE_SOURCE_DIR}/build/resources/cgi/cgi_cwd.php
	COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_SOURCE_DIR}/build/cgi_redir.php ${CMAKE_SOURCE_DIR}/build/resources/cgi/cgi_redir.php
)

include(GoogleTest)
gtest_discover_tests(test_webserv)
