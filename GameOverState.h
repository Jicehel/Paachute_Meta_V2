#include "Global.h"

// -------------------------------------------------------------------------
// Game Over state
// -------------------------------------------------------------------------

class GameOverState {

  public:

    void update()
    {
      if ((gb.buttons.released(BUTTON_A)) || (gb.buttons.released(BUTTON_B))) {
        gameState = GameState::home;
        gb.sound.play("gameOver.wav");
      } else {
        GameOverState::draw();
      }
    }


    // -------------------------------------------------------------------------
    // Draw Game Over screen
    // -------------------------------------------------------------------------

    void draw()    {
      // Go through each slice one by one
      for (uint8_t sliceIndex = 0; sliceIndex < slices; sliceIndex++) {

        // buffers are switched according to the parity of sliceIndex
        uint16_t* buffer = sliceIndex % 2 == 0 ? buffer1 : buffer2;

        // the top border of the current slice is calculated
        uint8_t sliceY = sliceIndex * sliceHeight;

        // starts by drawing the background
        memcpy(buffer, background + sliceY * screenWidth, 2 * screenWidth * sliceHeight);
        // and finally draws the pauseScreen sprite
        drawText(spriteGameOver, sliceY, buffer, 53, 60);

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

GameOverState gameOverState;
