include_guard(DIRECTORY)

set_property(DIRECTORY PROPERTY JOKER_COMPILER_FLAGS "-Wall;-Wextra;-Werror")
set_property(DIRECTORY PROPERTY JOKER_MSVC_COMPILER_FLAGS "/W4")

function( _joker_set_options name )
    if (MSVC)
        get_directory_property(compiler_flags JOKER_MSVC_COMPILER_FLAGS)
    else()
        get_directory_property(compiler_flags JOKER_COMPILER_FLAGS)
    endif()

    target_compile_options(${name} PRIVATE ${compiler_flags})

    target_compile_definitions(${name} PRIVATE
        $<$<CONFIG:Debug>:WLIAS_DEBUG>
    )

    if (ENABLE_UNITY_BUILD)
        set_target_properties(${name} PROPERTIES CMAKE_UNITY_BUILD ON)
    endif()
endfunction()

function( joker_add_lib name )
    add_library(${name} ${ARGN})
    _joker_set_options(${name})
endfunction()

function( joker_add_exe name )
    add_executable(${name} ${ARGN})
    _joker_set_options(${name})

    target_link_libraries(${name} PRIVATE raylib)
    if (UNIX)
        target_link_libraries(${name} PRIVATE m)
    endif()

    if (${PLATFORM} STREQUAL "Web")
        set_target_properties(${name} PROPERTIES SUFFIX ".html")
    endif()
endfunction()
