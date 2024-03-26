/*
  Firmware for Dipformer Clock board

  Settings:

  Key1 pressed 2 times. Time setting. Key1 toggles next parameter, Key2 change parameter.
   - Hour left digit
   - Hour right digit
   - Minutes left digit
   - Minutes right digit
   - AM or PM if set 12 hour format 
   - Month 
   - Day left digit
   - Day right digit
   - Year left digit
   - Year right digit

  Key1 pressed 3 times. Alarm setting. Key1 toggles next parameter, Key2 change parameter.
   - Hour left digit
   - Hour right digit
   - Minutes left digit
   - Minutes right digit
   - Alarm ON/OFF
   - Alarm melody

  Key1 pressed 4 times. Show and other settings
   - Show mode 1 - 7
     1 - time, date and temperature auto rotate, any Key or hand change it quickly
     2 - time and temperature auto rotate, Key1 or hand shows temperature, Key2 change it quickly  
     3 - time only, Key1 or hand shows temperature, Key2 shows date
     4 - temperature only, Key1 or hand shows time, Key2 shows date
     5 - digit off, Key1 or hand shows time, Key2 shows temperatureate
     6 - digit off, Key1 or hand shows time, Key2 shows date
     7 - digit off, Key1 or hand shows temperatureate, Key2 shows time
   - digits brightness (1-5)
   - digits auto brightness ON/OFF
   - 12 or 24 hour
   - temperature unit (C or F)

  Key1 pressed 5 times. Temp correction. Key2 add value from -5 to +5, Key1  - exit.
    
    
*/


#include <Dipformer_Clock.h>
#include <EEPROM.h>
#include <avr/wdt.h>

#define MODE_START 0
#define MODE_WORK 1
#define MODE_SETTING_TIME 2
#define MODE_SETTING_ALARM 3
#define MODE_SETTING_CLOCK 4
#define MODE_TEMPCORRECTION 5

#define SHOW_OFF 0
#define SHOW_TIME 1
#define SHOW_DATE 2
#define SHOW_TEMP 3

#define EEPROM_version 0xA5
#define EEPROM_versionAddr 0
#define EEPROM_settingsAddr 1

#define LIGHT_HISTORY_POINTS 20

DateTime dateTime;

struct  {
  uint8_t showMode;
  uint8_t brightness;
  uint8_t autoBrightness;
  int8_t temperatureCorrection; // -5 .. 5
  uint8_t temperatureUnit; // 0 - C, 1 - F
  uint8_t alarmMinutes;
  uint8_t alarmHour; 
  uint8_t alarmAM_PM; 
  uint8_t alarmEnable;
  uint8_t alarmMelody;
} settings;

uint8_t brightnessLevels[] = {2, 5, 9, 12, 15};
uint8_t daysOnMonth[] = {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};


// beep
const uint16_t melody1[] PROGMEM = { NOTE_PAUSE|DUR_4,
  NOTE_C|OCT_6|DUR_32, NOTE_PAUSE|DUR_16, 
  NOTE_C|OCT_6|DUR_32, NOTE_PAUSE|DUR_16, 
  NOTE_C|OCT_6|DUR_32, NOTE_PAUSE|DUR_16, 
  NOTE_C|OCT_6|DUR_32,  
  MELODY_REPEAT
};

// nokia
const uint16_t melody2[] PROGMEM = { NOTE_PAUSE|DUR_4, 
  NOTE_E|OCT_6|DUR_8, NOTE_D|OCT_6|DUR_8, NOTE_Fm|OCT_5|DUR_4, NOTE_Gm|OCT_5|DUR_4,
  NOTE_Cm|OCT_6|DUR_8, NOTE_B|OCT_5|DUR_8, NOTE_D|OCT_5|DUR_4, NOTE_E|OCT_5|DUR_4,
  NOTE_B|OCT_5|DUR_8, NOTE_A|OCT_5|DUR_8, NOTE_Cm|OCT_5|DUR_4, NOTE_E|OCT_5|DUR_4, NOTE_A|OCT_5|DUR_4,
  NOTE_PAUSE|DUR_4, 
  MELODY_REPEAT
};

