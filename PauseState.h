#include "Global.h"

// -------------------------------------------------------------------------
// Pause state
// -------------------------------------------------------------------------

class PauseState {

  public:

    void update() {
      if ((gb.buttons.released(BUTTON_A)) || (gb.buttons.released(BUTTON_B))) {
        gameState = GameState::run;
        gb.sound.play("pauseScreen.wav");
      }
    }


    // -------------------------------------------------------------------------
    // Draw Game Over screen
    // -------------------------------------------------------------------------

    void draw() {
      drawBackground(background, spritepauseScreen, 66, 60, true);
    }
};

PauseState pauseState;
