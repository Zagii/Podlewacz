#include "Config.h"


void CConfig::begin()
{
  DPRINTLN("Mounting FS...");
  if (!SPIFFS.begin()) {
   DPRINTLN("Failed to mount file system");
    return;
  }
  if (!saveConfig()) {
   DPRINTLN("Failed to save config");
  } else {
   DPRINTLN("Config saved");
  }
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
void CConfig::setProg(Program &a,int dzien, int mies, int rok,  int h, int m,  int s,  unsigned long czas_trwania,uint8_t co_ile_h,  uint8_t sekwencja)
{
  DPRINT("setProg #");
   tmElements_t t;
  t.Year = rok;  t.Month = mies;  t.Day = dzien;
  t.Hour = h;  t.Minute = m;  t.Second = s;
  a.dataOdKiedy=makeTime(t);
  /*a->dzien=dzien; a->mies=mies;
  a->rok=rok;    a->h=h;
  a->m=m;        a->s=s;
  a->czas=czas; a->sekwencja=sekwencja; */
  /*a.dzien=dzien; a.mies=mies;
  a.rok=rok;    a.h=h;
  a.m=m;        a.s=s;*/
  a.czas_trwania=czas_trwania; a.sekwencja=sekwencja; 
  a.co_ile_s=(unsigned long) (3600*co_ile_h);
  //a.od=
 // a.
   publishProg(a,progIle);
}


void CConfig::setProg(Program &a, Program &b)
{
  DPRINT("addProg ref #");
  publishProg(b,progIle);
 /* a.dzien=b.dzien; a.mies=b.mies;
  a.rok=b.rok;    a.h=b.h;
  a.m=b.m;        a.s=b.s;*/
  //a.czas=b.czas; 
  tmElements_t t;
   breakTime(b.dataOdKiedy, t); 
  a.dataOdKiedy=makeTime(t);
  a.co_ile_s=b.co_ile_s;
  a.sekwencja=b.sekwencja; 
}


void CConfig::addProg(Program p)
{
  DPRINT("addProg #");
  publishProg(p,progIle);
  if(progIle+1>=MAX_PROGR)return;
  /*prTab[progIle].dzien=p.dzien; prTab[progIle].mies=p.mies;
  prTab[progIle].rok=p.rok;    prTab[progIle].h=p.h;
  prTab[progIle].m=p.m;        prTab[progIle].s=p.s;
  prTab[progIle].czas=p.czas;  prTab[progIle].sekwencja=p.sekwencja; */
  setProg(prTab[progIle],p);
  progIle++;
}



void CConfig::publishProg(Program &p,uint8_t i)
{
  DPRINT("ID="); DPRINT(i); DPRINT("; ");
 // DPRINT("data="); DPRINT(p.dzien); DPRINT("-");DPRINT(p.mies); DPRINT("-");DPRINT(p.rok); 
//  DPRINT("; godz="); DPRINT(p.h); DPRINT(":");DPRINT(p.m); DPRINT(":");DPRINT(p.s); 
//  DPRINT("czas="); DPRINT(p.czas); DPRINT("; ");DPRINT("sekwencja="); DPRINTLN(p.sekwencja); 
  
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

bool CConfig::checkRangeProg(uint8_t id,unsigned long sysczasms)
{
  
}

