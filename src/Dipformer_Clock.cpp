
#include "Dipformer_Clock.h"

// AVR Clock pins

#define GB_PIN_BUZZER 13 
#define GB_BIT_BUZZER 5   
#define GB_PORT_BUZZER PORTB


#define GB_PIN_BUTTON_S1 A0
#define GB_PIN_BUTTON_S2 A1

#define GB_PIN_ANODE_1 A2 
#define GB_PIN_ANODE_2 A3 
#define GB_PIN_ANODE_3 A4 
#define GB_PIN_ANODE_4 A5 

#define GB_BIT_ANODE 0x04
#define GB_MASK_ANODE 0x3c
#define GB_PORT_ANODE PORTC

#define GB_PIN_DS1302_CE 12
#define GB_PIN_DS1302_SCLK 10
#define GB_PIN_DS1302_IO 11

// sensors defines

#define GB_SENSOR_TEMP_PIN 1
#define GB_SENSOR_LIGHT_PIN 0
#define GB_SENSOR_TEMP_R1 10000.0
#define GB_SENSOR_TEMP_RES 10000.0
#define GB_SENSOR_TEMP_BETA 3950
#define GB_SENSOR_LIGHT_R1 10000.0
#define GB_SENSOR_LIGHT_RES 15000.0


// DS1302 defines

#define DS1302_SECONDS           0x80
#define DS1302_MINUTES           0x82
#define DS1302_HOURS             0x84
#define DS1302_DATE              0x86
#define DS1302_MONTH             0x88
#define DS1302_DAY               0x8A
#define DS1302_YEAR              0x8C
#define DS1302_ENABLE            0x8E
#define DS1302_TRICKLE           0x90
#define DS1302_CLOCK_BURST       0xBE
#define DS1302_CLOCK_BURST_WRITE 0xBE
#define DS1302_CLOCK_BURST_READ  0xBF
#define DS1302_RAMSTART          0xC0
#define DS1302_RAMEND            0xFC
#define DS1302_RAM_BURST         0xFE
#define DS1302_RAM_BURST_WRITE   0xFE
#define DS1302_RAM_BURST_READ    0xFF

#define DS1302_READBIT 0x01
#define DS1302_RC      0x40
#define DS1302_CH      0x80
#define DS1302_AM_PM   0x20
#define DS1302_12_24   0x80
#define DS1302_WP      0x80
#define DS1302_ROUT0   0x01
#define DS1302_ROUT1   0x02
#define DS1302_DS0     0x04
#define DS1302_DS1     0x04
#define DS1302_TCS0    0x10
#define DS1302_TCS1    0x20
#define DS1302_TCS2    0x40
#define DS1302_TCS3    0x80

struct DS1302_dateTime {
  uint8_t seconds;
  uint8_t minutes; 
  uint8_t hour; 
  uint8_t date;
  uint8_t month;
  uint8_t day;
  uint8_t year;
  uint8_t control;
}; 

// key reader  defines

#define GB_KEY_FILTER_STEPS 30
#define GB_KEY_BUFFER_SIZE 4

// system defines

#define GB_ISR_FREQUENSY 20000 
#define GB_ISR_MICROSECONDS 1000000/GB_ISR_FREQUENSY
#define CLR(x,y) (x&=(~(1<<y)))
#define SET(x,y) (x|=(1<<y))
#define INVR(x,y) (x^=(1<<y))
#define isSET(x,y) (x&(1<<y))

CDipformer_Clock Clock;

const uint16_t notesFreq_PGM[] PROGMEM = { 0, 4186, 4435, 4699, 4978, 5274, 5588, 5920, 6272, 6645, 7040, 7459, 7902 };  // 5th octave
const uint8_t notesDurations_PGM[] PROGMEM = { 0, 1, 2, 3, 5, 7, 11, 15, 23, 31, 47, 63, 111, 127, 239, 255};  

uint16_t * buzzerBuf = 0;
uint16_t * buzzerBufForRepeat;
uint8_t buzzerTempo = 0;
uint32_t buzzerTimeCnt=0;



uint8_t clockDigits[GB_DIGIT_COUNT][8];
uint8_t clockPWM = 0;
uint8_t clockCurDigit = 0;

