#ifndef Config_h
#define Config_h

#include <ArduinoJson.h>
#include <Time.h>
#include <TimeLib.h>
#include "FS.h"
#include "Defy.h"


#define PROGRAM_CONFIG_FILE "/programy.json"

#define MAX_PROGR 250


typedef struct 
{
  time_t dataOdKiedy; //dzien przedstawia date od kiedy podlewac dla godz 00:00:00, 
  time_t godzinaStartu; //a godzina o której godzinie wzgledem daty
  /*uint8_t dzien;  //data od kiedy
  uint8_t mies;
  uint8_t rok;
  uint8_t h;      //o ktorej godzinie
  uint8_t m;
  uint8_t s;*/
  unsigned long czas_trwania_s;    //czas trwania
  uint8_t sekwencja;
  unsigned long co_ile_s;
}Program;

class CConfig
{
  uint16_t progIle=0;
  Program prTab[MAX_PROGR];
  
  public:
    CConfig(){};
    void begin();
    bool loadConfig();
    bool saveConfig();

    void setProg(Program &a,uint8_t dzien, uint8_t mies, uint16_t rok,  uint8_t h, uint8_t m,  uint8_t s,  unsigned long czas_trwania_s,uint8_t co_ile_h,  uint8_t sekwencja);
    void setProg(Program &a, Program &b);
    void addProg(Program p);
    void publishProg(Program &p,uint8_t i=-1);
    void delProg(uint16_t id);
    void publishAllProg();
    bool checkRangeProg(Program &p,time_t sysczas_s);
    void printCzas(time_t t);
    uint8_t wlaczoneSekcje(time_t sysczas_s);
    
};


#endif
