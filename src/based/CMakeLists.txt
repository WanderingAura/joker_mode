include(TargetCreationFuncs)

joker_add_lib(based STATIC based_arena.c)

target_include_directories(based PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})