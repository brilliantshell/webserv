cmake_minimum_required(VERSION 3.24)
project(
	webserv
	LANGUAGES CXX
)

# GoogleTest requires at least C++14
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "-pedantic -std=c++98")
set(CMAKE_CXX_FLAGS_DEBUG "-g3 -fsanitize=address")

include(FetchContent)
FetchContent_Declare(
  googletest
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
	tests/test_socket_generator.cpp
	tests/test_parser.cpp
	tests/test_router.cpp
	tests/test_path_resolver.cpp
	srcs/Validator.cpp
	srcs/PassiveSockets.cpp
	srcs/Connection.cpp
	srcs/HttpParser.cpp
	srcs/UriParser.cpp
	srcs/HeaderParser.cpp
	srcs/Router.cpp
	srcs/PathResolver.cpp
)

add_executable(
	client
	tests/test_client_io.cpp
	srcs/Validator.cpp
	srcs/Router.cpp
	srcs/ClientConnection.cpp
	srcs/PathResolver.cpp
)

add_executable(
	server
	tests/test_server_io.cpp
	srcs/Validator.cpp
	srcs/HttpServer.cpp
	srcs/PassiveSockets.cpp
	srcs/Connection.cpp
	srcs/HttpParser.cpp
	srcs/HeaderParser.cpp
	srcs/Router.cpp
	srcs/UriParser.cpp
	srcs/PathResolver.cpp
)

target_include_directories(
	test_webserv
	PUBLIC ${CMAKE_SOURCE_DIR}/includes
)

target_include_directories(
	client
	PUBLIC ${CMAKE_SOURCE_DIR}/includes
)

target_include_directories(
	server
	PUBLIC ${CMAKE_SOURCE_DIR}/includes
)

target_link_libraries(
	test_webserv
	GTest::gtest
)

include(GoogleTest)
gtest_discover_tests(test_webserv)
