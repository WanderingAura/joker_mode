joker_add_exe(http)

target_sources(http PRIVATE
    http.c
    main.c
)

target_link_libraries(http PRIVATE based vos)