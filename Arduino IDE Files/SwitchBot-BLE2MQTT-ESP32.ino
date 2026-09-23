/** SwitchBot-MQTT-BLE-ESP32:

  https://github.com/devWaves/SwitchBot-MQTT-BLE-ESP32

  **does not use/require switchbot hub

  Code can be installed using Arduino IDE OR using Visual Studio Code PlatformIO
    -For Arduino IDE - Use only the SwitchBot-BLE2MQTT-ESP32.ino file
  -For Visual Studio Code PlatformIO - Use the src/SwitchBot-BLE2MQTT-ESP32.cpp and platformio.ini files
  Allows for "unlimited" switchbots devices to be controlled via MQTT sent to ESP32. ESP32 will send BLE commands to switchbots and return MQTT responses to the broker
     ** I do not know where performance will be affected by number of devices **
     ** This is an unofficial SwitchBot integration. User takes full responsibility with the use of this code **

  v7.1

    Created: on Aug 17 2022
        Author: devWaves

        Contributions from:
                HardcoreWR
                vin-w

  based off of the work from https://github.com/combatistor/ESP32_BLE_Gateway

  Notes:
    - Supports Home Assistant MQTT Discovery

    - Support bots, curtains, temp meters, contact sensors, and motion sensors

    - It works for button press/on/off, set mode, set hold seconds

    - It works for curtain open/close/pause/position(%)

    - It can request status values (bots/curtain/meter/motion/contact: battery, mode, state, position, temp etc) using a "rescan" for all devices

    - It can request individual device status values (bots/curtain/meter/motion/contact: battery, mode, state, position, temp etc) using a "requestInfo"

    - Good for placing one ESP32 in a zone with 1 or more devices that has a bad bluetooth signal from your smart hub. MQTT will use Wifi to "boost" the bluetooth signal

    - ESP32 bluetooth is pretty strong and one ESP32 can work for entire house. The code will try around 60 times to connect/push button. It should not need this many but it depends on ESP32 bluetooth signal to switchbots. If one alone doesn't work, get another esp32 and place it in the problem area

    - OTA update added. Go to ESP32 IP address in browser. In Arduino IDE menu - Sketch / Export compile Binary . Upload the .bin file

    - Supports passwords on bot

    - Automatically rescan every X seconds

    - Automatically requestInfo X seconds after successful control command

    - Retry set/control command on busy response from bot/curtain until success

    - Get settings from bot (firmware, holdSecs, inverted, number of timers)

    - Add a defined delay between each set/control commands or per device

    - ESP32 will collect hold time from bots and automatically wait holdSecs+defaultBotWaitTime until next command is sent to bot

    - Retry on no response curtain or bot

    - holdPress = set bot hold value, then call press (without disconnecting in between)

    - Set/Control is prioritized over scanning. While scanning, if a set/control command is received scanning is stopped and resumed later

    - ESP32 can simulate ON/OFF for devices when bot is in PRESS mode. (Cannot guarantee it will always be accurate)

    - If you only have bots/curtain/meters the ESP32 will only scan when needed and requested. If you include motion or contact sensors the ESP32 will scan all the time

    - Curtain position will move as curtain moves when controlled from MQTT

    - Mesh 2 or more ESP32s together if you have motion or contact or meter. Mesh does not apply for bots or curtains

    - Active Scan (uses more battery of BLE devices) vs Passive Scan (uses less battery of BLE devices). Motion/Contact Sensors support Passive Scanning. Bot/Curtain/Meter require active scanning

  <ESPMQTTTopic> = <mqtt_main_topic>/<host>
      - Default = switchbot/esp32

  ESP32 will subscribe to MQTT 'set' topic for every configure device.
      - <ESPMQTTTopic>/bot/<name>/set
      - <ESPMQTTTopic>/curtain/<name>/set
      - <ESPMQTTTopic>/meter/<name>/set
      - <ESPMQTTTopic>/contact/<name>/set
      - <ESPMQTTTopic>/motion/<name>/set

    Send a payload to the 'set' topic of the device you want to control
      Strings:
        - "PRESS"
        - "ON"
        - "OFF"
        - "OPEN"
        - "CLOSE"
        - "PAUSE"
        - "STATEOFF"    (Only for bots in simulated ON/OFF mode)
        - "STATEON"     (Only for bots in simulated ON/OFF mode)

      Integer 0-100 (for curtain position) Example: 50
      Integer 0-100 (for setting bot hold seconds) Example: 5           (for bot only) Does the same thing as <ESPMQTTTopic>/setHold

      Strings:
        - "REQUESTINFO" or "GETINFO"                                    (for bot and curtain) Does the same thing as calling <ESPMQTTTopic>/requestInfo
        - "REQUESTSETTINGS" or "GETSETTINGS"                            (for bot only) Does the same thing as calling <ESPMQTTTopic>/requestSettings
        - "MODEPRESS", "MODESWITCH", "MODEPRESSINV", "MODESWITCHINV"    (for bot only) Does the same thing as <ESPMQTTTopic>/setMode

                      ESP32 will respond with MQTT on 'status' topic for every configured device
                        - <ESPMQTTTopic>/bot/<name>/status
                        - <ESPMQTTTopic>/curtain/<name>/status
                        - <ESPMQTTTopic>/meter/<name>/status

                        Example payload:
                          - {"status":"connected", "command":"ON"}
                          - {"status":"errorConnect", "command":"ON"}
                          - {"status":"errorCommand", "command":"NOTVALID"}
                          - {"status":"commandSent", "command":"ON"}
                          - {"status":"busy", "value":3, "command":"ON"}
                          - {"status":"failed", "value":9, "command":"ON"}
                          - {"status":"success", "value":1, "command":"ON"}
                          - {"status":"success", "value":5, "command":"PRESS"}
                          - {"status":"success", "command":"REQUESTINFO"}

                       ESP32 will respond with MQTT on 'state' topic for every configured device
                        - <ESPMQTTTopic>/bot/<name>/state
                        - <ESPMQTTTopic>/curtain/<name>/state

                        Example payload:
                          - "ON"
                          - "OFF"
                          - "OPEN"
                          - "CLOSE"

                        ESP32 will respond with MQTT on 'position' topic for every configured device
                        - <ESPMQTTTopic>/curtain/<name>/position

                        Example payload:
                          - {"pos":0}
                          - {"pos":100}
                          - {"pos":50}


  ESP32 will Subscribe to MQTT topic to rescan for all device information
      - <ESPMQTTTopic>/rescan

      send a JSON payload of how many seconds you want to rescan for
          example payloads =
            {"sec":30}
            {"sec":"30"}

  ESP32 will Subscribe to MQTT topic for device information
      - <ESPMQTTTopic>/requestInfo

      send a JSON payload of the device you want to requestInfo
          example payloads =
            {"id":"switchbotone"}

                      ESP32 will respond with MQTT on
                        - <ESPMQTTTopic>/#

                        Example attribute responses per device are detected:
                          - <ESPMQTTTopic>/bot/<name>/attributes
                          - <ESPMQTTTopic>/curtain/<name>/attributes
                          - <ESPMQTTTopic>/meter/<name>/attributes
                          - <ESPMQTTTopic>/contact/<name>/attributes
                          - <ESPMQTTTopic>/motion/<name>/attributes

                        Example response payloads:
                          - {"rssi":-78,"mode":"Press","state":"OFF","batt":94}
                          - {"rssi":-66,"calib":true,"batt":55,"pos":50,"state":"open","light":1}
                          - {"rssi":-66,"scale":"c","batt":55,"C":"21.5","F":"70.7","hum":"65"}

                        Example attribute responses per device are detected:
                          - <ESPMQTTTopic>/bot/<name>/state
                          - <ESPMQTTTopic>/curtain/<name>/state
                          - <ESPMQTTTopic>/meter/<name>/state
                          - <ESPMQTTTopic>/contact/<name>/state            (contact sensor has motion and contact. state = contact)
                          - <ESPMQTTTopic>/motion/<name>/state

                        Example payload:
                          - "ON"
                          - "OFF"
                          - "OPEN"
                          - "CLOSE"

                        ESP32 will respond with MQTT on 'position' topic for every configured device
                        - <ESPMQTTTopic>/curtain/<name>/position

                        Example response payload:
                          - {"pos":0}
                          - {"pos":100}
                          - {"pos":50}

                        Example topic responses specific to motion/contact sensors:
                          - <ESPMQTTTopic>/motion/<name>/motion           Example response payload: "MOTION", "NO MOTION"
                          - <ESPMQTTTopic>/motion/<name>/illuminance        Example response payload: "LIGHT", "DARK"
                          - <ESPMQTTTopic>/contact/<name>/contact         Example response payload: "OPEN", "CLOSED"
                          - <ESPMQTTTopic>/contact/<name>/motion          Example response payload: "MOTION", "NO MOTION"
                          - <ESPMQTTTopic>/contact/<name>/illuminance       Example response payload: "LIGHT", "DARK"
                          - <ESPMQTTTopic>/contact/<name>/in            Example response payload: "IDLE", "ENTERED"
                          - <ESPMQTTTopic>/contact/<name>/out           Example response payload: "IDLE", "EXITED"
                          - <ESPMQTTTopic>/contact/<name>/button                    Example response payload: "IDLE", "PUSHED"

                        Note:   You can use the button on the contact sensor to trigger other non-switchbot devices from your smarthub
                                When <ESPMQTTTopic>/contact/<name>/button = "PUSHED"


  // REQUESTSETTINGS WORKS FOR BOT ONLY - DOCUMENTATION NOT AVAILABLE ONLINE FOR CURTAIN
  ESP32 will Subscribe to MQTT topic for device settings information
      - <ESPMQTTTopic>/requestSettings

    send a JSON payload of the device you want to requestSettings
          example payloads =
            {"id":"switchbotone"}

                      ESP32 will respond with MQTT on
                        - <ESPMQTTTopic>/#

                      Example responses per device are detected:
                        - <ESPMQTTTopic>/bot/<name>/settings

                      Example payloads:
                         - {"firmware":4.9,"timers":0,"inverted":false,"hold":5}


  // SET HOLD TIME ON BOT
  ESP32 will Subscribe to MQTT topic setting hold time on bots
      - <ESPMQTTTopic>/setHold

    send a JSON payload of the device you want to set hold
          example payloads =
            {"id":"switchbotone", "hold":5}
            {"id":"switchbotone", "hold":"5"}

    ESP32 will respond with MQTT on
    - <ESPMQTTTopic>/#

                      ESP32 will respond with MQTT on 'status' topic for every configured device
                        - <ESPMQTTTopic>/bot/<name>/status

                        Example reponses:
                          - <ESPMQTTTopic>/bot/<name>/status

                        Example payload:
                          - {"status":"connected", "command":"5"}
                          - {"status":"errorConnect", "command":"5"}
                          - {"status":"errorCommand", "command":"NOTVALID"}
                          - {"status":"commandSent", "command":"5"}
                          - {"status":"busy", "value":3, "command":"5"}
                          - {"status":"failed", "value":9, "command":"5"}
                          - {"status":"success", "value":1, "command":"5"}
                          - {"status":"success", "value":5, "command":"5"}
                          - {"status":"success", "command":"REQUESTSETTINGS"}

  // holdPress = set bot hold value, then call press on bot (without disconnecting in between)
  ESP32 will Subscribe to MQTT topic to action a holdPress on bots
      - <ESPMQTTTopic>/holdPress

    send a JSON payload of the device you want to set hold then press
          example payloads =
            {"id":"switchbotone", "hold":5}
            {"id":"switchbotone", "hold":"5"}

    ESP32 will respond with MQTT on
    - <ESPMQTTTopic>/#

                      ESP32 will respond with MQTT on 'status' topic for every configured device
                        - <ESPMQTTTopic>/bot/<name>/status

                        Example reponses:
                          - <ESPMQTTTopic>/bot/<name>/status

                        Example response payload:
                          - {"status":"connected", "command":"5"}
                          - {"status":"errorConnect", "command":"5"}
                          - {"status":"errorCommand", "command":"NOTVALID"}
                          - {"status":"commandSent", "command":"5"}
                          - {"status":"busy", "value":3, "command":"5"}
                          - {"status":"failed", "value":9, "command":"5"}
                          - {"status":"success", "value":1, "command":"5"}
                          - {"status":"success", "value":5, "command":"5"}
                          - {"status":"success", "command":"REQUESTSETTINGS"}
                          - {"status":"connected", "command":"PRESS"}
                          - {"status":"errorConnect", "command":"PRESS"}
                          - {"status":"commandSent", "command":"PRESS"}
                          - {"status":"busy", "value":3, "command":"PRESS"}
                          - {"status":"failed", "value":9, "command":"PRESS"}
                          - {"status":"success", "value":1, "command":"PRESS"}
                          - {"status":"success", "value":5, "command":"PRESS"}


  // SET MODE ON BOT
  ESP32 will Subscribe to MQTT topic setting mode for bots
      - <ESPMQTTTopic>/setMode

    send a JSON payload of the device you want to set mode
          example payloads =
            {"id":"switchbotone", "mode":"MODEPRESS"}
            {"id":"switchbotone", "mode":"MODESWITCH"}
            {"id":"switchbotone", "mode":"MODEPRESSINV"}
            {"id":"switchbotone", "mode":"MODESWITCHINV"}

                      ESP32 will respond with MQTT on 'status' topic for every configured device
                        - <ESPMQTTTopic>/bot/<name>/status

                        Example reponses:
                          - <ESPMQTTTopic>/bot/<name>/status

                        Example payload:
                          - {"status":"connected", "command":"MODEPRESS"}
                          - {"status":"errorConnect", "command":"MODEPRESS"}
                          - {"status":"errorCommand", "command":"NOTVALID"}
                          - {"status":"commandSent", "command":"MODEPRESS"}
                          - {"status":"busy", "value":3, "command":"MODEPRESS"}
                          - {"status":"failed", "value":9, "command":"MODEPRESS"}
                          - {"status":"success", "value":1, "command":"MODEPRESS"}
                          - {"status":"success", "value":5, "command":"MODEPRESS"}

  ESP32 will respond with MQTT on ESPMQTTTopic with ESP32 status
      - <ESPMQTTTopic>

      example payloads:
        {status":"idle"}
        {status":"scanning"}
        {status":"boot"}
  {status":"controlling"}
  {status":"getsettings"}

  Errors that cannot be linked to a specific device will be published to
      - <ESPMQTTTopic>
*/

#include <algorithm>
#include <cctype>
#include <map>
#include <string>

#include <NimBLEDevice.h>
#include <EspMQTTClient.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <Update.h>
#include <ArduinoQueue.h>
#include <Preferences.h>

// BEGIN GENERATED ADMIN CORE
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
#include <ArduinoJson.h>

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
// END GENERATED ADMIN CORE

/****************** CONFIGURATIONS TO CHANGE *******************/

/********** REQUIRED SETTINGS TO CHANGE **********/

/* If using one ESP32 */
/* Enter all the switchbot device MAC addresses in the lists below */

/* If using multiple ESP32s - ESP32 can be meshed together for better motion/contact/meter performance */
/* Bot and Curtains and Plugs: (CANNOT BE MESHED) Enter the MAC addresses of the switchbot devices on the ESP32 closest to the switchbot device */
/* Motion and Contact and Meter: (CAN BE MESHED) Enter the MAC addresses of the switchbot devices into all or most of the ESP32s */

/* Wifi Settings */
static const char* host = "esp32";                                  //  Unique name for ESP32. The name detected by your router and MQTT. If you are using more then 1 ESPs to control different switchbots be sure to use unique hostnames. Host is the MQTT Client name and is used in MQTT topics
static const char* ssid = "SSID";                                   //  WIFI SSID
static const char* password = "Password";                           //  WIFI Password
static bool useStaticIP = false;                              //  Set true to use the fixed network settings below instead of DHCP
static const bool disableWiFiSleep = true;                          //  Keep WiFi awake to reduce MQTT/Home Assistant availability flapping
static IPAddress staticIP(192, 168, 0, 50);                         //  ESP32 fixed IP when useStaticIP = true
static IPAddress staticGateway(192, 168, 0, 1);                     //  Network gateway when useStaticIP = true
static IPAddress staticSubnet(255, 255, 255, 0);                    //  Network subnet when useStaticIP = true
static IPAddress staticPrimaryDNS(192, 168, 0, 1);                  //  Primary DNS when useStaticIP = true
static IPAddress staticSecondaryDNS(8, 8, 8, 8);                    //  Secondary DNS when useStaticIP = true

/* MQTT Settings */
/* MQTT Client name is set to WIFI host from Wifi Settings*/
static const char* mqtt_host = "192.168.0.1";                       //  MQTT Broker server ip
static const char* mqtt_user = "switchbot";                         //  MQTT Broker username. If empty or NULL, no authentication will be used
static const char* mqtt_pass = "switchbot";                         //  MQTT Broker password
static int mqtt_port = 1883;                                  //  MQTT Port
static std::string mqtt_main_topic = "switchbot";             //  MQTT main topic

/* Mesh Settings */
/* Ignore if only one ESP32 is used */
static const bool enableMesh = false;                               // Ignore if only one ESP32 is used. Set to false
static const char* meshHost = "";                                   // Ignore if only one ESP32 is used. Ignore if you don't have either meter/contact/motion. Enter the host value of the primary ESP32 if you are using multiple esp32s and you want to mesh them together for better contact/motion
static const bool meshMeters = true;                                // Mesh meters together if meshHost is set. The meter values will use the meshHost MQTT topics
static const bool meshContactSensors = true;                        // Mesh contact sensors together if meshHost is set. The contact values will use the meshHost MQTT topics.
static const bool meshMotionSensors = true;                         // Mesh motion sensors together if meshHost is set. The motion values will use the meshHost MQTT topics
static const bool onlyAllowRootESPToPublishContact = true;          // All meshed messages for contact and motions sensors will pass through the root mesh host ESP32. Only the root host will send contact and motion messages
static const bool onlyAllowRootESPToPublishMotion = false;          // All meshed messages for motion will pass through the root mesh host ESP32. Only the root host will send motion messages
static const bool onlyAllowRootESPToPublishLight = false;           // All meshed messages for illuminance will pass through the root mesh host ESP32. Only the root host will send illuminance messages
static const bool countContactToAvoidDuplicates = true;             // count the number of open/close/timeout over all esp32s so that none are duplicated
static const bool countMotionToAvoidDuplicates = false;             // count the number of motion/no motion over all esp32s so that none are duplicated
static const bool countLightToAvoidDuplicates = false;              // count the number of dark/bright over all esp32s so that none are duplicated
static const int timeToIgnoreDuplicates = 30;                       // if a duplicate is determined, ignore it within X seconds


/* Switchbot Bot Settings */
static std::map<std::string, std::string> allBots = {
  /*{ "switchbotone", "xX:xX:xX:xX:xX:xX" },
    { "switchbottwo", "yY:yY:yY:yY:yY:yY" }*/
};

/* Switchbot Curtain Settings */
static const int curtainClosedPosition = 10;    // When 2 curtains are controlled (left -> right and right -> left) it's possible one of the curtains pushes one of the switchbots more open. Change this value to set a position where a curtain is still considered closed
static std::map<std::string, std::string> allCurtains = {
  /*{ "curtainone", "xX:xX:xX:xX:xX:xX" },
    { "curtaintwo", "yY:yY:yY:yY:yY:yY" }*/
};

/* Switchbot Meter Settings */
static std::map<std::string, std::string> allMeters = {
  /*{ "meterone", "xX:xX:xX:xX:xX:xX" },
    { "metertwo", "yY:yY:yY:yY:yY:yY" }*/
};

/* Switchbot Contact Sensor Settings */
static std::map<std::string, std::string> allContactSensors = {
  /*{ "contactone", "xX:xX:xX:xX:xX:xX" },
    { "contacttwo", "yY:yY:yY:yY:yY:yY" }*/
};

/* Switchbot Motion Sensor Settings */
static std::map<std::string, std::string> allMotionSensors = {
  /*{ "motionone", "xX:xX:xX:xX:xX:xX" },
    { "motiontwo", "yY:yY:yY:yY:yY:yY" }*/
};

/* Switchbot Plug Mini Settings */
static std::map<std::string, std::string> allPlugs = {
  /*{ "plugone", "xX:xX:xX:xX:xX:xX" },
    { "plugtwo", "yY:yY:yY:yY:yY:yY" }*/
};

/* Switchbot Bot Passwords */
static std::map<std::string, std::string> allPasswords = {     // Set all the bot passwords (setup in app first). Ignore if passwords are not used
  /*{ "switchbotone", "switchbotonePassword" },
    { "switchbottwo", "switchbottwoPassword" }*/
};

/* Switchbot Bot Device Types - OPTIONAL */
/* Options include: "switch", "light", "button" */
static std::map<std::string, std::string> allBotTypes = {     // OPTIONAL - (DEFAULTS to "switch" if bot is not in list) - Will create HA entities for device types
  /* { "switchbotone", "switch" },
     { "switchbottwo", "light" },
     { "switchbotthree", "button" }*/
};

          /*** Bots in PRESS mode to simulate ON/OFF - ESP32 will try to keep track of the ON/OFF state of your device while in PRESS mode***/
          // Add bots while in PRESS mode that will simulate ON/OFF. Default state will be used if no MQTT retained on state topic
          // false = default state = OFF
          // true = default state = ON
          // If the state is incorrect, call set STATEOFF or STATEON
          static std::map<std::string, bool> botsSimulateONOFFinPRESSmode = {
            /*{ "switchbotone", false },
            { "switchbottwo", false }*/
          };

          //Add bots OFF hold time for simulated ON/OFF, if not in list, the current hold value will be used. Device must be in botsSimulateONOFFinPRESSmode list
          static std::map<std::string, int> botsSimulatedOFFHoldTimes = {
            /*{ "switchbotone", 0 },
              { "switchbottwo", 10 }*/
          };

          //Add bots ON hold time for simulated ON/OFF, if not in list, the current hold value will be used. Device must be in botsSimulateONOFFinPRESSmode list
          static std::map<std::string, int> botsSimulatedONHoldTimes = {
            /*{ "switchbotone", 15 },
              { "switchbottwo", 1}*/
          };

          // Add bots to be controlled using the ESP32 BOOT button (GPIO0). Leave empty to disable.
          static std::map<std::string, bool> botsControlledByESPButton = {
            /*{ "switchbotone", true },
              { "switchbottwo", true }*/
          };
/********************************************/


/********** ADVANCED SETTINGS - ONLY NEED TO CHANGE IF YOU WANT TO TWEAK SETTINGS **********/



/* ESP32 LED Settings */
#ifndef LED_BUILTIN
	#define LED_BUILTIN 2                            // If your board doesn't have a defined LED_BUILTIN, replace 2 with the LED pin value
#endif
static const bool ledHighEqualsON = true;            // ESP32 board LED ON=HIGH (Default). If your ESP32 LED is turning OFF on scanning and turning ON while IDLE, then set this value to false
static const bool ledOnESPButtonPress = true;        // Turn on LED while the ESP32 BOOT button is processing configured bot commands
static const bool ledOnBootScan = true;              // Turn on LED during initial boot scan
static const bool ledOnScan = true;                  // Turn on LED while scanning (non-boot)
static const bool ledOnCommand = true;               // Turn on LED while MQTT command is processing. If scanning, LED will blink after scan completes. You may not notice it, there is no delay after scan

/* Webserver Settings */
static String otaPass;                              // Loaded/generated in NVS; optional authentication for admin and OTA
static WebServer server(80);                         //  default port 80

/* Home Assistant Settings */
static const bool home_assistant_mqtt_discovery = true;                    // Enable to publish Home Assistant MQTT Discovery config
static const std::string home_assistant_mqtt_prefix = "homeassistant";     // MQTT Home Assistant prefix
static const bool home_assistant_expose_seperate_curtain_position = true;  // When enabled, a seperate sensor will be added that will expose the curtain position. This is useful when using the Prometheus integration to graph curtain positions. The cover entity doesn't expose the position for Prometheus
static const bool home_assistant_use_opt_mode = false;                     // For bots in switch mode assume on/off right away. Optimistic mode. (Icon will change in HA). If devices were already configured in HA, you need to delete them and reboot esp32
static const bool home_assistant_entity_names_include_device_name = false;  // false uses modern HA device-grouped names like "Battery" instead of "<device> Battery"

/* ESP32 General Settings */
static const bool includeSensorRecentFailures = true;                      // Include an MQTT sensor counting recent commands sent without receiving a valid response
static const bool includeSensorSystemInfo = true;                          // Include an MQTT sensor for ESP32 system status updates
static const int systemInfoTime = 60;                                      // How often to publish ESP32 system status updates in seconds
static const bool includeInfoBtMAC = false;                                // Include the ESP32 Bluetooth MAC in system info attributes

/* Switchbot General Settings */
static const int tryConnecting = 60;                         // How many times to try connecting to bot
static const int trySending = 30;                            // How many times to try sending command to bot
static int initialScan = 120;                          // How many seconds to scan for bots on ESP reboot and autoRescan. Once all devices are found scan stops, so you can set this to a big number
static const int infoScanTime = 60;                          // How many seconds to scan for single device status updates
static int rescanTime = 10800;                         // Automatically perform a full active scan for device info of all devices every X seconds (default 3 hours). XXXXActiveScanSecs will also active scan on schedule
static const int queueSize = 50;                             // Max number of control/requestInfo/rescan MQTT commands stored in the queue. If you send more then queueSize, they will be ignored
static const int defaultBotWaitTime = 2;                     // wait at least X seconds between control command send to bots. ESP32 will detect if bot is in press mode with a hold time and will add hold time to this value per device
static const int defaultCurtainWaitTime = 0;                 // wait at least X seconds between control command send to curtains
static const int waitForResponseSec = 20;                    // How many seconds to wait for a bot/curtain response
static int noResponseRetryAmount = 5;                  // How many times to retry if no response received
static const int defaultBotScanAfterControlSecs = 10;        // Default How many seconds to wait for state/status update call after set/control command. *override with botScanTime list
static const int defaultCurtainScanAfterControlSecs = 30;    // Default How many seconds to wait for state/status update call after set/control command. *override with botScanTime list. Also used by scanWhileCurtainIsMoving
static const int defaultBotMQTTUpdateSecs = 600;             // Used only when alwaysMQTTUpdate = false. Default MQTT Update for bot every X seconds. Note: a change in state will be always be published either way during active scanning
static const int defaultCurtainMQTTUpdateSecs = 600;         // Used only when alwaysMQTTUpdate = false. Default MQTT Update for curtain every X seconds.  Note: a change in state will be always be published either way during active scanning
static const int defaultMeterMQTTUpdateSecs = 600;           // Used only when alwaysMQTTUpdate = false. Default MQTT Update for meter temp sensors every X seconds. Note: a change in state will be always be published either way at all times (active or passive)
static const int defaultMotionMQTTUpdateSecs = 600;          // Used only when alwaysMQTTUpdate = false. Default MQTT Update for motion sensors every X seconds. Note: a change in state will be always be published either way at all times (active or passive)
static const int defaultContactMQTTUpdateSecs = 600;         // Used only when alwaysMQTTUpdate = false. Default MQTT Update for contact temp sensors every X seconds. Note: a change in state will be always be published either way at all times (active or passive)
static const int defaultPlugMQTTUpdateSecs = 600;            // Used only when alwaysMQTTUpdate = false. Default MQTT Update for motion sensors every X seconds. Note: a change in state will be always be published either way at all times (active or passive)
static const int defaultBotActiveScanSecs = 1800;            // Default Active Scan for bot every X seconds if not active scanned since X seconds. *override with botScanTime list.
static const int defaultCurtainActiveScanSecs = 1800;        // Default Active Scan for curtain every X seconds if not active scanned since X seconds. *override with botScanTime list.
static const int defaultMeterActiveScanSecs = 3600;          // Default Active Scan for meter temp sensors every X seconds if not active scanned since X seconds. *override with botScanTime list.
static const int defaultMotionActiveScanSecs = 3600;         // Default Active Scan for motion sensors every X seconds if not active scanned since X seconds. *override with botScanTime list.
static const int defaultContactActiveScanSecs = 3600;        // Default Active Scan for contact temp sensors every X seconds if not active scanned since X seconds. *override with botScanTime list.
static const int defaultPlugActiveScanSecs = 3600;           // Default Active Scan for motion sensors every X seconds if not active scanned since X seconds. *override with botScanTime list.
static const int waitForMQTTRetainMessages = 10;             // On boot ESP32 will look for retained MQTT state messages for X secs, otherwise default state is used. This is for bots in simulate ON/OFF, and when ESP32 mesh is used
static const int missedDataResend = 120;                     // Experimental. If a motion or contact is somehow missed while controlling bots, send the MQTT messages within X secs of it occuring as a backup. requires sendBackupMotionContact = true. Note: Not used if multiple ESP32s are meshed
static const int missedContactDelay = 30;                    // Experimental. If a contact is somehow missed while controlling bots, compare lastcontact from esp32 with contact sensor lastcontact. If different is greater than X, send a contact message. Note: Not used if multiple ESP32s are meshed
static const int missedMotionDelay = 30;                     // Experimental. If a motion is somehow missed while controlling bots, compare lastmotion from esp32 with motion/contact sensor lastmotion. If different is greater than X, send a motion message. Note: Not used if multiple ESP32s are meshed

static const bool sendBackupMotionContact = false;           // Experimental. Compares last contact/motion time value from switchbot contact/motion devices against what the esp32 received. If ESP32 missed one while controlling bots, it will send a motion/contact message after. Note: Not used if multiple ESP32s are meshed
static const bool autoRescan = true;                         // perform automatic rescan (uses rescanTime and initialScan).
static const bool activeScanOnSchedule = true;               // perform an active scan on decice types based on the scheduled seconds values for XXXXActiveScanSecs
static const bool scanAfterControl = true;                   // perform requestInfo after successful control command (uses botScanTime).
static const bool waitBetweenControl = true;                 // wait between commands sent to bot/curtain (avoids sending while bot is busy)
static const bool getSettingsOnBoot = true;                  // Currently only works for bot (curtain documentation not available but can probably be reverse engineered easily). Get bot extra settings values like firmware, holdSecs, inverted, number of timers. ***If holdSecs is available it is used by waitBetweenControl
static const bool retryBotOnBusy = true;                     // if bot responds with busy, the last control command will retry up to noResponseRetryAmount times
static const bool retryCurtainOnBusy = true;                 // if curtain responds with busy, the last control command will retry up to noResponseRetryAmount times
static const bool retryPlugOnBusy = true;                    // if plug responds with busy, the last control command will retry up to noResponseRetryAmount times
static const bool retryBotActionNoResponse = false;          // Retry if bot doesn't send a response. Bot default is false because no response can still mean the bot triggered.
static const bool retryPlugActionNoResponse = true;          // Retry if plug doesn't send a response. Default is true. It shouldn't matter if plug receives the same command twice (or multiple times)
static const bool retryBotSetNoResponse = true;              // Retry if bot doesn't send a response when requesting settings (hold, firwmare etc) or settings hold/mode
static const bool retryCurtainNoResponse = true;             // Retry if curtain doesn't send a response. Default is true. It shouldn't matter if curtain receives the same command twice (or multiple times)
static const bool immediateBotStateUpdate = true;            // ESP32 will send ON/OFF state update as soon as MQTT is received. You can set this = false if not using Home Assistant Discovery.
static const bool immediatePlugStateUpdate = true;           // ESP32 will send ON/OFF state update as soon as MQTT is received. You can set this = false if not using Home Assistant Discovery.
static const bool immediateCurtainStateUpdate = true;        // ESP32 will send OPEN/CLOSE and Position state update as soon as MQTT is received. You can set this = false if not using Home Assistant Discovery.
static const bool assumeNoResponseMeansSuccess = true;       // Only for bots in simulated ON/OFF: If the ESP32 does not receive a response after sending command (after noResponseRetryAmount reached and retryBotActionNoResponse = true) assume it worked and change state
static const bool alwaysMQTTUpdate = false;                  // If the ESP32 is scanning, always publish MQTT data instead of using set times. ***Note: This creates a lot of MQTT traffic
static const bool onlyActiveScan = false;                    // Active scanning requires more battery from the BLE switchbot devices. If false, passive scanning is used when possible for contact/motion
static const bool onlyPassiveScan = false;                   // If this ESP32 is a mesh ESP32 or you only have motion/contact/meter sensors. Passive scanning uses less battery from BLE switchbot devices. Passive scanning provides less data then active scanning, but uses less battery
static const bool alwaysActiveScan = false;                  // No battery optimizations. If you are using the switchbot hub or app to control devices also and you want immediate state updates for bot and curtains in MQTT set to true
static const bool scanWhileCurtainIsMoving = true;           // The ESP32 will scan for defaultCurtainScanAfterControlSecs seconds after control to keep the position slider in sync with the actual position

static const bool printSerialOutputForDebugging = false;     // Only set to true when you want to debug an issue from Arduino IDE. Lots of Serial output from scanning can crash the ESP32
static bool manualDebugStartESP32WithMQTT = false;           // Only set to true when you want to debug an issue. ESP32 will boot in an OFF state when set to true. To start the ESP32 processing send any MQTT message to the topic ESPMQTTTopic/manualstart. This will make it easier to catch the last serial output of the ESP32 before crashing

/* Switchbot Bot/Meter/Curtain scan interval */
/* Meters don't support commands so will be scanned every <int> interval automatically if scanAfterControl = true */
/* Requires scanAfterControl = true */
static std::map<std::string, int> botScanTime = {     // X seconds after a successful control command ESP32 will perform a requestInfo on the bot. If a "hold time" is set on the bot include that value + 5to10 secs. Hold time is auto added to default value. Default is 10+Hold sec if not in list
  /*{ "switchbotone", 10 },
    { "switchbottwo", 10 },
    { "curtainone", 20 },
    { "curtaintwo", 20 },
    { "meterone", 60 },
    { "metertwo", 60 }*/
};

/* Requires waitBetweenControl = true. Switchbot Bot/Curtain wait between control commands - overrides defaults */
/* for Bots: if defaultBotWaitTime is greater, defaultBotWaitTime is used */
/* for Curtains: if defaultCurtainWaitTime is greater, defaultBotWaitTime is used */
/* The ESP32 will wait at least X seconds before sending the next command to each device. 2+ different Bots can still be controlled together */
/* The ESP32 will automatically collect hold Secs for bots and will wait at least that long */
static std::map<std::string, int> botWaitBetweenControlTimes = {
  /*{ "switchbotone", 5 },
    { "switchbottwo", 5 },
    { "curtainone", 5 },
    { "curtaintwo", 5 }*/
};

/*************************************************************/

/* ANYTHING CHANGED BELOW THIS COMMENT MAY RESULT IN ISSUES - ALL SETTINGS TO CONFIGURE ARE ABOVE THIS LINE */

static const String versionNum = "v7.1-admin.1";

/*
   Server Index Page
*/
static const char* hostForScan = (meshHost == NULL || strlen(meshHost) < 1) ? host : meshHost;
static const char* hostForControl = host;

// BEGIN GENERATED ADMIN UI
static const uint8_t adminPage[] PROGMEM = {
31,139,8,0,0,0,0,0,2,255,205,125,91,115,27,71,150,230,187,126,69,26,246,24,
64,27,40,94,36,209,18,64,80,75,82,148,237,181,110,43,82,214,68,43,24,61,5,84,
2,40,179,80,133,174,11,33,146,66,68,63,244,63,216,136,125,234,221,126,113,236,79,232,
121,233,167,209,63,233,95,50,223,57,153,89,149,117,1,37,121,60,187,19,209,45,146,85,
153,39,79,158,60,151,239,156,204,74,239,127,225,69,147,244,106,41,197,60,93,4,7,119,
246,233,135,8,220,112,54,106,121,178,117,176,191,144,169,43,38,115,55,78,100,58,106,101,
233,180,255,192,60,13,221,133,28,181,46,125,185,90,70,113,218,18,147,40,76,101,136,86,
43,223,75,231,35,79,94,250,19,217,231,63,122,126,232,167,190,27,244,147,137,27,200,209,
78,11,35,165,126,26,200,131,211,149,159,78,230,71,81,42,254,237,95,197,79,50,94,185,
65,154,133,179,253,45,245,250,206,126,146,94,209,207,65,28,69,233,205,20,67,244,167,238,
194,15,174,6,201,85,146,202,69,63,243,123,137,27,38,253,68,198,254,116,56,137,130,40,
30,124,41,31,72,111,122,119,56,118,39,23,179,56,202,66,111,240,229,206,246,206,183,187,
219,170,1,216,152,203,133,28,120,110,124,177,254,221,205,56,122,215,79,252,107,63,156,13,
198,81,236,201,184,143,39,235,113,228,93,221,44,220,120,230,135,131,237,245,92,186,120,209,
91,184,126,136,135,239,212,180,6,59,219,123,219,203,119,67,221,202,205,210,104,184,116,61,
143,40,237,62,88,190,211,189,110,60,63,89,6,238,213,96,26,200,119,67,55,240,103,97,
223,7,239,201,96,2,121,201,120,248,115,150,164,254,244,170,175,37,56,72,150,46,36,55,
150,233,74,202,208,80,4,83,105,26,45,48,38,17,222,81,178,0,219,114,176,251,109,193,
195,189,229,59,1,110,119,173,215,59,15,139,215,219,98,91,236,50,129,187,118,139,61,60,
89,222,4,126,40,251,115,233,207,230,233,96,199,217,51,194,116,221,241,131,201,131,181,35,
175,228,56,142,86,55,250,241,222,67,111,50,185,63,180,168,236,98,156,64,166,152,81,159,
102,192,82,0,97,103,236,122,51,121,163,68,59,216,1,131,73,20,248,158,248,242,238,221,
123,247,238,239,13,181,204,99,215,243,179,100,64,220,229,66,132,12,5,147,181,70,185,75,
36,103,177,239,229,98,165,63,134,244,79,31,66,197,147,84,66,146,65,182,8,147,193,206,
52,22,248,255,112,230,46,7,59,180,34,137,156,164,126,20,54,112,179,59,185,251,240,222,
195,178,206,124,187,187,179,59,169,48,184,115,207,98,112,247,94,46,219,124,125,104,24,7,
250,33,111,152,39,197,202,96,103,171,191,179,14,220,177,12,114,190,199,65,52,185,168,76,
205,8,125,60,157,76,188,177,89,54,122,131,149,163,101,242,195,101,150,246,18,25,96,34,
189,113,134,33,67,94,201,129,31,206,97,2,233,176,62,179,123,219,247,119,246,170,114,254,
214,154,197,142,82,226,66,171,183,255,169,52,206,77,241,188,106,83,15,119,239,150,141,78,
117,124,75,62,101,4,43,155,92,192,150,206,53,1,54,16,45,173,152,213,140,100,165,39,
49,201,226,4,100,150,145,207,54,97,143,179,123,23,170,114,191,50,142,234,230,44,99,31,
20,175,110,236,246,123,247,161,154,187,166,253,246,195,221,251,59,210,200,101,91,9,124,165,
212,124,239,254,182,33,228,193,233,193,86,117,167,233,116,188,235,62,208,239,6,88,49,119,
28,72,239,38,34,189,78,175,6,14,177,163,24,94,185,126,186,118,92,86,171,164,108,234,
172,117,36,91,250,171,191,138,241,39,253,99,68,144,70,90,41,157,100,225,6,193,77,197,
148,52,39,15,61,119,58,217,93,59,202,159,106,197,85,125,107,166,148,47,232,125,210,151,
225,103,26,200,182,160,126,102,164,193,212,143,147,180,63,153,251,129,103,143,186,189,254,114,
33,147,196,133,73,47,163,196,167,105,15,224,190,38,23,87,67,102,138,230,123,221,247,67,
79,190,27,236,22,28,145,99,162,201,86,212,144,124,83,101,169,199,247,220,225,106,14,7,
201,94,68,14,150,177,100,217,229,227,14,48,137,244,42,23,117,24,133,50,127,231,200,56,
142,226,146,54,220,191,183,59,190,187,187,246,130,207,118,24,187,150,227,92,123,233,77,121,
69,60,47,15,15,195,84,190,75,251,236,217,7,172,216,195,232,82,198,211,32,90,169,85,
119,195,171,21,172,83,174,61,4,78,63,72,110,108,13,32,171,78,178,5,107,113,197,8,
140,7,126,56,158,120,59,107,103,138,24,88,13,39,183,199,14,154,6,123,212,122,212,49,
228,196,178,152,70,197,205,186,101,95,191,254,111,11,233,249,110,167,240,20,123,247,64,187,
123,99,7,199,124,193,31,228,126,122,163,164,115,111,92,238,164,53,125,115,183,170,32,216,
145,230,243,209,254,196,22,241,189,60,22,11,29,141,74,202,179,190,227,168,183,44,197,56,
10,146,143,134,108,214,15,90,57,135,192,82,6,205,51,125,11,105,46,48,188,241,156,15,
154,154,10,237,94,45,86,237,224,66,228,111,23,249,77,174,166,101,15,179,174,205,71,15,
17,200,105,202,110,24,115,222,223,210,216,106,95,53,62,216,247,252,75,254,71,76,2,55,
73,70,45,29,241,91,7,79,95,252,120,248,244,68,124,119,242,234,195,159,207,78,126,58,
121,245,230,240,233,217,235,231,223,237,111,113,151,249,142,133,227,142,176,108,51,185,191,133,
135,250,181,69,177,194,22,160,36,244,53,52,111,121,105,90,194,247,240,43,83,105,29,0,
18,142,225,75,0,9,197,202,143,61,225,102,211,153,28,99,6,224,30,61,15,246,57,164,
26,2,85,1,183,196,52,138,139,199,122,184,131,211,101,236,34,54,25,18,106,21,120,88,
171,101,180,36,213,20,151,110,144,73,6,195,22,44,126,44,179,20,24,114,127,75,53,170,
54,150,161,105,140,223,14,78,194,89,224,39,86,227,45,53,32,126,97,230,141,152,182,244,
50,220,217,39,67,82,98,35,158,180,103,107,9,76,8,196,147,212,77,179,164,37,220,216,
119,251,129,127,137,71,75,4,129,148,120,102,66,192,204,202,172,176,48,187,7,31,254,50,
150,113,226,79,230,144,24,254,220,247,2,38,170,169,224,239,244,224,148,127,71,103,176,228,
121,7,111,72,206,51,25,128,153,80,252,227,79,255,23,47,60,34,29,48,231,138,242,157,
125,8,118,161,40,1,241,65,91,137,214,212,151,129,135,191,249,185,250,163,37,76,236,20,
172,109,88,89,19,135,141,154,111,231,62,182,85,210,21,178,255,86,101,54,111,158,30,62,
23,95,139,231,50,189,94,201,248,66,77,73,169,0,175,244,60,74,210,214,193,115,100,38,
130,140,221,232,162,150,51,67,19,102,142,219,137,88,254,49,243,99,176,6,243,10,100,56,
67,214,210,186,187,219,18,75,151,64,108,56,106,189,61,236,255,222,237,95,111,247,31,246,
207,191,105,149,70,74,18,226,142,248,233,243,112,157,211,211,31,30,119,27,134,226,134,155,
134,42,145,92,249,83,255,37,166,190,130,132,48,9,153,201,68,240,0,250,97,218,64,189,
212,71,48,234,106,45,243,191,201,210,39,17,60,168,76,241,60,148,171,126,241,206,98,100,
239,46,230,28,32,110,204,163,0,98,27,181,158,74,8,143,86,65,134,3,168,66,178,148,
80,32,25,167,96,200,240,34,198,114,142,92,141,20,124,191,198,213,36,144,110,252,6,172,
25,150,12,18,108,29,188,152,78,101,168,39,38,162,121,40,11,138,8,153,43,9,220,16,
154,121,222,217,215,241,18,54,170,2,36,235,170,79,233,155,248,225,101,255,208,139,97,27,
100,199,250,109,157,19,210,115,127,114,232,113,203,58,55,54,185,203,123,134,96,157,19,123,
153,252,101,235,192,30,188,54,38,53,176,59,204,16,191,86,238,85,235,224,59,245,75,67,
151,188,73,73,197,178,113,40,161,206,167,252,243,122,225,38,23,77,195,153,102,118,87,47,
132,61,62,126,126,218,63,149,49,38,211,208,139,91,192,170,141,128,45,211,182,13,238,217,
255,56,59,171,89,217,226,143,105,250,61,91,218,81,28,93,192,159,111,150,69,222,180,209,
2,118,118,31,84,52,15,9,133,179,179,247,192,217,113,118,183,203,83,226,34,195,193,203,
102,51,80,21,8,181,186,97,182,128,203,179,7,244,97,202,59,172,241,208,245,251,247,239,
222,111,213,166,243,58,65,151,131,35,25,102,233,53,76,31,22,189,97,50,220,176,100,60,
247,170,118,22,77,167,245,1,170,134,77,130,189,205,176,75,125,126,173,97,215,229,251,155,
88,246,51,240,86,183,165,210,140,4,96,210,20,130,108,54,33,32,29,127,162,123,28,185,
137,159,240,131,6,41,168,134,141,186,243,112,15,252,45,77,176,224,12,10,42,239,75,237,
244,133,140,103,31,126,9,175,65,103,30,35,144,145,147,14,29,113,136,137,137,239,163,133,
236,31,38,52,174,11,192,123,2,176,29,127,248,5,125,16,83,40,232,141,165,191,16,31,
254,12,7,16,135,34,162,64,242,244,195,223,200,75,208,171,88,250,161,63,75,135,2,89,
192,135,191,199,98,145,37,9,7,155,49,130,60,50,240,25,113,64,86,1,14,98,146,238,
216,141,1,244,252,208,217,223,90,54,217,153,153,2,149,12,90,108,116,57,152,234,127,39,
193,88,42,149,5,86,39,251,83,20,207,93,242,82,82,232,118,180,36,33,123,47,94,64,
170,169,129,43,113,157,137,157,61,211,198,65,76,255,63,42,228,11,119,60,141,1,45,194,
127,252,233,47,152,217,74,206,82,53,93,204,149,8,164,226,2,124,235,142,138,125,3,74,
20,70,79,90,13,24,79,167,194,120,165,144,184,86,20,245,135,66,119,136,252,173,131,111,
52,97,49,247,195,235,12,178,156,145,178,168,102,57,38,106,246,74,135,154,111,64,129,55,
190,4,199,80,111,64,68,234,95,113,85,201,196,13,79,37,224,160,71,14,209,205,104,89,
124,138,63,151,81,16,36,41,52,196,243,137,208,41,218,37,8,226,242,34,35,137,54,6,
114,139,84,197,213,148,60,204,221,237,237,66,99,75,204,192,73,218,236,252,64,153,4,32,
99,80,102,38,254,20,102,202,164,154,216,217,219,214,252,60,64,202,176,153,163,52,246,105,
25,159,185,239,124,104,149,172,201,179,97,100,213,165,105,76,51,228,78,105,188,13,139,120,
202,158,199,90,67,224,212,84,6,129,89,197,170,186,159,188,35,47,47,201,179,40,255,196,
218,169,241,195,135,191,225,85,236,136,35,50,221,31,22,75,246,99,129,244,199,100,15,133,
157,252,62,155,1,151,39,158,75,221,49,36,211,25,138,16,46,185,252,14,11,48,153,7,
190,252,240,87,224,0,82,153,148,21,174,48,130,79,87,118,249,78,133,174,255,126,250,226,
185,80,127,248,240,34,150,174,111,236,234,47,172,174,234,143,74,87,101,38,22,246,224,70,
79,252,64,154,5,154,242,239,238,100,34,151,41,248,93,46,3,127,226,18,211,91,63,39,
81,216,115,232,223,22,108,208,131,128,26,132,14,251,215,226,132,133,6,65,10,103,18,126,
248,101,50,79,82,17,102,49,208,189,76,96,77,79,144,13,100,129,139,5,248,174,136,41,
42,99,163,85,133,118,164,240,145,11,49,139,63,252,29,49,65,28,169,154,91,213,35,170,
217,84,252,162,74,235,201,217,3,216,79,230,76,231,84,143,17,34,171,112,99,242,84,94,
225,249,177,154,2,134,35,150,24,108,154,42,164,73,127,83,184,113,246,199,241,193,41,150,
22,254,94,117,122,73,173,40,191,156,202,121,208,83,60,211,115,210,26,229,206,127,140,194,
169,63,203,98,150,26,26,104,101,69,244,36,117,213,142,81,175,161,230,89,87,37,205,26,
0,162,65,2,64,114,57,219,95,51,151,138,121,123,57,115,3,217,50,201,20,253,10,233,
86,61,160,7,131,67,252,82,76,233,212,174,154,59,169,132,190,102,193,0,15,243,147,144,
147,178,122,20,55,1,28,234,15,40,36,220,139,212,191,212,58,87,143,227,46,113,81,69,
54,204,218,109,208,166,220,235,243,176,141,31,22,216,166,154,195,212,89,59,166,149,139,23,
197,172,204,234,69,65,163,111,43,117,250,28,198,110,177,97,132,82,120,210,98,186,58,13,
174,148,167,106,130,55,86,100,41,71,213,54,17,196,67,207,141,189,197,135,95,62,252,213,
159,169,108,234,48,92,64,111,160,207,14,28,218,66,28,230,235,71,78,172,192,120,100,144,
59,187,255,248,211,255,220,187,43,126,207,35,113,131,153,36,135,73,198,98,9,138,188,106,
129,139,7,130,165,164,109,215,4,255,146,236,73,124,43,55,6,154,121,162,127,235,191,94,
146,95,21,29,103,236,55,198,179,188,75,163,215,162,94,183,201,56,91,6,145,123,139,108,
13,27,98,30,193,244,93,79,222,34,213,231,153,130,8,98,201,40,217,147,34,239,173,188,
88,156,200,128,164,244,18,182,203,229,111,136,151,252,71,158,52,150,28,66,157,89,132,110,
50,123,74,158,114,119,85,119,4,10,254,84,65,208,22,151,134,144,23,251,110,16,205,148,
126,41,117,101,63,96,73,192,212,11,239,61,216,206,139,238,31,221,20,226,82,98,195,14,
88,105,231,69,97,212,35,63,197,122,142,37,193,150,212,159,21,65,187,202,211,153,124,71,
201,233,199,226,166,213,237,216,13,39,50,104,149,69,7,204,55,142,37,233,105,45,110,90,
93,15,89,97,90,27,220,176,33,117,100,115,93,147,56,73,150,188,237,36,246,151,233,193,
29,80,78,40,157,225,34,222,232,166,181,97,199,186,53,16,229,87,101,15,221,234,225,181,
42,59,82,203,167,166,206,136,199,155,138,171,220,238,197,241,225,83,241,248,228,167,31,142,
79,196,179,195,231,135,223,157,60,59,121,126,70,221,54,86,70,169,31,124,88,72,74,3,
190,208,180,168,1,210,187,23,80,84,218,181,167,55,213,90,31,143,9,83,66,71,254,147,
154,148,75,110,212,226,141,223,127,226,115,8,75,225,76,46,168,85,165,228,70,173,140,122,
227,141,161,99,151,202,10,66,161,245,144,72,213,11,95,212,246,185,92,9,213,62,247,186,
36,188,207,201,103,105,114,210,189,4,132,164,13,36,145,70,64,144,114,41,210,185,20,9,
30,123,37,202,159,82,168,34,138,175,19,9,180,40,34,202,188,244,124,148,88,176,44,233,
60,130,127,115,75,116,155,42,88,172,60,92,169,194,67,248,87,85,173,170,52,110,170,79,
229,12,136,196,116,191,188,103,19,40,143,81,33,110,213,149,152,3,254,91,208,223,244,186,
40,31,209,75,252,133,44,150,255,234,209,218,218,117,31,181,218,156,243,90,228,237,160,161,
249,140,141,50,52,212,64,204,26,211,195,146,192,54,212,21,168,253,43,185,136,176,156,205,
93,138,194,2,53,229,54,99,23,178,82,143,104,130,255,149,139,5,196,243,25,214,93,109,
132,8,224,118,44,120,34,252,52,97,123,113,128,91,145,130,71,72,226,137,65,145,51,72,
2,162,252,76,80,192,162,106,77,8,189,206,150,98,69,12,41,44,226,137,40,70,122,70,
162,243,134,172,253,75,67,107,172,152,1,155,100,54,104,4,135,69,64,209,97,125,169,150,
37,202,62,207,148,4,200,55,253,191,173,76,240,250,186,33,218,139,36,91,114,150,232,105,
118,216,243,39,52,28,100,0,131,199,104,154,79,26,237,47,175,144,164,34,24,8,181,29,
242,143,63,253,111,225,69,104,30,98,58,172,89,16,84,194,19,132,133,169,126,44,137,166,
210,5,49,241,13,188,190,25,153,218,109,172,83,176,228,144,192,135,228,104,191,22,38,167,
38,165,252,244,26,5,17,121,146,5,129,160,82,128,240,76,130,210,73,84,85,128,157,233,
39,215,24,202,196,124,221,173,76,108,67,145,64,137,31,175,128,55,173,153,108,206,239,217,
95,0,101,96,73,104,238,48,210,152,197,245,95,34,195,39,230,20,35,176,212,136,85,129,
118,19,129,187,114,23,3,221,81,12,208,234,193,118,216,47,38,200,237,97,130,244,104,18,
99,190,33,29,120,75,134,130,247,139,49,218,21,120,88,217,175,132,59,77,201,30,136,30,
205,190,90,31,40,24,17,244,42,111,98,213,1,216,167,47,74,77,254,115,19,246,98,64,
154,103,20,6,87,2,72,29,115,73,231,176,102,202,83,29,65,241,136,156,202,44,150,228,
3,53,12,142,56,196,170,88,171,119,12,29,5,28,254,195,41,61,49,117,72,178,164,33,
64,184,199,163,104,199,169,209,54,164,141,230,156,230,38,58,76,187,121,127,82,214,223,182,
36,192,114,154,50,31,60,166,152,210,46,75,175,236,108,39,37,34,144,159,54,5,79,113,
212,92,43,96,207,65,146,252,218,76,141,26,111,204,218,89,137,56,215,207,181,23,12,68,
169,202,40,138,80,92,78,218,77,44,230,76,175,20,89,155,18,105,21,137,151,210,77,27,
155,86,19,218,124,6,27,24,250,255,155,214,242,212,35,129,28,0,116,243,125,135,241,21,
116,116,234,102,88,90,113,22,193,166,149,68,115,190,123,185,153,123,232,196,162,40,80,159,
136,166,57,83,116,148,22,185,15,108,140,13,165,60,52,38,223,156,48,179,115,54,89,104,
86,122,99,245,41,178,91,134,91,156,19,139,60,177,166,165,254,143,167,182,156,64,144,217,
103,176,114,42,134,64,121,73,20,102,24,6,189,26,81,47,104,32,77,8,24,253,10,88,
216,81,224,177,154,247,42,13,226,63,180,221,114,187,106,138,73,205,94,2,210,240,208,170,
72,195,65,86,39,134,156,248,168,220,145,225,103,169,223,113,209,65,195,13,131,75,31,43,
160,96,160,233,179,195,99,27,215,226,79,27,212,22,112,198,234,74,24,131,94,254,72,113,
226,34,133,104,147,40,214,131,166,88,109,161,159,48,87,192,50,16,105,82,52,122,22,177,
128,172,54,64,88,143,93,21,44,117,102,73,248,138,194,6,101,45,122,176,83,133,132,226,
2,132,209,211,51,55,209,207,148,199,230,28,201,100,126,79,233,104,90,97,243,52,78,213,
226,105,160,141,249,213,71,18,41,195,251,173,88,189,58,64,5,239,169,150,54,40,227,160,
86,39,84,44,6,50,223,132,35,136,137,105,197,184,143,224,234,227,11,104,162,10,106,97,
45,214,56,22,103,28,194,52,106,156,198,209,130,181,152,66,218,35,113,70,175,176,180,20,
228,145,22,98,69,93,43,222,56,197,60,244,52,196,42,67,110,160,60,15,96,253,28,94,
227,4,193,97,28,250,148,189,129,23,12,238,83,88,133,59,66,214,186,130,139,131,84,64,
48,143,103,90,44,177,22,5,254,205,16,23,4,140,4,62,40,65,63,32,53,0,244,101,
54,166,178,4,65,250,75,221,93,184,9,140,51,115,3,102,235,48,84,12,81,36,67,116,
34,255,80,145,50,69,38,233,85,42,10,229,230,104,109,213,20,160,171,170,15,158,105,93,
18,64,29,70,93,105,86,42,51,154,73,21,174,185,51,69,65,194,12,4,130,242,35,67,
133,51,0,247,227,204,100,180,250,217,68,13,40,243,148,78,132,164,201,229,182,252,220,67,
142,108,55,38,124,144,215,23,204,212,242,46,199,118,211,231,117,146,143,43,212,158,186,217,
244,90,250,169,242,170,169,175,252,196,19,44,44,225,104,173,74,236,162,129,120,196,2,218,
20,95,89,190,162,112,20,137,6,104,49,141,73,78,96,17,121,80,112,90,110,153,2,12,
227,79,149,54,80,116,163,156,197,156,134,184,231,236,48,200,129,244,161,168,149,23,212,65,
81,121,49,157,90,34,133,59,39,174,181,208,138,252,210,17,167,120,44,11,107,192,10,42,
215,75,30,153,227,135,99,175,66,134,158,97,74,46,63,116,47,177,240,156,15,66,19,57,
188,48,160,33,5,83,6,65,127,106,88,226,88,25,131,157,228,217,217,2,5,198,34,33,
211,210,177,20,41,199,12,28,187,235,113,227,81,142,37,140,74,233,8,172,230,83,192,192,
71,42,181,74,165,79,41,231,44,142,62,252,213,81,65,53,160,130,0,226,189,11,251,100,
14,94,135,51,224,102,138,27,73,25,230,37,228,11,92,141,237,66,36,71,190,87,129,112,
166,65,47,71,229,186,180,230,84,12,132,102,83,74,70,216,80,36,96,132,135,156,176,152,
182,133,183,5,137,155,92,0,91,82,121,190,86,82,193,16,51,100,172,205,124,84,193,16,
141,124,189,130,24,124,238,193,165,37,240,54,115,199,156,46,77,185,28,28,160,173,202,131,
24,212,212,177,139,141,115,172,88,175,188,186,115,11,32,77,55,131,174,151,117,52,168,113,
150,215,12,149,42,67,120,210,26,132,74,59,47,198,116,4,155,82,31,9,112,157,86,112,
164,109,16,155,134,55,71,23,129,249,136,6,9,99,234,78,136,26,239,76,36,9,67,31,
83,230,99,200,104,3,156,159,101,122,157,214,212,245,85,77,57,145,99,174,30,105,87,170,
222,129,235,108,154,26,67,5,16,149,233,202,21,59,247,133,73,215,43,166,170,105,82,44,
170,25,38,122,187,99,226,15,221,117,50,239,20,232,138,225,96,14,58,149,133,32,158,174,
62,252,50,15,52,113,141,184,244,41,89,183,128,122,180,71,227,52,3,80,214,179,234,204,
43,136,212,54,213,71,37,50,31,254,142,165,211,233,184,176,170,116,38,49,4,97,167,2,
137,141,117,88,37,51,157,80,105,185,112,140,86,126,207,136,110,32,78,253,244,154,116,1,
255,207,36,217,188,150,102,73,130,3,136,80,89,123,66,75,30,169,53,110,238,250,67,67,
203,124,85,185,128,85,4,217,100,25,103,146,211,204,192,149,180,220,182,118,68,177,9,127,
188,19,206,54,79,169,88,28,205,8,138,58,42,190,20,229,238,74,70,154,115,53,20,167,
68,111,197,109,140,55,50,94,179,234,191,244,60,135,38,215,73,140,211,49,53,189,166,228,
151,195,136,59,158,209,81,113,114,49,152,227,75,51,177,230,196,248,101,115,2,108,178,95,
29,74,76,21,42,206,150,84,206,99,17,56,57,224,40,243,112,1,81,209,202,114,164,51,
219,25,10,133,56,42,226,103,129,199,165,156,124,101,185,24,99,19,113,52,130,183,28,190,
17,111,129,59,166,153,156,25,143,113,92,98,129,120,39,115,179,131,100,141,162,14,62,180,
126,53,2,229,40,196,149,158,102,221,162,87,218,130,117,13,57,84,149,116,93,73,190,84,
26,70,209,100,44,47,161,72,136,188,174,76,101,207,148,154,89,15,207,168,10,14,89,185,
148,215,81,62,111,75,139,21,143,96,125,169,94,161,74,232,138,198,88,34,84,72,85,80,
38,88,167,99,120,207,188,199,176,92,103,215,138,202,12,244,183,120,208,254,27,100,107,240,
38,194,95,136,103,50,153,231,97,39,197,58,226,23,159,178,78,172,38,195,48,76,230,170,
127,164,235,232,148,254,43,48,89,30,53,31,45,97,111,7,8,6,170,58,119,39,45,99,
162,46,23,133,13,77,85,26,207,150,183,172,145,152,82,37,150,52,41,97,45,192,47,175,
162,32,160,237,209,190,65,125,162,115,247,254,246,182,56,186,74,101,210,109,88,84,249,110,
34,165,151,32,64,78,37,157,223,231,222,185,174,168,206,99,221,249,22,27,171,232,119,98,
202,117,77,26,206,35,100,203,91,74,77,60,212,25,172,77,37,88,101,234,215,25,140,119,
114,129,65,56,122,53,14,65,21,79,21,192,210,34,79,107,144,100,149,237,162,206,216,68,
53,175,15,214,153,109,84,133,74,146,66,41,22,92,59,155,105,146,145,39,114,114,48,111,
175,184,78,94,28,161,227,90,138,87,238,204,213,161,219,242,169,78,1,140,41,154,89,142,
155,103,29,32,43,160,81,185,128,201,241,249,225,118,17,159,43,161,102,72,219,68,226,9,
24,38,48,59,243,3,83,64,187,173,174,88,248,105,78,252,152,19,134,123,69,96,96,78,
236,184,176,1,151,43,4,0,6,53,2,24,10,127,90,44,224,231,23,39,149,69,243,10,
40,159,174,43,33,11,36,184,38,218,234,54,215,25,231,222,101,76,123,106,214,218,228,219,
244,173,97,169,56,132,88,162,21,135,119,209,116,244,124,67,0,128,87,60,164,164,17,6,
101,167,42,74,55,93,239,138,28,2,167,204,176,225,32,104,66,139,5,231,77,192,79,163,
235,210,116,242,170,70,67,87,187,93,185,146,58,168,150,35,85,181,186,84,87,29,52,85,
7,75,67,254,74,67,186,13,145,255,10,146,13,245,218,77,1,153,92,122,48,99,128,93,
228,214,108,70,153,196,122,134,209,98,161,199,120,94,141,195,69,218,47,96,198,132,178,105,
13,175,88,243,121,151,190,82,29,47,123,129,225,45,69,250,226,168,131,209,65,90,99,229,
14,134,140,12,232,239,143,248,205,38,252,86,117,68,38,76,216,192,238,184,138,230,140,27,
122,197,118,245,9,35,191,14,199,242,194,229,85,43,195,20,114,120,198,93,188,14,47,144,
77,132,21,145,230,45,64,166,188,65,150,239,180,157,166,88,69,42,24,228,74,122,108,109,
85,153,29,48,243,225,98,69,81,233,11,15,149,158,237,64,145,239,238,138,163,140,54,156,
40,169,236,137,223,251,211,105,190,55,126,68,200,36,73,169,2,162,74,1,181,174,234,6,
1,184,35,207,159,209,110,55,76,121,126,181,4,83,106,40,58,38,82,12,67,161,179,158,
182,150,218,112,120,205,43,249,78,126,20,165,176,208,64,26,80,244,160,110,170,229,35,39,
3,93,241,140,242,182,21,73,48,16,163,57,41,55,161,200,82,246,121,157,241,39,129,78,
94,179,162,79,208,140,227,80,0,134,118,29,180,19,41,200,240,137,1,218,227,160,227,24,
124,124,134,73,114,222,86,58,19,146,159,26,176,168,235,109,124,151,58,171,99,54,24,200,
62,34,82,12,70,31,214,40,210,37,5,105,134,160,234,192,69,164,243,148,66,79,10,114,
140,244,212,26,60,220,51,2,237,169,28,28,70,1,134,129,30,79,39,115,132,8,179,233,
202,68,211,82,191,66,184,189,34,205,150,202,78,73,100,49,108,136,126,79,2,23,80,88,
13,175,128,45,43,54,24,47,38,32,75,203,170,90,105,181,78,242,12,175,178,154,21,90,
170,102,75,234,130,31,135,99,24,124,136,36,186,137,30,146,23,165,40,137,156,193,211,165,
206,230,195,60,20,188,225,71,140,209,169,234,78,253,48,143,158,29,109,219,107,213,6,116,
132,88,242,13,254,129,216,219,198,99,62,237,79,47,202,123,244,3,65,47,119,182,29,115,
242,160,68,196,108,247,87,105,232,221,252,162,115,173,164,71,184,158,0,8,7,103,57,238,
215,1,76,99,173,143,186,144,187,91,201,113,131,167,211,116,195,92,219,173,61,153,13,26,
233,21,155,56,164,25,214,174,77,141,34,239,27,227,253,34,83,226,151,228,146,50,16,44,
76,200,218,18,82,181,46,139,30,158,24,187,202,66,31,40,163,226,156,243,124,43,189,90,
150,92,178,117,32,197,230,105,105,130,60,237,206,115,202,113,20,165,144,253,66,203,25,46,
204,86,92,205,90,126,20,129,207,249,240,70,124,113,244,5,238,208,38,2,225,131,72,69,
181,171,27,53,74,195,235,81,192,222,175,41,244,123,67,28,176,66,84,101,19,171,36,137,
202,110,86,67,230,144,123,61,74,118,251,47,198,63,203,139,180,88,158,227,106,91,237,229,
56,105,142,208,120,146,234,28,231,93,58,149,128,48,236,139,213,46,198,51,237,113,233,198,
6,193,103,236,213,54,198,119,110,120,125,237,206,131,166,182,100,30,51,25,171,134,165,47,
74,21,163,41,12,120,107,10,255,103,121,225,90,51,230,177,104,89,6,120,76,134,216,101,
2,67,68,139,133,56,210,187,109,48,146,153,218,129,171,128,78,67,19,221,134,34,162,29,
92,63,45,29,106,204,15,164,148,128,160,214,187,0,239,164,29,165,178,197,20,205,192,139,
156,35,97,64,208,47,44,220,214,60,234,103,199,173,92,39,22,84,16,128,30,133,213,154,
190,38,82,172,132,77,142,151,160,4,160,75,154,89,22,76,93,35,109,33,112,239,167,209,
5,157,80,66,38,77,71,197,50,1,199,165,79,17,125,239,198,30,87,11,115,64,234,168,
227,174,19,88,25,227,46,64,81,106,30,70,98,110,218,230,96,212,41,54,139,102,132,250,
244,94,209,115,48,51,165,179,210,229,173,3,83,89,108,168,9,218,213,29,213,186,90,220,
209,149,0,230,127,80,5,214,139,229,148,114,31,74,119,19,189,205,88,153,222,76,210,33,
47,74,96,243,98,155,92,13,106,197,182,137,244,57,179,124,110,205,118,229,38,230,140,160,
99,109,176,126,78,10,183,30,222,217,218,18,255,162,207,75,255,11,101,142,126,248,179,194,
243,106,87,53,118,67,196,107,70,174,252,241,17,29,174,224,173,86,93,205,221,50,107,49,
206,124,250,104,213,209,71,176,205,221,12,63,202,43,49,18,109,117,46,111,12,101,49,47,
218,67,221,50,247,132,230,152,117,130,14,111,219,158,108,247,68,91,134,237,243,225,157,59,
88,194,137,41,59,70,193,165,52,77,59,136,141,158,79,224,167,43,110,238,8,202,147,59,
228,164,16,190,242,55,226,139,17,49,144,18,106,111,119,41,74,102,192,183,33,210,148,33,
122,40,30,82,119,134,65,243,46,78,26,61,141,144,90,29,187,137,236,116,135,154,112,157,
81,199,15,39,65,6,136,220,1,129,110,78,27,127,20,164,249,20,235,136,158,57,201,50,
240,211,78,187,223,238,190,221,62,167,22,186,253,109,132,169,123,87,60,82,100,6,154,237,
181,37,17,200,127,74,187,35,222,81,28,173,0,212,114,209,40,137,40,38,242,153,145,108,
15,227,216,189,114,252,132,127,118,66,36,86,51,23,73,77,126,225,73,210,21,95,127,45,
26,158,59,234,243,29,112,211,240,18,204,189,173,63,230,105,82,132,235,84,24,17,246,18,
37,138,87,195,173,94,100,15,188,110,94,239,33,119,160,117,49,205,115,249,155,7,212,100,
93,136,153,148,137,101,135,164,37,215,79,140,177,89,130,195,59,84,111,42,196,168,75,222,
117,182,2,114,75,167,170,80,232,204,100,250,67,42,23,29,203,4,186,133,18,49,137,174,
61,190,122,4,206,32,16,58,188,218,249,3,196,33,182,126,39,52,63,121,5,18,214,169,
110,37,113,3,71,252,110,75,216,106,16,210,110,102,224,95,203,215,139,192,205,210,164,195,
183,155,40,185,234,249,243,19,150,154,19,75,254,206,188,179,245,225,151,173,25,204,204,149,
237,110,245,205,223,248,77,212,240,230,239,252,38,107,120,243,103,126,115,216,240,230,127,241,
155,23,13,111,254,194,111,94,55,188,249,43,191,73,146,118,183,172,242,218,93,61,137,98,
123,150,36,221,23,10,78,204,221,228,197,42,236,232,118,61,53,243,194,62,245,243,183,252,
248,188,48,213,92,134,180,196,218,141,112,27,39,127,35,70,228,76,12,39,109,152,66,165,
65,167,253,252,201,49,220,204,64,189,48,203,110,145,38,119,196,239,200,202,54,48,92,52,
175,115,93,188,179,88,7,74,81,69,140,81,93,19,44,98,134,157,188,57,49,99,241,182,
153,35,211,163,206,143,121,99,123,52,138,184,83,196,57,143,23,78,177,120,73,119,224,140,
3,249,76,221,144,131,80,55,245,223,177,67,210,67,94,200,171,196,12,72,186,224,76,125,
58,50,213,185,160,16,114,128,192,121,229,208,169,250,55,72,45,59,237,129,104,119,185,81,
2,207,217,233,184,61,49,238,82,171,177,113,82,125,225,234,95,187,118,0,73,171,42,147,
27,34,135,9,184,8,241,254,125,105,241,155,227,71,190,186,106,114,158,31,211,38,204,168,
174,155,70,228,186,5,17,203,165,147,83,83,47,169,41,194,49,29,182,166,83,206,140,49,
76,133,46,6,196,32,135,169,191,45,224,157,93,194,98,148,245,12,237,34,179,110,154,55,
155,184,25,240,107,217,11,47,89,246,130,103,216,184,40,198,31,19,227,74,193,213,185,92,
150,189,234,93,87,4,245,252,92,124,99,132,236,36,1,176,134,110,111,22,163,91,241,202,
90,144,80,19,76,253,71,2,195,152,101,20,250,132,243,24,246,211,156,168,106,175,62,89,
12,129,126,69,18,233,131,254,234,140,18,29,166,146,30,149,159,248,158,35,166,152,244,136,
30,164,26,75,14,123,26,247,144,71,237,233,106,4,3,27,73,231,175,232,14,204,140,190,
181,150,165,220,52,217,42,206,144,107,37,142,98,127,230,195,255,50,200,29,241,190,228,27,
233,94,60,115,151,20,45,202,109,94,22,87,121,212,155,22,10,105,24,121,252,226,89,135,
238,77,181,227,246,202,13,46,184,183,23,77,50,46,77,168,249,156,197,82,190,225,119,220,
165,7,92,232,201,39,108,46,206,233,247,47,222,252,225,236,228,159,207,88,210,171,57,29,
154,233,40,66,78,8,182,169,105,167,91,142,184,44,213,145,30,206,209,135,136,168,97,17,
101,169,137,179,116,233,197,73,32,21,47,65,4,152,12,68,163,190,190,235,241,231,140,61,
115,185,95,239,75,181,181,223,251,82,109,57,245,190,172,126,104,8,11,230,132,196,15,51,
61,80,89,128,224,200,150,55,5,86,102,3,120,232,17,179,236,208,63,63,25,75,84,124,
126,177,193,129,25,74,14,12,121,129,249,87,135,46,141,148,232,145,138,110,26,111,148,71,
181,24,204,195,86,101,156,30,140,161,54,180,49,1,203,36,213,199,181,176,72,90,79,231,
143,192,240,87,167,124,134,35,138,15,131,160,211,126,107,93,12,115,222,174,44,95,131,192,
44,229,99,185,49,125,22,28,255,70,207,14,83,240,51,206,82,196,44,139,120,187,91,150,
135,77,40,49,132,106,114,81,68,147,141,68,109,49,152,249,219,33,157,110,58,184,202,225,
20,169,169,154,161,5,147,66,54,57,242,197,72,13,16,120,249,199,64,163,58,81,88,136,
249,197,104,41,145,64,119,67,169,212,22,82,208,205,142,174,126,240,58,237,60,57,233,58,
151,122,129,237,126,37,91,53,68,186,42,238,81,18,167,206,129,118,172,39,218,175,170,71,
27,135,173,27,134,67,174,239,88,221,18,73,96,164,83,109,162,32,209,167,76,36,10,85,
146,72,193,233,146,201,29,176,104,203,34,231,87,78,74,71,26,82,167,136,92,140,127,69,
9,222,38,117,120,219,203,165,212,29,138,26,140,125,81,198,172,200,57,233,54,67,253,193,
110,254,229,110,155,206,168,243,94,67,154,167,134,95,141,124,111,116,176,105,146,62,240,12,
65,249,73,18,79,71,237,118,143,247,155,70,148,33,245,198,89,114,53,226,202,205,80,163,
125,218,58,85,203,163,90,104,39,69,162,164,190,250,207,19,10,158,170,99,175,42,113,52,
51,124,209,226,60,225,27,31,70,111,219,84,188,111,247,218,116,115,28,126,152,43,180,244,
175,116,20,14,191,114,161,28,63,253,37,254,209,215,135,81,31,254,134,19,191,120,97,130,
60,87,251,98,190,163,37,39,79,153,33,53,45,46,144,193,95,165,11,101,248,111,46,253,
82,174,92,152,84,114,209,33,70,187,55,38,229,69,4,122,137,164,222,71,82,171,19,152,
209,193,141,193,47,244,33,245,232,171,178,46,194,23,212,164,192,5,172,175,62,166,179,163,
84,141,173,231,228,69,161,28,169,251,22,15,110,212,88,42,122,192,48,52,43,6,46,173,
45,226,234,251,0,165,195,128,18,23,163,78,151,212,33,148,29,94,162,174,213,84,125,75,
222,212,148,170,120,221,161,30,20,111,153,230,136,24,65,60,139,89,239,31,171,19,84,96,
198,38,190,54,157,146,121,180,122,22,121,110,128,6,107,252,207,46,71,148,140,252,6,252,
104,77,170,137,195,82,56,230,187,104,199,159,191,211,102,209,200,214,194,71,109,70,114,237,
1,212,206,26,81,55,233,36,61,89,232,106,247,198,86,231,100,88,210,102,110,55,172,186,
163,245,29,55,185,10,39,194,242,192,126,103,233,166,243,30,93,165,222,213,106,17,143,92,
186,49,89,76,37,44,90,189,198,80,233,60,242,6,212,12,222,56,199,178,143,218,223,157,
156,129,219,151,47,78,207,218,61,107,155,105,208,166,3,167,125,229,253,219,61,117,111,102,
82,239,127,179,30,220,180,181,188,250,103,0,224,32,86,189,0,7,170,254,207,253,227,211,
87,79,250,103,17,48,114,123,64,182,191,102,150,235,4,243,223,6,124,8,75,33,120,127,
122,213,225,25,174,115,221,116,83,87,79,51,230,18,23,164,227,79,59,95,196,78,116,209,
77,231,64,140,130,5,217,161,134,234,198,226,247,239,219,205,71,245,219,164,206,10,209,163,
113,73,83,162,85,199,27,221,248,222,128,188,141,59,161,31,148,101,12,218,227,8,246,173,
10,236,3,93,39,107,175,205,2,64,83,43,208,79,123,192,78,219,243,47,49,156,12,44,
253,105,43,240,218,166,167,62,50,130,248,251,179,103,79,71,109,251,102,25,235,115,26,125,
161,7,113,218,71,114,53,106,221,114,245,101,211,45,155,127,80,215,108,150,110,154,57,176,
54,127,106,244,49,237,13,55,11,126,91,185,248,238,240,112,112,116,52,56,62,30,60,126,
60,56,57,25,60,121,82,27,167,248,180,199,220,8,91,140,195,31,221,84,175,121,133,152,
91,7,71,81,186,233,26,88,128,94,170,140,183,14,142,213,47,155,218,65,253,233,126,163,
103,244,99,35,45,245,37,81,235,160,244,141,209,70,138,252,73,17,93,116,81,250,218,104,
83,243,101,144,205,90,7,47,241,175,120,230,135,254,71,111,170,181,22,191,178,205,83,151,
156,82,195,154,236,148,86,182,14,204,87,76,155,88,51,87,118,168,239,154,54,181,10,248,
195,166,131,167,234,154,219,141,220,171,31,245,143,159,106,106,181,220,120,57,81,69,135,111,
187,171,104,243,189,139,214,5,139,101,214,106,140,240,117,139,245,75,154,154,191,178,202,137,
125,250,253,100,60,146,106,3,37,151,116,165,103,229,131,172,143,220,82,166,135,81,151,231,
87,232,121,146,196,66,23,32,231,12,54,92,111,3,8,132,164,69,231,44,84,152,65,198,
242,182,173,144,143,75,8,135,6,196,15,165,72,237,243,46,60,81,41,153,65,38,83,8,
172,253,13,126,124,211,110,157,27,188,61,242,222,226,201,249,251,247,130,170,62,240,230,134,
208,35,227,25,225,55,1,76,225,159,61,199,44,222,23,150,211,191,125,188,124,185,173,1,
115,50,195,59,155,250,86,68,116,110,161,12,14,162,4,53,110,192,146,10,34,132,187,218,
191,193,199,110,72,242,36,165,149,244,185,27,5,236,143,243,199,42,209,204,29,112,252,141,
102,15,113,190,189,133,127,183,168,121,187,71,33,233,86,161,249,150,184,16,52,13,0,105,
255,134,223,208,97,73,215,156,52,116,10,40,211,145,142,41,36,40,4,183,94,27,248,173,
190,169,101,132,167,195,36,156,251,232,214,73,112,72,40,86,29,138,69,97,119,104,233,178,
210,100,163,12,133,10,247,218,108,214,159,160,202,90,145,205,119,40,163,47,48,4,150,237,
211,248,50,89,218,72,77,110,168,126,116,24,42,234,237,89,180,82,85,189,142,68,218,93,
202,69,233,129,5,54,232,43,255,206,164,123,83,153,94,145,182,116,191,130,129,25,97,76,
222,94,192,226,218,53,105,216,137,72,247,171,114,123,98,171,180,105,78,56,150,60,30,205,
251,139,137,83,122,87,158,131,46,148,28,211,127,170,2,168,20,83,236,76,244,127,69,32,
121,255,254,237,121,215,1,31,39,176,135,14,48,83,183,190,68,246,37,218,58,203,202,255,
60,175,242,201,243,162,44,193,220,113,109,241,169,242,67,243,146,174,201,173,189,180,100,58,
137,2,90,191,142,65,102,147,209,141,62,168,55,216,233,105,246,7,111,207,123,165,153,15,
54,139,105,61,220,188,58,196,249,168,152,201,173,11,195,109,159,243,147,78,209,197,150,219,
91,174,20,247,120,150,231,44,195,154,16,11,241,156,247,222,150,69,218,179,196,115,126,222,
133,155,251,170,195,79,242,153,128,5,30,225,156,50,100,73,231,42,184,17,63,51,236,228,
77,202,207,237,104,34,3,226,173,164,42,19,173,35,70,230,0,208,235,186,66,220,18,125,
60,18,206,39,218,173,98,136,35,11,71,77,237,35,204,216,203,209,103,134,22,34,117,107,
23,5,23,206,11,149,232,22,193,200,22,229,210,126,190,132,167,52,214,226,44,179,100,222,
241,96,250,58,233,152,212,211,58,58,40,11,157,37,255,95,75,119,138,80,160,142,27,80,
186,79,181,20,78,116,232,55,50,14,235,254,201,146,141,115,35,235,165,246,88,135,120,162,
52,19,134,173,42,50,138,28,143,48,100,207,196,143,217,179,169,107,51,75,78,83,17,70,
174,230,6,67,197,102,98,170,106,245,24,209,190,229,43,232,246,55,245,8,82,205,220,77,
193,78,203,38,25,217,133,34,206,2,19,78,234,85,37,187,150,211,183,55,94,235,214,238,
26,15,153,53,251,60,189,24,233,230,244,14,52,122,158,183,249,61,150,99,232,165,37,142,
116,76,110,15,61,175,202,105,245,238,184,10,131,58,176,120,41,134,52,57,236,112,189,121,
230,137,90,161,71,237,143,127,80,222,30,36,14,121,148,71,237,234,215,227,0,115,77,31,
138,127,84,118,150,107,187,232,93,26,151,70,231,132,219,189,196,33,215,246,168,253,147,53,
72,229,171,113,246,113,52,48,181,86,156,125,172,117,113,49,27,245,241,151,244,204,124,104,
222,238,61,115,211,185,51,13,34,176,149,56,25,127,115,190,181,183,221,253,166,77,55,160,
114,255,202,231,231,186,11,95,219,136,46,115,233,46,183,118,182,119,239,81,151,31,253,35,
238,162,19,92,26,79,155,59,61,173,125,143,78,239,221,229,163,118,211,87,232,152,204,33,
90,192,107,223,252,167,168,91,138,112,83,87,181,203,219,52,107,93,115,80,198,186,45,23,
149,212,252,147,161,54,44,21,114,147,97,165,236,14,131,77,156,16,233,244,68,118,243,66,
153,121,160,222,178,218,118,27,93,15,57,9,203,199,144,245,83,29,57,167,212,254,13,62,
213,111,23,174,136,124,43,73,181,84,177,188,225,208,217,16,2,245,246,233,193,104,103,175,
171,189,125,206,87,195,119,251,102,28,42,59,113,238,64,75,162,191,66,231,33,213,37,199,
42,67,16,27,138,161,224,133,4,240,254,253,23,118,106,243,153,31,253,35,141,209,12,15,
185,42,207,82,46,214,58,230,59,50,54,5,164,94,142,186,54,132,11,166,102,4,161,104,
25,167,255,41,57,133,181,79,176,86,66,82,87,122,85,87,197,64,190,156,157,161,202,5,
197,196,177,145,84,241,212,134,80,195,34,96,27,100,75,187,26,170,105,17,218,77,76,200,
226,96,244,250,213,83,109,128,106,55,19,127,119,168,124,127,20,68,227,206,219,74,57,115,
210,227,29,141,221,238,121,239,70,149,20,107,101,211,117,215,80,119,55,90,184,11,243,114,
157,121,44,167,35,176,128,95,189,104,21,18,124,24,89,167,247,212,186,112,153,180,141,38,
44,34,10,244,50,61,131,215,139,178,180,67,242,34,246,161,76,209,133,197,62,104,118,123,
59,219,219,219,70,29,213,205,104,21,73,231,207,233,174,7,174,141,171,1,170,207,243,116,
169,156,226,234,195,48,163,106,115,250,248,61,161,67,119,100,212,83,163,143,248,99,234,208,
127,64,237,96,103,151,24,179,139,189,237,202,253,19,109,35,192,201,136,197,191,164,255,200,
170,54,139,41,123,64,210,81,80,156,56,58,45,248,98,52,218,129,233,148,79,219,229,170,
208,125,255,190,80,11,109,219,48,237,18,11,31,189,220,2,76,233,84,175,72,203,127,163,
187,44,62,57,37,207,29,121,29,255,89,249,167,239,41,172,94,186,77,28,72,221,190,195,
155,179,55,223,179,193,224,38,252,57,84,238,179,244,170,146,65,23,140,52,33,83,234,94,
190,235,123,99,217,196,92,41,172,88,218,196,81,47,71,232,236,215,237,89,218,249,128,234,
246,245,215,157,252,144,183,90,251,253,157,221,247,239,43,207,14,246,238,22,207,160,77,134,
176,17,151,201,173,202,74,243,235,239,16,33,251,175,184,225,162,28,114,163,89,207,231,185,
102,163,108,156,233,38,78,41,169,201,85,73,145,123,212,254,188,187,71,104,131,233,55,187,
73,228,147,117,28,243,209,151,83,108,42,253,149,34,228,198,27,69,172,88,216,84,144,51,
99,244,110,214,93,43,88,230,166,253,171,110,28,249,156,73,170,155,57,26,231,104,121,86,
115,244,123,147,95,45,128,201,39,221,90,98,160,74,85,138,159,112,79,137,37,79,83,7,
244,174,70,20,40,169,224,138,145,92,152,58,61,51,40,180,224,189,55,101,253,205,103,92,
70,20,22,62,41,237,128,182,183,148,47,105,231,219,160,159,188,223,121,179,113,231,114,189,
97,191,209,236,73,154,224,210,160,17,159,117,9,203,167,105,194,148,142,232,4,87,55,141,
194,41,160,82,249,244,72,113,252,67,213,27,8,16,152,111,221,58,185,141,48,154,46,178,
121,70,3,128,3,197,105,144,253,45,245,31,54,255,119,160,131,192,79,233,124,0,0
};
// END GENERATED ADMIN UI

static EspMQTTClient client(
  mqtt_host,
  mqtt_port,
  (mqtt_user == NULL || strlen(mqtt_user) < 1) ? NULL : mqtt_user,
  (mqtt_user == NULL || strlen(mqtt_user) < 1) ? NULL : mqtt_pass,
  host
);

static const uint16_t mqtt_packet_size = 1300;
static const bool home_assistant_discovery_set_up = false;
static const std::string manufacturer = "WonderLabs SwitchBot";
static const std::string curtainModel = "Curtain";
static const std::string curtainName = "WoCurtain";
static const std::string plugName = "WoPlug";
static const std::string plugModel = "Plug";
static const std::string botModel = "Bot";
static const std::string botName = "WoHand";
static const std::string meterModel = "Meter";
static const std::string meterName = "WoSensorTH";
static const std::string contactModel = "Contact";
static const std::string contactName = "WoContact";
static const std::string motionModel = "Motion";
static const std::string motionName = "WoMotion";

static int ledONValue = HIGH;
static int ledOFFValue = LOW;
static bool isActiveScan = true;
static bool isMeshNode = false;
void scanEndedCB(NimBLEScanResults results);
void rescanEndedCB(NimBLEScanResults results);
void initialScanEndedCB(NimBLEScanResults results);
bool isBotDevice(std::string aDevice);
bool isPlugDevice(std::string aDevice);
bool isMeterDevice(std::string & aDevice);
bool isCurtainDevice(std::string aDevice);
bool isMotionDevice(std::string & aDevice);
bool isContactDevice(std::string & aDevice);
bool processQueue();
void startForeverScan();
void recurringMeterScan();
uint32_t getPassCRC(std::string & aDevice);
bool is_number(const std::string & s);
std::string haEntityName(std::string deviceName, const char * entityName);
std::string haESPName(const char * entityName);
void deviceInfoPolling();
void pushBotButtons();
void publishRecentFailures(bool commandNoResponse, int currentTry);
bool controlMQTT(std::string & device, std::string payload, bool disconnectAfter);
bool sendCommand(NimBLEAdvertisedDevice * advDeviceToUse, const char * type, int attempts, bool disconnectAfter);
bool sendToDevice(NimBLEAdvertisedDevice * advDevice, std::string & aName, const char * command, std::string & deviceTopic, bool disconnectAfter);
bool requestInfo(NimBLEAdvertisedDevice * advDeviceToUse);
bool connectToServer(NimBLEAdvertisedDevice * advDeviceToUse);
void rescanMQTT(std::string & payload);
void requestInfoMQTT(std::string & payload);
void recurringScan();
void recurringRescan();
void notifyCB(NimBLERemoteCharacteristic * pRemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify);
std::string getPass(std::string aDevice);
bool shouldMQTTUpdateForDevice(std::string & anAddr);
bool shouldMQTTUpdateOrActiveScanForDevice(std::string & anAddr);
bool shouldActiveScanForDevice(std::string & anAddr);
void processAdvData(std::string & deviceMac, long anRSSI,  std::string & aValueString, bool useActiveScan);
static std::map<std::string, NimBLEAdvertisedDevice*> allSwitchbotsDev = {};
static std::map<std::string, NimBLEAdvertisedDevice*> allSwitchbotsScanned = {};
static std::map<std::string, unsigned long> rescanTimes = {};
static std::map<std::string, unsigned long> lastUpdateTimes = {};
static std::map<std::string, unsigned long> lastActiveScanTimes = {};
static std::map<std::string, bool> botsSimulatedStates = {};
static std::map<std::string, std::string> motionStates = {};
static std::map<std::string, std::string> contactStates = {};
static std::map<std::string, std::string> plugStates = {};
static std::map<std::string, std::string> botStates = {};
static std::map<std::string, std::string> curtainStates = {};
static std::map<std::string, int> meterHumidStates = {};
static std::map<std::string, float> meterTempCStates = {};
static std::map<std::string, float> meterTempFStates = {};
static std::map<std::string, int> curtainPositionStates = {};
static std::map<std::string, int> curtainLightStates = {};
static std::map<std::string, long> plugPowerStates = {};
static std::map<std::string, bool> plugOverloadStates = {};
static std::map<std::string, std::string> contactMeshStates = {};
static std::map<std::string, std::string> lightMeshStates = {};
static std::map<std::string, std::string> motionMeshStates = {};
static std::map<std::string, unsigned long> lastMotions = {};
static std::map<std::string, unsigned long> lastContacts = {};
static std::map<std::string, unsigned long> lastButton = {};
static std::map<std::string, unsigned long> lastIn = {};
static std::map<std::string, unsigned long> lastOut = {};
static std::map<std::string, std::string> illuminanceStates = {};
static std::map<std::string, std::string> ledStates = {};
static std::map<std::string, int> outCounts = {};
static std::map<std::string, int> entranceCounts = {};
static std::map<std::string, int> buttonCounts = {};
static std::map<std::string, int> meshOpenCounts = {};
static std::map<std::string, int> meshClosedCounts = {};
static std::map<std::string, int> meshDarkCounts = {};
static std::map<std::string, int> meshBrightCounts = {};
static std::map<std::string, int> meshTimeoutCounts = {};
static std::map<std::string, int> meshMotionCounts = {};
static std::map<std::string, int> meshNoMotionCounts = {};

static std::map<std::string, int> updateMeshOpenCount = {};
static std::map<std::string, int> updateMeshClosedCount = {};
static std::map<std::string, int> updateMeshTimeoutCount = {};
static std::map<std::string, int> updateMeshDarkCount = {};
static std::map<std::string, int> updateMeshBrightCount = {};
static std::map<std::string, int> updateMeshMotionCount = {};
static std::map<std::string, int> updateMeshNoMotionCount = {};

static std::map<std::string, unsigned long> updateClosedCount = {};
static std::map<std::string, unsigned long> updateOpenCount = {};
static std::map<std::string, unsigned long> updateTimeoutCount = {};

static std::map<std::string, unsigned long> updateMotionCount = {};
static std::map<std::string, unsigned long> updateNoMotionCount = {};
static std::map<std::string, unsigned long> updateDarkCount = {};
static std::map<std::string, unsigned long> updateBrightCount = {};

static std::map<std::string, int> openCounts = {};
static std::map<std::string, int> closedCounts = {};
static std::map<std::string, int> darkCounts = {};
static std::map<std::string, int> brightCounts = {};
static std::map<std::string, int> timeoutCounts = {};
static std::map<std::string, int> motionCounts = {};
static std::map<std::string, int> noMotionCounts = {};
static std::map<std::string, int> batteryValues = {};
static std::map<std::string, std::string> lastCommandSentStrings = {};
static std::map<std::string, std::string> allSwitchbots;
static std::map<std::string, std::string> allSwitchbotsOpp;
static std::map<std::string, bool> discoveredDevices = {};
static std::map<std::string, bool> botsInPressMode = {};
static std::map<std::string, bool> botsToWaitFor = {};
static std::map<std::string, int> botHoldSecs = {};
static std::map<std::string, const char *> botFirmwares = {};
static std::map<std::string, int> botNumTimers = {};
static std::map<std::string, bool> botInverteds = {};
static std::map<std::string, unsigned long> lastCommandSent = {};
static std::map<std::string, std::string> deviceTypes;
static NimBLEScan* pScan;
static std::string bluetooth_mac_address = "";
static bool isRescanning = false;
static bool processing = false;
static bool espButtonPressed = false;
static bool initialScanComplete = false;
static bool lastCommandWasBusy = false;
static bool deviceHasBooted = false;
static bool gotSettings = false;
static bool lastCommandSentPublished = false;
static bool forceRescan = false;
static bool overrideScan = false;
//static char aBuffer[120];
static std::string ESPMQTTTopic = mqtt_main_topic + "/" + std::string(hostForControl);
static std::string ESPMQTTTopicMesh = mqtt_main_topic + "/" + std::string(hostForScan);
static std::string esp32Topic = ESPMQTTTopic + "/esp32";
static std::string rssiStdStr = esp32Topic + "/rssi";
static std::string lastWillStr = ESPMQTTTopic + "/lastwill";
static std::string lastWillScanStr = ESPMQTTTopicMesh + "/lastwill";
static const char* lastWill = lastWillStr.c_str();
static const char* lastWillScan = lastWillScanStr.c_str();
static std::string botTopic = ESPMQTTTopic + "/bot/";
static std::string plugTopic = ESPMQTTTopic + "/plug/";
static std::string curtainTopic = ESPMQTTTopic + "/curtain/";
static std::string meterTopic = ESPMQTTTopic + "/meter/";
static std::string contactTopic = ESPMQTTTopic + "/contact/";
static std::string contactMainTopic = ESPMQTTTopic + "/contact/";
static std::string motionMainTopic = ESPMQTTTopic + "/motion/";
static std::string meterMainTopic = ESPMQTTTopic + "/meter/";
static std::string motionTopic = ESPMQTTTopic + "/motion/";
static std::string rescanStdStr = ESPMQTTTopic + "/rescan";
static std::string requestInfoStdStr = ESPMQTTTopic + "/requestInfo";
static std::string requestSettingsStdStr = ESPMQTTTopic + "/requestSettings";
static std::string setModeStdStr = ESPMQTTTopic + "/setMode";
static std::string setHoldStdStr = ESPMQTTTopic + "/setHold";
static std::string holdPressStdStr = ESPMQTTTopic + "/holdPress";
//static StaticJsonDocument<120> aJsonDoc;

struct to_lower {
  int operator() ( int ch )
  {
    return std::tolower ( ch );
  }
};

struct QueueCommand {
  std::string payload;
  std::string topic;
  std::string device;
  bool disconnectAfter;
  bool priority;
  int currentTry;
};

struct QueuePublish {
  std::string topic;
  std::string payload;
  char * aBuffer;
  bool retain;
};

struct QueueAdvData {
  std::string macAddr;
  long rssi;
  std::string aValueString;
  std::string deviceType;
  bool useActiveScan;
};


static const int publishQueueSize = 300;
static const int advDataQueueSize = 300;

ArduinoQueue<QueueCommand> commandQueue(queueSize);

ArduinoQueue<QueuePublish> publishQueue(publishQueueSize);

ArduinoQueue<QueueAdvData> advDataQueue(advDataQueueSize);

void addToAdvDevData(std::string aMac, long anRSSI, std::string aString, bool shouldBeActive) {
  bool queueIsFull = advDataQueue.isFull();
  if (!queueIsFull) {
    struct QueueAdvData anAdvData;
    anAdvData.macAddr = aMac;
    anAdvData.rssi = anRSSI;
    anAdvData.aValueString = aString;
    anAdvData.useActiveScan = shouldBeActive;
    advDataQueue.enqueue(anAdvData);
  }
}

static bool adminTrial = false;
static bool adminCleaning = false;
static bool adminCleanupOK = true;
static bool adminInterceptPublish(const std::string &topic) {
  if (!adminCleaning) {
    return adminTrial && topic.size() >= 7 && topic.compare(topic.size()-7,7,"/config")==0;
  }
  if (topic.size() >= 7 && topic.compare(topic.size()-7,7,"/config")==0) {
    if (!client.publish(topic.c_str(),"",true)) adminCleanupOK=false;
  }
  return true;
}

void addToPublish(std::string aTopic, std::string aPayload, bool retain) {
  if (adminInterceptPublish(aTopic)) return;
  bool queueIsFull = publishQueue.isFull();
  if (!queueIsFull) {
    struct QueuePublish aPublish;
    aPublish.payload = aPayload;
    aPublish.topic = aTopic;
    aPublish.retain = retain;
    publishQueue.enqueue(aPublish);
  }
}

void addToPublish(std::string aTopic, const char * aPayload, bool retain) {
  if (adminInterceptPublish(aTopic)) return;
  bool queueIsFull = publishQueue.isFull();
  if (!queueIsFull) {
    struct QueuePublish aPublish;
    aPublish.payload = aPayload;
    aPublish.topic = aTopic;
    aPublish.retain = retain;
    publishQueue.enqueue(aPublish);
  }
}

void addToPublish(std::string aTopic, int aPayload, bool retain) {
  bool queueIsFull = publishQueue.isFull();
  if (!queueIsFull) {
    struct QueuePublish aPublish;
    aPublish.payload = (String(aPayload)).c_str();
    aPublish.topic = aTopic;
    aPublish.retain = retain;
    publishQueue.enqueue(aPublish);
  }
}

void addToPublish(std::string aTopic, long aPayload, bool retain) {
  bool queueIsFull = publishQueue.isFull();
  if (!queueIsFull) {
    struct QueuePublish aPublish;
    aPublish.payload = (String(aPayload)).c_str();
    aPublish.topic = aTopic;
    aPublish.retain = retain;
    publishQueue.enqueue(aPublish);
  }
}

void addToPublish(std::string aTopic, std::string aPayload) {
  bool queueIsFull = publishQueue.isFull();
  if (!queueIsFull) {
    struct QueuePublish aPublish;
    aPublish.payload = aPayload;
    aPublish.topic = aTopic;
    aPublish.retain = false;
    publishQueue.enqueue(aPublish);
  }
}

void addToPublish(std::string aTopic, const char * aPayload) {
  bool queueIsFull = publishQueue.isFull();
  if (!queueIsFull) {
    struct QueuePublish aPublish;
    aPublish.payload = aPayload;
    aPublish.topic = aTopic;
    aPublish.retain = false;
    publishQueue.enqueue(aPublish);
  }
}

void printAString (const char * aString) {
  if (printSerialOutputForDebugging && !manualDebugStartESP32WithMQTT) {
    Serial.println(aString);
  }
}

void printAString (std::string & aString) {
  if (printSerialOutputForDebugging && !manualDebugStartESP32WithMQTT) {
    Serial.println(aString.c_str());
  }
}

void printAString (String & aString) {
  if (printSerialOutputForDebugging && !manualDebugStartESP32WithMQTT) {
    Serial.println(aString);
  }
}

void printAString (int aInt) {
  if (printSerialOutputForDebugging && !manualDebugStartESP32WithMQTT) {
    Serial.println(aInt);
  }
}

void publishStatus(std::string aTopic, const char * status) {
  char aBuffer[80];
  StaticJsonDocument<80> docOut;
  docOut["status"] = status;
  serializeJson(docOut, aBuffer, sizeof(aBuffer));
  addToPublish(aTopic, aBuffer);
}

bool parseMQTTPayload(JsonDocument & docIn, const char * payload, const char * context) {
  DeserializationError error = deserializeJson(docIn, payload);
  if (!error) {
    return true;
  }

  printAString("Parsing failed in ");
  printAString(context);
  printAString(": ");
  printAString(error.c_str());
  publishStatus(ESPMQTTTopic, "errorParsingJSON");
  return false;
}

std::string haEntityName(std::string deviceName, const char * entityName) {
  if (home_assistant_entity_names_include_device_name) {
    return deviceName + " " + std::string(entityName);
  }
  return std::string(entityName);
}

std::string haESPName(const char * entityName) {
  if (home_assistant_entity_names_include_device_name) {
    return std::string(host) + " " + std::string(entityName);
  }
  return std::string(entityName);
}

void publishRecentFailures(bool commandNoResponse, int currentTry) {
  if (!includeSensorRecentFailures) {
    return;
  }

  if (commandNoResponse) {
    addToPublish((esp32Topic + "/recentFailures").c_str(), currentTry, true);
  }
  else {
    addToPublish((esp32Topic + "/recentFailures").c_str(), "0", true);
  }
}

int le16_to_cpu_signed(const uint8_t data[2]) {
  unsigned value = data[0] | ((unsigned)data[1] << 8);
  if (value & 0x8000)
    return -(int)(~value) - 1;
  else
    return value;
}

void publishContactContact(std::string & aDevice, std::string & aValue) {
  addToPublish((contactTopic + aDevice + "/contact"), aValue, true);
}
void publishContactBinContact(std::string & aDevice, std::string & aValue) {
  addToPublish((contactTopic + aDevice + "/bin"), aValue, true);

}
void publishContactState(std::string & aDevice, std::string & aValue) {
  addToPublish((contactTopic + aDevice + "/state"), aValue, true);
}

void publishContactLastMotion(std::string & aDevice, long aValue) {
  addToPublish((contactTopic + aDevice + "/lastmotion"), aValue, true);
}

void publishContactLastContact(std::string & aDevice, long aValue) {
  addToPublish((contactTopic + aDevice + "/lastcontact"), aValue, true);
}

void publishMotionLastMotion(std::string & aDevice, long aValue) {
  addToPublish((motionTopic + aDevice + "/lastmotion"), aValue, true);
}

void processMotionMotion(std::string & aDevice, std::string & deviceMac, std::string & aValueString, bool isActive, bool aPublish) {

  bool aMotion = false;
  long lastMotionHighSeconds = 0;
  bool shouldPublish = aPublish;
  long lastMotion = -1;
  uint8_t byte1 = 0;
  uint8_t byte3 = 0;
  uint8_t byte4 = 0;
  uint8_t byte5 = 0;
  bool fakeLastMotion = false;

  if (isActive) {
    uint8_t byte1 = (uint8_t) aValueString[1];
    aMotion = (byte1 & 0b01000000);
    uint8_t byte3 = (uint8_t) aValueString[3];
    uint8_t byte4 = (uint8_t) aValueString[4];
    uint8_t byte5 = (uint8_t) aValueString[5];
    lastMotionHighSeconds = (byte5 & 0b10000000);

    byte data[] = {byte4, byte3};
    long lastMotionLowSeconds = le16_to_cpu_signed(data);
    lastMotion = lastMotionHighSeconds + lastMotionLowSeconds;
    std::map<std::string, unsigned long>::iterator itU = lastMotions.find(aDevice);
    itU = lastMotions.find(aDevice);
    if (itU == lastMotions.end())
    {
      lastMotions[aDevice] = millis();
    }

    if (!enableMesh && sendBackupMotionContact) {
      long theLastKnownMotion = millis() - lastMotions[aDevice];
      if (theLastKnownMotion < 0) {
        theLastKnownMotion = 0;
      }

      bool missedAMotion = (theLastKnownMotion > ((lastMotion * 1000) + (missedMotionDelay * 1000)));

      if (missedAMotion) {
        shouldPublish = true;
      }

      if ( missedAMotion && (lastMotion < missedDataResend) ) {
        aMotion = true;
      }
    }
  }
  else {
    byte1 = (uint8_t) aValueString[7];
    byte3 = (uint8_t) aValueString[9];
    byte5 = (uint8_t) aValueString[11];
    aMotion = (byte3 & 0b01000000);
    lastMotionHighSeconds = (byte5 & 0b10000000);

    if (aMotion) {
      fakeLastMotion = true;
      lastMotion = 0;
    }

  }

  if (aMotion) {
    lastMotions[aDevice] = millis();
  }

  std::string motion = aMotion ? "MOTION" : "NO MOTION";

  int motionCount = 0;
  int noMotionCount = 0;
  int meshMotionCount = 0;
  int meshNoMotionCount = 0;

  if (meshMotionSensors && enableMesh && countMotionToAvoidDuplicates) {
    std::map<std::string, int>::iterator itQQ = motionCounts.find(deviceMac);
    if (itQQ != motionCounts.end())
    {
      motionCount = itQQ->second;
    }

    itQQ = noMotionCounts.find(deviceMac);
    if (itQQ != noMotionCounts.end())
    {
      noMotionCount = itQQ->second;
    }

    itQQ = meshMotionCounts.find(deviceMac);
    if (itQQ != meshMotionCounts.end())
    {
      meshMotionCount = itQQ->second;
    }

    itQQ = meshNoMotionCounts.find(deviceMac);
    if (itQQ != meshNoMotionCounts.end())
    {
      meshNoMotionCount = itQQ->second;
    }

    if ((meshMotionCount == 0) || (meshNoMotionCount == 0)) {
      if (meshMotionCount == 0) {
        std::string deviceMotionMeshTopic = motionTopic + aDevice + "/motioncount";
        meshMotionCount = 1;
        motionCounts[deviceMac] = meshMotionCount;
        meshMotionCounts[deviceMac] = meshMotionCount;
        addToPublish(deviceMotionMeshTopic.c_str(), meshMotionCount, true);
      }
      if (meshNoMotionCount == 0) {
        std::string deviceNoMotionMeshTopic = motionTopic + aDevice + "/nomotioncount";
        meshNoMotionCount = 1;
        noMotionCounts[deviceMac] = meshNoMotionCount;
        meshNoMotionCounts[deviceMac] = meshNoMotionCount;
        addToPublish(deviceNoMotionMeshTopic.c_str(), meshNoMotionCount, true);
      }
    }
  }
  bool publishLastMotion = true;
  std::map<std::string, std::string>::iterator itH = motionStates.find(deviceMac.c_str());
  if (itH != motionStates.end())
  {
    std::string motionState = itH->second.c_str();
    if (strcmp(motionState.c_str(), motion.c_str()) != 0) {
      shouldPublish = true;
      if (meshMotionSensors && enableMesh && countMotionToAvoidDuplicates) {
        if (aMotion) {

          if ((meshMotionCount == motionCount) && (meshMotionCount != 0) && (meshNoMotionCount == noMotionCount)) {
            meshMotionCount = meshMotionCount + 1;
            if (meshMotionCount > 50) {
              meshMotionCount = 1;
            }
            meshMotionCounts[deviceMac.c_str()] = meshMotionCount;
            motionMeshStates[deviceMac.c_str()] = "MOTION";
            std::map<std::string, unsigned long>::iterator itW = updateMotionCount.find(deviceMac.c_str());
            if (itW != updateMotionCount.end())
            {
              updateMotionCount.erase(deviceMac.c_str());
            }
          }
          else {
            updateMotionCount[deviceMac.c_str()] = millis();
            updateMeshMotionCount[deviceMac.c_str()] = meshMotionCount;
            shouldPublish = false;
          }

          motionCount = motionCount + 1;
          if (motionCount > 50) {
            motionCount = 1;
          }
          motionCounts[deviceMac.c_str()] = motionCount;
        }
        else {
          if ((meshNoMotionCount == noMotionCount) && (meshNoMotionCount != 0) && (meshMotionCount == motionCount)) {
            meshNoMotionCount = meshNoMotionCount + 1;
            if (meshNoMotionCount > 50) {
              meshNoMotionCount = 1;
            }
            meshNoMotionCounts[deviceMac.c_str()] = meshNoMotionCount;
            motionMeshStates[deviceMac.c_str()] = "NO MOTION";
            std::map<std::string, unsigned long>::iterator itW = updateNoMotionCount.find(deviceMac.c_str());
            if (itW != updateNoMotionCount.end())
            {
              updateNoMotionCount.erase(deviceMac.c_str());
            }
          }
          else {
            updateNoMotionCount[deviceMac.c_str()] = millis();
            updateMeshNoMotionCount[deviceMac.c_str()] = meshNoMotionCount;
            shouldPublish = false;
          }

          noMotionCount = noMotionCount + 1;
          if (noMotionCount > 50) {
            noMotionCount = 1;
          }
          noMotionCounts[deviceMac.c_str()] = noMotionCount;
        }
      }
    }
    else {

      if (fakeLastMotion && (lastMotion == 0)) {
        publishLastMotion = false;
      }

      std::map<std::string, unsigned long>::iterator itW = updateMotionCount.find(deviceMac.c_str());
      if (itW != updateMotionCount.end())
      {
        shouldPublish = false;
      }
      itW = updateNoMotionCount.find(deviceMac.c_str());
      if (itW != updateNoMotionCount.end())
      {
        shouldPublish = false;
      }
    }
  }

  motionStates[deviceMac] = motion;

  if (((meshNoMotionCount != noMotionCount) || (meshMotionCount != motionCount)) && enableMesh && meshMotionSensors && countMotionToAvoidDuplicates) {
    shouldPublish = false;
  }

  if (shouldPublish) {
    if (meshMotionSensors && enableMesh && countMotionToAvoidDuplicates) {
      if (aMotion && (meshMotionCount != 0) && (meshMotionCount == motionCount)) {
        std::string deviceMotionMeshTopic = motionTopic + aDevice + "/motioncount";
        addToPublish(deviceMotionMeshTopic.c_str(), meshMotionCount, true);
      }
      else if (!aMotion && (meshNoMotionCount != 0) && (meshNoMotionCount == noMotionCount)) {
        std::string deviceNoMotionMeshTopic = motionTopic + aDevice + "/nomotioncount";
        addToPublish(deviceNoMotionMeshTopic.c_str(), meshNoMotionCount, true);
      }
    }

    if (!isMeshNode || !onlyAllowRootESPToPublishMotion) {
      std::string deviceMotionTopic = motionTopic + aDevice + "/motion";
      addToPublish(deviceMotionTopic.c_str(), motion.c_str(), true);
      std::string deviceStateTopic = motionTopic + aDevice + "/state";
      addToPublish(deviceStateTopic.c_str(), motion.c_str(), true);
      if (lastMotion >= 0 && publishLastMotion) {
        publishMotionLastMotion(aDevice, lastMotion);
      }
    }
  }
}

void processMotionContact(std::string & aDevice, std::string & deviceMac, std::string & aValueString, bool isActive, bool aPublish) {

  bool aMotion = false;
  long lastMotionHighSeconds = 0;
  bool shouldPublish = aPublish;
  long lastMotion = -1;
  uint8_t byte1 = 0;
  uint8_t byte3 = 0;
  uint8_t byte4 = 0;
  uint8_t byte5 = 0;
  bool fakeLastMotion = false;
  if (isActive) {
    byte1 = (uint8_t) aValueString[1];
    byte3 = (uint8_t) aValueString[3];
    byte4 = (uint8_t) aValueString[4];
    byte5 = (uint8_t) aValueString[5];
    aMotion = (byte1 & 0b01000000);
    lastMotionHighSeconds = (byte3 & 0b10000000);
    byte data[] = {byte5, byte4};
    long lastMotionLowSeconds = le16_to_cpu_signed(data);

    lastMotion = lastMotionHighSeconds + lastMotionLowSeconds;
    std::map<std::string, unsigned long>::iterator itU = lastMotions.find(aDevice);
    itU = lastMotions.find(aDevice);
    if (itU == lastMotions.end())
    {
      lastMotions[aDevice] = millis();
    }
    if (!enableMesh && sendBackupMotionContact) {
      long theLastKnownMotion = millis() - lastMotions[aDevice];
      if (theLastKnownMotion < 0) {
        theLastKnownMotion = 0;
      }

      bool missedAMotion = (theLastKnownMotion > ((lastMotion * 1000) + (missedMotionDelay * 1000)));
      if (missedAMotion) {
        shouldPublish = true;
      }

      if ( missedAMotion && (lastMotion < missedDataResend) ) {
        aMotion = true;
      }
    }
  }
  else {
    byte3 = (uint8_t) aValueString[9];
    aMotion = (byte3 & 0b10000000);
    lastMotionHighSeconds = (byte3 & 0b00000010);
    if (aMotion) {
      fakeLastMotion = true;
      lastMotion = 0;
    }
  }

  if (aMotion) {
    lastMotions[aDevice] = millis();
  }

  std::string motion = aMotion ? "MOTION" : "NO MOTION";

  int motionCount = 0;
  int noMotionCount = 0;
  int meshMotionCount = 0;
  int meshNoMotionCount = 0;

  if (meshContactSensors && enableMesh && countMotionToAvoidDuplicates) {
    std::map<std::string, int>::iterator itQQ = motionCounts.find(deviceMac);
    if (itQQ != motionCounts.end())
    {
      motionCount = itQQ->second;
    }

    itQQ = noMotionCounts.find(deviceMac);
    if (itQQ != noMotionCounts.end())
    {
      noMotionCount = itQQ->second;
    }

    itQQ = meshMotionCounts.find(deviceMac);
    if (itQQ != meshMotionCounts.end())
    {
      meshMotionCount = itQQ->second;
    }

    itQQ = meshNoMotionCounts.find(deviceMac);
    if (itQQ != meshNoMotionCounts.end())
    {
      meshNoMotionCount = itQQ->second;
    }

    if ((meshMotionCount == 0) || (meshNoMotionCount == 0)) {
      if (meshMotionCount == 0) {
        std::string deviceMotionMeshTopic = contactTopic + aDevice + "/motioncount";
        meshMotionCount = 1;
        motionCounts[deviceMac] = meshMotionCount;
        meshMotionCounts[deviceMac] = meshMotionCount;
        addToPublish(deviceMotionMeshTopic.c_str(), meshMotionCount, true);
      }
      if (meshNoMotionCount == 0) {
        std::string deviceNoMotionMeshTopic = contactTopic + aDevice + "/nomotioncount";
        meshNoMotionCount = 1;
        noMotionCounts[deviceMac] = meshNoMotionCount;
        meshNoMotionCounts[deviceMac] = meshNoMotionCount;
        addToPublish(deviceNoMotionMeshTopic.c_str(), meshNoMotionCount, true);
      }
    }
  }
  bool publishLastMotion = true;
  std::map<std::string, std::string>::iterator itH = motionStates.find(deviceMac.c_str());
  if (itH != motionStates.end())
  {
    std::string motionState = itH->second.c_str();
    if (strcmp(motionState.c_str(), motion.c_str()) != 0) {
      shouldPublish = true;
      if (meshContactSensors && enableMesh && countMotionToAvoidDuplicates) {
        if (aMotion) {

          if ((meshMotionCount == motionCount) && (meshMotionCount != 0) && (meshNoMotionCount == noMotionCount)) {
            meshMotionCount = meshMotionCount + 1;
            if (meshMotionCount > 50) {
              meshMotionCount = 1;
            }
            meshMotionCounts[deviceMac.c_str()] = meshMotionCount;
            motionMeshStates[deviceMac.c_str()] = "MOTION";
            std::map<std::string, unsigned long>::iterator itW = updateMotionCount.find(deviceMac.c_str());
            if (itW != updateMotionCount.end())
            {
              updateMotionCount.erase(deviceMac.c_str());
            }
          }
          else {
            updateMotionCount[deviceMac.c_str()] = millis();
            updateMeshMotionCount[deviceMac.c_str()] = meshMotionCount;
            shouldPublish = false;
          }

          motionCount = motionCount + 1;
          if (motionCount > 50) {
            motionCount = 1;
          }
          motionCounts[deviceMac.c_str()] = motionCount;
        }
        else {
          if ((meshNoMotionCount == noMotionCount) && (meshNoMotionCount != 0) && (meshMotionCount == motionCount)) {
            meshNoMotionCount = meshNoMotionCount + 1;
            if (meshNoMotionCount > 50) {
              meshNoMotionCount = 1;
            }
            meshNoMotionCounts[deviceMac.c_str()] = meshNoMotionCount;
            motionMeshStates[deviceMac.c_str()] = "NO MOTION";
            std::map<std::string, unsigned long>::iterator itW = updateNoMotionCount.find(deviceMac.c_str());
            if (itW != updateNoMotionCount.end())
            {
              updateNoMotionCount.erase(deviceMac.c_str());
            }
          }
          else {
            updateNoMotionCount[deviceMac.c_str()] = millis();
            updateMeshNoMotionCount[deviceMac.c_str()] = meshNoMotionCount;
            shouldPublish = false;
          }

          noMotionCount = noMotionCount + 1;
          if (noMotionCount > 50) {
            noMotionCount = 1;
          }
          noMotionCounts[deviceMac.c_str()] = noMotionCount;
        }
      }
    }
    else {

      if (fakeLastMotion && (lastMotion == 0)) {
        publishLastMotion = false;
      }

      std::map<std::string, unsigned long>::iterator itW = updateMotionCount.find(deviceMac.c_str());
      if (itW != updateMotionCount.end())
      {
        shouldPublish = false;
      }
      itW = updateNoMotionCount.find(deviceMac.c_str());
      if (itW != updateNoMotionCount.end())
      {
        shouldPublish = false;
      }
    }
  }

  motionStates[deviceMac] = motion;

  if (((meshNoMotionCount != noMotionCount) || (meshMotionCount != motionCount)) && enableMesh && meshContactSensors && countMotionToAvoidDuplicates) {
    shouldPublish = false;
  }

  if (shouldPublish) {
    if (meshContactSensors && enableMesh && countMotionToAvoidDuplicates) {
      if (aMotion && (meshMotionCount != 0) && (meshMotionCount == motionCount)) {
        std::string deviceMotionMeshTopic = contactTopic + aDevice + "/motioncount";
        addToPublish(deviceMotionMeshTopic.c_str(), meshMotionCount, true);
      }
      else if (!aMotion && (meshNoMotionCount != 0) && (meshNoMotionCount == noMotionCount)) {
        std::string deviceNoMotionMeshTopic = contactTopic + aDevice + "/nomotioncount";
        addToPublish(deviceNoMotionMeshTopic.c_str(), meshNoMotionCount, true);
      }
    }

    if (!isMeshNode || !onlyAllowRootESPToPublishMotion) {
      std::string deviceMotionTopic = contactTopic + aDevice + "/motion";
      addToPublish(deviceMotionTopic.c_str(), motion.c_str(), true);
      if (lastMotion >= 0 && publishLastMotion) {
        publishContactLastMotion(aDevice, lastMotion);
      }
    }
  }
}


void processLightMotion(std::string & aDevice, std::string & deviceMac, std::string & aValueString, bool isActive, bool aPublish) {

  bool shouldPublish = aPublish;
  bool lightA = false;
  bool lightB = false;
  if (isActive) {
    uint8_t byte5 = (uint8_t) aValueString[5];
    lightA = (byte5 & 0b00000010);
    lightB = (byte5 & 0b00000001);
  }
  else {
    uint8_t byte3 = (uint8_t) aValueString[9];
    lightA = (byte3 & 0b00100000);
    lightB = (byte3 & 0b00010000);
  }

  std::string light;
  if (!lightA && !lightB) {
    light = "RESERVE";
  }
  else if (!lightA && lightB) {
    light = "DARK";
  }
  else if (lightA && !lightB) {
    light = "BRIGHT";
  }
  else if (lightA && lightB) {
    light = "RESERVE";
  }

  int darkCount = 0;
  int brightCount = 0;
  int meshDarkCount = 0;
  int meshBrightCount = 0;

  if (meshMotionSensors && enableMesh && countLightToAvoidDuplicates) {
    std::map<std::string, int>::iterator itQQ = darkCounts.find(deviceMac);
    if (itQQ != darkCounts.end())
    {
      darkCount = itQQ->second;
    }

    itQQ = brightCounts.find(deviceMac);
    if (itQQ != brightCounts.end())
    {
      brightCount = itQQ->second;
    }

    itQQ = meshDarkCounts.find(deviceMac);
    if (itQQ != meshDarkCounts.end())
    {
      meshDarkCount = itQQ->second;
    }

    itQQ = meshBrightCounts.find(deviceMac);
    if (itQQ != meshBrightCounts.end())
    {
      meshBrightCount = itQQ->second;
    }

    if ((meshDarkCount == 0) || (meshBrightCount == 0)) {
      if (meshDarkCount == 0) {
        std::string deviceDarkMeshTopic = motionTopic + aDevice + "/darkcount";
        meshDarkCount = 1;
        darkCounts[deviceMac] = meshDarkCount;
        meshDarkCounts[deviceMac] = meshDarkCount;
        addToPublish(deviceDarkMeshTopic.c_str(), meshDarkCount, true);
      }
      if (meshBrightCount == 0) {
        std::string deviceBrightMeshTopic = motionTopic + aDevice + "/brightcount";
        meshBrightCount = 1;
        brightCounts[deviceMac] = meshBrightCount;
        meshBrightCounts[deviceMac] = meshBrightCount;
        addToPublish(deviceBrightMeshTopic.c_str(), meshBrightCount, true);
      }
    }
  }

  std::map<std::string, std::string>::iterator itH = illuminanceStates.find(deviceMac.c_str());
  if (itH != illuminanceStates.end())
  {
    std::string illuminanceState = itH->second.c_str();
    if (strcmp(illuminanceState.c_str(), light.c_str()) != 0) {
      shouldPublish = true;
      if (meshMotionSensors && enableMesh && countLightToAvoidDuplicates) {
        if (strcmp(light.c_str(), "DARK") == 0) {

          if ((meshDarkCount == darkCount) && (meshDarkCount != 0) && (meshBrightCount == brightCount)) {
            meshDarkCount = meshDarkCount + 1;
            if (meshDarkCount > 50) {
              meshDarkCount = 1;
            }
            meshDarkCounts[deviceMac.c_str()] = meshDarkCount;
            lightMeshStates[deviceMac.c_str()] = "DARK";
            std::map<std::string, unsigned long>::iterator itW = updateDarkCount.find(deviceMac.c_str());
            if (itW != updateDarkCount.end())
            {
              updateDarkCount.erase(deviceMac.c_str());
            }
          }
          else {
            updateDarkCount[deviceMac.c_str()] = millis();
            updateMeshDarkCount[deviceMac.c_str()] = meshDarkCount;
            shouldPublish = false;
          }

          darkCount = darkCount + 1;
          if (darkCount > 50) {
            darkCount = 1;
          }
          darkCounts[deviceMac.c_str()] = darkCount;
        }
        else if (strcmp(light.c_str(), "BRIGHT") == 0) {
          if ((meshBrightCount == brightCount) && (meshBrightCount != 0) && (meshDarkCount == darkCount)) {
            meshBrightCount = meshBrightCount + 1;
            if (meshBrightCount > 50) {
              meshBrightCount = 1;
            }
            meshBrightCounts[deviceMac.c_str()] = meshBrightCount;
            lightMeshStates[deviceMac.c_str()] = "BRIGHT";
            std::map<std::string, unsigned long>::iterator itW = updateBrightCount.find(deviceMac.c_str());
            if (itW != updateBrightCount.end())
            {
              updateBrightCount.erase(deviceMac.c_str());
            }
          }
          else {
            updateBrightCount[deviceMac.c_str()] = millis();
            updateMeshBrightCount[deviceMac.c_str()] = meshBrightCount;
            shouldPublish = false;
          }

          brightCount = brightCount + 1;
          if (brightCount > 50) {
            brightCount = 1;
          }
          brightCounts[deviceMac.c_str()] = brightCount;
        }
      }
    }
    else {
      std::map<std::string, unsigned long>::iterator itW = updateDarkCount.find(deviceMac.c_str());
      if (itW != updateDarkCount.end())
      {
        shouldPublish = false;
      }
      itW = updateBrightCount.find(deviceMac.c_str());
      if (itW != updateBrightCount.end())
      {
        shouldPublish = false;
      }
    }
  }

  illuminanceStates[deviceMac] = light;

  if (((meshBrightCount != brightCount) || (meshDarkCount != darkCount)) && enableMesh && meshMotionSensors && countLightToAvoidDuplicates) {
    shouldPublish = false;
  }

  if (shouldPublish) {
    if (meshMotionSensors && enableMesh && countLightToAvoidDuplicates) {
      if ((strcmp(light.c_str(), "DARK") == 0) && (meshDarkCount != 0) && (meshDarkCount == darkCount)) {
        std::string deviceDarkMeshTopic = motionTopic + aDevice + "/darkcount";
        addToPublish(deviceDarkMeshTopic.c_str(), meshDarkCount, true);
      }
      else if ((strcmp(light.c_str(), "BRIGHT") == 0) && (meshBrightCount != 0) && (meshBrightCount == brightCount)) {
        std::string deviceBrightMeshTopic = motionTopic + aDevice + "/brightcount";
        addToPublish(deviceBrightMeshTopic.c_str(), meshBrightCount, true);
      }
    }

    if (!isMeshNode || !onlyAllowRootESPToPublishLight) {
      std::string deviceLightTopic = motionTopic + aDevice + "/illuminance";
      addToPublish(deviceLightTopic.c_str(), light.c_str(), true);
    }
  }
}


void processLightContact(std::string & aDevice, std::string & deviceMac, std::string & aValueString, bool isActive, bool aPublish) {
  bool shouldPublish = aPublish;
  std::string light = "";
  int battLevel = 0;
  uint8_t byte3 = 0;
  if (isActive) {
    byte3 = (uint8_t) aValueString[3];
    light = (byte3 & 0b00000001) ? "BRIGHT" : "DARK";
  }
  else
  {
    byte3 = (uint8_t) aValueString[9];
    light = (byte3 & 0b01000000) ? "BRIGHT" : "DARK";

  }

  int darkCount = 0;
  int brightCount = 0;
  int meshDarkCount = 0;
  int meshBrightCount = 0;

  if (meshContactSensors && enableMesh && countLightToAvoidDuplicates) {
    std::map<std::string, int>::iterator itQQ = darkCounts.find(deviceMac);
    if (itQQ != darkCounts.end())
    {
      darkCount = itQQ->second;
    }

    itQQ = brightCounts.find(deviceMac);
    if (itQQ != brightCounts.end())
    {
      brightCount = itQQ->second;
    }

    itQQ = meshDarkCounts.find(deviceMac);
    if (itQQ != meshDarkCounts.end())
    {
      meshDarkCount = itQQ->second;
    }

    itQQ = meshBrightCounts.find(deviceMac);
    if (itQQ != meshBrightCounts.end())
    {
      meshBrightCount = itQQ->second;
    }

    if ((meshDarkCount == 0) || (meshBrightCount == 0)) {
      if (meshDarkCount == 0) {
        std::string deviceDarkMeshTopic = contactTopic + aDevice + "/darkcount";
        meshDarkCount = 1;
        darkCounts[deviceMac] = meshDarkCount;
        meshDarkCounts[deviceMac] = meshDarkCount;
        addToPublish(deviceDarkMeshTopic.c_str(), meshDarkCount, true);
      }
      if (meshBrightCount == 0) {
        std::string deviceBrightMeshTopic = contactTopic + aDevice + "/brightcount";
        meshBrightCount = 1;
        brightCounts[deviceMac] = meshBrightCount;
        meshBrightCounts[deviceMac] = meshBrightCount;
        addToPublish(deviceBrightMeshTopic.c_str(), meshBrightCount, true);
      }
    }
  }

  std::map<std::string, std::string>::iterator itH = illuminanceStates.find(deviceMac.c_str());
  if (itH != illuminanceStates.end())
  {
    std::string illuminanceState = itH->second.c_str();
    if (strcmp(illuminanceState.c_str(), light.c_str()) != 0) {
      shouldPublish = true;
      if (meshContactSensors && enableMesh && countLightToAvoidDuplicates) {
        if (strcmp(light.c_str(), "DARK") == 0) {

          if ((meshDarkCount == darkCount) && (meshDarkCount != 0) && (meshBrightCount == brightCount)) {
            meshDarkCount = meshDarkCount + 1;
            if (meshDarkCount > 50) {
              meshDarkCount = 1;
            }
            meshDarkCounts[deviceMac.c_str()] = meshDarkCount;
            lightMeshStates[deviceMac.c_str()] = "DARK";
            std::map<std::string, unsigned long>::iterator itW = updateDarkCount.find(deviceMac.c_str());
            if (itW != updateDarkCount.end())
            {
              updateDarkCount.erase(deviceMac.c_str());
            }
          }
          else {
            updateDarkCount[deviceMac.c_str()] = millis();
            updateMeshDarkCount[deviceMac.c_str()] = meshDarkCount;
            shouldPublish = false;
          }

          darkCount = darkCount + 1;
          if (darkCount > 50) {
            darkCount = 1;
          }
          darkCounts[deviceMac.c_str()] = darkCount;
        }
        else if (strcmp(light.c_str(), "BRIGHT") == 0) {
          if ((meshBrightCount == brightCount) && (meshBrightCount != 0) && (meshDarkCount == darkCount)) {
            meshBrightCount = meshBrightCount + 1;
            if (meshBrightCount > 50) {
              meshBrightCount = 1;
            }
            meshBrightCounts[deviceMac.c_str()] = meshBrightCount;
            lightMeshStates[deviceMac.c_str()] = "BRIGHT";
            std::map<std::string, unsigned long>::iterator itW = updateBrightCount.find(deviceMac.c_str());
            if (itW != updateBrightCount.end())
            {
              updateBrightCount.erase(deviceMac.c_str());
            }
          }
          else {
            updateBrightCount[deviceMac.c_str()] = millis();
            updateMeshBrightCount[deviceMac.c_str()] = meshBrightCount;
            shouldPublish = false;
          }

          brightCount = brightCount + 1;
          if (brightCount > 50) {
            brightCount = 1;
          }
          brightCounts[deviceMac.c_str()] = brightCount;
        }
      }
    }
    else {
      std::map<std::string, unsigned long>::iterator itW = updateDarkCount.find(deviceMac.c_str());
      if (itW != updateDarkCount.end())
      {
        shouldPublish = false;
      }
      itW = updateBrightCount.find(deviceMac.c_str());
      if (itW != updateBrightCount.end())
      {
        shouldPublish = false;
      }
    }
  }

  illuminanceStates[deviceMac] = light;

  if (((meshBrightCount != brightCount) || (meshDarkCount != darkCount)) && enableMesh && meshContactSensors && countLightToAvoidDuplicates) {
    shouldPublish = false;
  }

  if (shouldPublish) {
    if (meshContactSensors && enableMesh && countLightToAvoidDuplicates) {
      if ((strcmp(light.c_str(), "DARK") == 0) && (meshDarkCount != 0) && (meshDarkCount == darkCount)) {
        std::string deviceDarkMeshTopic = contactTopic + aDevice + "/darkcount";
        addToPublish(deviceDarkMeshTopic.c_str(), meshDarkCount, true);
      }
      else if ((strcmp(light.c_str(), "BRIGHT") == 0) && (meshBrightCount != 0) && (meshBrightCount == brightCount)) {
        std::string deviceBrightMeshTopic = contactTopic + aDevice + "/brightcount";
        addToPublish(deviceBrightMeshTopic.c_str(), meshBrightCount, true);
      }
    }

    if (!isMeshNode || !onlyAllowRootESPToPublishLight) {
      std::string deviceLightTopic = contactTopic + aDevice + "/illuminance";
      addToPublish(deviceLightTopic.c_str(), light.c_str(), true);
    }
  }
}

void processSenseDistance(std::string & aDevice, std::string & deviceMac, std::string & aValueString, bool isActive, bool aPublish) {

  if (!isActive) {
    return;
  }
  bool shouldPublish = aPublish;
  uint8_t byte5 = (uint8_t) aValueString[5];
  bool sensingDistanceA = (byte5 & 0b00001000);
  bool sensingDistanceB = (byte5 & 0b00000100);
  std::string sensingDistance;
  if (!sensingDistanceA && !sensingDistanceB) {
    sensingDistance = "LONG";
  }
  else if (!sensingDistanceA && sensingDistanceB) {
    sensingDistance = "MIDDLE";
  }
  else if (sensingDistanceA && !sensingDistanceB) {
    sensingDistance = "SHORT";
  }
  else if (sensingDistanceA && sensingDistanceB) {
    sensingDistance = "RESERVE";
  }
  //aJsonDoc["sensedist"] = sensingDistance;
  if (shouldPublish) {
    std::string deviceSenseTopic = motionTopic + aDevice + "/sensedist";
    addToPublish(deviceSenseTopic.c_str(), sensingDistance.c_str(), true);
  }
}


void processLED(std::string & aDevice, std::string & deviceMac, std::string & aValueString, bool isActive, bool aPublish) {
  if (!isActive) {
    return;
  }
  bool shouldPublish = aPublish;
  uint8_t byte5 = (uint8_t) aValueString[5];
  std::string ledState = (byte5 & 0b00100000) ? "ON" : "OFF";
  //aJsonDoc["led"] = ledState;

  std::map<std::string, std::string>::iterator itL = ledStates.find(deviceMac);
  if (itL != ledStates.end())
  {
    std::string aLED = itL->second.c_str();
    if (strcmp(aLED.c_str(), ledState.c_str()) != 0) {
      shouldPublish = true;
    }
  }
  ledStates[deviceMac] = ledState;

  if (shouldPublish) {
    std::string deviceLEDTopic = motionTopic + aDevice + "/led";
    addToPublish(deviceLEDTopic.c_str(), ledState.c_str(), true);
  }
}

void processContact(std::string & aDevice, std::string & deviceMac, std::string & aValueString, bool isActive, bool aPublish) {
  bool contactA = false;
  bool contactB = false;
  long lastContactHighSeconds = 0;
  bool shouldPublish = aPublish;
  uint8_t byte3 = 0;
  uint8_t byte6 = 0;
  uint8_t byte7 = 0;
  std::string light = "";
  int battLevel = 0;

  if ( isActive) {
    byte3 = (uint8_t) aValueString[3];
    byte6 = (uint8_t) aValueString[6];
    byte7 = (uint8_t) aValueString[7];
    contactA = (byte3 & 0b00000100);
    contactB = (byte3 & 0b00000010);
    lastContactHighSeconds = (byte3 & 0b01000000);
  }
  else
  {
    byte3 = (uint8_t) aValueString[9];
    byte6 = (uint8_t) aValueString[12];
    byte7 = (uint8_t) aValueString[13];
    contactA = (byte3 & 0b00100000);
    contactB = (byte3 & 0b00010000);
    lastContactHighSeconds = (byte3 & 0b00000001);
  }

  byte data2[] = {byte7, byte6};
  long lastContactLowSeconds = le16_to_cpu_signed(data2);

  long lastContact = lastContactHighSeconds + lastContactLowSeconds;

  std::map<std::string, unsigned long>::iterator itU = lastContacts.find(aDevice.c_str());
  if (itU == lastContacts.end())
  {
    lastContacts[aDevice.c_str()] = millis();
  }

  std::string contact;
  std::string binContact;
  if (!contactA && !contactB) {
    contact = "CLOSED";
  }
  else if (!contactA && contactB) {
    contact = "OPEN";
    lastContacts[aDevice] = millis();
  }
  else if (contactA && !contactB) {
    contact = "TIMEOUT";
    lastContacts[aDevice] = millis();
  }
  else if (contactA && contactB) {
    contact = "RESERVE";
  }

  if (!enableMesh && sendBackupMotionContact) {
    long theLastKnownContact = millis() - lastContacts[aDevice];
    if (theLastKnownContact < 0) {
      theLastKnownContact = 0;
    }

    bool missedAContact = (theLastKnownContact > ((lastContact * 1000) + (missedContactDelay * 1000)));
    if (missedAContact) {
      shouldPublish = true;
    }

    if ( missedAContact && (lastContact < missedDataResend) ) {
      contact = "OPEN";
      lastContacts[aDevice.c_str()] = millis();
    }
  }

  if (strcmp(contact.c_str(), "CLOSED") == 0) {
    binContact = "CLOSED";
  }
  else {
    binContact = "OPEN";
  }

  int openCount = 0;
  int closedCount = 0;
  int timeoutCount = 0;
  int meshOpenCount = 0;
  int meshClosedCount = 0;
  int meshTimeoutCount = 0;

  if (meshContactSensors && enableMesh && countContactToAvoidDuplicates) {
    std::map<std::string, int>::iterator itQQ = openCounts.find(deviceMac.c_str());
    if (itQQ != openCounts.end())
    {
      openCount = itQQ->second;
    }

    itQQ = closedCounts.find(deviceMac.c_str());
    if (itQQ != closedCounts.end())
    {
      closedCount = itQQ->second;
    }

    itQQ = timeoutCounts.find(deviceMac.c_str());
    if (itQQ != timeoutCounts.end())
    {
      timeoutCount = itQQ->second;
    }

    itQQ = meshOpenCounts.find(deviceMac.c_str());
    if (itQQ != meshOpenCounts.end())
    {
      meshOpenCount = itQQ->second;
    }

    itQQ = meshClosedCounts.find(deviceMac.c_str());
    if (itQQ != meshClosedCounts.end())
    {
      meshClosedCount = itQQ->second;
    }

    itQQ = meshTimeoutCounts.find(deviceMac.c_str());
    if (itQQ != meshTimeoutCounts.end())
    {
      meshTimeoutCount = itQQ->second;
    }

    if ((meshOpenCount == 0) || (meshClosedCount == 0) || (meshTimeoutCount == 0)) {
      if (meshOpenCount == 0) {
        std::string deviceOpenMeshTopic = contactTopic + aDevice + "/opencount";
        meshOpenCount = 1;
        openCounts[deviceMac.c_str()] = meshOpenCount;
        meshOpenCounts[deviceMac.c_str()] = meshOpenCount;
        addToPublish(deviceOpenMeshTopic.c_str(), meshOpenCount, true);
      }
      if (meshClosedCount == 0) {
        std::string deviceClosedMeshTopic = contactTopic + aDevice + "/closedcount";
        meshClosedCount = 1;
        closedCounts[deviceMac.c_str()] = meshClosedCount;
        meshClosedCounts[deviceMac.c_str()] = meshClosedCount;
        addToPublish(deviceClosedMeshTopic.c_str(), meshClosedCount, true);
      }
      if (meshTimeoutCount == 0) {
        std::string deviceTimeoutMeshTopic = contactTopic + aDevice + "/timeoutcount";
        meshTimeoutCount = 1;
        timeoutCounts[deviceMac.c_str()] = meshTimeoutCount;
        meshTimeoutCounts[deviceMac.c_str()] = meshTimeoutCount;
        addToPublish(deviceTimeoutMeshTopic.c_str(), meshTimeoutCount, true);
      }
    }
  }

  std::map<std::string, std::string>::iterator itH = contactStates.find(deviceMac.c_str());
  if (itH != contactStates.end())
  {
    std::string contactState = itH->second.c_str();
    if (strcmp(contactState.c_str(), contact.c_str()) != 0) {
      shouldPublish = true;
      if (meshContactSensors && enableMesh && countContactToAvoidDuplicates) {
        if (strcmp(contact.c_str(), "OPEN") == 0) {

          if ((meshOpenCount == openCount) && (meshOpenCount != 0) && (meshTimeoutCount == timeoutCount) && (meshClosedCount == closedCount)) {
            meshOpenCount = meshOpenCount + 1;
            if (meshOpenCount > 50) {
              meshOpenCount = 1;
            }
            meshOpenCounts[deviceMac.c_str()] = meshOpenCount;
            contactMeshStates[deviceMac.c_str()] = "OPEN";
            std::map<std::string, unsigned long>::iterator itW = updateOpenCount.find(deviceMac.c_str());
            if (itW != updateOpenCount.end())
            {
              updateOpenCount.erase(deviceMac.c_str());
            }
          }
          else {
            updateOpenCount[deviceMac.c_str()] = millis();
            updateMeshOpenCount[deviceMac.c_str()] = meshOpenCount;
            shouldPublish = false;
          }

          openCount = openCount + 1;
          if (openCount > 50) {
            openCount = 1;
          }
          openCounts[deviceMac.c_str()] = openCount;
        }
        else if (strcmp(contact.c_str(), "CLOSED") == 0) {
          if ((meshClosedCount == closedCount) && (meshClosedCount != 0) && (meshTimeoutCount == timeoutCount) && (meshOpenCount == openCount)) {
            meshClosedCount = meshClosedCount + 1;
            if (meshClosedCount > 50) {
              meshClosedCount = 1;
            }
            meshClosedCounts[deviceMac.c_str()] = meshClosedCount;
            contactMeshStates[deviceMac.c_str()] = "CLOSED";
            std::map<std::string, unsigned long>::iterator itW = updateClosedCount.find(deviceMac.c_str());
            if (itW != updateClosedCount.end())
            {
              updateClosedCount.erase(deviceMac.c_str());
            }
          }
          else {
            updateClosedCount[deviceMac.c_str()] = millis();
            updateMeshClosedCount[deviceMac.c_str()] = meshClosedCount;
            shouldPublish = false;
          }

          closedCount = closedCount + 1;
          if (closedCount > 50) {
            closedCount = 1;
          }
          closedCounts[deviceMac.c_str()] = closedCount;
        }
        else if (strcmp(contact.c_str(), "TIMEOUT") == 0) {

          if ((meshTimeoutCount == timeoutCount) && (meshTimeoutCount != 0) && (meshOpenCount == openCount) && (meshClosedCount == closedCount)) {
            meshTimeoutCount = meshTimeoutCount + 1;
            if (meshTimeoutCount > 50) {
              meshTimeoutCount = 1;
            }
            meshTimeoutCounts[deviceMac.c_str()] = meshTimeoutCount;
            contactMeshStates[deviceMac.c_str()] = "TIMEOUT";
            std::map<std::string, unsigned long>::iterator itW = updateTimeoutCount.find(deviceMac.c_str());
            if (itW != updateTimeoutCount.end())
            {
              updateTimeoutCount.erase(deviceMac.c_str());
            }
          }
          else {
            updateTimeoutCount[deviceMac.c_str()] = millis();
            updateMeshTimeoutCount[deviceMac.c_str()] = meshTimeoutCount;
            shouldPublish = false;
          }

          timeoutCount = timeoutCount + 1;

          if (timeoutCount > 50) {
            timeoutCount = 1;
          }

          timeoutCounts[deviceMac.c_str()] = timeoutCount;
        }
      }
    }
    else {

      std::map<std::string, unsigned long>::iterator itW = updateOpenCount.find(deviceMac.c_str());
      if (itW != updateOpenCount.end())
      {
        shouldPublish = false;
      }
      itW = updateClosedCount.find(deviceMac.c_str());
      if (itW != updateClosedCount.end())
      {
        shouldPublish = false;
      }
      itW = updateTimeoutCount.find(deviceMac.c_str());
      if (itW != updateTimeoutCount.end())
      {
        shouldPublish = false;
      }
    }
  }

  contactStates[deviceMac.c_str()] = contact;

  if (((meshClosedCount != closedCount) || (meshTimeoutCount != timeoutCount) || (meshOpenCount != openCount)) && enableMesh && meshContactSensors && countContactToAvoidDuplicates) {
    shouldPublish = false;
  }

  if (shouldPublish) {
    if (meshContactSensors && enableMesh && countContactToAvoidDuplicates) {
      if ((strcmp(contact.c_str(), "OPEN") == 0) && (meshOpenCount != 0) && (meshOpenCount == openCount)) {
        std::string deviceOpenMeshTopic = contactTopic + aDevice + "/opencount";
        addToPublish(deviceOpenMeshTopic.c_str(), meshOpenCount, true);
      }
      else if ((strcmp(contact.c_str(), "CLOSED") == 0) && (meshClosedCount != 0) && (meshClosedCount == closedCount)) {
        std::string deviceClosedMeshTopic = contactTopic + aDevice + "/closedcount";
        addToPublish(deviceClosedMeshTopic.c_str(), meshClosedCount, true);
      }
      else if ((strcmp(contact.c_str(), "TIMEOUT") == 0) && (meshTimeoutCount != 0) && (meshTimeoutCount == timeoutCount)) {
        std::string deviceTimeoutMeshTopic = contactTopic + aDevice + "/timeoutcount";
        addToPublish(deviceTimeoutMeshTopic.c_str(), meshTimeoutCount, true);
      }
    }

    if (!isMeshNode || !onlyAllowRootESPToPublishContact) {
      std::string deviceContactTopic = contactTopic + aDevice + "/contact";
      std::string deviceBinContactTopic = contactTopic + aDevice + "/bin";
      std::string deviceStateTopic = contactTopic + aDevice + "/state";
      addToPublish(deviceBinContactTopic.c_str(), binContact.c_str(), true);
      addToPublish(deviceContactTopic.c_str(), contact.c_str(), true);
      addToPublish(deviceStateTopic.c_str(), contact, true);
      if (lastContact >= 0) {
        publishContactLastContact(aDevice, lastContact);
      }
    }
  }
}

void processButton(std::string & aDevice, std::string & deviceMac, std::string & aValueString, bool isActive, bool aPublish) {
  uint8_t byte8 = 0;
  bool shouldPublish = aPublish;
  bool valueChanged = false;
  if (isActive) {
    byte8 = (uint8_t) aValueString[8];
  }
  else {
    byte8 = (uint8_t) aValueString[14];
  }

  int buttonCountA = (byte8 & 0b00001000) ? 8 : 0;
  int buttonCountB = (byte8 & 0b00000100) ? 4 : 0;
  int buttonCountC = (byte8 & 0b00000010) ? 2 : 0;
  int buttonCountD = (byte8 & 0b00000001) ? 1 : 0;
  int buttonCount = buttonCountA + buttonCountB + buttonCountC + buttonCountD;

  std::string deviceButtonTopic = contactTopic + aDevice + "/button";

  std::map<std::string, int>::iterator itE = buttonCounts.find(deviceMac);
  if (itE != buttonCounts.end())
  {
    int bCount = itE->second;
    if (((bCount < buttonCount) ||  ((bCount > 10) && (buttonCount < 5))) && (buttonCount != 0)) {
      shouldPublish = true;
      valueChanged = true;
    }
    else if (bCount != buttonCount) {
      shouldPublish = false;
    }
  }
  else {
    buttonCounts[deviceMac] = buttonCount;
    shouldPublish = false;
  }

  if (shouldPublish) {
    buttonCounts[deviceMac] = buttonCount;
    std::string deviceButtonMeshTopic = contactTopic + aDevice + "/buttoncount";
    addToPublish(deviceButtonMeshTopic.c_str(), buttonCount, false);
    if (!isMeshNode && valueChanged) {
      //addToPublish(deviceButtonTopic.c_str(), "PUSHED", false);
      client.publish(deviceButtonTopic.c_str(), "PUSHED", false);
      lastButton[deviceMac] = millis();
    }
    else if (!isMeshNode) {
      addToPublish(deviceButtonTopic.c_str(), "IDLE", false);
    }
  }
}

void processEntrance(std::string & aDevice, std::string & deviceMac, std::string & aValueString, bool isActive, bool aPublish) {
  uint8_t byte8 = 0;
  bool shouldPublish = aPublish;
  bool valueChanged = false;
  if (isActive) {
    byte8 = (uint8_t) aValueString[8];
  }
  else {
    byte8 = (uint8_t) aValueString[14];
  }
  int entranceCountA = (byte8 & 0b10000000) ? 2 : 0;
  int entranceCountB = (byte8 & 0b01000000) ? 1 : 0;
  int entranceCount = entranceCountA + entranceCountB;
  std::string deviceInTopic = contactTopic + aDevice + "/in";
  std::map<std::string, int>::iterator itE = entranceCounts.find(deviceMac);
  if (itE != entranceCounts.end())
  {
    int eCount = itE->second;
    if (((eCount < entranceCount) ||  ((eCount == 3) && (entranceCount == 1))) && (entranceCount != 0)) {
      shouldPublish = true;
      valueChanged = true;
    }
    else if (eCount != entranceCount) {
      shouldPublish = false;
    }
  }
  else {
    entranceCounts[deviceMac] = entranceCount;
    shouldPublish = false;
  }

  if (shouldPublish) {
    entranceCounts[deviceMac] = entranceCount;

    std::string deviceInMeshTopic = contactTopic + aDevice + "/incount";
    addToPublish(deviceInMeshTopic.c_str(), entranceCount, false);
    if (!isMeshNode && valueChanged) {
      //addToPublish(deviceInTopic.c_str(), "ENTERED", false);
      client.publish(deviceInTopic.c_str(), "ENTERED", false);
      lastIn[deviceMac] = millis();
    }
    else if (!isMeshNode) {
      addToPublish(deviceInTopic.c_str(), "IDLE", false);
    }
  }
}

void processExit(std::string & aDevice, std::string & deviceMac, std::string & aValueString, bool isActive, bool aPublish) {
  uint8_t byte8 = 0;
  bool shouldPublish = aPublish;
  bool valueChanged = false;
  if (isActive) {
    byte8 = (uint8_t) aValueString[8];
  }
  else {
    byte8 = (uint8_t) aValueString[14];
  }
  std::string deviceOutTopic = contactTopic + aDevice + "/out";
  int outCountA = (byte8 & 0b00100000) ? 2 : 0;
  int outCountB = (byte8 & 0b00010000) ? 1 : 0;
  int outCount = outCountA + outCountB;
  std::map<std::string, int>::iterator itE = outCounts.find(deviceMac.c_str());
  if (itE != outCounts.end())
  {
    int oCount = itE->second;
    if (((oCount < outCount) ||  ((oCount == 3) && (outCount == 1))) && (outCount != 0)) {
      shouldPublish = true;
      valueChanged = true;
    }
    else if (oCount != outCount) {
      shouldPublish = false;
    }
  }
  else {
    outCounts[deviceMac.c_str()] = outCount;
    shouldPublish = false;
  }

  if (shouldPublish) {
    outCounts[deviceMac.c_str()] = outCount;

    std::string deviceOutMeshTopic = contactTopic + aDevice + "/outcount";
    addToPublish(deviceOutMeshTopic.c_str(), outCount, false);
    if (!isMeshNode && valueChanged) {
      //addToPublish(deviceOutTopic.c_str(), "EXITED", false);
      client.publish(deviceOutTopic.c_str(), "EXITED", false);
      lastOut[deviceMac] = millis();
    }
    else if (!isMeshNode) {
      addToPublish(deviceOutTopic.c_str(), "IDLE", false);
    }
  }
}


void processBotBattery(std::string & aDevice, std::string & deviceMac, std::string & aValueString, bool isActive, bool aPublish, JsonDocument & aJsonDoc) {
  int battLevel = 0;
  bool shouldPublish = aPublish;
  if (isActive) {
    uint8_t byte2 = (uint8_t) aValueString[2];
    battLevel = (byte2 & 0b01111111);
    batteryValues[aDevice] = battLevel;
    aJsonDoc["batt"] = battLevel;
  }
  else if (onlyPassiveScan) {
    shouldPublish = false;
  }
  else {
    battLevel = batteryValues[aDevice];
    aJsonDoc["batt"] = battLevel;
  }
  if (shouldPublish) {
    //std::string deviceBatteryTopic = ;
    addToPublish(botTopic + aDevice + "/battery", battLevel, true);
  }
}

void processCurtainBattery(std::string & aDevice, std::string & deviceMac, std::string & aValueString, bool isActive, bool aPublish, JsonDocument & aJsonDoc) {
  int battLevel = 0;
  bool shouldPublish = aPublish;
  if (isActive) {
    uint8_t byte2 = (uint8_t) aValueString[2];
    battLevel = (byte2 & 0b01111111);
    batteryValues[aDevice] = battLevel;
    aJsonDoc["batt"] = battLevel;
  }
  else if (onlyPassiveScan) {
    shouldPublish = false;
  }
  else {
    battLevel = batteryValues[aDevice];
    aJsonDoc["batt"] = battLevel;
  }
  if (shouldPublish) {
    //std::string deviceBatteryTopic = ;
    addToPublish(curtainTopic + aDevice + "/battery", battLevel, true);
  }
}

void processMeterBattery(std::string & aDevice, std::string & deviceMac, std::string & aValueString, bool isActive, bool aPublish, JsonDocument & aJsonDoc) {
  int battLevel = 0;
  bool shouldPublish = aPublish;
  if (isActive) {
    uint8_t byte2 = (uint8_t) aValueString[2];
    battLevel = (byte2 & 0b01111111);
    batteryValues[aDevice] = battLevel;
    aJsonDoc["batt"] = battLevel;
  }
  else if (onlyPassiveScan) {
    shouldPublish = false;
  }
  else {
    battLevel = batteryValues[aDevice];
    aJsonDoc["batt"] = battLevel;
  }
  if (shouldPublish) {
    //std::string deviceBatteryTopic = ;
    addToPublish(meterTopic + aDevice + "/battery", battLevel, true);
  }
}

void processContactBattery(std::string & aDevice, std::string & deviceMac, std::string & aValueString, bool isActive, bool aPublish) {
  int battLevel = 0;
  bool shouldPublish = aPublish;
  if (isActive) {
    uint8_t byte2 = (uint8_t) aValueString[2];
    battLevel = (byte2 & 0b01111111);
    batteryValues[aDevice] = battLevel;
    // aJsonDoc["batt"] = battLevel;
  }
  else if (onlyPassiveScan) {
    shouldPublish = false;
  }
  else {
    battLevel = batteryValues[aDevice];
  }
  if (shouldPublish) {
    //std::string deviceBatteryTopic = ;
    addToPublish(contactTopic + aDevice + "/battery", battLevel, true);
  }
}

void processMotionBattery(std::string & aDevice, std::string & deviceMac, std::string & aValueString, bool isActive, bool aPublish) {
  int battLevel = 0;
  bool shouldPublish = aPublish;
  if (isActive) {
    uint8_t byte2 = (uint8_t) aValueString[2];
    battLevel = (byte2 & 0b01111111);
    batteryValues[aDevice] = battLevel;
    //aJsonDoc["batt"] = battLevel;
  }
  else if (onlyPassiveScan) {
    shouldPublish = false;
  }
  else {
    battLevel = batteryValues[aDevice];
  }
  if (shouldPublish) {
    //std::string deviceBatteryTopic = ;
    addToPublish(motionTopic + aDevice + "/battery", battLevel, true);
  }
}

void processBotRSSI(std::string & aDevice, std::string & deviceMac, long anRSSI, bool isActive, bool aPublish, JsonDocument & aJsonDoc) {
  bool shouldPublish = aPublish;
  aJsonDoc["rssi"] = anRSSI;
  if (shouldPublish) {
    //std::string deviceRSSITopic = ;
    addToPublish(botTopic + aDevice + "/rssi", anRSSI, true);
  }
}

void processCurtainRSSI(std::string & aDevice, std::string & deviceMac, long anRSSI, bool isActive, bool aPublish, JsonDocument & aJsonDoc) {
  bool shouldPublish = aPublish;
  aJsonDoc["rssi"] = anRSSI;
  if (shouldPublish) {
    //std::string deviceRSSITopic = curtainTopic + aDevice + "/rssi";
    addToPublish(curtainTopic + aDevice + "/rssi", anRSSI, true);
  }
}

void processPlugRSSI(std::string & aDevice, std::string & deviceMac, long anRSSI, bool isActive, bool aPublish, JsonDocument & aJsonDoc) {
  bool shouldPublish = aPublish;
  aJsonDoc["rssi"] = anRSSI;
  if (shouldPublish) {
    addToPublish(plugTopic + aDevice + "/rssi", anRSSI, true);
  }
}

void processContactRSSI(std::string & aDevice, std::string & deviceMac, long anRSSI, bool isActive, bool aPublish) {
  bool shouldPublish = aPublish;
  //aJsonDoc["rssi"] = anRSSI;

  if (shouldPublish) {
    //std::string deviceRSSITopic = ;
    addToPublish(contactTopic + aDevice + "/rssi", anRSSI, true);
    if (meshContactSensors && enableMesh) {
      //deviceRSSITopic = ;
      addToPublish(contactMainTopic + aDevice + "/rssi", anRSSI, true);
    }
  }
}

void processMotionRSSI(std::string & aDevice, std::string & deviceMac, long anRSSI, bool isActive, bool aPublish) {
  bool shouldPublish = aPublish;
  //aJsonDoc["rssi"] = anRSSI;

  if (shouldPublish) {
    //std::string deviceRSSITopic = ;
    addToPublish(motionTopic + aDevice + "/rssi", anRSSI, true);
    if (meshMotionSensors && enableMesh) {
      //deviceRSSITopic = motionMainTopic + aDevice + "/rssi";
      addToPublish(motionMainTopic + aDevice + "/rssi", anRSSI, true);
    }
  }
}

void processMeterRSSI(std::string & aDevice, std::string & deviceMac, long anRSSI, bool isActive, bool aPublish, JsonDocument & aJsonDoc) {
  bool shouldPublish = aPublish;
  aJsonDoc["rssi"] = anRSSI;

  if (shouldPublish) {
    //std::string deviceRSSITopic = ;
    addToPublish(meterTopic + aDevice + "/rssi", anRSSI, true);
    if (meshMeters && enableMesh) {
      //deviceRSSITopic = ;
      addToPublish(meterMainTopic + aDevice + "/rssi", anRSSI, true);
    }
  }
}

static unsigned long lastOnlinePublished = 0;
static unsigned long lastSystemInfoPoll = 0;
static unsigned long lastRescan = 0;
static unsigned long lastScanCheck = 0;
static bool noResponse = false;
static bool waitForResponse = false;
static std::string lastDeviceControlled = "";

bool publishMQTT(QueuePublish aCommand) {
  if (client.isConnected()) {
    client.publish(aCommand.topic.c_str(), aCommand.payload.c_str(), aCommand.retain);
    return true;
  }
  return false;
}

/*bool publishMQTT(std::string topic, std::string payload) {
  return publishMQTT(topic, payload, false);
  }

  bool publishMQTT(std::string topic, char * payload) {
  return publishMQTT(topic, payload, false);
  }*/

void processContactSensorTasks() {
  std::string anAddr;
  std::string aDevice;
  unsigned long aTime = 0;
  std::map<std::string, std::string>::iterator itT = allContactSensors.begin();
  while (itT != allContactSensors.end())
  {
    anAddr = itT->second;
    aDevice = itT->first;
    std::map<std::string, unsigned long>::iterator itW = lastButton.find(anAddr.c_str());
    if (itW != lastButton.end())
    {
      aTime = itW->second;
      if ((millis() - aTime) > 1000) {
        std::string deviceButtonTopic = contactTopic + aDevice + "/button";
        addToPublish(deviceButtonTopic.c_str(), "IDLE", false);
        lastButton.erase(anAddr.c_str());
      }
    }

    itW = lastIn.find(anAddr.c_str());
    if (itW != lastIn.end())
    {
      aTime = itW->second;
      if ((millis() - aTime) > 1000) {
        std::string deviceInTopic = contactTopic + aDevice + "/in";
        addToPublish(deviceInTopic.c_str(), "IDLE", false);
        lastIn.erase(anAddr.c_str());
      }
    }

    itW = lastOut.find(anAddr.c_str());
    if (itW != lastOut.end())
    {
      aTime = itW->second;
      if ((millis() - aTime) > 1000) {
        std::string deviceOutTopic = contactTopic + aDevice + "/out";
        addToPublish(deviceOutTopic.c_str(), "IDLE", false);
        lastOut.erase(anAddr.c_str());
      }
    }

    itW = updateOpenCount.find(anAddr.c_str());
    if (itW != updateOpenCount.end())
    {
      aTime = itW->second;
      if ((millis() - aTime) > (timeToIgnoreDuplicates * 1000)) {
        std::map<std::string, int>::iterator itQQ = updateMeshOpenCount.find(anAddr.c_str());
        if (itQQ != updateMeshOpenCount.end())
        {
          openCounts[anAddr.c_str()] = itQQ->second;
          updateOpenCount.erase(anAddr.c_str());
        }
      }
    }

    itW = updateClosedCount.find(anAddr.c_str());
    if (itW != updateClosedCount.end())
    {
      aTime = itW->second;
      if ((millis() - aTime) > (timeToIgnoreDuplicates * 1000)) {
        std::map<std::string, int>::iterator itQQ = updateMeshClosedCount.find(anAddr.c_str());
        if (itQQ != updateMeshClosedCount.end())
        {
          closedCounts[anAddr.c_str()] = itQQ->second;
          updateClosedCount.erase(anAddr.c_str());
        }
      }
    }

    itW = updateTimeoutCount.find(anAddr.c_str());
    if (itW != updateTimeoutCount.end())
    {
      aTime = itW->second;
      if ((millis() - aTime) > (timeToIgnoreDuplicates * 1000)) {
        std::map<std::string, int>::iterator itQQ = updateMeshTimeoutCount.find(anAddr.c_str());
        if (itQQ != updateMeshTimeoutCount.end())
        {
          timeoutCounts[anAddr.c_str()] = itQQ->second;
          updateTimeoutCount.erase(anAddr.c_str());
        }
      }
    }

    itW = updateMotionCount.find(anAddr.c_str());
    if (itW != updateMotionCount.end())
    {
      aTime = itW->second;
      if ((millis() - aTime) > (timeToIgnoreDuplicates * 1000)) {
        std::map<std::string, int>::iterator itQQ = meshMotionCounts.find(anAddr.c_str());
        if (itQQ != meshMotionCounts.end())
        {
          motionCounts[anAddr.c_str()] = itQQ->second;
          updateMotionCount.erase(anAddr.c_str());
        }
      }
    }

    itW = updateNoMotionCount.find(anAddr.c_str());
    if (itW != updateNoMotionCount.end())
    {
      aTime = itW->second;
      if ((millis() - aTime) > (timeToIgnoreDuplicates * 1000)) {
        std::map<std::string, int>::iterator itQQ = meshNoMotionCounts.find(anAddr.c_str());
        if (itQQ != meshNoMotionCounts.end())
        {
          noMotionCounts[anAddr.c_str()] = itQQ->second;
          updateNoMotionCount.erase(anAddr.c_str());
        }
      }
    }

    itW = updateDarkCount.find(anAddr.c_str());
    if (itW != updateDarkCount.end())
    {
      aTime = itW->second;
      if ((millis() - aTime) > (timeToIgnoreDuplicates * 1000)) {
        std::map<std::string, int>::iterator itQQ = meshDarkCounts.find(anAddr.c_str());
        if (itQQ != meshDarkCounts.end())
        {
          darkCounts[anAddr.c_str()] = itQQ->second;
          updateDarkCount.erase(anAddr.c_str());
        }
      }
    }

    itW = updateBrightCount.find(anAddr.c_str());
    if (itW != updateBrightCount.end())
    {
      aTime = itW->second;
      if ((millis() - aTime) > (timeToIgnoreDuplicates * 1000)) {
        std::map<std::string, int>::iterator itQQ = meshBrightCounts.find(anAddr.c_str());
        if (itQQ != meshBrightCounts.end())
        {
          brightCounts[anAddr.c_str()] = itQQ->second;
          updateBrightCount.erase(anAddr.c_str());
        }
      }
    }
    itT++;
  }
}

void processMotionSensorTasks() {
  std::string anAddr;
  std::string aDevice;
  unsigned long aTime = 0;
  std::map<std::string, std::string>::iterator itT = allMotionSensors.begin();
  while (itT != allMotionSensors.end())
  {
    anAddr = itT->second;
    aDevice = itT->first;

    std::map<std::string, unsigned long>::iterator itW = updateMotionCount.find(anAddr.c_str());
    if (itW != updateMotionCount.end())
    {
      aTime = itW->second;
      if ((millis() - aTime) > (timeToIgnoreDuplicates * 1000)) {
        std::map<std::string, int>::iterator itQQ = meshMotionCounts.find(anAddr.c_str());
        if (itQQ != meshMotionCounts.end())
        {
          motionCounts[anAddr.c_str()] = itQQ->second;
          updateMotionCount.erase(anAddr.c_str());
        }
      }
    }

    itW = updateNoMotionCount.find(anAddr.c_str());
    if (itW != updateNoMotionCount.end())
    {
      aTime = itW->second;
      if ((millis() - aTime) > (timeToIgnoreDuplicates * 1000)) {
        std::map<std::string, int>::iterator itQQ = meshNoMotionCounts.find(anAddr.c_str());
        if (itQQ != meshNoMotionCounts.end())
        {
          noMotionCounts[anAddr.c_str()] = itQQ->second;
          updateNoMotionCount.erase(anAddr.c_str());
        }
      }
    }

    itW = updateDarkCount.find(anAddr.c_str());
    if (itW != updateDarkCount.end())
    {
      aTime = itW->second;
      if ((millis() - aTime) > (timeToIgnoreDuplicates * 1000)) {
        std::map<std::string, int>::iterator itQQ = meshDarkCounts.find(anAddr.c_str());
        if (itQQ != meshDarkCounts.end())
        {
          darkCounts[anAddr.c_str()] = itQQ->second;
          updateDarkCount.erase(anAddr.c_str());
        }
      }
    }

    itW = updateBrightCount.find(anAddr.c_str());
    if (itW != updateBrightCount.end())
    {
      aTime = itW->second;
      if ((millis() - aTime) > (timeToIgnoreDuplicates * 1000)) {
        std::map<std::string, int>::iterator itQQ = meshBrightCounts.find(anAddr.c_str());
        if (itQQ != meshBrightCounts.end())
        {
          brightCounts[anAddr.c_str()] = itQQ->second;
          updateBrightCount.erase(anAddr.c_str());
        }
      }
    }
    itT++;
  }
}

void publishAllMQTT() {
  int attempts = 0;
  while (!(publishQueue.isEmpty()) && attempts < 3) {
    if (!(client.isConnected())) {
      client.loop();
    }
    else {
      attempts++;
    }
    bool success = false;
    QueuePublish aCommand = publishQueue.getHead();
    success = publishMQTT(aCommand);
    if (success) {
      publishQueue.dequeue();
    }
  }
}

void processAllAdvData() {
  int attempts = 0;
  while (!(advDataQueue.isEmpty()) && attempts < 3) {
    if (!(client.isConnected())) {
      client.loop();
    }
    else {
      attempts++;
    }
    bool success = false;
    QueueAdvData aAdvData = advDataQueue.getHead();
    processAdvData(aAdvData.macAddr, aAdvData.rssi, aAdvData.aValueString, aAdvData.useActiveScan);
    advDataQueue.dequeue();
  }
}

void processAdvData(std::string & deviceMac, long anRSSI,  std::string & aValueString, bool useActiveScan) {
  yield();
  bool shouldPublish = alwaysMQTTUpdate;
  if (!initialScanComplete) {
    shouldPublish = true;
  }
  if (!shouldPublish) {
    if (shouldMQTTUpdateOrActiveScanForDevice(deviceMac)) {
      shouldPublish = true;
    }
  }
  std::string aDevice;
  std::string aState = "";
  std::string deviceStateTopic;
  std::string deviceAttrTopic;

  std::map<std::string, std::string>::iterator itS = allSwitchbotsOpp.find(deviceMac);
  if (itS != allSwitchbotsOpp.end())
  {
    aDevice = itS->second.c_str();
  }
  else {
    return;
  }
  std::string deviceName;
  itS = deviceTypes.find(deviceMac);
  if (itS != deviceTypes.end())
  {
    deviceName = itS->second.c_str();
  }

  int aLength = aValueString.length();
  if (deviceName == botName) {
    StaticJsonDocument<200> aJsonDoc;
    char aBuffer[200];

    uint8_t byte1 = (uint8_t) aValueString[1];
    uint8_t byte2 = (uint8_t) aValueString[2];
    bool isSwitch = (byte1 & 0b10000000);
    deviceStateTopic = botTopic + aDevice + "/state";
    deviceAttrTopic = botTopic + aDevice + "/attributes";

    std::string aMode = isSwitch ? "Switch" : "Press"; // Whether the light switch Add-on is used or not
    std::string deviceAssumedStateTopic = botTopic + aDevice + "/assumedstate";

    if (isSwitch) {
      std::map<std::string, bool>::iterator itP = botsInPressMode.find(deviceMac);
      if (itP != botsInPressMode.end())
      {
        botsInPressMode.erase(deviceMac);
      }
      aState = (byte1 & 0b01000000) ? "OFF" : "ON"; // Mine is opposite, not sure why
      addToPublish(deviceAssumedStateTopic.c_str(), aState.c_str(), true);
    }
    else {
      botsInPressMode[deviceMac] = true;
      std::map<std::string, bool>::iterator itE = botsSimulateONOFFinPRESSmode.find(aDevice);

      if (itE != botsSimulateONOFFinPRESSmode.end())
      {
        aMode = "PressOnOff";
        std::map<std::string, bool>::iterator itF = botsSimulatedStates.find(aDevice);
        bool boolState = false;
        if (itF != botsSimulatedStates.end())
        {
          boolState = itF->second;
        }
        else {
          boolState = itE->second;
        }
        if (boolState) {
          aState = "ON";
        } else {
          aState = "OFF";
        }
        addToPublish(deviceAssumedStateTopic.c_str(), aState.c_str(), true);
      }
      else {
        aState = "OFF";
      }
    }

    std::map<std::string, std::string>::iterator itH = botStates.find(deviceMac.c_str());
    if (itH != botStates.end())
    {
      std::string botState = itH->second.c_str();
      if (strcmp(botState.c_str(), aState.c_str()) != 0) {
        shouldPublish = true;

      }
    }
    else {
      shouldPublish = true;
    }

    botStates[deviceMac] = aState.c_str();

    aJsonDoc["mode"] = aMode;
    aJsonDoc["state"] = aState;

    processBotBattery(aDevice, deviceMac, aValueString, useActiveScan, shouldPublish, aJsonDoc);
    processBotRSSI(aDevice, deviceMac, anRSSI, useActiveScan, shouldPublish, aJsonDoc);

    if (shouldPublish) {
      if (useActiveScan) {
        delay(50);
        serializeJson(aJsonDoc, aBuffer, sizeof(aBuffer));
        addToPublish(deviceAttrTopic.c_str(), aBuffer, true);
        delay(50);
        addToPublish(deviceStateTopic.c_str(), aState.c_str(), true);
      }
      lastUpdateTimes[deviceMac] = millis();
    }
  }
  else if (deviceName == meterName) {
    StaticJsonDocument<200> aJsonDoc;
    char aBuffer[200];

    deviceStateTopic = meterTopic + aDevice + "/state";
    deviceAttrTopic = meterTopic + aDevice + "/attributes";

    uint8_t byte2 = 0;
    uint8_t byte3 = 0;
    uint8_t byte4 = 0;
    uint8_t byte5 = 0;

    if ( useActiveScan) {
      byte2 = (uint8_t) aValueString[2];
      byte3 = (uint8_t) aValueString[3];
      byte4 = (uint8_t) aValueString[4];
      byte5 = (uint8_t) aValueString[5];
    }
    else
    {
      byte3 = (uint8_t) aValueString[10];
      byte4 = (uint8_t) aValueString[11];
      byte5 = (uint8_t) aValueString[12];
    }

    int tempSign = (byte4 & 0b10000000) ? 1 : -1;
    float tempC = tempSign * ((byte4 & 0b01111111) + ((byte3 & 0b00001111) / 10.0));
    float tempF = (tempC * 9 / 5.0) + 32;
    tempF = round(tempF * 10) / 10.0;
    bool tempScale = (byte5 & 0b10000000) ;
    std::string str1 = (tempScale == true) ? "f" : "c";
    aJsonDoc["scale"] = str1;
    aJsonDoc["C"] = serialized(String(tempC, 1));
    aJsonDoc["F"] = serialized(String(tempF, 1));
    int humidity = byte5 & 0b01111111;
    aJsonDoc["hum"] = humidity;
    aState = String(tempC, 1).c_str();

    std::map<std::string, float>::iterator itH = meterTempCStates.find(deviceMac.c_str());
    if (itH != meterTempCStates.end())
    {
      float tempCState = itH->second;
      if (tempCState != tempC) {
        shouldPublish = true;

      }
    }
    else {
      shouldPublish = true;
    }
    std::map<std::string, float>::iterator itW = meterTempFStates.find(deviceMac.c_str());
    if (itW != meterTempFStates.end())
    {
      float tempFState = itW->second;
      if (tempFState != tempF) {
        shouldPublish = true;

      }
    }
    else {
      shouldPublish = true;
    }

    std::map<std::string, int>::iterator itR = meterHumidStates.find(deviceMac.c_str());
    if (itR != meterHumidStates.end())
    {
      int humidState = itR->second;
      if (humidState != humidity) {
        shouldPublish = true;

      }
    }
    else {
      shouldPublish = true;
    }

    meterTempCStates[deviceMac] = tempC;
    meterTempFStates[deviceMac] = tempF;
    meterHumidStates[deviceMac] = humidity;

    processMeterBattery(aDevice, deviceMac, aValueString, useActiveScan, shouldPublish, aJsonDoc);
    processMeterRSSI(aDevice, deviceMac, anRSSI, useActiveScan, shouldPublish, aJsonDoc);

    if (shouldPublish) {
      delay(50);
      serializeJson(aJsonDoc, aBuffer, sizeof(aBuffer));
      addToPublish(deviceAttrTopic.c_str(), aBuffer, true);
      delay(50);
      addToPublish(deviceStateTopic.c_str(), aState.c_str(), true);
      lastUpdateTimes[deviceMac] = millis();
    }
  }
  else if (deviceName == motionName) {

    processMotionMotion(aDevice, deviceMac, aValueString, useActiveScan, shouldPublish);
    processLightMotion(aDevice, deviceMac, aValueString, useActiveScan, shouldPublish);
    processLED(aDevice, deviceMac, aValueString, useActiveScan, shouldPublish);
    processSenseDistance(aDevice, deviceMac, aValueString, useActiveScan, shouldPublish);
    processMotionBattery(aDevice, deviceMac, aValueString, useActiveScan, shouldPublish);
    processMotionRSSI(aDevice, deviceMac, anRSSI, useActiveScan, shouldPublish);
  }

  else if (deviceName == contactName) {

    processContact(aDevice, deviceMac, aValueString, useActiveScan, shouldPublish);
    processMotionContact(aDevice, deviceMac, aValueString, useActiveScan, shouldPublish);
    processLightContact(aDevice, deviceMac, aValueString, useActiveScan, shouldPublish);
    processButton(aDevice, deviceMac, aValueString, useActiveScan, shouldPublish);
    processEntrance(aDevice, deviceMac, aValueString, useActiveScan, shouldPublish);
    processExit(aDevice, deviceMac, aValueString, useActiveScan, shouldPublish);
    processContactBattery(aDevice, deviceMac, aValueString, useActiveScan, shouldPublish);
    processContactRSSI(aDevice, deviceMac, anRSSI, useActiveScan, shouldPublish);
  }

  else if (deviceName == curtainName) {

    StaticJsonDocument<200> aJsonDoc;
    char aBuffer[200];
    processCurtainBattery(aDevice, deviceMac, aValueString, useActiveScan, shouldPublish, aJsonDoc);
    processCurtainRSSI(aDevice, deviceMac, anRSSI, useActiveScan, shouldPublish, aJsonDoc);

    std::string devicePosTopic = curtainTopic + aDevice + "/position";
    deviceStateTopic = curtainTopic + aDevice + "/state";
    deviceAttrTopic = curtainTopic + aDevice + "/attributes";

    uint8_t byte1 = (uint8_t) aValueString[1];
    uint8_t byte2 = (uint8_t) aValueString[2];
    uint8_t byte3 = (uint8_t) aValueString[3];
    uint8_t byte4 = (uint8_t) aValueString[4];

    bool calibrated = byte1 & 0b01000000;
    //int battLevel = byte2 & 0b01111111;
    int currentPosition = 100 - (byte3 & 0b01111111);
    int lightLevel = (byte4 >> 4) & 0b00001111;
    aState = "OPEN";

    aJsonDoc["calib"] = calibrated;
    //aJsonDoc["batt"] = battLevel;
    aJsonDoc["pos"] = currentPosition;
    if (currentPosition <= curtainClosedPosition)
      aState = "CLOSE";
    aJsonDoc["state"] = aState;
    aJsonDoc["light"] = lightLevel;

    std::map<std::string, std::string>::iterator itH = curtainStates.find(deviceMac.c_str());
    if (itH != curtainStates.end())
    {
      std::string curtainState = itH->second.c_str();
      if (strcmp(curtainState.c_str(), aState.c_str()) != 0) {
        shouldPublish = true;

      }
    }
    else {
      shouldPublish = true;
    }

    std::map<std::string, int>::iterator itW = curtainPositionStates.find(deviceMac.c_str());
    if (itW != curtainPositionStates.end())
    {
      int positionState = itW->second;
      if (positionState != currentPosition) {
        shouldPublish = true;

      }
    }
    else {
      shouldPublish = true;
    }

    std::map<std::string, int>::iterator itE = curtainLightStates.find(deviceMac.c_str());
    if (itE != curtainLightStates.end())
    {
      int lightState = itE->second;
      if (lightState != lightLevel) {
        shouldPublish = true;

      }
    }
    else {
      shouldPublish = true;
    }

    curtainStates[deviceMac] = aState.c_str();
    curtainPositionStates[deviceMac] = currentPosition;
    curtainLightStates[deviceMac] = lightLevel;

    if (shouldPublish) {
      if (useActiveScan) {
        delay(50);
        serializeJson(aJsonDoc, aBuffer, sizeof(aBuffer));
        addToPublish(deviceAttrTopic.c_str(), aBuffer, true);
        delay(50);
        addToPublish(deviceStateTopic.c_str(), aState.c_str(), true);
        StaticJsonDocument<50> docPos;
        docPos["pos"] = currentPosition;
        serializeJson(docPos, aBuffer, sizeof(aBuffer));
        addToPublish(devicePosTopic.c_str(), aBuffer, true);
      }
      lastUpdateTimes[deviceMac] = millis();
    }
  }

  else if (deviceName == plugName) {
    StaticJsonDocument<200> aJsonDoc;
    char aBuffer[200];

    deviceStateTopic = plugTopic + aDevice + "/state";
    deviceAttrTopic = plugTopic + aDevice + "/attributes";
    std::string devicePowerTopic = plugTopic + aDevice + "/energy";
    std::string deviceOverloadTopic = plugTopic + aDevice + "/overload";

    uint8_t byte9 = (uint8_t) aValueString[9];
    uint8_t byte12 = (uint8_t) aValueString[12];
    uint8_t byte13 = (uint8_t) aValueString[13];

    bool overload = byte12 & 0b10000000;

    byte powerHIGH = (byte13 & 0b11111111);
    byte powerLOW = (byte12 & 0b01111111);
    byte data2[] = {powerHIGH, powerLOW};
    long powerData = le16_to_cpu_signed(data2);

    std::string overloadStr = "";
    aState = "UNKNOWN";
    if (byte9 == 0) {
      aState = "OFF";
    }
    else if (byte9 == 128) {
      aState = "ON";
    }

    if (overload) {
      overloadStr = "true";
    }
    else {
      overloadStr = "false";
    }

    std::map<std::string, std::string>::iterator itH = plugStates.find(deviceMac.c_str());
    if (itH != plugStates.end())
    {
      std::string plugState = itH->second.c_str();
      if (strcmp(plugState.c_str(), aState.c_str()) != 0) {
        shouldPublish = true;

      }
    }
    else {
      shouldPublish = true;
    }

    std::map<std::string, long>::iterator itP = plugPowerStates.find(deviceMac.c_str());
    if (itP != plugPowerStates.end())
    {
      long plugPowerState = itP->second;
      if (plugPowerState != powerData) {
        shouldPublish = true;

      }
    }
    else {
      shouldPublish = true;
    }

    std::map<std::string, bool>::iterator itW = plugOverloadStates.find(deviceMac.c_str());
    if (itW != plugOverloadStates.end())
    {
      bool plugOverloadState = itW->second;
      if (plugOverloadState != overload) {
        shouldPublish = true;

      }
    }
    else {
      shouldPublish = true;
    }

    plugStates[deviceMac] = aState.c_str();
    plugPowerStates[deviceMac] = powerData;
    plugOverloadStates[deviceMac] = overload;

    processPlugRSSI(aDevice, deviceMac, anRSSI, useActiveScan, shouldPublish, aJsonDoc);

    aJsonDoc["state"] = aState;
    float powerLevel = powerData / 10.0;
    aJsonDoc["energy"] = serialized(String(powerLevel, 1));
    aJsonDoc["overload"] = overload;

    if (shouldPublish) {
      delay(50);
      serializeJson(aJsonDoc, aBuffer, sizeof(aBuffer));
      addToPublish(deviceAttrTopic.c_str(), aBuffer, true);
      delay(50);
      addToPublish(deviceStateTopic.c_str(), aState.c_str(), true);
      addToPublish(devicePowerTopic.c_str(), (String(powerLevel, 1)).c_str(), true);
      addToPublish(deviceOverloadTopic.c_str(), overloadStr, true);
      lastUpdateTimes[deviceMac] = millis();
    }
  }

  yield();

  if (shouldPublish) {
    lastUpdateTimes[deviceMac] = millis();
  }
  yield();

}

void publishLastwillOnline() {
  if ((millis() - lastOnlinePublished) > 30000) {
    if (client.isConnected()) {
      addToPublish(lastWill, "online", true);
      lastOnlinePublished = millis();
      String rssi = String(WiFi.RSSI());
      addToPublish(rssiStdStr.c_str(), rssi.c_str());
    }
  }
}

void publishHomeAssistantDiscoveryESPConfig() {
  String wifiMAC = String(WiFi.macAddress());
  addToPublish((home_assistant_mqtt_prefix + "/sensor/" + host + "/linkquality/config").c_str(), ("{\"~\":\"" + esp32Topic + "\"," +
               + "\"name\":\"" + haESPName("Linkquality") + "\"," +
               + "\"device\": {\"identifiers\":[\"switchbotesp_" + host + "_" + wifiMAC.c_str() + "\"],\"manufacturer\":\"" + manufacturer + "\",\"model\":\"" + "ESP32" + "\",\"name\": \"" + host + "\" }," +
               + "\"avty_t\": \"" + lastWill + "\"," +
               + "\"uniq_id\":\"switchbotesp_" + host + "_" + wifiMAC.c_str() + "_linkquality\"," +
               + "\"stat_t\":\"~/rssi\"," +
               + "\"icon\":\"mdi:signal\"," +
               + "\"unit_of_meas\": \"dBm\"}").c_str(), true);

  addToPublish((home_assistant_mqtt_prefix + "/sensor/" + host + "/firmware/config").c_str(), ("{\"~\":\"" + esp32Topic + "\"," +
               + "\"name\":\"" + haESPName("Firmware") + "\"," +
               + "\"device\": {\"identifiers\":[\"switchbotesp_" + host + "_" + wifiMAC.c_str() + "\"],\"manufacturer\":\"" + manufacturer + "\",\"model\":\"" + "ESP32" + "\",\"name\": \"" + host + "\" }," +
               + "\"avty_t\": \"" + lastWill + "\"," +
               + "\"uniq_id\":\"switchbotesp_" + host + "_" + wifiMAC.c_str() + "_firmware\"," +
               + "\"icon\":\"mdi:cog\"," +
               + "\"stat_t\":\"~/firmware\"}").c_str(), true);

  if (includeSensorRecentFailures) {
    addToPublish((home_assistant_mqtt_prefix + "/sensor/" + host + "/recentFailures/config").c_str(), ("{\"~\":\"" + esp32Topic + "\"," +
                 + "\"name\":\"" + haESPName("Recent Failures") + "\"," +
                 + "\"device\": {\"identifiers\":[\"switchbotesp_" + host + "_" + wifiMAC.c_str() + "\"],\"manufacturer\":\"" + manufacturer + "\",\"model\":\"" + "ESP32" + "\",\"name\": \"" + host + "\" }," +
                 + "\"avty_t\": \"" + lastWill + "\"," +
                 + "\"uniq_id\":\"switchbotesp_" + host + "_" + wifiMAC.c_str() + "_recentfailures\"," +
                 + "\"icon\":\"mdi:alert\"," +
                 + "\"stat_t\":\"~/recentFailures\"}").c_str(), true);
  }

  if (includeSensorSystemInfo) {
    addToPublish((home_assistant_mqtt_prefix + "/sensor/" + host + "/systemUptime/config").c_str(), ("{\"~\":\"" + esp32Topic + "\"," +
                 + "\"name\":\"" + haESPName("System Uptime") + "\"," +
                 + "\"device\": {\"identifiers\":[\"switchbotesp_" + host + "_" + wifiMAC.c_str() + "\"],\"manufacturer\":\"" + manufacturer + "\",\"model\":\"" + "ESP32" + "\",\"name\": \"" + host + "\" }," +
                 + "\"avty_t\": \"" + lastWill + "\"," +
                 + "\"uniq_id\":\"switchbotesp_" + host + "_" + wifiMAC.c_str() + "_systemuptime\"," +
                 + "\"icon\":\"mdi:timer-outline\"," +
                 + "\"stat_t\":\"~/systemUptime/state\"," +
                 + "\"json_attr_t\":\"~/systemUptime/json_attr\"," +
                 + "\"unit_of_meas\": \"h\"}").c_str(), true);
  }
}


void publishHomeAssistantDiscoveryPlugConfig(std::string & deviceName, std::string deviceMac, bool optimistic) {
  std::transform(deviceMac.begin(), deviceMac.end(), deviceMac.begin(), ::toupper);

  addToPublish((home_assistant_mqtt_prefix + "/sensor/" + deviceName + "/linkquality/config").c_str(), ("{\"~\":\"" + (plugTopic + deviceName) + "\"," +
               + "\"name\":\"" + haEntityName(deviceName, "Linkquality") + "\"," +
               + "\"device\": {\"identifiers\":[\"switchbot_" + deviceMac + "\"],\"manufacturer\":\"" + manufacturer + "\",\"model\":\"" + plugModel + "\",\"name\": \"" + deviceName + "\" }," +
               + "\"avty_t\": \"" + lastWill + "\"," +
               + "\"uniq_id\":\"switchbot_" + deviceMac + "_linkquality\"," +
               + "\"icon\":\"mdi:signal\"," +
               + "\"unit_of_meas\": \"dBm\", " +
               + "\"stat_t\":\"~/rssi\"}").c_str(), true);

  std::string optiString;
  if (optimistic) {
    optiString = "true";
  }
  else {
    optiString = "false";
  }

  addToPublish((home_assistant_mqtt_prefix + "/switch/" + deviceName + "/config").c_str(), ("{\"~\":\"" + (plugTopic + deviceName) + "\", " +
               + "\"name\":\"" + haEntityName(deviceName, "Switch") + "\"," +
               + "\"device\": {\"identifiers\":[\"switchbot_" + deviceMac + "\"],\"manufacturer\":\"" + manufacturer + "\",\"model\":\"" + plugModel + "\",\"name\": \"" + deviceName + "\" }," +
               + "\"avty_t\": \"" + lastWill + "\"," +
               + "\"uniq_id\":\"switchbot_" + deviceMac + "\", " +
               + "\"stat_t\":\"~/state\", " +
               + "\"opt\":" + optiString + ", " +
               + "\"cmd_t\": \"~/set\" }").c_str(), true);

  addToPublish((home_assistant_mqtt_prefix + "/sensor/" + deviceName + "/energy/config").c_str(), ("{\"~\":\"" + (plugTopic + deviceName) + "\"," +
               + "\"name\":\"" + haEntityName(deviceName, "Energy") + "\"," +
               + "\"device\": {\"identifiers\":[\"switchbot_" + deviceMac + "\"],\"manufacturer\":\"" + manufacturer + "\",\"model\":\"" + plugModel + "\",\"name\": \"" + deviceName + "\" }," +
               + "\"avty_t\": \"" + lastWill + "\"," +
               + "\"uniq_id\":\"switchbot_" + deviceMac + "_energy\"," +
               + "\"unit_of_meas\":\"W\"," +
               + "\"state_class\":\"measurement\"," +
               + "\"dev_cla\":\"power\"," +
               + "\"stat_t\":\"~/energy\"}").c_str(), true);

  addToPublish((home_assistant_mqtt_prefix + "/binary_sensor/" + deviceName + "/overload/config").c_str(), ("{\"~\":\"" + (plugTopic + deviceName) + "\"," +
               + "\"name\":\"" + haEntityName(deviceName, "Overload") + "\"," +
               + "\"device\": {\"identifiers\":[\"switchbot_" + deviceMac + "\"],\"manufacturer\":\"" + manufacturer + "\",\"model\":\"" + plugModel + "\",\"name\": \"" + deviceName + "\" }," +
               + "\"avty_t\": \"" + lastWill + "\"," +
               + "\"uniq_id\":\"switchbot_" + deviceMac + "_overload\"," +
               + "\"icon\":\"mdi:cog\"," +
               + "\"pl_on\":\"true\"," +
               + "\"pl_off\":\"false\"," +
               + "\"stat_t\":\"~/overload\"}").c_str(), true);

}


void publishHomeAssistantDiscoveryBotConfig(std::string & deviceName, std::string deviceMac, bool optimistic) {
  std::transform(deviceMac.begin(), deviceMac.end(), deviceMac.begin(), ::toupper);
  addToPublish((home_assistant_mqtt_prefix + "/sensor/" + deviceName + "/battery/config").c_str(), ("{\"~\":\"" + (botTopic + deviceName) + "\"," +
               + "\"name\":\"" + haEntityName(deviceName, "Battery") + "\"," +
               + "\"device\": {\"identifiers\":[\"switchbot_" + deviceMac + "\"],\"manufacturer\":\"" + manufacturer + "\",\"model\":\"" + botModel + "\",\"name\": \"" + deviceName + "\" }," +
               + "\"avty_t\": \"" + lastWill + "\"," +
               + "\"uniq_id\":\"switchbot_" + deviceMac + "_battery\"," +
               + "\"dev_cla\":\"battery\"," +
               + "\"unit_of_meas\": \"%\", " +
               + "\"stat_t\":\"~/battery\"}").c_str(), true);

  addToPublish((home_assistant_mqtt_prefix + "/sensor/" + deviceName + "/linkquality/config").c_str(), ("{\"~\":\"" + (botTopic + deviceName) + "\"," +
               + "\"name\":\"" + haEntityName(deviceName, "Linkquality") + "\"," +
               + "\"device\": {\"identifiers\":[\"switchbot_" + deviceMac + "\"],\"manufacturer\":\"" + manufacturer + "\",\"model\":\"" + botModel + "\",\"name\": \"" + deviceName + "\" }," +
               + "\"avty_t\": \"" + lastWill + "\"," +
               + "\"uniq_id\":\"switchbot_" + deviceMac + "_linkquality\"," +
               + "\"icon\":\"mdi:signal\"," +
               + "\"unit_of_meas\": \"dBm\", " +
               + "\"stat_t\":\"~/rssi\"}").c_str(), true);

  addToPublish((home_assistant_mqtt_prefix + "/binary_sensor/" + deviceName + "/inverted/config").c_str(), ("{\"~\":\"" + (botTopic + deviceName) + "\"," +
               + "\"name\":\"" + haEntityName(deviceName, "Inverted") + "\"," +
               + "\"device\": {\"identifiers\":[\"switchbot_" + deviceMac + "\"],\"manufacturer\":\"" + manufacturer + "\",\"model\":\"" + botModel + "\",\"name\": \"" + deviceName + "\" }," +
               + "\"avty_t\": \"" + lastWill + "\"," +
               + "\"uniq_id\":\"switchbot_" + deviceMac + "inverted\"," +
               + "\"stat_t\":\"~/settings\"," +
               + "\"icon\":\"mdi:cog\"," +
               + "\"pl_on\":true," +
               + "\"pl_off\":false," +
               + "\"value_template\":\"{{ value_json.inverted }}\"}").c_str(), true);

  addToPublish((home_assistant_mqtt_prefix + "/sensor/" + deviceName + "/mode/config").c_str(), ("{\"~\":\"" + (botTopic + deviceName) + "\"," +
               + "\"name\":\"" + haEntityName(deviceName, "Mode") + "\"," +
               + "\"device\": {\"identifiers\":[\"switchbot_" + deviceMac + "\"],\"manufacturer\":\"" + manufacturer + "\",\"model\":\"" + botModel + "\",\"name\": \"" + deviceName + "\" }," +
               + "\"avty_t\": \"" + lastWill + "\"," +
               + "\"uniq_id\":\"switchbot_" + deviceMac + "_mode\"," +
               + "\"icon\":\"mdi:cog\"," +
               + "\"stat_t\":\"~/attributes\"," +
               + "\"value_template\":\"{{ value_json.mode }}\"}").c_str(), true);

  addToPublish((home_assistant_mqtt_prefix + "/sensor/" + deviceName + "/firmware/config").c_str(), ("{\"~\":\"" + (botTopic + deviceName) + "\"," +
               + "\"name\":\"" + haEntityName(deviceName, "Firmware") + "\"," +
               + "\"device\": {\"identifiers\":[\"switchbot_" + deviceMac + "\"],\"manufacturer\":\"" + manufacturer + "\",\"model\":\"" + botModel + "\",\"name\": \"" + deviceName + "\" }," +
               + "\"avty_t\": \"" + lastWill + "\"," +
               + "\"uniq_id\":\"switchbot_" + deviceMac + "_firmware\"," +
               + "\"icon\":\"mdi:cog\"," +
               + "\"stat_t\":\"~/settings\"," +
               + "\"value_template\":\"{{ value_json.firmware }}\"}").c_str(), true);

  addToPublish((home_assistant_mqtt_prefix + "/sensor/" + deviceName + "/holdsecs/config").c_str(), ("{\"~\":\"" + (botTopic + deviceName) + "\"," +
               + "\"name\":\"" + haEntityName(deviceName, "HoldSecs") + "\"," +
               + "\"device\": {\"identifiers\":[\"switchbot_" + deviceMac + "\"],\"manufacturer\":\"" + manufacturer + "\",\"model\":\"" + botModel + "\",\"name\": \"" + deviceName + "\" }," +
               + "\"avty_t\": \"" + lastWill + "\"," +
               + "\"uniq_id\":\"switchbot_" + deviceMac + "_holdsecs\"," +
               + "\"icon\":\"mdi:cog\"," +
               + "\"stat_t\":\"~/settings\"," +
               + "\"value_template\":\"{{ value_json.hold }}\"}").c_str(), true);

  addToPublish((home_assistant_mqtt_prefix + "/sensor/" + deviceName + "/timers/config").c_str(), ("{\"~\":\"" + (botTopic + deviceName) + "\"," +
               + "\"name\":\"" + haEntityName(deviceName, "Timers") + "\"," +
               + "\"device\": {\"identifiers\":[\"switchbot_" + deviceMac + "\"],\"manufacturer\":\"" + manufacturer + "\",\"model\":\"" + botModel + "\",\"name\": \"" + deviceName + "\" }," +
               + "\"avty_t\": \"" + lastWill + "\"," +
               + "\"uniq_id\":\"switchbot_" + deviceMac + "_timers\"," +
               + "\"icon\":\"mdi:cog\"," +
               + "\"stat_t\":\"~/settings\"," +
               + "\"value_template\":\"{{ value_json.timers }}\"}").c_str(), true);

  std::string optiString;
  if (optimistic) {
    optiString = "true";
  }
  else {
    optiString = "false";
  }

  std::string aType = "switch";
  std::map<std::string, std::string>::iterator itS = allBotTypes.find(deviceName.c_str());
  if (itS != allBotTypes.end())
  {
    std::string aTypeTemp = itS->second;
    std::transform(aTypeTemp.begin(), aTypeTemp.end(), aTypeTemp.begin(), to_lower());
    if (strcmp(aTypeTemp.c_str(), "light") == 0) {
      aType = "light";
    }
    else if (strcmp(aTypeTemp.c_str(), "button") == 0) {
      aType = "button";
    }
  }

  if (strcmp(aType.c_str(), "light") == 0) {
    addToPublish((home_assistant_mqtt_prefix + "/light/" + deviceName + "/config").c_str(), ("{\"~\":\"" + (botTopic + deviceName) + "\", " +
                 + "\"name\":\"" + haEntityName(deviceName, "Light") + "\"," +
                 + "\"device\": {\"identifiers\":[\"switchbot_" + deviceMac + "\"],\"manufacturer\":\"" + manufacturer + "\",\"model\":\"" + botModel + "\",\"name\": \"" + deviceName + "\" }," +
                 + "\"avty_t\": \"" + lastWill + "\"," +
                 + "\"uniq_id\":\"switchbot_" + deviceMac + "\", " +
                 + "\"stat_t\":\"~/state\", " +
                 + "\"opt\":" + optiString + ", " +
                 + "\"cmd_t\": \"~/set\" }").c_str(), true);
  }
  else if (strcmp(aType.c_str(), "button") == 0) {
    addToPublish((home_assistant_mqtt_prefix + "/button/" + deviceName + "/config").c_str(), ("{\"~\":\"" + (botTopic + deviceName) + "\", " +
                 + "\"name\":\"" + haEntityName(deviceName, "Button") + "\"," +
                 + "\"device\": {\"identifiers\":[\"switchbot_" + deviceMac + "\"],\"manufacturer\":\"" + manufacturer + "\",\"model\":\"" + botModel + "\",\"name\": \"" + deviceName + "\" }," +
                 + "\"avty_t\": \"" + lastWill + "\"," +
                 + "\"uniq_id\":\"switchbot_" + deviceMac + "\", " +
                 + "\"cmd_t\": \"~/set\" }").c_str(), true);
  }
  else {
    addToPublish((home_assistant_mqtt_prefix + "/switch/" + deviceName + "/config").c_str(), ("{\"~\":\"" + (botTopic + deviceName) + "\", " +
                 + "\"name\":\"" + haEntityName(deviceName, "Switch") + "\"," +
                 + "\"device\": {\"identifiers\":[\"switchbot_" + deviceMac + "\"],\"manufacturer\":\"" + manufacturer + "\",\"model\":\"" + botModel + "\",\"name\": \"" + deviceName + "\" }," +
                 + "\"avty_t\": \"" + lastWill + "\"," +
                 + "\"uniq_id\":\"switchbot_" + deviceMac + "\", " +
                 + "\"stat_t\":\"~/state\", " +
                 + "\"opt\":" + optiString + ", " +
                 + "\"cmd_t\": \"~/set\" }").c_str(), true);
  }
}

void publishHomeAssistantDiscoveryCurtainConfig(std::string & deviceName, std::string deviceMac) {
  std::transform(deviceMac.begin(), deviceMac.end(), deviceMac.begin(), ::toupper);
  addToPublish((home_assistant_mqtt_prefix + "/sensor/" + deviceName + "/battery/config").c_str(), ("{\"~\":\"" + (curtainTopic + deviceName) + "\"," +
               + "\"name\":\"" + haEntityName(deviceName, "Battery") + "\"," +
               + "\"device\": {\"identifiers\":[\"switchbot_" + deviceMac + "\"],\"manufacturer\":\"" + manufacturer + "\",\"model\":\"" + curtainModel + "\",\"name\": \"" + deviceName + "\" }," +
               + "\"avty_t\": \"" + lastWill + "\"," +
               + "\"uniq_id\":\"switchbot_" + deviceMac + "_battery\"," +
               + "\"dev_cla\":\"battery\"," +
               + "\"unit_of_meas\": \"%\", " +
               + "\"stat_t\":\"~/battery\"}").c_str(), true);

  addToPublish((home_assistant_mqtt_prefix + "/sensor/" + deviceName + "/linkquality/config").c_str(), ("{\"~\":\"" + (curtainTopic + deviceName) + "\"," +
               + "\"name\":\"" + haEntityName(deviceName, "Linkquality") + "\"," +
               + "\"device\": {\"identifiers\":[\"switchbot_" + deviceMac + "\"],\"manufacturer\":\"" + manufacturer + "\",\"model\":\"" + curtainModel + "\",\"name\": \"" + deviceName + "\" }," +
               + "\"avty_t\": \"" + lastWill + "\"," +
               + "\"uniq_id\":\"switchbot_" + deviceMac + "_linkquality\"," +
               + "\"icon\":\"mdi:signal\"," +
               + "\"unit_of_meas\": \"dBm\", " +
               + "\"stat_t\":\"~/rssi\"}").c_str(), true);

  addToPublish((home_assistant_mqtt_prefix + "/sensor/" + deviceName + "/illuminance/config").c_str(), ("{\"~\":\"" + (curtainTopic + deviceName) + "\"," +
               + "\"name\":\"" + haEntityName(deviceName, "Light level") + "\"," +
               + "\"device\": {\"identifiers\":[\"switchbot_" + deviceMac + "\"],\"manufacturer\":\"" + manufacturer + "\",\"model\":\"" + curtainModel + "\",\"name\": \"" + deviceName + "\" }," +
               + "\"avty_t\": \"" + lastWill + "\"," +
               + "\"uniq_id\":\"switchbot_" + deviceMac + "_illuminance\"," +
               + "\"stat_t\":\"~/attributes\"," +
               + "\"unit_of_meas\": \"Level\", " +
               + "\"value_template\":\"{{ value_json.light }}\"}").c_str(), true);

  addToPublish((home_assistant_mqtt_prefix + "/binary_sensor/" + deviceName + "/calibrated/config").c_str(), ("{\"~\":\"" + (curtainTopic + deviceName) + "\"," +
               + "\"name\":\"" + haEntityName(deviceName, "Calibrated") + "\"," +
               + "\"device\": {\"identifiers\":[\"switchbot_" + deviceMac + "\"],\"manufacturer\":\"" + manufacturer + "\",\"model\":\"" + curtainModel + "\",\"name\": \"" + deviceName + "\" }," +
               + "\"avty_t\": \"" + lastWill + "\"," +
               + "\"uniq_id\":\"switchbot_" + deviceMac + "_calibrated\"," +
               + "\"stat_t\":\"~/attributes\"," +
               + "\"icon\":\"mdi:cog\"," +
               + "\"pl_on\":true," +
               + "\"pl_off\":false," +
               + "\"value_template\":\"{{ value_json.calib }}\"}").c_str(), true);

  addToPublish((home_assistant_mqtt_prefix + "/cover/" + deviceName + "/config").c_str(), ("{\"~\":\"" + (curtainTopic + deviceName) + "\", " +
               + "\"name\":\"" + haEntityName(deviceName, "Curtain") + "\"," +
               + "\"device\": {\"identifiers\":[\"switchbot_" + deviceMac + "\"],\"manufacturer\":\"" + manufacturer + "\",\"model\":\"" + curtainModel + "\",\"name\": \"" + deviceName + "\" }," +
               + "\"avty_t\": \"" + lastWill + "\"," +
               + "\"uniq_id\":\"switchbot_" + deviceMac + "\", " +
               + "\"dev_cla\":\"curtain\", " +
               + "\"stat_t\":\"~/state\", " +
               + "\"stat_open\": \"OPEN\", " +
               + "\"stat_clsd\": \"CLOSE\", " +
               + "\"stat_stopped\": \"PAUSE\", " +
               + "\"pl_stop\":\"PAUSE\", " +
               + "\"pos_open\": 100, " +
               + "\"pos_clsd\": 0, " +
               + "\"cmd_t\": \"~/set\", " +
               + "\"pos_t\":\"~/position\", " +
               + "\"pos_tpl\":\"{{ value_json.pos }}\", " +
               + "\"set_pos_t\": \"~/set\", " +
               + "\"set_pos_tpl\": \"{{ position }}\" }").c_str(), true);
  if (home_assistant_expose_seperate_curtain_position) {
    addToPublish((home_assistant_mqtt_prefix + "/sensor/" + deviceName + "/position/config").c_str(), ("{\"~\":\"" + (curtainTopic + deviceName) + "\"," +
                 + "\"name\":\"" + haEntityName(deviceName, "Position") + "\"," +
                 + "\"device\": {\"identifiers\":[\"switchbot_" + deviceMac + "\"],\"manufacturer\":\"" + manufacturer + "\",\"model\":\"" + curtainModel + "\",\"name\": \"" + deviceName + "\" }," +
                 + "\"avty_t\": \"" + lastWill + "\"," +
                 + "\"uniq_id\":\"switchbot_" + deviceMac + "_position\"," +
                 + "\"stat_t\":\"~/position\"," +
                 + "\"unit_of_meas\": \"%\", " +
                 + "\"value_template\":\"{{ value_json.pos }}\"}").c_str(), true);
  }
}

void publishHomeAssistantDiscoveryMeterConfig(std::string & deviceName, std::string deviceMac) {

  const char* lastWillToUse = lastWill;

  if (meshMeters) {
    lastWillToUse = lastWillScan;
  }

  std::transform(deviceMac.begin(), deviceMac.end(), deviceMac.begin(), ::toupper);
  addToPublish((home_assistant_mqtt_prefix + "/sensor/" + deviceName + "/battery/config").c_str(), ("{\"~\":\"" + (meterTopic + deviceName) + "\"," +
               + "\"name\":\"" + haEntityName(deviceName, "Battery") + "\"," +
               + "\"device\": {\"identifiers\":[\"switchbot_" + deviceMac + "\"],\"manufacturer\":\"" + manufacturer + "\",\"model\":\"" + meterModel + "\",\"name\": \"" + deviceName + "\" }," +
               + "\"avty_t\": \"" + lastWillToUse + "\"," +
               + "\"uniq_id\":\"switchbot_" + deviceMac + "_battery\"," +
               + "\"dev_cla\":\"battery\"," +
               + "\"unit_of_meas\": \"%\", " +
               + "\"stat_t\":\"~/battery\"}").c_str(), true);

  addToPublish((home_assistant_mqtt_prefix + "/sensor/" + deviceName + "/linkquality/config").c_str(), ("{\"~\":\"" + (meterTopic + deviceName) + "\"," +
               + "\"name\":\"" + haEntityName(deviceName, "Linkquality") + "\"," +
               + "\"device\": {\"identifiers\":[\"switchbot_" + deviceMac + "\"],\"manufacturer\":\"" + manufacturer + "\",\"model\":\"" + meterModel + "\",\"name\": \"" + deviceName + "\" }," +
               + "\"avty_t\": \"" + lastWillToUse + "\"," +
               + "\"uniq_id\":\"switchbot_" + deviceMac + "_linkquality\"," +
               + "\"icon\":\"mdi:signal\"," +
               + "\"unit_of_meas\": \"dBm\", " +
               + "\"stat_t\":\"~/rssi\"}").c_str(), true);

  addToPublish((home_assistant_mqtt_prefix + "/sensor/" + deviceName + "/temperature/config").c_str(), ("{\"~\":\"" + (meterTopic + deviceName) + "\"," +
               + "\"name\":\"" + haEntityName(deviceName, "Temperature") + "\"," +
               + "\"device\": {\"identifiers\":[\"switchbot_" + deviceMac + "\"],\"manufacturer\":\"" + manufacturer + "\",\"model\":\"" + meterModel + "\",\"name\": \"" + deviceName + "\" }," +
               + "\"avty_t\": \"" + lastWillToUse + "\"," +
               + "\"uniq_id\":\"switchbot_" + deviceMac + "_temperature\"," +
               + "\"stat_t\":\"~/attributes\"," +
               + "\"dev_cla\":\"temperature\", " +
               + "\"unit_of_meas\": \"°C\", " +
               + "\"value_template\":\"{{ value_json.C }}\"}").c_str(), true);

  addToPublish((home_assistant_mqtt_prefix + "/sensor/" + deviceName + "/humidity/config").c_str(), ("{\"~\":\"" + (meterTopic + deviceName) + "\"," +
               + "\"name\":\"" + haEntityName(deviceName, "Humidity") + "\"," +
               + "\"device\": {\"identifiers\":[\"switchbot_" + deviceMac + "\"],\"manufacturer\":\"" + manufacturer + "\",\"model\":\"" + meterModel + "\",\"name\": \"" + deviceName + "\" }," +
               + "\"avty_t\": \"" + lastWillToUse + "\"," +
               + "\"uniq_id\":\"switchbot_" + deviceMac + "_humidity\"," +
               + "\"stat_t\":\"~/attributes\"," +
               + "\"dev_cla\":\"humidity\", " +
               + "\"unit_of_meas\": \"%\", " +
               + "\"value_template\":\"{{ value_json.hum }}\"}").c_str(), true);
}


void publishHomeAssistantDiscoveryContactConfig(std::string & deviceName, std::string deviceMac) {

  const char* lastWillToUse = lastWill;

  if (meshContactSensors) {
    lastWillToUse = lastWillScan;
  }

  std::transform(deviceMac.begin(), deviceMac.end(), deviceMac.begin(), ::toupper);
  addToPublish((home_assistant_mqtt_prefix + "/sensor/" + deviceName + "/battery/config").c_str(), ("{\"~\":\"" + (contactTopic + deviceName) + "\"," +
               + "\"name\":\"" + haEntityName(deviceName, "Battery") + "\"," +
               + "\"device\": {\"identifiers\":[\"switchbot_" + deviceMac + "\"],\"manufacturer\":\"" + manufacturer + "\",\"model\":\"" + contactModel + "\",\"name\": \"" + deviceName + "\" }," +
               + "\"avty_t\": \"" + lastWillToUse + "\"," +
               + "\"uniq_id\":\"switchbot_" + deviceMac + "_battery\"," +
               + "\"dev_cla\":\"battery\"," +
               + "\"unit_of_meas\": \"%\", " +
               + "\"stat_t\":\"~/battery\"}").c_str(), true);

  addToPublish((home_assistant_mqtt_prefix + "/sensor/" + deviceName + "/linkquality/config").c_str(), ("{\"~\":\"" + (contactTopic + deviceName) + "\"," +
               + "\"name\":\"" + haEntityName(deviceName, "Linkquality") + "\"," +
               + "\"device\": {\"identifiers\":[\"switchbot_" + deviceMac + "\"],\"manufacturer\":\"" + manufacturer + "\",\"model\":\"" + contactModel + "\",\"name\": \"" + deviceName + "\" }," +
               + "\"avty_t\": \"" + lastWillToUse + "\"," +
               + "\"uniq_id\":\"switchbot_" + deviceMac + "_linkquality\"," +
               + "\"icon\":\"mdi:signal\"," +
               + "\"unit_of_meas\": \"dBm\", " +
               + "\"stat_t\":\"~/rssi\"}").c_str(), true);

  addToPublish((home_assistant_mqtt_prefix + "/sensor/" + deviceName + "/contact/config").c_str(), ("{\"~\":\"" + (contactTopic + deviceName) + "\"," +
               + "\"name\":\"" + haEntityName(deviceName, "Contact") + "\"," +
               + "\"device\": {\"identifiers\":[\"switchbot_" + deviceMac + "\"],\"manufacturer\":\"" + manufacturer + "\",\"model\":\"" + contactModel + "\",\"name\": \"" + deviceName + "\" }," +
               + "\"avty_t\": \"" + lastWillToUse + "\"," +
               + "\"uniq_id\":\"switchbot_" + deviceMac + "_contact\"," +
               + "\"icon\":\"mdi:door\"," +
               + "\"stat_t\":\"~/contact\"}").c_str(), true);

  addToPublish((home_assistant_mqtt_prefix + "/binary_sensor/" + deviceName + "/motion/config").c_str(), ("{\"~\":\"" + (contactTopic + deviceName) + "\"," +
               + "\"name\":\"" + haEntityName(deviceName, "Motion") + "\"," +
               + "\"device\": {\"identifiers\":[\"switchbot_" + deviceMac + "\"],\"manufacturer\":\"" + manufacturer + "\",\"model\":\"" + contactModel + "\",\"name\": \"" + deviceName + "\" }," +
               + "\"avty_t\": \"" + lastWillToUse + "\"," +
               + "\"uniq_id\":\"switchbot_" + deviceMac + "_motion\"," +
               + "\"stat_t\":\"~/motion\"," +
               + "\"dev_cla\":\"motion\"," +
               + "\"pl_on\":\"MOTION\"," +
               + "\"pl_off\":\"NO MOTION\"}").c_str(), true);

  addToPublish((home_assistant_mqtt_prefix + "/binary_sensor/" + deviceName + "/bincontact/config").c_str(), ("{\"~\":\"" + (contactTopic + deviceName) + "\"," +
               + "\"name\":\"" + haEntityName(deviceName, "BinaryContact") + "\"," +
               + "\"device\": {\"identifiers\":[\"switchbot_" + deviceMac + "\"],\"manufacturer\":\"" + manufacturer + "\",\"model\":\"" + contactModel + "\",\"name\": \"" + deviceName + "\" }," +
               + "\"avty_t\": \"" + lastWillToUse + "\"," +
               + "\"uniq_id\":\"switchbot_" + deviceMac + "_bincontact\"," +
               + "\"icon\":\"mdi:door\"," +
               + "\"stat_t\":\"~/bin\"," +
               + "\"pl_on\":\"OPEN\"," +
               + "\"pl_off\":\"CLOSED\"}").c_str(), true);

  addToPublish((home_assistant_mqtt_prefix + "/binary_sensor/" + deviceName + "/in/config").c_str(), ("{\"~\":\"" + (contactTopic + deviceName) + "\"," +
               + "\"name\":\"" + haEntityName(deviceName, "In") + "\"," +
               + "\"device\": {\"identifiers\":[\"switchbot_" + deviceMac + "\"],\"manufacturer\":\"" + manufacturer + "\",\"model\":\"" + contactModel + "\",\"name\": \"" + deviceName + "\" }," +
               + "\"avty_t\": \"" + lastWillToUse + "\"," +
               + "\"uniq_id\":\"switchbot_" + deviceMac + "_in\"," +
               + "\"icon\":\"mdi:motion-sensor\"," +
               + "\"stat_t\":\"~/in\"," +
               + "\"pl_on\":\"ENTERED\"," +
               + "\"pl_off\":\"IDLE\"}").c_str(), true);

  addToPublish((home_assistant_mqtt_prefix + "/binary_sensor/" + deviceName + "/out/config").c_str(), ("{\"~\":\"" + (contactTopic + deviceName) + "\"," +
               + "\"name\":\"" + haEntityName(deviceName, "Out") + "\"," +
               + "\"device\": {\"identifiers\":[\"switchbot_" + deviceMac + "\"],\"manufacturer\":\"" + manufacturer + "\",\"model\":\"" + contactModel + "\",\"name\": \"" + deviceName + "\" }," +
               + "\"avty_t\": \"" + lastWillToUse + "\"," +
               + "\"uniq_id\":\"switchbot_" + deviceMac + "_out\"," +
               + "\"icon\":\"mdi:exit-run\"," +
               + "\"stat_t\":\"~/out\"," +
               + "\"pl_on\":\"EXITED\"," +
               + "\"pl_off\":\"IDLE\"}").c_str(), true);

  addToPublish((home_assistant_mqtt_prefix + "/binary_sensor/" + deviceName + "/button/config").c_str(), ("{\"~\":\"" + (contactTopic + deviceName) + "\"," +
               + "\"name\":\"" + haEntityName(deviceName, "Button") + "\"," +
               + "\"device\": {\"identifiers\":[\"switchbot_" + deviceMac + "\"],\"manufacturer\":\"" + manufacturer + "\",\"model\":\"" + contactModel + "\",\"name\": \"" + deviceName + "\" }," +
               + "\"avty_t\": \"" + lastWillToUse + "\"," +
               + "\"uniq_id\":\"switchbot_" + deviceMac + "_button\"," +
               + "\"stat_t\":\"~/button\"," +
               + "\"icon\":\"mdi:gesture-tap-button\"," +
               + "\"pl_on\":\"PUSHED\"," +
               + "\"pl_off\":\"IDLE\"}").c_str(), true);

  addToPublish((home_assistant_mqtt_prefix + "/binary_sensor/" + deviceName + "/illuminance/config").c_str(), ("{\"~\":\"" + (contactTopic + deviceName) + "\"," +
               + "\"name\":\"" + haEntityName(deviceName, "Illuminance") + "\"," +
               + "\"device\": {\"identifiers\":[\"switchbot_" + deviceMac + "\"],\"manufacturer\":\"" + manufacturer + "\",\"model\":\"" + contactModel + "\",\"name\": \"" + deviceName + "\" }," +
               + "\"avty_t\": \"" + lastWillToUse + "\"," +
               + "\"uniq_id\":\"switchbot_" + deviceMac + "__illuminance\"," +
               + "\"stat_t\":\"~/illuminance\"," +
               + "\"dev_cla\":\"light\"," +
               + "\"pl_on\":\"BRIGHT\"," +
               + "\"pl_off\":\"DARK\"}").c_str(), true);

  addToPublish((home_assistant_mqtt_prefix + "/sensor/" + deviceName + "/lastmotion/config").c_str(), ("{\"~\":\"" + (contactTopic + deviceName) + "\"," +
               + "\"name\":\"" + haEntityName(deviceName, "LastMotion") + "\"," +
               + "\"device\": {\"identifiers\":[\"switchbot_" + deviceMac + "\"],\"manufacturer\":\"" + manufacturer + "\",\"model\":\"" + contactModel + "\",\"name\": \"" + deviceName + "\" }," +
               + "\"avty_t\": \"" + lastWillToUse + "\"," +
               + "\"uniq_id\":\"switchbot_" + deviceMac + "_lastmotion\"," +
               + "\"dev_cla\":\"timestamp\"," +
               + "\"stat_t\":\"~/lastmotion\"," +
               + "\"value_template\":\"{{ now() - timedelta(seconds = (value | int)) }}\"}").c_str(), true);

  addToPublish((home_assistant_mqtt_prefix + "/sensor/" + deviceName + "/lastcontact/config").c_str(), ("{\"~\":\"" + (contactTopic + deviceName) + "\"," +
               + "\"name\":\"" + haEntityName(deviceName, "LastContact") + "\"," +
               + "\"device\": {\"identifiers\":[\"switchbot_" + deviceMac + "\"],\"manufacturer\":\"" + manufacturer + "\",\"model\":\"" + contactModel + "\",\"name\": \"" + deviceName + "\" }," +
               + "\"avty_t\": \"" + lastWillToUse + "\"," +
               + "\"uniq_id\":\"switchbot_" + deviceMac + "_lastcontact\"," +
               + "\"dev_cla\":\"timestamp\"," +
               + "\"stat_t\":\"~/lastcontact\"," +
               + "\"value_template\":\"{{ now() - timedelta(seconds = (value | int)) }}\"}").c_str(), true);

  addToPublish((home_assistant_mqtt_prefix + "/sensor/" + deviceName + "/buttoncount/config").c_str(), ("{\"~\":\"" + (contactTopic + deviceName) + "\"," +
               + "\"name\":\"" + haEntityName(deviceName, "ButtonCount") + "\"," +
               + "\"device\": {\"identifiers\":[\"switchbot_" + deviceMac + "\"],\"manufacturer\":\"" + manufacturer + "\",\"model\":\"" + contactModel + "\",\"name\": \"" + deviceName + "\" }," +
               + "\"avty_t\": \"" + lastWillToUse + "\"," +
               + "\"uniq_id\":\"switchbot_" + deviceMac + "_buttoncount\"," +
               + "\"icon\":\"mdi:counter\"," +
               + "\"stat_t\":\"~/buttoncount\"}").c_str(), true);

  addToPublish((home_assistant_mqtt_prefix + "/sensor/" + deviceName + "/incount/config").c_str(), ("{\"~\":\"" + (contactTopic + deviceName) + "\"," +
               + "\"name\":\"" + haEntityName(deviceName, "InCount") + "\"," +
               + "\"device\": {\"identifiers\":[\"switchbot_" + deviceMac + "\"],\"manufacturer\":\"" + manufacturer + "\",\"model\":\"" + contactModel + "\",\"name\": \"" + deviceName + "\" }," +
               + "\"avty_t\": \"" + lastWillToUse + "\"," +
               + "\"uniq_id\":\"switchbot_" + deviceMac + "_entrancecount\"," +
               + "\"icon\":\"mdi:counter\"," +
               + "\"stat_t\":\"~/incount\"}").c_str(), true);

  addToPublish((home_assistant_mqtt_prefix + "/sensor/" + deviceName + "/outcount/config").c_str(), ("{\"~\":\"" + (contactTopic + deviceName) + "\"," +
               + "\"name\":\"" + haEntityName(deviceName, "OutCount") + "\"," +
               + "\"device\": {\"identifiers\":[\"switchbot_" + deviceMac + "\"],\"manufacturer\":\"" + manufacturer + "\",\"model\":\"" + contactModel + "\",\"name\": \"" + deviceName + "\" }," +
               + "\"avty_t\": \"" + lastWillToUse + "\"," +
               + "\"uniq_id\":\"switchbot_" + deviceMac + "_outcount\"," +
               + "\"icon\":\"mdi:counter\"," +
               + "\"stat_t\":\"~/outcount\"}").c_str(), true);;
}

void publishHomeAssistantDiscoveryMotionConfig(std::string & deviceName, std::string deviceMac) {

  const char* lastWillToUse = lastWill;

  if (meshMotionSensors) {
    lastWillToUse = lastWillScan;
  }

  std::transform(deviceMac.begin(), deviceMac.end(), deviceMac.begin(), ::toupper);
  addToPublish((home_assistant_mqtt_prefix + "/sensor/" + deviceName + "/battery/config").c_str(), ("{\"~\":\"" + (motionTopic + deviceName) + "\"," +
               + "\"name\":\"" + haEntityName(deviceName, "Battery") + "\"," +
               + "\"device\": {\"identifiers\":[\"switchbot_" + deviceMac + "\"],\"manufacturer\":\"" + manufacturer + "\",\"model\":\"" + motionModel + "\",\"name\": \"" + deviceName + "\" }," +
               + "\"avty_t\": \"" + lastWillToUse + "\"," +
               + "\"uniq_id\":\"switchbot_" + deviceMac + "_battery\"," +
               + "\"dev_cla\":\"battery\"," +
               + "\"unit_of_meas\": \"%\", " +
               + "\"stat_t\":\"~/battery\"}").c_str(), true);

  addToPublish((home_assistant_mqtt_prefix + "/sensor/" + deviceName + "/linkquality/config").c_str(), ("{\"~\":\"" + (motionTopic + deviceName) + "\"," +
               + "\"name\":\"" + haEntityName(deviceName, "Linkquality") + "\"," +
               + "\"device\": {\"identifiers\":[\"switchbot_" + deviceMac + "\"],\"manufacturer\":\"" + manufacturer + "\",\"model\":\"" + motionModel + "\",\"name\": \"" + deviceName + "\" }," +
               + "\"avty_t\": \"" + lastWillToUse + "\"," +
               + "\"uniq_id\":\"switchbot_" + deviceMac + "_linkquality\"," +
               + "\"icon\":\"mdi:signal\"," +
               + "\"unit_of_meas\": \"dBm\", " +
               + "\"stat_t\":\"~/rssi\"}").c_str(), true);

  addToPublish((home_assistant_mqtt_prefix + "/binary_sensor/" + deviceName + "/motion/config").c_str(), ("{\"~\":\"" + (motionTopic + deviceName) + "\"," +
               + "\"name\":\"" + haEntityName(deviceName, "Motion") + "\"," +
               + "\"device\": {\"identifiers\":[\"switchbot_" + deviceMac + "\"],\"manufacturer\":\"" + manufacturer + "\",\"model\":\"" + motionModel + "\",\"name\": \"" + deviceName + "\" }," +
               + "\"avty_t\": \"" + lastWillToUse + "\"," +
               + "\"uniq_id\":\"switchbot_" + deviceMac + "_motion\"," +
               + "\"stat_t\":\"~/motion\"," +
               + "\"dev_cla\":\"motion\"," +
               + "\"pl_on\":\"MOTION\"," +
               + "\"pl_off\":\"NO MOTION\"}").c_str(), true);

  addToPublish((home_assistant_mqtt_prefix + "/binary_sensor/" + deviceName + "/illuminance/config").c_str(), ("{\"~\":\"" + (motionTopic + deviceName) + "\"," +
               + "\"name\":\"" + haEntityName(deviceName, "Illuminance") + "\"," +
               + "\"device\": {\"identifiers\":[\"switchbot_" + deviceMac + "\"],\"manufacturer\":\"" + manufacturer + "\",\"model\":\"" + motionModel + "\",\"name\": \"" + deviceName + "\" }," +
               + "\"avty_t\": \"" + lastWillToUse + "\"," +
               + "\"uniq_id\":\"switchbot_" + deviceMac + "_illuminance\"," +
               + "\"stat_t\":\"~/illuminance\"," +
               + "\"dev_cla\":\"light\"," +
               + "\"pl_on\":\"BRIGHT\"," +
               + "\"pl_off\":\"DARK\"}").c_str(), true);

  addToPublish((home_assistant_mqtt_prefix + "/binary_sensor/" + deviceName + "/led/config").c_str(), ("{\"~\":\"" + (motionTopic + deviceName) + "\"," +
               + "\"name\":\"" + haEntityName(deviceName, "LED") + "\"," +
               + "\"device\": {\"identifiers\":[\"switchbot_" + deviceMac + "\"],\"manufacturer\":\"" + manufacturer + "\",\"model\":\"" + motionModel + "\",\"name\": \"" + deviceName + "\" }," +
               + "\"avty_t\": \"" + lastWillToUse + "\"," +
               + "\"uniq_id\":\"switchbot_" + deviceMac + "_led\"," +
               + "\"icon\":\"mdi:led-on\"," +
               + "\"pl_on\":\"ON\"," +
               + "\"pl_off\":\"OFF\"," +
               + "\"stat_t\":\"~/led\"}").c_str(), true);

  addToPublish((home_assistant_mqtt_prefix + "/sensor/" + deviceName + "/lastmotion/config").c_str(), ("{\"~\":\"" + (motionTopic + deviceName) + "\"," +
               + "\"name\":\"" + haEntityName(deviceName, "LastMotion") + "\"," +
               + "\"device\": {\"identifiers\":[\"switchbot_" + deviceMac + "\"],\"manufacturer\":\"" + manufacturer + "\",\"model\":\"" + motionModel + "\",\"name\": \"" + deviceName + "\" }," +
               + "\"avty_t\": \"" + lastWillToUse + "\"," +
               + "\"uniq_id\":\"switchbot_" + deviceMac + "_lastmotion\"," +
               + "\"dev_cla\":\"timestamp\"," +
               + "\"stat_t\":\"~/lastmotion\"," +
               + "\"value_template\":\"{{ now() - timedelta(seconds = (value | int)) }}\"}").c_str(), true);

  addToPublish((home_assistant_mqtt_prefix + "/sensor/" + deviceName + "/sensedistance/config").c_str(), ("{\"~\":\"" + (motionTopic + deviceName) + "\"," +
               + "\"name\":\"" + haEntityName(deviceName, "SenseDistance") + "\"," +
               + "\"device\": {\"identifiers\":[\"switchbot_" + deviceMac + "\"],\"manufacturer\":\"" + manufacturer + "\",\"model\":\"" + motionModel + "\",\"name\": \"" + deviceName + "\" }," +
               + "\"avty_t\": \"" + lastWillToUse + "\"," +
               + "\"uniq_id\":\"switchbot_" + deviceMac + "_sensedistance\"," +
               + "\"icon\":\"mdi:cog\"," +
               + "\"stat_t\":\"~/sensedist\"}").c_str(), true);
}


class ClientCallbacks : public NimBLEClientCallbacks {

    void onConnect(NimBLEClient* pClient) {
      printAString("Connected");
      pClient->updateConnParams(120, 120, 0, 60);
    };

    void onDisconnect(NimBLEClient* pClient) {
    };

    bool onConnParamsUpdateRequest(NimBLEClient* pClient, const ble_gap_upd_params* params) {
      if (params->itvl_min < 24) { /** 1.25ms units */
        return false;
      } else if (params->itvl_max > 40) { /** 1.25ms units */
        return false;
      } else if (params->latency > 2) { /** Number of intervals allowed to skip */
        return false;
      } else if (params->supervision_timeout > 100) { /** 10ms units */
        return false;
      }

      return true;
    };

    uint32_t onPassKeyRequest() {
      printAString("Client Passkey Request");
      return 123456;
    };

    bool onConfirmPIN(uint32_t pass_key) {
      printAString("The passkey YES/NO number: ");
      printAString(pass_key);
      return true;
    };

    void onAuthenticationComplete(ble_gap_conn_desc* desc) {
      if (!desc->sec_state.encrypted) {

        printAString("Encrypt connection failed - disconnecting");

        NimBLEDevice::getClientByID(desc->conn_handle)->disconnect();
        return;
      }
    };
};

bool unsubscribeToNotify(NimBLEClient* pClient) {
  NimBLERemoteService* pSvc = nullptr;
  NimBLERemoteCharacteristic* pChr = nullptr;

  pSvc = pClient->getService("cba20d00-224d-11e6-9fb8-0002a5d5c51b"); // custom device service
  if (pSvc) {    /** make sure it's not null */
    pChr = pSvc->getCharacteristic("cba20003-224d-11e6-9fb8-0002a5d5c51b"); // custom characteristic to notify
  }
  if (pChr) {    /** make sure it's not null */
    if (pChr->canNotify()) {
      if (!pChr->unsubscribe()) {
        return false;
      }
    }
  }
  else {
    printAString("CUSTOM notify service not found.");
    return false;
  }
  printAString("unsubscribed to notify");
  return true;
}

bool subscribeToNotify(NimBLEAdvertisedDevice* advDeviceToUse) {
  NimBLEClient* pClient = NimBLEDevice::getClientByPeerAddress(advDeviceToUse->getAddress());
  NimBLERemoteService* pSvc = nullptr;
  NimBLERemoteCharacteristic* pChr = nullptr;

  pSvc = pClient->getService("cba20d00-224d-11e6-9fb8-0002a5d5c51b"); // custom device service
  if (pSvc) {    /** make sure it's not null */
    pChr = pSvc->getCharacteristic("cba20003-224d-11e6-9fb8-0002a5d5c51b"); // custom characteristic to notify
  }
  if (pChr) {    /** make sure it's not null */
    if (pChr->canNotify()) {
      if (!pChr->subscribe(true, notifyCB)) {
        return false;
      }
    }
  }
  else {
    printAString("CUSTOM notify service not found.");
    return false;
  }
  printAString("subscribed to notify");
  return true;
}

bool writeSettings(NimBLEAdvertisedDevice* advDeviceToUse) {
  NimBLEClient* pClient = NimBLEDevice::getClientByPeerAddress(advDeviceToUse->getAddress());
  NimBLERemoteService* pSvc = nullptr;
  NimBLERemoteCharacteristic* pChr = nullptr;

  pSvc = pClient->getService("cba20d00-224d-11e6-9fb8-0002a5d5c51b"); // custom device service
  if (pSvc) {    /** make sure it's not null */
    pChr = pSvc->getCharacteristic("cba20002-224d-11e6-9fb8-0002a5d5c51b"); // custom characteristic to write
  }
  if (pChr) {    /** make sure it's not null */
    if (pChr->canWrite()) {
      std::string aPass = "";
      std::map<std::string, std::string>::iterator itU = allSwitchbotsOpp.find(advDeviceToUse->getAddress());
      if (itU != allSwitchbotsOpp.end())
      {
        aPass = getPass(itU->second.c_str());
      }
      uint8_t aPassCRC[4];
      if (aPass != "") {
        uint32_t aCRC = getPassCRC(aPass);
        for (int i = 0; i < 4; ++i)
        {
          aPassCRC[i] = ((uint8_t*)&aCRC)[3 - i];
        }

        byte bArray[] = {0x57, 0x12, aPassCRC[0] , aPassCRC[1] , aPassCRC[2]  , aPassCRC[3]}; // write to get settings of device
        if (pChr->writeValue(bArray, 6)) {
          printAString("Wrote new value to: ");
          printAString(pChr->getUUID().toString().c_str());
        }
        else {
          return false;
        }
      }
      else {
        byte bArray[] = {0x57, 0x02}; // write to get settings of device
        if (pChr->writeValue(bArray, 2)) {
          printAString("Wrote new value to: ");
          printAString(pChr->getUUID().toString().c_str());
        }
        else {
          return false;
        }
      }
    }
    else {
      printAString("CUSTOM write service not found.");

      return false;
    }

    printAString("Success! subscribed and got settings");

    return true;
  }

  return false;
}






/** Define a class to handle the callbacks when advertisments are received */
class AdvertisedDeviceCallbacks: public NimBLEAdvertisedDeviceCallbacks {

    void checkToContinueScan() {
      bool stopScan = false;
      if (client.isConnected()) {
        if (((allContactSensors.size() + allMotionSensors.size() + allPlugs.size() + allMeters.size()) != 0) || alwaysActiveScan ) {
          if (allSwitchbotsDev.size() == (allBots.size() + allCurtains.size() + allMeters.size() + allContactSensors.size() + allMotionSensors.size() + allPlugs.size())) {
            if (!initialScanComplete) {
              initialScanComplete = true;
              stopScan = true;
            }
            else if (overrideScan) {
              stopScan = true;
            }
          }
          if (!(commandQueue.isEmpty()) && initialScanComplete && !overrideScan) {
            stopScan = true;
            forceRescan = true;
            lastUpdateTimes = {};
            allSwitchbotsScanned = {};
          }
        }
        else {
          if ((allSwitchbotsDev.size() == (allBots.size() + allCurtains.size() + allMeters.size() + allContactSensors.size() + allMotionSensors.size() + allPlugs.size())) && (allSwitchbotsScanned.size() == (allBots.size() + allCurtains.size() + allMeters.size() + allContactSensors.size() + allMotionSensors.size() + allPlugs.size())))  {
            stopScan = true;
            forceRescan = false;
            allSwitchbotsScanned = {};
          }
          else if (!(commandQueue.isEmpty()) && initialScanComplete && !overrideScan) {
            forceRescan = true;
            lastUpdateTimes = {};
            stopScan = true;
            allSwitchbotsScanned = {};
          }
          else if (overrideScan && (allSwitchbotsDev.size() == (allBots.size() + allCurtains.size() + allMeters.size() + allContactSensors.size() + allMotionSensors.size() + allPlugs.size()))) {
            stopScan = true;
            allSwitchbotsScanned = {};
          }

        }
      }

      if (stopScan) {
        printAString("Stopping Scan found devices ... ");
        NimBLEDevice::getScan()->stop();
      }
      else {

        bool shouldActiveScan = false;
        if (allSwitchbotsDev.size() != (allBots.size() + allCurtains.size() + allMeters.size() + allContactSensors.size() + allMotionSensors.size() + allPlugs.size()))
        {
          shouldActiveScan = true;
        }
        std::string anAddr;
        std::map<std::string, std::string>::iterator itT = allSwitchbotsOpp.begin();
        while ((itT != allSwitchbotsOpp.end()) && !shouldActiveScan)
        {
          anAddr = itT->first;
          shouldActiveScan = shouldActiveScanForDevice(anAddr);
          itT++;
        }

        if (!alwaysActiveScan && !onlyActiveScan) {
          if ( shouldActiveScan && !isActiveScan)   {
            isActiveScan = true;
            NimBLEDevice::getScan()->stop();
          }
          else if (!shouldActiveScan && isActiveScan) {
            isActiveScan = false;
            NimBLEDevice::getScan()->stop();
          }
        }
      }
    }
    void onResult(NimBLEAdvertisedDevice* advertisedDevice) {
      printAString("START onResult");
      printAString("Advertised Device found: ");
      printAString(advertisedDevice->toString().c_str());
      if (ledOnScan) {
        digitalWrite(LED_BUILTIN, ledONValue);
      }
      publishLastwillOnline();
      std::string advStr = advertisedDevice->getAddress().toString();
      std::map<std::string, std::string>::iterator itS = allSwitchbotsOpp.find(advStr);
      bool gotAllStatus = false;

      if (itS != allSwitchbotsOpp.end())
      {
        /*if (!(NimBLEDevice::onWhiteList(advertisedDevice->getAddress()))) {
          NimBLEDevice::whiteListAdd(advertisedDevice->getAddress());
          }*/
        std::string deviceName = itS->second.c_str();
        if ((advertisedDevice->isAdvertisingService(NimBLEUUID("cba20d00-224d-11e6-9fb8-0002a5d5c51b"))) || isBotDevice(deviceName) || isCurtainDevice(deviceName) || isPlugDevice(deviceName) || isContactDevice(deviceName) || isMotionDevice(deviceName) || isMeterDevice(deviceName))
        {
          std::map<std::string, NimBLEAdvertisedDevice*>::iterator itY;
          itY = allSwitchbotsScanned.find(advStr);
          if (itY != allSwitchbotsScanned.end())
          {
            allSwitchbotsScanned.erase(advStr);
          }
          itY = allSwitchbotsScanned.find(advStr);
          if (((itY == allSwitchbotsScanned.end()) || alwaysMQTTUpdate) && client.isConnected())
          {
            if (home_assistant_mqtt_discovery) {
              std::map<std::string, bool>::iterator itM = discoveredDevices.find(advStr.c_str());
              if (itM == discoveredDevices.end()) {
                if (printSerialOutputForDebugging) {
                  Serial.printf("Publishing MQTT Discovery for %s (%s)\n", deviceName.c_str(), advStr.c_str());
                }
                if (isBotDevice(deviceName)) {
                  publishHomeAssistantDiscoveryBotConfig(deviceName, advStr, home_assistant_use_opt_mode);
                }
                else if (isMeterDevice(deviceName)) {
                  publishHomeAssistantDiscoveryMeterConfig(deviceName, advStr);
                }
                else if (isContactDevice(deviceName)) {
                  publishHomeAssistantDiscoveryContactConfig(deviceName, advStr);
                }
                else if (isMotionDevice(deviceName)) {
                  publishHomeAssistantDiscoveryMotionConfig(deviceName, advStr);
                }
                else if (isCurtainDevice(deviceName)) {
                  publishHomeAssistantDiscoveryCurtainConfig(deviceName, advStr);
                }
                else if (isPlugDevice(deviceName)) {
                  publishHomeAssistantDiscoveryPlugConfig(deviceName, advStr, home_assistant_use_opt_mode);
                }
                printAString("adding discovered device ... ");
                printAString(advStr.c_str());
                discoveredDevices[advStr.c_str()] = true;
                delay(100);
              }
            }

            printAString("Adding Our Service ... ");
            printAString(itS->second.c_str());

            std::string aValueString = "";
            if (isActiveScan) {

              if (client.isConnected()) {
                if (isPlugDevice(deviceName)) {
                  aValueString = advertisedDevice->getManufacturerData();
                  gotAllStatus = callForInfoAdvDev(advertisedDevice->getAddress().toString(), advertisedDevice->getRSSI() , aValueString);
                }
                else {
                  aValueString = advertisedDevice->getServiceData(0);
                  gotAllStatus = callForInfoAdvDev(advertisedDevice->getAddress().toString(), advertisedDevice->getRSSI() , aValueString);
                }
              }
              if (gotAllStatus) {
                allSwitchbotsScanned[advStr] = advertisedDevice;
                allSwitchbotsDev[advStr] = advertisedDevice;
                std::map<std::string, unsigned long>::iterator itR = rescanTimes.find(advStr);
                if (itR != rescanTimes.end())
                {
                  if (isCurtainDevice(deviceName)) {
                    if ((millis() - (itR->second) ) > (defaultCurtainScanAfterControlSecs * 1000)) {
                      rescanTimes.erase(advStr);
                    }
                  }
                  else  {
                    rescanTimes.erase(advStr);
                  }
                }
                lastActiveScanTimes[advStr] = millis();
                /* if (isContactDevice(deviceName) || isMotionDevice(deviceName) || isMeterDevice(deviceName)) {
                   rescanTimes[advStr] = millis();
                  }*/

                printAString("Assigned advDevService");
              }
            }
            else {
              if (isContactDevice(deviceName) || isMotionDevice(deviceName) || isPlugDevice(deviceName) || isMeterDevice(deviceName)) {
                aValueString = advertisedDevice->getManufacturerData();
                callForInfoAdvDev(advertisedDevice->getAddress().toString(), advertisedDevice->getRSSI() , aValueString);
              }

            }
          }
        }

      }
      else {
        NimBLEDevice::addIgnored(advStr);
      }
      //waitForDeviceCreation = false;

      checkToContinueScan();
      printAString("END onResult");
    };



    bool callForInfoAdvDev(std::string deviceMac, long anRSSI,  std::string & aValueString) {
      yield();
      printAString("START callForInfoAdvDev");
      printAString("callForInfoAdvDev");
      if ((strcmp(deviceMac.c_str(), "") == 0)) {
        return false;
      }
      if ((strcmp(aValueString.c_str(), "") == 0)) {
        return false;
      }

      std::map<std::string, std::string>::iterator itS = allSwitchbotsOpp.find(deviceMac);
      if (itS == allSwitchbotsOpp.end())
      {
        return false;
      }

      std::string deviceName;
      itS = deviceTypes.find(deviceMac);
      if (itS != deviceTypes.end())
      {
        deviceName = itS->second.c_str();
      }

      int aLength = aValueString.length();
      if (deviceName == botName) {
        if (aLength < 3) {
          return false;
        }
        addToAdvDevData(deviceMac, anRSSI,  aValueString, isActiveScan);
      }
      else if (deviceName == meterName) {
        if (aLength < 6) {
          return false;
        }

        if (((aLength < 6) && isActiveScan) || (!isActiveScan && (aLength < 13))) {
          return false;
        }

        addToAdvDevData(deviceMac, anRSSI,  aValueString, isActiveScan);
      }
      else if (deviceName == motionName) {

        if (aLength < 6) {
          return false;
        }

        if (((aLength < 6) && isActiveScan) || (!isActiveScan && (aLength < 12))) {
          return false;
        }

        addToAdvDevData(deviceMac, anRSSI,  aValueString, isActiveScan);
      }
      else if (deviceName == contactName) {
        if (aLength < 9) {
          return false;
        }

        if (((aLength < 9) && isActiveScan) || (!isActiveScan && (aLength < 15))) {
          return false;
        }
        addToAdvDevData(deviceMac, anRSSI,  aValueString, isActiveScan);
      }
      else if (deviceName == plugName) {
        if (aLength < 12) {
          return false;
        }

        addToAdvDevData(deviceMac, anRSSI,  aValueString, isActiveScan);
      }
      else if (deviceName == curtainName) {
        if (aLength < 5) {
          return false;
        }
        addToAdvDevData(deviceMac, anRSSI,  aValueString, isActiveScan);
      }
      else {
        return false;
      }
      yield();
      printAString("END callForInfoAdvDev");
      return true;

    };
};

void initialScanEndedCB(NimBLEScanResults results) {
  printAString("START initialScanEndedCB");
  //pScan->setFilterPolicy(BLE_HCI_SCAN_FILT_USE_WL);
  lastOnlinePublished = (((millis() - 60000) > 0) ? (millis() - 60000) : 0);
  yield();
  if (!alwaysActiveScan && !onlyActiveScan & initialScanComplete) {
    isActiveScan = false;
  }
  pScan->setActiveScan(isActiveScan);
  delay(50);
  printAString("initialScanEndedCB");
  if (ledOnBootScan) {
    digitalWrite(LED_BUILTIN, ledOFFValue);
  }
  initialScanComplete = true;
  isRescanning = false;
  allSwitchbotsScanned = {};
  delay(20);
  printAString("Scan Ended");
  /* if (allSwitchbotsDev.size() == (allBots.size() + allCurtains.size() + allMeters.size() + allContactSensors.size() + allMotionSensors.size())) {
     pScan->setFilterPolicy(BLE_HCI_SCAN_FILT_USE_WL);
     }*/

  client.publish(ESPMQTTTopic.c_str(), "{\"status\":\"idle\"}");
  printAString("END initialScanEndedCB");
}

void scanEndedCB(NimBLEScanResults results) {
  printAString("START scanEndedCB");
  lastOnlinePublished = (((millis() - 60000) > 0) ? (millis() - 60000) : 0);
  yield();
  if (!alwaysActiveScan && !onlyActiveScan & initialScanComplete) {
    isActiveScan = false;
  }
  pScan->setActiveScan(isActiveScan);
  if (ledOnScan || ledOnCommand) {
    digitalWrite(LED_BUILTIN, ledOFFValue);
  }
  allSwitchbotsScanned = {};
  delay(50);
  printAString("Scan Ended");
  /*if (allSwitchbotsDev.size() == (allBots.size() + allCurtains.size() + allMeters.size() + allContactSensors.size() + allMotionSensors.size())) {

    pScan->setFilterPolicy(BLE_HCI_SCAN_FILT_USE_WL);
    } */

  client.publish(ESPMQTTTopic.c_str(), "{\"status\":\"idle\"}");

  printAString("END scanEndedCB");
}

void rescanEndedCB(NimBLEScanResults results) {
  printAString("START rescanEndedCB");
  lastOnlinePublished = (((millis() - 60000) > 0) ? (millis() - 60000) : 0);
  yield();
  if (!alwaysActiveScan && !onlyActiveScan & initialScanComplete) {
    isActiveScan = false;
  }
  pScan->setActiveScan(isActiveScan);
  if (ledOnScan || ledOnCommand) {
    digitalWrite(LED_BUILTIN, ledOFFValue);
  }
  isRescanning = false;
  lastRescan = millis();
  allSwitchbotsScanned = {};
  delay(50);
  printAString("ReScan Ended");

  /* if (allSwitchbotsDev.size() == (allBots.size() + allCurtains.size() + allMeters.size() + allContactSensors.size() + allMotionSensors.size())) {

      pScan->setFilterPolicy(BLE_HCI_SCAN_FILT_USE_WL);
    } */
  client.publish(ESPMQTTTopic.c_str(), "{\"status\":\"idle\"}");

  printAString("END rescanEndedCB");
}

void scanForeverEnded(NimBLEScanResults results) {
  printAString("START scanForeverEnded");
  lastOnlinePublished = (((millis() - 60000) > 0) ? (millis() - 60000) : 0);
  yield();
  pScan->setActiveScan(isActiveScan);
  if (ledOnScan || ledOnCommand) {
    digitalWrite(LED_BUILTIN, ledOFFValue);
  }
  isRescanning = false;
  allSwitchbotsScanned = {};
  delay(50);
  printAString("forever scan Ended");

  /*  if (allSwitchbotsDev.size() == (allBots.size() + allCurtains.size() + allMeters.size() + allContactSensors.size() + allMotionSensors.size())) {

     pScan->setFilterPolicy(BLE_HCI_SCAN_FILT_USE_WL);
     } */
  client.publish(ESPMQTTTopic.c_str(), "{\"status\":\"idle\"}");

  printAString("END scanForeverEnded");
}

std::string getPass(std::string aDevice) {
  std::map<std::string, std::string>::iterator itS = allPasswords.find(aDevice);
  std::string aPass = "";
  if (itS != allPasswords.end())
  {
    aPass = itS->second;
  }
  return aPass;
}

uint32_t getPassCRC(std::string & aDevice) {
  const uint8_t * byteBuffer = (const unsigned char *)(aDevice.c_str());
  uint32_t crc = 0xFFFFFFFF;
  for (size_t i = 0; i < aDevice.length(); i++)
  {
    crc ^= byteBuffer[i];
    for (uint8_t bit = 0; bit < 8; bit++) {
      if (crc & 1) {
        crc = (crc >> 1) ^ 0xEDB88320;
      }
      else {
        crc >>= 1;
      }
    }
  }
  return ~crc;
}

static ClientCallbacks clientCB;

// BEGIN GENERATED ADMIN RUNTIME
// Configuration is applied once, before WiFi, BLE and MQTT are started.
static admin::Config adminConfig;
static Preferences adminStore;
static bool adminStorageOK=false, adminAP=false, adminAuthEnabled=false;
static String adminToken, adminNotice, adminCandidate;
static uint32_t adminBootAt=0, adminConnectedAt=0, adminRestartAt=0, adminAPAt=0;

static String adminRandom() {
  char value[33];
  snprintf(value,sizeof(value),"%08lx%08lx%08lx%08lx",(unsigned long)esp_random(),(unsigned long)esp_random(),(unsigned long)esp_random(),(unsigned long)esp_random());
  return String(value);
}
static void adminJson(int status,const JsonDocument &doc) {
  String body;serializeJson(doc,body);
  server.sendHeader("Cache-Control","no-store");
  server.sendHeader("X-Content-Type-Options","nosniff");
  server.send(status,"application/json; charset=utf-8",body);
}
static void adminError(int status,const char *message) {
  StaticJsonDocument<256> doc;doc["error"]=message;adminJson(status,doc);
}
static bool adminAuthorized(bool write=false) {
  if(adminAuthEnabled && !server.authenticate("admin",otaPass.c_str())) {server.requestAuthentication();return false;}
  if(write && (adminToken.isEmpty() || server.header("X-CSRF-Token")!=adminToken)) {adminError(403,"Seite neu laden: Sitzung ungueltig.");return false;}
  if(write && (adminTrial || adminRestartAt)) {adminError(409,"Neustart oder Verbindungspruefung laeuft.");return false;}
  return true;
}
static void adminCollectDevices(std::map<std::string,std::string> &list,const char *type) {
  for(const auto &item:list){admin::Device d;d.id=item.first;d.mac=admin::lowerMac(item.second);d.type=type;
    if(d.type=="bot"){auto p=allPasswords.find(d.id);if(p!=allPasswords.end())d.password=p->second;auto t=allBotTypes.find(d.id);if(t!=allBotTypes.end())d.entity=t->second;}
    adminConfig.devices.push_back(d);
  }
}
static void adminCaptureDefaults() {
  adminConfig.host=host;adminConfig.ssid=ssid;adminConfig.wifiPassword=password;
  adminConfig.mqttHost=mqtt_host;adminConfig.mqttUser=mqtt_user?mqtt_user:"";adminConfig.mqttPassword=mqtt_pass?mqtt_pass:"";
  adminConfig.port=mqtt_port;adminConfig.topic=mqtt_main_topic;
  adminConfig.scanSeconds=initialScan;adminConfig.rescanSeconds=rescanTime;adminConfig.retries=noResponseRetryAmount;
  adminConfig.staticAddress=useStaticIP;
  adminConfig.ip=staticIP.toString().c_str();adminConfig.gateway=staticGateway.toString().c_str();adminConfig.subnet=staticSubnet.toString().c_str();adminConfig.dns=staticPrimaryDNS.toString().c_str();
  adminCollectDevices(allBots,"bot");adminCollectDevices(allCurtains,"curtain");adminCollectDevices(allMeters,"meter");adminCollectDevices(allContactSensors,"contact");adminCollectDevices(allMotionSensors,"motion");adminCollectDevices(allPlugs,"plug");
}
static void adminApply() {
  host=adminConfig.host.c_str();ssid=adminConfig.ssid.c_str();password=adminConfig.wifiPassword.c_str();
  mqtt_host=adminConfig.mqttHost.c_str();mqtt_user=adminConfig.mqttUser.c_str();mqtt_pass=adminConfig.mqttPassword.c_str();mqtt_port=adminConfig.port;mqtt_main_topic=adminConfig.topic;
  initialScan=adminConfig.scanSeconds;rescanTime=adminConfig.rescanSeconds;noResponseRetryAmount=adminConfig.retries;
  useStaticIP=adminConfig.staticAddress;
  staticIP.fromString(adminConfig.ip.c_str());staticGateway.fromString(adminConfig.gateway.c_str());staticSubnet.fromString(adminConfig.subnet.c_str());staticPrimaryDNS.fromString(adminConfig.dns.c_str());
  allBots.clear();allCurtains.clear();allMeters.clear();allContactSensors.clear();allMotionSensors.clear();allPlugs.clear();allPasswords.clear();allBotTypes.clear();
  for(const auto &d:adminConfig.devices){
    auto *list=&allBots;if(d.type=="curtain")list=&allCurtains;else if(d.type=="meter")list=&allMeters;else if(d.type=="contact")list=&allContactSensors;else if(d.type=="motion")list=&allMotionSensors;else if(d.type=="plug")list=&allPlugs;
    (*list)[d.id]=d.mac;if(d.type=="bot"){allPasswords[d.id]=d.password;allBotTypes[d.id]=d.entity;}
  }
  hostForControl=host;hostForScan=(meshHost==nullptr||strlen(meshHost)==0)?host:meshHost;
  ESPMQTTTopic=mqtt_main_topic+"/"+hostForControl;ESPMQTTTopicMesh=mqtt_main_topic+"/"+hostForScan;
  esp32Topic=ESPMQTTTopic+"/esp32";rssiStdStr=esp32Topic+"/rssi";
  lastWillStr=ESPMQTTTopic+"/lastwill";lastWillScanStr=ESPMQTTTopicMesh+"/lastwill";lastWill=lastWillStr.c_str();lastWillScan=lastWillScanStr.c_str();
  botTopic=ESPMQTTTopic+"/bot/";plugTopic=ESPMQTTTopic+"/plug/";curtainTopic=ESPMQTTTopic+"/curtain/";
  meterTopic=ESPMQTTTopic+"/meter/";contactTopic=ESPMQTTTopic+"/contact/";motionTopic=ESPMQTTTopic+"/motion/";
  meterMainTopic=meterTopic;contactMainTopic=contactTopic;motionMainTopic=motionTopic;
  rescanStdStr=ESPMQTTTopic+"/rescan";requestInfoStdStr=ESPMQTTTopic+"/requestInfo";requestSettingsStdStr=ESPMQTTTopic+"/requestSettings";
  setModeStdStr=ESPMQTTTopic+"/setMode";setHoldStdStr=ESPMQTTTopic+"/setHold";holdPressStdStr=ESPMQTTTopic+"/holdPress";
  client.setMqttClientName(host);client.setMqttServer(mqtt_host,adminConfig.mqttUser.empty()?nullptr:mqtt_user,adminConfig.mqttUser.empty()?nullptr:mqtt_pass,mqtt_port);
}
static void adminStartAP() {
  if(adminAP)return;
  String name="SwitchBot-"+WiFi.macAddress().substring(12);name.replace(":","");
  WiFi.mode(WIFI_AP_STA);adminAP=WiFi.softAP(name.c_str(),otaPass.c_str());adminAPAt=millis();
  Serial.printf("Setup WiFi: %s\nSetup address: http://192.168.4.1\nAdmin user: admin\nAdmin password: %s\n",name.c_str(),otaPass.c_str());
}
static void adminLoad() {
  adminCaptureDefaults();
  adminStorageOK=adminStore.begin("sb-admin",false);
  adminToken=adminRandom();
  adminAuthEnabled=adminStorageOK && adminStore.getBool("authEnabled",false);
  otaPass=adminStorageOK?adminStore.getString("password",""):String("");
  if(otaPass.length()<12){otaPass=adminRandom().substring(0,20);if(adminStorageOK)adminStorageOK=adminStore.putString("password",otaPass)==otaPass.length();Serial.printf("Initial admin user: admin\nInitial admin password: %s\n",otaPass.c_str());}
  String stored=adminStorageOK?adminStore.getString("active",""):String("");
  if(stored.length()){
    DynamicJsonDocument doc(12000);admin::Config loaded;std::string error;
    if(!deserializeJson(doc,stored)&&admin::decode(doc.as<JsonVariantConst>(),adminConfig,loaded,error))adminConfig=loaded;
    else adminNotice="Gespeicherte Konfiguration ungueltig; Startwerte geladen.";
  }
  if(adminStorageOK){
    adminCandidate=adminStore.getString("pending","");
    if(adminCandidate.length()){
      if(adminStore.getBool("trying",false)){adminStore.remove("pending");adminStore.remove("trying");adminCandidate="";adminNotice="Vorherige Konfiguration nach abgebrochener Pruefung wiederhergestellt.";}
      else {
        DynamicJsonDocument doc(12000);admin::Config candidate;std::string error;
        if(!deserializeJson(doc,adminCandidate)&&admin::decode(doc.as<JsonVariantConst>(),adminConfig,candidate,error)&&adminStore.putBool("trying",true)) {adminConfig=candidate;adminTrial=true;}
        else {adminStore.remove("pending");adminCandidate="";adminNotice="Neue Konfiguration konnte nicht geladen werden.";}
      }
    }
  }
  adminApply();adminBootAt=millis();
}
static bool adminCleanupDiscovery() {
  if(!client.isConnected())return false;
  adminCleaning=true;adminCleanupOK=true;
  publishHomeAssistantDiscoveryESPConfig();
  for(auto &d:adminConfig.devices){
    if(d.type=="bot")publishHomeAssistantDiscoveryBotConfig(d.id,d.mac,home_assistant_use_opt_mode);
    else if(d.type=="curtain")publishHomeAssistantDiscoveryCurtainConfig(d.id,d.mac);
    else if(d.type=="meter")publishHomeAssistantDiscoveryMeterConfig(d.id,d.mac);
    else if(d.type=="contact")publishHomeAssistantDiscoveryContactConfig(d.id,d.mac);
    else if(d.type=="motion")publishHomeAssistantDiscoveryMotionConfig(d.id,d.mac);
    else if(d.type=="plug")publishHomeAssistantDiscoveryPlugConfig(d.id,d.mac,home_assistant_use_opt_mode);
  }
  adminCleaning=false;
  return adminCleanupOK;
}
static void adminRoutes() {
  const char *headers[]={"X-CSRF-Token"};server.collectHeaders(headers,1);
  server.on("/api/config",HTTP_GET,[](){
    if(!adminAuthorized())return;
    DynamicJsonDocument doc(12000),redacted(10000);admin::encode(redacted,adminConfig,false);
    doc["config"]=redacted.as<JsonObject>();doc["csrf"]=adminToken;doc["authEnabled"]=adminAuthEnabled;doc["trial"]=adminTrial;adminJson(200,doc);
  });
  server.on("/api/status",HTTP_GET,[](){
    if(!adminAuthorized())return;
    StaticJsonDocument<768> doc;doc["wifi"]=WiFi.status()==WL_CONNECTED;doc["mqtt"]=client.isConnected();doc["ip"]=WiFi.localIP().toString();doc["uptime"]=millis()/1000;doc["heap"]=ESP.getFreeHeap();doc["devices"]=adminConfig.devices.size();doc["ap"]=adminAP;doc["trial"]=adminTrial;doc["notice"]=adminNotice;adminJson(200,doc);
  });
  server.on("/api/config",HTTP_POST,[](){
    if(!adminAuthorized(true))return;
    if(!adminStorageOK){adminError(507,"Konfigurationsspeicher nicht verfuegbar.");return;}
    String body=server.arg("plain");if(body.length()>7000){adminError(413,"Konfiguration zu gross.");return;}
    DynamicJsonDocument doc(12000);admin::Config next;std::string error;
    if(deserializeJson(doc,body)){adminError(400,"JSON ungueltig.");return;}
    if(!admin::decode(doc.as<JsonVariantConst>(),adminConfig,next,error)){adminError(400,error.c_str());return;}
    const bool knownDiscovery=!adminConfig.devices.empty()||adminStore.getBool("hadBroker",false)||client.getConnectionEstablishedCount()>0;
    const bool cleanup=home_assistant_mqtt_discovery&&knownDiscovery&&admin::cleanupNeeded(adminConfig,next);
    if(cleanup&&!client.isConnected()){adminError(409,"Bisherigen MQTT-Broker verbinden, bevor Geraete, Broker oder Topic geaendert werden.");return;}
    if(cleanup&&(enableMesh||isMeshNode)){adminError(409,"Geraete-/Topic-Wechsel im Mesh erfordert koordinierte Discovery-Bereinigung.");return;}
    admin::encode(doc,next,true);String serialized;serializeJson(doc,serialized);
    if(doc.overflowed()||serialized.length()>3500){adminError(413,"Konfiguration zu gross fuer den sicheren Rollback-Speicher (3500 Bytes).");return;}
    // Save the previous settings on first use, including compiled-in credentials.
    if(!adminStore.isKey("active")){
      DynamicJsonDocument previous(12000);admin::encode(previous,adminConfig,true);String backup;serializeJson(previous,backup);
      if(previous.overflowed()||backup.length()>3500||adminStore.putString("active",backup)!=backup.length()){adminError(507,"Vorherige Konfiguration konnte nicht gesichert werden.");return;}
    }
    if(adminStore.isKey("trying")&&!adminStore.remove("trying")){adminError(507,"Teststatus konnte nicht zurueckgesetzt werden.");return;}
    if(adminStore.putString("pending",serialized)!=serialized.length()){adminError(507,"Konfiguration konnte nicht gespeichert werden.");return;}
    if(cleanup&&!adminCleanupDiscovery()){adminStore.remove("pending");discoveredDevices.clear();adminError(503,"Discovery-Bereinigung fehlgeschlagen. Erneut versuchen.");return;}
    client.publish(lastWill,"offline",true);
    StaticJsonDocument<256> result;result["message"]="Gespeichert. Neustart und Verbindungstest laufen. Nach etwa 90 Sekunden Seite neu laden; bei Fehlern gilt wieder die vorherige Konfiguration.";adminJson(200,result);adminRestartAt=millis()+1500;
  });
  server.on("/api/test",HTTP_POST,[](){
    if(!adminAuthorized(true))return;
    StaticJsonDocument<128> doc;if(deserializeJson(doc,server.arg("plain"))||!doc["id"].is<const char*>()){adminError(400,"Geraet fehlt.");return;}
    std::string id=doc["id"].as<std::string>();bool found=false;for(const auto &d:adminConfig.devices)if(d.id==id)found=true;
    if(!found){adminError(404,"Geraet zuerst speichern.");return;}
    if(!client.isConnected()||!initialScanComplete||commandQueue.isFull()){adminError(409,"Bridge noch nicht bereit oder Warteschlange voll.");return;}
    QueueCommand command;command.device=id;command.topic=ESPMQTTTopic+"/control";command.payload="REQUESTINFO";command.currentTry=1;command.priority=false;command.disconnectAfter=true;commandQueue.enqueue(command);
    server.send(202,"application/json","{}");
  });
  server.on("/api/password",HTTP_POST,[](){
    if(!adminAuthorized(true))return;
    StaticJsonDocument<256> doc;
    if(deserializeJson(doc,server.arg("plain"))||!doc["enabled"].is<bool>()){adminError(400,"Passwortschutz fehlt.");return;}
    const bool enabled=doc["enabled"].as<bool>();
    String value;
    if(enabled){
      if(!doc["password"].is<const char*>()){adminError(400,"Passwort fehlt.");return;}
      value=doc["password"].as<String>();
      if(value.length()<12||value.length()>63){adminError(400,"Admin-Passwort: 12–63 Zeichen.");return;}
    }
    if(!adminStorageOK){adminError(507,"Konfigurationsspeicher nicht verfuegbar.");return;}
    if(enabled){
      if(adminStore.putString("password",value)!=value.length()){adminError(507,"Passwort konnte nicht gespeichert werden.");return;}
      otaPass=value;
    }
    if(!adminStore.putBool("authEnabled",enabled)){adminError(507,"Passwortschutz konnte nicht gespeichert werden.");return;}
    adminAuthEnabled=enabled;server.send(200,"application/json","{}");
  });
  server.on("/api/restart",HTTP_POST,[](){if(!adminAuthorized(true))return;server.send(200,"application/json","{}");adminRestartAt=millis()+1000;});
}
static bool adminTick() {
  if(adminRestartAt){if(static_cast<int32_t>(millis()-adminRestartAt)>=0)ESP.restart();return true;}
  if(adminTrial){
    if(client.isConnected()){
      if(!adminConnectedAt)adminConnectedAt=millis();
      if(millis()-adminConnectedAt>=5000){
        if(adminStore.putString("active",adminCandidate)==adminCandidate.length()){
          adminStore.remove("pending");adminStore.remove("trying");adminCandidate="";adminTrial=false;adminNotice="Neue Konfiguration erfolgreich verbunden und uebernommen.";
          publishHomeAssistantDiscoveryESPConfig();
        } else {adminNotice="Speichern fehlgeschlagen; vorherige Konfiguration wird geladen.";adminRestartAt=millis()+1000;}
      }
    } else adminConnectedAt=0;
    if(adminTrial&&millis()-adminBootAt>=75000){adminNotice="Verbindungspruefung fehlgeschlagen. Rollback laeuft.";adminRestartAt=millis()+1000;}
    return true;
  }
  if(WiFi.status()!=WL_CONNECTED&&millis()-adminBootAt>60000)adminStartAP();
  if(adminStorageOK&&client.isConnected()&&!adminStore.getBool("hadBroker",false))adminStore.putBool("hadBroker",true);
  if(adminAP&&WiFi.status()==WL_CONNECTED&&millis()-adminAPAt>600000){WiFi.softAPdisconnect(true);WiFi.mode(WIFI_STA);adminAP=false;}
  return false;
}
// END GENERATED ADMIN RUNTIME

void setup () {
  Serial.begin(115200);
  pinMode(0, INPUT_PULLUP);
  adminLoad();

  if (ledHighEqualsON) {
    ledONValue = HIGH;
    ledOFFValue = LOW;
  }
  else {
    ledONValue = LOW;
    ledOFFValue = HIGH;
  }

  if (strcmp(hostForScan, hostForControl) != 0) {
    isMeshNode = true;
  }

  if (isMeshNode) {
    if (meshMeters) {
      meterTopic = ESPMQTTTopicMesh + "/meter/";
    }
    if (meshContactSensors) {
      contactTopic = ESPMQTTTopicMesh + "/contact/";
    }
    if (meshMotionSensors) {
      motionTopic = ESPMQTTTopicMesh + "/motion/";
    }
  }

  forceRescan = false;
  pinMode (LED_BUILTIN, OUTPUT);
  pinMode(0, INPUT_PULLUP);
  Serial.begin(115200);
  // Connect to WiFi network
  if (disableWiFiSleep) {
    WiFi.setSleep(false);
  }
  if (useStaticIP) {
    WiFi.config(staticIP, staticGateway, staticSubnet, staticPrimaryDNS, staticSecondaryDNS);
  }
  WiFi.mode(WIFI_STA);
  WiFi.setHostname(host);
  WiFi.setAutoReconnect(true);
  WiFi.begin(ssid, password);
  Serial.println("Admin setup: hold BOOT for 3 seconds now to open recovery WiFi.");
  delay(500);
  const uint32_t buttonAt=millis();
  while(digitalRead(0)==LOW && millis()-buttonAt<3000) delay(10);
  if(digitalRead(0)==LOW) {
    otaPass=adminRandom().substring(0,20);
    if(adminStorageOK) adminStore.putString("password",otaPass);
    adminStartAP();
  }
  if(adminConfig.ssid=="SSID" || adminConfig.ssid.empty()) adminStartAP();
  adminRoutes();
  printAString("");

  // Wait for connection
  /*while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    if (printSerialOutputForDebugging) {
      Serial.print(".");
    }
    }
    if (printSerialOutputForDebugging) {
    Serial.println("");
    Serial.print("Connected to ");
    Serial.println(ssid);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    }
  */
  /*use mdns for host name resolution*/
  /*  if (!MDNS.begin(host)) { //http://esp32.local
      if (printSerialOutputForDebugging) {
        Serial.println("Error setting up MDNS responder!");
      }
      while (1) {
        delay(1000);
      }
    }
    if (printSerialOutputForDebugging) {
      Serial.println("mDNS responder started");
    }*/
  /*return index page which is stored in serverIndex */
  server.on("/", HTTP_GET, []() {
    if(!adminAuthorized()) return;
    server.sendHeader("Cache-Control","no-store");
    server.sendHeader("Content-Encoding","gzip");
    server.sendHeader("X-Frame-Options","DENY");
    server.sendHeader("Content-Security-Policy","default-src 'self'; script-src 'self' 'unsafe-inline'; style-src 'self' 'unsafe-inline'; frame-ancestors 'none'; form-action 'self'");
    server.send_P(200,"text/html; charset=utf-8",reinterpret_cast<const char *>(adminPage),sizeof(adminPage));
  });
  static bool otaUploadAuthorized = false;
  static bool otaUploadSucceeded = false;
  /*handling uploading firmware file */
  server.on("/update", HTTP_POST, []() {
    server.sendHeader("Connection", "close");
    if (adminAuthEnabled && !server.authenticate("admin", otaPass.c_str())) {
      return server.requestAuthentication();
    }
    if(server.header("X-CSRF-Token") != adminToken) { adminError(403,"Sitzung ungueltig."); return; }
    const bool succeeded = otaUploadAuthorized && otaUploadSucceeded;
    otaUploadAuthorized = false;
    otaUploadSucceeded = false;
    server.send(succeeded ? 200 : 400, "text/plain", succeeded ? "OK" : "FAIL");
    if (succeeded) {
      ESP.restart();
    }
  }, []() {
    HTTPUpload& upload = server.upload();
    if (upload.status == UPLOAD_FILE_START) {
      otaUploadSucceeded = false;
      otaUploadAuthorized = !adminTrial && !adminRestartAt && server.header("X-CSRF-Token") == adminToken && (!adminAuthEnabled || server.authenticate("admin", otaPass.c_str()));
    }
    if (!otaUploadAuthorized) {
      return;
    }
    if (upload.status == UPLOAD_FILE_START) {
      if (printSerialOutputForDebugging) {
        Serial.printf("Update: %s\n", upload.filename.c_str());
      }
      if (!Update.begin(UPDATE_SIZE_UNKNOWN)) { //start with max available size
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_WRITE) {
      /* flashing firmware to ESP*/
      if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_ABORTED) {
      Update.abort();
      otaUploadAuthorized = false;
      otaUploadSucceeded = false;
    } else if (upload.status == UPLOAD_FILE_END) {
      if (Update.end(true)) {
        otaUploadSucceeded = true; //true to set the size to the current progress
        if (printSerialOutputForDebugging) {
          Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
        }
      } else {
        Update.printError(Serial);
      }
    }
  });
  server.begin();

  client.setMqttReconnectionAttemptDelay(10000); // milliseconds; MQTT failure must not reset WiFi
  client.enableLastWillMessage(lastWill, "offline", true);
  client.setKeepAlive(60);
  client.setMaxPacketSize(mqtt_packet_size);
  //client.enableMQTTPersistence();

  static std::map<std::string, std::string> allBotsTemp = {};
  static std::map<std::string, std::string> allCurtainsTemp = {};
  static std::map<std::string, std::string> allMetersTemp = {};
  static std::map<std::string, std::string> allContactSensorsTemp = {};
  static std::map<std::string, std::string> allMotionSensorsTemp = {};
  static std::map<std::string, std::string> allPlugsTemp = {};
  static std::map<std::string, std::string> allPasswordsTemp = {};
  static std::map<std::string, bool> botsSimulateONOFFinPRESSmodeTemp = {};
  static std::map<std::string, int> botsSimulatedOFFHoldTimesTemp = {};
  static std::map<std::string, int> botsSimulatedONHoldTimesTemp = {};
  NimBLEDevice::init("");
  bluetooth_mac_address = NimBLEDevice::getAddress().toString();

  std::map<std::string, std::string>::iterator it = allBots.begin();
  std::string anAddr;
  std::string aName;
  while (it != allBots.end())
  {
    aName = it->first;
    anAddr = it->second;
    std::transform(anAddr.begin(), anAddr.end(), anAddr.begin(), to_lower());
    std::replace( aName.begin(), aName.end(), ' ', '_');
    allSwitchbotsOpp[anAddr.c_str()] = aName;
    allSwitchbots[aName] = anAddr.c_str();
    deviceTypes[anAddr.c_str()] = botName;
    allBotsTemp[aName] = anAddr.c_str();
    NimBLEDevice::whiteListAdd(NimBLEAddress(anAddr, 1));
    it++;
  }
  allBots = allBotsTemp;

  it = allCurtains.begin();
  while (it != allCurtains.end())
  {
    aName = it->first;
    anAddr = it->second;
    std::transform(anAddr.begin(), anAddr.end(), anAddr.begin(), to_lower());
    std::replace( aName.begin(), aName.end(), ' ', '_');
    allSwitchbotsOpp[anAddr.c_str()] = aName;
    allSwitchbots[aName] = anAddr.c_str();
    deviceTypes[anAddr.c_str()] = curtainName;
    allCurtainsTemp[aName] = anAddr.c_str();
    NimBLEDevice::whiteListAdd(NimBLEAddress(anAddr, 1));
    it++;
  }
  allCurtains = allCurtainsTemp;

  it = allMeters.begin();
  while (it != allMeters.end())
  {
    aName = it->first;
    anAddr = it->second;
    std::transform(anAddr.begin(), anAddr.end(), anAddr.begin(), to_lower());
    std::replace( aName.begin(), aName.end(), ' ', '_');
    allSwitchbotsOpp[anAddr.c_str()] = aName;
    allSwitchbots[aName] = anAddr.c_str();
    deviceTypes[anAddr.c_str()] = meterName;
    allMetersTemp[aName] = anAddr.c_str();
    NimBLEDevice::whiteListAdd(NimBLEAddress(anAddr, 1));
    it++;
  }
  allMeters = allMetersTemp;

  it = allContactSensors.begin();
  while (it != allContactSensors.end())
  {
    aName = it->first;
    anAddr = it->second;
    std::transform(anAddr.begin(), anAddr.end(), anAddr.begin(), to_lower());
    std::replace( aName.begin(), aName.end(), ' ', '_');
    allSwitchbotsOpp[anAddr.c_str()] = aName;
    allSwitchbots[aName] = anAddr.c_str();
    deviceTypes[anAddr.c_str()] = contactName;
    allContactSensorsTemp[aName] = anAddr.c_str();
    NimBLEDevice::whiteListAdd(NimBLEAddress(anAddr, 1));
    it++;
  }
  allContactSensors = allContactSensorsTemp;

  it = allMotionSensors.begin();
  while (it != allMotionSensors.end())
  {
    aName = it->first;
    anAddr = it->second;
    std::transform(anAddr.begin(), anAddr.end(), anAddr.begin(), to_lower());
    std::replace( aName.begin(), aName.end(), ' ', '_');
    allSwitchbotsOpp[anAddr.c_str()] = aName;
    allSwitchbots[aName] = anAddr.c_str();
    deviceTypes[anAddr.c_str()] = motionName;
    allMotionSensorsTemp[aName] = anAddr.c_str();
    NimBLEDevice::whiteListAdd(NimBLEAddress(anAddr, 1));
    it++;
  }
  allMotionSensors = allMotionSensorsTemp;

  it = allPlugs.begin();
  while (it != allPlugs.end())
  {
    aName = it->first;
    anAddr = it->second;
    std::transform(anAddr.begin(), anAddr.end(), anAddr.begin(), to_lower());
    std::replace( aName.begin(), aName.end(), ' ', '_');
    allSwitchbotsOpp[anAddr.c_str()] = aName;
    allSwitchbots[aName] = anAddr.c_str();
    deviceTypes[anAddr.c_str()] = plugName;
    allPlugsTemp[aName] = anAddr.c_str();
    NimBLEDevice::whiteListAdd(NimBLEAddress(anAddr, 1));
    it++;
  }
  allPlugs = allPlugsTemp;

  it = allPasswords.begin();
  std::string aPass;
  while (it != allPasswords.end())
  {
    aName = it->first;
    aPass = it->second;
    std::replace( aName.begin(), aName.end(), ' ', '_');
    allPasswordsTemp[aName] = aPass.c_str();
    it++;
  }
  allPasswords = allPasswordsTemp;

  std::map<std::string, bool>::iterator itT = botsSimulateONOFFinPRESSmode.begin();
  bool aBool;
  while (itT != botsSimulateONOFFinPRESSmode.end())
  {
    aName = itT->first;
    aBool = itT->second;
    std::replace( aName.begin(), aName.end(), ' ', '_');
    botsSimulateONOFFinPRESSmodeTemp[aName] = aBool;
    itT++;
  }
  botsSimulateONOFFinPRESSmode = botsSimulateONOFFinPRESSmodeTemp;

  std::map<std::string, int>::iterator itY = botsSimulatedOFFHoldTimes.begin();
  int aInt;
  while (itY != botsSimulatedOFFHoldTimes.end())
  {
    aName = itY->first;
    aInt = itY->second;
    std::replace( aName.begin(), aName.end(), ' ', '_');
    botsSimulatedOFFHoldTimesTemp[aName] = aInt;
    itY++;
  }
  botsSimulatedOFFHoldTimes = botsSimulatedOFFHoldTimesTemp;

  itY = botsSimulatedONHoldTimes.begin();
  while (itY != botsSimulatedONHoldTimes.end())
  {
    aName = itY->first;
    aInt = itY->second;
    std::replace( aName.begin(), aName.end(), ' ', '_');
    botsSimulatedONHoldTimesTemp[aName] = aInt;
    itY++;
  }
  botsSimulatedONHoldTimes = botsSimulatedONHoldTimesTemp;

  Serial.println("Switchbot ESP32 starting...");
  if (!printSerialOutputForDebugging) {
    Serial.println("Set printSerialOutputForDebugging = true to see more Serial output");
  }
  printAString("Starting NimBLE Client");

  NimBLEDevice::setSecurityAuth(/*BLE_SM_PAIR_AUTHREQ_BOND | BLE_SM_PAIR_AUTHREQ_MITM |*/ BLE_SM_PAIR_AUTHREQ_SC);
  NimBLEDevice::setPower(ESP_PWR_LVL_P9);
  //NimBLEDevice::setScanFilterMode(2);
  pScan = NimBLEDevice::getScan();
  pScan->setAdvertisedDeviceCallbacks(new AdvertisedDeviceCallbacks());
  pScan->setInterval(70);
  pScan->setWindow(40);
  pScan->setDuplicateFilter(false);
  isActiveScan = true;
  pScan->setActiveScan(isActiveScan);
  pScan->setMaxResults(100);
  //pScan->setFilterPolicy(BLE_HCI_SCAN_FILT_USE_WL);

}

bool waitForScanEnd(unsigned long timeoutMs) {
  const unsigned long started = millis();
  while (pScan->isScanning()) {
    if (static_cast<uint32_t>(millis() - started) >= timeoutMs) {
      pScan->stop();
      isRescanning = false;
      publishStatus(ESPMQTTTopic, "errorScanTimeout");
      return false;
    }
    delay(10);
  }
  return true;
}

void rescan(int seconds) {
  lastRescan = millis();
  pScan->stop();
  if (!waitForScanEnd(2000UL)) {
    return;
  }
  allSwitchbotsScanned = {};
  //pScan->clearResults();
  lastRescan = millis();
  if (onlyPassiveScan && initialScanComplete) {
    isActiveScan = false;
    delay(50);
    addToPublish(ESPMQTTTopic.c_str(), "{\"status\":\"passivescanning\"}");
  }
  else {
    isActiveScan = true;
    delay(50);
    addToPublish(ESPMQTTTopic.c_str(), "{\"status\":\"activescanning\"}");
  }

  pScan->setActiveScan(isActiveScan);

  delay(50);
  if (ledOnScan) {
    digitalWrite(LED_BUILTIN, ledONValue);
  }
  isRescanning = true;
  if (!pScan->start(seconds, rescanEndedCB, true)) {
    isRescanning = false;
    publishStatus(ESPMQTTTopic, "errorScanStart");
  }
}

void scanForever() {
  //lastRescan = millis();
  pScan->stop();
  if (!waitForScanEnd(2000UL)) {
    return;
  }
  //allSwitchbotsScanned = {};
  //lastRescan = millis();

  if (onlyPassiveScan && initialScanComplete) {
    isActiveScan = false;
  }

  if (isActiveScan) {
    lastRescan = millis();
    isRescanning = true;
    addToPublish(ESPMQTTTopic.c_str(), "{\"status\":\"activescanning\"}");
  }
  else {
    addToPublish(ESPMQTTTopic.c_str(), "{\"status\":\"passivescanning\"}");
  }
  delay(50);
  pScan->setActiveScan(isActiveScan);
  if (ledOnScan) {
    digitalWrite(LED_BUILTIN, ledONValue);
  }
  if (!pScan->start(0, scanForeverEnded, true)) {
    isRescanning = false;
    publishStatus(ESPMQTTTopic, "errorScanStart");
  }
}

void rescanFind(std::string aMac) {
  if (isRescanning) {
    return;
  }
  pScan->stop();
  if (!waitForScanEnd(2000UL)) {
    return;
  }

  if (onlyPassiveScan && initialScanComplete) {
    isActiveScan = false;
    delay(100);
    addToPublish(ESPMQTTTopic.c_str(), "{\"status\":\"passivescanning\"}");
  }
  else {
    isActiveScan = true;
    delay(100);
    addToPublish(ESPMQTTTopic.c_str(), "{\"status\":\"activescanning\"}");
  }
  pScan->setActiveScan(isActiveScan);

  allSwitchbotsScanned = {};
  std::map<std::string, NimBLEAdvertisedDevice*>::iterator it = allSwitchbotsDev.begin();
  std::string anAddr;

  while (it != allSwitchbotsDev.end())
  {
    anAddr = it->first;
    if (anAddr != aMac) {
      allSwitchbotsScanned[anAddr] = it->second;
    }
    it++;
  }

  //allSwitchbotsDev.erase(aMac);
  //pScan->erase(NimBLEAddress(aMac));

  delay(50);
  if (ledOnScan) {
    digitalWrite(LED_BUILTIN, ledONValue);
  }
  if (!pScan->start(infoScanTime, scanEndedCB, true)) {
    isRescanning = false;
    publishStatus(ESPMQTTTopic, "errorScanStart");
  }
}

void getAllBotSettings() {
  if (client.isConnected() && initialScanComplete) {
    if (ledOnBootScan) {
      digitalWrite(LED_BUILTIN, ledONValue);
    }

    printAString("In all get bot settings...");
    gotSettings = true;
    std::map<std::string, std::string>::iterator itT = allBots.begin();
    std::string aDevice;
    std::string aMac;
    while (itT != allBots.end())
    {
      aDevice = itT->first;
      aMac = itT->second;
      bool hasFirmware = false;
      bool hasTimers = false;
      bool hasHold = false;
      bool hasInverted = false;
      std::map<std::string, int>::iterator itP = botHoldSecs.find(aMac);
      if (itP != botHoldSecs.end()) {
        hasHold = true;
      }
      std::map<std::string, int>::iterator itX = botNumTimers.find(aMac);
      if (itX != botNumTimers.end()) {
        hasTimers = true;
      }
      std::map<std::string, bool>::iterator itU = botInverteds.find(aMac);
      if (itU != botInverteds.end()) {
        hasInverted = true;
      }
      std::map<std::string, const char *>::iterator itZ = botFirmwares.find(aMac);
      if (itZ != botFirmwares.end()) {
        hasFirmware = true;
      }
      if ((!hasFirmware) || (!hasTimers) || (!hasHold) || (!hasInverted)) {
        printAString("get settings");

        processing = true;
        client.publish(ESPMQTTTopic.c_str(), "{\"status\":\"getsettings\"}");
        controlMQTT(aDevice, "REQUESTSETTINGS", true);
        client.publish(ESPMQTTTopic.c_str(), "{\"status\":\"idle\"}");

      }
      else {
        printAString("Skip settings");
      }
      processing = false;
      itT++;
    }
    if (ledOnBootScan) {
      digitalWrite(LED_BUILTIN, ledOFFValue);
    }
    printAString("Added all get bot settings...");
  }
}

void pushBotButtons() {
  if (espButtonPressed || botsControlledByESPButton.empty()) {
    return;
  }
  espButtonPressed = true;
  printAString("START pushBotButtons");

  if (ledOnESPButtonPress) {
    digitalWrite(LED_BUILTIN, ledONValue);
  }

  if (client.isConnected() && initialScanComplete) {
    std::map<std::string, bool>::iterator itT = botsControlledByESPButton.begin();
    while (itT != botsControlledByESPButton.end())
    {
      std::string aDevice = itT->first;
      std::replace(aDevice.begin(), aDevice.end(), ' ', '_');
      bool shouldCtrl = itT->second;
      std::map<std::string, std::string>::iterator itN = allBots.find(aDevice);
      if (shouldCtrl && itN != allBots.end() && !commandQueue.isFull()) {
        std::string anAddr = itN->second;
        std::map<std::string, bool>::iterator itP = botsInPressMode.find(anAddr);
        if (itP != botsInPressMode.end()) {
          std::string cmdPayload = "PRESS";
          std::map<std::string, bool>::iterator itE = botsSimulateONOFFinPRESSmode.find(aDevice);
          if (itE != botsSimulateONOFFinPRESSmode.end()) {
            std::map<std::string, bool>::iterator itF = botsSimulatedStates.find(aDevice);
            if (itF != botsSimulatedStates.end()) {
              cmdPayload = itF->second ? "OFF" : "ON";
            }
          }

          struct QueueCommand queueCommand;
          queueCommand.payload = cmdPayload;
          queueCommand.topic = ESPMQTTTopic + "/control";
          queueCommand.device = aDevice;
          queueCommand.disconnectAfter = true;
          queueCommand.priority = false;
          queueCommand.currentTry = 1;
          commandQueue.enqueue(queueCommand);
        }
      }
      itT++;
    }
  }

  if (ledOnESPButtonPress) {
    digitalWrite(LED_BUILTIN, ledOFFValue);
  }

  printAString("END pushBotButtons");
  espButtonPressed = false;
}

unsigned long retainStartTime = 0;
bool waitForRetained = true;
unsigned long lastWebServerReboot = 0;

void checkWebServer() {
  if ((millis() - lastWebServerReboot) > (30 * 1000 * 60)) {
    server.close();
    server.begin();
    lastWebServerReboot = millis();
  }
}

static uint64_t uptimeMillis = 0;
static uint32_t previousUptimeMillis = 0;

void loop () {
  const uint32_t now = millis();
  uptimeMillis += (uint32_t)(now - previousUptimeMillis);
  previousUptimeMillis = now;
  //printAString("START loop...");
  // WiFi recovery is independent of MQTT broker availability.
  static unsigned long lastWifiRetry = 0;
  if (WiFi.status() != WL_CONNECTED && millis() - lastWifiRetry >= 30000UL) {
    lastWifiRetry = millis();
    WiFi.reconnect();
  }
  client.loop();
  checkWebServer();
  server.handleClient();
  if(adminTick()) return;
  //printAString("at processAllAdvData...");
  processAllAdvData();
  //printAString("at publishLastwillOnline...");
  publishLastwillOnline();
  //printAString("at publishAllMQTT...");
  publishAllMQTT();
  //printAString("at processContactSensorTasks...");
  processContactSensorTasks();
  //printAString("at processMotionSensorTasks...");
  processMotionSensorTasks();
  //printAString("at loopcode...");
  if (waitForRetained && client.isConnected() && !manualDebugStartESP32WithMQTT) {
    if (retainStartTime == 0) {
      client.publish(ESPMQTTTopic.c_str(), "{\"status\":\"waiting\"}");
      retainStartTime = millis();
    }
    else if ((millis() - retainStartTime) > (waitForMQTTRetainMessages * 1000)) {
      waitForRetained = false;
    }
    /* if ((botFirmwares.size() >= allBots.size()) && (botInverteds.size() >= allBots.size()) && (botNumTimers.size() >= allBots.size()) && (botHoldSecs.size() >= allBots.size()) && (botsSimulatedStates.size() >= botsSimulateONOFFinPRESSmode.size()))
      {
       waitForRetained = false;
      }*/
  }

  if ((!initialScanComplete) && client.isConnected() && (!waitForResponse) && (!processing) && (!(pScan->isScanning())) && (!isRescanning) && (!waitForRetained) && !manualDebugStartESP32WithMQTT) {
    addToPublish(ESPMQTTTopic.c_str(), "{\"status\":\"activescanning\"}");
    isRescanning = true;
    isActiveScan = true;
    pScan->setActiveScan(isActiveScan);
    delay(50);
    if (!pScan->start(initialScan, initialScanEndedCB, true)) {
      isRescanning = false;
      publishStatus(ESPMQTTTopic, "errorScanStart");
    }
  }

  if (initialScanComplete && client.isConnected() && !manualDebugStartESP32WithMQTT) {
    if (includeSensorSystemInfo) {
      deviceInfoPolling();
    }
    if (isRescanning) {
      lastRescan = millis();
    }
    static bool buttonWasDown = false;
    static unsigned long lastButtonChange = 0;
    const bool buttonDown = digitalRead(0) == LOW;
    if (buttonDown != buttonWasDown && millis() - lastButtonChange >= 50UL) {
      buttonWasDown = buttonDown;
      lastButtonChange = millis();
      if (buttonDown && !processing) { pushBotButtons(); }
    }
    if ((!waitForResponse) && (!processing) && (!(pScan->isScanning())) && (!isRescanning)) {
      if (getSettingsOnBoot && !gotSettings ) {
        getAllBotSettings();
      }
    }

    if (((allContactSensors.size() + allMotionSensors.size() + allPlugs.size() + allMeters.size()) != 0) || alwaysActiveScan) {
      if ((!waitForResponse) && (!processing) && (!(pScan->isScanning())) && (!isRescanning)) {
        bool queueProcessed = false;
        queueProcessed = processQueue();
      }
      if (commandQueue.isEmpty() && (!waitForResponse) && (!processing) && (!(pScan->isScanning())) && (!isRescanning)) {
        if (!getSettingsOnBoot || (getSettingsOnBoot && gotSettings) ) {
          startForeverScan();
        }
      }
    }
    else {
      if ((!waitForResponse) && (!processing) && (!(pScan->isScanning())) && (!isRescanning)) {
        bool queueProcessed = false;
        queueProcessed = processQueue();
        if (commandQueue.isEmpty() && queueProcessed && !waitForResponse && !processing && !(pScan->isScanning()) && !isRescanning) {
          if (scanAfterControl || activeScanOnSchedule) {
            recurringScan();
          }
        }
      }
      if (commandQueue.isEmpty() && (!waitForResponse) && (!processing) && (!(pScan->isScanning())) && (!isRescanning)) {
        if (autoRescan || forceRescan) {
          recurringRescan();
        }
      }
    }
  }
  //printAString("END loop...");
}

void deviceInfoPolling() {
  if (lastSystemInfoPoll == 0 || ((millis() - lastSystemInfoPoll) >= ((unsigned long)systemInfoTime * 1000UL))) {
    lastSystemInfoPoll = millis();

    float uptimeHours = uptimeMillis / 3600000.0;
    addToPublish((esp32Topic + "/systemUptime/state").c_str(), String(uptimeHours, 2).c_str(), true);

    StaticJsonDocument<160> doc;
    char aBuffer[160];
    doc["uptime_ms"] = millis();
    doc["free_heap"] = ESP.getFreeHeap();
    doc["wifi_rssi"] = WiFi.RSSI();
    if (includeInfoBtMAC) {
      doc["bluetooth_mac_address"] = bluetooth_mac_address;
    }
    serializeJson(doc, aBuffer, sizeof(aBuffer));
    addToPublish((esp32Topic + "/systemUptime/json_attr").c_str(), aBuffer, true);
  }
}

void recurringRescan() {
  if (isRescanning) {
    lastRescan = millis();
    return;
  }

  if (((millis() - lastRescan) >= (rescanTime * 1000)) || forceRescan) {
    if (!processing && !(pScan->isScanning()) && !isRescanning) {
      rescan(initialScan);
    }
    else {
      unsigned long tempVal = (millis() - ((rescanTime * 1000) - 5000));
      if (tempVal < 0) {
        tempVal = 0;
      }
      lastRescan = tempVal;
    }
  }
}

void startForeverScan() {
  if (isRescanning) {
    lastRescan = millis();
    return;
  }
  if (!processing && !(pScan->isScanning()) && !isRescanning) {
    scanForever();
  }
}

bool shouldMQTTUpdateOrActiveScanForDevice(std::string & anAddr) {
  return (shouldMQTTUpdateForDevice(anAddr) || shouldActiveScanForDevice(anAddr)  );
}


bool shouldActiveScanForDevice(std::string & anAddr) {
  //yield();
  if (!client.isConnected()) {
    return false;
  }
  std::map<std::string, std::string>::iterator itB = allSwitchbotsOpp.find(anAddr);
  unsigned long lastActiveScanTime = 0;
  std::map<std::string, unsigned long>::iterator itX = lastActiveScanTimes.find(anAddr);
  if (itX != lastActiveScanTimes.end())
  {
    lastActiveScanTime = itX->second;
  }
  else
  {
    return true;
  }

  if ((millis() - lastActiveScanTime) >= (rescanTime * 1000)) {
    return true;
  }

  if (isBotDevice(itB->second)) {
    if ((millis() - lastActiveScanTime) >= (defaultBotActiveScanSecs * 1000)) {
      return true;
    }
  }
  else if (isCurtainDevice(itB->second)) {
    if ((millis() - lastActiveScanTime) >= (defaultCurtainActiveScanSecs * 1000)) {
      return true;
    }
  }
  else if (isMeterDevice(itB->second)) {
    if ((millis() - lastActiveScanTime) >= (defaultMeterActiveScanSecs * 1000)) {
      return true;
    }
  }
  else if (isContactDevice(itB->second)) {
    if ((millis() - lastActiveScanTime) >= (defaultContactActiveScanSecs * 1000)) {
      return true;
    }
  }
  else if (isMotionDevice(itB->second)) {
    if ((millis() - lastActiveScanTime) >= (defaultMotionActiveScanSecs * 1000)) {
      return true;
    }
  }
  else if (isPlugDevice(itB->second)) {
    if ((millis() - lastActiveScanTime) >= (defaultPlugActiveScanSecs * 1000)) {
      return true;
    }
  }

  if (isBotDevice(itB->second) || isCurtainDevice(itB->second)) {
    std::map<std::string, int>::iterator itS = botScanTime.find(itB->second);
    unsigned long lastTime;
    std::map<std::string, unsigned long>::iterator it = rescanTimes.find(anAddr);
    if (it != rescanTimes.end())
    {
      lastTime = it->second;
    }
    else {
      return false;
    }

    unsigned long scanTime = defaultBotScanAfterControlSecs;
    if (isCurtainDevice(itB->second)) {
      scanTime = defaultCurtainScanAfterControlSecs;
    }
    /* else if (isMeterDevice(itB->second)) {
       scanTime = defaultMeterMQTTUpdateSecs;
      }*/
    if (itS != botScanTime.end())
    {
      scanTime = itS->second;
    }
    if (isCurtainDevice(itB->second)) {
      if (scanWhileCurtainIsMoving && ((millis() - lastTime ) <= (scanTime * 1000))) {
        return true;
      }
    }
    else if (isBotDevice(itB->second)) {
      std::map<std::string, bool>::iterator itP = botsInPressMode.find(anAddr);
      if (itP != botsInPressMode.end())
      {
        std::map<std::string, int>::iterator itH = botHoldSecs.find(anAddr);
        if (itH != botHoldSecs.end())
        {
          int holdTimePlus = (itH->second) + defaultBotScanAfterControlSecs;
          if (holdTimePlus > scanTime) {
            scanTime =  holdTimePlus;
          }
        }
      }
    }

    if ((millis() - lastTime) >= (scanTime * 1000)) {
      return true;
    }
  }

  return false;
}

bool shouldMQTTUpdateForDevice(std::string & anAddr) {
  //yield();
  if (!client.isConnected()) {
    return false;
  }

  if (alwaysMQTTUpdate) {
    return true;
  }

  unsigned long lastUpdateTime = 0;
  std::map<std::string, unsigned long>::iterator itX = lastUpdateTimes.find(anAddr);
  if (itX != lastUpdateTimes.end())
  {
    lastUpdateTime = itX->second;
  }
  else
  {
    return true;
  }

  if ((millis() - lastUpdateTime) >= (rescanTime * 1000)) {
    return true;
  }
  std::map<std::string, std::string>::iterator itB = allSwitchbotsOpp.find(anAddr);
  std::map<std::string, std::string>::iterator itM = allMeters.find(itB->second);
  if (itM != allMeters.end())
  {
    if ((lastUpdateTime == 0 ) || ((millis() - lastUpdateTime) >= (defaultMeterMQTTUpdateSecs * 1000))) {
      return true;
    }
  }

  itM = allContactSensors.find(itB->second);
  if (itM != allContactSensors.end())
  {
    if ((lastUpdateTime == 0 ) || ((millis() - lastUpdateTime) >= (defaultContactMQTTUpdateSecs * 1000))) {
      return true;
    }
  }
  itM = allMotionSensors.find(itB->second);
  if (itM != allMotionSensors.end())
  {
    if ((lastUpdateTime == 0 ) || ((millis() - lastUpdateTime) >= (defaultMotionMQTTUpdateSecs * 1000))) {
      return true;
    }
  }

  itM = allPlugs.find(itB->second);
  if (itM != allPlugs.end())
  {
    if ((lastUpdateTime == 0 ) || ((millis() - lastUpdateTime) >= (defaultPlugMQTTUpdateSecs * 1000))) {
      return true;
    }
  }

  itM = allCurtains.find(itB->second);
  if (itM != allCurtains.end())
  {
    if ((lastUpdateTime == 0 ) || ((millis() - lastUpdateTime) >= (defaultCurtainMQTTUpdateSecs * 1000))) {
      return true;
    }
  }

  itM = allBots.find(itB->second);
  if (itM != allBots.end())
  {
    if ((lastUpdateTime == 0 ) || ((millis() - lastUpdateTime) >= (defaultBotMQTTUpdateSecs * 1000))) {
      return true;
    }
  }

  return false;
}

void recurringScan() {
  if ((millis() - lastScanCheck) >= 200) {
    std::string anAddr;
    std::map<std::string, std::string>::iterator itS = allSwitchbotsOpp.begin();
    while (itS != allSwitchbotsOpp.end())
    {
      anAddr = itS->first;
      bool shouldActiveScan = false;
      shouldActiveScan = shouldActiveScanForDevice(anAddr);
      if (shouldActiveScan) {
        if (onlyPassiveScan && initialScanComplete) {
          isActiveScan = false;
        }
        else {
          isActiveScan = true;
        }
        pScan->setActiveScan(isActiveScan);
        if (!processing && !(pScan->isScanning()) && !isRescanning) {
          rescanFind(anAddr);
          delay(100);
          std::map<std::string, unsigned long>::iterator itR = rescanTimes.find(anAddr);
          if (itR != rescanTimes.end())
          {
            std::map<std::string, std::string>::iterator itS = allSwitchbotsOpp.find(anAddr);
            std::string deviceName = itS->second.c_str();
            if (isCurtainDevice(deviceName)) {
              if ((millis() - (itR->second) ) > (defaultCurtainScanAfterControlSecs * 1000)) {
                rescanTimes.erase(anAddr);
              }
            }
            else {
              rescanTimes.erase(anAddr);
            }
          }
        }
      }
      itS++;
    }
    lastScanCheck = millis();
  }
}

/*void recurringMeterScan() {
  if (!allMeters.empty()) {
    std::map<std::string, std::string>::iterator it = allMeters.begin();
    std::string anAddr = it->second;
    while (it != allMeters.end())
    {
      bool shouldMQTTOrActiveScanUpdate = false;
      shouldMQTTOrActiveScanUpdate = shouldActiveScanForDevice(anAddr);
      if (shouldMQTTOrActiveScanUpdate) {
        rescanTimes[anAddr] = (((millis() - defaultMeterMQTTUpdateSecs) > 0) ? (millis() - defaultMeterMQTTUpdateSecs) : 0 ) ;
      }
      it++;
    }
  }
  }*/


static unsigned long commandStartedAt = 0;
static const unsigned long commandBudgetMs = 20000UL;

bool commandBudgetAvailable() {
  return static_cast<uint32_t>(millis() - commandStartedAt) < commandBudgetMs;
}

bool busyRetryAllowed(bool busy, bool enabled, int attempt) {
  return busy && enabled && attempt <= noResponseRetryAmount;
}

bool processRequest(std::string macAdd, std::string aName, const char * command, std::string deviceTopic, bool disconnectAfter) {
  commandStartedAt = millis();
  bool isSuccess = false;
  int count = 1;
  std::map<std::string, NimBLEAdvertisedDevice*>::iterator itS = allSwitchbotsDev.find(macAdd);
  NimBLEAdvertisedDevice* advDevice = nullptr;
  if (itS != allSwitchbotsDev.end())
  {
    advDevice =  itS->second;
  }
  bool shouldContinue = (advDevice == nullptr);
  while (shouldContinue && commandBudgetAvailable()) {
    if (count > 3) {
      shouldContinue = false;
    }
    else {
      if (pScan->isScanning()) {
        pScan->stop();
        if (!waitForScanEnd(2000UL)) {
          return false;
        }
      }
      if (ledOnScan) {
        digitalWrite(LED_BUILTIN, ledONValue);
      }
      overrideScan = true;
      rescanFind(macAdd);
      //pScan->start(10 * count, scanEndedCB, true);
      delay(100);
      if (!waitForScanEnd((unsigned long)infoScanTime * 1000UL + 2000UL)) {
        overrideScan = false;
        return false;
      }
      overrideScan = false;
      itS = allSwitchbotsDev.find(macAdd);
      if (itS != allSwitchbotsDev.end())
      {
        advDevice =  itS->second;
      }
      shouldContinue = (advDevice == nullptr);
      count++;
    }
  }
  if (advDevice == nullptr)
  {
    StaticJsonDocument<100> doc;
    char aBuffer[100];
    doc["id"] = aName.c_str();
    doc["status"] = "errorLocatingDevice";
    serializeJson(doc, aBuffer, sizeof(aBuffer));
    addToPublish((deviceTopic + "/status").c_str(), aBuffer);
  }
  else {
    isSuccess = sendToDevice(advDevice, aName, command, deviceTopic, disconnectAfter);
    if (!isSuccess && !commandBudgetAvailable()) {
      publishStatus(deviceTopic + "/status", "errorCommandTimeout");
    }
  }
  return isSuccess;
}

bool waitToProcess(QueueCommand aCommand) {
  bool wait = false;
  unsigned long waitTimeLeft = 0;

  std::map<std::string, bool>::iterator itP = botsToWaitFor.find(aCommand.device);
  if (itP != botsToWaitFor.end())
  {
    if (!aCommand.priority) {
      return true;
    }
  }

  if (waitBetweenControl) {
    if (aCommand.payload != "REQUESTINFO" && aCommand.payload != "GETINFO" && (aCommand.topic != (ESPMQTTTopic + "/requestInfo"))) {
      std::string anAddr;
      std::map<std::string, std::string>::iterator itY = allSwitchbots.find(aCommand.device.c_str());
      if (itY != allSwitchbots.end())
      {
        anAddr = itY->second;
        std::map<std::string, unsigned long>::iterator itZ = lastCommandSent.find(anAddr);
        if (itZ != lastCommandSent.end())
        {
          wait = true;
          unsigned long lastTime = itZ->second;
          int waitForSecs = 0;
          std::string deviceName;
          std::map<std::string, std::string>::iterator itK = deviceTypes.find(anAddr);
          if (itK != deviceTypes.end())
          {
            deviceName = itK->second.c_str();
          }
          if (deviceName == botName) {
            waitForSecs = defaultBotWaitTime;
          }
          else if (deviceName == curtainName) {
            waitForSecs = defaultCurtainWaitTime;
          }

          std::map<std::string, bool>::iterator itP = botsInPressMode.find(anAddr);
          if (itP != botsInPressMode.end())
          {
            std::map<std::string, int>::iterator itH = botHoldSecs.find(anAddr);
            if (itH != botHoldSecs.end())
            {
              int holdTimePlus = (itH->second) + defaultBotWaitTime;
              if (holdTimePlus > waitForSecs) {
                waitForSecs =  holdTimePlus;
              }
            }
          }
          std::map<std::string, int>::iterator itV = botWaitBetweenControlTimes.find(aCommand.device.c_str());
          if (itV != botWaitBetweenControlTimes.end())
          {
            int waitTimePlus = (itV->second);
            if (waitTimePlus > waitForSecs) {
              waitForSecs =  waitTimePlus;
            }
          }
          if ((millis() - lastTime) >= (waitForSecs * 1000)) {
            wait = false;

          }
          else {
            waitTimeLeft = (waitForSecs * 1000) - (millis() - lastTime);
          }
        }
      }
    }
  }
  if (wait) {
    printAString("Control for device: ");
    printAString(aCommand.device.c_str());
    printAString(" will wait ");
    printAString(waitTimeLeft);
    printAString(" millisecondSeconds");
  }
  return wait;
}

class ProcessingScope {
  bool previous;
public:
  ProcessingScope() : previous(processing) { processing = true; }
  ~ProcessingScope() { processing = previous; }
};

bool processQueue() {
  ProcessingScope processingScope;
  struct QueueCommand aCommand;
  if (!commandQueue.isEmpty()) {
    bool disconnectAfter = true;
    if (ledOnCommand) {
      digitalWrite(LED_BUILTIN, ledONValue);
    }
    if (!waitForResponse) {
      bool requeue = false;
      bool skip = false;
      aCommand = commandQueue.getHead();
      bool getSettingsAfter = false;
      std::string requestDevice = aCommand.device.c_str();
      std::string deviceStateTopic = botTopic + aCommand.device + "/state";
      if ((aCommand.topic == ESPMQTTTopic + "/rescan") && isRescanning) {
        commandQueue.dequeue();
      }
      else {
        if ( pScan->isScanning() || isRescanning ) {
          return false;
        }
        processing = true;
        printAString("Received something on ");
        printAString(aCommand.topic.c_str());
        printAString(aCommand.device.c_str());
        if (aCommand.topic == ESPMQTTTopic + "/control") {
          if (aCommand.disconnectAfter == false) {
            disconnectAfter = false;
          };
          processing = true;
          bool boolState = false;

          if (waitToProcess(aCommand)) {
            std::map<std::string, bool>::iterator itP = botsToWaitFor.find(aCommand.device);
            if (itP == botsToWaitFor.end())
            {
              botsToWaitFor[aCommand.device] = true;
              aCommand.priority = true;
            }
            commandQueue.enqueue(aCommand);
          }
          else {
            bool skipProcess = false;
            if ((strcmp(aCommand.payload.c_str(), "OFF") == 0) || (strcmp(aCommand.payload.c_str(), "ON") == 0)) {
              if (isBotDevice(aCommand.device.c_str()))
              {
                std::map<std::string, std::string>::iterator itN = allBots.find(aCommand.device);
                std::string anAddr = itN->second;
                std::transform(anAddr.begin(), anAddr.end(), anAddr.begin(), to_lower());
                std::map<std::string, bool>::iterator itP = botsInPressMode.find(anAddr);
                std::map<std::string, bool>::iterator itE = botsSimulateONOFFinPRESSmode.find(aCommand.device);
                if (itP != botsInPressMode.end()) {
                  if (itE != botsSimulateONOFFinPRESSmode.end()) {
                    std::map<std::string, bool>::iterator itF = botsSimulatedStates.find(aCommand.device);
                    if (itF != botsSimulatedStates.end())
                    {
                      boolState = itF->second;
                      if (boolState && (strcmp(aCommand.payload.c_str(), "ON") == 0)) {
                        skipProcess = true;
                        addToPublish(deviceStateTopic.c_str(), "ON", true);
                        std::map<std::string, bool>::iterator itP = botsToWaitFor.find(aCommand.device);
                        if (itP != botsToWaitFor.end())
                        {
                          botsToWaitFor.erase(aCommand.device);
                        }
                      }
                      else if (!boolState && (strcmp(aCommand.payload.c_str(), "OFF") == 0)) {
                        skipProcess = true;
                        addToPublish(deviceStateTopic.c_str(), "OFF", true);
                        std::map<std::string, bool>::iterator itP = botsToWaitFor.find(aCommand.device);
                        if (itP != botsToWaitFor.end())
                        {
                          botsToWaitFor.erase(aCommand.device);
                        }
                      }
                    }
                  }
                  else {
                    if (strcmp(aCommand.payload.c_str(), "OFF") == 0) {
                      skipProcess = true;
                      std::map<std::string, bool>::iterator itP = botsToWaitFor.find(aCommand.device);
                      if (itP != botsToWaitFor.end())
                      {
                        botsToWaitFor.erase(aCommand.device);
                      }
                    }
                  }
                }
              }
            }
            if (!skipProcess) {
              if (ledOnCommand) {
                digitalWrite(LED_BUILTIN, ledONValue);
              }
              noResponse = true;
              bool shouldContinue = true;
              unsigned long timeSent = millis();
              bool isSuccess = false;
              if (isBotDevice(aCommand.device.c_str()))
              {

                String tempPayload = aCommand.payload.c_str();
                int dotIndex = tempPayload.indexOf(".");
                if (dotIndex >= 0) {
                  tempPayload.remove(dotIndex, tempPayload.length() - 1);
                }
                bool isNum = is_number(tempPayload.c_str());

                if (isNum) {
                  isSuccess = controlMQTT(aCommand.device, tempPayload.c_str(), false);
                }
                else {
                  isSuccess = controlMQTT(aCommand.device, aCommand.payload, disconnectAfter);
                }
                while (noResponse && shouldContinue )
                {
                  delay(1); // Let the BLE and WiFi tasks run while awaiting notification.
                  waitForResponse = true;
                  //if (printSerialOutputForDebugging) {Serial.println("waiting for response...");}
                  if ((millis() - timeSent) > (waitForResponseSec * 1000)) {
                    shouldContinue = false;
                  }
                }
                if (noResponse && assumeNoResponseMeansSuccess && !retryBotActionNoResponse && isSuccess) {
                  std::string deviceAssumedStateTopic = botTopic + aCommand.device + "/state";
                  std::map<std::string, bool>::iterator itE = botsSimulateONOFFinPRESSmode.find(aCommand.device);
                  if (itE != botsSimulateONOFFinPRESSmode.end()) {
                    if ((strcmp(aCommand.payload.c_str(), "ON") == 0) || (strcmp(aCommand.payload.c_str(), "OFF") == 0)) {
                      if (strcmp(aCommand.payload.c_str(), "OFF") == 0) {
                        botsSimulatedStates[aCommand.device] = false;
                        addToPublish(deviceStateTopic.c_str(), "OFF", true);
                        addToPublish(deviceAssumedStateTopic.c_str(), "OFF", true);
                      }
                      else if (strcmp(aCommand.payload.c_str(), "ON") == 0) {
                        botsSimulatedStates[aCommand.device] = true;
                        addToPublish(deviceStateTopic.c_str(), "ON", true);
                        addToPublish(deviceAssumedStateTopic.c_str(), "ON", true);
                      }
                      else if (strcmp(aCommand.payload.c_str(), "PRESS") == 0) {
                        botsSimulatedStates[aCommand.device] = !(botsSimulatedStates[aCommand.device]);
                        if (botsSimulatedStates[aCommand.device]) {
                          addToPublish(deviceStateTopic.c_str(), "ON", true);
                          addToPublish(deviceAssumedStateTopic.c_str(), "ON", true);
                        }
                        else {
                          addToPublish(deviceStateTopic.c_str(), "OFF", true);
                          addToPublish(deviceAssumedStateTopic.c_str(), "OFF", true);
                        }
                      }
                    }
                  }
                }
                std::map<std::string, std::string>::iterator itN = allBots.find(aCommand.device);
                std::string anAddr = itN->second;
                std::transform(anAddr.begin(), anAddr.end(), anAddr.begin(), to_lower());
                NimBLEClient* pClient = nullptr;
                if (NimBLEDevice::getClientListSize()) {
                  pClient = NimBLEDevice::getClientByPeerAddress(anAddr);
                  if (pClient) {
                    if (pClient->isConnected()) {
                      unsubscribeToNotify(pClient);
                      if (disconnectAfter) {
                        pClient->disconnect();
                      }
                    }
                  }
                }

                if (isNum && !lastCommandWasBusy) {
                  getSettingsAfter = true;
                }
                if (busyRetryAllowed(lastCommandWasBusy, retryBotOnBusy, aCommand.currentTry)) {
                  requeue = true;
                  botsToWaitFor[aCommand.device] = true;
                  lastCommandWasBusy = false;
                  getSettingsAfter = false;

                  lastCommandSent[anAddr] = 0;
                }
                else if ((retryBotActionNoResponse && noResponse && (aCommand.currentTry <= noResponseRetryAmount)) || (retryBotSetNoResponse && noResponse && (aCommand.currentTry <= noResponseRetryAmount) && ((strcmp(aCommand.payload.c_str(), "REQUESTSETTINGS") == 0) || (strcmp(aCommand.payload.c_str(), "GETSETTINGS") == 0)
                         || (strcmp(aCommand.payload.c_str(), "MODEPRESS") == 0) || (strcmp(aCommand.payload.c_str(), "MODEPRESSINV") == 0) || (strcmp(aCommand.payload.c_str(), "MODESWITCH") == 0) || (strcmp(aCommand.payload.c_str(), "MODESWITCHINV") == 0) || isNum ))) {
                  printAString("current retry...");
                  printAString(aCommand.currentTry);

                  requeue = true;
                  botsToWaitFor[aCommand.device] = true;
                  lastCommandWasBusy = false;
                  getSettingsAfter = false;
                }
                else {
                  std::map<std::string, bool>::iterator itP = botsToWaitFor.find(aCommand.device);
                  if (itP != botsToWaitFor.end())
                  {
                    botsToWaitFor.erase(aCommand.device);
                  }
                }
                publishRecentFailures(noResponse, aCommand.currentTry);
                waitForResponse = false;
                noResponse = false;
              }


              else if (isPlugDevice(aCommand.device.c_str()))
              {

                String tempPayload = aCommand.payload.c_str();
                isSuccess = controlMQTT(aCommand.device, aCommand.payload, disconnectAfter);

                while (noResponse && shouldContinue )
                {
                  delay(1); // Let the BLE and WiFi tasks run while awaiting notification.
                  waitForResponse = true;
                  //if (printSerialOutputForDebugging) {Serial.println("waiting for response...");}
                  if ((millis() - timeSent) > (waitForResponseSec * 1000)) {
                    shouldContinue = false;
                  }
                }

                std::map<std::string, std::string>::iterator itN = allPlugs.find(aCommand.device);
                std::string anAddr = itN->second;
                std::transform(anAddr.begin(), anAddr.end(), anAddr.begin(), to_lower());
                NimBLEClient* pClient = nullptr;
                if (NimBLEDevice::getClientListSize()) {
                  pClient = NimBLEDevice::getClientByPeerAddress(anAddr);
                  if (pClient) {
                    if (pClient->isConnected()) {
                      unsubscribeToNotify(pClient);
                      if (disconnectAfter) {
                        pClient->disconnect();
                      }
                    }
                  }
                }

                if (busyRetryAllowed(lastCommandWasBusy, retryPlugOnBusy, aCommand.currentTry)) {
                  requeue = true;
                  botsToWaitFor[aCommand.device] = true;
                  lastCommandWasBusy = false;
                  getSettingsAfter = false;

                  lastCommandSent[anAddr] = 0;
                }
                else if ((retryPlugActionNoResponse && noResponse && (aCommand.currentTry <= noResponseRetryAmount))) {
                  printAString("current retry...");
                  printAString(aCommand.currentTry);

                  requeue = true;
                  botsToWaitFor[aCommand.device] = true;
                  lastCommandWasBusy = false;
                  getSettingsAfter = false;
                }
                else {
                  std::map<std::string, bool>::iterator itP = botsToWaitFor.find(aCommand.device);
                  if (itP != botsToWaitFor.end())
                  {
                    botsToWaitFor.erase(aCommand.device);
                  }
                }
                publishRecentFailures(noResponse, aCommand.currentTry);
                waitForResponse = false;
                noResponse = false;
              }

              else if (isCurtainDevice(aCommand.device.c_str()))
              {
                controlMQTT(aCommand.device, aCommand.payload, disconnectAfter);
                std::string anAddr;

                while (noResponse && shouldContinue )
                {
                  delay(1); // Let the BLE and WiFi tasks run while awaiting notification.
                  waitForResponse = true;
                  printAString("waiting for response...");

                  if ((millis() - timeSent) > (waitForResponseSec * 1000)) {
                    shouldContinue = false;
                  }
                }
                std::map<std::string, std::string>::iterator itN = allCurtains.find(aCommand.device);
                anAddr = itN->second;
                std::transform(anAddr.begin(), anAddr.end(), anAddr.begin(), to_lower());
                NimBLEClient* pClient = nullptr;
                if (NimBLEDevice::getClientListSize()) {
                  pClient = NimBLEDevice::getClientByPeerAddress(anAddr);
                  if (pClient) {
                    if (pClient->isConnected()) {
                      unsubscribeToNotify(pClient);
                      if (disconnectAfter) {
                        pClient->disconnect();
                      }
                    }
                  }
                }
                if (busyRetryAllowed(lastCommandWasBusy, retryCurtainOnBusy, aCommand.currentTry)) {
                  requeue = true;
                  botsToWaitFor[aCommand.device] = true;
                  lastCommandWasBusy = false;
                  lastCommandSent[anAddr] = 0;
                }
                else if (retryCurtainNoResponse && noResponse && (aCommand.currentTry <= noResponseRetryAmount)) {
                  printAString("current retry...");
                  printAString(aCommand.currentTry);

                  requeue = true;
                  botsToWaitFor[aCommand.device] = true;
                  lastCommandWasBusy = false;
                }
                else {
                  std::map<std::string, bool>::iterator itP = botsToWaitFor.find(aCommand.device);
                  if (itP != botsToWaitFor.end())
                  {
                    botsToWaitFor.erase(aCommand.device);
                  }
                }

                publishRecentFailures(noResponse, aCommand.currentTry);
                waitForResponse = false;
                noResponse = false;
              }
            }

            std::map<std::string, bool>::iterator itE = botsSimulateONOFFinPRESSmode.find(aCommand.device);
            if (itE != botsSimulateONOFFinPRESSmode.end())
            {
              std::map<std::string, bool>::iterator itF = botsSimulatedStates.find(aCommand.device);
              if (itF != botsSimulatedStates.end())
              {
                bool boolState = itF->second;
                if (boolState && (strcmp(aCommand.payload.c_str(), "OFF") == 0))  {
                  addToPublish(deviceStateTopic.c_str(), "ON", true);
                }
                else if (!boolState && (strcmp(aCommand.payload.c_str(), "ON") == 0)) {
                  addToPublish(deviceStateTopic.c_str(), "OFF", true);
                }
              }
            }
          }
        }
        else if (aCommand.topic == ESPMQTTTopic + "/requestInfo") {
          if (ledOnCommand) {
            digitalWrite(LED_BUILTIN, ledONValue);
          }
          requestInfoMQTT(aCommand.payload);
        }
        else if (aCommand.topic == ESPMQTTTopic + "/rescan") {
          if (ledOnCommand) {
            digitalWrite(LED_BUILTIN, ledONValue);
          }
          rescanMQTT(aCommand.payload);
        }
        if (requeue) {
          aCommand.currentTry = aCommand.currentTry + 1;
          aCommand.priority = true;
          commandQueue.enqueue(aCommand);
        }

        lastCommandWasBusy = false;
        if (getSettingsAfter && !skip) {
          // Return to loop() between requests so MQTT keepalive and OTA are serviced.
          if (!commandQueue.isFull()) {
            QueueCommand followup;
            followup.payload = "REQUESTSETTINGS";
            followup.topic = ESPMQTTTopic + "/control";
            followup.device = requestDevice;
            followup.disconnectAfter = true;
            followup.priority = false;
            followup.currentTry = 1;
            commandQueue.enqueue(followup);
          }
          else {
            publishStatus(ESPMQTTTopic, "errorQueueFull");
          }
        }
        commandQueue.dequeue();
      }
    }
    client.publish(ESPMQTTTopic.c_str(), "{\"status\":\"idle\"}");
  }
  if (ledOnCommand) {
    digitalWrite(LED_BUILTIN, ledOFFValue);
  }
  processing = false;
  return true;
}

bool sendToDevice(NimBLEAdvertisedDevice * advDevice, std::string & aName, const char * command, std::string & deviceTopic, bool disconnectAfter) {
  bool isSuccess = false;
  NimBLEAdvertisedDevice* advDeviceToUse = advDevice;
  std::string addr = advDeviceToUse->getAddress().toString();
  //std::transform(addr.begin(), addr.end(), addr.begin(), ::toupper);
  addr = addr.c_str();
  std::string deviceStateTopic = deviceTopic + "/state";
  std::string deviceStatusTopic = deviceTopic + "/status";

  if ((advDeviceToUse != nullptr) && (advDeviceToUse != NULL))
  {
    char aBuffer[100];
    StaticJsonDocument<100> doc;
    //    doc["id"] = aName.c_str();
    if (strcmp(command, "requestInfo") == 0 || strcmp(command, "REQUESTINFO") == 0 || strcmp(command, "GETINFO") == 0) {
      isSuccess = requestInfo(advDeviceToUse);
      if (!isSuccess) {
        doc["status"] = "errorRequestInfo";
        doc["command"] = command;
        serializeJson(doc, aBuffer, sizeof(aBuffer));
        client.publish(deviceStatusTopic.c_str(),  aBuffer);
      }
      return isSuccess;
    }
    bool isConnected = false;
    int count = 0;
    bool shouldContinue = true;
    while (shouldContinue && commandBudgetAvailable()) {
      if (count > 1) {
        delay(50);
      }
      isConnected = connectToServer(advDeviceToUse);
      count++;
      if (isConnected) {
        shouldContinue = false;
        doc["status"] = "connected";
        doc["command"] = command;
        serializeJson(doc, aBuffer, sizeof(aBuffer));
        client.publish(deviceStatusTopic.c_str(),  aBuffer);
      }
      else {
        if (count > tryConnecting) {
          shouldContinue = false;
          doc["status"] = "errorConnect";
          doc["command"] = command;
          serializeJson(doc, aBuffer, sizeof(aBuffer));
          client.publish(deviceStatusTopic.c_str(),  aBuffer);
        }
      }
    }
    count = 0;
    if (isConnected) {
      shouldContinue = true;
      while (shouldContinue && commandBudgetAvailable()) {
        if (count > 1) {
          delay(50);
        }
        isSuccess = sendCommand(advDeviceToUse, command, count, disconnectAfter);
        count++;
        if (isSuccess) {
          delay(100);
          shouldContinue = false;
          if (!lastCommandSentPublished) {
            StaticJsonDocument<100> doc;
            char aBuffer[100];
            doc["status"] = "commandSent";
            doc["command"] = command;
            serializeJson(doc, aBuffer, sizeof(aBuffer));
            client.publish(deviceStatusTopic.c_str(), aBuffer);
          }
          lastCommandSentPublished = false;
          if (strcmp(command, "REQUESTSETTINGS") != 0 && strcmp(command, "GETSETTINGS") != 0) {

            String tempPayload = command;
            int dotIndex = tempPayload.indexOf(".");
            if (dotIndex >= 0) {
              tempPayload.remove(dotIndex, tempPayload.length() - 1);
            }
            bool isNum = is_number(tempPayload.c_str());
            bool scanAfterNum = true;
            std::string aDevice = "";
            std::map<std::string, std::string>::iterator itI = allSwitchbotsOpp.find(addr);
            aDevice = itI->second;
            if (isNum && isBotDevice(aDevice)) {
              scanAfterNum = false;
            }
            if (isNum) {
              int aVal;

              sscanf(tempPayload.c_str(), "%d", &aVal);
              if (aVal < 0) {
                aVal = 0;
              }
              else if (aVal > 100) {
                aVal = 100;
              }
              if (isCurtainDevice(aDevice)) {
                std::string devicePosTopic = deviceTopic + "/position";
                StaticJsonDocument<50> docPos;
                char aBuffer[100];
                docPos["pos"] = aVal;
                serializeJson(docPos, aBuffer, sizeof(aBuffer));
                addToPublish(devicePosTopic.c_str(), aBuffer);
              }
            }
            else {
              std::map<std::string, bool>::iterator itP = botsInPressMode.find(addr);
              std::map<std::string, bool>::iterator itE = botsSimulateONOFFinPRESSmode.find(aDevice);
              if (itP != botsInPressMode.end() && itE == botsSimulateONOFFinPRESSmode.end())
              {
                addToPublish(deviceStateTopic.c_str(), "OFF", true);
              }
              else {
                addToPublish(deviceStateTopic.c_str(), command, true);
              }
            }
            if (scanAfterControl && scanAfterNum) {
              rescanTimes[addr] = millis();
            }
          }
        }
        else {
          if (count > trySending) {
            shouldContinue = false;
            doc["status"] = "errorCommand";
            doc["command"] = command;
            serializeJson(doc, aBuffer, sizeof(aBuffer));
            client.publish(deviceStatusTopic.c_str(),  aBuffer);
          }
        }
      }
    }
  }
  printAString("Done sendCommand...");

  return isSuccess;
}

bool is_number(const std::string & s)
{
  std::string::const_iterator it = s.begin();
  while (it != s.end() && std::isdigit(*it)) ++it;
  return !s.empty() && it == s.end();
}

bool controlMQTT(std::string & device, std::string payload, bool disconnectAfter) {
  client.publish(ESPMQTTTopic.c_str(), "{\"status\":\"controlling\"}");
  bool isSuccess = false;
  processing = true;
  printAString("Processing Control MQTT...");

  std::string deviceAddr = "";
  std::string deviceTopic;
  std::string anAddr;

  printAString("Device: ");
  printAString(device.c_str());
  printAString("Device value: ");
  printAString(payload.c_str());

  std::map<std::string, std::string>::iterator itS = allBots.find(device.c_str());
  if (itS != allBots.end())
  {
    anAddr = itS->second;
    std::transform(anAddr.begin(), anAddr.end(), anAddr.begin(), to_lower());
    deviceAddr = anAddr.c_str();
    deviceTopic = botTopic;
  }
  itS = allCurtains.find(device.c_str());
  if (itS != allCurtains.end())
  {
    anAddr = itS->second;
    std::transform(anAddr.begin(), anAddr.end(), anAddr.begin(), to_lower());
    deviceAddr = anAddr.c_str();
    deviceTopic = curtainTopic;
  }
  itS = allMeters.find(device.c_str());
  if (itS != allMeters.end())
  {
    anAddr = itS->second;
    std::transform(anAddr.begin(), anAddr.end(), anAddr.begin(), to_lower());
    deviceAddr = anAddr.c_str();
    deviceTopic = meterTopic;
  }
  itS = allContactSensors.find(device.c_str());
  if (itS != allContactSensors.end())
  {
    anAddr = itS->second;
    std::transform(anAddr.begin(), anAddr.end(), anAddr.begin(), to_lower());
    deviceAddr = anAddr.c_str();
    deviceTopic = contactTopic;
  }
  itS = allMotionSensors.find(device.c_str());
  if (itS != allMotionSensors.end())
  {
    anAddr = itS->second;
    std::transform(anAddr.begin(), anAddr.end(), anAddr.begin(), to_lower());
    deviceAddr = anAddr.c_str();
    deviceTopic = motionTopic;
  }

  itS = allPlugs.find(device.c_str());
  if (itS != allPlugs.end())
  {
    anAddr = itS->second;
    std::transform(anAddr.begin(), anAddr.end(), anAddr.begin(), to_lower());
    deviceAddr = anAddr.c_str();
    deviceTopic = plugTopic;
  }

  bool diffDevice = false;
  if (lastDeviceControlled != deviceAddr) {
    diffDevice = true;
  }
  lastDeviceControlled = deviceAddr;
  if (lastDeviceControlled != "") {
    if (diffDevice) {
      NimBLEClient* pClient = nullptr;
      if (NimBLEDevice::getClientListSize()) {
        pClient = NimBLEDevice::getClientByPeerAddress(lastDeviceControlled);
        if (pClient) {
          if (pClient->isConnected()) {
            unsubscribeToNotify(pClient);
            pClient->disconnect();
          }
        }
      }
    }


    String tempPayload = payload.c_str();
    StaticJsonDocument<200> payloadDoc;
    DeserializationError payloadError = deserializeJson(payloadDoc, tempPayload);
    if (!payloadError) {
      if (payloadDoc.containsKey("position")) {
        tempPayload = payloadDoc["position"].as<String>();
      }
      else if (payloadDoc.containsKey("pos")) {
        tempPayload = payloadDoc["pos"].as<String>();
      }
      else if (payloadDoc.containsKey("level")) {
        tempPayload = payloadDoc["level"].as<String>();
      }
      else if (payloadDoc.containsKey("switchcmd")) {
        tempPayload = payloadDoc["switchcmd"].as<String>();
      }
    }
    int dotIndex = tempPayload.indexOf(".");
    if (dotIndex >= 0) {
      tempPayload.remove(dotIndex, tempPayload.length() - 1);
    }

    bool isNum = is_number(tempPayload.c_str());
    deviceTopic = deviceTopic + device;
    if (isNum) {
      payload = tempPayload.c_str();
      int aVal;
      sscanf(payload.c_str(), "%d", &aVal);
      if (aVal < 0) {
        payload = "0";
      }
      else if (aVal > 100) {
        payload = "100";
      }
      isSuccess = processRequest(deviceAddr, device, payload.c_str(), deviceTopic, disconnectAfter);
    }
    else {
      tempPayload.toUpperCase();
      payload = tempPayload.c_str();
      if ((strcmp(payload.c_str(), "PRESS") == 0) || (strcmp(payload.c_str(), "ON") == 0) || (strcmp(payload.c_str(), "OFF") == 0) || (strcmp(payload.c_str(), "OPEN") == 0) || (strcmp(payload.c_str(), "CLOSE") == 0) || (strcmp(payload.c_str(), "PAUSE") == 0)
          || (strcmp(payload.c_str(), "REQUESTSETTINGS") == 0) || (strcmp(payload.c_str(), "REQUESTINFO") == 0) || (strcmp(payload.c_str(), "GETSETTINGS") == 0) || (strcmp(payload.c_str(), "GETINFO") == 0)
          || (strcmp(payload.c_str(), "MODEPRESS") == 0) || (strcmp(payload.c_str(), "MODEPRESSINV") == 0) || (strcmp(payload.c_str(), "MODESWITCH") == 0) || (strcmp(payload.c_str(), "MODESWITCHINV") == 0)) {
        isSuccess = processRequest(deviceAddr, device, payload.c_str(), deviceTopic, disconnectAfter);
      }
      else {
        char aBuffer[100];
        StaticJsonDocument<100> docOut;
        docOut["status"] = "errorJSONValue";
        serializeJson(docOut, aBuffer, sizeof(aBuffer));
        printAString("Parsing failed = value not a valid command");
        addToPublish(ESPMQTTTopic.c_str(), aBuffer);
      }
    }
  }
  else {
    char aBuffer[100];
    StaticJsonDocument<100> docOut;
    docOut["status"] = "errorJSONDevice";
    serializeJson(docOut, aBuffer, sizeof(aBuffer));
    printAString("Parsing failed = device not from list");
    addToPublish(ESPMQTTTopic.c_str(), aBuffer);
  }

  delay(100);
  return isSuccess;
}

void performHoldPressSequence(std::string aDevice, std::string aCommand, int aHold ) {
  String holdString = String(aHold);
  struct QueueCommand queueCommandHold;
  queueCommandHold.payload = holdString.c_str();
  queueCommandHold.topic = ESPMQTTTopic + "/control";
  queueCommandHold.device = aDevice;
  queueCommandHold.currentTry = 1;
  queueCommandHold.priority = false;
  queueCommandHold.disconnectAfter = false;
  commandQueue.enqueue(queueCommandHold);
  delay(50);
  struct QueueCommand queueCommandPress;
  std::string aPress = aCommand;
  queueCommandPress.payload = aPress.c_str();
  queueCommandPress.topic = ESPMQTTTopic + "/control";
  queueCommandPress.device = aDevice;
  queueCommandPress.currentTry = 1;
  queueCommandPress.priority = false;
  queueCommandPress.disconnectAfter = true;
  commandQueue.enqueue(queueCommandPress);
}

void performHoldPress(std::string aDevice, int aHold) {
  performHoldPressSequence(aDevice, "PRESS", aHold );
}

void performHoldOn(std::string aDevice, int aHold) {
  performHoldPressSequence(aDevice, "ON", aHold );
}

void performHoldOff(std::string aDevice, int aHold) {
  performHoldPressSequence(aDevice, "OFF", aHold );
}

bool parseScanSeconds(const char *text, int &seconds) {
  if (text == nullptr || *text == '\0') { return false; }
  unsigned int value = 0;
  for (const char *p = text; *p; ++p) {
    if (*p < '0' || *p > '9') { return false; }
    value = value * 10 + (*p - '0');
    if (value > 300) { return false; }
  }
  if (value == 0) { return false; }
  seconds = value;
  return true;
}

void rescanMQTT(std::string & payload) {
  ProcessingScope processingScope;
  StaticJsonDocument<100> docIn;
  if (!parseMQTTPayload(docIn, payload.c_str(), "rescanMQTT")) {
    return;
  }
  // Zero means an endless NimBLE scan; never accept it for a timed rescan.
  String seconds = docIn["sec"].as<String>();
  int value = 0;
  if (!parseScanSeconds(seconds.c_str(), value)) {
    publishStatus(ESPMQTTTopic, "errorJSONValue");
    return;
  }
  rescan(value);
}

void requestInfoMQTT(std::string & payload) {
  processing = true;
  printAString("Processing Request Info MQTT...");
  StaticJsonDocument<100> docIn;

  if (!parseMQTTPayload(docIn, payload.c_str(), "requestInfoMQTT")) {
    processing = false;
    return;
  }

  const char * aName = docIn["id"]; //Get sensor type value
  if (aName == nullptr) {
    publishStatus(ESPMQTTTopic, "errorJSONId");
    processing = false;
    return;
  }
  printAString("Device: ");
  printAString(aName);

  std::string deviceAddr = "";
  std::string deviceTopic;
  std::string anAddr;

  std::map<std::string, std::string>::iterator itS = allBots.find(aName);
  if (itS != allBots.end())
  {
    anAddr = itS->second;
    std::transform(anAddr.begin(), anAddr.end(), anAddr.begin(), to_lower());
    deviceAddr = anAddr.c_str();
    deviceTopic = botTopic;
  }
  itS = allCurtains.find(aName);
  if (itS != allCurtains.end())
  {
    anAddr = itS->second;
    std::transform(anAddr.begin(), anAddr.end(), anAddr.begin(), to_lower());
    deviceAddr = anAddr.c_str();
    deviceTopic = curtainTopic;
  }
  itS = allMeters.find(aName);
  if (itS != allMeters.end())
  {
    anAddr = itS->second;
    std::transform(anAddr.begin(), anAddr.end(), anAddr.begin(), to_lower());
    deviceAddr = anAddr.c_str();
    deviceTopic = meterTopic;
  }
  itS = allContactSensors.find(aName);
  if (itS != allContactSensors.end())
  {
    anAddr = itS->second;
    std::transform(anAddr.begin(), anAddr.end(), anAddr.begin(), to_lower());
    deviceAddr = anAddr.c_str();
    deviceTopic = contactTopic;
  }
  itS = allMotionSensors.find(aName);
  if (itS != allMotionSensors.end())
  {
    anAddr = itS->second;
    std::transform(anAddr.begin(), anAddr.end(), anAddr.begin(), to_lower());
    deviceAddr = anAddr.c_str();
    deviceTopic = motionTopic;
  }
  itS = allPlugs.find(aName);
  if (itS != allPlugs.end())
  {
    anAddr = itS->second;
    std::transform(anAddr.begin(), anAddr.end(), anAddr.begin(), to_lower());
    deviceAddr = anAddr.c_str();
    deviceTopic = plugTopic;
  }
  if (deviceAddr != "") {
    deviceTopic = deviceTopic + aName;
    processRequest(deviceAddr, aName, "requestInfo", deviceTopic, true);
  }
  else {
    publishStatus(ESPMQTTTopic, "errorJSONId");
    printAString("Parsing failed = device not from list");
  }
  processing = false;
}

void onConnectionEstablished() {
  if (!MDNS.begin(host)) {
    printAString("Error starting mDNS");
  }
  printAString("Reconnected to WIFI/MQTT");
  server.close();
  server.begin();
  std::string anAddr;
  std::string aDevice;
  std::map<std::string, std::string>::iterator it;

  if (manualDebugStartESP32WithMQTT) {
    client.subscribe((ESPMQTTTopic + "/manualstart").c_str(), [aDevice] (const String & payload)  {
      if ((payload != NULL) && !(payload.isEmpty())) {
        printAString("Manually starting ESP32...");
        manualDebugStartESP32WithMQTT = false;
        client.unsubscribe((ESPMQTTTopic + "/manualstart").c_str());
        onConnectionEstablished();
      }
    });
  }

  else {
    if (!deviceHasBooted) {
      deviceHasBooted = true;
      if (ledOnBootScan) {
        digitalWrite(LED_BUILTIN, ledONValue);
      }
      client.publish(ESPMQTTTopic.c_str(), "{\"status\":\"boot\"}");
      client.publish((esp32Topic + "/firmware").c_str(), versionNum, true);

      delay(100);
      it = allBots.begin();
      while (it != allBots.end())
      {
        std::string deviceStr ;
        aDevice = it->first.c_str();
        client.subscribe((botTopic + aDevice + "/assumedstate").c_str(), [aDevice] (const String & payload)  {
          if ((payload != NULL) && !(payload.isEmpty())) {
            printAString("state MQTT Received (from retained)...updating ON/OFF simulate states");

            if (isBotDevice(aDevice)) {
              std::map<std::string, std::string>::iterator itP = allBots.find(aDevice);
              if (itP != allBots.end())
              {
                std::string aMac = itP->second.c_str();
                std::map<std::string, bool>::iterator itZ = botsInPressMode.find(aMac);
                std::map<std::string, bool>::iterator itE = botsSimulateONOFFinPRESSmode.find(aDevice);
                std::map<std::string, bool>::iterator itI = botsSimulatedStates.find(aDevice);

                if (itE != botsSimulateONOFFinPRESSmode.end())
                {
                  printAString("settings the value");
                  if ((strcmp(payload.c_str(), "OFF") == 0)) {
                    if (itI == botsSimulatedStates.end()) {
                      botsSimulatedStates[aDevice] = false;
                    }

                  } else if ((strcmp(payload.c_str(), "ON") == 0)) {
                    if (itI == botsSimulatedStates.end()) {
                      botsSimulatedStates[aDevice] = true;
                    }
                  }
                }
              }
            }
          }
          client.unsubscribe((botTopic + aDevice + "/assumedstate").c_str());
        });

        client.subscribe((botTopic + aDevice + "/settings").c_str(), [aDevice] (const String & payload)  {
          if ((payload != NULL) && !(payload.isEmpty())) {
            printAString("settings MQTT Received (from retained)...updating firmware/timers/hold");
            StaticJsonDocument<100> docIn;
            if (isBotDevice(aDevice)) {
              printAString("going thru bot settings retained");
              std::map<std::string, std::string>::iterator itP = allBots.find(aDevice);
              if (itP != allBots.end())
              {
                std::string aMac = itP->second.c_str();
                if (!parseMQTTPayload(docIn, payload.c_str(), "retainedBotSettings")) {
                  return;
                }
                if (docIn.containsKey("firmware")) {
                  printAString("contains firmware");
                  const char * firmware = docIn["firmware"];
                  botFirmwares[aMac] = firmware;
                }
                if (docIn.containsKey("timers")) {
                  printAString("contains timers");
                  botNumTimers[aMac] = docIn["timers"];
                }
                if (docIn.containsKey("hold")) {
                  printAString("contains hold");
                  botHoldSecs[aMac] = docIn["hold"];
                }
                if (docIn.containsKey("inverted")) {
                  printAString("contains inverted");
                  botInverteds[aMac] = docIn["inverted"];
                }
              }
            }
          }
          client.unsubscribe((botTopic + aDevice + "/settings").c_str());
        });
        it++;
      }
    }
    addToPublish(lastWill, "online", true);
    publishRecentFailures(false, 0);
    if (includeSensorSystemInfo) {
      deviceInfoPolling();
    }

    it = allCurtains.begin();
    while (it != allCurtains.end())
    {
      std::string deviceStr ;
      aDevice = it->first.c_str();
      client.subscribe((curtainTopic + aDevice + "/set").c_str(), [aDevice] (const String & payload)  {
        if ((payload != NULL) && !(payload.isEmpty())) {
          printAString("Control MQTT Received...");
          if (pScan->isScanning() || isRescanning) {
            if (pScan->isScanning()) {
              pScan->stop();
            }
            allSwitchbotsScanned = {};
            forceRescan = true;
            lastUpdateTimes = {};
          }
          if (!commandQueue.isFull()) {
            if (immediateCurtainStateUpdate && isCurtainDevice(aDevice)) {
              std::string deviceStateTopic = curtainTopic + aDevice + "/state";
              std::string devicePosTopic = curtainTopic + aDevice + "/position";
              std::map<std::string, std::string>::iterator itP = allCurtains.find(aDevice);
              if (itP != allCurtains.end())
              {
                std::string aMac = itP->second.c_str();

                String tempPayload = payload.c_str();
                int dotIndex = tempPayload.indexOf(".");
                if (dotIndex >= 0) {
                  tempPayload.remove(dotIndex, tempPayload.length() - 1);
                }

                bool isNum = is_number(tempPayload.c_str());

                if (isNum) {
                  int aVal;
                  sscanf(tempPayload.c_str(), "%d", &aVal);
                  if (aVal < 0) {
                    aVal = 0;
                  }
                  else if (aVal > 100) {
                    aVal = 100;
                  }
                  StaticJsonDocument<50> docPos;
                  char aBuffer[100];
                  docPos["pos"] = aVal;
                  serializeJson(docPos, aBuffer, sizeof(aBuffer));
                  addToPublish(devicePosTopic.c_str(), aBuffer);
                }
                else if ((strcmp(payload.c_str(), "OPEN") == 0))  {
                  addToPublish(deviceStateTopic.c_str(), "OPEN", true);
                } else if ((strcmp(payload.c_str(), "CLOSE") == 0))  {
                  addToPublish(deviceStateTopic.c_str(), "CLOSE", true);
                } else if ((strcmp(payload.c_str(), "PAUSE") == 0)) {
                  addToPublish(deviceStateTopic.c_str(), "PAUSE", true);
                }
              }
            }
            struct QueueCommand queueCommand;
            queueCommand.payload = payload.c_str();
            queueCommand.topic = ESPMQTTTopic + "/control";
            queueCommand.device = aDevice;
            queueCommand.disconnectAfter = true;
            queueCommand.priority = false;
            queueCommand.currentTry = 1;
            commandQueue.enqueue(queueCommand);
          }
          else {
            client.publish(ESPMQTTTopic.c_str(), "{\"status\":\"errorQueueFull\"}");
          }
        }
      });

      it++;
    }

    it = allBots.begin();
    while (it != allBots.end())
    {
      std::string deviceStr ;
      aDevice = it->first.c_str();

      client.subscribe((botTopic + aDevice + "/set").c_str(), [aDevice] (const String & payload)  {
        if ((payload != NULL) && !(payload.isEmpty())) {
          printAString("Control MQTT Received...");

          std::map<std::string, bool>::iterator itE = botsSimulateONOFFinPRESSmode.find(aDevice);
          std::string deviceStateTopic = botTopic + aDevice + "/state";
          if (itE != botsSimulateONOFFinPRESSmode.end() && ((strcmp(payload.c_str(), "STATEOFF") == 0) || (strcmp(payload.c_str(), "STATEON") == 0))) {
            std::string deviceAssumedStateTopic = botTopic + aDevice + "/assumedstate";
            if (strcmp(payload.c_str(), "STATEOFF") == 0) {
              botsSimulatedStates[aDevice] = false;
              addToPublish(deviceStateTopic.c_str(), "OFF", true);
              addToPublish(deviceAssumedStateTopic.c_str(), "OFF", true);
            }
            else if (strcmp(payload.c_str(), "STATEON") == 0) {
              botsSimulatedStates[aDevice] = true;
              addToPublish(deviceStateTopic.c_str(), "ON", true);
              addToPublish(deviceAssumedStateTopic.c_str(), "ON", true);
            }
          }
          else {
            if (pScan->isScanning() || isRescanning) {
              if (pScan->isScanning()) {
                pScan->stop();
              }
              allSwitchbotsScanned = {};
              forceRescan = true;
              lastUpdateTimes = {};
            }
            if (!commandQueue.isFull()) {
              if (isBotDevice(aDevice)) {
                std::map<std::string, std::string>::iterator itP = allBots.find(aDevice);
                if (itP != allBots.end())
                {
                  std::string aMac = itP->second.c_str();
                  std::map<std::string, bool>::iterator itZ = botsInPressMode.find(aMac);
                  std::map<std::string, bool>::iterator itE = botsSimulateONOFFinPRESSmode.find(aDevice);
                  if (immediateBotStateUpdate) {
                    if (itZ != botsInPressMode.end() && itE == botsSimulateONOFFinPRESSmode.end())
                    {
                      addToPublish(deviceStateTopic.c_str(), "OFF", true);
                    }
                    else {
                      if ((strcmp(payload.c_str(), "OFF") == 0)) {
                        addToPublish(deviceStateTopic.c_str(), "OFF", true);
                      } else if ((strcmp(payload.c_str(), "ON") == 0)) {
                        addToPublish(deviceStateTopic.c_str(), "ON", true);
                      }
                    }
                  }

                  int aHold = -1;
                  std::string commandString = "";
                  if (itE != botsSimulateONOFFinPRESSmode.end())
                  {
                    if (strcmp(payload.c_str(), "OFF") == 0) {
                      commandString = "OFF";
                      std::map<std::string, int>::iterator itI = botsSimulatedOFFHoldTimes.find(aDevice);
                      if (itI != botsSimulatedOFFHoldTimes.end())
                      {
                        aHold = itI->second;
                      }
                    }

                    else if (strcmp(payload.c_str(), "ON") == 0) {
                      commandString = "ON";
                      std::map<std::string, int>::iterator itI = botsSimulatedONHoldTimes.find(aDevice);
                      if (itI != botsSimulatedONHoldTimes.end())
                      {
                        aHold = itI->second;
                      }
                    }
                  }
                  if (aHold >= 0) {
                    performHoldPressSequence(aDevice, commandString, aHold);
                  }
                  else {
                    struct QueueCommand queueCommand;
                    queueCommand.payload = payload.c_str();
                    queueCommand.topic = ESPMQTTTopic + "/control";
                    queueCommand.device = aDevice;
                    queueCommand.disconnectAfter = true;
                    queueCommand.priority = false;
                    queueCommand.currentTry = 1;
                    commandQueue.enqueue(queueCommand);
                  }
                }
              }
            }
            else {
              client.publish(ESPMQTTTopic.c_str(), "{\"status\":\"errorQueueFull\"}");
            }
          }
        }
      });

      it++;
    }


    it = allPlugs.begin();
    while (it != allPlugs.end())
    {
      std::string deviceStr ;
      aDevice = it->first.c_str();

      client.subscribe((plugTopic + aDevice + "/set").c_str(), [aDevice] (const String & payload)  {
        if ((payload != NULL) && !(payload.isEmpty())) {
          printAString("Control MQTT Received...");
          std::string deviceStateTopic = plugTopic + aDevice + "/state";

          if (pScan->isScanning() || isRescanning) {
            if (pScan->isScanning()) {
              pScan->stop();
            }
            allSwitchbotsScanned = {};
            forceRescan = true;
            lastUpdateTimes = {};
          }
          if (!commandQueue.isFull()) {
            if (isPlugDevice(aDevice)) {
              std::map<std::string, std::string>::iterator itP = allPlugs.find(aDevice);
              if (itP != allPlugs.end())
              {
                std::string aMac = itP->second.c_str();
                if (immediatePlugStateUpdate) {
                  if ((strcmp(payload.c_str(), "OFF") == 0)) {
                    addToPublish(deviceStateTopic.c_str(), "OFF", true);
                  } else if ((strcmp(payload.c_str(), "ON") == 0)) {
                    addToPublish(deviceStateTopic.c_str(), "ON", true);
                  }
                }
                struct QueueCommand queueCommand;
                queueCommand.payload = payload.c_str();
                queueCommand.topic = ESPMQTTTopic + "/control";
                queueCommand.device = aDevice;
                queueCommand.disconnectAfter = true;
                queueCommand.priority = false;
                queueCommand.currentTry = 1;
                commandQueue.enqueue(queueCommand);
              }
            }
          }
          else {
            client.publish(ESPMQTTTopic.c_str(), "{\"status\":\"errorQueueFull\"}");
          }
        }
      });

      it++;
    }

    it = allMeters.begin();
    while (it != allMeters.end())
    {
      std::string deviceStr ;
      aDevice = it->first.c_str();
      client.subscribe((meterTopic + aDevice + "/set").c_str(), [aDevice] (const String & payload)  {
        if ((payload != NULL) && !(payload.isEmpty())) {
          printAString("Control MQTT Received...");
          bool skip = false;
          if (isRescanning) {
            if (pScan->isScanning() || isRescanning) {
              if (pScan->isScanning()) {
                pScan->stop();
              }
              allSwitchbotsScanned = {};
              forceRescan = true;
              lastUpdateTimes = {};
            }
            if ((strcmp(payload.c_str(), "REQUESTINFO") == 0) || (strcmp(payload.c_str(), "GETINFO") == 0)) {
              skip = true;
            }
          }
          if (!skip) {
            if (!commandQueue.isFull()) {
              struct QueueCommand queueCommand;
              queueCommand.payload = payload.c_str();
              queueCommand.topic = ESPMQTTTopic + "/control";
              queueCommand.device = aDevice;
              queueCommand.disconnectAfter = true;
              queueCommand.priority = false;
              queueCommand.currentTry = 1;
              commandQueue.enqueue(queueCommand);
            }
            else {
              client.publish(ESPMQTTTopic.c_str(), "{\"status\":\"errorQueueFull\"}");
            }
          }
        }
      });

      it++;
    }

    it = allContactSensors.begin();
    while (it != allContactSensors.end())
    {
      std::string deviceStr ;
      aDevice = it->first.c_str();
      client.subscribe((contactTopic + aDevice + "/set").c_str(), [aDevice] (const String & payload)  {
        if ((payload != NULL) && !(payload.isEmpty())) {
          printAString("Control MQTT Received...");
          bool skip = false;
          if (isRescanning) {
            if (pScan->isScanning() || isRescanning) {
              if (pScan->isScanning()) {
                pScan->stop();
              }
              allSwitchbotsScanned = {};
              forceRescan = true;
              lastUpdateTimes = {};
            }
            if ((strcmp(payload.c_str(), "REQUESTINFO") == 0) || (strcmp(payload.c_str(), "GETINFO") == 0)) {
              skip = true;
            }
          }
          if (!skip) {
            if (!commandQueue.isFull()) {
              struct QueueCommand queueCommand;
              queueCommand.payload = payload.c_str();
              queueCommand.topic = ESPMQTTTopic + "/control";
              queueCommand.device = aDevice;
              queueCommand.disconnectAfter = true;
              queueCommand.priority = false;
              queueCommand.currentTry = 1;
              commandQueue.enqueue(queueCommand);
            }
            else {
              client.publish(ESPMQTTTopic.c_str(), "{\"status\":\"errorQueueFull\"}");
            }
          }
        }
      });



      if (!isMeshNode && meshContactSensors && enableMesh) {
        client.subscribe((contactTopic + aDevice + "/buttoncount").c_str(), [aDevice] (const String & payload)  {

          printAString("START contactTopic + aDevice + buttoncount");
          if ((payload != NULL) && !(payload.isEmpty())) {
            printAString("MQTT meshButtonCount received...");
            std::string anAddr;
            std::map<std::string, std::string>::iterator itY = allSwitchbots.find(aDevice);
            if (itY != allSwitchbots.end())
            {
              anAddr = itY->second;
              bool isNum = is_number(payload.c_str());
              if (isNum) {
                int buttonCount;
                sscanf(payload.c_str(), "%d", &buttonCount);
                if (buttonCount != 0) {
                  std::map<std::string, int>::iterator itE = buttonCounts.find(anAddr);
                  if (itE != buttonCounts.end())
                  {
                    int bCount = itE->second;

                    if ((bCount < buttonCount) ||  ((bCount > 10) && (buttonCount < 5))) {
                      buttonCounts[anAddr] = buttonCount;
                      std::string deviceButtonTopic = contactTopic + aDevice + "/button";
                      //addToPublish(deviceButtonTopic.c_str(), "PUSHED", false);
                      client.publish(deviceButtonTopic.c_str(), "PUSHED", false);
                      lastButton[anAddr] = millis();
                    }
                  }
                }
              }
            }
          }
          printAString("END contactTopic + aDevice + buttoncount");
        });

        client.subscribe((contactTopic + aDevice + "/outcount").c_str(), [aDevice] (const String & payload)  {
          printAString("START contactTopic + aDevice + outcount");
          if ((payload != NULL) && !(payload.isEmpty())) {
            printAString("MQTT meshOutCount received...");
            std::string anAddr;
            std::map<std::string, std::string>::iterator itY = allSwitchbots.find(aDevice);
            if (itY != allSwitchbots.end())
            {
              anAddr = itY->second;
              bool isNum = is_number(payload.c_str());
              if (isNum) {
                int outCount;
                sscanf(payload.c_str(), "%d", &outCount);
                if (outCount != 0) {
                  std::map<std::string, int>::iterator itE = outCounts.find(anAddr);
                  if (itE != outCounts.end())
                  {
                    int bCount = itE->second;

                    if ((bCount < outCount) ||  ((bCount == 3) && (outCount == 1))) {
                      outCounts[anAddr] = outCount;
                      std::string deviceOutTopic = contactTopic + aDevice + "/out";
                      //addToPublish(deviceOutTopic.c_str(), "EXITED", false);
                      client.publish(deviceOutTopic.c_str(), "EXITED", false);
                      lastOut[anAddr] = millis();
                    }
                  }
                }
              }
            }
          }
          printAString("END contactTopic + aDevice + outcount");
        });

        client.subscribe((contactTopic + aDevice + "/incount").c_str(), [aDevice] (const String & payload)  {
          printAString("START contactTopic + aDevice + incount");
          if ((payload != NULL) && !(payload.isEmpty())) {
            printAString("MQTT meshInCount received...");
            std::string anAddr;
            std::map<std::string, std::string>::iterator itY = allSwitchbots.find(aDevice);
            if (itY != allSwitchbots.end())
            {
              anAddr = itY->second;
              bool isNum = is_number(payload.c_str());
              if (isNum) {
                int inCount;
                sscanf(payload.c_str(), "%d", &inCount);
                if (inCount != 0) {
                  std::map<std::string, int>::iterator itE = entranceCounts.find(anAddr);
                  if (itE != entranceCounts.end())
                  {
                    int bCount = itE->second;
                    if ((bCount < inCount) ||  ((bCount == 3) && (inCount == 1))) {
                      entranceCounts[anAddr] = inCount;
                      std::string deviceInTopic = contactTopic + aDevice + "/in";
                      //addToPublish(deviceInTopic.c_str(), "ENTERED", false);
                      client.publish(deviceInTopic.c_str(), "ENTERED", false);
                      lastIn[anAddr] = millis();
                    }
                  }
                }
              }
            }
          }
          printAString("END contactTopic + aDevice + outcount");
        });

      }

      if (meshContactSensors && enableMesh) {
        if (countMotionToAvoidDuplicates) {
          client.subscribe((contactTopic + aDevice + "/motioncount").c_str(), [aDevice] (const String & payload)  {
            printAString("START contactTopic + aDevice + motioncount");
            if ((payload != NULL) && !(payload.isEmpty())) {
              printAString("MQTT meshMotionCount received...");
              std::string anAddr;
              std::map<std::string, std::string>::iterator itY = allSwitchbots.find(aDevice);
              if (itY != allSwitchbots.end())
              {
                anAddr = itY->second;
                bool isNum = is_number(payload.c_str());
                if (isNum) {
                  int newMeshMotionCount;
                  sscanf(payload.c_str(), "%d", &newMeshMotionCount);
                  if (newMeshMotionCount != 0) {
                    std::map<std::string, int>::iterator itWE = motionCounts.find(anAddr.c_str());
                    int motionCount = 0;
                    if (itWE != motionCounts.end())
                    {
                      motionCount = itWE->second;
                    }

                    itWE = meshMotionCounts.find(anAddr.c_str());
                    int meshMotionCount = 0;
                    if (itWE != meshMotionCounts.end())
                    {
                      meshMotionCount = itWE->second;
                    }

                    itWE = noMotionCounts.find(anAddr.c_str());
                    int noMotionCount = 0;
                    if (itWE != noMotionCounts.end())
                    {
                      noMotionCount = itWE->second;
                    }

                    itWE = meshNoMotionCounts.find(anAddr.c_str());
                    int meshNoMotionCount = 0;
                    if (itWE != meshNoMotionCounts.end())
                    {
                      meshNoMotionCount = itWE->second;
                    }

                    if ((motionCount == 0) && (meshMotionCount == 0)) {
                      meshMotionCounts[anAddr.c_str()] = newMeshMotionCount;
                      motionCounts[anAddr.c_str()] = newMeshMotionCount;
                    }
                    else if (meshMotionCount != 0)
                    {
                      if ((meshMotionCount < newMeshMotionCount) ||  ((meshMotionCount > 40) && (newMeshMotionCount < 10))) {
                        motionMeshStates[anAddr.c_str()] = "MOTION";
                        meshMotionCounts[anAddr.c_str()] = newMeshMotionCount;
                        if (!isMeshNode && onlyAllowRootESPToPublishMotion) {
                          std::string deviceMotionTopic = contactTopic + aDevice + "/motion";
                          addToPublish(deviceMotionTopic.c_str(), "MOTION", true);
                        }
                        if (motionCount != newMeshMotionCount) {
                          updateMotionCount[anAddr.c_str()] = millis();
                          updateMeshMotionCount[anAddr.c_str()] = newMeshMotionCount;
                        }
                      }
                      else if ((meshMotionCount == newMeshMotionCount) && (noMotionCount == meshNoMotionCount)) {
                        std::map<std::string, std::string>::iterator itH = motionMeshStates.find(anAddr.c_str());
                        if (itH != motionMeshStates.end())
                        {
                          std::string motionState = itH->second.c_str();
                          if (strcmp(motionState.c_str(), "MOTION") == 0) {
                            if (!isMeshNode && onlyAllowRootESPToPublishMotion) {
                              std::string deviceMotionTopic = contactTopic + aDevice + "/motion";
                              addToPublish(deviceMotionTopic.c_str(), "MOTION", true);
                            }
                          }
                        }
                      }
                    }
                  }
                }
              }
            }
            printAString("END contactTopic + aDevice + motioncount");
          });

          client.subscribe((contactTopic + aDevice + "/nomotioncount").c_str(), [aDevice] (const String & payload)  {
            printAString("START contactTopic + aDevice + nomotioncount");
            if ((payload != NULL) && !(payload.isEmpty())) {
              printAString("MQTT meshNoMotionCount received...");
              std::string anAddr;
              std::map<std::string, std::string>::iterator itY = allSwitchbots.find(aDevice);
              if (itY != allSwitchbots.end())
              {
                anAddr = itY->second;
                bool isNum = is_number(payload.c_str());
                if (isNum) {
                  int newMeshNoMotionCount;
                  sscanf(payload.c_str(), "%d", &newMeshNoMotionCount);
                  if (newMeshNoMotionCount != 0) {
                    std::map<std::string, int>::iterator itWE = noMotionCounts.find(anAddr.c_str());
                    int noMotionCount = 0;
                    if (itWE != noMotionCounts.end())
                    {
                      noMotionCount = itWE->second;
                    }

                    itWE = meshNoMotionCounts.find(anAddr.c_str());
                    int meshNoMotionCount = 0;
                    if (itWE != meshNoMotionCounts.end())
                    {
                      meshNoMotionCount = itWE->second;
                    }

                    itWE = motionCounts.find(anAddr.c_str());
                    int motionCount = 0;
                    if (itWE != motionCounts.end())
                    {
                      motionCount = itWE->second;
                    }

                    itWE = meshMotionCounts.find(anAddr.c_str());
                    int meshMotionCount = 0;
                    if (itWE != meshMotionCounts.end())
                    {
                      meshMotionCount = itWE->second;
                    }

                    if ((noMotionCount == 0) && (meshNoMotionCount == 0)) {
                      meshNoMotionCounts[anAddr.c_str()] = newMeshNoMotionCount;
                      noMotionCounts[anAddr.c_str()] = newMeshNoMotionCount;
                    }
                    else if (meshNoMotionCount != 0)
                    {
                      if ((meshNoMotionCount < newMeshNoMotionCount) ||  ((meshNoMotionCount > 40) && (newMeshNoMotionCount < 10))) {
                        motionMeshStates[anAddr.c_str()] = "NO MOTION";
                        meshNoMotionCounts[anAddr.c_str()] = newMeshNoMotionCount;
                        if (!isMeshNode && onlyAllowRootESPToPublishMotion) {
                          std::string deviceMotionTopic = contactTopic + aDevice + "/motion";
                          addToPublish(deviceMotionTopic.c_str(), "NO MOTION", true);
                        }
                        if (noMotionCount != newMeshNoMotionCount) {
                          updateNoMotionCount[anAddr.c_str()] = millis();
                          updateMeshNoMotionCount[anAddr.c_str()] = newMeshNoMotionCount;
                        }
                      }
                      else if ((meshNoMotionCount == newMeshNoMotionCount) && (motionCount == meshMotionCount)) {
                        std::map<std::string, std::string>::iterator itH = motionMeshStates.find(anAddr.c_str());
                        if (itH != motionMeshStates.end())
                        {
                          std::string motionState = itH->second.c_str();
                          if (strcmp(motionState.c_str(), "NO MOTION") == 0) {
                            if (!isMeshNode && onlyAllowRootESPToPublishMotion) {
                              std::string deviceMotionTopic = contactTopic + aDevice + "/motion";
                              addToPublish(deviceMotionTopic.c_str(), "NO MOTION", true);
                            }
                          }
                        }
                      }
                    }
                  }
                }
              }
            }
            printAString("END contactTopic + aDevice + nomotioncount");
          });

        }

        if (countContactToAvoidDuplicates) {
          client.subscribe((contactTopic + aDevice + "/closedcount").c_str(), [aDevice] (const String & payload)  {
            printAString("START contactTopic + aDevice + closedcount");
            if ((payload != NULL) && !(payload.isEmpty())) {
              printAString("MQTT meshClosedCount received...");
              std::string anAddr;
              std::map<std::string, std::string>::iterator itY = allSwitchbots.find(aDevice);
              if (itY != allSwitchbots.end())
              {
                anAddr = itY->second;
                bool isNum = is_number(payload.c_str());
                if (isNum) {
                  int newMeshClosedCount;
                  sscanf(payload.c_str(), "%d", &newMeshClosedCount);
                  if (newMeshClosedCount != 0) {
                    std::map<std::string, int>::iterator itWE = closedCounts.find(anAddr.c_str());
                    int closedCount = 0;
                    if (itWE != closedCounts.end())
                    {
                      closedCount = itWE->second;
                    }

                    itWE = meshClosedCounts.find(anAddr.c_str());
                    int meshClosedCount = 0;
                    if (itWE != meshClosedCounts.end())
                    {
                      meshClosedCount = itWE->second;
                    }

                    itWE = openCounts.find(anAddr.c_str());
                    int openCount = 0;
                    if (itWE != openCounts.end())
                    {
                      openCount = itWE->second;
                    }

                    itWE = meshOpenCounts.find(anAddr.c_str());
                    int meshOpenCount = 0;
                    if (itWE != meshOpenCounts.end())
                    {
                      meshOpenCount = itWE->second;
                    }

                    itWE = timeoutCounts.find(anAddr.c_str());
                    int timeoutCount = 0;
                    if (itWE != timeoutCounts.end())
                    {
                      timeoutCount = itWE->second;
                    }

                    itWE = meshTimeoutCounts.find(anAddr.c_str());
                    int meshTimeoutCount = 0;
                    if (itWE != meshTimeoutCounts.end())
                    {
                      meshTimeoutCount = itWE->second;
                    }

                    if ((closedCount == 0) && (meshClosedCount == 0)) {
                      meshClosedCounts[anAddr.c_str()] = newMeshClosedCount;
                      closedCounts[anAddr.c_str()] = newMeshClosedCount;
                    }
                    else if (meshClosedCount != 0)
                    {
                      if ((meshClosedCount < newMeshClosedCount) ||  ((meshClosedCount > 40) && (newMeshClosedCount < 10))) {
                        contactMeshStates[anAddr.c_str()] = "CLOSED";
                        meshClosedCounts[anAddr.c_str()] = newMeshClosedCount;
                        if (!isMeshNode && onlyAllowRootESPToPublishContact) {
                          std::string deviceContactTopic = contactTopic + aDevice + "/contact";
                          std::string deviceStateTopic = contactTopic + aDevice + "/state";
                          std::string deviceBinContactTopic = contactTopic + aDevice + "/bin";
                          addToPublish(deviceContactTopic.c_str(), "CLOSED", true);
                          addToPublish(deviceStateTopic.c_str(), "CLOSED", true);
                          addToPublish(deviceBinContactTopic.c_str(), "CLOSED", true);
                        }
                        if (closedCount != newMeshClosedCount) {
                          updateClosedCount[anAddr.c_str()] = millis();
                          updateMeshClosedCount[anAddr.c_str()] = newMeshClosedCount;
                        }
                      }
                      else if ((meshClosedCount == newMeshClosedCount) && (openCount == meshOpenCount) && (timeoutCount == meshTimeoutCount)) {
                        std::map<std::string, std::string>::iterator itH = contactMeshStates.find(anAddr.c_str());
                        if (itH != contactMeshStates.end())
                        {
                          std::string contactState = itH->second.c_str();
                          if (strcmp(contactState.c_str(), "CLOSED") == 0) {
                            if (!isMeshNode && onlyAllowRootESPToPublishContact) {
                              std::string deviceContactTopic = contactTopic + aDevice + "/contact";
                              std::string deviceStateTopic = contactTopic + aDevice + "/state";
                              std::string deviceBinContactTopic = contactTopic + aDevice + "/bin";
                              addToPublish(deviceContactTopic.c_str(), "CLOSED", true);
                              addToPublish(deviceStateTopic.c_str(), "CLOSED", true);
                              addToPublish(deviceBinContactTopic.c_str(), "CLOSED", true);
                            }
                          }
                        }
                      }
                    }
                  }
                }
              }
            }
            printAString("END contactTopic + aDevice + closedcount");
          });

          client.subscribe((contactTopic + aDevice + "/opencount").c_str(), [aDevice] (const String & payload)  {
            printAString("START contactTopic + aDevice + opencount");
            if ((payload != NULL) && !(payload.isEmpty())) {
              printAString("MQTT meshOpenCount received...");
              std::string anAddr;
              std::map<std::string, std::string>::iterator itY = allSwitchbots.find(aDevice);
              if (itY != allSwitchbots.end())
              {
                anAddr = itY->second;
                bool isNum = is_number(payload.c_str());
                if (isNum) {
                  int newMeshOpenCount;
                  sscanf(payload.c_str(), "%d", &newMeshOpenCount);
                  if (newMeshOpenCount != 0) {
                    std::map<std::string, int>::iterator itWE = closedCounts.find(anAddr.c_str());
                    int closedCount = 0;
                    if (itWE != closedCounts.end())
                    {
                      closedCount = itWE->second;
                    }

                    itWE = meshClosedCounts.find(anAddr.c_str());
                    int meshClosedCount = 0;
                    if (itWE != meshClosedCounts.end())
                    {
                      meshClosedCount = itWE->second;
                    }

                    itWE = openCounts.find(anAddr.c_str());
                    int openCount = 0;
                    if (itWE != openCounts.end())
                    {
                      openCount = itWE->second;
                    }

                    itWE = meshOpenCounts.find(anAddr.c_str());
                    int meshOpenCount = 0;
                    if (itWE != meshOpenCounts.end())
                    {
                      meshOpenCount = itWE->second;
                    }

                    itWE = timeoutCounts.find(anAddr.c_str());
                    int timeoutCount = 0;
                    if (itWE != timeoutCounts.end())
                    {
                      timeoutCount = itWE->second;
                    }

                    itWE = meshTimeoutCounts.find(anAddr.c_str());
                    int meshTimeoutCount = 0;
                    if (itWE != meshTimeoutCounts.end())
                    {
                      meshTimeoutCount = itWE->second;
                    }

                    if ((openCount == 0) && (meshOpenCount == 0)) {
                      meshOpenCounts[anAddr.c_str()] = newMeshOpenCount;
                      openCounts[anAddr.c_str()] = newMeshOpenCount;
                    }
                    else if (meshOpenCount != 0)
                    {
                      if ((meshOpenCount < newMeshOpenCount) || ((meshOpenCount > 40) && (newMeshOpenCount < 10)))  {
                        contactMeshStates[anAddr.c_str()] = "OPEN";
                        meshOpenCounts[anAddr.c_str()] = newMeshOpenCount;
                        lastContacts[aDevice.c_str()] = millis();
                        if (!isMeshNode && onlyAllowRootESPToPublishContact) {
                          // addToPublish("esp32mesh1/" + aDevice + "/contact", "OPENFROMMESH+", true);
                          std::string deviceContactTopic = contactTopic + aDevice + "/contact";
                          std::string deviceStateTopic = contactTopic + aDevice + "/state";
                          std::string deviceBinContactTopic = contactTopic + aDevice + "/bin";
                          addToPublish(deviceContactTopic.c_str(), "OPEN", true);
                          addToPublish(deviceStateTopic.c_str(), "OPEN", true);
                          addToPublish(deviceBinContactTopic.c_str(), "OPEN", true);
                          addToPublish((contactTopic + aDevice + "/lastcontact"), 0, true);
                        }

                        if (openCount != newMeshOpenCount) {
                          updateOpenCount[anAddr.c_str()] = millis();
                          updateMeshOpenCount[anAddr.c_str()] = newMeshOpenCount;
                        }
                      }
                      else if ((meshOpenCount == newMeshOpenCount) && (closedCount == meshClosedCount) && (timeoutCount == meshTimeoutCount)) {
                        std::map<std::string, std::string>::iterator itH = contactMeshStates.find(anAddr.c_str());
                        if (itH != contactMeshStates.end())
                        {
                          std::string contactState = itH->second.c_str();
                          if (strcmp(contactState.c_str(), "OPEN") == 0) {
                            lastContacts[aDevice.c_str()] = millis();
                            if (!isMeshNode && onlyAllowRootESPToPublishContact) {
                              std::string deviceContactTopic = contactTopic + aDevice + "/contact";
                              std::string deviceStateTopic = contactTopic + aDevice + "/state";
                              std::string deviceBinContactTopic = contactTopic + aDevice + "/bin";
                              addToPublish(deviceContactTopic.c_str(), "OPEN", true);
                              addToPublish(deviceStateTopic.c_str(), "OPEN", true);
                              addToPublish(deviceBinContactTopic.c_str(), "OPEN", true);
                              addToPublish((contactTopic + aDevice + "/lastcontact"), 0, true);
                            }
                          }
                        }
                      }
                    }
                  }
                }
              }
            }
            printAString("END contactTopic + aDevice + closedcount");
          });

          client.subscribe((contactTopic + aDevice + "/timeoutcount").c_str(), [aDevice] (const String & payload)  {
            printAString("START contactTopic + aDevice + timeoutcount");
            if ((payload != NULL) && !(payload.isEmpty())) {
              printAString("MQTT meshTimeoutCount received...");
              std::string anAddr;
              std::map<std::string, std::string>::iterator itY = allSwitchbots.find(aDevice);
              if (itY != allSwitchbots.end())
              {
                anAddr = itY->second;
                bool isNum = is_number(payload.c_str());
                if (isNum) {
                  int newMeshTimeoutCount;
                  sscanf(payload.c_str(), "%d", &newMeshTimeoutCount);
                  if (newMeshTimeoutCount != 0) {
                    std::map<std::string, int>::iterator itWE = closedCounts.find(anAddr.c_str());
                    int closedCount = 0;
                    if (itWE != closedCounts.end())
                    {
                      closedCount = itWE->second;
                    }

                    itWE = meshClosedCounts.find(anAddr.c_str());
                    int meshClosedCount = 0;
                    if (itWE != meshClosedCounts.end())
                    {
                      meshClosedCount = itWE->second;
                    }

                    itWE = openCounts.find(anAddr.c_str());
                    int openCount = 0;
                    if (itWE != openCounts.end())
                    {
                      openCount = itWE->second;
                    }

                    itWE = meshOpenCounts.find(anAddr.c_str());
                    int meshOpenCount = 0;
                    if (itWE != meshOpenCounts.end())
                    {
                      meshOpenCount = itWE->second;
                    }

                    itWE = timeoutCounts.find(anAddr.c_str());
                    int timeoutCount = 0;
                    if (itWE != timeoutCounts.end())
                    {
                      timeoutCount = itWE->second;
                    }

                    itWE = meshTimeoutCounts.find(anAddr.c_str());
                    int meshTimeoutCount = 0;
                    if (itWE != meshTimeoutCounts.end())
                    {
                      meshTimeoutCount = itWE->second;
                    }

                    if ((timeoutCount == 0) && (meshTimeoutCount == 0)) {
                      meshTimeoutCounts[anAddr.c_str()] = newMeshTimeoutCount;
                      timeoutCounts[anAddr.c_str()] = newMeshTimeoutCount;
                    }
                    else if (meshTimeoutCount != 0)
                    {
                      if ((meshTimeoutCount < newMeshTimeoutCount) ||  ((meshTimeoutCount > 40) && (newMeshTimeoutCount < 10))) {
                        contactMeshStates[anAddr.c_str()] = "TIMEOUT";
                        meshTimeoutCounts[anAddr.c_str()] = newMeshTimeoutCount;
                        lastContacts[aDevice.c_str()] = millis();
                        if (!isMeshNode && onlyAllowRootESPToPublishContact) {
                          std::string deviceContactTopic = contactTopic + aDevice + "/contact";
                          std::string deviceStateTopic = contactTopic + aDevice + "/state";
                          std::string deviceBinContactTopic = contactTopic + aDevice + "/bin";
                          addToPublish(deviceContactTopic.c_str(), "TIMEOUT", true);
                          addToPublish(deviceStateTopic.c_str(), "TIMEOUT", true);
                          addToPublish(deviceBinContactTopic.c_str(), "OPEN", true);
                        }
                        if (timeoutCount != newMeshTimeoutCount) {
                          updateTimeoutCount[anAddr.c_str()] = millis();
                          updateMeshTimeoutCount[anAddr.c_str()] = newMeshTimeoutCount;
                        }
                      }
                      else if ((meshTimeoutCount == newMeshTimeoutCount) && (openCount == meshOpenCount) && (closedCount == meshClosedCount)) {
                        std::map<std::string, std::string>::iterator itH = contactMeshStates.find(anAddr.c_str());
                        if (itH != contactMeshStates.end())
                        {
                          std::string contactState = itH->second.c_str();
                          if (strcmp(contactState.c_str(), "TIMEOUT") == 0) {
                            lastContacts[aDevice.c_str()] = millis();
                            if (!isMeshNode && onlyAllowRootESPToPublishContact) {
                              std::string deviceContactTopic = contactTopic + aDevice + "/contact";
                              std::string deviceStateTopic = contactTopic + aDevice + "/state";
                              std::string deviceBinContactTopic = contactTopic + aDevice + "/bin";
                              addToPublish(deviceContactTopic.c_str(), "TIMEOUT", true);
                              addToPublish(deviceStateTopic.c_str(), "TIMEOUT", true);
                              addToPublish(deviceBinContactTopic.c_str(), "OPEN", true);
                            }
                          }
                        }
                      }
                    }
                  }
                }
              }
            }
            printAString("END contactTopic + aDevice + timeoutcount");
          });
        }

        if (countLightToAvoidDuplicates) {
          client.subscribe((contactTopic + aDevice + "/darkcount").c_str(), [aDevice] (const String & payload)  {
            printAString("START contactTopic + aDevice + darkcount");
            if ((payload != NULL) && !(payload.isEmpty())) {
              printAString("MQTT meshDarkCount received...");
              std::string anAddr;
              std::map<std::string, std::string>::iterator itY = allSwitchbots.find(aDevice);
              if (itY != allSwitchbots.end())
              {
                anAddr = itY->second;
                bool isNum = is_number(payload.c_str());
                if (isNum) {
                  int newMeshDarkCount;
                  sscanf(payload.c_str(), "%d", &newMeshDarkCount);
                  if (newMeshDarkCount != 0) {
                    std::map<std::string, int>::iterator itWE = darkCounts.find(anAddr.c_str());
                    int darkCount = 0;
                    if (itWE != darkCounts.end())
                    {
                      darkCount = itWE->second;
                    }

                    itWE = meshDarkCounts.find(anAddr.c_str());
                    int meshDarkCount = 0;
                    if (itWE != meshDarkCounts.end())
                    {
                      meshDarkCount = itWE->second;
                    }

                    itWE = brightCounts.find(anAddr.c_str());
                    int brightCount = 0;
                    if (itWE != brightCounts.end())
                    {
                      brightCount = itWE->second;
                    }

                    itWE = meshBrightCounts.find(anAddr.c_str());
                    int meshBrightCount = 0;
                    if (itWE != meshBrightCounts.end())
                    {
                      meshBrightCount = itWE->second;
                    }

                    if ((darkCount == 0) && (meshDarkCount == 0)) {
                      meshDarkCounts[anAddr.c_str()] = newMeshDarkCount;
                      darkCounts[anAddr.c_str()] = newMeshDarkCount;
                    }
                    else if (meshDarkCount != 0)
                    {
                      if ((meshDarkCount < newMeshDarkCount) ||  ((meshDarkCount > 40) && (newMeshDarkCount < 10))) {
                        lightMeshStates[anAddr.c_str()] = "DARK";
                        meshDarkCounts[anAddr.c_str()] = newMeshDarkCount;
                        if (!isMeshNode && onlyAllowRootESPToPublishLight) {
                          std::string deviceLightTopic = contactTopic + aDevice + "/illuminance";
                          addToPublish(deviceLightTopic.c_str(), "DARK", true);
                        }
                        if (darkCount != newMeshDarkCount) {
                          updateDarkCount[anAddr.c_str()] = millis();
                          updateMeshDarkCount[anAddr.c_str()] = newMeshDarkCount;
                        }
                      }
                      else if ((meshDarkCount == newMeshDarkCount) && (brightCount == meshBrightCount)) {
                        std::map<std::string, std::string>::iterator itH = lightMeshStates.find(anAddr.c_str());
                        if (itH != lightMeshStates.end())
                        {
                          std::string illuminanceState = itH->second.c_str();
                          if (strcmp(illuminanceState.c_str(), "DARK") == 0) {
                            if (!isMeshNode && onlyAllowRootESPToPublishLight) {
                              std::string deviceLightTopic = contactTopic + aDevice + "/illuminance";
                              addToPublish(deviceLightTopic.c_str(), "DARK", true);
                            }
                          }
                        }
                      }
                    }
                  }
                }
              }
            }
            printAString("END contactTopic + aDevice + darkcount");
          });

          client.subscribe((contactTopic + aDevice + "/brightcount").c_str(), [aDevice] (const String & payload)  {
            printAString("START contactTopic + aDevice + brightcount");
            if ((payload != NULL) && !(payload.isEmpty())) {
              printAString("MQTT meshBrightCount received...");
              std::string anAddr;
              std::map<std::string, std::string>::iterator itY = allSwitchbots.find(aDevice);
              if (itY != allSwitchbots.end())
              {
                anAddr = itY->second;
                bool isNum = is_number(payload.c_str());
                if (isNum) {
                  int newMeshBrightCount;
                  sscanf(payload.c_str(), "%d", &newMeshBrightCount);
                  if (newMeshBrightCount != 0) {
                    std::map<std::string, int>::iterator itWE = brightCounts.find(anAddr.c_str());
                    int brightCount = 0;
                    if (itWE != brightCounts.end())
                    {
                      brightCount = itWE->second;
                    }

                    itWE = meshBrightCounts.find(anAddr.c_str());
                    int meshBrightCount = 0;
                    if (itWE != meshBrightCounts.end())
                    {
                      meshBrightCount = itWE->second;
                    }

                    itWE = darkCounts.find(anAddr.c_str());
                    int darkCount = 0;
                    if (itWE != darkCounts.end())
                    {
                      darkCount = itWE->second;
                    }

                    itWE = meshDarkCounts.find(anAddr.c_str());
                    int meshDarkCount = 0;
                    if (itWE != meshDarkCounts.end())
                    {
                      meshDarkCount = itWE->second;
                    }

                    if ((brightCount == 0) && (meshBrightCount == 0)) {
                      meshBrightCounts[anAddr.c_str()] = newMeshBrightCount;
                      brightCounts[anAddr.c_str()] = newMeshBrightCount;
                    }
                    else if (meshBrightCount != 0)
                    {
                      if ((meshBrightCount < newMeshBrightCount) ||  ((meshBrightCount > 40) && (newMeshBrightCount < 10))) {
                        lightMeshStates[anAddr.c_str()] = "BRIGHT";
                        meshBrightCounts[anAddr.c_str()] = newMeshBrightCount;
                        if (!isMeshNode && onlyAllowRootESPToPublishLight) {
                          std::string deviceLightTopic = contactTopic + aDevice + "/illuminance";
                          addToPublish(deviceLightTopic.c_str(), "BRIGHT", true);
                        }
                        if (brightCount != newMeshBrightCount) {
                          updateBrightCount[anAddr.c_str()] = millis();
                          updateMeshBrightCount[anAddr.c_str()] = newMeshBrightCount;
                        }
                      }
                      else if ((meshBrightCount == newMeshBrightCount) && (darkCount == meshDarkCount)) {
                        std::map<std::string, std::string>::iterator itH = lightMeshStates.find(anAddr.c_str());
                        if (itH != lightMeshStates.end())
                        {
                          std::string illuminanceState = itH->second.c_str();
                          if (strcmp(illuminanceState.c_str(), "BRIGHT") == 0) {
                            if (!isMeshNode && onlyAllowRootESPToPublishLight) {
                              std::string deviceLightTopic = contactTopic + aDevice + "/illuminance";
                              addToPublish(deviceLightTopic.c_str(), "BRIGHT", true);
                            }
                          }
                        }
                      }
                    }
                  }
                }
              }
            }
            printAString("END contactTopic + aDevice + brightcount");
          });
        }
      }

      it++;
    }

    it = allMotionSensors.begin();
    while (it != allMotionSensors.end())
    {
      std::string deviceStr ;
      aDevice = it->first.c_str();
      client.subscribe((motionTopic + aDevice + "/set").c_str(), [aDevice] (const String & payload)  {
        if ((payload != NULL) && !(payload.isEmpty())) {
          printAString("Control MQTT Received...");
          bool skip = false;
          if (isRescanning) {
            if (pScan->isScanning() || isRescanning) {
              if (pScan->isScanning()) {
                pScan->stop();
              }
              allSwitchbotsScanned = {};
              forceRescan = true;
              lastUpdateTimes = {};
            }
            if ((strcmp(payload.c_str(), "REQUESTINFO") == 0) || (strcmp(payload.c_str(), "GETINFO") == 0)) {
              skip = true;
            }
          }
          if (!skip) {
            if (!commandQueue.isFull()) {
              struct QueueCommand queueCommand;
              queueCommand.payload = payload.c_str();
              queueCommand.topic = ESPMQTTTopic + "/control";
              queueCommand.device = aDevice;
              queueCommand.disconnectAfter = true;
              queueCommand.priority = false;
              queueCommand.currentTry = 1;
              commandQueue.enqueue(queueCommand);
            }
            else {
              client.publish(ESPMQTTTopic.c_str(), "{\"status\":\"errorQueueFull\"}");
            }
          }
        }
      });

      if (meshMotionSensors && enableMesh) {
        if (countMotionToAvoidDuplicates) {
          client.subscribe((motionTopic + aDevice + "/motioncount").c_str(), [aDevice] (const String & payload)  {
            printAString("START motionTopic + aDevice + motioncount");
            if ((payload != NULL) && !(payload.isEmpty())) {
              printAString("MQTT meshMotionCount received...");
              std::string anAddr;
              std::map<std::string, std::string>::iterator itY = allSwitchbots.find(aDevice);
              if (itY != allSwitchbots.end())
              {
                anAddr = itY->second;
                bool isNum = is_number(payload.c_str());
                if (isNum) {
                  int newMeshMotionCount;
                  sscanf(payload.c_str(), "%d", &newMeshMotionCount);
                  if (newMeshMotionCount != 0) {
                    std::map<std::string, int>::iterator itWE = motionCounts.find(anAddr.c_str());
                    int motionCount = 0;
                    if (itWE != motionCounts.end())
                    {
                      motionCount = itWE->second;
                    }

                    itWE = meshMotionCounts.find(anAddr.c_str());
                    int meshMotionCount = 0;
                    if (itWE != meshMotionCounts.end())
                    {
                      meshMotionCount = itWE->second;
                    }

                    itWE = noMotionCounts.find(anAddr.c_str());
                    int noMotionCount = 0;
                    if (itWE != noMotionCounts.end())
                    {
                      noMotionCount = itWE->second;
                    }

                    itWE = meshNoMotionCounts.find(anAddr.c_str());
                    int meshNoMotionCount = 0;
                    if (itWE != meshNoMotionCounts.end())
                    {
                      meshNoMotionCount = itWE->second;
                    }

                    if ((motionCount == 0) && (meshMotionCount == 0)) {
                      meshMotionCounts[anAddr.c_str()] = newMeshMotionCount;
                      motionCounts[anAddr.c_str()] = newMeshMotionCount;
                    }
                    else if (meshMotionCount != 0)
                    {
                      if ((meshMotionCount < newMeshMotionCount) ||  ((meshMotionCount > 40) && (newMeshMotionCount < 10))) {
                        motionMeshStates[anAddr.c_str()] = "MOTION";
                        meshMotionCounts[anAddr.c_str()] = newMeshMotionCount;
                        if (!isMeshNode && onlyAllowRootESPToPublishMotion) {
                          std::string deviceMotionTopic = motionTopic + aDevice + "/motion";
                          addToPublish(deviceMotionTopic.c_str(), "MOTION", true);
                          std::string deviceStateTopic = motionTopic + aDevice + "/state";
                          addToPublish(deviceStateTopic.c_str(), "MOTION", true);
                        }
                        if (motionCount != newMeshMotionCount) {
                          updateMotionCount[anAddr.c_str()] = millis();
                          updateMeshMotionCount[anAddr.c_str()] = newMeshMotionCount;
                        }
                      }
                      else if ((meshMotionCount == newMeshMotionCount) && (noMotionCount == meshNoMotionCount)) {
                        std::map<std::string, std::string>::iterator itH = motionMeshStates.find(anAddr.c_str());
                        if (itH != motionMeshStates.end())
                        {
                          std::string motionState = itH->second.c_str();
                          if (strcmp(motionState.c_str(), "MOTION") == 0) {
                            if (!isMeshNode && onlyAllowRootESPToPublishMotion) {
                              std::string deviceMotionTopic = motionTopic + aDevice + "/motion";
                              addToPublish(deviceMotionTopic.c_str(), "MOTION", true);
                              std::string deviceStateTopic = motionTopic + aDevice + "/state";
                              addToPublish(deviceStateTopic.c_str(), "MOTION", true);
                            }
                          }
                        }
                      }
                    }
                  }
                }
              }
            }
            printAString("END motionTopic + aDevice + motioncount");
          });

          client.subscribe((motionTopic + aDevice + "/nomotioncount").c_str(), [aDevice] (const String & payload)  {
            printAString("START motionTopic + aDevice + nomotioncount");
            if ((payload != NULL) && !(payload.isEmpty())) {
              printAString("MQTT meshNoMotionCount received...");
              std::string anAddr;
              std::map<std::string, std::string>::iterator itY = allSwitchbots.find(aDevice);
              if (itY != allSwitchbots.end())
              {
                anAddr = itY->second;
                bool isNum = is_number(payload.c_str());
                if (isNum) {
                  int newMeshNoMotionCount;
                  sscanf(payload.c_str(), "%d", &newMeshNoMotionCount);
                  if (newMeshNoMotionCount != 0) {
                    std::map<std::string, int>::iterator itWE = noMotionCounts.find(anAddr.c_str());
                    int noMotionCount = 0;
                    if (itWE != noMotionCounts.end())
                    {
                      noMotionCount = itWE->second;
                    }

                    itWE = meshNoMotionCounts.find(anAddr.c_str());
                    int meshNoMotionCount = 0;
                    if (itWE != meshNoMotionCounts.end())
                    {
                      meshNoMotionCount = itWE->second;
                    }

                    itWE = motionCounts.find(anAddr.c_str());
                    int motionCount = 0;
                    if (itWE != motionCounts.end())
                    {
                      motionCount = itWE->second;
                    }

                    itWE = meshMotionCounts.find(anAddr.c_str());
                    int meshMotionCount = 0;
                    if (itWE != meshMotionCounts.end())
                    {
                      meshMotionCount = itWE->second;
                    }

                    if ((noMotionCount == 0) && (meshNoMotionCount == 0)) {
                      meshNoMotionCounts[anAddr.c_str()] = newMeshNoMotionCount;
                      noMotionCounts[anAddr.c_str()] = newMeshNoMotionCount;
                    }
                    else if (meshNoMotionCount != 0)
                    {
                      if ((meshNoMotionCount < newMeshNoMotionCount) ||  ((meshNoMotionCount > 40) && (newMeshNoMotionCount < 10))) {
                        motionMeshStates[anAddr.c_str()] = "NO MOTION";
                        meshNoMotionCounts[anAddr.c_str()] = newMeshNoMotionCount;
                        if (!isMeshNode && onlyAllowRootESPToPublishMotion) {
                          std::string deviceMotionTopic = motionTopic + aDevice + "/motion";
                          addToPublish(deviceMotionTopic.c_str(), "NO MOTION", true);
                        }
                        if (noMotionCount != newMeshNoMotionCount) {
                          updateNoMotionCount[anAddr.c_str()] = millis();
                          updateMeshNoMotionCount[anAddr.c_str()] = newMeshNoMotionCount;
                        }
                      }
                      else if ((meshNoMotionCount == newMeshNoMotionCount) && (motionCount == meshMotionCount)) {
                        std::map<std::string, std::string>::iterator itH = motionMeshStates.find(anAddr.c_str());
                        if (itH != motionMeshStates.end())
                        {
                          std::string motionState = itH->second.c_str();
                          if (strcmp(motionState.c_str(), "NO MOTION") == 0) {
                            if (!isMeshNode && onlyAllowRootESPToPublishMotion) {
                              std::string deviceMotionTopic = motionTopic + aDevice + "/motion";
                              addToPublish(deviceMotionTopic.c_str(), "NO MOTION", true);
                            }
                          }
                        }
                      }
                    }
                  }
                }
              }
            }
            printAString("END motionTopic + aDevice + nomotioncount");
          });

        }

        if (countLightToAvoidDuplicates) {
          client.subscribe((motionTopic + aDevice + "/darkcount").c_str(), [aDevice] (const String & payload)  {
            printAString("START motionTopic + aDevice + darkcount");
            if ((payload != NULL) && !(payload.isEmpty())) {
              printAString("MQTT meshDarkCount received...");
              std::string anAddr;
              std::map<std::string, std::string>::iterator itY = allSwitchbots.find(aDevice);
              if (itY != allSwitchbots.end())
              {
                anAddr = itY->second;
                bool isNum = is_number(payload.c_str());
                if (isNum) {
                  int newMeshDarkCount;
                  sscanf(payload.c_str(), "%d", &newMeshDarkCount);
                  if (newMeshDarkCount != 0) {
                    std::map<std::string, int>::iterator itWE = darkCounts.find(anAddr.c_str());
                    int darkCount = 0;
                    if (itWE != darkCounts.end())
                    {
                      darkCount = itWE->second;
                    }

                    itWE = meshDarkCounts.find(anAddr.c_str());
                    int meshDarkCount = 0;
                    if (itWE != meshDarkCounts.end())
                    {
                      meshDarkCount = itWE->second;
                    }

                    itWE = brightCounts.find(anAddr.c_str());
                    int brightCount = 0;
                    if (itWE != brightCounts.end())
                    {
                      brightCount = itWE->second;
                    }

                    itWE = meshBrightCounts.find(anAddr.c_str());
                    int meshBrightCount = 0;
                    if (itWE != meshBrightCounts.end())
                    {
                      meshBrightCount = itWE->second;
                    }

                    if ((darkCount == 0) && (meshDarkCount == 0)) {
                      meshDarkCounts[anAddr.c_str()] = newMeshDarkCount;
                      darkCounts[anAddr.c_str()] = newMeshDarkCount;
                    }
                    else if (meshDarkCount != 0)
                    {
                      if ((meshDarkCount < newMeshDarkCount) ||  ((meshDarkCount > 40) && (newMeshDarkCount < 10))) {
                        lightMeshStates[anAddr.c_str()] = "DARK";
                        meshDarkCounts[anAddr.c_str()] = newMeshDarkCount;
                        if (!isMeshNode && onlyAllowRootESPToPublishLight) {
                          std::string deviceLightTopic = motionTopic + aDevice + "/illuminance";
                          addToPublish(deviceLightTopic.c_str(), "DARK", true);
                        }
                        if (darkCount != newMeshDarkCount) {
                          updateDarkCount[anAddr.c_str()] = millis();
                          updateMeshDarkCount[anAddr.c_str()] = newMeshDarkCount;
                        }
                      }
                      else if ((meshDarkCount == newMeshDarkCount) && (brightCount == meshBrightCount)) {
                        std::map<std::string, std::string>::iterator itH = lightMeshStates.find(anAddr.c_str());
                        if (itH != lightMeshStates.end())
                        {
                          std::string illuminanceState = itH->second.c_str();
                          if (strcmp(illuminanceState.c_str(), "DARK") == 0) {
                            if (!isMeshNode && onlyAllowRootESPToPublishLight) {
                              std::string deviceLightTopic = motionTopic + aDevice + "/illuminance";
                              addToPublish(deviceLightTopic.c_str(), "DARK", true);
                            }
                          }
                        }
                      }
                    }
                  }
                }
              }
            }
            printAString("END motionTopic + aDevice + darkcount");
          });

          client.subscribe((motionTopic + aDevice + "/brightcount").c_str(), [aDevice] (const String & payload)  {
            printAString("START motionTopic + aDevice + brightcount");
            if ((payload != NULL) && !(payload.isEmpty())) {
              printAString("MQTT meshBrightCount received...");
              std::string anAddr;
              std::map<std::string, std::string>::iterator itY = allSwitchbots.find(aDevice);
              if (itY != allSwitchbots.end())
              {
                anAddr = itY->second;
                bool isNum = is_number(payload.c_str());
                if (isNum) {
                  int newMeshBrightCount;
                  sscanf(payload.c_str(), "%d", &newMeshBrightCount);
                  if (newMeshBrightCount != 0) {
                    std::map<std::string, int>::iterator itWE = brightCounts.find(anAddr.c_str());
                    int brightCount = 0;
                    if (itWE != brightCounts.end())
                    {
                      brightCount = itWE->second;
                    }

                    itWE = meshBrightCounts.find(anAddr.c_str());
                    int meshBrightCount = 0;
                    if (itWE != meshBrightCounts.end())
                    {
                      meshBrightCount = itWE->second;
                    }

                    itWE = darkCounts.find(anAddr.c_str());
                    int darkCount = 0;
                    if (itWE != darkCounts.end())
                    {
                      darkCount = itWE->second;
                    }

                    itWE = meshDarkCounts.find(anAddr.c_str());
                    int meshDarkCount = 0;
                    if (itWE != meshDarkCounts.end())
                    {
                      meshDarkCount = itWE->second;
                    }

                    if ((brightCount == 0) && (meshBrightCount == 0)) {
                      meshBrightCounts[anAddr.c_str()] = newMeshBrightCount;
                      brightCounts[anAddr.c_str()] = newMeshBrightCount;
                    }
                    else if (meshBrightCount != 0)
                    {
                      if ((meshBrightCount < newMeshBrightCount) ||  ((meshBrightCount > 40) && (newMeshBrightCount < 10))) {
                        lightMeshStates[anAddr.c_str()] = "BRIGHT";
                        meshBrightCounts[anAddr.c_str()] = newMeshBrightCount;
                        if (!isMeshNode && onlyAllowRootESPToPublishLight) {
                          std::string deviceLightTopic = motionTopic + aDevice + "/illuminance";
                          addToPublish(deviceLightTopic.c_str(), "BRIGHT", true);
                        }
                        if (brightCount != newMeshBrightCount) {
                          updateBrightCount[anAddr.c_str()] = millis();
                          updateMeshBrightCount[anAddr.c_str()] = newMeshBrightCount;
                        }
                      }
                      else if ((meshBrightCount == newMeshBrightCount) && (darkCount == meshDarkCount)) {
                        std::map<std::string, std::string>::iterator itH = lightMeshStates.find(anAddr.c_str());
                        if (itH != lightMeshStates.end())
                        {
                          std::string illuminanceState = itH->second.c_str();
                          if (strcmp(illuminanceState.c_str(), "BRIGHT") == 0) {
                            if (!isMeshNode && onlyAllowRootESPToPublishLight) {
                              std::string deviceLightTopic = motionTopic + aDevice + "/illuminance";
                              addToPublish(deviceLightTopic.c_str(), "BRIGHT", true);
                            }
                          }
                        }
                      }
                    }
                  }
                }
              }
            }
            printAString("END motionTopic + aDevice + brightcount");
          });
        }
      }
      it++;
    }

    client.subscribe(requestInfoStdStr.c_str(), [] (const String & payload)  {
      if ((payload != NULL) && !(payload.isEmpty())) {
        printAString("Request Info MQTT Received...");
        bool skip = false;
        if (isRescanning) {
          if (pScan->isScanning() || isRescanning) {
            if (pScan->isScanning()) {
              pScan->stop();
            }
            allSwitchbotsScanned = {};
            forceRescan = true;
            lastUpdateTimes = {};
          }
          skip = true;
        }
        if (!skip) {
          if (!commandQueue.isFull()) {
            struct QueueCommand queueCommand;
            queueCommand.payload = payload.c_str();
            queueCommand.topic = ESPMQTTTopic + "/requestInfo";
            queueCommand.disconnectAfter = true;
            queueCommand.priority = false;
            queueCommand.currentTry = 1;
            commandQueue.enqueue(queueCommand);
          }
          else {
            client.publish(ESPMQTTTopic.c_str(), "{\"status\":\"errorQueueFull\"}");
          }
        }
      }
    });

    client.subscribe(requestSettingsStdStr.c_str(), [] (const String & payload)  {
      if ((payload != NULL) && !(payload.isEmpty())) {
        printAString("Request Settings MQTT Received...");
        if (!commandQueue.isFull()) {
          StaticJsonDocument<100> docIn;
          if (!parseMQTTPayload(docIn, payload.c_str(), "requestSettings")) {
            return;
          }
          const char * aDevice = docIn["id"];
          if (aDevice == nullptr) {
            publishStatus(ESPMQTTTopic, "errorJSONId");
            return;
          }
          struct QueueCommand queueCommand;
          queueCommand.payload = "REQUESTSETTINGS";
          queueCommand.topic = ESPMQTTTopic + "/control";
          queueCommand.device = aDevice;
          queueCommand.disconnectAfter = true;
          queueCommand.priority = false;
          queueCommand.currentTry = 1;
          commandQueue.enqueue(queueCommand);
        }
        else {
          client.publish(ESPMQTTTopic.c_str(), "{\"status\":\"errorQueueFull\"}");
        }
      }
    });

    client.subscribe(setModeStdStr.c_str(), [] (const String & payload)  {
      if ((payload != NULL) && !(payload.isEmpty())) {
        printAString("setMode  MQTT Received...");
        if (!commandQueue.isFull()) {
          StaticJsonDocument<100> docIn;
          if (!parseMQTTPayload(docIn, payload.c_str(), "setMode")) {
            return;
          }
          const char * aDevice = docIn["id"];
          const char * aMode = docIn["mode"];
          if (aDevice == nullptr || aMode == nullptr) {
            publishStatus(ESPMQTTTopic, "errorJSONId");
            return;
          }
          struct QueueCommand queueCommand;
          queueCommand.payload = aMode;
          queueCommand.topic = ESPMQTTTopic + "/control";
          queueCommand.device = aDevice;
          queueCommand.disconnectAfter = true;
          queueCommand.priority = false;
          queueCommand.currentTry = 1;
          commandQueue.enqueue(queueCommand);
        }
        else {
          client.publish(ESPMQTTTopic.c_str(), "{\"status\":\"errorQueueFull\"}");
        }
      }
    });

    client.subscribe(setHoldStdStr.c_str(), [] (const String & payload)  {
      if ((payload != NULL) && !(payload.isEmpty())) {
        printAString("setHold MQTT Received...");
        if (!commandQueue.isFull()) {
          StaticJsonDocument<100> docIn;
          if (!parseMQTTPayload(docIn, payload.c_str(), "setHold")) {
            return;
          }
          const char * aDevice = docIn["id"];
          if (aDevice == nullptr || !docIn.containsKey("hold")) {
            publishStatus(ESPMQTTTopic, "errorJSONId");
            return;
          }
          int aHold = docIn["hold"];
          String holdString = String(aHold);
          struct QueueCommand queueCommand;
          queueCommand.payload = holdString.c_str();
          queueCommand.topic = ESPMQTTTopic + "/control";
          queueCommand.device = aDevice;
          queueCommand.disconnectAfter = true;
          queueCommand.priority = false;
          queueCommand.currentTry = 1;
          commandQueue.enqueue(queueCommand);
        }
        else {
          client.publish(ESPMQTTTopic.c_str(), "{\"status\":\"errorQueueFull\"}");
        }
      }
    });

    client.subscribe(holdPressStdStr.c_str(), [] (const String & payload)  {
      if ((payload != NULL) && !(payload.isEmpty())) {
        printAString("holdPress MQTT Received...");
        if (!commandQueue.isFull()) {
          StaticJsonDocument<100> docIn;
          if (!parseMQTTPayload(docIn, payload.c_str(), "holdPress")) {
            return;
          }
          const char * aDevice = docIn["id"];
          if (aDevice == nullptr || !docIn.containsKey("hold")) {
            publishStatus(ESPMQTTTopic, "errorJSONId");
            return;
          }
          int aHold = docIn["hold"];
          performHoldPress(aDevice, aHold);
        }
        else {
          client.publish(ESPMQTTTopic.c_str(), "{\"status\":\"errorQueueFull\"}");
        }
      }
    });

    client.subscribe(rescanStdStr.c_str(), [] (const String & payload)  {
      if ((payload != NULL) && !(payload.isEmpty())) {
        printAString("Rescan MQTT Received...");

        bool skip = false;
        if (isRescanning) {
          if (pScan->isScanning() || isRescanning) {
            if (pScan->isScanning()) {
              pScan->stop();
            }
            allSwitchbotsScanned = {};
            forceRescan = true;
            lastUpdateTimes = {};
          }
          skip = true;
        }
        if (!skip) {

          if (!commandQueue.isFull()) {
            struct QueueCommand queueCommand;
            queueCommand.payload = payload.c_str();
            queueCommand.topic = ESPMQTTTopic + "/rescan";
            queueCommand.disconnectAfter = true;
            queueCommand.priority = false;
            queueCommand.currentTry = 1;
            commandQueue.enqueue(queueCommand);
          }
          else {
            client.publish(ESPMQTTTopic.c_str(), "{\"status\":\"errorQueueFull\"}");
          }
        }
      }
    });

    publishHomeAssistantDiscoveryESPConfig();
    discoveredDevices = {};
  }
}

bool connectToServer(NimBLEAdvertisedDevice * advDeviceToUse) {
  if (!commandBudgetAvailable()) { return false; }
  printAString("Try to connect. Try a reconnect first...");
  NimBLEClient* pClient = nullptr;
  if (NimBLEDevice::getClientListSize()) {

    pClient = NimBLEDevice::getClientByPeerAddress(advDeviceToUse->getAddress());
    if (pClient) {
      if (!pClient->connect(advDeviceToUse, false)) {
        printAString("Reconnect failed");
      }
      else {
        printAString("Reconnected client");
      }
    }
    else {
      pClient = NimBLEDevice::getDisconnectedClient();
    }
  }
  if (!pClient) {
    if (NimBLEDevice::getClientListSize() >= NIMBLE_MAX_CONNECTIONS) {
      printAString("Max clients reached - no more connections available");
      return false;
    }
    pClient = NimBLEDevice::createClient();
    printAString("New client created");
    pClient->setClientCallbacks(&clientCB, false);
    pClient->setConnectionParams(12, 12, 0, 51);
    pClient->setConnectTimeout(10);

  }
  if (!pClient->isConnected()) {
    if (!commandBudgetAvailable()) { return false; }
    if (!pClient->connect(advDeviceToUse)) {
      NimBLEDevice::deleteClient(pClient);
      printAString("Failed to connect, deleted client");
      return false;
    }
  }
  printAString("Connected to: ");
  printAString(pClient->getPeerAddress().toString().c_str());
  printAString("RSSI: ");
  printAString(pClient->getRssi());
  return true;
}


bool sendCommandBytesNoResponse(NimBLERemoteCharacteristic * pChr, byte * bArray, int aSize ) {
  if (pChr == nullptr) {
    return false;
  }
  return pChr->writeValue(bArray, aSize, false);
}

bool sendCommandBytesWithResponse(NimBLERemoteCharacteristic * pChr, byte * bArray, int aSize ) {
  if (pChr == nullptr) {
    return false;
  }
  return pChr->writeValue(bArray, aSize, true);
}

bool sendCurtainCommandBytes(NimBLERemoteCharacteristic * pChr, byte * bArray, int aSize ) {
  if (pChr == nullptr) {
    return false;
  }
  return sendCommandBytesWithResponse(pChr, bArray, aSize);
}

bool sendBotCommandBytes(NimBLERemoteCharacteristic * pChr, byte * bArray, int aSize ) {
  if (pChr == nullptr) {
    return false;
  }
  return sendCommandBytesWithResponse(pChr, bArray, aSize);
}

bool sendPlugCommandBytes(NimBLERemoteCharacteristic * pChr, byte * bArray, int aSize ) {
  if (pChr == nullptr) {
    return false;
  }
  return sendCommandBytesWithResponse(pChr, bArray, aSize);
}

bool isBotDevice(std::string aDevice) {
  std::map<std::string, std::string>::iterator itS = allBots.find(aDevice);
  if (itS != allBots.end())
  {
    return true;
  }
  return false;
}

bool isPlugDevice(std::string aDevice) {
  std::map<std::string, std::string>::iterator itS = allPlugs.find(aDevice);
  if (itS != allPlugs.end())
  {
    return true;
  }
  return false;
}

bool isCurtainDevice(std::string aDevice) {
  std::map<std::string, std::string>::iterator itS = allCurtains.find(aDevice);
  if (itS != allCurtains.end())
  {
    return true;
  }
  return false;
}

bool isMeterDevice(std::string & aDevice) {
  std::map<std::string, std::string>::iterator itS = allMeters.find(aDevice);
  if (itS != allMeters.end())
  {
    return true;
  }
  return false;
}

bool isContactDevice(std::string & aDevice) {
  std::map<std::string, std::string>::iterator itS = allContactSensors.find(aDevice);
  if (itS != allContactSensors.end())
  {
    return true;
  }
  return false;
}

bool isMotionDevice(std::string & aDevice) {
  std::map<std::string, std::string>::iterator itS = allMotionSensors.find(aDevice);
  if (itS != allMotionSensors.end())
  {
    return true;
  }
  return false;
}

bool sendCommand(NimBLEAdvertisedDevice * advDeviceToUse, const char * type, int attempts, bool disconnectAfter) {
  if (advDeviceToUse == nullptr) {
    return false;
  }
  printAString("Sending command...");

  byte bArrayPress[] = {0x57, 0x01};
  byte bArrayOn[] = {0x57, 0x01, 0x01};
  byte bArrayOff[] = {0x57, 0x01, 0x02};
  byte bArrayPlugOn[] = {0x57, 0x0F, 0x50, 0x01, 0x01, 0x80};
  byte bArrayPlugOff[] = {0x57, 0x0F, 0x50, 0x01, 0x01, 0x00};
  byte bArrayOpen[] =  {0x57, 0x0F, 0x45, 0x01, 0x05, 0xFF, 0x00};
  byte bArrayClose[] = {0x57, 0x0F, 0x45, 0x01, 0x05, 0xFF, 0x64};
  byte bArrayPause[] = {0x57, 0x0F, 0x45, 0x01, 0x00, 0xFF};
  byte bArrayPos[] =  {0x57, 0x0F, 0x45, 0x01, 0x05, 0xFF, 0x00};
  byte bArrayGetSettings[] = {0x57, 0x02};
  byte bArrayHoldSecs[] = {0x57, 0x0F, 0x08, 0x00};
  byte bArrayBotMode[] = {0x57, 0x03, 0x64, 0x00, 0x00};

  byte bArrayPressPass[] = {0x57, 0x11, 0x00, 0x00, 0x00, 0x00};
  byte bArrayOnPass[] = {0x57, 0x11, 0x00, 0x00, 0x00, 0x00, 0x01};
  byte bArrayOffPass[] = {0x57, 0x11, 0x00, 0x00, 0x00, 0x00, 0x02};
  byte bArrayGetSettingsPass[] = {0x57, 0x12, 0x00, 0x00, 0x00, 0x00};
  byte bArrayHoldSecsPass[] = {0x57, 0x1F, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00};
  byte bArrayBotModePass[] = {0x57, 0x13, 0x00, 0x00, 0x00, 0x00, 0x64, 0x00};       // The proper array to use for setting mode with password (firmware 4.9)

  std::string anAddr = advDeviceToUse->getAddress();
  if (!NimBLEDevice::getClientListSize()) {
    return false;
  }
  NimBLEClient* pClient = NimBLEDevice::getClientByPeerAddress(anAddr);
  if (!pClient) {
    return false;
  }

  bool tryConnect = !(pClient->isConnected());
  int count = 1;
  while (tryConnect || !pClient) {
    if (!commandBudgetAvailable()) { return false; }
    if (count > 20) {
      printAString("Failed to connect for sending command");
      return false;
    }
    count++;
    printAString("Attempt to send command. Not connecting. Try connecting...");
    tryConnect = !(connectToServer(advDeviceToUse));
    if (!tryConnect) {
      pClient = NimBLEDevice::getClientByPeerAddress(anAddr);
    }
  }
  bool returnValue = true;
  std::string aPass = "";
  std::string aDevice = "";
  std::map<std::string, std::string>::iterator itU = allSwitchbotsOpp.find(anAddr);
  if (itU != allSwitchbotsOpp.end())
  {
    aDevice = itU->second.c_str();
    aPass = getPass(aDevice);
  }
  if (isBotDevice(aDevice))
  {
    returnValue = subscribeToNotify(advDeviceToUse);
  }
  else if (isCurtainDevice(aDevice))
  {
    returnValue = subscribeToNotify(advDeviceToUse);
  }
  else if (isPlugDevice(aDevice))
  {
    returnValue = subscribeToNotify(advDeviceToUse);
  }
  bool skipWaitAfter = false;
  if (returnValue) {
    NimBLERemoteService* pSvc = nullptr;
    NimBLERemoteCharacteristic* pChr = nullptr;
    pSvc = pClient->getService("cba20d00-224d-11e6-9fb8-0002a5d5c51b");
    if (pSvc) {
      pChr = pSvc->getCharacteristic("cba20002-224d-11e6-9fb8-0002a5d5c51b");
    }
    if (pChr) {
      if (pChr->canWrite()) {
        bool wasSuccess = false;
        bool isNum = is_number(type);
        uint8_t aPassCRC[4];
        if (aPass != "") {
          uint32_t aCRC = getPassCRC(aPass);
          for (int i = 0; i < 4; ++i)
          {
            aPassCRC[i] = ((uint8_t*)&aCRC)[3 - i];
          }
        }
        lastCommandSentStrings[anAddr] = type;
        if (isNum) {
          int aVal;
          sscanf(type, "%d", &aVal);
          if (isBotDevice(aDevice)) {
            skipWaitAfter = true;
            if (aPass == "") {
              printAString("Num is for a bot device - no pass");
              byte anArray[4];
              for (int i = 0; i < 4; i++) {
                if (i == 3) {
                  anArray[i] = aVal;
                }
                else {
                  anArray[i] = bArrayHoldSecs[i];
                }
              }
              wasSuccess = sendBotCommandBytes(pChr, anArray, 4);
            }
            else {
              byte anArray[8];
              for (int i = 0; i < 8; i++) {
                if ((i >= 2) && (i <= 5)) {
                  anArray[i] = aPassCRC[i - 2];
                }
                else if (i == 7) {
                  anArray[i] = aVal;
                }
                else {
                  anArray[i] = bArrayHoldSecsPass[i];
                }
              }
              wasSuccess = sendBotCommandBytes(pChr, anArray , 8);
            }
          }
          else if (isCurtainDevice(aDevice)) {
            byte anArray[7];
            for (int i = 0; i < 7; i++) {
              if (i == 6) {
                anArray[i] = (100 - aVal);
              }
              else {
                anArray[i] = bArrayPos[i];
              }
            }
            wasSuccess = sendCurtainCommandBytes(pChr, anArray , 7);
          }
        }
        else {
          if (strcmp(type, "PRESS") == 0) {
            if (aPass == "") {
              wasSuccess = sendBotCommandBytes(pChr, bArrayPress, 2);
            }
            else {
              byte anArray[6];
              for (int i = 0; i < 6; i++) {
                if ((i >= 2) && (i <= 5)) {
                  anArray[i] = aPassCRC[i - 2];
                }
                else {
                  anArray[i] = bArrayPressPass[i];
                }
              }
              wasSuccess = sendBotCommandBytes(pChr, anArray , 6);
            }
          }
          else if (strcmp(type, "ON") == 0) {
            if (isBotDevice(aDevice)) {
              if (aPass == "") {
                wasSuccess = sendBotCommandBytes(pChr, bArrayOn, 3);
              }
              else {
                byte anArray[7];
                for (int i = 0; i < 7; i++) {
                  if (i >= 2 &&  i <= 5) {
                    anArray[i] = aPassCRC[i - 2];
                  }
                  else {
                    anArray[i] = bArrayOnPass[i];
                  }
                }
                wasSuccess = sendBotCommandBytes(pChr, anArray , 7);
              }
            }
            if (isPlugDevice(aDevice)) {
              wasSuccess = sendPlugCommandBytes(pChr, bArrayPlugOn, 6);

            }

          }
          else if (strcmp(type, "OFF") == 0) {
            if (isBotDevice(aDevice)) {
              if (aPass == "") {
                wasSuccess = sendBotCommandBytes(pChr, bArrayOff, 3);
              }
              else {
                byte anArray[7];
                for (int i = 0; i < 7; i++) {
                  if (i >= 2 &&  i <= 5) {
                    anArray[i] = aPassCRC[i - 2];
                  }
                  else {
                    anArray[i] = bArrayOffPass[i];
                  }
                }
                wasSuccess = sendBotCommandBytes(pChr, anArray , 7);
              }
            }
            if (isPlugDevice(aDevice)) {
              wasSuccess = sendPlugCommandBytes(pChr, bArrayPlugOff, 6);
            }
          }
          else if (strcmp(type, "OPEN") == 0) {
            wasSuccess = sendCurtainCommandBytes(pChr, bArrayOpen, 7);
          }
          else if (strcmp(type, "CLOSE") == 0) {
            wasSuccess = sendCurtainCommandBytes(pChr, bArrayClose, 7);
          }
          else if (strcmp(type, "PAUSE") == 0) {
            wasSuccess = sendCurtainCommandBytes(pChr, bArrayPause, 6);
          }
          else if (strcmp(type, "GETSETTINGS") == 0 || strcmp(type, "REQUESTSETTINGS") == 0) {
            skipWaitAfter = true;
            if (aPass == "") {
              wasSuccess = sendBotCommandBytes(pChr, bArrayGetSettings, 2);
            }
            else {
              byte anArray[6];
              for (int i = 0; i < 6; i++) {
                if ((i >= 2) && (i <= 5)) {
                  anArray[i] = aPassCRC[i - 2];
                }
                else {
                  anArray[i] = bArrayGetSettingsPass[i];
                }
              }
              wasSuccess = sendBotCommandBytes(pChr, anArray , 6);
            }
          }
          else if (strcmp(type, "MODEPRESS") == 0) {
            if (aPass == "") {
              byte anArray[4];
              for (int i = 0; i < 4; i++) {
                if (i == 3) {
                  anArray[i] = 0x00;
                }
                else {
                  anArray[i] = bArrayBotMode[i];
                }
              }
              wasSuccess = sendBotCommandBytes(pChr, anArray , 4);
            }
            else {
              byte anArray[8];
              for (int i = 0; i < 8; i++) {
                if ((i >= 2) && (i <= 5)) {
                  anArray[i] = aPassCRC[i - 2];
                }
                else if (i == 7) {
                  anArray[i] = 0x00;
                }
                else {
                  anArray[i] = bArrayBotModePass[i];
                }
              }
              wasSuccess = sendBotCommandBytes(pChr, anArray , 8);
            }
          }
          else if (strcmp(type, "MODEPRESSINV") == 0) {
            if (aPass == "") {
              byte anArray[4];
              for (int i = 0; i < 4; i++) {
                if (i == 3) {
                  anArray[i] = 0x01;
                }
                else {
                  anArray[i] = bArrayBotMode[i];
                }
              }
              wasSuccess = sendBotCommandBytes(pChr, anArray , 4);
            }
            else {
              byte anArray[8];
              for (int i = 0; i < 8; i++) {
                if ((i >= 2) && (i <= 5)) {
                  anArray[i] = aPassCRC[i - 2];
                }
                else if (i == 7) {
                  anArray[i] = 0x01;
                }
                else {
                  anArray[i] = bArrayBotModePass[i];
                }
              }
              wasSuccess = sendBotCommandBytes(pChr, anArray , 8);
            }
          }
          else if (strcmp(type, "MODESWITCH") == 0) {
            if (aPass == "") {
              byte anArray[4];
              for (int i = 0; i < 4; i++) {
                if (i == 3) {
                  anArray[i] = 0x10;
                }
                else {
                  anArray[i] = bArrayBotMode[i];
                }
              }
              wasSuccess = sendBotCommandBytes(pChr, anArray , 4);
            }
            else {
              byte anArray[8];
              for (int i = 0; i < 8; i++) {
                if ((i >= 2) && (i <= 5)) {
                  anArray[i] = aPassCRC[i - 2];
                }
                else if (i == 7) {
                  anArray[i] = 0x10;
                }
                else {
                  anArray[i] = bArrayBotModePass[i];
                }
              }
              wasSuccess = sendBotCommandBytes(pChr, anArray , 8);
            }
          }
          else if (strcmp(type, "MODESWITCHINV") == 0) {
            if (aPass == "") {
              byte anArray[4];
              for (int i = 0; i < 4; i++) {
                if (i == 3) {
                  anArray[i] = 0x11;
                }
                else {
                  anArray[i] = bArrayBotMode[i];
                }
              }
              wasSuccess = sendBotCommandBytes(pChr, anArray , 4);
            }
            else {
              byte anArray[8];
              for (int i = 0; i < 8; i++) {
                if ((i >= 2) && (i <= 5)) {
                  anArray[i] = aPassCRC[i - 2];
                }
                else if (i == 7) {
                  anArray[i] = 0x11;
                }
                else {
                  anArray[i] = bArrayBotModePass[i];
                }
              }
              wasSuccess = sendBotCommandBytes(pChr, anArray , 8);
            }
          }
          if (wasSuccess) {
            printAString("Wrote new value to: ");
            printAString(pChr->getUUID().toString().c_str());
          }
          else {
            returnValue = false;
          }
        }
      }
      else {
        returnValue = false;
      }
    }
    else {
      printAString("CUSTOM write service not found.");
      returnValue = false;
    }
  }
  if (!returnValue) {
    if (attempts >= 10) {
      printAString("Sending failed. Disconnecting client");
      pClient->disconnect();
    } return false;
  }
  if (disconnectAfter) {
    pClient->disconnect();
  }
  printAString("Success! Command sent/received to/from SwitchBot");
  if (!skipWaitAfter) {
    lastCommandSent[anAddr] = millis();
  }
  return true;
}

bool getGeneric(NimBLEAdvertisedDevice * advDeviceToUse) {
  NimBLEClient* pClient = NimBLEDevice::getClientByPeerAddress(advDeviceToUse->getAddress());
  NimBLERemoteService* pSvc = nullptr;
  NimBLERemoteCharacteristic* pChr = nullptr;;

  pSvc = pClient->getService((uint16_t) 0x1800); // GENERIC ACCESS service
  if (pSvc) {    /** make sure it's not null */
    pChr = pSvc->getCharacteristic((uint16_t) 0x2a00); // DEVICE NAME characteristic
  }

  if (pChr) {    /** make sure it's not null */
    if (pChr->canRead()) {
      printAString(pChr->getUUID().toString().c_str());
      printAString(" Value: ");
      printAString(pChr->readValue().c_str());
      // should return WoHand
      deviceTypes[advDeviceToUse->getAddress().toString().c_str()] = pChr->readValue().c_str();
      return true;
    }
  }
  return false;
}

bool requestInfo(NimBLEAdvertisedDevice * advDeviceToUse) {
  if (advDeviceToUse == nullptr) {
    return false;
  }
  printAString("Requesting info...");
  rescanFind(advDeviceToUse->getAddress().toString().c_str());
  return true;
}

void notifyCB(NimBLERemoteCharacteristic * pRemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify) {
  printAString("notifyCB");
  noResponse = false;
  std::string aDevice;
  std::string aState;
  std::string deviceSettingsTopic;
  std::string deviceAttrTopic;
  std::string deviceStatusTopic;
  std::string deviceMac = pRemoteCharacteristic->getRemoteService()->getClient()->getPeerAddress();
  std::map<std::string, bool>::iterator itM = discoveredDevices.find(deviceMac);
  std::map<std::string, std::string>::iterator itS = allSwitchbotsOpp.find(deviceMac);

  if (itS != allSwitchbotsOpp.end())
  {
    aDevice = itS->second.c_str();
  }
  else {
    return;
  }

  std::string aCommand = "";
  std::map<std::string, std::string>::iterator itH = lastCommandSentStrings.find(deviceMac);
  if (itH != lastCommandSentStrings.end())
  {
    aCommand = itH->second.c_str();
  }

  char aBuffer[120];
  std::string deviceName;
  itS = deviceTypes.find(deviceMac.c_str());
  if (itS != deviceTypes.end())
  {
    deviceName = itS->second.c_str();
  }

  if (printSerialOutputForDebugging) {
    Serial.printf("deviceName: %s\n", deviceName.c_str());
  }

  if (deviceName == botName) {
    deviceStatusTopic = botTopic + aDevice + "/status";
    deviceSettingsTopic = botTopic + aDevice + "/settings";
    deviceAttrTopic = botTopic + aDevice + "/attributes";
    std::string deviceAssumedStateTopic = botTopic + aDevice + "/assumedstate";

    if (!lastCommandSentPublished) {
      StaticJsonDocument<60> statDoc;
      statDoc["status"] = "commandSent";
      statDoc["command"] = aCommand;
      serializeJson(statDoc, aBuffer, sizeof(aBuffer));
      client.publish(deviceStatusTopic.c_str(), aBuffer);
      lastCommandSentPublished = true;
    }

    if (length == 1) {
      StaticJsonDocument<100> statDoc;
      uint8_t byte1 = pData[0];
      printAString("The response value from bot set mode or holdSecs: ");
      printAString(byte1);
      if (byte1 == 3) {
        statDoc["status"] = "busy";
        lastCommandWasBusy = true;
      }
      //SUCCESS == 1 when setting hold secs or mode
      else if (byte1 == 1) {
        statDoc["status"] = "success";
        lastCommandWasBusy = false;
      }
      else if (byte1 == 6) {
        statDoc["status"] = "lowbattery";
        lastCommandWasBusy = false;
        //HACK to send battery 1% if bot cannot be controlled
        std::string deviceBatteryTopic = botTopic + aDevice + "/battery";
        addToPublish(deviceBatteryTopic, 1, true);
      }
      else {
        statDoc["status"] = "failed";
        lastCommandWasBusy = false;
      }
      statDoc["value"] = byte1;
      statDoc["command"] = aCommand;
      serializeJson(statDoc, aBuffer, sizeof(aBuffer));
      client.publish(deviceStatusTopic.c_str(), aBuffer);
    }
    if (length == 3) {
      StaticJsonDocument<60> statDoc;
      uint8_t byte1 = pData[0];
      printAString("The response value from bot action: ");
      printAString(byte1);
      if (byte1 == 3) {
        statDoc["status"] = "busy";
        lastCommandWasBusy = true;
      }
      //SUCCESS == 1 (on/off) or == 5 (press) for bot
      else if (byte1 == 1 || byte1 == 5) {
        statDoc["status"] = "success";
        lastCommandWasBusy = false;
        if (strcmp(aCommand.c_str(), "OFF") == 0) {
          addToPublish(deviceAssumedStateTopic.c_str(), "OFF", true);
        }
        else if (strcmp(aCommand.c_str(), "ON") == 0) {
          addToPublish(deviceAssumedStateTopic.c_str(), "ON", true);
        }
        std::map<std::string, bool>::iterator itE = botsSimulateONOFFinPRESSmode.find(aDevice.c_str());
        if (itE != botsSimulateONOFFinPRESSmode.end())
        {
          if (strcmp(aCommand.c_str(), "OFF") == 0) {
            botsSimulatedStates[aDevice] = false;
          }
          else if (strcmp(aCommand.c_str(), "ON") == 0) {
            botsSimulatedStates[aDevice] = true;
          }
          else if (strcmp(aCommand.c_str(), "PRESS") == 0) {
            botsSimulatedStates[aDevice] = !(botsSimulatedStates[aDevice]);
            addToPublish(deviceAssumedStateTopic.c_str(), botsSimulatedStates[aDevice] ? "ON" : "OFF", true);
          }
        }
      }
      else if (byte1 == 6) {
        statDoc["status"] = "lowbattery";
        lastCommandWasBusy = false;
        //HACK to send battery 1% if bot cannot be controlled
        std::string deviceBatteryTopic = botTopic + aDevice + "/battery";
        addToPublish(deviceBatteryTopic, 1, true);
      }
      else {
        statDoc["status"] = "failed";
        lastCommandWasBusy = false;
      }
      statDoc["value"] = byte1;
      statDoc["command"] = aCommand;
      serializeJson(statDoc, aBuffer, sizeof(aBuffer));
      client.publish(deviceStatusTopic.c_str(), aBuffer);
    }
    else if (length == 13) {
      StaticJsonDocument<50> statDoc;
      statDoc["status"] = "success";
      statDoc["command"] = aCommand;
      lastCommandWasBusy = false;
      serializeJson(statDoc, aBuffer, sizeof(aBuffer));
      client.publish(deviceStatusTopic.c_str(), aBuffer);

      /**** THESE SETTINGS ARE ALSO COLLECTED BY A SCAN SO IT IS REDUNDANT. Commented out because of RSSI. The rest works****/
      /*
            StaticJsonDocument<100> attDoc;
            std::map<std::string, NimBLEAdvertisedDevice*>::iterator itS = allSwitchbotsDev.find(deviceMac);
            NimBLEAdvertisedDevice* advDevice = nullptr;
            if (itS != allSwitchbotsDev.end())
            {
              advDevice =  itS->second;
            }

            //RSSI doesn't always work for some reason here. It can return 0
            attDoc["rssi"] = advDevice->getRSSI();

            uint8_t byte1 = pData[0];
            uint8_t byte2 = pData[1];

            bool isSwitch = bitRead(pData[9], 4);
            std::string aMode = isSwitch ? "Switch" : "Press"; // Whether the light switch Add-on is used or not
            if (isSwitch) {
              std::map<std::string, bool>::iterator itP = botsInPressMode.find(deviceMac);
              if (itP != botsInPressMode.end())
              {
                botsInPressMode.erase(deviceMac);
              }
              aState = (byte1 & 0b01000000) ? "OFF" : "ON"; // Mine is opposite, not sure why
            }
            else {
              botsInPressMode[deviceMac] = true;
              aState = "OFF";
            }
            int battLevel = byte2 & 0b01111111; // %

            attDoc["mode"] = aMode;
            attDoc["state"] = aState;
            attDoc["batt"] = battLevel;
            serializeJson(attDoc, aBuffer, sizeof(aBuffer));
            addToPublish(deviceAttrTopic.c_str(), aBuffer, true);
            addToPublish(deviceStateTopic.c_str(), aState.c_str(), true);*/
      /***************************************/
      StaticJsonDocument<100> settDoc;
      float fwVersion = pData[2] / 10.0;
      settDoc["firmware"] = serialized(String(fwVersion, 1));
      int timersNumber = pData[8];
      settDoc["timers"] = timersNumber;
      bool inverted = bitRead(pData[9], 0) ;
      settDoc["inverted"] = inverted;
      int holdSecs = pData[10];
      settDoc["hold"] = holdSecs;

      serializeJson(settDoc, aBuffer, sizeof(aBuffer));
      addToPublish(deviceSettingsTopic.c_str(), aBuffer, true);

      botHoldSecs[deviceMac] = holdSecs;
      botFirmwares[deviceMac] = (String(fwVersion, 1)).c_str();
      botNumTimers[deviceMac] = timersNumber;
      botInverteds[deviceMac] = inverted;
    }
  }

  else if (deviceName == curtainName) {
    deviceStatusTopic = curtainTopic + aDevice + "/status";
    deviceSettingsTopic = curtainTopic + aDevice + "/settings";
    deviceAttrTopic = curtainTopic + aDevice + "/attributes";

    if (!lastCommandSentPublished) {
      StaticJsonDocument<50> statDoc;
      statDoc["status"] = "commandSent";
      statDoc["command"] = aCommand;
      serializeJson(statDoc, aBuffer, sizeof(aBuffer));
      client.publish(deviceStatusTopic.c_str(), aBuffer);
      lastCommandSentPublished = true;
    }
    if (length < 3) {
      return;
    }
    else if (length == 3) {
      StaticJsonDocument<50> statDoc;
      uint8_t byte1 = pData[0];

      printAString("The response value from curtain: ");
      printAString(byte1);
      if (byte1 == 3) {
        statDoc["status"] = "busy";
        lastCommandWasBusy = true;
      }
      //SUCCESS == 1 or == 5 for curtain ????? just assuming based on bot
      else if (byte1 == 1 || byte1 == 5) {
        statDoc["status"] = "success";
        lastCommandWasBusy = false;
      }
      else if (byte1 == 6) {
        statDoc["status"] = "lowbattery";
        lastCommandWasBusy = false;
        //HACK to send battery 1% if curtain cannot be controlled
        std::string deviceBatteryTopic = curtainTopic + aDevice + "/battery";
        addToPublish(deviceBatteryTopic, 1, true);
      }
      else {
        statDoc["status"] = "failed";
        lastCommandWasBusy = false;
      }
      statDoc["value"] = byte1;
      statDoc["command"] = aCommand;
      serializeJson(statDoc, aBuffer, sizeof(aBuffer));
      client.publish(deviceStatusTopic.c_str(), aBuffer);
    }
  }

  else if (deviceName == plugName) {
    deviceStatusTopic = plugTopic + aDevice + "/status";
    deviceSettingsTopic = plugTopic + aDevice + "/settings";
    deviceAttrTopic = plugTopic + aDevice + "/attributes";

    if (!lastCommandSentPublished) {
      StaticJsonDocument<50> statDoc;
      statDoc["status"] = "commandSent";
      statDoc["command"] = aCommand;
      serializeJson(statDoc, aBuffer, sizeof(aBuffer));
      client.publish(deviceStatusTopic.c_str(), aBuffer);
      lastCommandSentPublished = true;
    }
    if (length < 2) {
      return;
    }
    else if (length == 2) {
      Serial.println("length:");
      Serial.println(length);
      StaticJsonDocument<50> statDoc;
      uint8_t byte1 = pData[0];

      printAString("The response value from plug: ");
      printAString(byte1);
      if (byte1 == 3) {
        statDoc["status"] = "busy";
        lastCommandWasBusy = true;
      }
      //SUCCESS == 1 or == 5 for plugTopic ????? just assuming based on bot
      else if (byte1 == 1 || byte1 == 5) {
        statDoc["status"] = "success";
        lastCommandWasBusy = false;
      }
      else {
        statDoc["status"] = "failed";
        lastCommandWasBusy = false;
      }
      statDoc["value"] = byte1;
      statDoc["command"] = aCommand;
      serializeJson(statDoc, aBuffer, sizeof(aBuffer));
      client.publish(deviceStatusTopic.c_str(), aBuffer);
    }
  }

}