const uint8_t digit_Anodes_PGM[] PROGMEM = {GB_PIN_ANODE_1, GB_PIN_ANODE_2, GB_PIN_ANODE_3, GB_PIN_ANODE_4};
//const uint8_t digit_Chars_PGM[] PROGMEM = {0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x6f, 0x40, 0x39, 0x71, 0x63, 0x78, 0x00, 0x77, 0x37};  // -, C, F, °, t, ' ', A, n
const uint8_t digit_Chars_PGM[] PROGMEM = 
  {0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x6f,  // 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
   0x00, 0x40, 0x01, 0x04, 0x63, 0x00, // ' ', -, ^, _, °, reserved 
   0x77, 0x7c, 0x39, 0x5e, 0x79, 0x71, 0x76, 0x06,  0x31, 0x37, 0x3f, 0x73, 0x6d, 0x71, 0x7e};  // A, b, C, d, E, F, H, I, L, n, O, P, S, t, U
   
const uint8_t digit_SegmentsNormal_PGM[] PROGMEM = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80};
const uint8_t digit_SegmentsRotate_PGM[] PROGMEM = {0x08, 0x10, 0x20, 0x01, 0x02, 0x04, 0x40, 0x80};
const uint8_t digit_SegmentsPWM_PGM[] PROGMEM = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 10, 12, 14, 17, 20, 25, 31};  




uint8_t keysPositions[2] = {GB_KEYCODE_S1, GB_KEYCODE_S2};  // current keys state
int8_t keysFilter[2];
uint8_t keysState = 0;  // current keys state
uint8_t keyPressed = 0; 

uint8_t tracedKeyStates = 0;
uint8_t tracedKeyPressedCount = 0;
long tracedKeyMillis = 0;

uint16_t adcPinValue[2][2] = {{1023, 1023},{1023, 1023}};
uint16_t analogValue[2] = {1023, 1023};
uint8_t currentAnalogPin;

void CDipformer_Clock::begin () {
  pinMode(GB_PIN_BUZZER, OUTPUT);
  
  for (uint8_t i = 0; i<GB_DIGIT_COUNT; i++) pinMode(digit_Anodes_PGM[i], OUTPUT);
  for (uint8_t i = 2; i<10; i++) pinMode(i, OUTPUT);
  
  digitalWrite(GB_PIN_ANODE_1, HIGH);
  digitalWrite(GB_PIN_ANODE_2, HIGH);
  digitalWrite(GB_PIN_ANODE_3, HIGH);
  digitalWrite(GB_PIN_ANODE_4, HIGH);

  defIintensity = GB_MAX_INTENSITY;
  clearDigits ();
  show ();
  
  //for (uint8_t i=0; i<GB_KEY_FILTER_STEPS; i++) keysStates[i]=0;
  keysFilter[0] = 1;
  keysFilter[1] = 1;
  
  //RTC  1302 init
  uint8_t r;
  r = DS1302_read (DS1302_SECONDS);  
  if (r & DS1302_CH) DS1302_write (DS1302_SECONDS, r & ~DS1302_CH);
  r = DS1302_read (DS1302_ENABLE);  
  if (r & DS1302_WP) DS1302_write (DS1302_ENABLE, r & ~DS1302_WP);
  r = DS1302_read (DS1302_TRICKLE); 
  if (r == 0x5c) { // 1302 was power off
    DateTime dt = {0, 0, 8, 1, 1, 6, 2022, TIME_AM, TIME_24_HOUR_FORMAT};
    setDateTime (dt);
  } 
  DS1302_write (DS1302_TRICKLE, 0);   // charge disable         
  
  // enable interrupt TIMER1_COMPA_vect
  TCCR1A = 0;
  TCCR1B = (1 << WGM12) | (1 << CS11);
  TIMSK1 |= (1 << OCIE1A);
  OCR1A = 2000000/GB_ISR_FREQUENSY;  
  TCNT1 = 0;
  
  // ADC
  ADCSRA=(1<<ADEN)|(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0);
  currentAnalogPin = 0;
  ADMUX = 0x40 | currentAnalogPin; // AVcc
  ADCSRA |= (1<<ADSC); // start ADC
  

}



void CDipformer_Clock::playMelody (const uint16_t * buf) {
  playMelody (buf, MELODY_DEF_TEMPO);
}

void CDipformer_Clock::playMelody (const uint16_t * buf, uint8_t tempo) {
  CLR (TIMSK1, OCIE1A);
  buzzerTempo = tempo;
  buzzerBuf = (uint16_t*) buf;
  buzzerBufForRepeat = (uint16_t*) buf;
  buzzerTimeCnt = 0;
  SET (TIMSK1, OCIE1A);
}

