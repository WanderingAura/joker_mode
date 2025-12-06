include_guard(DIRECTORY)

set_property(DIRECTORY PROPERTY JOKER_COMPILER_FLAGS "-Wall;-Wextra;-Werror")
set_property(DIRECTORY PROPERTY JOKER_MSVC_COMPILER_FLAGS "/W4;/WX;/permissive-")

function( _joker_set_options target )
    if (MSVC)
        get_directory_property(compiler_flags JOKER_MSVC_COMPILER_FLAGS)
    else()
        get_directory_property(compiler_flags JOKER_COMPILER_FLAGS)
    endif()

    target_compile_options(${target} PRIVATE ${compiler_flags})

    target_compile_definitions(${target} PRIVATE
        $<$<CONFIG:Debug>:WLIAS_DEBUG>
    )

    if (ENABLE_UNITY_BUILD)
        set_target_properties(${target} PROPERTIES CMAKE_UNITY_BUILD ON)
    endif()
endfunction()

function( joker_add_lib target )
    add_library(${target} ${ARGN})
    _joker_set_options(${target})
endfunction()

function( joker_add_exe target )
    add_executable(${target} ${ARGN})
    _joker_set_options(${target})

endfunction()

function( joker_target_link_raylib target )
    target_link_libraries(${target} PRIVATE raylib)
    if (UNIX)
        target_link_libraries(${target} PRIVATE m)
    endif()

    if (${PLATFORM} STREQUAL "Web")
        set_target_properties(${target} PROPERTIES SUFFIX ".html")
    elseif(UNIX)
        # since we put our output binaries in the project root
        # (requested by mien fuhrer) we need an extension
        # in linux for .gitignore
        set_target_properties(${target} PROPERTIES SUFFIX ".bin")
    endif()
endfunction()
