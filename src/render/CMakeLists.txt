joker_add_lib(render STATIC)
joker_target_link_raylib(render)

target_sources(render PRIVATE
    core_texture_types.c
    core_texture.c
    render_font_types.c
    render_font.c
)

target_link_libraries(render PRIVATE based efs game_api)


add_library(render_interface INTERFACE)

target_include_directories(render_interface INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})