/*
  Blink for Dipformer Clock board
  Display test
*/

#include <Dipformer_Clock.h>

char digits[12][2] = {
  {0, 0}, {1,0}, {2,0}, {3,0}, {3,1}, {3,2}, {3,3}, {2,3}, {1,3}, {0,3}, {0,4}, {0,5}  
};
char intensity[] = {1, 3, 7, 12, 15};

int pos = 0;

void setup() {
  Clock.begin (); 
}


void loop() {
  Clock.clearDigits ();
  int k = pos;
  for (int i = 0; i<5; i++) {
    Clock.setDigitIntensity (digits[k][0], digits[k][1], intensity[i]);
    k++;
    if (k >= 12) k = 0;
  }
  Clock.show ();
  pos++;
  if (pos >= 12) pos = 0;
  delay (70);
}
