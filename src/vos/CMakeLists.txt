joker_add_lib(vos OBJECT vos_common.c)
joker_target_link_raylib(vos)

target_include_directories(vos PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

if (UNIX)
    target_sources(vos PRIVATE vos_linux.c)
elseif(WIN32)
    target_sources(vos PRIVATE vos_windows.c)
else()
    message(FATAL_ERROR "Unsupported compiler/os")
endif()

target_link_libraries(vos PRIVATE based)