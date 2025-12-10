cmake_minimum_required(VERSION 3.26)
project(we_live_in_a_society)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED TRUE)

# put binary output dir to be the project dir, wacky but convenient
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")

# needed to do the above for multi-config generators
foreach(CONFIG DEBUG RELEASE RELWITHDEBINFO MINSIZEREL)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${CONFIG} ${CMAKE_CURRENT_SOURCE_DIR})
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${CONFIG} ${CMAKE_CURRENT_SOURCE_DIR})
endforeach()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

option(ENABLE_UNITY_BUILD "Enable unity build compilation for targets that support it" ON)

include(FindJokerLibraries)

# Web Configurations
if (${PLATFORM} STREQUAL "Web")
    set(CMAKE_SYSROOT "$ENV{EMSDK}/upstream/emscripten/cache/sysroot")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s USE_GLFW=3 -s ASSERTIONS=1 -s WASM=1 -s ASYNCIFY -s GL_ENABLE_GET_PROC_ADDRESS=1")
endif()

add_subdirectory(src)
