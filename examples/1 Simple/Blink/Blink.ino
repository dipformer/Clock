/*
  Blink for Dipformer Clock board

  Turns a pixel on for one second, then off for one second, repeatedly.

*/

#include <Dipformer_Clock.h>

void setup() {
  Clock.begin (); 
}

void loop() {
  Clock.setDigitSegments (0, 1);
  Clock.show ();
  delay (1000);
  Clock.setDigitSegments (0, 0);
  Clock.show ();
  delay (1000);
}
