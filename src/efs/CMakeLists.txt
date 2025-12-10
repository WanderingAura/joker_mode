joker_add_lib(efs)

target_sources(efs PRIVATE
    efs_entity.c
)

target_include_directories(efs PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(efs PRIVATE based)