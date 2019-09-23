#ifndef PARACHUTE_ANIMATE
#define PARACHUTE_ANIMATE

#include "Global.h"

void animateShark() {
  if (moveTick <= 0) {
    if ((sharkAnimation > -2) || ((sharkAnimation == -2) && (random(0, 50) < 3))) {
      ++sharkAnimation;
      if (sharkAnimation > -1) gb.sound.playTick();
    }
    if (sharkAnimation > 4) {
      sharkAnimation = -2;
      if (floodedAnimation > 4) floodedAnimation = -2;
    }
  }
}


void animateHelicopter() {
  if (helicopterAnimation >  2) helicopterAnimation = -1;
  helicopterAnimation++;
}


void animateFlooded() {
  if ((moveTick <= 0) && (floodedAnimation < 6)) floodedAnimation++;
}


void testBoat(uint8_t column, uint8_t paraCourant) {
  // Test if boat is unde the paratrooper => score or tansform the para as swimmer ...
  for (size_t count = paraCourant; count < parachuteLaunchCount ; count++) {
    parachutes[count] = parachutes[count + 1];
    parachuteLaunchCount --;
  }
  if (column == 1 + player.spriteIndex ) {
    ++score;
    gb.sound.playOK();
  } else {
    ++misses;
    gb.sound.playCancel();
    floodedAnimation = 2 - column;
    sharkAnimation = 2 - column;
  }
}


void animateParatrooper() {
  if (moveTick <= 0) {
    for (size_t count = 0 ; count < parachuteLaunchCount ; ++count)  {
      uint8_t p = parachutes[count];
      if (p > -1) {
        if (p == (firstSpriteColumn[1] - 1))
        {
          testBoat(1, count);   // Test if paratrooper is at last step of colonne 0
        }
        else if (p == (firstSpriteColumn[2] - 1))
        {
          testBoat(2, count);   // Test if paratrooper is at last step of colonne 1
        }
        else if (p == firstSpriteColumn[3])
        {
          testBoat(3, count);   // Test if paratrooper is at last step of colonne 2
        }
        else if (p < firstSpriteColumn[3])
        {
          ++parachutes[count];  // Next step for paratrooper fall
          gb.sound.playTick();
        }
      }
    }
  }
}


#endif


