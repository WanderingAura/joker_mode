include(TargetCreationFuncs)

joker_add_exe(society
    main.c
    $<TARGET_OBJECTS:vos>
)
joker_target_link_raylib(society)

joker_exe_ext(society)

target_link_libraries(society PRIVATE based efs vos game_api)

if (MSVC)
    # points the working directory to the correct place for the generated VS files
    set_target_properties(society PROPERTIES
        VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
    )
endif()