include(TargetCreationFuncs)

if (${PLATFORM} STREQUAL "Android")
    joker_add_lib(society SHARED
        "main_android.c"
        $<TARGET_OBJECTS:vos>
    )
else()
    joker_add_exe(society
        "main.c"
        $<TARGET_OBJECTS:vos>
    )
    joker_exe_ext(society)
endif()

joker_target_link_raylib(society)

target_link_libraries(society PRIVATE based efs vos game_api)
if (${PLATFORM} STREQUAL "Android")
    target_link_libraries(society PRIVATE soc)
    # force the linker to keep the android entry point function.
    target_link_options(society PRIVATE "-Wl,-u,ANativeActivity_onCreate")
endif()

if (MSVC)
    # points the working directory to the correct place for the generated VS files
    set_target_properties(society PROPERTIES
        VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
    )
endif()