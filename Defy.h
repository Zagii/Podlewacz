#ifndef Defy_h
#define Defy_h
#include <arduino.h>

#define R_PWM_NAWIEW 'N'  // pwm wiatrak 1
#define R_PWM_WYWIEW 'W'   // pwm wiatrak 2
#define R_ROZMRAZANIE_WIATRAKI 'R' //
#define R_ROZMRAZANIE_GGWC 'G' //
#define R_KOMINEK 'K' //
#define R_AUTO 'A' //tryb automatycznego dobrania predkosci
#define R_OFF 'O'

#define T_AUTO 'a'
#define T_MANUAL 'm'
#define T_KOMINEK 'k'
#define T_ROZMRAZANIE_WIATRAKI 'r'
#define T_ROZMRAZANIE_GGWC 'g'
#define T_OFF 'o'

#define KOMORA_CZERPNIA 0
#define KOMORA_WYRZUTNIA 1
#define KOMORA_NAWIEW 2
#define KOMORA_WYWIEW 3
#define KOMORA_ZEWN 4
#define KOMORA_SZT 4
#define WIATRAKI_SZT 2
#define LED 2

#define OFF HIGH
#define ON LOW

#define MAX_TOPIC_LENGHT 50
#define MAX_MSG_LENGHT 20

/*
 piny bez ryzyka
 D1, D2, D5, D6, D7
 
 flashowanie
 TX(D10),RX(D8)

 status boot
 D8, D3, D4
 */

#define WIATRAK_IN 0
#define WIATRAK_OUT 1
#define PIN_WIATRAK_CZERPNIA D6
#define PIN_TACHO_WIATRAK_CZERPNIA D2  //pin gpio4
#define PIN_WIATRAK_WYWIEW D5
#define PIN_TACHO_WIATRAK_WYWIEW D1   
#define PIN_ONEWIRE D7


#define DEBUG   //If you comment this line, the DPRINT & DPRINTLN lines are defined as blank.
#ifdef DEBUG    //Macros are usually in all capital letters.
  #define DPRINT(...)    Serial.print(__VA_ARGS__)     //DPRINT is a macro, debug print
  #define DPRINTLN(...)  Serial.println(__VA_ARGS__)   //DPRINTLN is a macro, debug print with new line
#else
  #define DPRINT(...)     //now defines a blank line
  #define DPRINTLN(...)   //now defines a blank line
#endif

bool isFloatChars(char * ctab);
bool isFloatString(String tString);
bool isIntChars(char * ctab) ;
 
#endif
