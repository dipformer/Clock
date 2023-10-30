/*
  Temperature sensor for Dipformer Clock board

  Show temperature sensor value.

*/

#include <Dipformer_Clock.h>


void setup() {
  Clock.begin (); 
}

void loop() {
  Clock.clearDigits ();
  int t = Clock.getTemperature ();
  Clock.writeInt (2, t);
  Clock.setDigitChar (3, DIGIT_CHAR_GRAD);
  Clock.show ();
  delay (500);
}
