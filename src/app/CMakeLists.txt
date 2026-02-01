include(TargetCreationFuncs)

joker_add_exe(society
    main.c
    $<TARGET_OBJECTS:vos>
)
joker_target_link_raylib(society)

joker_exe_ext(society)

target_link_libraries(society PRIVATE based efs vos game_api)
