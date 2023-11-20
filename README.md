﻿# Библиотека для платы DIPFORMER "Часы настольные"

Плата **DIPFORMER Часы настольные** это Arduino совместимое устройство на микроконтроллере ATmega168 в форм-факторе настольных часов. Плата разработана для обучения программированию. 
Программирование платы осуществляется при помощи Arduino IDE и подключением библиотеки `Dipformer_Clock.h`. Библиотека 
поддерживает все основные функции по работе с железом платы, при использовании библиотеки написание программного 
кода полностью сосредотачивается на логике работы часов.

Плата **DIPFORMER Часы настольные** имеет следующие модули, размещенные на плате:
- дисплей состоящий из четырех семисегментных индикатора;
- часы реального времени;
- две кнопки управления и настройки;
- зуммер для воспроизведения звуков;
- датчик температуры;
- датчика освещенности.

Управление всеми перечисленными модулями реализовано в библиотеке для Arduino IDE. Поддержка перечисленных 
модулей разделена в библиотеке на пять отдельных групп:

- отображение;
- часы RTS;
- кнопки;
- датчики;
- звук.

## Инициализация
Подключение библиотеки для **DIPFORMER Часы настольные** осуществляется в два этапа. Первый - это собственно команда 
препроцессора для подключения библиотеки `#include <Dipformer_Clock.h>`. Второй - это в процедуре `setup()` 
инициализация класса библиотеки `Clock.begin ()`.

```
#include <Dipformer_Clock.h>

void setup() {
  Clock.begin (); 
}

void loop() {
}
```


## Oтображение
Для формирования кадра изображения необходимо использовать функции отображения, описанные далее. При этом вызов этих функции не будут напрямую выводить информацию на индикаторы. Формирование отображаемых данных будет происходить в отдельном фоновом кадре, не связанном с индикаторами и не влияя на то что сейчас отображают индикаторы. Что бы отобразить новый фоновый кадр на индикаторах,  необходимо вызвать функцию `Clock.show ()`. Данная функция переместит фоновый новый кадр на индикаторы, где он будет отображаться до следующего вызова `Clock.show ()` . Таким образом вы можете в произвольном порядке формировать новый кадр отображения не затрагивая текущие отображаемые данные. 

- `Clock.show()` - делает фоновый кадр видимым.

Каждый отдельный сегмент каждого индикатора может быть включен с любым уровнем яркости свечения от 0 до 15, где 0 означает что сегмент выключен. 

Сегменты индикатора нумеруются от 0 до 7. Сегмент с номером 7 отвечает за точку. Если для включения сегментов указывается не их номер а байт целиком, то каждый бит этого байта соответствует сегменту от 0 до 7.

Сами индикаторы нумеруются от 0 до 3. Левый индикатор имеет номер 0.

Функции индивидуального управления сегментами:

- `clearDigits ()` - очистить все сегменты на всех индикаторах.
- `setDigitIntensity (uint8_t n, uint8_t bit, uint8_t intensity)` - устанавливает яркость свечения `intensity` для сегмента `bit` индикатора `n`.
- `setDigitIntensity (uint8_t n, uint8_t intensity)` - установить яркость `intensity` для всех сегментов индикатора `n`.

Функции управления индикаторами:

- `setIntensity (uint8_t intensity)` - установить яркость `intensity` для всех последующих операций управления индикаторами.
- `setDigitSegments (uint8_t n, uint8_t seg)` - установить для индикатора `n` светящиеся сегменты из байта `seg`.
- `setDigitChar (uint8_t n, uint8_t ch)` - отобразить на индикаторе `n` символ из `ch` (в соответствии с таблицей символов от 0 до 30).
- `setDot (uint8_t n)` - включить точку на индикаторе `n`.
- `writeInt (int8_t n, int v)` - отобразить целое число `v` на индикаторах начиная с позиции индикатора `n`.
- `writeDigitInt (int8_t n, uint8_t count, int v)` - отобразить целое число `v` на индикаторах начиная с позиции индикатора `n` и отвести под число `count` индикаторов. Число будет выравнено к правой стороне. Если число не помещается на указанное количество индикаторов то оно будет обрезано. Если для отображения требуется меньше места то левые индикаторы будут заполнены нулями.

Пример вывода на индикаторы простого счетчика.

```
#include <Dipformer_Clock.h>

int counter = 0;

void setup() {
  Clock.begin (); 
}

void loop() {
  Clock.clearDigits ();
  Clock.setIntensity (15)
  Clock.writeInt (0, counter);
  Clock.show ();
  delay (400);
}
```

Таблица символов для функции `setDigitChar` .

