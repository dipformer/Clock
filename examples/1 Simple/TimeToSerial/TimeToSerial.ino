/*

  Example.
  Print time to Serial.

*/

#include <Dipformer_Clock.h>


void setup() {
  Serial.begin (9600);
  Clock.begin ();
  Clock.clearDigits ();
}

void loop() {
  DateTime dateTime = Clock.getDateTime ();
  
  Serial.print (dateTime.hour);
  Serial.print (":");
  Serial.print (dateTime.minutes);
  Serial.print (":");
  Serial.print (dateTime.seconds);
  Serial.print (" ");
  Serial.print (dateTime.date);
  Serial.print (".");
  Serial.print (dateTime.month);
  Serial.print (".");
  Serial.print (dateTime.year);
  Serial.print (" ");
  Serial.print (dateTime.day);
  Serial.println (); 
  
  delay (1000);
}