include(TargetCreationFuncs)

joker_add_lib(game_core STATIC)

target_sources(game_core PRIVATE core_tilemap.c)

target_include_directories(game_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(game_core PRIVATE based)