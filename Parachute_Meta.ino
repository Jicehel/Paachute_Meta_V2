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

// includes the official library
#include <Gamebuino-Meta.h>

// includes assets
#include "background.h"
#include "spritesheet_A.h"
#include "spritesheet_B.h"
#include "splatchscreen.h"

// -------------------------------------------------------------------------
// Global constants
// -------------------------------------------------------------------------

const uint8_t  SCREEN_WIDTH  = 160;
const uint8_t  SCREEN_HEIGHT = 128;
const uint8_t  SLICE_HEIGHT  = 8;
const uint16_t TRANS_COLOR   = 0xdfff;

// All possible game state

const uint8_t  STATE_NOT_DEFINED = -1;   // Undefined state
const uint8_t  STATE_HOME = 0;           // Start screen
// const uint8_t  STATE_LAUNCH = 1;         // Launch game
const uint8_t  STATE_RUN = 2;            // Game run
const uint8_t  STATE_GAME_OVER = 3;      // Game over
const uint8_t  STATE_PAUSE = 4;          // Pause

// Define value for spitesheet to use

const uint8_t  SC_A = 0;
const uint8_t  SC_B = 1;

// Define global variables
uint8_t  gameState;
uint8_t  oldGameState;
uint16_t score;
uint16_t minHighscore;
int16_t  misses;
int16_t  moveTick;
int16_t  spawnDelay;
int8_t   spawnCount;
int8_t   speedmax;
uint8_t  manage_joystick;
uint8_t  nb_Parachutes_launched;
int8_t   parachutes[10];
int8_t   helico_anim;
int8_t   shark_anim;
int8_t   flooded_anim;
uint8_t  vitesse_pales;

// -------------------------------------------------------------------------
// Sprites
// -------------------------------------------------------------------------
// The surface of the spritesheet is deliberately not optimized here to
// facilitate positioning calculations at the time of rendering.
// -------------------------------------------------------------------------

// each sprite is characterized by its dimensions and coordinates
// on the spritesheet
struct Sprite {
  uint8_t spritesheet;
  uint8_t x, y;
  uint8_t w, h;
};

// a static sprite for the Helicopter
Sprite helico = { SC_B, 125, 5, 30, 25 };


// Spites of the blades of the Helicopter
Sprite blades[] = {
  { SC_A, 120,  6,  9, 1 },  //  0
  { SC_A, 132,  6,  9, 1 },  //  1
  { SC_A, 135, 10,  9, 1 },  //  2
  { SC_A, 147, 10,  9, 1 }   //  3
};

// the different postures and positions of the player's avatar
Sprite barque[] = {
  { SC_B, 16, 80,  27, 20 },  //  0
  { SC_A, 47, 80,  28, 24 },  //  1
  { SC_B, 79, 80,  31, 19 }   //  2
};

// the misses sprites
Sprite miss[] = {
  { SC_A, 120, 86,  22, 7  },  //  0
  { SC_A, 117, 95,  13, 11 },  //  1
  { SC_A, 131, 95,  13, 11 },  //  2
  { SC_A, 145, 95,  13, 11 }   //  3
};

// the sprites of the flooded
Sprite flooded[] = {
  { SC_A, 88 , 103,  20, 10 },  //  0
  { SC_B, 56 , 103,  21, 11 },  //  1
  { SC_A, 26 , 102,  20, 12 },  //  2
  { SC_A, 46 , 115,  13,  8 },  //  3
  { SC_A, 74 , 113,  13,  9 },  //  4
  { SC_A, 116, 109,  12, 14 }   //  5
};

// the sprites of the shark
Sprite shark[] = {
  { SC_A, 79 , 106,  7 ,  6 },  //  0
  { SC_A, 47 , 106,  7 ,  6 },  //  1
  { SC_A, 32 , 116,  6 ,  6 },  //  2
  { SC_A, 60 , 115,  7 ,  7 },  //  3
  { SC_B, 89 , 107,  25, 16 }   //  4
};

// the sprite pause
Sprite Sprite_pause = { SC_A, 1, 8, 28, 7 };

// the sprite Game Over
Sprite Sprite_gameOver = { SC_A, 1, 17, 53, 7 };

