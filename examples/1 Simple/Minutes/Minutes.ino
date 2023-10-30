/*
  Light sensor for Dipformer Clock board

  Show hour and minutes.

*/

#include <Dipformer_Clock.h>


void setup() {
  Clock.begin (); 
}

void loop() {
  DateTime dt = Clock.getDateTime ();
  Clock.clearDigits ();
  Clock.writeDigitInt (3, 2, dt.minutes);
  Clock.writeDigitInt (1, 2, dt.hour);
  Clock.setDot (1);
  Clock.setDot (2);
  Clock.show ();
  delay (500);
  Clock.clearDigits ();
  Clock.writeDigitInt (3, 2, dt.minutes);
  Clock.writeDigitInt (1, 2, dt.hour);
  Clock.show ();
  delay (500);
}
