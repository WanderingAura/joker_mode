# TODO list
- [x] Add basic state machine for the following:
    - [x] Add basic game over screen
    - [x] Add basic title screen
- [ ] Transition to game over screen when player health reaches 0
- [ ] Add a moving projectile spawner
- [ ] Change initialisation of resources (mainly the entity pool) to be idempotent. Saves us from destroying the resources (we can just reset them during initialisation)