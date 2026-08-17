include(TargetCreationFuncs)

joker_add_lib(based STATIC
    based_arena.c
    based_logging.c
    based_basic.c
    based_raylib.c
)

joker_target_link_raylib(based)

if (!MSVC)
    target_compile_options(based PRIVATE -fPIC)
endif()

target_include_directories(based
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(based PRIVATE vos_interface)