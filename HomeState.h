#include "Global.h"

// -------------------------------------------------------------------------
// Home state
// -------------------------------------------------------------------------

class HomeState {

  public:

    void update() {
      if ((gb.buttons.released(BUTTON_A)) || (gb.buttons.released(BUTTON_B))) {
        HomeState::initGame();
      }
    }

    // -------------------------------------------------------------------------
    // Initialization of the game
    // -------------------------------------------------------------------------

    void initGame() {

      gb.lights.fill(BLACK);
      gameState = GameState::run;
      gb.sound.playTick();
      misses = 0;
      score = 0;
      player.spriteIndex = 0;
      spawnDelay = 4;       // Delay before next possibility to launch a para
      sharkAnimation = -2;
      floodedAnimation = -2;
      speedMax = 25;   // Number of cycles before update
      helicopterAnimation = 0;
      player.spriteIndex = 0;
      speedBlades = 5;
      moveTick = speedMax;
      spawnCount = spawnDelay;
      parachuteLaunchCount = 0;

      for (auto &value : parachutes)
        value = -1;
    }

    // -------------------------------------------------------------------------
    // Draw Titlescreen
    // -------------------------------------------------------------------------

    void draw() {
      drawBackground(splachScreen, spriteGameOver, 0, 0, false);
    }
};

HomeState homeState;





