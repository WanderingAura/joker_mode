include(TargetCreationFuncs)

joker_add_exe(society main.c)
joker_target_link_raylib(society)

target_link_libraries(society PRIVATE based efs)