joker_add_lib(level STATIC)
joker_target_link_raylib(level)

target_sources(level PRIVATE
    core_tilemap.c
    lvl_collision.c
)

target_link_libraries(level PRIVATE based efs game_api)

add_library(level_interface INTERFACE)

target_include_directories(level_interface INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})