// simphony
const uint16_t melody3[] PROGMEM = { NOTE_PAUSE|DUR_4, 
  NOTE_Eb|OCT_5|DUR_8, NOTE_D|OCT_5|DUR_16, NOTE_PAUSE|DUR_16, 
  NOTE_D|OCT_5|DUR_4, NOTE_Eb|OCT_5|DUR_8, NOTE_D|OCT_5|DUR_16, NOTE_PAUSE|DUR_16, NOTE_D|OCT_5|DUR_4, NOTE_Eb|OCT_5|DUR_8, NOTE_D|OCT_5|DUR_16, NOTE_PAUSE|DUR_16,
  NOTE_D|OCT_5|DUR_4, NOTE_Bb|OCT_5|DUR_4, NOTE_PAUSE|DUR_4, NOTE_Bb|OCT_5|DUR_8, NOTE_A|OCT_5|DUR_8,  
  NOTE_G|OCT_5|DUR_8P, NOTE_PAUSE|DUR_16, NOTE_G|OCT_5|DUR_8, NOTE_F|OCT_5|DUR_8, NOTE_Eb|OCT_5|DUR_8P, NOTE_PAUSE|DUR_16, NOTE_Eb|OCT_5|DUR_8, NOTE_D|OCT_5|DUR_8,
  NOTE_C|OCT_5|DUR_8P, NOTE_PAUSE|DUR_16, NOTE_C|OCT_5|DUR_4, NOTE_PAUSE|DUR_4,
  NOTE_D|OCT_5|DUR_8, NOTE_C|OCT_5|DUR_16, NOTE_PAUSE|DUR_16, 
  NOTE_C|OCT_5|DUR_4, NOTE_D|OCT_5|DUR_8, NOTE_C|OCT_5|DUR_16, NOTE_PAUSE|DUR_16, NOTE_C|OCT_5|DUR_4, NOTE_D|OCT_5|DUR_8, NOTE_C|OCT_5|DUR_16, NOTE_PAUSE|DUR_16,
  NOTE_C|OCT_5|DUR_4, NOTE_A|OCT_5|DUR_4, NOTE_PAUSE|DUR_4, NOTE_A|OCT_5|DUR_8, NOTE_G|OCT_5|DUR_8,  
  NOTE_Fm|OCT_5|DUR_8P, NOTE_PAUSE|DUR_16, NOTE_Fm|OCT_5|DUR_8, NOTE_E|OCT_5|DUR_8, NOTE_D|OCT_5|DUR_8P, NOTE_PAUSE|DUR_16, NOTE_D|OCT_5|DUR_8, NOTE_C|OCT_5|DUR_8,
  NOTE_Bb|OCT_5|DUR_8P, NOTE_PAUSE|DUR_16, NOTE_Bb|OCT_5|DUR_4, NOTE_PAUSE|DUR_4,  
  NOTE_PAUSE|DUR_2, MELODY_REPEAT
};

