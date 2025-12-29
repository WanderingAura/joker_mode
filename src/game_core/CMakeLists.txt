include(TargetCreationFuncs)

joker_add_lib(game_core STATIC)

joker_target_link_raylib(game_core)

target_sources(game_core PRIVATE
    core_tilemap.c
    core_projectile.c
    core_game_memory.c
    core_texture.c
    core_texture_types.c
)

target_include_directories(game_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(game_core PRIVATE based efs)