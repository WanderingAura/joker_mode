// Unity build to fix horribly slow hot reload on Windows due to
// CMake generating bloated visual studio project files.

#include "./based/based_arena.c"
#include "./based/based_basic.c"
#include "./based/based_logging.c"
#include "./efs/efs_entity.c"
#include "./game/core_game_memory.c"
#include "./game/game.c"
#include "./game/gameover.c"
#include "./gameplay/core_entity_template.c"
#include "./http/http.c"
#include "./http/main.c"
#include "./level/core_tilemap.c"
#include "./render/core_texture.c"
#include "./render/core_texture_types.c"
#include "./vos/vos_common.c"
#include "./vos/vos_socket.c"
#include "./vos/vos_windows.c"