// Jingle
const uint16_t melody4[] PROGMEM = { NOTE_PAUSE|DUR_4, 
  NOTE_A|OCT_4|DUR_16, NOTE_PAUSE|DUR_16, NOTE_A|OCT_4|DUR_16, NOTE_PAUSE|DUR_16, NOTE_A|OCT_4|DUR_8P, NOTE_PAUSE|DUR_16, 
  NOTE_A|OCT_4|DUR_16, NOTE_PAUSE|DUR_16, NOTE_A|OCT_4|DUR_16, NOTE_PAUSE|DUR_16, NOTE_A|OCT_4|DUR_8P, NOTE_PAUSE|DUR_16, 
  NOTE_A|OCT_4|DUR_8, NOTE_PAUSE|DUR_16, NOTE_C|OCT_5|DUR_16, NOTE_F|OCT_4|DUR_8, NOTE_G|OCT_4|DUR_8,
  NOTE_A|OCT_4|DUR_4, NOTE_PAUSE|DUR_4,
  NOTE_Bb|OCT_4|DUR_16, NOTE_PAUSE|DUR_16, NOTE_Bb|OCT_4|DUR_16, NOTE_PAUSE|DUR_16, NOTE_Bb|OCT_4|DUR_16, NOTE_PAUSE|DUR_16, NOTE_Bb|OCT_4|DUR_16, NOTE_PAUSE|DUR_16,
  NOTE_A|OCT_4|DUR_16, NOTE_PAUSE|DUR_16, NOTE_A|OCT_4|DUR_16, NOTE_PAUSE|DUR_16, NOTE_A|OCT_4|DUR_16, NOTE_PAUSE|DUR_16, NOTE_A|OCT_4|DUR_32, NOTE_PAUSE|DUR_32, NOTE_A|OCT_4|DUR_32, NOTE_PAUSE|DUR_32,
  NOTE_G|OCT_4|DUR_8, NOTE_D|OCT_4|DUR_8, NOTE_E|OCT_4|DUR_8, NOTE_F|OCT_4|DUR_8,
  NOTE_G|OCT_4|DUR_4, NOTE_PAUSE|DUR_4,
  NOTE_A|OCT_4|DUR_16, NOTE_PAUSE|DUR_16, NOTE_A|OCT_4|DUR_16, NOTE_PAUSE|DUR_16, NOTE_A|OCT_4|DUR_8P, NOTE_PAUSE|DUR_16, 
  NOTE_A|OCT_4|DUR_16, NOTE_PAUSE|DUR_16, NOTE_A|OCT_4|DUR_16, NOTE_PAUSE|DUR_16, NOTE_A|OCT_4|DUR_8P, NOTE_PAUSE|DUR_16, 
  NOTE_A|OCT_4|DUR_8, NOTE_PAUSE|DUR_16, NOTE_C|OCT_5|DUR_16, NOTE_F|OCT_4|DUR_8, NOTE_G|OCT_4|DUR_8,
  NOTE_A|OCT_4|DUR_4, NOTE_PAUSE|DUR_4,
  NOTE_Bb|OCT_4|DUR_16, NOTE_PAUSE|DUR_16, NOTE_Bb|OCT_4|DUR_16, NOTE_PAUSE|DUR_16, NOTE_Bb|OCT_4|DUR_8, NOTE_PAUSE|DUR_16, NOTE_Bb|OCT_4|DUR_16,  
  NOTE_A|OCT_4|DUR_16, NOTE_PAUSE|DUR_16, NOTE_A|OCT_4|DUR_16, NOTE_PAUSE|DUR_16, NOTE_A|OCT_4|DUR_16, NOTE_PAUSE|DUR_16, NOTE_A|OCT_4|DUR_32, NOTE_PAUSE|DUR_32, NOTE_A|OCT_4|DUR_32, NOTE_PAUSE|DUR_32,
  NOTE_C|OCT_5|DUR_16, NOTE_PAUSE|DUR_16, NOTE_C|OCT_5|DUR_16, NOTE_PAUSE|DUR_16, NOTE_Bb|OCT_4|DUR_8, NOTE_G|OCT_4|DUR_8,
  NOTE_F|OCT_4|DUR_4, NOTE_PAUSE|DUR_4,
  NOTE_PAUSE|DUR_4,MELODY_REPEAT 
};

const uint16_t * melodies[] = {melody1, melody2, melody3, melody4};

char effect_digits[12][2] = {{0, 0}, {1,0}, {2,0}, {3,0}, {3,1}, {3,2}, {3,3}, {2,3}, {1,3}, {0,3}, {0,4}, {0,5}};
char effect_intensity[] = {1, 3, 7, 12, 15};

const uint16_t effect_Sound_PGM [] PROGMEM = { 
  NOTE_Fm|OCT_4|DUR_16, NOTE_Fm|OCT_4|DUR_16, NOTE_Fm|OCT_4|DUR_16, NOTE_D|OCT_4|DUR_4, NOTE_PAUSE|DUR_16, 
  NOTE_E|OCT_4|DUR_16, NOTE_E|OCT_4|DUR_16, NOTE_E|OCT_4|DUR_16, NOTE_C|OCT_4|DUR_4, 
  MELODY_END
};

uint8_t modeState = MODE_START;
uint8_t showState = SHOW_OFF;
long showStateMillis = 0;

uint8_t settingStep = 0;

uint8_t prevSeconds = 0xff;
long cpuMillis = 0;
uint8_t halfSecond = 0;
uint8_t blynk = 0;
long counter = 0; // millis


float temperature = 25;
float light;
float lights[LIGHT_HISTORY_POINTS];  // array of history light

//uint8_t keyStates = 0;
//uint8_t tracedKeyStates = 0;
//uint8_t tracedKeyPressedCount = 0;
//long keyMillis = 0;
uint8_t keyEvent = 0; 

uint8_t handStates = 0;
long handMillis = 0;
float handLight = 0;

uint8_t temp_hour_12_24;

#define ALARM_WAIT 0
#define ALARM_ON 1
#define ALARM_OFF 2
uint8_t alarmState = ALARM_OFF; 


