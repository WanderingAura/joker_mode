include(TargetCreationFuncs)

# building statically on android is simpler and doesn't cost anything
# because we can't hot reload anyway
if (${PLATFORM} STREQUAL "Android")
    joker_add_lib(soc STATIC)
else()
    joker_add_lib(soc SHARED)
endif()
joker_target_link_raylib(soc)

target_sources(soc PRIVATE
    core_game_memory.c
    state_transition.c
    game.c
    title_screen.c
    test_screen.c
    demo_level.c
    gameover.c
)

target_link_libraries(soc PRIVATE based input efs gameplay game_api level render behaviours soc_http vos)

add_library(game_api INTERFACE)

target_include_directories(game_api INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(game_api INTERFACE
    gameplay_interface
    level_interface
    render_interface
)

if (${PLATFORM} STREQUAL "Android")
    target_compile_definitions(game_api INTERFACE
        GAME_PLATFORM_ANDROID
    )
endif()