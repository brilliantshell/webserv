cmake_minimum_required(VERSION 3.24)
project(webserv)

# GoogleTest requires at least C++14
set(CMAKE_CXX_STANDARD 14)
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

include_directories("includes")

add_executable(
	test_webserv
	tests/test.cpp
	srcs/Validator.cpp
)

target_link_libraries(
	test_webserv
	GTest::gtest
)

include(GoogleTest)
gtest_discover_tests(test_webserv)