void setup() {
  wdt_enable(WDTO_8S);
  
  
  Clock.begin (); 
  
  if (restoreSettings () == 0) { // первый запуск контроллера, в памяти совсем нет данных
    Clock.resetClock();
    settings.showMode = 3;
    settings.brightness = 4;
    settings.autoBrightness = 1;
    settings.temperatureCorrection = 0;
    settings.temperatureUnit = 0;
    settings.alarmMinutes = 0;
    settings.alarmHour = 7;
    settings.alarmAM_PM = TIME_AM;
    settings.alarmEnable = 0;
    settings.alarmMelody = 1; 
    saveSettings ();    
  }

  light = 999;
  for (int i=0; i<LIGHT_HISTORY_POINTS; i++) lights[i] = light; 
  
  delay (300);

}

void saveSettings () {
  uint8_t * p = (uint8_t*)&settings;
  for (uint16_t i=0; i<sizeof(settings); i++) EEPROM.write(EEPROM_settingsAddr+i, *p++); 
  EEPROM.write(EEPROM_versionAddr, EEPROM_version);  
}

uint8_t restoreSettings () {
  if (EEPROM.read(EEPROM_versionAddr) == EEPROM_version) {
    uint8_t * p = (uint8_t*)&settings;
    for (uint16_t i=0; i<sizeof(settings); i++) *p++ = EEPROM.read(EEPROM_settingsAddr+i); 
    return 1;
  }
  return 0;
}

void loop() {
  wdt_reset();
  
  // sensors
  float curLight = Clock.getLight ();
  for (int i=0; i<LIGHT_HISTORY_POINTS-1; i++) lights[i] = lights[i+1];
  lights[LIGHT_HISTORY_POINTS-1] = curLight;
  light = lights[0];  
  temperature = Clock.getTemperature ();

  //////////////////
  // calculate key events
  //////////////////
  keyEvent = Clock.getPressedKey ();
  
  //////////////////
  // calculate hand
  //////////////////
  if ((millis () > 2000) && (keyEvent == 0)) {
    if (handStates == 0) {
      if ((curLight < lights[0] * 0.7) && (lights[0] >= 2)) {

        handStates = 1;
        handLight = lights[0];
        handMillis = millis ();       
      }
    }
    else {
      if (millis () - handMillis < 800) {
        if (curLight > handLight * 0.9) {
          keyEvent = 0xff; 
          handStates = 0;
          for (int i=0; i<LIGHT_HISTORY_POINTS; i++) lights[i] = handLight;
        }
      }
      else handStates = 0;
    }
  }


  if (alarmState == ALARM_ON) {
    if (keyEvent !=0) {
      Clock.stopMelody ();
      alarmState = ALARM_OFF;
      keyEvent = 0;
    }
  }
  
  //////////////////
  // show
  //////////////////
  if (modeState == MODE_START) {
    startEffects ();
    modeState = MODE_WORK;
  }
  else if (modeState == MODE_WORK) {
    dateTime = Clock.getDateTime ();
    alarmHandler ();
    keysHandlerModeWork ();
    
    if (prevSeconds != dateTime.seconds) { 
      prevSeconds = dateTime.seconds;
      cpuMillis = millis ();
      halfSecond = 0;
      showModeWork ();
    }
    else if (halfSecond == 0) {
      long ms = millis ();
      if (ms - cpuMillis >= 500) {
        halfSecond = 1;
        showModeWork ();
      }
    }    
  }
  else if (modeState == MODE_SETTING_TIME) {
    keysHandlerModeTimeSettings ();
    showModeTimeSettings ();
  }  
  else if (modeState == MODE_SETTING_ALARM) {
    keysHandlerModeAlarmSettings ();
    showModeAlarmSettings (); 
  }
  else if (modeState == MODE_SETTING_CLOCK) {
    keysHandlerModeClockSettings ();
    showModeClockSettings (); 
  } 
  else if (modeState == MODE_TEMPCORRECTION) {
    keysHandlerModeTempCorrection ();
    showModeTempCorrection ();
  }
  if (counter>=50) counter = counter - 50;
  else counter = 0;
  blynk++;
  if (blynk>=0x10) blynk = 0;
  delay (50);
}


void setShowState (uint8_t state) {
  showState = state;
  counter = 3000;  
}

void setShowState (uint8_t state, long c) {
  showState = state;
  counter = c;  
}

void setModeState (uint8_t state) {
  modeState = state;
  settingStep = 0; 
  Clock.resetKeyClicks ();
}

void addLeft10 (uint8_t * v, uint8_t maxV) {
  uint8_t a = *v % 10;
  uint8_t b = *v / 10; 
  b++;
  if (b > maxV) b = 0;
  *v = b * 10 + a;
}

