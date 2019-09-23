#include "Global.h"
#include "Sprites.h"
#include "Pictures.h"

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
uint16_t buffer1[screenWidth * sliceHeight];
uint16_t buffer2[screenWidth * sliceHeight];

// flag for an ongoing data transfer
bool drawPending = false;



void drawSprite(Sprite sprite, uint8_t sliceY, uint16_t* buffer) {
  // Check if sprite has one part to show on the current slice
  if (sliceY < sprite.y + sprite.h && sprite.y < sliceY + sliceHeight) {
    // Determines the boundaries of the sprite surface within the current slice
    uint8_t  xMin = sprite.x;
    uint8_t  xmax = sprite.x + sprite.w - 1;
    uint8_t  yMin = sprite.y < sliceY ? sliceY : sprite.y;
    uint8_t  yMax = sprite.y + sprite.h >= sliceY + sliceHeight ? sliceY + sliceHeight - 1 : sprite.y + sprite.h - 1;
    uint8_t  px, py;
    uint16_t color;
    // Display the sprite pixels to be drawn
    for (py = yMin; py <= yMax; py++) {
      for (px = xMin; px <= xmax; px++) {
        // Picks the pixel color from the spritesheet
        if (sprite.spritesheet == idSpritesheetA) {
          color = spritesheetA[px + py * screenWidth];
        } else {
          color = spritesheetB[px + py * screenWidth];
        }
        // It colo is different from the transparency color
        if (color != transColor) {
          // Copies the color code into the rendering buffer
          buffer[px + (py - sliceY) * screenWidth] = color;
        }
      }
    }
  }
}



void drawText(Sprite sprite, uint8_t sliceY, uint16_t* buffer, uint8_t x, uint8_t y) {
  if (sliceY < y + sprite.h && y < sliceY + sliceHeight) {
    uint8_t xMin = x;
    uint8_t xmax = x + sprite.w - 1;
    uint8_t yMin = y < sliceY ? sliceY : y;
    uint8_t yMax = y + sprite.h >= sliceY + sliceHeight ? sliceY + sliceHeight - 1 : y + sprite.h - 1;

    uint8_t  px, py, sx, sy;
    uint16_t color;

    for (py = yMin; py <= yMax; ++py) {
      sy = py - y + sprite.y;
      for (px = xMin; px <= xmax; ++px) {
        sx = px - xMin + sprite.x;
        color = spritesheetA[sx + sy * screenWidth];
        if (color != transColor) {
          buffer[px + (py - sliceY) * screenWidth] = color;
        }
      }
    }
  }
}



void drawScore(uint16_t displayScore, uint8_t sliceY, uint16_t* buffer) {
  // we check first of all that the intersection between
  // the sprite and the current slice is not empty
  if (sliceY < 20 && sliceY + sliceHeight > 11 ) {
    // determines the boundaries of the sprite surface within the current slice
    uint8_t  xMin = 12;
    uint8_t  yMin = (12 < sliceY) ? sliceY : 12;
    uint8_t  yMax = (19 >= sliceY + sliceHeight) ? sliceY + sliceHeight - 1 : 18;
    uint16_t color;
    uint16_t remainder = displayScore;

    // Draw each digit of the score
    for (uint16_t divisor = 1000; divisor > 0; divisor /= 10)     {  // The limit is 4 because 10000 is pow(10, 4)
      uint16_t quotient = (remainder / divisor);
      remainder = (remainder % divisor);
      xMin += 6;
      // Go through the sprite pixels to be drawn
      for (uint8_t py = yMin; py <= yMax; ++py)   {
        for (uint8_t px = 1; px <= 5; ++px)   {

          // Calculate the colour offset
          size_t colourIndex = (px + (6 * quotient) + ((py - 11) * screenWidth));

          // Pick the pixel colour from the spritesheet
          uint16_t colour = spritesheetA[colourIndex];

          // If it is not the transparency colour
          if (colour != transColor)     {
            // Calculate the colour offset
            size_t bufferIndex = (xMin + px + ((py - sliceY) * screenWidth));
            // Copy the colour code into the rendering buffer
            buffer[bufferIndex] = colour;
          }
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
