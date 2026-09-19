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
static String otaPass;                              // Loaded/generated in NVS; admin and OTA always require authentication
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
31,139,8,0,0,0,0,0,2,255,173,91,219,114,27,71,146,125,215,87,180,96,239,52,
176,2,154,0,120,177,4,16,208,146,18,37,123,45,201,90,147,182,55,134,193,216,104,116,
87,3,101,54,186,219,125,33,8,130,136,152,135,249,140,137,157,23,199,126,194,204,139,159,
134,127,50,95,178,39,235,210,55,0,180,180,177,17,54,209,151,170,172,204,172,188,156,204,
106,29,63,117,67,39,93,70,204,152,165,115,127,252,228,152,126,12,223,14,166,163,134,203,
26,227,227,57,75,109,195,153,217,113,194,210,81,35,75,189,206,115,253,52,176,231,108,212,
184,225,108,17,133,113,218,48,156,48,72,89,128,81,11,238,166,179,145,203,110,184,195,58,
226,166,205,3,158,114,219,239,36,142,237,179,81,175,129,149,82,158,250,108,124,190,224,169,
51,59,13,83,227,31,127,55,126,100,241,194,246,211,44,152,30,239,201,215,79,142,147,116,
73,191,131,56,12,211,149,135,37,58,158,61,231,254,114,144,44,147,148,205,59,25,111,39,
118,144,116,18,22,115,111,232,132,126,24,15,190,96,207,153,235,237,15,39,182,115,61,141,
195,44,112,7,95,244,186,189,175,250,93,57,0,108,204,216,156,13,92,59,190,94,255,235,
106,18,222,118,18,126,199,131,233,96,18,198,46,139,59,120,178,158,132,238,114,53,183,227,
41,15,6,221,245,140,217,120,209,158,219,60,192,195,91,41,214,160,215,61,234,70,183,67,
53,202,206,210,112,24,217,174,75,148,250,207,163,91,53,107,229,242,36,242,237,229,192,243,
217,237,208,246,249,52,232,112,240,158,12,28,232,139,197,195,159,179,36,229,222,178,163,52,
56,72,34,27,154,155,176,116,193,88,160,41,130,169,52,13,231,88,147,8,247,164,46,192,
54,27,244,191,42,120,56,136,110,13,112,219,47,189,238,189,40,94,119,141,174,209,23,4,
246,203,35,142,240,36,90,249,60,96,157,25,227,211,89,58,232,89,71,90,153,182,61,121,
238,60,95,91,108,201,38,113,184,88,169,199,71,47,92,199,57,28,150,168,244,177,142,207,
82,72,212,33,9,132,22,64,216,154,216,238,148,173,164,106,7,61,48,152,132,62,119,141,
47,246,247,15,14,14,143,134,74,231,177,237,242,44,25,16,119,185,18,161,67,67,144,45,
173,178,79,36,167,49,119,115,181,210,205,144,254,116,160,84,60,73,25,52,233,103,243,32,
25,244,188,216,192,255,195,169,29,13,122,180,35,9,115,82,30,6,91,184,233,59,251,47,
14,94,84,109,230,171,126,175,239,212,24,236,29,148,24,236,31,228,186,205,247,135,150,177,
96,31,108,37,120,146,172,12,122,123,157,222,218,183,39,204,207,249,158,248,161,115,93,19,
77,43,125,226,57,142,59,209,219,70,111,176,115,180,77,60,136,178,180,157,48,31,130,180,
39,25,150,12,196,78,14,120,48,131,11,164,195,77,201,14,186,135,189,163,186,158,191,42,
73,209,147,70,92,88,117,247,95,42,235,172,138,231,117,159,122,209,223,175,58,157,156,120,
73,49,101,4,47,115,174,225,75,87,138,128,112,16,165,173,88,152,25,233,74,9,225,100,
113,2,50,81,200,133,79,148,215,233,239,195,84,14,107,235,200,105,86,20,115,80,92,174,
202,227,143,14,97,154,125,61,190,251,162,127,216,99,90,47,93,169,240,133,52,243,163,195,
174,38,228,34,232,193,87,213,36,207,155,244,237,231,234,221,0,59,102,79,124,230,174,66,
178,235,116,57,176,136,29,201,240,194,230,233,218,178,133,89,37,85,87,23,86,71,186,165,
187,206,34,198,45,253,209,42,72,67,101,148,86,50,183,125,127,85,115,37,197,201,11,215,
246,156,254,218,146,241,84,25,174,156,187,225,74,249,134,30,146,189,12,63,211,65,186,6,
205,211,43,13,60,30,39,105,199,153,113,223,45,175,218,93,127,49,103,73,98,195,165,163,
48,225,36,246,0,225,203,185,94,14,5,83,36,239,93,135,7,46,187,29,244,11,142,40,
48,145,176,53,51,164,216,84,219,234,201,129,61,92,204,16,32,69,20,97,131,40,102,66,
119,249,186,3,8,145,46,115,85,7,97,192,242,119,22,139,227,48,174,88,195,225,65,127,
178,223,95,187,254,103,7,140,126,41,112,174,221,116,85,221,17,215,205,211,195,48,101,183,
105,71,68,246,129,48,236,97,120,195,98,207,15,23,114,215,237,96,185,128,119,178,181,139,
196,201,253,100,85,182,0,242,234,36,155,11,43,174,57,129,142,192,47,38,142,219,91,91,
30,114,96,61,157,60,158,59,72,12,17,81,55,179,142,38,103,68,133,24,181,48,107,87,
99,253,250,223,230,204,229,118,179,136,20,71,7,160,221,90,149,147,99,190,225,207,243,56,
189,83,211,121,52,174,78,82,150,190,123,90,93,17,34,144,230,242,168,120,82,86,241,65,
158,139,13,149,141,42,198,179,126,114,188,167,112,198,177,28,54,62,118,249,141,248,99,56,
190,157,36,163,134,202,126,141,241,187,239,190,61,121,119,102,188,61,251,254,225,207,23,103,
63,158,125,255,211,201,187,139,31,62,188,61,222,19,83,102,189,18,166,57,133,8,83,118,
188,135,135,234,53,182,39,208,36,5,39,13,131,187,184,20,3,27,99,32,160,9,92,7,
8,200,88,240,216,53,236,204,155,178,9,226,38,24,196,76,80,81,252,61,57,38,109,75,
14,137,128,50,255,134,17,135,0,88,141,36,181,211,44,105,24,118,204,237,142,207,111,240,
40,66,164,72,9,210,9,70,0,172,164,238,193,113,127,252,240,151,9,139,19,238,204,176,
14,110,143,93,95,16,85,84,112,159,142,207,197,53,38,167,184,117,199,63,17,119,83,230,
131,153,192,248,231,159,254,7,47,92,34,237,227,143,166,252,228,216,11,227,185,164,4,88,
128,13,38,90,30,103,190,139,123,241,92,222,52,12,29,96,13,177,13,208,135,14,214,218,
50,186,185,35,54,42,219,66,70,210,168,73,243,211,187,147,15,198,31,140,15,44,189,91,
176,248,90,138,36,82,175,1,134,70,141,89,152,164,141,241,7,192,87,131,44,66,111,146,
24,49,62,22,249,75,48,39,198,25,49,251,37,227,49,88,131,217,251,44,152,2,218,54,
246,251,13,35,178,9,233,4,163,198,229,73,231,143,118,231,174,219,121,209,185,122,214,168,
172,148,36,196,29,241,211,17,203,53,207,207,191,121,221,218,178,148,24,184,107,169,10,201,
5,247,248,71,136,190,128,134,32,4,203,88,98,136,5,212,195,116,11,245,202,28,67,164,
230,70,148,223,83,86,118,66,184,25,75,241,60,96,139,78,241,174,196,200,209,62,100,246,
17,92,102,161,15,181,141,26,239,24,148,71,187,192,130,1,76,33,137,24,12,136,197,41,
24,210,188,24,19,54,3,160,103,129,150,161,204,149,227,51,59,254,9,172,105,150,52,92,
104,140,191,243,60,22,40,193,140,112,22,176,130,34,226,234,130,33,185,4,90,206,39,199,
42,168,194,185,100,20,21,182,202,9,227,27,223,124,236,156,184,49,124,3,251,171,223,110,
114,66,118,206,157,19,87,140,220,228,166,76,238,230,64,19,220,228,164,188,77,60,106,140,
203,139,111,172,73,3,202,19,166,8,114,11,123,217,24,191,149,23,91,166,228,67,42,38,
150,77,2,6,115,62,23,191,119,115,59,185,222,182,156,30,86,158,234,6,240,199,215,31,
206,59,231,44,134,48,91,102,137,17,240,106,173,224,146,107,151,29,238,253,127,92,92,108,
120,217,252,151,52,253,90,120,218,105,28,94,35,227,239,214,69,62,116,171,7,244,250,207,
107,150,7,212,105,245,142,158,91,61,171,223,173,138,36,42,209,241,199,237,110,32,203,84,
185,187,65,54,71,200,43,47,200,225,202,61,97,241,176,245,195,195,253,195,198,134,56,63,
160,200,132,56,44,200,210,59,184,62,60,122,135,48,98,96,197,121,14,234,126,22,122,222,
230,2,117,199,38,197,62,230,216,149,57,255,87,199,222,212,239,255,139,103,191,7,111,155,
190,84,145,200,0,16,241,160,200,237,46,132,204,205,29,53,227,212,78,120,34,30,108,209,
130,28,184,213,118,94,28,129,191,72,39,11,1,179,97,242,156,169,160,111,176,120,250,240,
107,112,7,58,179,24,137,140,130,116,96,25,39,16,204,248,58,156,179,206,73,66,235,218,
64,69,103,64,100,241,195,175,152,131,156,66,73,111,194,248,220,120,248,51,2,64,28,24,
33,37,146,119,15,127,163,40,65,175,98,198,3,62,77,135,6,160,226,195,111,177,49,207,
146,68,36,155,9,79,168,76,155,18,7,228,21,224,32,38,237,78,236,216,72,48,201,58,
222,139,182,249,153,22,129,234,202,134,112,186,28,101,116,222,50,48,150,50,233,129,117,97,
127,12,227,153,77,81,138,25,106,28,109,73,32,162,151,216,64,106,188,128,43,227,46,51,
122,71,122,140,133,156,254,223,50,229,27,246,196,139,1,45,130,127,254,233,47,144,108,193,
166,169,20,23,178,18,129,212,184,6,223,106,162,100,95,131,18,9,228,18,141,57,202,121,
91,213,75,120,37,225,154,50,20,121,35,49,17,50,127,99,252,76,17,54,102,60,184,203,
160,203,41,25,139,28,166,201,238,136,74,39,138,111,64,129,159,56,3,199,48,111,0,43,
154,95,11,85,137,99,7,231,12,8,218,165,128,104,103,180,45,156,242,207,77,232,251,73,
10,11,113,57,17,58,199,184,4,73,156,93,103,164,209,173,137,188,68,170,22,106,42,17,
102,191,219,45,44,182,194,12,130,100,153,157,111,8,171,223,96,43,171,204,196,159,194,76,
149,212,54,118,142,186,138,159,231,128,242,187,57,74,99,78,219,248,222,190,69,177,237,179,
13,125,110,89,89,78,217,182,166,94,178,87,89,111,199,38,158,139,200,83,218,67,224,212,
148,249,190,222,197,186,185,159,221,82,148,103,20,89,100,124,18,214,169,240,195,195,223,240,
42,182,140,83,114,221,111,230,145,136,99,62,227,19,242,135,194,79,254,152,77,109,96,85,
215,166,233,88,82,208,25,26,1,66,114,245,29,54,192,153,249,156,61,252,21,56,128,76,
38,21,6,87,56,193,167,27,59,187,149,169,235,223,207,191,251,96,200,27,142,40,82,178,
245,157,83,249,188,52,85,222,212,166,74,55,41,97,15,49,232,13,247,153,222,32,79,92,
219,142,195,162,20,252,70,145,207,29,155,152,222,251,57,9,131,182,69,127,27,240,65,23,
10,218,162,116,248,191,82,39,60,212,247,83,4,147,224,225,87,103,150,164,70,144,197,64,
247,44,129,55,189,65,53,144,249,54,54,224,109,145,83,100,157,67,187,10,235,72,17,35,
231,198,52,126,248,13,57,193,56,149,141,153,122,68,84,133,84,53,46,202,218,143,130,61,
128,189,51,19,116,206,213,26,1,170,10,59,166,72,229,22,145,31,187,105,192,113,140,8,
139,121,169,68,154,116,79,233,198,58,158,196,227,115,108,45,226,189,156,244,145,70,81,85,
230,177,153,223,150,60,211,115,178,26,25,206,191,13,3,143,79,179,88,104,13,3,148,177,
34,123,146,185,170,192,168,246,80,241,172,90,87,122,15,0,209,160,1,32,185,156,237,63,
8,46,37,243,229,237,204,29,100,79,23,83,116,9,237,214,35,160,11,135,67,254,146,76,
169,210,174,94,59,201,74,183,228,238,54,205,170,35,17,65,234,49,40,82,157,245,121,88,
132,7,5,22,169,215,28,155,172,189,34,77,199,115,192,60,13,35,22,58,28,109,141,69,
149,73,159,195,216,35,62,135,212,135,200,87,136,171,202,214,90,207,161,196,34,133,109,108,
105,177,137,117,31,42,163,202,129,33,120,6,16,169,4,24,74,103,169,42,89,195,69,34,
192,3,12,4,62,38,45,62,198,248,57,236,1,118,170,156,70,103,221,138,18,73,15,11,
59,6,140,120,163,174,58,63,68,20,208,140,166,53,225,91,19,73,62,101,107,184,160,89,
143,41,43,139,252,208,126,68,73,154,13,99,22,66,90,219,101,143,168,233,67,38,115,179,
17,9,120,234,50,35,159,45,195,71,156,48,159,226,249,71,56,141,104,78,34,22,146,227,
230,213,90,197,19,55,153,69,206,36,127,163,170,37,143,19,155,30,40,113,71,29,125,236,
137,158,12,10,82,110,251,225,84,26,138,180,59,225,128,37,13,232,6,218,193,243,110,222,
18,253,221,150,189,232,71,110,57,159,168,244,197,37,56,60,229,41,246,115,194,8,47,164,
124,90,100,203,58,79,23,236,150,170,194,223,75,88,165,105,175,236,192,97,126,163,170,58,
128,173,73,204,8,252,110,36,172,210,212,19,97,48,141,29,241,79,147,58,45,115,189,161,
113,210,44,133,57,39,230,81,58,126,98,102,168,196,17,225,184,147,154,195,39,88,7,121,
228,203,17,214,28,187,161,147,1,205,167,214,148,165,103,62,163,203,211,229,55,110,147,187,
45,58,165,50,156,36,246,70,166,217,78,236,27,230,142,130,204,247,219,147,44,89,142,60,
219,79,152,38,69,125,221,55,34,198,142,46,77,234,7,153,109,147,122,53,248,209,69,171,
186,164,146,15,151,162,22,193,47,143,240,71,21,236,52,71,148,223,184,64,69,109,94,13,
37,113,137,138,114,242,148,63,105,104,1,217,112,87,129,112,226,94,0,43,208,120,130,124,
36,83,32,74,254,38,49,218,90,225,109,134,188,129,40,134,140,21,206,121,194,154,32,16,
250,55,108,52,94,201,69,165,6,71,95,54,205,178,25,152,173,97,237,9,25,134,217,178,
136,238,43,117,176,75,215,138,117,55,12,216,8,168,52,35,194,146,164,229,248,33,214,107,
13,213,138,77,241,186,53,92,151,40,75,219,1,217,48,112,128,48,174,71,205,22,109,84,
192,154,66,235,101,38,164,173,108,27,154,198,68,86,45,138,183,130,230,136,24,97,86,20,
179,27,176,250,154,121,118,230,167,96,166,76,124,173,39,37,179,112,241,62,116,109,31,3,
214,248,175,80,165,106,163,54,147,182,56,71,144,198,208,90,129,45,245,166,166,146,100,88,
121,37,12,155,138,200,145,152,254,210,20,63,230,192,52,177,136,157,44,3,199,40,118,45,
226,205,200,78,103,109,58,104,110,169,221,137,71,54,157,39,1,102,160,182,147,175,87,115,
150,206,66,119,64,195,70,163,17,129,125,15,1,208,125,105,190,61,187,0,233,143,223,157,
95,152,109,7,32,26,12,65,190,100,96,38,224,160,19,2,150,240,192,108,203,134,113,178,
57,127,181,30,172,76,37,72,231,2,46,8,98,117,228,7,139,251,207,206,171,243,239,223,
116,46,80,173,6,230,128,188,102,45,88,222,36,152,95,13,8,138,90,228,149,193,148,123,
203,166,144,16,138,86,182,99,167,182,18,51,22,192,18,187,192,189,230,211,216,10,175,91,
233,12,169,205,56,35,181,53,105,160,60,207,185,191,55,79,2,81,205,9,0,6,80,69,
137,2,1,194,36,115,19,54,79,131,203,59,9,50,77,119,180,226,46,148,223,158,219,14,
253,80,156,25,152,147,16,110,70,186,74,151,80,149,168,162,205,181,222,0,88,82,30,56,
160,83,56,176,138,29,77,19,17,8,203,49,191,180,203,166,172,112,77,122,202,131,128,197,
95,95,188,127,55,50,75,41,119,172,10,106,209,44,146,9,149,56,237,92,179,37,112,184,
251,89,237,229,255,146,253,229,74,171,100,252,254,228,85,222,89,171,211,135,216,59,90,106,
95,213,58,62,39,39,131,211,211,193,171,87,131,215,175,7,103,103,131,55,111,54,214,41,
154,7,199,242,80,184,180,14,233,21,19,194,72,104,94,134,133,6,212,140,88,30,2,43,
202,231,245,247,78,22,167,54,1,135,87,242,98,215,56,152,63,1,251,247,244,179,147,22,
172,24,57,171,49,254,150,46,174,83,32,131,36,220,57,122,30,138,108,140,68,179,96,83,
96,131,228,241,225,145,159,77,129,228,240,215,120,15,56,93,12,219,147,138,40,52,85,199,
91,36,126,231,181,45,43,86,250,170,101,67,115,210,12,55,116,39,173,178,65,53,8,21,
159,59,89,211,41,243,194,78,30,25,229,211,137,100,99,252,78,158,239,236,228,94,254,72,
184,79,140,231,96,191,110,86,209,78,148,95,179,225,199,64,255,238,134,99,169,179,88,101,
109,131,17,209,103,220,236,49,150,153,223,210,98,252,244,194,92,172,36,199,192,200,25,245,
178,107,61,177,223,41,207,213,50,242,211,130,26,61,151,145,90,80,60,23,12,110,129,151,
0,53,0,237,77,25,156,32,180,17,122,198,165,41,1,136,77,64,131,22,196,143,52,36,
243,170,133,72,244,75,198,226,229,185,216,93,76,53,47,11,133,153,207,240,243,204,108,92,
33,87,73,227,112,47,241,228,234,254,222,104,210,128,209,72,19,122,169,35,35,226,102,107,
248,4,241,217,181,244,230,61,45,5,253,199,215,203,183,187,180,96,78,102,248,100,215,220,
154,138,174,74,40,64,36,81,130,2,43,176,36,147,8,193,31,83,117,9,237,44,17,165,
191,110,53,20,219,255,210,248,137,199,215,200,140,178,215,16,108,52,9,44,179,69,194,196,
108,30,222,16,140,89,255,62,127,194,36,182,115,151,198,203,149,98,15,121,222,220,195,223,
61,26,110,182,41,37,61,170,52,94,82,23,146,166,134,36,166,52,61,101,121,198,34,67,
105,64,165,208,148,154,205,179,212,66,210,156,178,9,202,253,132,218,218,212,136,230,212,68,
65,109,108,76,217,34,156,5,169,17,193,203,68,127,3,91,186,70,150,7,196,0,184,209,
11,48,75,93,181,37,194,90,175,53,10,206,68,121,40,16,152,74,147,8,238,163,71,133,
16,41,161,216,117,24,22,165,221,97,201,150,165,37,107,99,40,76,184,109,10,183,254,4,
83,86,134,172,207,142,71,79,177,4,182,237,211,248,194,150,137,26,126,36,133,27,202,159,
166,128,160,170,111,141,81,192,67,40,26,155,204,175,160,68,148,191,126,211,105,173,106,210,
20,197,66,235,75,248,147,150,221,185,188,134,131,153,27,194,151,225,127,235,203,234,120,226,
162,114,42,73,224,146,2,28,137,249,212,177,42,239,170,44,199,76,132,214,87,244,221,78,
204,8,92,53,29,245,73,69,114,127,127,121,213,178,192,199,25,204,191,9,136,212,218,220,
145,242,97,177,170,109,242,219,171,58,159,66,46,2,237,250,44,183,196,167,44,164,244,75,
58,14,218,120,89,210,41,138,88,218,174,166,6,98,206,104,117,67,223,36,132,193,160,215,
86,236,15,46,175,218,21,201,7,187,213,180,30,238,222,29,226,124,84,72,242,232,198,136,
177,31,196,147,102,49,165,172,183,75,209,132,107,11,41,175,132,14,55,148,88,168,231,170,
125,89,85,105,187,164,158,171,171,22,162,218,151,77,241,36,151,4,44,136,21,174,80,173,
194,184,225,222,98,144,120,166,217,201,135,84,159,151,147,7,243,137,183,138,169,56,202,70,
180,206,129,151,215,155,6,241,72,178,113,73,57,159,232,166,146,33,145,72,68,146,84,33,
65,175,29,141,62,51,147,16,169,71,167,72,116,112,85,152,68,171,200,61,101,85,70,229,
231,17,2,163,246,22,43,202,146,89,211,133,235,171,26,195,217,172,226,168,179,5,155,165,
112,191,81,221,20,145,95,212,180,83,68,93,209,116,16,117,13,93,13,101,239,65,222,139,
33,67,17,90,196,99,17,137,100,127,183,18,228,158,138,225,168,173,108,127,40,215,145,159,
233,52,183,197,116,179,244,65,81,181,114,26,24,230,179,205,136,95,23,79,147,46,9,152,
108,72,39,7,201,22,130,252,144,169,94,37,75,118,95,154,4,46,141,51,30,104,64,76,
249,73,158,166,78,153,108,196,155,131,196,34,255,120,105,170,254,27,130,192,68,28,112,1,
137,80,238,50,2,130,177,165,199,58,84,102,219,131,95,201,79,175,219,55,218,63,169,221,
111,182,19,139,252,244,165,80,146,94,227,67,141,60,57,44,173,75,163,37,99,191,55,186,
248,236,131,230,240,136,158,189,179,51,239,142,113,164,255,247,168,233,45,207,15,193,86,98,
101,192,226,115,182,119,212,109,61,51,169,13,46,230,191,65,62,167,243,61,5,76,212,20,
209,242,195,20,148,243,209,94,175,219,63,160,41,223,242,83,49,69,21,103,180,158,178,93,
122,10,69,199,196,31,149,56,243,208,133,130,240,222,142,94,154,39,215,41,48,240,63,254,
110,232,15,41,14,172,30,132,57,193,8,132,32,109,200,233,238,74,24,110,219,118,221,221,
239,93,24,131,155,86,108,224,122,232,186,149,7,55,149,141,83,137,214,77,65,23,134,8,
183,76,172,0,117,154,195,90,121,79,70,63,24,138,183,194,164,90,91,125,132,172,185,228,
12,212,92,160,214,94,78,73,219,22,106,15,218,20,101,82,197,73,188,101,156,227,49,43,
64,34,236,86,244,128,69,55,88,116,168,45,179,240,25,240,96,147,200,149,86,213,74,4,
233,45,193,214,146,21,210,120,212,59,106,169,184,146,243,165,14,88,203,7,241,122,29,234,
103,8,80,74,90,83,95,211,137,37,229,177,145,132,158,198,142,46,24,120,33,5,220,223,
63,45,99,230,170,35,38,249,177,19,29,131,109,118,191,95,2,31,43,134,135,162,81,42,
180,92,196,5,88,60,22,219,21,250,218,121,126,223,17,215,4,53,173,8,73,75,71,167,
79,1,171,165,214,237,90,42,73,30,162,214,119,69,131,139,156,157,161,44,50,12,199,42,
231,236,226,105,57,89,15,139,212,160,49,20,53,154,229,208,34,137,232,70,87,22,251,163,
31,190,127,167,188,227,187,201,207,88,16,247,77,106,207,158,250,225,164,121,89,235,147,57,
109,209,134,238,183,174,218,43,217,171,218,232,199,173,91,154,186,189,211,253,108,120,159,109,
205,98,230,141,192,2,46,221,112,17,80,162,26,169,162,14,121,183,35,247,69,244,223,76,
12,17,42,130,54,96,90,23,8,73,97,150,54,73,95,196,62,140,41,188,46,177,15,154,
173,118,175,219,237,106,115,148,199,201,53,77,231,207,233,152,89,180,70,229,2,245,231,57,
14,175,214,78,82,70,111,84,31,78,39,79,201,101,247,74,116,12,61,109,143,184,241,44,
250,110,121,220,235,19,99,229,46,162,249,26,186,225,244,121,203,52,14,31,254,74,181,143,
54,2,161,254,136,254,109,147,114,11,79,68,40,178,81,80,116,44,5,64,81,239,246,224,
58,39,113,108,47,45,158,136,223,2,83,183,238,239,11,179,80,190,13,215,174,176,240,67,
48,125,248,205,167,35,141,164,122,80,156,208,169,173,157,18,83,170,168,40,234,61,117,162,
174,62,178,181,106,121,147,156,180,242,49,130,200,159,44,104,3,129,4,37,111,254,244,90,
143,144,122,229,88,115,103,97,171,16,155,198,76,34,254,149,78,127,203,8,77,15,82,154,
57,238,245,239,239,75,125,4,61,85,29,207,234,153,85,237,189,167,127,76,0,209,131,196,
232,245,141,63,10,193,164,2,238,22,216,89,46,26,223,226,251,76,168,104,106,79,196,87,
27,158,56,58,3,243,51,210,64,61,42,21,101,231,74,95,174,133,109,110,21,100,23,155,
132,34,243,237,202,59,78,83,38,15,123,83,241,161,137,56,151,85,31,67,128,177,252,132,
22,219,148,200,79,74,54,63,109,253,172,61,83,7,150,187,122,36,149,136,175,162,250,207,
44,189,75,119,197,246,109,157,11,189,70,123,181,110,149,130,127,46,123,158,31,253,135,95,
51,47,213,249,147,99,35,210,133,109,244,14,13,253,153,82,57,131,126,134,144,242,8,121,
171,140,165,72,161,143,169,119,197,137,34,209,202,243,81,113,144,156,31,130,203,56,97,103,
201,226,225,215,153,95,74,241,117,45,110,158,87,11,99,220,161,79,221,48,113,151,35,10,
252,212,153,194,74,54,66,33,61,211,192,167,224,189,237,9,67,204,37,174,102,200,82,190,
173,28,21,153,123,178,129,97,230,231,69,159,124,48,180,218,121,196,179,222,113,48,163,15,
111,116,176,220,98,17,185,142,30,126,67,229,172,190,139,50,74,95,94,234,79,114,160,181,
79,180,4,143,7,182,239,47,87,91,149,83,164,126,89,145,81,34,211,159,206,53,115,95,
16,40,176,40,151,68,22,67,26,163,127,254,33,207,147,143,247,228,191,131,253,95,229,95,
87,63,24,59,0,0
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
static bool adminStorageOK=false, adminAP=false;
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
  if(!server.authenticate("admin",otaPass.c_str())) {server.requestAuthentication();return false;}
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
    doc["config"]=redacted.as<JsonObject>();doc["csrf"]=adminToken;doc["trial"]=adminTrial;adminJson(200,doc);
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
    StaticJsonDocument<192> doc;if(deserializeJson(doc,server.arg("plain"))||!doc["password"].is<const char*>()){adminError(400,"Passwort fehlt.");return;}
    String value=doc["password"].as<String>();if(value.length()<12||value.length()>63){adminError(400,"Admin-Passwort: 12–63 Zeichen.");return;}
    if(!adminStorageOK||adminStore.putString("password",value)!=value.length()){adminError(507,"Passwort konnte nicht gespeichert werden.");return;}
    otaPass=value;server.send(200,"application/json","{}");
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
    if (!server.authenticate("admin", otaPass.c_str())) {
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
      otaUploadAuthorized = !adminTrial && !adminRestartAt && server.header("X-CSRF-Token") == adminToken && server.authenticate("admin", otaPass.c_str());
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
