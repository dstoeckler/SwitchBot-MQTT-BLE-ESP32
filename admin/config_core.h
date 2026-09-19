#pragma once
#include <string>
#include <vector>
#include <set>
#include <cstdint>

// Portable policy shared by firmware and host-side regression tests.
namespace admin {
struct Device {
  std::string id, mac, type, password, entity = "switch";
};
struct Config {
  int version = 1;
  std::string host, ssid, wifiPassword, mqttHost, mqttUser, mqttPassword, topic;
  int port = 1883, scanSeconds = 120, rescanSeconds = 10800, retries = 5;
  bool staticAddress = false;
  std::string ip, gateway, subnet, dns;
  std::vector<Device> devices;
};
inline bool identifier(const std::string &s, size_t max) {
  if (s.empty() || s.size() > max) return false;
  for (char c : s) if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
      (c >= '0' && c <= '9') || c == '_' || c == '-')) return false;
  return true;
}
inline bool macValid(const std::string &s) {
  if (s.size() != 17) return false;
  for (size_t i=0; i<s.size(); ++i) {
    char c=s[i];
    if (i%3==2) { if(c!=':') return false; }
    else if (!((c>='0' && c<='9') || (c>='a' && c<='f') || (c>='A' && c<='F'))) return false;
  }
  return true;
}
inline std::string lowerMac(std::string s) {
  for (char &c : s) if (c>='A' && c<='F') c += 'a'-'A';
  return s;
}
inline bool ipv4(const std::string &s) {
  int parts=0, value=0, digits=0;
  for (size_t i=0; i<=s.size(); ++i) {
    char c=i<s.size()?s[i]:'.';
    if(c=='.') { if(!digits || value>255 || ++parts>4) return false; value=0; digits=0; }
    else { if(c<'0'||c>'9'||++digits>3) return false; value=value*10+c-'0'; }
  }
  return parts==4;
}
inline std::string validate(const Config &c) {
  if(c.version!=1) return "Unbekannte Konfigurationsversion.";
  for(const auto *s:{&c.ssid,&c.wifiPassword,&c.mqttPassword,&c.mqttUser})
    for(unsigned char x:*s) if(x==0||x<32||x==127) return "Zugangsdaten enthalten Steuerzeichen.";
  if(!identifier(c.host,32) || c.host.find('_')!=std::string::npos) return "Hostname: 1–32 Buchstaben, Ziffern oder Bindestriche.";
  if(c.ssid.empty()||c.ssid.size()>32) return "SSID: 1–32 Bytes erforderlich.";
  if(!c.wifiPassword.empty() && (c.wifiPassword.size()<8||c.wifiPassword.size()>63)) return "WLAN-Passwort: leer oder 8–63 Zeichen.";
  if(c.mqttHost.empty()||c.mqttHost.size()>128) return "MQTT-Host fehlt oder ist zu lang.";
  for(char x:c.mqttHost) if(!((x>='a'&&x<='z')||(x>='A'&&x<='Z')||(x>='0'&&x<='9')||x=='.'||x=='-')) return "MQTT-Host muss ein DNS-Name oder eine IPv4-Adresse sein.";
  if(c.port<1||c.port>65535||c.mqttUser.size()>64||c.mqttPassword.size()>128) return "MQTT-Port oder Zugangsdaten ungueltig.";
  if(c.topic.empty()||c.topic.size()>96||c.topic.front()=='/'||c.topic.back()=='/') return "MQTT-Topic: 1–96 Zeichen, ohne aeussere Schraegen.";
  for(char x:c.topic) if(!((x>='a'&&x<='z')||(x>='A'&&x<='Z')||(x>='0'&&x<='9')||x=='/'||x=='_'||x=='-')) return "Topic enthaelt ungueltige Zeichen.";
  if(c.topic.find("//")!=std::string::npos) return "Topic enthaelt einen leeren Abschnitt.";
  if(c.staticAddress && (!ipv4(c.ip)||!ipv4(c.gateway)||!ipv4(c.subnet)||!ipv4(c.dns))) return "Statische IPv4-Adressen pruefen.";
  if(c.scanSeconds<1||c.scanSeconds>300||c.rescanSeconds<60||c.rescanSeconds>86400||c.retries<0||c.retries>10) return "Scan: 1–300 s, Intervall: 60–86400 s, Wiederholungen: 0–10.";
  if(c.devices.size()>16) return "Maximal 16 Geraete in der Web-Konfiguration.";
  std::set<std::string> names, macs;
  for(const auto &d:c.devices) {
    if(!identifier(d.id,32)||!macValid(d.mac)) return "Geraetename oder MAC-Adresse ungueltig.";
    if(!names.insert(d.id).second||!macs.insert(lowerMac(d.mac)).second) return "Geraetename und MAC muessen eindeutig sein.";
    if(d.type!="bot"&&d.type!="curtain"&&d.type!="meter"&&d.type!="contact"&&d.type!="motion"&&d.type!="plug") return "Unbekannter Geraetetyp.";
    if(d.password.size()>32 || (d.type!="bot"&&!d.password.empty())) return "Geraetepasswort nur fuer Bots, maximal 32 Zeichen.";
    for(unsigned char x:d.password) if(x<32||x==127) return "Bot-Passwort enthaelt Steuerzeichen.";
    if(d.entity!="switch"&&d.entity!="light"&&d.entity!="button") return "Unbekannte Bot-Darstellung.";
  }
  return "";
}
inline bool cleanupNeeded(const Config &a,const Config &b) {
  if(a.host!=b.host||a.topic!=b.topic||a.mqttHost!=b.mqttHost||a.port!=b.port||a.mqttUser!=b.mqttUser||a.mqttPassword!=b.mqttPassword||a.devices.size()!=b.devices.size()) return true;
  for(const auto &d:a.devices) {
    bool found=false;
    for(const auto &n:b.devices) if(d.id==n.id&&lowerMac(d.mac)==lowerMac(n.mac)&&d.type==n.type&&d.entity==n.entity) found=true;
    if(!found) return true;
  }
  return false;
}
}
