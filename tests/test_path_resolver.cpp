#include <gtest/gtest.h>

#include "PathResolver.hpp"

TEST(PathResolverTest, ResolvePathForValidator) {
  {
    std::string path("/a/../././/////");
    PathResolver resolver;
    resolver.Resolve(path);
    EXPECT_EQ(path, "/");
  }
  {
    std::string path("/..");
    PathResolver resolver;
    bool result = resolver.Resolve(path);
    EXPECT_FALSE(result);
  }
  {
    std::string path("/a/../././/b////");
    PathResolver resolver;
    resolver.Resolve(path);
    EXPECT_EQ(path, "/b/");
  }
  {
    std::string path("/a/../././/////bcd");
    PathResolver resolver;
    resolver.Resolve(path);
    EXPECT_EQ(path, "/bcd/");
  }
  {
    std::string path(
        "/a/../././//////a/../././//////a/../././//////a/../././//////a/../././"
        "//////a/../././//////a/../././//////a/../././//////a/../././//////a/"
        "../././//////a/../././/////");
    PathResolver resolver;
    resolver.Resolve(path);
    EXPECT_EQ(path, "/");
  }
}

TEST(PathResolverTest, ResolvePathForRouter) {
  {
    std::string path("/a/../index.html");
    PathResolver resolver;
    resolver.Resolve(path, PathResolver::kRouter);
    EXPECT_EQ(path, "/index.html");
  }
  {
    std::string path("/a/../index.html/");
    PathResolver resolver;
    resolver.Resolve(path, PathResolver::kRouter);
    EXPECT_EQ(path, "/index.html/");
  }
  {
    std::string path("/a/../index.html/./../.../");
    PathResolver resolver;
    resolver.Resolve(path, PathResolver::kRouter);
    EXPECT_EQ(path, "/.../");
  }
  {
    std::string path("/a/../index.html/./../..thisisdir/");
    PathResolver resolver;
    resolver.Resolve(path, PathResolver::kRouter);
    EXPECT_EQ(path, "/..thisisdir/");
  }

  {
    std::string path("/a/../index.html/./../.느낌이오잖아~/");
    PathResolver resolver;
    resolver.Resolve(path, PathResolver::kRouter);
    EXPECT_EQ(path, "/.느낌이오잖아~/");
  }

  {
    std::string path("/a/./../");
    PathResolver resolver;
    resolver.Resolve(path, PathResolver::kRouter);
    EXPECT_EQ(path, "/");
  }
}
