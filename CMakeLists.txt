cmake_minimum_required(VERSION 3.31)
project(we_live_in_a_society)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED TRUE)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(FindRaylib)

# Web Configurations
if (${PLATFORM} STREQUAL "Web")
    set(CMAKE_SYSROOT "$ENV{EMSDK}/upstream/emscripten/cache/sysroot")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s USE_GLFW=3 -s ASSERTIONS=1 -s WASM=1 -s ASYNCIFY -s GL_ENABLE_GET_PROC_ADDRESS=1")
endif()

add_executable(society_test src/main.c)

target_link_libraries(society_test PRIVATE raylib m)
