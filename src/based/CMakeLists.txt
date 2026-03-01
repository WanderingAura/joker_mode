include(TargetCreationFuncs)

joker_add_lib(based STATIC
    based_arena.c
    based_logging.c
    based_basic.c
)

joker_target_link_raylib(based)
target_compile_options(based PRIVATE -fPIC)

target_include_directories(based PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})