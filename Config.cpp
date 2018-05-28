#include "Config.h"


void CConfig::begin()
{
  DPRINTLN("Mounting FS...");
  if (!SPIFFS.begin()) {
   DPRINTLN("Failed to mount file system");
    return;
  }
/*  if (!saveConfig()) {
   DPRINTLN("Failed to save config");
  } else {
   DPRINTLN("Config saved");
  }*/

  
  if (!loadConfig()) {
    DPRINTLN("Failed to load config");
  } else {
    DPRINTLN("Config loaded");
  }  
}

 bool CConfig::loadJSON( JsonObject& json, DynamicJsonBuffer jsonBuffer, const char *nazwaPliku)
 {
  File configFile = SPIFFS.open(nazwaPliku, "r");
  if (!configFile) {
     DPRINT("Blad odczytu pliku ");DPRINTLN(nazwaPliku);
   return false;
  }

  size_t size = configFile.size();
  if (size > 2048) {
    DPRINT("Za duży plik ");DPRINTLN(nazwaPliku);
    return false;
  }

  // Allocate a buffer to store contents of the file.
  std::unique_ptr<char[]> buf(new char[size]);

  // We don't use String here because ArduinoJson library requires the input
  // buffer to be mutable. If you don't use ArduinoJson, you may as well
  // use configFile.readString instead.
  configFile.readBytes(buf.get(), size);
  yield();
  const size_t bufferSize = JSON_ARRAY_SIZE(2) + 10*JSON_ARRAY_SIZE(6) + JSON_OBJECT_SIZE(1) + 80;
  
  json = jsonBuffer.parseObject(buf.get());

  if (!json.success()) {
    DPRINT("Blad parsowania json ");DPRINTLN(nazwaPliku);
    return false;
  }
  
  //uint8_t n = json["n"]; // progIle
  return true;
 }

bool CConfig::loadConfig() {
  File configFile = SPIFFS.open(PROGRAM_CONFIG_FILE, "r");
  if (!configFile) {
     DPRINT("Blad odczytu pliku ");DPRINTLN(PROGRAM_CONFIG_FILE);
   return false;
  }

  size_t size = configFile.size();
  if (size > 2048) {
    DPRINT("Za duży plik ");DPRINTLN(PROGRAM_CONFIG_FILE);
    return false;
  }

  // Allocate a buffer to store contents of the file.
  std::unique_ptr<char[]> buf(new char[size]);

  // We don't use String here because ArduinoJson library requires the input
  // buffer to be mutable. If you don't use ArduinoJson, you may as well
  // use configFile.readString instead.
  configFile.readBytes(buf.get(), size);
  yield();
  const size_t bufferSize = JSON_ARRAY_SIZE(2) + 10*JSON_ARRAY_SIZE(6) + JSON_OBJECT_SIZE(1) + 80;
  DynamicJsonBuffer jsonBuffer(bufferSize);
  JsonObject& json = jsonBuffer.parseObject(buf.get());

  if (!json.success()) {
    DPRINT("Blad parsowania json ");DPRINTLN(PROGRAM_CONFIG_FILE);
    return false;
  }
  
  uint8_t n = json["n"]; // progIle
  for(uint8_t i=0;i<n;i++)
  {
    yield();
    JsonArray& prog = json["Programy"][i];
    Program pp;
    setProg(pp,prog[0],prog[1],prog[2], prog[3], prog[4],prog[5]);
    addProg(pp);
  }
  DPRINT("progIle=");DPRINTLN(n);
  return true;
}

bool CConfig::saveConfigStr(const char *nazwaPliku,const char * str) {

  DPRINT(" saveConfigJSON: ");DPRINT(str);DPRINT(" / ");DPRINTLN(nazwaPliku);

  File configFile = SPIFFS.open(nazwaPliku, "w");
  if (!configFile) {
    DPRINT("Blad zapisu pliku ");DPRINTLN(nazwaPliku);
    return false;
  }
  configFile.println(str);
  return true;
}

