/*
  Light sensor for Dipformer Clock board

  Show minutes and seconds.

*/

#include <Dipformer_Clock.h>


void setup() {
  Clock.begin (); 
}

void loop() {
  DateTime dt = Clock.getDateTime ();
  Clock.clearDigits ();
  Clock.writeDigitInt (3, 2, dt.seconds);
  Clock.writeDigitInt (1, 2, dt.minutes);
  Clock.setDot (1);
  Clock.setDot (2);
  Clock.show ();
  delay (500);
  Clock.clearDigits ();
  Clock.writeDigitInt (3, 2, dt.seconds);
  Clock.writeDigitInt (1, 2, dt.minutes);
  Clock.show ();
  delay (500);
}
