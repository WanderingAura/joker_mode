include_guard(GLOBAL)

# raylib has part of the bootstrapping code on Android (it needs to be static) so we can only build it as shared on other platforms.
if (NOT ${PLATFORM} STREQUAL "Android")
  set(BUILD_SHARED_LIBS ON CACHE BOOL "Build raylib as shared library" FORCE)
endif()
if (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
  if (DEFINED ENV{WAYLAND_DISPLAY})
    set(GLFW_BUILD_WAYLAND ON CACHE BOOL "build raylib with wayland" FORCE)
  endif()
endif()

set(RAYLIB_VERSION 5.5)
include(FetchContent)

FetchContent_Declare(
  raylib
  DOWNLOAD_EXTRACT_TIMESTAMP OFF
  URL https://github.com/raysan5/raylib/archive/refs/tags/${RAYLIB_VERSION}.tar.gz
)
FetchContent_GetProperties(raylib)
if (NOT raylib_POPULATED) # Have we downloaded raylib yet?
  set(FETCHCONTENT_QUIET NO)
  FetchContent_MakeAvailable(raylib)
endif()

# we need to unset this to allow other libraries to be built statically by default
set(BUILD_SHARED_LIBS OFF)