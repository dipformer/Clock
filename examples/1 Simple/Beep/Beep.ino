
/*
  Beep for Dipformer Clock board

*/

#include <Dipformer_Clock.h>

const uint16_t melody[] PROGMEM = { 
  NOTE_C|OCT_4|DUR_8, NOTE_PAUSE|DUR_8, 
  MELODY_REPEAT
};

void setup() {
  Clock.begin (); 
  Clock.playMelody (melody);
}

void loop() {
  // the melody plays in the background
}
