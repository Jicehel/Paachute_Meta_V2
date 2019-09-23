#include "Global.h"

// -------------------------------------------------------------------------
// Home state
// -------------------------------------------------------------------------

class HomeState {

  public:

    void update()
    {
      if ((gb.buttons.released(BUTTON_A)) || (gb.buttons.released(BUTTON_B))) {
        HomeState::initGame();
      } else {
        HomeState::draw();
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
      for (auto &value : parachutes) value = -1;
    }

    // -------------------------------------------------------------------------
    // Draw Titlescreen
    // -------------------------------------------------------------------------

    void draw()    {
      // Go through each slice one by one
      for (uint8_t sliceIndex = 0; sliceIndex < slices; sliceIndex++) {

        // buffers are switched according to the parity of sliceIndex
        uint16_t* buffer = sliceIndex % 2 == 0 ? buffer1 : buffer2;

        // the top border of the current slice is calculated
        uint8_t sliceY = sliceIndex * sliceHeight;

        // starts by drawing the background
        memcpy(buffer, splachScreen + sliceY * screenWidth, 2 * screenWidth * sliceHeight);

        // then we make sure that the sending of the previous buffer
        // to the DMA controller has taken place
        if (sliceIndex != 0) waitForPreviousDraw();
        // after which we can then send the current buffer
        customDrawBuffer(0, sliceY, buffer, screenWidth, sliceHeight);
      }

      // always wait until the DMA transfer is completed
      // for the last slice before entering the next cycle
      waitForPreviousDraw();
    }
};

HomeState homeState;