// the sprites of the para
Sprite para[] = {
  // col 1
  { SC_A, 106,  5,  9, 13 },  //  0
  { SC_B,  90,  8, 14, 14 },  //  1
  { SC_A,  76,  8, 15, 21 },  //  2
  { SC_B,  63, 14, 11, 25 },  //  3
  { SC_A,  48, 33, 15, 22 },  //  4
  { SC_B,  36, 52, 18, 22 },  //  5
  { SC_A,  28, 72, 19, 22 },  //  6
  // col 2
  { SC_A, 112, 19,  10, 12 },  //  7
  { SC_B, 100, 17,  15, 14 },  //  8
  { SC_A,  89, 18,  11, 27 },  //  9
  { SC_B,  74, 32,  15, 23 },  //  10
  { SC_A,  64, 48,  17, 24 },  //  11
  { SC_B,  59, 72,  19, 22 },  //  12
  // col 3
  { SC_A, 124, 30,  10, 11 },  //  13
  { SC_B, 114, 28,  11, 18 },  //  14
  { SC_A, 100, 32,  14, 21 },  //  15
  { SC_B,  92, 49,  18, 23 },  //  16
  { SC_A,  93, 72,  18, 22 }   //  17
};

// -------------------------------------------------------------------------
// Player definition
// -------------------------------------------------------------------------

struct Player {
  uint8_t  sprite_index;
};

Player player = {
  0      // sprite_index
};

// -------------------------------------------------------------------------
// Initialization related to the DMA controller
// -------------------------------------------------------------------------

namespace Gamebuino_Meta {
#define DMA_DESC_COUNT 3
extern volatile uint32_t dma_desc_free_count;
static inline void wait_for_transfers_done(void) {
  while (dma_desc_free_count < DMA_DESC_COUNT);
}
static SPISettings tftSPISettings = SPISettings(24000000, MSBFIRST, SPI_MODE0);
};

// rendering buffers
uint16_t buffer1[SCREEN_WIDTH * SLICE_HEIGHT];
uint16_t buffer2[SCREEN_WIDTH * SLICE_HEIGHT];

// flag for an ongoing data transfer
bool drawPending = false;

// -------------------------------------------------------------------------
// Initialization of the META
// -------------------------------------------------------------------------

void setup() {
  // initializes the META
  gb.begin();
  SerialUSB.begin(9600);
  // default screen buffer won't be used
  // so it must be set to 0x0 pixels
  gb.display.init(0, 0, ColorMode::rgb565);
  gameState = STATE_HOME;
}


// -------------------------------------------------------------------------
// Initialization of the game
// -------------------------------------------------------------------------

void initGame() {

  gb.lights.fill(BLACK);
  gameState = STATE_RUN;
  gb.sound.playTick();
  misses = 0;
  score = 0;
  player.sprite_index = 0;
  spawnDelay = 4;   // Delay before next possibility to launch a para
  shark_anim = -2;
  flooded_anim = -1;
  speedmax = 25;   // Nombre de cycles avant mise à jour de la boucle
  helico_anim = 0;
  player.sprite_index = 0;
  vitesse_pales = 5;
  moveTick = speedmax;
  spawnCount = spawnDelay;
  nb_Parachutes_launched = 0;
  manage_joystick = 0;
  for (int i = 0; i < 10; i++) {
    parachutes[i] = -1;
  }
}


// -------------------------------------------------------------------------
// Main control loop
// -------------------------------------------------------------------------

void loop() {

  gb.waitForUpdate();

  switch (gameState) {

    case STATE_HOME: // Start screen
      if (gb.buttons.released(BUTTON_A) || gb.buttons.released(BUTTON_B)) {
        initGame();
      } else {
        drawScreen();
      }
      break;

    case STATE_RUN: // Game running
      // a classic sequence :)
      if (misses == 3) gameState = STATE_GAME_OVER;
      else {
        getUserInput();
        update();
        draw();
      }
      break;

    case STATE_GAME_OVER:
      if (gb.buttons.released(BUTTON_A) || gb.buttons.released(BUTTON_B)) {
        gameState = STATE_HOME;
        gb.sound.playTick();
      } else {
        drawScreen();
      }
      break;

    case STATE_PAUSE: // Pause Game
      if (gb.buttons.released(BUTTON_A) || gb.buttons.released(BUTTON_B)) {
        gameState = STATE_RUN;
        gb.sound.playTick();
      } else {
        drawScreen();
      }
      break;

  }
}

