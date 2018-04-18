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
  time_t dataOdKiedy;
  /*uint8_t dzien;  //data od kiedy
  uint8_t mies;
  uint8_t rok;
  uint8_t h;      //o ktorej godzinie
  uint8_t m;
  uint8_t s;*/
  unsigned long czas_trwania;    //czas trwania
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

    void setProg(Program &a,int dzien, int mies, int rok,  int h, int m,  int s,  unsigned long czas_trwania,uint8_t co_ile_h,  uint8_t sekwencja);
    void setProg(Program &a, Program &b);
    void addProg(Program p);
    void publishProg(Program &p,uint8_t i=-1);
    void delProg(uint16_t id);
    void publishAllProg();
    bool checkRangeProg(uint8_t id,unsigned long sysczasms);
    
};


#endif
