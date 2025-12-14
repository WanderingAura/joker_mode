joker_add_lib(efs STATIC)

target_sources(efs PRIVATE
    efs_entity.c
)

if (MSVC)
    joker_target_link_raylib(efs)
endif()

target_include_directories(efs PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(efs PRIVATE based)