void CDipformer_Clock::stopMelody () {
  CLR (TIMSK1, OCIE1A);
  buzzerBuf = 0;
  buzzerTimeCnt = 0;
  SET (TIMSK1, OCIE1A);
}

uint8_t CDipformer_Clock::isMelody () {
  if (buzzerTimeCnt != 0) return 1;
  return 0;
}

void CDipformer_Clock::setDigitIntensity (uint8_t n, uint8_t bit, uint8_t intensity) {
  if ((bit>=8) || (n>=GB_DIGIT_COUNT)) return;
  if (intensity <= GB_MAX_INTENSITY) digits[n][bit] = intensity;
  else digits[n][bit] = GB_MAX_INTENSITY;
}

void CDipformer_Clock::setIntensity (uint8_t intensity) {
  if (intensity > GB_MAX_INTENSITY) defIintensity = GB_MAX_INTENSITY;
  else defIintensity = intensity;
}
  
void CDipformer_Clock::setDigitSegments (uint8_t n, uint8_t bits) {
  if (n>=GB_DIGIT_COUNT) return;
  for (uint8_t i=0; i<8; i++) {
    uint8_t f = 0;
    if (bits & 1) f = defIintensity;
    digits[n][i] = f;
    bits = bits >> 1;
  }
}

void CDipformer_Clock::setDigitChar (uint8_t n, uint8_t ch) {
  if ((ch > 30) || (n>=GB_DIGIT_COUNT)) return;
  setDigitSegments (n, pgm_read_byte (digit_Chars_PGM + ch));
}

void CDipformer_Clock::setDot (uint8_t n) {
  if (n>=GB_DIGIT_COUNT) return;
  digits[n][7] = defIintensity;
} 

void CDipformer_Clock::writeInt (int8_t n, int v) {
  if (n < 0) return;
  if (v == 0) {
    if (n < GB_DIGIT_COUNT) setDigitChar (n, 0);
    return;
  }
  uint8_t s = 0;
  if (v < 0) {
    v = -v;
    s = 1;
  }
  while (v > 0) {
    uint8_t d = v % 10;
    if (n < GB_DIGIT_COUNT) setDigitChar (n, d);
    n--;
    v = v / 10;
  }
  if (s) {
    if (n < GB_DIGIT_COUNT) setDigitChar (n, DIGIT_CHAR_MINUS);
  }
}

void CDipformer_Clock::writeDigitInt (int8_t n, uint8_t count, int v) {
  if (n < 0) return;
  uint8_t s = 0;
  if (v < 0) {
    v = -v;
    s = 1;
  }
  while ((v > 0) && (count>0)) {
    uint8_t d = v % 10;
    if (n < GB_DIGIT_COUNT) setDigitChar (n, d);
    n--;
    v = v / 10;
    count--;
  }
  while (count>s) {
    if (n < GB_DIGIT_COUNT) setDigitChar (n, 0);
    n--;
    count--;
  }
  if ((s) && (count>0)) {
    if (n < GB_DIGIT_COUNT) setDigitChar (n, DIGIT_CHAR_MINUS);
  }
}       


void CDipformer_Clock::clearDigits () {
  for (uint8_t n = 0; n<GB_DIGIT_COUNT; n++) 
    for (uint8_t i = 0; i<8; i++)
      digits[n][i] = 0;
}

void CDipformer_Clock::show ()  {
  for (uint8_t n = 0; n<GB_DIGIT_COUNT; n++) 
    for (uint8_t i = 0; i<8; i++)
      clockDigits[n][i] = digits[n][i];  
}



uint8_t CDipformer_Clock::getKeysState () {
  return keysState;
}



uint8_t CDipformer_Clock::getPressedKey () {
  uint8_t key = keyPressed;
  keyPressed = 0;
  return key;

}

void CDipformer_Clock::resetKeyClicks () {
  tracedKeyStates = 0;
}


float CDipformer_Clock::getTemperature () {
  float vu = (float) analogValue[GB_SENSOR_TEMP_PIN] / 1023.0;
  float r = vu * GB_SENSOR_TEMP_R1 / (1.0 - vu); 
  return (GB_SENSOR_TEMP_BETA * 298.15) / (GB_SENSOR_TEMP_BETA + (298.15 * log(r / GB_SENSOR_TEMP_RES))) - 273.15;
}