void addRight10 (uint8_t * v, uint8_t maxV) {
  uint8_t a = *v % 10;
  uint8_t b = *v / 10; 
  a++;
  if (a > maxV) a = 0;
  *v = b * 10 + a;
}

void addInRange (uint8_t * v, uint8_t minV, uint8_t maxV) {
  *v = *v+1;
  if (*v > maxV) *v = minV;
}

void reverseOnOff (uint8_t * v) {
  if (*v == 0) *v = 1;
  else *v = 0;
}


void startEffects () {

  Clock.playMelody (effect_Sound_PGM);
  int pos = 0;
  for (int i=0; i<30; i++) {
    Clock.clearDigits ();
    int k = pos;
    for (int i = 0; i<5; i++) {
      Clock.setDigitIntensity (effect_digits[k][0], effect_digits[k][1], effect_intensity[i]);
      k++;
      if (k >= 12) k = 0;
    }
    Clock.show ();
    pos++;
    if (pos >= 12) pos = 0;
    delay (70);
  }
}

void alarmHandler () {
  uint8_t timeEqual = false;
  if ((dateTime.minutes == settings.alarmMinutes) && (dateTime.hour == settings.alarmHour)) {
    if (dateTime.hour_12_24 == TIME_24_HOUR_FORMAT) timeEqual = true;
    else if (dateTime.AM_PM == settings.alarmAM_PM) timeEqual = true; 
  }
  if (timeEqual) {
    if (alarmState == ALARM_WAIT) {
      if (settings.alarmEnable != 0) {
        alarmState = ALARM_ON;
        Clock.playMelody (melodies[settings.alarmMelody-1]); 
      }
      else alarmState = ALARM_OFF;
    }
  }
  else {
    if (ALARM_OFF == 2) alarmState = ALARM_WAIT; 
  }   
}

void keysHandlerModeWork () {
  if ((keyEvent != 0) && (Clock.isMelody ()) != 0) {
    Clock.stopMelody ();
    Clock.resetKeyClicks ();
    return;
  }
  switch (settings.showMode) {
    case 1:
      if (((keyEvent & 0xf0) == 0x10) || (keyEvent == 0xff)) counter = 0;
      break;
    case 2:  
      if ((keyEvent == 0x11) || (keyEvent == 0xff)) setShowState (SHOW_DATE);
      if (keyEvent == 0x12) counter = 0;
      break;
    case 3:  
      if ((keyEvent == 0x11) || (keyEvent == 0xff)) setShowState (SHOW_TEMP);
      if (keyEvent == 0x12) setShowState (SHOW_DATE);
      break;
    case 4:  
      if ((keyEvent == 0x11) || (keyEvent == 0xff)) setShowState (SHOW_TIME);
      if (keyEvent == 0x12) setShowState (SHOW_DATE);
      break;
    case 5:  
      if ((keyEvent == 0x11) || (keyEvent == 0xff)) setShowState (SHOW_TIME);
      if (keyEvent == 0x12) setShowState (SHOW_TEMP);
      break;
    case 6:  
      if ((keyEvent == 0x11) || (keyEvent == 0xff)) setShowState (SHOW_TIME);
      if (keyEvent == 0x12) setShowState (SHOW_DATE);
      break;
    case 7:  
      if ((keyEvent == 0x11) || (keyEvent == 0xff)) setShowState (SHOW_TEMP);
      if (keyEvent == 0x12) setShowState (SHOW_TIME);
      break;
  }
  if (keyEvent == 0x21) {
    setModeState (MODE_SETTING_TIME);
    dateTime = Clock.getDateTime ();
  }
  if (keyEvent == 0x31) setModeState (MODE_SETTING_ALARM);
  if (keyEvent == 0x41) {
    temp_hour_12_24 = dateTime.hour_12_24;
    setModeState (MODE_SETTING_CLOCK);
  }
  if (keyEvent == 0x51) setModeState (MODE_TEMPCORRECTION);  
  if (keyEvent == 0x61) startEffects ();  
}