// -------------------------------------------------------------------------
// User interaction
// -------------------------------------------------------------------------

void getUserInput() {
  if (gb.buttons.pressed(BUTTON_LEFT) && player.sprite_index > 0)  {
    player.sprite_index--;
    gb.sound.playTick();
  } else if (gb.buttons.pressed(BUTTON_RIGHT) && player.sprite_index < 2) {
    player.sprite_index++;
    gb.sound.playTick();
  }
  if (gb.buttons.released(BUTTON_A) || gb.buttons.released(BUTTON_B)) {
    gameState = STATE_PAUSE;
    gb.sound.playCancel();
  }
}

// -------------------------------------------------------------------------
// Game logic
// -------------------------------------------------------------------------

void update() {
  uint8_t temp;
  anim_shark();
  anim_helico();
  anim_para();   // Animation du parachutiste
  // anim_helico(); // Animtion de l'hélicoptère
  if (flooded_anim > -1) anim_flooded();
  if (moveTick > 0) {
    moveTick --;
  } else {
    spawnCount--;
    if ((spawnCount < 1) && (random(6 - (score / 200)) < 4)) {
      temp = int(random(0, 3));
      if (temp == 0) parachutes[nb_Parachutes_launched] = 0;
      else if (temp == 1) parachutes[nb_Parachutes_launched] = 7;
      else if (temp == 2) parachutes[nb_Parachutes_launched] = 13;
      nb_Parachutes_launched++;
      spawnCount = spawnDelay - int(score / 100);
    }
    moveTick = speedmax - (score / 100);
  }
}

/* void debug(char* message) {
  SerialUSB.print(message);
  SerialUSB.print(" - nb_Parachutes_launched : ");
  SerialUSB.print(nb_Parachutes_launched);
  SerialUSB.print(" - moveTick: ");
  SerialUSB.print(moveTick);
  SerialUSB.print(" - spawnCount: ");
  SerialUSB.println(spawnCount);

  uint8_t compteur;
  SerialUSB.print("Parachute : ");
  for (compteur = 0 ; compteur < nb_Parachutes_launched ; compteur++)  {
    SerialUSB.print("compteur: ");
    SerialUSB.print(compteur);
    SerialUSB.print(" - ");
    SerialUSB.print(parachutes[compteur]);
    SerialUSB.print("; ");
  }
  SerialUSB.println("");
  } */

void anim_shark() {
  if (moveTick <= 0) {
    if ((shark_anim >  2) || ((shark_anim > 4) && (flooded_anim > -1))) shark_anim = -2;
    if ((shark_anim > -2) || ((shark_anim == -2) && (random(0, 50) < 3))) {
      shark_anim++;
      if (shark_anim > -1) gb.sound.playTick();
    }
  }
}

void anim_helico() {

  if (helico_anim >  2) helico_anim = -1;
  helico_anim++;

}

void anim_flooded() {
  if (moveTick <= 0) {
    if (flooded_anim < 5) flooded_anim++;
    else flooded_anim = -1;
  }
}

void test_Barque(uint8_t colonne, uint8_t para_courant) {
  int count;
  // Test si la barque est sous le para => score ou passe le para en mode nageur (colonne théorique: 4) ...
  for (count = para_courant; count < nb_Parachutes_launched ; count++) {
    parachutes[count] = parachutes[count + 1];
    nb_Parachutes_launched --;
  }
  if (colonne == 1 + player.sprite_index ) {
    score ++;
    gb.sound.playOK();
  } else {
    misses++;
    gb.sound.playCancel();
    flooded_anim = 2 - colonne;
    shark_anim = 2 - colonne;
  }
}

