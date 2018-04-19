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

bool CConfig::loadConfig() {
  File configFile = SPIFFS.open(PROGRAM_CONFIG_FILE, "r");
  if (!configFile) {
     DPRINT("Blad odczytu pliku ");DPRINTLN(PROGRAM_CONFIG_FILE);
   return false;
  }

  size_t size = configFile.size();
  if (size > 1024) {
    DPRINT("Za duży plik ");DPRINTLN(PROGRAM_CONFIG_FILE);
    return false;
  }

  // Allocate a buffer to store contents of the file.
  std::unique_ptr<char[]> buf(new char[size]);

  // We don't use String here because ArduinoJson library requires the input
  // buffer to be mutable. If you don't use ArduinoJson, you may as well
  // use configFile.readString instead.
  configFile.readBytes(buf.get(), size);

  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& json = jsonBuffer.parseObject(buf.get());

  if (!json.success()) {
    DPRINT("Blad parsowania json ");DPRINTLN(PROGRAM_CONFIG_FILE);
    return false;
  }

  const char* serverName = json["serverName"];
  const char* accessToken = json["accessToken"];

  // Real world application would store these values in some variables for
  // later use.

  Serial.print("Loaded serverName: ");
  Serial.println(serverName);
  Serial.print("Loaded accessToken: ");
  Serial.println(accessToken);
  return true;
}

bool CConfig::saveConfig() {
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& json = jsonBuffer.createObject();
  json["serverName"] = "api.example.com";
  json["accessToken"] = "128du9as8du12eoue8da98h123ueh9h98";

  File configFile = SPIFFS.open(PROGRAM_CONFIG_FILE, "w");
  if (!configFile) {
    DPRINT("Blad zapisu pliku ");DPRINTLN(PROGRAM_CONFIG_FILE);
    return false;
  }

  json.printTo(configFile);
  return true;
}

//----------------------------------------------------
void CConfig::setProg(Program &a,uint8_t dzien, uint8_t mies, uint16_t rok,  uint8_t h, uint8_t m,  uint8_t s,  unsigned long czas_trwania_s,uint8_t co_ile_h,  uint8_t sekwencja)
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
  a.co_ile_s=(unsigned long) (3600*co_ile_h);
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
  a.co_ile_s=b.co_ile_s;
  a.czas_trwania_s=b.czas_trwania_s;
  a.sekwencja=b.sekwencja; 
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
  DPRINT(" czas_trwania_s="); DPRINT(p.czas_trwania_s); DPRINT("; ");DPRINT("co_ile_s="); DPRINT(p.co_ile_s); DPRINT("; ");
  DPRINT(" sekwencja="); DPRINTLN(p.sekwencja); 
  
}

void CConfig::printCzas(time_t t)
{
  DPRINT("czas* time_t= ");DPRINT(t); 
  DPRINT("data="); DPRINT(day(t)); DPRINT("-");DPRINT(month(t)); DPRINT("-");DPRINT(year(t)); 
  DPRINT("; godz="); DPRINT(hour(t)); DPRINT(":");DPRINT(minute(t)); DPRINT(":");DPRINT(second(t)); 
}

void CConfig::delProg(uint16_t id)
{
  DPRINT("delProg #");
  publishProg(prTab[id],id);
  if(id>=progIle)return;
  for(uint16_t i=id;i<progIle;i++)
  {
    setProg(prTab[i],prTab[i+1]);
  }
  progIle--;
}

void CConfig::publishAllProg()
{
  for(uint16_t i=0;i<progIle;i++)
  {
   publishProg(prTab[i],i);
  }
  
}

bool CConfig::checkRangeProg(Program &p,time_t sysczas_t)
{
  DPRINT("checkRangeProg start, sysczas_s");
  printCzas(sysczas_t);DPRINTLN("");
  DPRINT(" p.dataOdKiedy ");printCzas(p.dataOdKiedy);DPRINTLN("");
   DPRINT(" p.godzinaStartu ");printCzas(p.godzinaStartu);DPRINTLN("");
  time_t  czasSysOdKiedy = sysczas_t - p.dataOdKiedy;
  DPRINT(" czasSysOdKiedy ");printCzas(czasSysOdKiedy);DPRINTLN("");
  time_t czas_w_oknie= sysczas_t % p.co_ile_s;
 DPRINT(" czas_w_oknie ");printCzas(czas_w_oknie);DPRINTLN("");
  if(czas_w_oknie >=p.godzinaStartu && p.godzinaStartu+p.czas_trwania_s<=czas_w_oknie)
  {
    DPRINTLN(" w zakresie ");
    return true;
  }
  else
  { 
    DPRINTLN(" poza zakresem ");
    return false;
  }
}
uint8_t  CConfig::wlaczoneSekcje(time_t sysczas_s)
{
  for(uint16_t i=0;i<progIle;i++)
  {
    DPRINT("test programu:");DPRINTLN(i);
      checkRangeProg(prTab[i], sysczas_s);
    DPRINTLN(" koniec.");
  }
  return 0;
}

