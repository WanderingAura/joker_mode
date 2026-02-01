joker_add_lib(gameplay STATIC)
joker_target_link_raylib(gameplay)

target_sources(gameplay PRIVATE
    core_entity_template.c
)

target_link_libraries(gameplay PRIVATE based efs game_api)


add_library(gameplay_interface INTERFACE)

target_include_directories(gameplay_interface INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})