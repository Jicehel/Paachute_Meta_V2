// -------------------------------------------------------------------------
// Game & Watch Parachute © 2019 Jicehel (https://gamebuino.com/@jicehel)
// Based on Nintendo Game & Watch Parachute
// -------------------------------------------------------------------------
// Many thanks to Andy (https://gamebuino.com/@aoneill) for the routines
// related to the DMA controller to use High Definition 160x128 on the META
// and to Steph (https://gamebuino.com/@steph) for his help and for the
// structure of games like Nintendo's Game & Watch console series, in High
// Definition on the META.
// -------------------------------------------------------------------------
// Special thanks to Pharap for his help to improve the source
// -------------------------------------------------------------------------

// includes the official library
#include <Gamebuino-Meta.h>

// includes other parts of pogram

#include "Global.h"
#include "HD_Tools.h"
#include "HomeState.h"
#include "RunState.h"
#include "PauseState.h"
#include "GameOverState.h"
#include "Pictures.h"
#include "Sprites.h"


// -------------------------------------------------------------------------
// Initialization of the META
// -------------------------------------------------------------------------

void setup() {
  // initializes the META
  gb.begin();
  // SerialUSB.begin(9600);
  // default screen buffer won't be used
  // so it must be set to 0x0 pixels
  gb.display.init(0, 0, ColorMode::rgb565);
  gameState = GameState::home;
}


// -------------------------------------------------------------------------
// Main control loop
// -------------------------------------------------------------------------

void loop() {

  gb.waitForUpdate();

  switch (gameState) {

    case GameState::home: // Start screen
      homeState.update();
      homeState.draw();
      break;

    case GameState::run: // Game running
      runState.update();
      runState.draw();
      break;

    case GameState::gameOver:  // GameOver screen
      gameOverState.update();
      gameOverState.draw();
      break;

    case GameState::pauseScreen: // pauseScreen Game
      pauseState.update();
      pauseState.draw();
      break;

  }
}