bool CConfig::saveConfig() {

  DPRINT(" saveConfig bufferSize=");
  const size_t bufferSize = JSON_ARRAY_SIZE(2) + progIle*JSON_ARRAY_SIZE(5) + JSON_OBJECT_SIZE(1);
  DPRINTLN(bufferSize);
  DynamicJsonBuffer jsonBuffer(bufferSize);

  JsonObject& root = jsonBuffer.createObject();
    root["n"] = progIle;
  JsonArray& Programy = root.createNestedArray("Programy");
  for(uint8_t i=0;i<progIle;i++)
  {
    yield();
    JsonArray& pr = Programy.createNestedArray();
    pr.add(prTab[i].dataOdKiedy);
    pr.add(prTab[i].godzinaStartu);
    pr.add(prTab[i].czas_trwania_s);
    pr.add(prTab[i].sekwencja);
    pr.add(prTab[i].co_ile_dni);
    pr.add(prTab[i].aktywny);
  } 

  File configFile = SPIFFS.open(PROGRAM_CONFIG_FILE, "w");
  if (!configFile) {
    DPRINT("Blad zapisu pliku ");DPRINTLN(PROGRAM_CONFIG_FILE);
    return false;
  }
  root.printTo(Serial);
  root.printTo(configFile);
  return true;
}

//----------------------------------------------------
void CConfig::setProg(Program &a,uint8_t dzien, uint8_t mies, uint16_t rok,  uint8_t h, uint8_t m,  uint8_t s,  unsigned long czas_trwania_s,uint8_t co_ile_dni,  uint8_t sekwencja, bool aktywny)
{
  DPRINT("setProg #->");
  tmElements_t t;
  t.Year = CalendarYrToTm(rok);
  t.Month = mies;  t.Day = dzien;
  t.Hour = 0;  t.Minute = 0;  t.Second = 0;
  a.dataOdKiedy=makeTime(t);
  
  t.Year = CalendarYrToTm(1970);
  t.Month = 1;  t.Day = 1;
  t.Hour = h;  t.Minute = m;  t.Second = s;
  a.godzinaStartu=makeTime(t);
  
  a.czas_trwania_s=czas_trwania_s; 
  a.sekwencja=sekwencja; 
  a.co_ile_dni=co_ile_dni;
  a.aktywny=aktywny;
  publishProg(a,progIle);
}

void CConfig::setProg(Program &a,time_t data,time_t godzina,  unsigned long czas_trwania_s,uint8_t co_ile_dni,  uint8_t sekwencja,bool aktywny)
{ 
  DPRINT("setProg time_t #->");
  a.dataOdKiedy=data;
  a.godzinaStartu=godzina;
  a.czas_trwania_s=czas_trwania_s; 
  a.sekwencja=sekwencja; 
  a.co_ile_dni=co_ile_dni;
  a.aktywny=aktywny;
  publishProg(a,progIle);
  
}
void CConfig::setProg(Program &a, Program &b)
{
  DPRINT("addProg ref #");
  publishProg(b,progIle);
//  tmElements_t t;
 // breakTime(b.dataOdKiedy, t); 
 // a.dataOdKiedy=makeTime(t);
  a.dataOdKiedy=b.dataOdKiedy;
  a.godzinaStartu=b.godzinaStartu;
  a.co_ile_dni=b.co_ile_dni;
  a.czas_trwania_s=b.czas_trwania_s;
  a.sekwencja=b.sekwencja; 
  a.aktywny=b.aktywny;
}


void CConfig::addProg(Program p)
{
  DPRINT("addProg #");
  publishProg(p,progIle);
  if(progIle+1>=MAX_PROGR)return;

  setProg(prTab[progIle],p);
  progIle++;
}