| Код в десятичной системе счисления | Код в шестнадцатеричной системе счисления |      Символ      |
| :--------------------------------: | :---------------------------------------: | :--------------: |
|                 0                  |                   0x00                    |        0         |
|                 1                  |                   0x01                    |        1         |
|                 2                  |                   0x02                    |        2         |
|                 3                  |                   0x03                    |        3         |
|                 4                  |                   0x04                    |        4         |
|                 5                  |                   0x05                    |        5         |
|                 6                  |                   0x06                    |        6         |
|                 7                  |                   0x07                    |        7         |
|                 8                  |                   0x08                    |        8         |
|                 9                  |                   0x09                    |        9         |
|                 10                 |                   0x0A                    |      пробел      |
|                 11                 |                   0x0B                    |     - дефис      |
|                 12                 |                   0x0C                    |  ^ черта сверху  |
|                 13                 |                   0x0D                    |  _ черта снизу   |
|                 14                 |                   0x0E                    | ° символ градуса |
|                 15                 |                   0x0F                    | зарезервировано  |
|                 16                 |                   0x10                    |        A         |
|                 17                 |                   0x11                    |        b         |
|                 18                 |                   0x12                    |        C         |
|                 19                 |                   0x13                    |        d         |
|                 20                 |                   0x14                    |        E         |
|                 21                 |                   0x15                    |        F         |
|                 22                 |                   0x16                    |        H         |
|                 23                 |                   0x17                    |        I         |
|                 24                 |                   0x18                    |        L         |
|                 25                 |                   0x19                    |        n         |
|                 26                 |                   0x1A                    |        O         |
|                 27                 |                   0x1B                    |        P         |
|                 28                 |                   0x1C                    |        S         |
|                 29                 |                   0x1D                    |        t         |
|                 30                 |                   0x1E                    |        U         |



## Часы реального времени

Библиотека самостоятельно производит инициализацию и обмен данными с микросхемой DS1302. Для считывания и установки реального времени вы можете использовать две функции.

- `DateTime getDateTime ()` - получить текущее время.
- `setDateTime (DateTime dateTime)` - установить текущее время.

Для использования этих функций вам потребуется создать переменную структуры DateTime которая определена в библиотеке. В этой структуре размещены поля отображающие текущее время.

```
struct DateTime {
  uint8_t seconds; // секунды 0..59
  uint8_t minutes; // минуты 0..59
  uint8_t hour;    // часы 0..23 или 0..11 
  uint8_t date;    // день месяца 1..31
  uint8_t month;   // месяц 1..12
  uint8_t day;     // день недели 1..7
  uint16_t year;   // год, например 2024
  uint8_t AM_PM;   // 0 - AM, 1 = PM
  uint8_t hour_12_24; // 0 - 24 hours, 1 = 12 hours
}; 
```

Что бы получить и далее использовать текущее время можно выполнить следующий код:

```
DateTime dateTime; // создаем переменную для времени
dateTime = getDateTime (); // получаем время

// выводим время в порт Serial
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
```

Что бы записать новое время в часы реального времени, вы должны создать переменную структуры DateTime и заполнить все ее поля и вызвать функцию `setDateTime (DateTime dateTime)` . Это очень важно что бы все поля структуры должны быть заполнены. Кроме того все поля в этой структуре должны быть валидными. Это значит что например секунды могут быть заданы от 0 до 59, все остальные значения будут ошибочными.

```
DateTime dateTime;

dateTime.seconds = 0;
dateTime.minutes= 0;
dateTime.hour= 8;
dateTime.date= 1;
dateTime.month= 1;
dateTime.day= 1;
dateTime.year= 2024;
dateTime.AM_PM= 0;
dateTime.hour_12_24= 0;

setDateTime (DateTime dateTime);
```



## Кнопки

Библиотека постоянно опрашивает состояние кнопок и производит фильтрацию от дребезга контактов. Библиотека имеет буфер нажатых кнопок. Каждое нажатие кнопки помещается в буфер в фоновом режиме. Вы можете извлекать коды нажатых кнопок из буфера в том же порядке в котором они были нажаты. Так же библиотека отслеживает количество последовательных нажатий одной кнопки. 

Функции модуля кнопок:

- `uint8_t getKeysState ()` - возвращает текущее состояние кнопок в виде кода в соответствии с таблицей ниже.

| Код  | Состояние                          |
| :--: | ---------------------------------- |
|  0   | Кнопки не нажаты                   |
|  1   | Нажата кнопка S1                   |
|  2   | Нажата кнопка S2                   |
|  3   | Нажата кнопка S1 и S2 одновременно |