void keysHandlerModeTimeSettings () {
  if (keyEvent == 0x01) {
    settingStep++;
    if (settingStep==10) { // save settings
      dateTime.seconds = 0;
      Clock.setDateTime (dateTime);
      saveSettings ();
      setModeState (MODE_WORK);          
      return;
    }
    if ((settingStep==4) && (dateTime.hour_12_24==TIME_24_HOUR_FORMAT)) {
      dateTime.AM_PM = TIME_AM;
      settingStep = 5; 
    }
    if (settingStep==1) {
      if ((dateTime.hour_12_24==TIME_24_HOUR_FORMAT) && (dateTime.hour >= 24)) dateTime.hour = 23;
      if ((dateTime.hour_12_24==TIME_12_HOUR_FORMAT) && (dateTime.hour >= 12)) dateTime.hour = 11;
    }
    if (settingStep==6) {
      uint8_t dof = daysOnMonth[dateTime.month-1];
      if (dateTime.date > dof) dateTime.date = dof;
    }
    if (settingStep==7) {
      if (dateTime.date == 0) dateTime.date = 1;
    }
  }
  if (keyEvent == 0x02) {
    uint8_t year = dateTime.year - 2000;
    if (settingStep==0) {
      if (dateTime.hour_12_24==TIME_24_HOUR_FORMAT) addLeft10 (&dateTime.hour, 2);
      if (dateTime.hour_12_24==TIME_12_HOUR_FORMAT) addLeft10 (&dateTime.hour, 1);
    }
    if (settingStep==1) {
      addRight10 (&dateTime.hour, 9);
      if ((dateTime.hour_12_24==TIME_24_HOUR_FORMAT) && (dateTime.hour >= 24)) dateTime.hour = 20;
      if ((dateTime.hour_12_24==TIME_12_HOUR_FORMAT) && (dateTime.hour >= 12)) dateTime.hour = 10;
    }
    if (settingStep==2) addLeft10 (&dateTime.minutes, 5);
    if (settingStep==3) addRight10 (&dateTime.minutes, 9);
    if (settingStep==4) reverseOnOff (&dateTime.AM_PM); 
    if (settingStep==5) addInRange (&dateTime.month, 1, 12); 
    if (settingStep==6) {
      uint8_t dof = daysOnMonth[dateTime.month-1];
      if (dof < 30) addLeft10 (&dateTime.date, 2);
      else addLeft10 (&dateTime.date, 3);
    }
    if (settingStep==7) {
      addRight10 (&dateTime.date, 9);
      uint8_t dof = daysOnMonth[dateTime.month-1];
      if (dateTime.date > dof) dateTime.date = (dof / 10) * 10;
      if (dateTime.date == 0) dateTime.date = 1;
    }
    if (settingStep==8) addLeft10 (&year, 9);
    if (settingStep==9) addRight10 (&year, 9);    
    dateTime.year = year + 2000;
  }
}

void keysHandlerModeAlarmSettings () {
  if (keyEvent == 0x01) {
    settingStep++;
    if (settingStep==7) { // save settings
      Clock.stopMelody ();
      saveSettings ();
      setModeState (MODE_WORK);          
      return;
    }
    if (settingStep==1) {
      if ((dateTime.hour_12_24==TIME_24_HOUR_FORMAT) && (settings.alarmHour >= 24)) settings.alarmHour = 23;
      if ((dateTime.hour_12_24==TIME_12_HOUR_FORMAT) && (settings.alarmHour >= 12)) settings.alarmHour = 11;
    }
    if ((settingStep==4) && (dateTime.hour_12_24==TIME_24_HOUR_FORMAT)) {
      settings.alarmAM_PM = TIME_AM;
      settingStep = 5; 
    }
    if (settingStep==6) {
      Clock.playMelody (melodies[settings.alarmMelody-1]);
    }
  }
  if (keyEvent == 0x02) {
    if (settingStep==0) {
      if (dateTime.hour_12_24==TIME_24_HOUR_FORMAT) addLeft10 (&settings.alarmHour, 2);
      if (dateTime.hour_12_24==TIME_12_HOUR_FORMAT) addLeft10 (&settings.alarmHour, 1);
    }
    if (settingStep==1) {
      addRight10 (&settings.alarmHour, 9);
      if ((dateTime.hour_12_24==TIME_24_HOUR_FORMAT) && (settings.alarmHour >= 24)) settings.alarmHour = 20;
      if ((dateTime.hour_12_24==TIME_12_HOUR_FORMAT) && (settings.alarmHour >= 12)) settings.alarmHour = 10;
    }
    if (settingStep==2) addLeft10 (&settings.alarmMinutes, 5);
    if (settingStep==3) addRight10 (&settings.alarmMinutes, 9);
    if (settingStep==4) reverseOnOff (&settings.alarmAM_PM); 
    if (settingStep==5) reverseOnOff (&settings.alarmEnable);
    if (settingStep==6) {
      addInRange (&settings.alarmMelody, 1, 4);
      Clock.playMelody (melodies[settings.alarmMelody-1]);
    }
  }
  
}


