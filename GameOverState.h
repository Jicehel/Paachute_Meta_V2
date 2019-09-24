#include "Global.h"

// -------------------------------------------------------------------------
// Game Over state
// -------------------------------------------------------------------------

class GameOverState {

  public:

    void update() {
      if ((gb.buttons.released(BUTTON_A)) || (gb.buttons.released(BUTTON_B))) {
        gameState = GameState::home;
        gb.sound.play("gameOver.wav");
      } 
    }


    // -------------------------------------------------------------------------
    // Draw Game Over screen
    // -------------------------------------------------------------------------

    void draw()    {

      drawBackground(background, spriteGameOver, 53, 60, true);  
    
    }
};

GameOverState gameOverState;