void CConfig::publishProg(Program &p,uint8_t i)
{
  DPRINT("ID="); DPRINT(i); DPRINT("; ");
  DPRINT("data="); DPRINT(day(p.dataOdKiedy)); DPRINT("-");DPRINT(month(p.dataOdKiedy)); DPRINT("-");DPRINT(year(p.dataOdKiedy)); 
  DPRINT("; godz="); DPRINT(hour(p.godzinaStartu)); DPRINT(":");DPRINT(minute(p.godzinaStartu)); DPRINT(":");DPRINT(second(p.godzinaStartu)); 
  DPRINT(" czas_trwania_s="); DPRINT(p.czas_trwania_s); DPRINT("; ");DPRINT("co_ile_dni="); DPRINT(p.co_ile_dni); DPRINT("; ");
  DPRINT(" sekwencja="); DPRINT(p.sekwencja); DPRINT(" aktywny=");DPRINTLN(p.aktywny);
  
}

void CConfig::printCzas(time_t t)
{
  DPRINT("czas* time_t= ");DPRINT(t); 
  DPRINT("; data="); DPRINT(day(t)); DPRINT("-");DPRINT(month(t)); DPRINT("-");DPRINT(year(t)); 
  DPRINT("; godz="); DPRINT(hour(t)); DPRINT(":");DPRINT(minute(t)); DPRINT(":");DPRINT(second(t)); 
}

void CConfig::delProg(uint16_t id)
{
  DPRINT("delProg #");
  publishProg(prTab[id],id);
  if(id>=progIle)return;
  for(uint16_t i=id;i<progIle;i++)
  {
    yield();
    setProg(prTab[i],prTab[i+1]);
  }
  progIle--;
}

void CConfig::publishAllProg()
{
  for(uint16_t i=0;i<progIle;i++)
  {
    yield();
   publishProg(prTab[i],i);
  }
  
}

bool CConfig::checkRangeProg(Program &p,time_t sysczas_t)
{
 // DPRINT("checkRangeProg start, sysczas_s ");  printCzas(sysczas_t);DPRINTLN("");
 // publishProg(p);
  if(!p.aktywny) return false;
  
  if(p.dataOdKiedy>sysczas_t)
  {
    DPRINTLN(" Program z przyszlosci");
    return false;
  }
  time_t  czasSysOdKiedy = sysczas_t - p.dataOdKiedy;
  uint8_t deltaDni=czasSysOdKiedy/SEK_W_DNIU;
  if(deltaDni%p.co_ile_dni!=0)
  {
    // DPRINT(" Program nie z tego dnia. delta=");DPRINTLN(deltaDni);
    return false;
  }
   time_t aktualnaGodzina=sysczas_t % SEK_W_DNIU;

// DPRINT(" poczatkowaGodzina ");printCzas(p.godzinaStartu);DPRINT(" / ");
// DPRINT(" aktualnaGodzina ");printCzas(aktualnaGodzina);DPRINT(" / ");
// DPRINT(" koncowaGodzina ");printCzas(p.godzinaStartu+p.czas_trwania_s);DPRINT(" ");
 //DPRINT(" okno [ ");
 
  if((aktualnaGodzina >=p.godzinaStartu) && (p.godzinaStartu+p.czas_trwania_s>=aktualnaGodzina))
  {
 //   DPRINTLN(" w zakresie ]");
    return true;
  }
  else
  { 
  //  DPRINTLN(" poza zakresem ]");
    return false;
  }
}
uint8_t  CConfig::wlaczoneSekcje(time_t sysczas_s)
{
  uint8_t stan=0;
  for(uint16_t i=0;i<progIle;i++)
  {
    yield();
   // DPRINT("test programu:");DPRINTLN(i);
      if(checkRangeProg(prTab[i], sysczas_s))
      {
        bitSet(stan,prTab[i].sekwencja);
        //stan |=1<<prTab[i].sekwencja;
      }
  //  DPRINTLN(" koniec.");
  }
  return stan;
}

