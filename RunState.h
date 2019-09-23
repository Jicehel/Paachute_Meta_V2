#ifndef RUNSTATE
#define RUNSTATE

#include "Global.h"
#include "Animate.h"

// -------------------------------------------------------------------------
// Run state
// -------------------------------------------------------------------------

class RunState {

  public:

    void update() {
      if (misses == 3) gameState = GameState::gameOver;
      else {
        // -------------------------------------------------------------------------
        // User interaction
        // -------------------------------------------------------------------------

        if ((gb.buttons.pressed(BUTTON_LEFT)) && (player.spriteIndex > 0))  {
          --player.spriteIndex;
          gb.sound.playTick();
        } else if ((gb.buttons.pressed(BUTTON_RIGHT)) && (player.spriteIndex < 2)) {
          ++player.spriteIndex;
          gb.sound.playTick();
        }
        if ((gb.buttons.released(BUTTON_A)) || (gb.buttons.released(BUTTON_B))) {
          gameState = GameState::pauseScreen;
          gb.sound.play("pauseScreen.wav");
        }

        // -------------------------------------------------------------------------
        // Anim objects
        // -------------------------------------------------------------------------

        animateShark();
        animateHelicopter();
        animateParatrooper();       // Animate paratrooper
        if (floodedAnimation > -2) animateFlooded(); // Animate Flooded if needed
        if (moveTick > 0) {
          --moveTick;
        } else {
          --spawnCount;
          if ((spawnCount < 1) && (random(6 - (score / 200)) < 4)) {   // Check if we launch a new paratrooper
            if (parachuteLaunchCount < 9) {
              size_t spriteColumn = random(0, 3);
              parachutes[parachuteLaunchCount] = firstSpriteColumn[spriteColumn];
              ++parachuteLaunchCount;
              spawnCount = spawnDelay - int(score / 60);
              if (spawnCount < 2) spawnCount = 2;
            }
          }
          moveTick = speedMax - (score / 75);
        }
        if ((score > 0)  && (score % 500 == 0)) {
          if (misses > 0) {
            --misses;
            gb.sound.play("Chance.wav");
          }
        }
        RunState::draw();
      }
    }


    void draw() {
      // Declares a pointer that will alternate between the two memory buffers
      uint16_t* buffer;
      // Declares the top border of current slice
      uint8_t sliceY;
      // Go through each slice one by one
      for (uint8_t sliceIndex = 0; sliceIndex < slices; sliceIndex++) {
        // Buffers are switched according to the parity of sliceIndex
        buffer = sliceIndex % 2 == 0 ? buffer1 : buffer2;
        // Top border of the current slice is calculated
        sliceY = sliceIndex * sliceHeight;

        // Starts by drawing the background
        memcpy(buffer, background + sliceY * screenWidth, 2 * screenWidth * sliceHeight);

        // then draws helicopter (a static sprite)
        drawSprite(helicopter, sliceY, buffer);

        // then draw blades
        if ((helicopterAnimation == 1) || (helicopterAnimation == 3)) {
          drawSprite(blades[0], sliceY, buffer);
          drawSprite(blades[1], sliceY, buffer);
        }
        if (helicopterAnimation == 2) {
          drawSprite(blades[2], sliceY, buffer);
          drawSprite(blades[3], sliceY, buffer);
        }

        if (misses > 0) {
          drawSprite(miss[0], sliceY, buffer);
          drawSprite(miss[1], sliceY, buffer);
          if (misses > 1) {
            drawSprite(miss[2], sliceY, buffer);
            if (misses > 2) {
              drawSprite(miss[3], sliceY, buffer);
            }
          }
        }

        // Draw parachutes
        for (uint8_t count = 0 ; count < parachuteLaunchCount ; count++) drawSprite(para[parachutes[count]], sliceY, buffer);

        // Draw flooded
        if ((floodedAnimation > -1) && (floodedAnimation < 6)) drawSprite(flooded[floodedAnimation], sliceY, buffer);

        // Draw shark if needed
        if ((sharkAnimation > -1) && (floodedAnimation < 6)) drawSprite(shark[sharkAnimation], sliceY, buffer);

        // Draws the boat of the player
        drawSprite(boat[player.spriteIndex], sliceY, buffer);

        // Draw score
        drawScore(score, sliceY, buffer);

        // Verify that previous buffer has been sent to the DMA controller
        if (sliceIndex != 0) waitForPreviousDraw();
        // Then send the current buffer
        customDrawBuffer(0, sliceY, buffer, screenWidth, sliceHeight);
      }

      // Wait until the DMA transfer is completed before entering the next cycle
      waitForPreviousDraw();
    }
};

RunState runState;

#endif
