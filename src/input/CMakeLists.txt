joker_add_lib(input STATIC)
joker_target_link_raylib(input)

target_sources(input PRIVATE
    input.c
)

target_link_libraries(input PRIVATE based game_api)

target_include_directories(input PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

option(MOCK_TOUCH_INPUTS "Mock touch inputs with the mouse" OFF)
if (${PLATFORM} STREQUAL "Android")
    target_compile_definitions(input PUBLIC TOUCH_INPUTS_ENABLED)
elseif(MOCK_TOUCH_INPUTS)
    target_compile_definitions(input PUBLIC TOUCH_INPUTS_MOCKED)
endif()