void keysHandlerModeClockSettings () {
  if (keyEvent == 0x01) {
    settingStep++;
    if (settingStep==5) { 
      if (dateTime.hour_12_24 != temp_hour_12_24) {
        dateTime = Clock.getDateTime ();
        if (temp_hour_12_24 == TIME_24_HOUR_FORMAT) {
          if ((dateTime.AM_PM == TIME_PM) && (dateTime.hour < 12)) dateTime.hour += 12;
          dateTime.AM_PM = TIME_AM;
        }
        else {
          if (dateTime.hour >=12) {
            dateTime.hour -= 12;
            dateTime.AM_PM = TIME_PM;
          }
          else dateTime.AM_PM = TIME_AM;
        }
        dateTime.hour_12_24 = temp_hour_12_24;
        Clock.setDateTime (dateTime);
      }
      saveSettings ();
      setModeState (MODE_WORK);          
      return;
    }
  }
  if (keyEvent == 0x02) {
    if (settingStep==0) addInRange (&settings.showMode, 1, 7);
    if (settingStep==1) addInRange (&settings.brightness, 1, 5);
    if (settingStep==2) reverseOnOff (&settings.autoBrightness);
    if (settingStep==3) reverseOnOff (&temp_hour_12_24);
    if (settingStep==4) reverseOnOff (&settings.temperatureUnit);    
  }
}

void keysHandlerModeTempCorrection () {
  if (keyEvent == 0x01) {
    saveSettings ();
    setModeState (MODE_WORK);          
    return;
  }
  if (keyEvent == 0x02) {
    settings.temperatureCorrection++;
    if (settings.temperatureCorrection>5) settings.temperatureCorrection = -5;
  }
}

void showModeWork () {
    Clock.clearDigits ();
    uint8_t brightness = brightnessLevels[settings.brightness-1];
    if (settings.autoBrightness) {
      if (light<=GB_MAX_INTENSITY) brightness = int ((float)brightness/(float)GB_MAX_INTENSITY * light);
      if (brightness==0) brightness = 1;
      if (brightness > GB_MAX_INTENSITY) brightness = GB_MAX_INTENSITY;
    }
    Clock.setIntensity (brightness);   

    if (counter==0) {
      if (settings.showMode == 1) {
        if (showState == SHOW_TIME) setShowState (SHOW_DATE);
        else if (showState == SHOW_DATE) setShowState (SHOW_TEMP);
        else setShowState (SHOW_TIME, 10000);
      }
      else if (settings.showMode == 2) {
        if (showState == SHOW_TIME) setShowState (SHOW_TEMP);
        else setShowState (SHOW_TIME, 10000); 
      }       
      else if (settings.showMode == 3) showState = SHOW_TIME;
      else if (settings.showMode == 4) showState = SHOW_TEMP;
      else showState = SHOW_OFF;
    } 

    if (showState ==  SHOW_TIME) showTime ();
    else if (showState ==  SHOW_DATE) showDate ();
    else if (showState ==  SHOW_TEMP) showTemperature ();
    Clock.show ();
}

void showModeTimeSettings () {
  Clock.clearDigits ();
  Clock.setIntensity (GB_MAX_INTENSITY); 
  halfSecond = 0;
  uint8_t bl = blynk & 0x04;
  if (settingStep<=3) {
    Clock.writeDigitInt (3, 2, dateTime.minutes);
    Clock.writeDigitInt (1, 2, dateTime.hour);
    if (bl) Clock.setDigitChar (settingStep, DIGIT_CHAR_NO);
    Clock.setDot (1);
    Clock.setDot (2);    
  }
  else if (settingStep<=4) {
    if (bl) {
      if (dateTime.AM_PM == 0) Clock.setDigitChar (1, DIGIT_CHAR_A);
      else Clock.setDigitChar (1, DIGIT_CHAR_P);
    }
    if (dateTime.AM_PM == 1) Clock.setDot (3);
  }
  else if (settingStep<=5) {
    if (bl) Clock.writeDigitInt (3, 2, dateTime.month);
    Clock.writeDigitInt (1, 2, dateTime.date);
    Clock.setDot (1);
  }
  else if (settingStep<=7) {
    Clock.writeDigitInt (3, 2, dateTime.month);
    Clock.writeDigitInt (1, 2, dateTime.date);
    if (bl) Clock.setDigitChar (settingStep-6, DIGIT_CHAR_NO);
    Clock.setDot (1);
  }
  else if (settingStep<=9) {
    Clock.writeDigitInt (3, 4, dateTime.year);
    if (bl) Clock.setDigitChar (settingStep-6, DIGIT_CHAR_NO);
  }
  Clock.show ();
}