void anim_para() {
  uint8_t compteur;
  if (moveTick <= 0) {
    for (compteur = 0 ; compteur < nb_Parachutes_launched ; compteur++)  {
      if (parachutes[compteur] > -1) {
        if ((parachutes[compteur] < 6) || ((parachutes[compteur] > 6) && (parachutes[compteur] < 12)) || ((parachutes[compteur] > 12) && (parachutes[compteur] < 17)))  {
          parachutes[compteur]++;  // Next step fo para fall
          gb.sound.playTick();
        } else if (parachutes[compteur] == 6) {
          test_Barque(1, compteur);
        } else if (parachutes[compteur] == 12) {
          test_Barque(2, compteur);
        } else if (parachutes[compteur] == 17) {
          test_Barque(3, compteur);
        }
      }
    }
  }
}


// -------------------------------------------------------------------------
// Graphic rendering
// -------------------------------------------------------------------------

void drawScreen() {
  // the number of horizontal slices to be cut is calculated
  uint8_t slices = SCREEN_HEIGHT / SLICE_HEIGHT;
  // declares a pointer that will alternate between the two memory buffers
  uint16_t* buffer;
  // declares the top border of current slice
  uint8_t sliceY;
  // then we go through each slice one by one
  for (uint8_t sliceIndex = 0; sliceIndex < slices; sliceIndex++) {
    // buffers are switched according to the parity of sliceIndex
    buffer = sliceIndex % 2 == 0 ? buffer1 : buffer2;
    // the top border of the current slice is calculated
    sliceY = sliceIndex * SLICE_HEIGHT;

    // starts by drawing the background
    switch (gameState) {

      case STATE_HOME: // Start screen
        if (gb.buttons.released(BUTTON_A) || gb.buttons.released(BUTTON_B)) {
          initGame();
          return;
        } else {
          memcpy(buffer, SPLATCHSCREEN + sliceY * SCREEN_WIDTH, 2 * SCREEN_WIDTH * SLICE_HEIGHT);
        }
        break;

      case STATE_GAME_OVER:
        if (gb.buttons.released(BUTTON_A) || gb.buttons.released(BUTTON_B)) {
          gameState = STATE_HOME;
          return;
        } else {
          memcpy(buffer, BACKGROUND + sliceY * SCREEN_WIDTH, 2 * SCREEN_WIDTH * SLICE_HEIGHT);
          // and finally draws the pause sprite
          drawSpriteXY(Sprite_gameOver, sliceY, buffer, 53, 60);
        }
        break;

      case STATE_PAUSE:
        if (gb.buttons.released(BUTTON_A) || gb.buttons.released(BUTTON_B)) {
          gameState = STATE_RUN;
          return;
        } else {
          memcpy(buffer, BACKGROUND + sliceY * SCREEN_WIDTH, 2 * SCREEN_WIDTH * SLICE_HEIGHT);
          // and finally draws the pause sprite
          drawSpriteXY(Sprite_pause, sliceY, buffer, 66, 60);
        }
        break;
    }

    // then we make sure that the sending of the previous buffer
    // to the DMA controller has taken place
    if (sliceIndex != 0) waitForPreviousDraw();
    // after which we can then send the current buffer
    customDrawBuffer(0, sliceY, buffer, SCREEN_WIDTH, SLICE_HEIGHT);
  }

  // always wait until the DMA transfer is completed
  // for the last slice before entering the next cycle
  waitForPreviousDraw();
}