float CDipformer_Clock::getLight () { 
  if (analogValue[GB_SENSOR_LIGHT_PIN] >= 1023) return 0;
  float vu = (float) analogValue[GB_SENSOR_LIGHT_PIN] / 1023.0;
  float r = vu * GB_SENSOR_LIGHT_R1 / (1.0 - vu); 
  return GB_SENSOR_LIGHT_RES / r * 15;
}
   
ISR(TIMER2_COMPA_vect) {
  INVR(GB_PORT_BUZZER, GB_BIT_BUZZER);
}

ISR(TIMER1_COMPA_vect) {
  
  // BUZZER
  if (buzzerTimeCnt > 0) { 
    buzzerTimeCnt--;
  }
  if (buzzerTimeCnt == 0) { 
    if (isSET(TIMSK2, OCIE2A)) {
      TCCR2B = (TCCR2B & 0b11111000);
      CLR(TIMSK2, OCIE2A);  // disable interrupt
      CLR(GB_PORT_BUZZER, GB_BIT_BUZZER);
    }
  }
  if ((buzzerTimeCnt == 0) && (buzzerBuf !=0 )) {
    uint16_t c = pgm_read_word (buzzerBuf++);
    if (c == MELODY_END) buzzerBuf=0;
    else if (c == MELODY_REPEAT) buzzerBuf = buzzerBufForRepeat;
    else {
      uint8_t note = c & 0xf;
      uint32_t duration = ((uint32_t)pgm_read_byte(notesDurations_PGM+((c & 0xf00) >> 8))+1)* buzzerTempo;
      if (note != 0) {
        uint32_t frequency = pgm_read_word (notesFreq_PGM+(note & 0xf));
        uint8_t oct = (c & 0xf0) >> 4;
        while (oct-- > 0) frequency=frequency>>1;
        //tone (13, f, dur);
        //buzzer_comp = 1000000 / 2 / frequency;   
        
        uint32_t ocr = F_CPU / frequency / 2 - 1;
        uint8_t prescalarbits = 0b001;  // ck/1: same for both timers
        if (ocr > 255) {
          ocr = F_CPU / frequency / 2 / 8 - 1;
          prescalarbits = 0b010;  // ck/8: same for both timers
          if (ocr > 255) {
            ocr = F_CPU / frequency / 2 / 32 - 1;
            prescalarbits = 0b011;
          }    
          if (ocr > 255) {
            ocr = F_CPU / frequency / 2 / 64 - 1;
            prescalarbits = 0b100;   
            if (ocr > 255) {
              ocr = F_CPU / frequency / 2 / 128 - 1;
              prescalarbits = 0b101;
            }    
            if (ocr > 255) {
              ocr = F_CPU / frequency / 2 / 256 - 1;
              prescalarbits = 0b110;
              if (ocr > 255) {
                // can't do any better than /1024
                ocr = F_CPU / frequency / 2 / 1024 - 1;
                prescalarbits = 0b111;
              }
            }
          }
        }
        TCCR2A = 0;
        SET(TCCR2A, WGM21);          
        TCCR2B = (TCCR2B & 0b11111000) | prescalarbits;
        OCR2A = ocr;
        SET(TIMSK2, OCIE2A);  // enable interrupt
      }   
      buzzerTimeCnt = GB_ISR_FREQUENSY / 1000 * duration;
    }
  }


  // DISPLAY
  clockPWM++;
  if (clockPWM >= 0x20) {
    clockPWM = 0;
    clockCurDigit++;
    if (clockCurDigit>=GB_DIGIT_COUNT) clockCurDigit = 0;
    GB_PORT_ANODE = (GB_PORT_ANODE | GB_MASK_ANODE) & (~(GB_BIT_ANODE<<clockCurDigit)); 
  }
  uint8_t b = 0;
  uint8_t * p = clockDigits[clockCurDigit]; 
  uint8_t i = 0;
  uint8_t t;
  while (i < 8) {
    t = pgm_read_byte (digit_SegmentsPWM_PGM + *p++);
    if (t > clockPWM) {
      if (clockCurDigit == 2) b|= pgm_read_byte (digit_SegmentsRotate_PGM+i);
      else b|= pgm_read_byte (digit_SegmentsNormal_PGM+i);
    }
    i++;
  }
  PORTB = (PORTB | 0x03) & (~(b & 0x03));   
  PORTD = (PORTD | 0xfc) & (~(b & 0xfc));  
  
  // KEYS AND SENSORS
  if (! (ADCSRA & (1 << ADSC))) {
    uint16_t adc = ADC;
    int8_t * pKeyFilter = keysFilter+currentAnalogPin;
    uint8_t keyBit = keysPositions[currentAnalogPin];
    long mls = millis ();
    uint16_t prev0 = adcPinValue[0][currentAnalogPin];
    uint16_t prev1 = adcPinValue[1][currentAnalogPin];
    adcPinValue[0][currentAnalogPin] = prev1;
    adcPinValue[1][currentAnalogPin] = adc;
    if ((prev0>1) && (prev1>1) && (adc>1)) analogValue[currentAnalogPin] = prev1;
    if (adc>1) {
      // not pressed
      if (keysState & keyBit) (*pKeyFilter)++;   
      else (*pKeyFilter)--;
    }
    else {
      // pressed
      if (keysState & keyBit) (*pKeyFilter)--;   
      else (*pKeyFilter)++;
    }
    if (*pKeyFilter < 0) *pKeyFilter = 0;
    if (*pKeyFilter >= GB_KEY_FILTER_STEPS) {
      uint8_t prev_keysState = keysState;
      keysState ^=keyBit;
      *pKeyFilter = 0;

      if (prev_keysState == 0) {  // push
        keyPressed = keysState;
        if (tracedKeyStates == keysState) {
          tracedKeyPressedCount++;
        }
        else {
          tracedKeyStates = keysState;
          tracedKeyPressedCount = 1;
        }
        tracedKeyMillis = mls;
      }   
    }
    if (tracedKeyStates != 0) {
      if (mls - tracedKeyMillis >= 500) {
        keyPressed = tracedKeyStates | (tracedKeyPressedCount << 4);
        tracedKeyStates = 0;
        tracedKeyPressedCount = 0;
      }
    }

    currentAnalogPin++;
    if (currentAnalogPin>1) currentAnalogPin=0;  
    ADMUX = 0x40 | currentAnalogPin; 
    ADCSRA |= (1<<ADSC); // start ADC  
  }

}

