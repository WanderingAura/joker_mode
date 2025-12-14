joker_add_lib(vos OBJECT)

target_include_directories(vos PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

if (UNIX)
    target_sources(vos PRIVATE vos_linux.c)
elseif(MSVC)
    target_sources(vos PRIVATE vos_windows.c)
else()
    message(FATAL_ERROR "Unsupported compiler/os")
endif()

target_link_libraries(vos PRIVATE based)