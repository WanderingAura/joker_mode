include(TargetCreationFuncs)

joker_add_lib(soc SHARED)
joker_target_link_raylib(soc)

target_sources(soc PRIVATE
    core_game_memory.c
    game.c
)

target_link_libraries(soc PRIVATE based efs gameplay game_api)

add_library(game_api INTERFACE)

target_include_directories(game_api INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(game_api INTERFACE
    gameplay_interface
    level_interface
    render_interface
)