void draw() {
  // the number of horizontal slices to be cut is calculated
  uint8_t slices = SCREEN_HEIGHT / SLICE_HEIGHT;
  // declares a pointer that will alternate between the two memory buffers
  uint16_t* buffer;
  // declares the top border of current slice
  uint8_t sliceY;
  // then we go through each slice one by one
  for (uint8_t sliceIndex = 0; sliceIndex < slices; sliceIndex++) {
    // buffers are switched according to the parity of sliceIndex
    buffer = sliceIndex % 2 == 0 ? buffer1 : buffer2;
    // the top border of the current slice is calculated
    sliceY = sliceIndex * SLICE_HEIGHT;

    // starts by drawing the background
    memcpy(buffer, BACKGROUND + sliceY * SCREEN_WIDTH, 2 * SCREEN_WIDTH * SLICE_HEIGHT);

    // below are commented lines to display other screens (spritesheets, splatchscreen, ...)
    //       memcpy(buffer, SPRITESHEET_A + sliceY * SCREEN_WIDTH, 2 * SCREEN_WIDTH * SLICE_HEIGHT);
    //       memcpy(buffer, SPRITESHEET_B + sliceY * SCREEN_WIDTH, 2 * SCREEN_WIDTH * SLICE_HEIGHT);
    //       memcpy(buffer, SPLATCHSCREEN + sliceY * SCREEN_WIDTH, 2 * SCREEN_WIDTH * SLICE_HEIGHT);

    // then draws helico (a static sprite)
    drawSprite(helico, sliceY, buffer);

    // then draw blades
    if ((helico_anim == 1) || (helico_anim == 3)) {
      drawSprite(blades[0], sliceY, buffer);
      drawSprite(blades[1], sliceY, buffer);
    }
    if (helico_anim == 2) {
      drawSprite(blades[2], sliceY, buffer);
      drawSprite(blades[3], sliceY, buffer);
    }
    // then draw shark if needed
    if (shark_anim > -1) drawSprite(shark[shark_anim], sliceY, buffer);

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

    // then draw para
    uint8_t compteur;
    for (compteur = 0 ; compteur < nb_Parachutes_launched ; compteur++) drawSprite(para[parachutes[compteur]], sliceY, buffer);

    // then draw flooded
    if (flooded_anim > -1) drawSprite(flooded[flooded_anim], sliceY, buffer);

    // and finally draws the player's avatar
    drawSprite(barque[player.sprite_index], sliceY, buffer);

    // then draw score
    drawScore(score, sliceY, buffer);

    // then we make sure that the sending of the previous buffer
    // to the DMA controller has taken place
    if (sliceIndex != 0) waitForPreviousDraw();
    // after which we can then send the current buffer
    customDrawBuffer(0, sliceY, buffer, SCREEN_WIDTH, SLICE_HEIGHT);
  }

  // always wait until the DMA transfer is completed
  // for the last slice before entering the next cycle
  waitForPreviousDraw();
}

void drawSprite(Sprite sprite, uint8_t sliceY, uint16_t* buffer) {
  // we check first of all that the intersection between
  // the sprite and the current slice is not empty
  if (sliceY < sprite.y + sprite.h && sprite.y < sliceY + SLICE_HEIGHT) {
    // determines the boundaries of the sprite surface within the current slice
    uint8_t  xmin = sprite.x;
    uint8_t  xmax = sprite.x + sprite.w - 1;
    uint8_t  ymin = sprite.y < sliceY ? sliceY : sprite.y;
    uint8_t  ymax = sprite.y + sprite.h >= sliceY + SLICE_HEIGHT ? sliceY + SLICE_HEIGHT - 1 : sprite.y + sprite.h - 1;

    uint8_t  px, py;
    uint16_t color;
    // goes through the sprite pixels to be drawn
    for (py = ymin; py <= ymax; py++) {
      for (px = xmin; px <= xmax; px++) {
        // picks the pixel color from the spritesheet
        if (sprite.spritesheet == SC_A) {
          color = SPRITESHEET_A[px + py * SCREEN_WIDTH];
        } else {
          color = SPRITESHEET_B[px + py * SCREEN_WIDTH];
        }
        // and if it is different from the transparency color
        if (color != TRANS_COLOR) {
          // copies the color code into the rendering buffer
          buffer[px + (py - sliceY) * SCREEN_WIDTH] = color;
        }
      }
    }
  }
}

void drawSpriteXY(Sprite sprite, uint8_t sliceY, uint16_t* buffer, uint8_t x, uint8_t y) {
  // we check first of all that the intersection between
  // the sprite and the current slice is not empty
  if (((sliceY < (y + sprite.h)) && (y  <= sliceY + SLICE_HEIGHT))) {
    // determines the boundaries of the sprite surface within the current slice
    uint8_t  ymin = y < sliceY ? sliceY : y;
    uint8_t  ymax = y + sprite.h >= sliceY + SLICE_HEIGHT ? sliceY + SLICE_HEIGHT - 1 : y + sprite.h - 1;
    uint8_t  px, py, temp;
    uint16_t color;
    // goes through the sprite pixels to be drawn
    temp = 0;
    for (py = ymin; py <= ymax; py++) {
      for (px = 0; px < sprite.w; px++) {
        // picks the pixel color from the spritesheet
        if (sprite.spritesheet == SC_A) {
          color = SPRITESHEET_A[sprite.x + px + (sprite.y + temp) * SCREEN_WIDTH];
        } else {
          color = SPRITESHEET_B[sprite.x + px + (sprite.y + temp) * SCREEN_WIDTH];
        }
        // and if it is different from the transparency color
        if (color != TRANS_COLOR) {
          // copies the color code into the rendering buffer
          buffer[x + px + (py - sliceY) * SCREEN_WIDTH] = color;
        }
      }
      temp++;
    }
  }
}