void showModeAlarmSettings () {
  Clock.clearDigits ();
  Clock.setIntensity (GB_MAX_INTENSITY); 
  halfSecond = 0;
  uint8_t bl = blynk & 0x04;
  if (settingStep<=3) {
    Clock.writeDigitInt (3, 2, settings.alarmMinutes);
    Clock.writeDigitInt (1, 2, settings.alarmHour);
    if (bl) Clock.setDigitChar (settingStep, DIGIT_CHAR_NO);
    Clock.setDot (1);
    Clock.setDot (2);    
  }  
  else if (settingStep<=4) {
    if (bl) {
      if (settings.alarmAM_PM == 0) Clock.setDigitChar (1, DIGIT_CHAR_A);
      else Clock.setDigitChar (1, DIGIT_CHAR_P);
    }
    if (settings.alarmAM_PM == 1) Clock.setDot (3);
  }
  else if (settingStep<=5) {
    Clock.setDigitChar (0, DIGIT_CHAR_A);
    if (bl) showOnOff (settings.alarmEnable);
  }
  else if (settingStep<=6) {
    Clock.setDigitChar (0, DIGIT_CHAR_A);
    if (bl) Clock.writeDigitInt (3, 1, settings.alarmMelody);
  }
  Clock.show ();
}

void showModeClockSettings () {
  Clock.clearDigits ();
  Clock.setIntensity (GB_MAX_INTENSITY); 
  halfSecond = 0;
  uint8_t bl = blynk & 0x04;
  Clock.setDigitChar (0, settingStep+1);
  if (settingStep==0) {
    if (bl) Clock.writeDigitInt (3, 1, settings.showMode);
  }
  else if (settingStep<=1) {
    uint8_t brightness = brightnessLevels[settings.brightness-1];
    Clock.setIntensity (brightness); 
    if (bl) Clock.writeInt (3, settings.brightness);
  }
  else if (settingStep<=2) {
    if (bl) showOnOff (settings.autoBrightness);
  }
  else if (settingStep<=3) {
    if (bl) {
      if (temp_hour_12_24 == TIME_24_HOUR_FORMAT) Clock.writeDigitInt (3, 2, 24);
      else Clock.writeDigitInt (3, 2, 12);
    }    
  }
  else if (settingStep<=4) {
    if (bl) {
      if (settings.temperatureUnit == 0) Clock.setDigitChar (3, DIGIT_CHAR_C);
      else Clock.setDigitChar (3, DIGIT_CHAR_F);
    }    
  }
  Clock.show ();
}

void showModeTempCorrection () {
  Clock.clearDigits ();
  Clock.setIntensity (GB_MAX_INTENSITY);
  if (blynk & 0x04) showTemperature ();
  Clock.show ();
}

void showTime () {
  Clock.writeDigitInt (3, 2, dateTime.minutes);
  Clock.writeDigitInt (1, 2, dateTime.hour);
  if (dateTime.hour_12_24 == TIME_12_HOUR_FORMAT) {
    if (dateTime.AM_PM == 1) Clock.setDot (3);
  }
  if (halfSecond == 0) {
    Clock.setDot (1);
    Clock.setDot (2);
  }  
}

void showDate () {
  Clock.writeDigitInt (3, 2, dateTime.month);
  Clock.writeDigitInt (1, 2, dateTime.date);
  Clock.setDot (1);
}

void showTemperature () {
  float t = temperature;
  if (settings.temperatureUnit == 1) t = t * 9 / 5 + 32;
  Clock.writeInt (2, t + settings.temperatureCorrection);
  Clock.setDigitChar (3, DIGIT_CHAR_GRAD);  
}

void showOnOff (uint8_t v) {
  if (v != 0) {
    Clock.setDigitChar (2, DIGIT_CHAR_O);
    Clock.setDigitChar (3, DIGIT_CHAR_N);
  }
  else {
    Clock.setDigitChar (1, DIGIT_CHAR_O);
    Clock.setDigitChar (2, DIGIT_CHAR_F);
    Clock.setDigitChar (3, DIGIT_CHAR_F);
  }
}
