joker_add_exe(http)

target_sources(http PRIVATE
    http.c
    main.c
)

target_link_libraries(http PRIVATE based vos)

joker_add_lib(soc_http)

target_sources(soc_http PRIVATE
    http.c
)

target_include_directories(soc_http PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(soc_http PRIVATE based vos)