void CDipformer_Clock::DS1302_start () {
  digitalWrite(GB_PIN_DS1302_CE, LOW);
  pinMode(GB_PIN_DS1302_CE, OUTPUT);
  digitalWrite(GB_PIN_DS1302_SCLK, LOW);
  pinMode(GB_PIN_DS1302_SCLK, OUTPUT);
  pinMode(GB_PIN_DS1302_IO, OUTPUT);
  digitalWrite(GB_PIN_DS1302_CE, HIGH);
  delayMicroseconds(4);
}

void CDipformer_Clock::DS1302_stop () {
  digitalWrite (GB_PIN_DS1302_CE, LOW);
  delayMicroseconds(4);
}


uint8_t CDipformer_Clock::DS1302_toggleReadByte () {
  uint8_t data = 0;
  for (uint8_t i = 0; i < 8; i++) {
  	digitalWrite(GB_PIN_DS1302_SCLK, HIGH);
  	delayMicroseconds(1);
  	digitalWrite(GB_PIN_DS1302_SCLK, LOW);
  	delayMicroseconds(1);
    data |= digitalRead(GB_PIN_DS1302_IO) << i;
  }
  
  return data;
}

void CDipformer_Clock::DS1302_toggleWriteByte (uint8_t data, uint8_t release) {
  for (uint8_t i = 0; i < 8; i++) { 
    digitalWrite(GB_PIN_DS1302_IO, (data >> i) & 1);
    delayMicroseconds(1);
    digitalWrite(GB_PIN_DS1302_SCLK, HIGH);
    delayMicroseconds(1);
    if (release && (i == 7)) {
      pinMode (GB_PIN_DS1302_IO, INPUT);
    }
    else {
      digitalWrite(GB_PIN_DS1302_SCLK, LOW);
      delayMicroseconds(1);
    }
  }
}

uint8_t CDipformer_Clock::DS1302_read (uint8_t address) {
  address |= DS1302_READBIT;
  DS1302_start();
  DS1302_toggleWriteByte (address, 1);
  uint8_t data = DS1302_toggleReadByte ();
  DS1302_stop();
  return(data);
}

