include_guard(GLOBAL)

set(BUILD_SHARED_LIBS ON CACHE BOOL "Build raylib as shared library" FORCE)

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
