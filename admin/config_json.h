#pragma once
#include <ArduinoJson.h>
#include "config_core.h"

namespace admin {
inline void encode(JsonDocument &doc,const Config &c,bool secrets) {
  doc.clear();
  doc["version"]=c.version; doc["host"]=c.host; doc["ssid"]=c.ssid;
  doc["mqttHost"]=c.mqttHost; doc["mqttUser"]=c.mqttUser; doc["topic"]=c.topic;
  doc["port"]=c.port; doc["scanSeconds"]=c.scanSeconds; doc["rescanSeconds"]=c.rescanSeconds; doc["retries"]=c.retries;
  doc["staticAddress"]=c.staticAddress; doc["ip"]=c.ip; doc["gateway"]=c.gateway; doc["subnet"]=c.subnet; doc["dns"]=c.dns;
  if(secrets){doc["wifiPassword"]=c.wifiPassword;doc["mqttPassword"]=c.mqttPassword;}
  JsonArray devices=doc.createNestedArray("devices");
  for(const auto &d:c.devices){JsonObject o=devices.createNestedObject();o["id"]=d.id;o["mac"]=d.mac;o["type"]=d.type;o["entity"]=d.entity;if(secrets)o["password"]=d.password;}
}
inline bool decode(JsonVariantConst obj,const Config &old,Config &c,std::string &error) {
  if(!obj.is<JsonObjectConst>()){error="Konfiguration muss ein JSON-Objekt sein.";return false;}
  c=old;
  const char *strings[]={"host","ssid","mqttHost","mqttUser","topic","ip","gateway","subnet","dns"};
  std::string *targets[]={&c.host,&c.ssid,&c.mqttHost,&c.mqttUser,&c.topic,&c.ip,&c.gateway,&c.subnet,&c.dns};
  for(size_t i=0;i<9;++i){if(!obj[strings[i]].is<const char*>()){error=std::string("Textfeld fehlt: ")+strings[i];return false;}*targets[i]=obj[strings[i]].as<std::string>();}
  const char *integers[]={"version","port","scanSeconds","rescanSeconds","retries"};
  int *values[]={&c.version,&c.port,&c.scanSeconds,&c.rescanSeconds,&c.retries};
  for(size_t i=0;i<5;++i){if(!obj[integers[i]].is<int>()){error=std::string("Ganzzahl fehlt: ")+integers[i];return false;}*values[i]=obj[integers[i]].as<int>();}
  if(!obj["staticAddress"].is<bool>()){error="staticAddress muss true/false sein.";return false;}c.staticAddress=obj["staticAddress"].as<bool>();
  for(const char *key:{"wifiPassword","mqttPassword"}){
    if(obj.as<JsonObjectConst>().containsKey(key)){
      if(!obj[key].is<const char*>()){error="Passwort muss Text sein; zum Behalten weglassen.";return false;}
      (std::string(key)=="wifiPassword"?c.wifiPassword:c.mqttPassword)=obj[key].as<std::string>();
    }
  }
  if(!obj["devices"].is<JsonArrayConst>()||obj["devices"].size()>16){error="Geraeteliste fehlt oder umfasst mehr als 16 Geraete.";return false;}
  c.devices.clear();
  for(JsonObjectConst o:obj["devices"].as<JsonArrayConst>()) {
    for(const char *key:{"id","mac","type","entity"})if(!o[key].is<const char*>()){error="Geraetefeld fehlt.";return false;}
    Device d;d.id=o["id"].as<std::string>();d.mac=lowerMac(o["mac"].as<std::string>());d.type=o["type"].as<std::string>();d.entity=o["entity"].as<std::string>();
    if(o.containsKey("password")){
      if(!o["password"].is<const char*>()){error="Bot-Passwort muss Text sein.";return false;}
      d.password=o["password"].as<std::string>();
    } else for(const auto &previous:old.devices)if(lowerMac(previous.mac)==d.mac&&previous.type==d.type)d.password=previous.password;
    c.devices.push_back(d);
  }
  error=validate(c);return error.empty();
}
}
