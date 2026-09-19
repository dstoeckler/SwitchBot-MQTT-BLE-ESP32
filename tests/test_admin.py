from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest
import sys

ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/'tools'))
from sync_admin import rendered
from test_regressions import block


def run_cpp(source, includes=()):
    compiler=shutil.which('g++')
    if not compiler: raise RuntimeError('g++ required')
    with tempfile.TemporaryDirectory() as temp:
        cpp=Path(temp)/'test.cpp';exe=Path(temp)/'test.exe'
        cpp.write_text(source,encoding='utf-8')
        subprocess.run([compiler,'-std=c++11','-Wall','-Wextra','-Werror','-I',str(ROOT/'admin'),*[v for p in includes for v in ('-I',str(p))],str(cpp),'-o',str(exe)],check=True)
        subprocess.run([str(exe)],check=True)


class AdminTests(unittest.TestCase):
    def test_generated_assets_current(self):
        self.assertEqual(rendered(),(ROOT/'Arduino IDE Files/SwitchBot-BLE2MQTT-ESP32.ino').read_text(encoding='utf-8'))

    def test_policy(self):
        run_cpp(r'''
#include "config_core.h"
#include <cassert>
int main(){
 admin::Config c;c.host="bridge";c.ssid="Home";c.wifiPassword="a-password";c.mqttHost="192.168.1.2";c.topic="switchbot/home";
 assert(admin::validate(c).empty());auto good=c;
 c.port=0;assert(!admin::validate(c).empty());c=good;c.retries=11;assert(!admin::validate(c).empty());
 c=good;c.scanSeconds=0;assert(!admin::validate(c).empty());c=good;c.host="<script>";assert(!admin::validate(c).empty());
 c=good;c.topic="house/#";assert(!admin::validate(c).empty());c=good;c.topic="house//room";assert(!admin::validate(c).empty());
 c=good;c.staticAddress=true;c.ip="999.1.1.1";assert(!admin::validate(c).empty());
 c=good;admin::Device d;d.id="desk";d.mac="AA:BB:CC:DD:EE:FF";d.type="bot";c.devices.push_back(d);assert(admin::validate(c).empty());
 auto configured=c;d.id="desk2";d.mac="aa:bb:cc:dd:ee:ff";c.devices.push_back(d);assert(!admin::validate(c).empty());
 c=configured;c.devices[0].mac="aa:bb:cc:dd:ee:fz";assert(!admin::validate(c).empty());
 c=configured;c.devices[0].type="new-device";assert(!admin::validate(c).empty());
 c=configured;c.devices[0].id="../other";assert(!admin::validate(c).empty());
 c=configured;c.wifiPassword="new-password";assert(!admin::cleanupNeeded(configured,c));
 c=configured;c.topic="other";assert(admin::cleanupNeeded(configured,c));
 c=configured;c.devices.clear();assert(admin::cleanupNeeded(configured,c));
 c=configured;c.devices[0].entity="button";assert(admin::cleanupNeeded(configured,c));
 assert(admin::ipv4("192.168.1.1")&&!admin::ipv4("192.168.1")&&!admin::ipv4("1.2.3.4.")&&!admin::ipv4("1.2.3.256"));
}
''')

    def test_codec_and_persistent_rollback(self):
        candidates=list((ROOT/'PlatformIO Files/SwitchBot-BLE2MQTT-ESP32/.pio/libdeps').glob('*/ArduinoJson/src'))
        if not candidates:self.skipTest('Install/build PlatformIO dependencies first for ArduinoJson codec and rollback tests')
        runtime=(ROOT/'admin/runtime.inc').read_text(encoding='utf-8')
        source=r'''
#include "config_json.h"
#include <cassert>
#include <map>
using String=std::string;
uint32_t now=0;
uint32_t millis(){return now;}
admin::Config adminConfig;
bool adminStorageOK=false,adminTrial=false,adminAP=false;
String adminToken,adminNotice,adminCandidate,otaPass;
uint32_t adminBootAt=0,adminConnectedAt=0,adminRestartAt=0,adminAPAt=0;
struct Store {
 std::map<std::string,std::string> values;std::string fail;
 bool begin(const char*,bool){return true;}
 std::string getString(const char*k,const char*f){auto it=values.find(k);return it==values.end()?f:it->second;}
 size_t putString(const char*k,const std::string&v){if(fail==k)return 0;values[k]=v;return v.size();}
 bool getBool(const char*k,bool f){auto it=values.find(k);return it==values.end()?f:it->second=="1";}
 size_t putBool(const char*k,bool v){if(fail==k)return 0;values[k]=v?"1":"0";return 1;}
 bool remove(const char*k){return values.erase(k)>0;}
} adminStore;
struct SerialMock{template<class...T>void printf(T...) {}} Serial;
int applied=0,published=0;
void adminApply(){++applied;}
void adminCaptureDefaults(){adminConfig=admin::Config();adminConfig.host="old";adminConfig.ssid="home";adminConfig.wifiPassword="wifi-secret";adminConfig.mqttHost="broker";adminConfig.mqttPassword="mqtt-secret";adminConfig.topic="switchbot";}
std::string adminRandom(){return "1234567890abcdefghij1234567890abcd";}
constexpr int WL_CONNECTED=3;
struct Wifi{int state=0;int status(){return state;}void softAPdisconnect(bool){}void mode(int){}}WiFi;
constexpr int WIFI_STA=1;
struct Client{bool connected=false;bool isConnected(){return connected;}}client;
struct Esp{int restarts=0;void restart(){++restarts;}}ESP;
void adminStartAP(){adminAP=true;}
void publishHomeAssistantDiscoveryESPConfig(){++published;}
struct Server{bool authorized=false;int status=0;String token="";bool authenticate(const char*,const char*){return authorized;}void requestAuthentication(){status=401;}String header(const char*){return token;}}server;
void adminError(int status,const char*){server.status=status;}
'''
        source+=block(runtime,'static void adminLoad()').replace('.substring(','.substr(')+'\n'
        source+=block(runtime,'static bool adminTick()')+'\n'
        source+=block(runtime,'static bool adminAuthorized(').replace('.isEmpty()', '.empty()')+'\n'
        source+=r'''
std::string encoded(const admin::Config&c){DynamicJsonDocument doc(12000);admin::encode(doc,c,true);std::string s;serializeJson(doc,s);return s;}
void boot(){adminTrial=false;adminRestartAt=adminConnectedAt=adminBootAt=0;client.connected=false;now=0;adminLoad();}
int main(){
 boot();assert(adminConfig.host=="old"&&!adminTrial&&otaPass.size()>=12&&applied==1);
 auto old=adminConfig;
 admin::Device d;d.id="desk";d.mac="aa:bb:cc:dd:ee:ff";d.type="bot";d.password="bot-secret";old.devices.push_back(d);
 DynamicJsonDocument doc(12000);admin::encode(doc,old,false);std::string publicJson;serializeJson(doc,publicJson);
 assert(publicJson.find("wifi-secret")==std::string::npos&&publicJson.find("mqtt-secret")==std::string::npos&&publicJson.find("bot-secret")==std::string::npos);
 admin::Config decoded;std::string error;
 assert(admin::decode(doc.as<JsonVariantConst>(),old,decoded,error));assert(decoded.wifiPassword=="wifi-secret"&&decoded.devices[0].password=="bot-secret");
 doc["wifiPassword"]="";doc["devices"][0]["password"]="";assert(admin::decode(doc.as<JsonVariantConst>(),old,decoded,error));assert(decoded.wifiPassword.empty()&&decoded.devices[0].password.empty());
 doc["port"]=true;assert(!admin::decode(doc.as<JsonVariantConst>(),old,decoded,error));
 admin::encode(doc,old,false);doc["devices"][0]["mac"]="aa:bb:cc:dd:ee:11";assert(admin::decode(doc.as<JsonVariantConst>(),old,decoded,error));assert(decoded.devices[0].password.empty());
 admin::encode(doc,old,false);doc["devices"].add(42);assert(!admin::decode(doc.as<JsonVariantConst>(),old,decoded,error));
 auto next=old;next.host="new";
 adminStore.values["active"]=encoded(old);adminStore.values["pending"]=encoded(next);boot();
 assert(adminTrial&&adminConfig.host=="new"&&adminStore.getBool("trying",false));
 boot();assert(!adminTrial&&adminConfig.host=="old"&&adminStore.values.count("pending")==0); // power loss rolls back
 adminStore.values["pending"]=encoded(next);boot();now=76000;assert(adminTick());assert(adminRestartAt);
 now+=1001;adminTick();assert(ESP.restarts==1);boot();assert(adminConfig.host=="old"&&!adminTrial);
 adminStore.values["pending"]=encoded(next);boot();client.connected=true;now=100;adminTick();now=5101;adminTick();
 assert(!adminTrial&&adminStore.values["active"]==encoded(next)&&adminStore.values.count("pending")==0&&published==1);
 adminStore.values["active"]=encoded(old);adminStore.values["pending"]=encoded(next);boot();adminStore.fail="active";client.connected=true;now=100;adminTick();now=5101;adminTick();
 assert(adminRestartAt&&adminStore.values["active"]==encoded(old));adminStore.fail="";boot();assert(adminConfig.host=="old");
 assert(!adminAuthorized(true)&&server.status==401);
 server.authorized=true;assert(!adminAuthorized(true)&&server.status==403);
 server.token=adminToken;assert(adminAuthorized(true));adminTrial=true;assert(!adminAuthorized(true)&&server.status==409);
 adminTrial=false;adminRestartAt=100;assert(!adminAuthorized(true));
}
'''
        run_cpp(source,[candidates[0]])


if __name__=='__main__':unittest.main()