void CDipformer_Clock::DS1302_write (uint8_t address, uint8_t data) {
  address &= ~DS1302_READBIT;
  DS1302_start();
  DS1302_toggleWriteByte (address, 0); 
  DS1302_toggleWriteByte (data, 0); 
  DS1302_stop();
}


DateTime CDipformer_Clock::getDateTime () {
  DateTime dateTime;
  DS1302_dateTime dt;
  uint8_t * p = (uint8_t*)&dt;
  DS1302_start();
  DS1302_toggleWriteByte (DS1302_CLOCK_BURST_READ, 1);
  for (uint8_t i = 0; i<8; i++) *p++ = DS1302_toggleReadByte();
  DS1302_stop();
  
  dateTime.seconds = (dt.seconds & 0x0f) + ((dt.seconds & 0x70) >> 4) * 10;
  dateTime.minutes = (dt.minutes & 0x0f) + ((dt.minutes & 0x70) >> 4) * 10;
  dateTime.AM_PM = TIME_AM;
  if (dt.hour & DS1302_12_24) {
    dateTime.hour = (dt.hour & 0x0f) + ((dt.hour & 0x10) >> 4) * 10;  
    if (dt.hour & DS1302_AM_PM) dateTime.AM_PM = TIME_PM;
    dateTime.hour_12_24 = TIME_12_HOUR_FORMAT;
  }
  else {
    dateTime.hour = (dt.hour & 0x0f) + ((dt.hour & 0x30) >> 4) * 10;   
    dateTime.AM_PM = 0; 
    dateTime.hour_12_24 = TIME_24_HOUR_FORMAT;
  }
  dateTime.date = (dt.date & 0x0f) + ((dt.date & 0x30) >> 4) * 10;
  dateTime.month = (dt.month & 0x0f) + ((dt.month & 0x10) >> 4) * 10;
  dateTime.day = dt.day & 0x07;
  dateTime.year = (dt.year & 0x0f) + ((dt.year & 0xf0) >> 4) * 10 + 2000;
  //Serial.println (dt.hour);
  return dateTime;
}

void CDipformer_Clock::setDateTime (DateTime dateTime) {
  DS1302_dateTime dt;

  dt.seconds = (dateTime.seconds % 10) | (((dateTime.seconds / 10) << 4) & 0x70);
  dt.minutes = (dateTime.minutes % 10) | (((dateTime.minutes / 10) << 4) & 0x70);
  if (dateTime.hour_12_24 == TIME_12_HOUR_FORMAT) {   
    dt.hour = (dateTime.hour % 10) | (((dateTime.hour / 10) << 4) & 0x10) | DS1302_12_24;
    if (dateTime.AM_PM == TIME_PM) dt.hour |= DS1302_AM_PM;
    //Serial.print ("srt "); Serial.println (dt.hour);
  }
  else { 
    dt.hour = (dateTime.hour % 10) | (((dateTime.hour / 10) << 4) & 0x30);
  }
  dt.date = (dateTime.date % 10) | (((dateTime.date / 10) << 4) & 0x30);
  dt.month = (dateTime.month % 10) | (((dateTime.month / 10) << 4) & 0x10);
  dt.day = (dateTime.day & 0x07);
  uint16_t year = dateTime.year - 2000;
  dt.year = (year % 10) | ((year / 10) << 4) ;
  
  uint8_t * p = (uint8_t*)&dt;
  DS1302_start();
  DS1302_toggleWriteByte (DS1302_CLOCK_BURST_WRITE, 0);
  for (uint8_t i = 0; i<8; i++) DS1302_toggleWriteByte (*p++, 0);
  DS1302_stop();

}


void CDipformer_Clock::readRam (uint8_t * p) {
  DS1302_start();
  DS1302_toggleWriteByte (DS1302_RAM_BURST_READ, 1);
  for (uint8_t i = 0; i<DS1302_RAM_SIZE; i++) *p++ = DS1302_toggleReadByte();
  DS1302_stop();
}

void CDipformer_Clock::writeRam (uint8_t * p) {
  DS1302_start();
  DS1302_toggleWriteByte (DS1302_RAM_BURST_WRITE, 0);
  for (uint8_t i = 0; i<DS1302_RAM_SIZE; i++) DS1302_toggleWriteByte (*p++, 0);
  DS1302_stop();
}