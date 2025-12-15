include(TargetCreationFuncs)

joker_add_lib(based STATIC
    based_arena.c
    based_logging.c
)

if (MSVC)
    joker_target_link_raylib(based)
endif()

target_include_directories(based PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})