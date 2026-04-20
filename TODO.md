# TODO list
- [x] Add basic states for the following:
    - [x] Add basic game over screen
    - [x] Add basic title screen
- [x] Transition to game over screen when player health reaches 0
- [x] Change initialisation of resources (mainly the entity pool) to be idempotent. Saves us from destroying the resources (we can just reset them during initialisation)
- [x] Refactor the folder structure slightly
    - [x] Move game.c into core and make a standalone build option for soc.dll
    - [x] Move entity templates into a separate folder
    - [x] Textures should be in a separate render folder
    - [x] tilemap-like and level init code in the world folder
- [ ] Improve entity gameplay code
    - [x] Add a scaling attribute to all entities, that make them faster/slower
    - [x] Support entities to be spawned at an offset location to parent.
    - [x] Projectiles should die when they are too far away from the player/camera
    - [ ] Add a moving projectile spawner
- [ ] Make a 4x bigger demo level with walls and fixed location spawners, spawning different projectiles
    - [ ] Add walls
    - [ ] Add basic rectangular collision
    - [ ] Make a demo level 
- [ ] Make some better looking textures
- [ ] Possible improvements later on
    - [ ] Chunk the world so that only the closest 9 chunks are loaded. Chunks should be large enough that all walls have lenght less than half a chunk. (this will be needed if we get levels large enough for collision checks to be laggy).

- [x] make a cross platform socket library with recv buffering
    - [x] make a data type for the recv buffer: maybe an array which gets flushed whenever the read pointer gets to the end
    - [x] just make a blocking API for now, we can make it non-blocking later, if we need
- [x] make an http library

- [x] update the game over screen to load scores from the server
- [x] update the game over screen to be able to input your username
- [x] once username is inputted, it should be added to the highscores.
  - [ ] could look into pagination later
- [x] set up duckdns domain for hiscore server and make the server a systemd service on my pi

- [ ] add a window icon (does linux have this?)
- [x] fix windows rebuild being horribly slow (use unity build)
- [ ] add support for unity build on linux
- [ ] auto generate the unity build (don't use cmake because that's bloated af on windows)

- [ ] fix the game over leaderboard screen so that it displays less than 10 hiscores if there are < 10
- [x] change debugging logs to be dynamic at runtime based on env var

- [ ] refactor the entity pooling logic to have the pool allocator only responsible for the free list and the active list should be implemented by the user code. asked chatgpt their opinion on this and its take seems pretty reasonable: https://chatgpt.com/share/69b6f89a-86bc-8004-8526-74ba06309d0e.
