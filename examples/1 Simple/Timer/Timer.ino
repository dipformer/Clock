/*
  Light sensor for Dipformer Clock board

  Show minutes and seconds.

*/

#include <Dipformer_Clock.h>

int seconds = 0;
int minutes = 0;
long prevMillis = 0;
long key = 0;

void setup() {
  Clock.begin (); 
}

void loop() {
  Clock.clearDigits ();
  Clock.writeDigitInt (3, 2, seconds);
  Clock.writeDigitInt (1, 2, minutes);
  Clock.setDot (1);
  Clock.setDot (2);
  Clock.show ();
  if (checkKey ()) return;
  Clock.clearDigits ();
  Clock.writeDigitInt (3, 2, seconds);
  Clock.writeDigitInt (1, 2, minutes);
  Clock.show ();
  if (checkKey ()) return;
  seconds++;
  if (seconds>=60) {
    seconds=0;
    minutes++;
    if (minutes>=60) {
      minutes=0;
    }  
  }
}

int checkKey () {
  while (true) {
    long m = millis ();
    if (m - prevMillis > 500) {
      prevMillis = prevMillis + 500;
      return 0;
    }
    int prevKey = key;
    key = Clock.getKeysState ();
    if (key && prevKey == 0) {  
      seconds = 0; 
      minutes = 0; 
      prevMillis = m;
      return 1;
    }
    delay (10);
  }
  return 0;
}
