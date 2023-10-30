#ifndef DIPFORMER_CLOCK_H
#define DIPFORMER_CLOCK_H

#include "Arduino.h"

// AVR Clock defines


#define GB_DIGIT_COUNT 4

#define GB_KEYCODE_NOT_PRESSED 0x00
#define GB_KEYCODE_S1 0x01
#define GB_KEYCODE_S2 0x02

#define GB_MAX_INTENSITY 15   // 0..15

#define DIGIT_CHAR_NO 10
#define DIGIT_CHAR_MINUS 11
#define DIGIT_CHAR_TOP 12
#define DIGIT_CHAR_BOTTOM 13
#define DIGIT_CHAR_GRAD 14
#define DIGIT_CHAR_A 16 
#define DIGIT_CHAR_B 17
#define DIGIT_CHAR_C 18
#define DIGIT_CHAR_D 19
#define DIGIT_CHAR_E 20
#define DIGIT_CHAR_F 21
#define DIGIT_CHAR_H 22
#define DIGIT_CHAR_I 23
#define DIGIT_CHAR_L 24
#define DIGIT_CHAR_N 25
#define DIGIT_CHAR_O 26
#define DIGIT_CHAR_P 27
#define DIGIT_CHAR_S 28
#define DIGIT_CHAR_T 29
#define DIGIT_CHAR_U 30


//SOUND defines:

#define MELODY_DEF_TEMPO 25 // ms for DUR_d64


#define NOTE_PAUSE 0x0000
#define NOTE_C 0x0001  // до
#define NOTE_Cm 0x0002
#define NOTE_Db 0x0002
#define NOTE_D 0x0003  // ре
#define NOTE_Dm 0x0004
#define NOTE_Eb 0x0004
#define NOTE_E 0x0005  // ми
#define NOTE_F 0x0006  // фа
#define NOTE_Fm 0x0007
#define NOTE_Gb 0x0007
#define NOTE_G 0x0008  // соль
#define NOTE_Gm 0x0009
#define NOTE_Ab 0x0009
#define NOTE_A 0x000a   // ля
#define NOTE_Am 0x000b
#define NOTE_Bb 0x000b
#define NOTE_B 0x000c  // си

#define OCT_8 0x0000
#define OCT_7 0x0010
#define OCT_6 0x0020
#define OCT_5 0x0030  // second octave
#define OCT_4 0x0040  // first octave
#define OCT_3 0x0050
#define OCT_2 0x0060
#define OCT_1 0x0070
#define OCT_0 0x0080

#define DUR_64 0x0000
#define DUR_32 0x0100
#define DUR_32P 0x0200
#define DUR_16 0x0300
#define DUR_16P 0x0400
#define DUR_8 0x0500
#define DUR_8P 0x0600
#define DUR_4 0x0700
#define DUR_4P 0x0800
#define DUR_2 0x0900
#define DUR_2P 0x0a00
#define DUR_1 0x0b00
#define DUR_1P 0x0c00
#define DUR_F2 0x0d00
#define DUR_F2P 0x0e00
#define DUR_F4 0x0f00


#define MELODY_END 0xffff
#define MELODY_REPEAT 0xfffe

#define DS1302_RAM_SIZE  31

#define TIME_24_HOUR_FORMAT 0
#define TIME_12_HOUR_FORMAT 1
#define TIME_AM 0
#define TIME_PM 1

struct DateTime {
  uint8_t seconds;
  uint8_t minutes; 
  uint8_t hour; 
  uint8_t date;
  uint8_t month;
  uint8_t day;
  uint16_t year;
  uint8_t AM_PM; // 0 - AM, 1 = PM
  uint8_t hour_12_24; // 0 - 24 hours, 1 = 12 hours
}; 



class CDipformer_Clock {
  public:
  CDipformer_Clock () {};
  void begin ();

  void playMelody (const uint16_t * buf);
  void playMelody (const uint16_t * buf, uint8_t tempo);
  void stopMelody ();
  uint8_t isMelody ();

  void setDigitIntensity (uint8_t n, uint8_t bit, uint8_t intensity);  // set intensity for n digit b bit 
  void setIntensity (uint8_t intensity);                                // set intensity for next steps
  void setDigitIntensity (uint8_t n, uint8_t intensity);                // set intensity for n digit
  
  void setDigitSegments (uint8_t n, uint8_t seg);                       // set segment values to position n
  void setDigitChar (uint8_t n, uint8_t ch);                            // set char to position n
  void setDot (uint8_t n);                                              // set dot to position n
  void writeInt (int8_t n, int v);                                      // write int to right position n
  void writeDigitInt (int8_t n, uint8_t count, int v);                  // write int to right position n and fill zero
  void clearDigits ();  
  void show ();               

  uint8_t getKeysState ();                                              // return current key positions
  uint8_t getPressedKey ();                                               // return key event XY, x-click counter, Y-key
  void resetKeyClicks ();                                               // reset clicks counter for  getKeyEvent
  
  float getTemperature ();                                   // Celsius
  float getLight ();                                         // LUX

  DateTime getDateTime ();
  void setDateTime (DateTime dateTime);

  void readRam (uint8_t * p);
  void writeRam (uint8_t * p);

  
  public:  
  uint8_t digits[GB_DIGIT_COUNT][8];
  
  private:
  uint8_t defIintensity;
  
  private:
  void DS1302_start ();
  void DS1302_stop ();
  uint8_t DS1302_toggleReadByte ();
  void DS1302_toggleWriteByte (uint8_t data, uint8_t release);
  uint8_t DS1302_read (uint8_t address);
  void DS1302_write (uint8_t address, uint8_t data);
      
};


extern CDipformer_Clock Clock;


#endif // DIPFORMER_CLOCK_H