void drawScore(uint16_t Ascore, uint8_t sliceY, uint16_t* buffer) {
  // we check first of all that the intersection between
  // the sprite and the current slice is not empty
  if (sliceY < 20 && sliceY + SLICE_HEIGHT > 11 ) {
    // determines the boundaries of the sprite surface within the current slice
    uint8_t  xmin = 12;
    uint8_t  ymin = 12 < sliceY ? sliceY : 12;
    uint8_t  ymax = 19 >= sliceY + SLICE_HEIGHT ? sliceY + SLICE_HEIGHT - 1 : 18;
    uint8_t  px, py;
    uint16_t temp, temp2;
    uint16_t color;
    // goes through the sprite pixels to be drawn
    for (py = ymin; py <= ymax; py++) {
      for (px = 1; px <= 5; px++) {
        // draw thousands
        temp = Ascore / 1000;
        temp2 = Ascore % 1000;
        // picks the pixel color from the spritesheet
        color = SPRITESHEET_A[px + 6 * temp + (py - 11) * SCREEN_WIDTH];
        // and if it is different from the transparency color
        if (color != TRANS_COLOR) {
          // copies the color code into the rendering buffer
          buffer[xmin + px + (py - sliceY) * SCREEN_WIDTH] = color;
        }
        // draw hundreds
        temp = temp2 / 100;
        temp2 = temp2 % 100;
        // picks the pixel color from the spritesheet
        color = SPRITESHEET_A[px + 6 * temp + (py - 11) * SCREEN_WIDTH];
        // and if it is different from the transparency color
        if (color != TRANS_COLOR) {
          // copies the color code into the rendering buffer
          buffer[xmin + 6 + px + (py - sliceY) * SCREEN_WIDTH] = color;
        }
        // draw decades
        temp = temp2 / 10;
        temp2 = temp2 % 10;
        // picks the pixel color from the spritesheet
        color = SPRITESHEET_A[px + 6 * temp + (py - 11) * SCREEN_WIDTH];
        // and if it is different from the transparency color
        if (color != TRANS_COLOR) {
          // copies the color code into the rendering buffer
          buffer[xmin + 12 + px + (py - sliceY) * SCREEN_WIDTH] = color;
        }
        // draw units
        // picks the pixel color from the spritesheet
        color = SPRITESHEET_A[px + 6 * temp2 + (py - 11) * SCREEN_WIDTH];
        // and if it is different from the transparency color
        if (color != TRANS_COLOR) {
          // copies the color code into the rendering buffer
          buffer[xmin + 18 + px + (py - sliceY) * SCREEN_WIDTH] = color;
        }
      }
    }
  }
}
// -------------------------------------------------------------------------
// Memory transfer to DMA controller
// -------------------------------------------------------------------------

// initiates memory forwarding to the DMA controller....
void customDrawBuffer(uint8_t x, uint8_t y, uint16_t* buffer, uint8_t w, uint8_t h) {
  drawPending = true;
  gb.tft.setAddrWindow(x, y, x + w - 1, y + h - 1);
  SPI.beginTransaction(Gamebuino_Meta::tftSPISettings);
  gb.tft.dataMode();
  gb.tft.sendBuffer(buffer, w * h);
}

// waits for the memory transfer to be completed
// and close the transaction with the DMA controller
void waitForPreviousDraw() {
  if (drawPending) {
    Gamebuino_Meta::wait_for_transfers_done();
    gb.tft.idleMode();
    SPI.endTransaction();
    drawPending = false;
  }
}
