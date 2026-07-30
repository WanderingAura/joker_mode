joker_add_lib(behaviours STATIC)

target_sources(behaviours PRIVATE
    candamage.c
    canmove.c
    collidable.c
    despawnwhenfarfromplayer.c
    followsother.c
    hashealth.c
    haslifetime.c
    hasrotation.c
    movesbetweentwopoints.c
    playercontrolled.c
    prop_behaviours.h
    scaleswithdifficulty.c
    shootsatmouse.c
    solid.c
    spawner.c
    tempinvincible.c
)

joker_target_link_raylib(behaviours)

target_include_directories(behaviours PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(behaviours PRIVATE based efs gameplay_interface level game_api)