- `uint8_t getPressedKey ()` - возвращает следующий код нажатой кнопки из буфера. Если не было нажатых кнопок то вернет 0. Функция возвращает сложный код, который включает в себя номер нажатой кнопки в соответствии с таблицей выше и количество раз сколько эта кнопка была нажата если кнопку нажали несколько раз. Под код кнопки отводится четыре правых бита 0-3. Под количество нажатий отводятся левые четыре бита 4-7. Пример: если была нажата кнопка S1 то функция вернет значение 0x01; если кнопка S1 был нажата 3 раза то функция вернет значение 0x31. 
- `resetKeyClicks ()` - очистить буфер нажатий.



## Датчики

Плата **DIPFORMER Часы настольные** имеет два датчика: датчик температуры и датчик освещенности. Библиотека полностью реализует считывание данных с датчиков, их фильтрацию и преобразование к удобным единицам измерения. Вы можете получить значения датчиков при помощи следующих функций:

- `float getTemperature ()` - получить текущую температуру в градусах цельсия.
- `float getLight ()` - получить текущую освещенность в LUX.



## Звук

Библиотека поддерживает воспроизведение монофонической мелодии в фоновом режиме. Это означает, что воспроизведение мелодии
выполняется совершенно параллельно вашей программе и не останавливает ее выполнение. 

Мелодии задаются при помощи массива в памяти программ `const uint16_t myMelody [] PROGMEM = {}`. Мелодия записывается с использованием
специальной азбуки, где каждый звук определяется его нотой, октавой и длительностью. Для определения конкретного звука все три 
параметра соединяются по функции ИЛИ. Например нота до первой октавы и длительностью одна восьмая будет иметь вид
`NOTE_C|OCT_4|DUR_8`. В конце мелодии обязательно должен присутствовать специальный код окончания мелодии `MELODY_END` или ее повторения `MELODY_REPEAT`.

Для воспроизведения мелодии необходимо вызвать функцию `GameBoard.playMelody (myMelody)`.

Пример определения массива c мелодией и код, запускающий ее воспроизведение при нажатии любой клавиши. 

```
const uint16_t myMelody [] PROGMEM = { 
  NOTE_C|OCT_5|DUR_32, 
  NOTE_E|OCT_5|DUR_32, 
  NOTE_G|OCT_5|DUR_32,  
  MELODY_END};

void loop () {  
  if (Clock.getPressedKey () != 0) { 
    Clock.playMelody (myMelody);
  }
}  
```

Функции для воспроизведения мелодий:

- `GameBoard.playMelody (const uint16_t * buf)` - запуск мелодии на воспроизведение в фоновом режиме. Программа сразу же начнет выполняться дальше.
- `GameBoard.playMelody (const uint16_t * buf, uint8_t tempo)` - запуск мелодии на воспроизведение с указанием темпа. По умолчанию используется темп равный 20.
- `GameBoard.stopMelody ()` - останавливает воспроизведение мелодии если она воспроизводится в текущий момент.


| Код | Нота |
|-------------------------|------------------------------|
| `NOTE_C` | **до** |
| `NOTE_Cm` или `NOTE_Db` | до диез или ре бемоль |
| `NOTE_D` | **ре** |
| `NOTE_Dm` или `NOTE_Eb` | ре диез или ми бемоль |
| `NOTE_E` | **ми** |
| `NOTE_F` | **фа** |
| `NOTE_Fm` или `NOTE_Gb` | фа диез или соль бемоль |
| `NOTE_G` | **соль** |
| `NOTE_Gm` или `NOTE_Ab` | соль диез или ля бемоль |
| `NOTE_A` | **ля** |
| `NOTE_Am` или `NOTE_Bb` | ля диез или си бемоль |
| `NOTE_B` | **си** |
| `NOTE_PAUSE` | пауза, октава не указывается |

| Код | Октава |
|-------------------------|------------------------------|
| `OCT_8` | пятая |
| `OCT_7` | четвертая |
| `OCT_6` | третья |
| `OCT_5` | вторая |
| `OCT_4` | первая |
| `OCT_3` | малая |
| `OCT_2` | большая |
| `OCT_1` | контр |
| `OCT_0` | суб-контр | 

| Код | Длительность |
|-------------------------|------------------------------|
| `DUR_64` | шестьдесят четвертая |
| `DUR_32` | тридцать вторая |
| `DUR_32P` | тридцать вторая с точкой |
| `DUR_16` | шестнадцатая |
| `DUR_16P` | шестнадцатая с точкой |
| `DUR_8` | восьмая |
| `DUR_8P` | восьмая с точкой |
| `DUR_4` | четвертная |
| `DUR_4P` | четвертная с точкой |
| `DUR_2` | половинная |
| `DUR_2P` | половинная с точкой |
| `DUR_1` | целая |
| `DUR_1P` | целая с точкой |
| `DUR_F2` | две целых |
| `DUR_F2P` | две целых с точкой |
| `DUR_F4` | четыре целых |