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
31,139,8,0,0,0,0,0,2,255,205,125,91,115,27,71,150,230,187,126,69,26,246,26,
64,27,40,94,36,209,18,64,80,75,74,148,237,177,110,43,210,214,68,43,24,61,5,84,
2,40,179,80,133,169,11,33,10,66,68,63,204,63,152,215,222,237,23,199,254,132,217,151,
126,90,253,147,254,37,251,157,147,153,85,89,23,80,148,199,51,59,17,22,1,84,101,158,
60,121,242,92,190,115,50,171,124,248,133,23,77,210,235,165,20,243,116,17,28,221,57,164,
15,17,184,225,108,212,242,100,235,232,112,33,83,87,76,230,110,156,200,116,212,202,210,105,
255,129,185,26,186,11,57,106,93,249,114,181,140,226,180,37,38,81,152,202,16,173,86,190,
151,206,71,158,188,242,39,178,207,63,122,126,232,167,190,27,244,147,137,27,200,209,94,11,
35,165,126,26,200,163,179,149,159,78,230,39,81,42,254,239,255,17,63,203,120,229,6,105,
22,206,14,119,212,237,59,135,73,122,77,159,131,56,138,210,245,20,67,244,167,238,194,15,
174,7,201,117,146,202,69,63,243,123,137,27,38,253,68,198,254,116,56,137,130,40,30,124,
41,31,72,111,122,119,56,118,39,151,179,56,202,66,111,240,229,222,238,222,183,251,187,170,
1,216,152,203,133,28,120,110,124,185,249,195,122,28,189,235,39,254,123,63,156,13,198,81,
236,201,184,143,43,155,113,228,93,175,23,110,60,243,195,193,238,102,46,93,220,232,45,92,
63,196,197,119,106,90,131,189,221,131,221,229,187,161,110,229,102,105,52,92,186,158,71,148,
246,31,44,223,233,94,107,207,79,150,129,123,61,152,6,242,221,208,13,252,89,216,247,193,
123,50,152,64,94,50,30,254,146,37,169,63,189,238,107,9,14,146,165,11,201,141,101,186,
146,50,52,20,193,84,154,70,11,140,73,132,247,148,44,192,182,28,236,127,91,240,112,111,
249,78,128,219,125,235,246,222,195,226,246,174,216,21,251,76,224,174,221,226,0,87,150,235,
192,15,101,127,46,253,217,60,29,236,57,7,70,152,174,59,126,48,121,176,113,228,181,28,
199,209,106,173,47,31,60,244,38,147,251,67,139,202,62,198,9,100,138,25,245,105,6,44,
5,16,118,198,174,55,147,107,37,218,193,30,24,76,162,192,247,196,151,119,239,222,187,119,
255,96,168,101,30,187,158,159,37,3,226,46,23,34,100,40,152,172,53,202,93,34,57,139,
125,47,23,43,253,24,210,159,62,132,138,43,169,132,36,131,108,17,38,131,189,105,44,240,
111,56,115,151,131,61,90,145,68,78,82,63,10,27,184,217,159,220,125,120,239,97,89,103,
190,221,223,219,159,84,24,220,187,103,49,184,127,47,151,109,190,62,52,140,3,253,144,107,
230,73,177,50,216,219,233,239,109,2,119,44,131,156,239,113,16,77,46,43,83,51,66,31,
79,39,19,111,108,150,141,238,96,229,104,153,252,112,153,165,189,68,6,152,72,111,156,97,
200,144,87,114,224,135,115,152,64,58,172,207,236,222,238,253,189,131,170,156,191,181,102,177,
167,148,184,208,234,221,255,86,26,103,93,92,175,218,212,195,253,187,101,163,83,29,223,146,
79,25,193,202,38,151,176,165,11,77,128,13,68,75,43,102,53,35,89,233,73,76,178,56,
1,153,101,228,179,77,216,227,236,223,133,170,220,175,140,163,186,57,203,216,7,197,235,181,
221,254,224,62,84,115,223,180,223,125,184,127,127,79,26,185,236,42,129,175,148,154,31,220,
223,53,132,60,56,61,216,170,238,52,157,142,247,221,7,250,222,0,43,230,142,3,233,173,
35,210,235,244,122,224,16,59,138,225,149,235,167,27,199,101,181,74,202,166,206,90,71,178,
165,95,253,85,140,159,244,199,136,32,141,180,82,58,201,194,13,130,117,197,148,52,39,15,
61,119,58,217,223,56,202,159,106,197,85,125,107,166,148,47,232,125,210,151,225,103,26,200,
174,160,126,102,164,193,212,143,147,180,63,153,251,129,103,143,186,187,249,114,33,147,196,133,
73,47,163,196,167,105,15,224,190,38,151,215,67,102,138,230,251,190,239,135,158,124,55,216,
47,56,34,199,68,147,173,168,33,249,166,202,82,143,239,185,195,213,28,14,146,189,136,28,
44,99,201,178,203,199,29,96,18,233,117,46,234,48,10,101,126,207,145,113,28,197,37,109,
184,127,111,127,124,119,127,227,5,159,237,48,246,45,199,185,241,210,117,121,69,60,47,15,
15,195,84,190,75,251,236,217,7,172,216,195,232,74,198,211,32,90,169,85,119,195,235,21,
172,83,110,60,4,78,63,72,214,182,6,144,85,39,217,130,181,184,98,4,198,3,63,28,
79,188,189,141,51,69,12,172,134,147,155,99,7,77,131,61,106,61,234,24,114,98,89,76,
163,226,102,221,178,175,223,252,247,133,244,124,183,83,120,138,131,123,160,221,93,219,193,49,
95,240,7,185,159,222,42,233,220,27,151,59,105,77,223,222,173,42,8,118,164,249,124,180,
63,177,69,124,47,143,197,66,71,163,146,242,108,238,56,234,46,75,49,142,130,228,147,33,
155,245,131,86,206,33,176,148,65,243,76,223,66,154,11,12,111,60,231,131,166,166,66,187,
87,139,85,59,184,16,249,155,69,190,206,213,180,236,97,54,181,249,232,33,2,57,77,217,
13,99,206,135,59,26,91,29,170,198,71,135,158,127,197,127,196,36,112,147,100,212,210,17,
191,117,244,236,229,143,199,207,78,197,119,167,175,63,254,203,249,233,207,167,175,223,28,63,
59,255,233,197,119,135,59,220,101,190,103,225,184,19,44,219,76,30,238,224,162,190,109,81,
172,176,5,40,9,125,13,205,93,94,154,150,240,61,124,101,42,173,35,64,194,49,124,9,
32,161,88,249,177,39,220,108,58,147,99,204,0,220,163,231,209,33,135,84,67,160,42,224,
150,152,70,113,113,89,15,119,116,182,140,93,196,38,67,66,173,2,15,107,181,140,150,164,
154,226,202,13,50,201,96,216,130,197,79,100,150,2,67,30,238,168,70,213,198,50,52,141,
241,237,232,52,156,5,126,98,53,222,81,3,226,11,51,111,196,180,163,151,225,206,33,25,
146,18,27,241,164,61,91,75,96,66,32,158,164,110,154,37,45,225,198,190,219,15,252,43,
92,90,34,8,164,196,51,19,2,102,86,102,133,133,217,63,250,248,151,177,140,19,127,50,
135,196,240,243,208,11,152,168,166,130,223,233,209,25,127,71,103,176,228,121,71,111,72,206,
51,25,128,153,80,252,253,207,255,27,55,60,34,29,48,231,138,242,157,67,8,118,161,40,
1,241,65,91,137,214,212,151,129,135,223,124,93,253,104,9,19,59,5,107,27,86,214,196,
97,163,230,187,185,143,109,149,116,133,236,191,85,153,205,155,103,199,47,196,215,226,133,76,
223,175,100,124,169,166,164,84,128,87,122,30,37,105,235,232,5,50,19,65,198,110,116,81,
203,153,161,9,51,199,237,68,44,255,57,243,99,176,6,243,10,100,56,67,214,210,186,187,
223,18,75,151,64,108,56,106,189,61,238,255,209,237,191,223,237,63,236,95,124,211,42,141,
148,36,196,29,241,211,231,225,58,103,103,63,60,233,54,12,197,13,183,13,85,34,185,242,
167,254,43,76,125,5,9,97,18,50,147,137,224,1,244,197,180,129,122,169,143,96,212,213,
90,230,191,201,210,39,17,60,168,76,113,61,148,171,126,113,207,98,228,224,46,230,28,32,
110,204,163,0,98,27,181,158,73,8,143,86,65,134,3,168,66,178,148,80,32,25,167,96,
200,240,34,198,114,142,92,141,20,252,176,198,213,36,144,110,252,6,172,25,150,12,18,108,
29,189,156,78,101,168,39,38,162,121,40,11,138,8,153,43,9,220,16,154,121,222,57,212,
241,18,54,170,2,36,235,170,79,233,155,248,225,85,255,216,139,97,27,100,199,250,110,157,
19,210,115,127,114,236,113,203,58,55,54,185,171,123,134,96,157,19,123,153,252,101,235,200,
30,188,54,38,53,176,59,204,16,191,86,238,117,235,232,59,245,165,161,75,222,164,164,98,
217,56,148,80,231,51,254,124,191,112,147,203,166,225,76,51,187,171,23,194,30,159,188,56,
235,159,201,24,147,105,232,197,45,96,213,70,192,150,105,219,6,247,252,127,156,159,215,172,
108,241,207,105,250,61,91,218,73,28,93,194,159,111,151,69,222,180,209,2,246,246,31,84,
52,15,9,133,179,119,240,192,217,115,246,119,203,83,226,34,195,209,171,102,51,80,21,8,
181,186,97,182,128,203,179,7,244,97,202,123,172,241,208,245,251,247,239,222,111,213,166,243,
83,130,46,71,39,50,204,210,247,48,125,88,244,150,201,112,195,146,241,220,171,218,89,52,
157,214,7,168,26,54,9,246,38,195,46,245,249,173,134,93,151,239,239,98,217,207,193,91,
221,150,74,51,18,128,73,83,8,178,217,132,128,116,252,137,238,113,226,38,126,194,23,26,
164,160,26,54,234,206,195,3,240,183,52,193,130,51,40,168,188,47,181,211,23,50,158,125,
252,53,124,15,58,243,24,129,140,156,116,232,136,99,76,76,124,31,45,100,255,56,161,113,
93,0,222,83,128,237,248,227,175,232,131,152,66,65,111,44,253,133,248,248,47,112,0,113,
40,34,10,36,207,62,254,27,121,9,186,21,75,63,244,103,233,80,32,11,248,248,183,88,
44,178,36,225,96,51,70,144,71,6,62,35,14,200,42,192,65,76,210,29,187,49,128,158,
31,58,135,59,203,38,59,51,83,160,146,65,139,141,46,7,83,253,239,36,24,75,165,178,
192,234,100,127,142,226,185,75,94,74,10,221,142,150,36,100,239,197,11,72,53,53,112,37,
222,103,98,239,192,180,113,16,211,255,151,10,249,194,29,79,99,64,139,240,239,127,254,11,
102,182,146,179,84,77,23,115,37,2,169,184,4,223,186,163,98,223,128,18,133,209,147,86,
3,198,211,169,48,110,41,36,174,21,69,253,80,232,14,145,191,117,244,141,38,44,230,126,
248,62,131,44,103,164,44,170,89,142,137,154,189,210,177,230,27,80,224,141,47,193,49,212,
27,16,145,250,87,92,85,50,113,195,51,9,56,232,145,67,116,51,90,22,159,226,207,85,
20,4,73,10,13,241,124,34,116,134,118,9,130,184,188,204,72,162,141,129,220,34,85,113,
53,37,15,115,119,119,183,208,216,18,51,112,146,54,59,63,80,38,1,200,24,148,153,137,
111,195,76,153,84,19,59,7,187,154,159,7,72,25,182,115,148,198,62,45,227,115,247,157,
15,173,146,53,121,54,140,172,186,52,141,105,134,220,43,141,183,101,17,207,216,243,88,107,
8,156,154,202,32,48,171,88,85,247,211,119,228,229,37,121,22,229,159,88,59,53,126,248,
248,111,184,21,59,226,132,76,247,135,197,146,253,88,32,253,49,217,67,97,39,127,204,102,
192,229,137,231,82,119,12,201,116,134,34,132,75,46,223,195,2,76,230,129,47,63,254,21,
56,128,84,38,101,133,43,140,224,246,202,46,223,169,208,245,15,103,47,95,8,245,195,135,
23,177,116,125,107,87,127,97,117,85,63,42,93,149,153,88,216,131,27,61,245,3,105,22,
104,202,223,221,201,68,46,83,240,187,92,6,254,196,37,166,119,126,73,162,176,231,208,223,
22,108,208,131,128,26,132,14,251,215,226,132,133,6,65,10,103,18,126,252,117,50,79,82,
17,102,49,208,189,76,96,77,79,145,13,100,129,139,5,248,174,136,41,42,99,163,85,133,
118,164,240,145,11,49,139,63,254,13,49,65,156,168,154,91,213,35,170,217,84,252,162,74,
235,201,217,3,216,79,230,76,231,76,143,17,34,171,112,99,242,84,94,225,249,177,154,2,
134,35,150,24,108,154,42,164,73,191,41,220,56,135,227,248,232,12,75,11,127,175,58,189,
162,86,148,95,78,229,60,232,41,158,233,58,105,141,114,231,63,70,225,212,159,101,49,75,
13,13,180,178,34,122,146,186,106,199,168,215,80,243,172,171,146,102,13,0,209,32,1,32,
185,156,237,175,153,75,197,188,189,156,185,129,236,152,100,138,190,66,186,85,15,232,193,224,
16,191,20,83,58,181,171,230,78,42,161,175,89,48,192,195,252,52,228,164,172,30,197,77,
0,135,250,3,10,9,247,50,245,175,180,206,213,227,184,75,92,84,145,13,179,118,19,180,
41,247,250,60,108,227,135,5,182,169,230,48,117,214,30,211,202,197,139,98,86,102,245,162,
160,209,183,149,58,125,14,99,55,216,48,66,41,60,105,49,93,157,6,87,202,83,53,193,
27,43,178,148,163,106,155,8,226,161,231,198,222,226,227,175,31,255,234,207,84,54,117,28,
46,160,55,208,103,7,14,109,33,142,243,245,35,39,86,96,60,50,200,189,253,191,255,249,
95,15,238,138,63,242,72,220,96,38,201,97,146,177,88,130,34,175,90,224,226,129,96,41,
105,219,53,193,191,36,123,18,223,202,141,129,102,158,234,111,253,159,150,228,87,69,199,25,
251,141,241,44,239,210,232,181,168,215,77,50,206,150,65,228,222,32,91,195,134,152,71,48,
125,215,147,55,72,245,69,166,32,130,88,50,74,246,164,200,123,43,47,22,39,50,32,41,
189,130,237,114,249,27,226,37,255,145,39,141,37,135,80,103,22,161,155,204,158,146,167,220,
93,213,29,129,130,63,85,16,180,195,165,33,228,197,190,27,68,51,165,95,74,93,217,15,
88,18,48,245,194,123,15,118,243,162,251,39,55,133,184,148,216,176,3,86,218,121,81,24,
245,196,79,177,158,99,73,176,37,245,103,69,208,174,242,116,46,223,81,114,250,169,184,105,
117,123,236,134,19,25,180,202,162,3,230,27,199,146,244,180,22,55,173,174,199,172,48,173,
45,110,216,144,58,177,185,174,73,156,36,75,222,118,18,251,203,244,232,14,40,39,148,206,
112,17,111,180,110,109,217,177,110,13,68,249,86,217,67,183,122,184,173,202,142,212,242,153,
169,51,226,242,182,226,42,183,123,249,248,248,153,120,114,250,243,15,143,79,197,243,227,23,
199,223,157,62,63,125,113,78,221,182,86,70,169,31,124,88,72,74,3,190,208,180,168,1,
210,189,151,80,84,218,181,167,59,213,90,31,143,9,83,66,71,254,73,77,202,37,55,106,
241,198,239,63,245,57,132,165,112,38,151,212,170,82,114,163,86,70,189,113,199,208,177,75,
101,5,161,208,186,72,164,234,133,47,106,251,66,174,132,106,159,123,93,18,222,231,228,179,
52,57,233,94,1,66,210,6,146,72,35,32,72,185,20,233,92,138,4,151,189,18,229,219,
20,170,136,226,79,137,4,90,20,17,101,94,122,62,74,44,88,150,116,30,193,191,185,37,
186,77,21,44,86,30,174,84,225,34,252,171,170,86,85,26,55,213,167,114,6,68,98,186,
95,221,179,9,148,199,168,16,183,234,74,204,1,255,22,244,155,110,23,229,35,186,137,95,
200,98,249,87,143,214,214,174,251,168,213,230,156,215,34,111,7,13,205,103,108,148,161,161,
6,98,214,152,46,150,4,182,165,174,64,237,95,203,69,132,229,108,238,82,20,22,168,41,
183,25,187,144,149,186,68,19,252,175,92,44,32,158,207,177,238,106,35,68,0,183,99,193,
19,225,167,9,219,139,3,220,138,20,60,66,18,79,12,138,156,65,18,16,229,103,130,2,
22,85,107,66,232,117,182,20,43,98,72,97,17,79,68,49,210,51,18,157,55,100,237,95,
26,90,99,197,12,216,36,179,65,35,56,44,2,138,14,235,75,181,44,81,246,121,166,36,
64,190,233,63,183,50,193,235,235,134,104,47,146,108,201,89,162,167,217,97,207,159,208,112,
144,1,12,30,163,105,62,105,180,191,188,70,146,138,96,32,212,118,200,223,255,252,63,133,
23,161,121,136,233,176,102,65,80,9,79,16,22,166,250,177,36,154,74,23,196,196,55,240,
250,102,100,106,183,181,78,193,146,67,2,31,146,163,253,90,152,156,154,148,242,246,53,10,
34,242,52,11,2,65,165,0,225,153,4,165,147,168,170,0,59,211,91,215,24,202,196,124,
221,173,76,108,75,145,64,137,31,183,128,55,173,153,108,207,239,217,95,0,101,96,73,104,
238,48,210,152,197,245,95,34,195,39,230,20,35,176,212,136,85,129,118,19,129,187,114,23,
3,221,81,12,208,234,193,118,216,47,38,200,237,97,130,116,105,18,99,190,33,29,120,75,
134,130,247,139,49,218,53,120,88,217,183,132,59,77,201,30,136,30,205,190,90,31,40,24,
17,116,43,111,98,213,1,216,167,47,74,77,254,99,19,246,98,64,154,103,20,6,215,2,
72,29,115,73,231,176,102,202,83,29,65,241,136,156,202,44,150,228,3,53,12,142,56,196,
170,88,171,119,12,29,5,28,254,221,41,61,49,117,76,178,164,33,64,184,199,163,104,199,
169,209,54,164,141,230,156,230,38,58,76,187,121,127,82,214,223,183,36,192,114,154,50,31,
60,166,152,210,46,75,175,236,108,39,37,34,144,159,54,5,79,113,212,92,43,96,207,65,
146,252,218,76,141,26,111,205,218,89,137,56,215,207,181,23,12,68,169,202,40,138,80,92,
78,218,77,44,230,76,175,20,89,155,18,105,21,137,151,210,77,27,155,86,19,218,124,6,
91,24,250,255,155,214,242,212,35,129,28,0,116,243,125,135,241,53,116,116,234,102,88,90,
113,30,193,166,149,68,115,190,123,185,153,123,232,196,162,40,80,159,136,166,57,83,116,148,
22,185,15,108,140,13,165,60,52,38,223,156,48,179,115,54,89,104,86,186,99,245,41,178,
91,134,91,156,19,139,60,177,166,165,254,247,167,182,156,64,144,217,103,176,114,42,134,64,
121,73,20,102,24,6,189,26,81,47,104,32,77,8,24,253,26,88,216,81,224,177,154,247,
42,13,226,31,218,110,185,93,53,197,164,102,175,0,105,120,104,85,164,225,32,171,19,67,
78,124,84,238,200,240,179,212,239,113,209,65,195,13,131,75,159,40,160,96,160,233,243,227,
199,54,174,197,79,27,212,22,112,198,234,74,24,131,110,254,72,113,226,50,133,104,147,40,
214,131,166,88,109,161,175,48,87,192,50,16,105,82,52,122,30,177,128,172,54,64,88,79,
92,21,44,117,102,73,248,138,194,6,101,45,122,176,51,133,132,226,2,132,209,213,115,55,
209,215,148,199,230,28,201,100,126,207,232,104,90,97,243,52,78,213,226,105,160,173,249,213,
39,18,41,195,251,141,88,189,58,64,5,239,169,150,54,40,227,160,86,39,84,44,6,50,
223,132,35,136,137,105,197,184,143,224,234,227,75,104,162,10,106,97,45,214,56,22,103,28,
194,52,106,156,198,209,130,181,152,66,218,35,113,78,183,176,180,20,228,145,22,98,69,93,
43,222,56,197,60,244,52,196,42,67,110,160,60,15,96,253,28,94,227,20,193,97,28,250,
148,189,129,23,12,238,83,88,133,59,66,214,186,130,139,131,84,64,48,143,103,90,44,177,
22,5,254,102,136,11,2,70,2,31,148,160,31,144,26,0,250,50,27,83,89,130,32,253,
149,238,46,220,4,198,153,185,1,179,117,28,42,134,40,146,33,58,145,127,168,72,153,34,
147,244,42,21,133,114,115,180,182,106,10,208,85,213,7,215,180,46,9,160,14,163,174,52,
43,149,25,205,164,10,215,220,153,162,32,97,6,2,65,249,145,161,194,25,128,251,113,102,
50,90,125,109,162,6,148,121,74,39,66,210,228,114,91,190,238,33,71,182,27,19,62,200,
235,11,102,106,121,151,199,118,211,23,117,146,79,42,212,158,185,217,244,189,244,83,229,85,
83,95,249,137,167,88,88,194,209,90,149,216,69,3,241,136,5,180,41,190,182,124,69,225,
40,18,13,208,98,26,147,156,192,34,242,160,224,180,220,50,5,24,198,79,149,54,80,116,
163,156,197,156,134,184,231,236,49,200,129,244,161,168,149,27,212,65,81,121,57,157,90,34,
133,59,39,174,181,208,138,252,210,17,103,184,44,11,107,192,10,42,215,75,30,153,227,135,
99,175,66,134,158,97,74,46,63,116,175,176,240,156,15,66,19,57,188,48,160,33,5,83,
6,65,63,53,44,113,172,140,193,78,242,236,108,129,2,99,145,144,105,233,88,138,148,99,
6,142,221,245,184,241,40,199,18,70,165,116,4,86,243,41,96,224,35,149,90,165,210,167,
148,115,22,71,31,255,234,168,160,26,80,65,0,241,222,133,125,50,7,63,133,51,224,102,
138,27,73,25,230,37,228,11,92,141,237,66,36,71,190,87,129,112,166,65,47,71,229,186,
180,230,84,12,132,102,83,74,70,216,80,36,96,132,135,156,176,152,182,133,183,5,137,155,
92,0,91,82,121,190,86,82,193,16,51,100,172,205,124,84,193,16,141,252,126,5,49,248,
220,131,75,75,224,109,230,142,57,93,154,114,57,56,64,91,149,7,49,168,169,99,23,27,
231,88,177,94,121,117,231,6,64,154,110,7,93,175,234,104,80,227,44,175,25,42,85,134,
240,164,53,8,149,118,94,142,233,8,54,165,62,18,224,58,173,224,72,219,32,182,13,111,
142,46,2,243,17,13,18,198,212,157,16,53,222,153,72,18,134,62,166,204,199,144,209,6,
56,191,200,244,125,90,83,215,215,53,229,68,142,185,122,164,93,169,186,7,174,179,105,106,
12,21,64,84,166,43,87,236,221,23,38,93,175,152,170,166,73,177,168,102,152,232,237,142,
137,63,116,215,201,188,83,160,43,134,131,57,232,84,22,130,120,186,250,248,235,60,208,196,
53,226,210,167,100,221,2,234,209,30,141,211,12,64,89,207,170,51,175,32,82,219,84,31,
149,200,124,252,27,150,78,167,227,194,170,210,153,196,16,132,157,10,36,54,214,97,149,204,
116,66,165,229,194,49,90,249,61,35,186,129,56,243,211,247,164,11,248,151,73,178,121,45,
205,146,4,7,16,161,178,246,132,150,60,82,107,220,220,245,135,134,150,249,170,114,1,171,
8,178,201,50,206,36,167,153,129,43,105,185,109,237,136,98,19,254,120,39,156,109,158,82,
177,56,154,17,20,117,84,124,41,202,221,149,140,52,231,106,40,206,136,222,138,219,24,111,
100,188,102,213,127,233,121,14,77,174,147,24,167,99,106,122,77,201,47,135,17,119,60,163,
163,226,228,98,48,199,87,102,98,205,137,241,171,230,4,216,100,191,58,148,152,42,84,156,
45,169,156,199,34,112,114,192,81,230,225,18,162,162,149,229,72,103,182,51,20,10,113,84,
196,207,2,143,75,57,249,202,114,49,198,38,226,104,4,111,57,124,35,222,2,119,76,51,
57,51,30,227,113,137,5,226,157,204,205,14,146,53,138,58,248,208,250,213,8,148,163,16,
87,122,154,117,139,110,105,11,214,53,228,80,85,210,117,37,249,74,105,24,69,147,177,188,
130,34,33,242,186,50,149,61,83,106,102,61,60,167,42,56,100,229,82,94,71,249,188,45,
45,86,60,130,245,165,122,133,42,161,43,26,99,137,80,33,85,65,153,96,157,142,225,61,
115,31,195,114,157,93,43,42,51,208,223,225,65,251,111,144,173,193,155,8,127,33,158,203,
100,158,135,157,20,235,136,47,62,101,157,88,77,134,97,152,204,117,255,68,215,209,41,253,
87,96,178,60,106,62,90,194,222,14,16,12,84,117,238,78,90,198,68,93,46,10,27,154,
170,52,158,45,111,88,35,49,165,74,44,105,82,194,90,128,47,175,163,32,160,237,209,190,
65,125,162,115,247,254,238,174,56,185,78,101,210,109,88,84,249,110,34,165,151,32,64,78,
37,157,223,231,222,185,174,168,206,99,221,249,6,27,171,232,119,98,202,117,77,26,206,35,
100,203,27,74,77,60,212,57,172,77,37,88,101,234,239,51,24,239,228,18,131,112,244,106,
28,130,42,158,42,128,165,69,158,214,32,201,42,219,69,157,177,137,106,94,31,172,51,219,
168,10,149,36,133,82,44,184,118,54,211,36,35,79,228,228,96,222,94,113,157,188,56,66,
199,181,20,183,220,153,171,67,183,229,83,157,2,24,83,52,179,28,55,207,58,64,86,64,
163,114,1,147,227,243,195,221,34,62,87,66,205,144,182,137,196,83,48,76,96,118,230,7,
166,128,118,83,93,177,240,211,156,248,49,39,12,247,138,192,192,156,216,113,97,11,46,87,
8,0,12,106,4,48,20,254,180,88,192,207,47,78,42,139,230,21,80,62,93,87,66,22,
72,112,77,180,213,109,222,103,156,123,151,49,237,153,89,107,147,111,211,179,134,165,226,16,
98,137,86,28,222,69,211,209,243,13,1,0,94,241,144,146,70,24,148,157,170,40,221,116,
189,107,114,8,156,50,195,134,131,160,9,45,22,156,55,1,63,141,174,75,211,201,171,26,
13,93,237,118,229,74,234,160,90,142,84,213,234,82,93,117,208,84,29,44,13,249,27,13,
233,38,68,254,27,72,54,212,107,183,5,100,114,233,193,140,1,118,145,91,179,25,101,18,
235,25,70,139,133,30,227,69,53,14,23,105,191,128,25,19,202,166,53,188,102,205,231,93,
250,74,117,188,236,5,134,55,20,233,139,163,14,70,7,105,141,149,59,24,50,50,160,223,
159,240,155,77,248,173,234,136,76,152,176,129,221,227,42,154,51,110,232,53,219,213,45,70,
254,41,28,203,75,151,87,173,12,83,200,225,25,119,241,83,120,137,108,34,172,136,52,111,
1,50,229,13,178,124,167,237,44,197,42,82,193,32,87,210,199,214,86,149,217,1,51,15,
46,86,20,149,158,240,80,233,217,30,20,249,238,190,56,201,104,195,137,146,202,158,248,163,
63,157,230,123,227,39,132,76,146,148,42,32,170,20,80,235,170,222,32,0,119,228,249,51,
218,237,134,41,207,175,151,96,74,13,69,199,68,138,97,40,116,214,211,214,82,27,14,175,
121,37,223,201,143,162,20,22,26,72,3,138,30,212,77,181,124,228,100,160,43,158,81,222,
182,34,9,6,98,52,39,229,38,20,89,202,62,223,103,252,72,160,147,215,172,232,17,52,
227,56,20,128,161,93,7,237,68,10,50,124,98,128,246,56,232,56,6,31,159,97,146,156,
183,149,206,132,228,167,6,44,234,122,27,223,165,206,234,152,13,6,178,143,136,20,131,209,
131,53,138,116,73,65,154,33,168,58,112,17,233,60,165,208,147,130,28,35,61,181,6,15,
15,140,64,123,42,7,135,81,128,97,160,199,179,201,28,33,194,108,186,50,209,180,212,175,
16,110,175,72,179,165,178,83,18,89,12,27,162,239,73,224,2,10,171,225,21,176,101,197,
6,227,197,4,100,105,89,85,43,173,214,73,158,225,85,86,179,66,75,213,108,73,93,240,
113,60,134,193,135,72,162,155,232,33,121,81,138,146,200,25,60,93,234,108,63,204,67,193,
27,126,196,24,157,170,238,212,15,243,232,217,209,182,189,86,109,64,71,136,37,223,224,31,
136,131,93,92,230,211,254,116,163,188,71,63,16,116,115,111,215,49,39,15,74,68,204,118,
127,149,134,222,205,47,58,215,74,122,132,235,9,128,112,112,150,227,126,29,192,52,214,250,
168,11,185,187,149,28,55,120,58,77,55,204,181,221,218,147,217,162,145,94,177,137,67,154,
97,237,218,212,40,242,190,49,238,47,50,37,126,73,46,41,3,193,194,132,172,45,33,85,
235,178,232,225,138,177,171,44,244,129,50,42,206,57,207,183,210,235,101,201,37,91,7,82,
108,158,150,38,200,211,238,60,167,28,39,81,10,217,47,180,156,225,194,108,197,213,172,229,
71,17,248,156,15,111,196,23,71,95,224,14,109,34,16,62,136,84,84,187,186,81,163,52,
188,30,5,236,253,154,66,191,183,196,1,43,68,85,54,177,74,146,168,236,102,53,100,14,
185,215,163,100,183,255,114,252,139,188,76,139,229,121,92,109,171,189,28,39,205,17,26,79,
82,157,227,188,75,167,18,16,134,125,177,218,197,120,174,61,46,189,177,65,240,25,123,181,
141,241,157,27,190,127,239,206,131,166,182,100,30,51,25,171,134,165,39,74,21,163,41,12,
120,103,10,255,103,121,225,90,51,230,177,104,89,6,120,76,134,216,101,2,67,68,139,133,
56,209,187,109,48,146,153,218,129,171,128,78,67,19,221,134,34,162,29,92,63,45,29,106,
204,15,164,148,128,160,214,187,0,247,164,29,165,178,197,20,205,192,139,156,35,97,64,208,
47,44,220,214,60,234,103,199,173,92,39,22,84,16,128,30,133,213,154,190,38,82,172,132,
77,142,151,160,4,160,75,154,89,22,76,93,35,109,33,112,239,103,209,37,157,80,66,38,
77,71,197,50,1,199,165,79,17,125,239,198,30,87,11,115,64,234,168,227,174,19,88,25,
227,46,64,81,106,30,70,98,110,218,230,96,212,41,54,139,102,132,250,244,94,209,11,48,
51,165,179,210,229,173,3,83,89,108,168,9,218,213,29,213,186,90,220,209,149,0,230,127,
80,5,214,139,229,148,114,31,74,119,19,189,205,88,153,222,76,210,33,47,74,96,243,98,
155,92,13,106,197,182,137,244,57,179,124,97,205,118,229,38,230,140,160,99,109,176,126,78,
10,183,25,222,217,217,17,255,164,207,75,255,19,101,142,126,248,139,194,243,106,87,53,118,
67,196,107,70,174,252,240,17,29,174,224,173,86,93,205,221,49,107,49,206,124,122,104,213,
209,71,176,205,187,25,126,148,215,98,36,218,234,92,222,24,202,98,110,180,135,119,0,30,
243,118,104,20,2,226,207,92,192,235,252,213,27,78,26,61,139,144,226,60,70,234,223,233,
58,234,108,208,27,64,139,78,219,147,237,174,120,36,232,83,12,68,91,134,160,71,197,129,
245,29,33,20,7,186,62,57,2,78,131,206,156,169,42,142,51,147,233,15,169,92,116,44,
254,186,67,244,65,134,221,49,61,70,35,69,247,195,7,97,95,194,24,93,155,95,117,111,
120,103,35,38,46,29,58,236,252,169,43,214,98,231,15,84,185,91,37,180,225,172,43,71,
144,170,122,155,132,27,56,226,15,59,98,115,231,14,116,82,215,4,58,252,26,138,46,51,
78,76,228,3,124,161,199,36,54,200,241,34,36,115,83,117,131,80,121,56,3,67,136,250,
89,172,95,102,97,38,242,82,121,213,185,155,188,92,133,29,189,186,61,213,166,155,119,209,
215,223,242,229,11,234,11,93,160,147,126,116,196,142,21,220,164,135,49,244,91,208,73,67,
117,176,149,183,21,200,17,80,200,29,218,21,14,221,52,111,54,113,51,56,79,80,166,32,
215,81,235,2,133,153,250,239,8,97,188,109,87,125,126,187,39,218,85,223,78,215,110,216,
236,110,95,40,217,169,153,243,92,108,69,81,163,213,39,173,174,95,136,111,204,10,56,9,
50,18,169,219,59,234,65,165,46,171,198,6,255,202,82,198,2,66,84,63,146,231,134,84,
34,164,229,112,74,28,163,72,6,84,98,82,207,215,132,112,213,34,137,244,169,84,181,161,
78,59,255,210,163,92,137,95,202,193,20,147,30,209,131,149,199,210,165,252,71,27,41,169,
81,79,67,103,182,66,73,135,5,232,133,109,25,61,24,40,75,64,42,217,41,14,60,106,
27,68,162,58,243,161,116,236,145,71,92,68,127,35,221,203,231,238,178,131,137,149,219,188,
42,158,59,175,55,45,180,213,48,242,228,229,243,14,189,228,175,107,25,220,202,13,46,185,
183,23,77,50,198,209,106,62,231,177,148,111,248,30,119,233,193,137,121,242,169,79,199,97,
156,179,239,95,190,249,211,249,233,63,158,179,164,87,115,218,225,237,40,66,78,8,182,169,
105,167,107,86,88,141,195,82,29,233,225,28,189,227,77,13,135,185,26,80,19,103,233,210,
141,211,64,42,94,130,8,62,61,237,180,213,163,34,61,126,246,166,103,222,68,213,251,82,
237,67,245,190,84,245,209,222,151,213,167,98,218,224,130,162,167,31,102,122,160,178,0,193,
145,45,111,114,52,204,6,188,212,35,102,217,161,63,63,27,51,85,124,126,177,197,82,13,
37,7,86,190,192,252,171,67,151,70,74,244,72,69,183,174,106,85,30,213,98,208,137,37,
191,103,160,83,25,167,7,99,168,13,109,76,192,50,97,245,36,24,44,152,214,211,1,170,
142,175,207,120,195,49,138,143,131,160,211,126,107,189,197,224,162,93,89,190,6,129,89,202,
199,114,99,250,44,56,254,70,215,142,83,240,51,206,82,217,105,91,196,219,221,178,60,108,
66,137,33,84,147,139,34,154,108,37,106,139,193,204,223,246,217,244,88,238,181,121,40,168,
67,106,170,102,104,199,50,54,57,19,72,42,113,74,20,22,98,190,24,45,37,18,20,178,
52,165,82,91,72,65,55,59,185,254,193,235,180,243,72,218,117,174,244,2,219,253,74,182,
106,136,240,108,98,222,69,82,135,150,58,214,149,231,202,22,212,165,173,195,214,13,195,33,
215,247,88,189,210,12,76,164,157,106,147,46,187,204,219,76,36,10,21,162,1,25,120,74,
34,119,196,162,45,139,156,111,57,41,237,191,165,106,238,93,53,99,196,255,114,184,79,234,
225,190,151,75,169,59,20,181,216,253,178,28,168,1,144,232,213,91,250,233,178,252,49,179,
54,29,168,228,194,88,218,54,158,244,171,145,239,141,142,182,77,210,247,186,244,186,77,49,
73,226,233,168,221,238,113,113,116,20,102,65,208,27,103,201,245,136,211,12,3,137,168,206,
175,150,71,181,208,78,138,68,73,125,245,207,83,10,182,170,99,175,42,113,52,51,124,209,
226,60,229,199,147,71,111,219,84,105,106,247,218,244,154,35,124,152,247,189,232,175,116,110,
3,95,185,170,131,79,127,137,63,250,93,55,212,135,31,56,194,23,47,76,218,23,67,237,
139,249,133,2,57,121,74,105,169,105,241,182,3,252,42,189,253,128,127,115,157,2,52,44,
147,74,46,59,196,104,119,173,35,45,69,160,87,64,160,62,144,31,8,68,193,149,28,29,
173,213,160,234,169,191,209,87,101,93,132,47,168,73,129,179,173,175,62,165,179,163,84,141,
173,231,228,69,161,28,169,151,131,29,173,213,88,42,122,192,48,52,43,26,180,13,55,22,
113,117,152,85,233,48,160,196,229,168,211,37,117,8,101,135,151,168,107,53,85,15,62,54,
53,165,148,179,59,212,131,226,46,211,28,17,35,136,103,49,235,253,19,181,221,15,102,108,
226,27,211,41,153,71,171,231,145,231,6,104,176,193,127,133,140,43,70,190,6,63,90,147,
106,226,176,20,142,249,46,218,241,179,154,84,217,28,217,90,248,168,205,200,175,61,128,218,
89,35,234,38,157,164,39,11,93,237,174,109,117,78,134,37,109,230,118,195,170,59,218,220,
113,147,235,112,34,44,15,236,119,150,110,58,239,209,123,127,187,90,45,226,145,75,175,247,
4,66,132,69,171,219,24,42,157,71,222,128,154,193,27,83,34,56,69,250,229,61,106,127,
119,122,14,110,95,189,60,59,111,247,172,154,232,160,77,167,163,250,202,251,183,123,234,37,
111,73,189,255,122,51,88,183,181,188,250,231,64,231,32,86,125,91,3,84,253,31,251,143,
207,94,63,237,159,71,192,212,237,1,217,254,134,89,174,19,204,191,13,248,196,128,130,247,
254,244,186,195,51,220,228,186,233,166,174,158,102,204,249,24,164,227,79,59,95,196,78,116,
217,77,231,64,140,130,5,217,161,134,234,245,154,31,62,180,155,207,149,182,73,157,217,216,
168,113,73,83,162,85,199,27,173,125,111,64,222,198,157,208,7,165,32,131,54,50,184,118,
79,85,131,6,58,169,107,111,204,2,64,83,43,208,79,123,64,164,107,254,21,134,147,129,
165,63,109,5,94,219,116,213,71,6,17,127,127,254,252,217,168,109,191,6,193,58,251,173,
159,62,39,78,251,151,242,122,212,186,225,61,109,77,175,132,251,147,122,39,92,233,181,8,
71,86,165,178,70,31,211,222,242,26,172,111,43,111,105,58,62,30,156,156,12,30,63,30,
60,121,50,56,61,29,60,125,90,27,167,56,135,110,94,95,88,140,195,39,196,171,239,36,
132,152,91,71,39,81,186,237,157,133,0,189,84,198,105,29,61,86,95,182,181,131,250,211,
203,56,158,211,199,86,90,234,216,123,235,168,116,32,126,43,69,62,255,78,79,101,151,142,
198,111,107,190,12,178,89,235,232,21,254,138,231,126,232,127,242,181,138,214,226,87,106,146,
117,201,41,53,172,201,78,105,101,235,200,28,185,223,198,154,121,190,92,29,194,223,214,42,
224,83,248,71,207,212,59,25,183,114,175,62,234,39,245,107,106,181,220,250,38,141,138,14,
223,244,98,141,237,47,9,179,222,6,86,102,173,198,8,191,27,172,254,70,145,230,71,2,
114,98,183,127,153,14,143,164,218,64,201,37,189,127,174,242,244,192,39,94,169,163,135,81,
111,122,174,208,243,36,137,133,222,214,153,51,216,240,46,6,64,32,36,45,58,103,193,164,
85,205,65,33,31,151,16,14,13,136,15,165,72,237,139,46,60,81,41,153,65,38,83,8,
172,253,13,62,190,105,183,46,12,222,30,121,111,113,229,226,195,7,209,161,6,163,145,33,
244,200,120,70,248,77,0,83,248,103,207,49,139,247,133,229,244,111,30,47,95,110,107,192,
156,204,240,206,182,190,21,17,93,88,40,131,131,40,65,141,53,88,82,65,132,112,87,251,
119,120,50,3,73,158,164,180,146,158,205,160,128,253,105,254,88,37,154,185,3,142,95,107,
246,16,231,219,59,248,187,67,205,219,61,10,73,55,10,205,183,196,133,160,105,0,72,251,
119,124,224,3,75,186,225,164,161,83,64,153,142,116,76,33,65,33,184,205,198,192,111,245,
0,24,35,60,29,38,225,220,71,55,78,130,67,66,177,234,80,44,10,187,67,75,151,149,
38,27,101,40,84,184,215,102,179,190,133,42,107,69,54,135,166,71,95,96,8,44,219,237,
248,50,89,218,72,77,110,168,62,58,12,21,245,94,2,90,169,42,96,71,34,237,46,229,
162,116,193,2,27,244,72,106,103,210,93,87,166,87,164,45,221,175,96,96,70,24,147,183,
151,176,184,118,77,26,118,34,210,253,170,220,158,216,42,237,240,16,142,37,143,71,243,254,
98,226,148,238,149,231,160,11,37,143,233,189,234,64,165,152,98,103,162,95,121,157,124,248,
240,246,162,235,128,143,83,216,67,7,152,169,91,95,34,251,141,175,58,203,202,127,94,84,
249,228,121,81,150,96,94,200,106,241,169,242,67,115,147,222,233,88,187,105,201,116,18,5,
180,126,29,131,204,38,163,181,62,85,50,216,235,105,246,7,111,47,122,165,153,15,182,139,
105,51,220,190,58,196,249,168,152,201,141,11,195,109,95,240,149,78,209,197,150,219,91,174,
44,247,120,150,23,44,195,154,16,11,241,92,244,222,150,69,218,179,196,115,113,209,133,155,
251,170,195,87,242,153,128,5,30,225,130,50,100,73,155,128,220,136,175,25,118,242,38,229,
235,118,52,145,1,241,86,82,149,137,214,17,35,115,0,232,77,93,33,110,136,62,30,9,
231,150,118,171,24,226,200,194,81,83,251,8,51,246,114,244,153,161,133,72,221,216,69,193,
133,139,66,37,186,69,48,178,69,185,180,175,47,225,41,141,181,56,203,44,153,119,60,152,
190,78,58,38,245,180,142,78,117,65,103,201,255,215,210,157,34,20,168,189,49,74,247,169,
150,194,137,14,125,35,227,176,94,150,86,178,113,110,100,221,212,30,235,24,87,148,102,194,
176,85,69,70,145,227,17,134,236,153,248,50,123,54,245,142,183,146,211,84,132,145,171,185,
193,80,177,153,152,170,90,61,70,220,184,139,241,77,61,130,84,51,119,83,176,211,178,73,
70,118,161,136,179,192,132,147,122,85,201,174,229,244,237,173,239,32,106,119,141,135,204,154,
125,158,94,140,116,123,122,7,26,61,207,219,126,31,203,49,244,210,18,71,58,38,183,135,
158,87,229,180,250,162,163,10,131,58,176,120,41,134,52,57,236,112,179,125,230,137,90,161,
71,237,79,63,253,216,30,36,14,121,148,71,237,234,163,142,0,115,77,79,53,126,82,118,
150,107,187,236,93,25,151,70,135,218,218,189,196,33,215,246,168,253,179,53,72,229,17,71,
246,113,52,48,181,86,156,125,170,117,241,22,33,234,227,47,233,154,121,42,178,221,123,238,
166,115,103,26,68,96,43,113,50,126,64,114,231,96,183,251,77,155,94,215,199,253,43,207,
74,234,46,252,142,49,116,153,75,119,185,179,183,187,127,143,186,252,232,159,112,23,157,224,
210,120,218,220,233,106,237,225,73,186,239,46,31,181,155,30,153,196,100,142,209,2,94,123,
253,31,162,110,41,194,77,93,213,174,110,210,172,77,205,65,25,235,182,92,84,82,243,79,
134,218,176,84,200,77,134,149,178,59,12,54,113,66,164,211,19,217,205,11,101,230,130,186,
203,106,219,109,116,61,228,36,44,31,67,214,79,117,228,156,82,251,119,120,174,180,93,184,
34,242,173,36,213,82,197,114,205,161,179,33,4,234,237,211,163,209,222,65,87,123,251,156,
175,134,135,76,205,56,84,118,226,220,129,150,68,63,50,201,67,170,55,114,170,12,65,108,
41,134,130,23,18,192,135,15,95,216,169,205,103,62,161,138,52,70,51,60,228,170,60,75,
185,88,235,152,31,232,222,22,144,122,57,234,218,18,46,152,154,17,132,162,101,156,254,109,
114,10,107,159,96,163,132,164,222,63,83,93,21,3,249,114,118,134,42,23,20,19,199,70,
82,197,85,27,66,13,139,128,109,144,45,237,106,168,166,69,104,55,49,33,139,131,209,79,
175,159,105,3,84,187,153,248,221,161,242,253,73,16,141,59,111,43,229,204,73,143,119,52,
246,187,23,189,181,42,41,214,202,166,155,174,161,238,110,181,112,23,230,229,58,243,88,78,
71,96,1,95,189,104,21,18,124,24,89,71,77,212,186,112,153,180,141,38,44,34,10,244,
50,61,135,215,139,178,180,67,242,34,246,161,76,209,165,197,62,104,118,123,123,187,187,187,
70,29,213,107,124,42,146,206,175,211,131,201,92,27,87,3,84,175,231,233,82,57,197,85,
115,156,142,170,205,233,73,205,228,237,238,5,135,244,169,209,71,252,152,58,244,127,251,57,
218,219,39,198,236,98,111,187,242,176,116,219,8,112,50,98,241,47,233,255,8,168,205,98,
202,30,144,116,20,20,39,142,78,11,190,24,141,246,96,58,199,113,236,94,59,126,194,159,
69,166,211,253,240,161,80,11,109,219,48,237,18,11,159,124,18,27,76,233,84,175,72,203,
127,167,7,175,111,157,146,231,142,188,142,255,172,252,211,247,20,86,47,189,250,22,72,221,
126,225,44,103,111,190,103,131,193,109,248,115,168,220,103,233,86,37,131,46,24,105,66,166,
212,189,252,98,218,173,101,19,243,254,75,197,210,54,142,122,57,66,103,191,110,207,210,206,
7,84,183,175,191,238,228,39,18,213,218,31,238,237,127,248,80,185,118,116,112,183,184,6,
109,50,132,141,184,76,110,85,86,154,223,254,192,59,217,127,197,13,23,229,144,181,102,61,
159,231,134,141,178,113,166,219,56,165,164,38,87,37,69,238,81,251,243,30,148,167,13,166,
223,237,177,247,91,235,56,230,163,159,164,222,86,250,43,69,200,173,143,191,91,177,176,169,
32,103,198,232,173,55,93,43,88,230,166,253,155,30,143,255,156,73,170,199,200,27,231,104,
121,86,115,78,113,155,95,45,128,201,173,30,177,55,80,165,42,197,91,60,84,111,201,211,
212,1,189,235,17,5,74,42,184,98,36,23,166,78,215,12,10,45,120,239,77,89,127,243,
25,151,17,133,133,79,74,59,160,237,29,229,75,218,249,54,232,173,247,59,215,91,119,46,
55,91,246,27,205,158,164,9,46,13,26,241,89,111,12,184,157,38,76,233,136,78,112,189,
110,20,78,1,149,202,167,71,138,227,31,170,222,64,128,192,60,152,209,201,109,132,209,116,
145,205,51,26,0,28,40,78,131,28,238,168,255,11,239,255,3,145,106,169,132,150,119,0,
0
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
  WiFi.mode(WIFI_AP_STA);adminAP=WiFi.softAP(name.c_str());adminAPAt=millis();
  Serial.printf("Setup WiFi: %s (open)\nSetup address: http://192.168.4.1\n",name.c_str());
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
