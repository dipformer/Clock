/*
  Light sensor for Dipformer_Clock board

  Show light sensor value.

*/

#include <Dipformer_Clock.h>


void setup() {
  Clock.begin (); 
}

void loop() {
  int t = Clock.getLight ();
  Clock.clearDigits ();
  Clock.writeInt (3, t);
  Clock.show ();
  delay (300);
}
