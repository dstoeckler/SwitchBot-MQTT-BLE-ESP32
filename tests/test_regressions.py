"""Execute actual firmware functions/callbacks with deterministic hardware doubles."""
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / 'Arduino IDE Files/SwitchBot-BLE2MQTT-ESP32.ino'


def block(source, signature):
    start = source.index(signature)
    brace = source.index('{', start)
    depth = 1
    end = brace + 1
    while depth:
        depth += (source[end] == '{') - (source[end] == '}')
        end += 1
    return source[start:end]


class Regressions(unittest.TestCase):
    def test_source_consistency(self):
        self.assertEqual(SOURCE.read_bytes(), (ROOT / 'PlatformIO Files/SwitchBot-BLE2MQTT-ESP32/src/SwitchBot-BLE2MQTT-ESP32.cpp').read_bytes())

    def test_firmware_behaviour(self):
        source = SOURCE.read_text(encoding='utf-8')
        ota = source[source.index('  static bool otaUploadAuthorized'):source.index('  server.begin();', source.index('  static bool otaUploadAuthorized'))]
        program = r'''
#include <cassert>
#include <cstdint>
#include <functional>
#include <string>
#include <algorithm>
#include <map>
using String = std::string;
bool processing = false, isRescanning = false;
uint32_t clockMs = 0;
unsigned long millis() { return clockMs; }
void delay(unsigned long ms) { clockMs += ms; }
int errors = 0;
const char *ESPMQTTTopic = "test";
void publishStatus(const char *, const char *) { ++errors; }
struct Scan {
  bool active = false;
  uint32_t ends = 0;
  bool isScanning() { if (ends && clockMs >= ends) active = false; return active; }
  void stop() { active = false; }
} scan, *pScan = &scan;
enum { HTTP_POST, UPLOAD_FILE_START, UPLOAD_FILE_WRITE, UPLOAD_FILE_END, UPLOAD_FILE_ABORTED };
struct HTTPUpload { int status = 0; String filename = "firmware.bin"; unsigned char buf[4] = {}; size_t currentSize = 4; size_t totalSize = 4; };
struct Server {
  bool authorized = false;
  String token="valid";
  int status = 0;
  HTTPUpload data;
  std::function<void()> completed, chunk;
  void on(const char *, int, std::function<void()> a, std::function<void()> b) { completed=a; chunk=b; }
  bool authenticate(const char *, const char *) { return authorized; }
  String header(const char *) { return token; }
  void requestAuthentication() { status=401; }
  void sendHeader(const char *, const char *) {}
  void send(int code, const char *, const char *) { status=code; }
  HTTPUpload& upload() { return data; }
} server;
void adminError(int code,const char *) { server.status=code; }
bool adminTrial=false,adminAuthEnabled=true;
unsigned long adminRestartAt=0;
String adminToken="valid";
struct Updater {
  int starts=0, writes=0, ends=0, aborts=0;
  bool success=true;
  bool begin(int) { ++starts; return success; }
  size_t write(unsigned char *, size_t n) { ++writes; return success ? n : 0; }
  bool end(bool) { ++ends; return success; }
  void abort() { ++aborts; }
  template<class T> void printError(T&) {}
} Update;
struct Esp { int reboots=0; void restart() { ++reboots; } } ESP;
struct SerialMock { template<class... T> void printf(T...) {} } Serial;
bool useLoginScreen=true, printSerialOutputForDebugging=false;
String otaUserId="test", otaPass="test";
constexpr int UPDATE_SIZE_UNKNOWN = -1;
unsigned long commandStartedAt=0;
const unsigned long commandBudgetMs=20000;
const int noResponseRetryAmount=5;
bool home_assistant_entity_names_include_device_name=false;
bool home_assistant_expose_seperate_curtain_position=false;
std::string home_assistant_mqtt_prefix="homeassistant", curtainTopic="test/curtain/";
std::string manufacturer="SwitchBot", curtainModel="Curtain";
const char *lastWill="test/availability";
std::map<std::string,std::string> published;
void addToPublish(const char *topic, const char *payload, bool) { published[topic]=payload; }
template<class T> void printAString(T) {}
struct Address { std::string toString() { return "aa:bb"; } };
struct NimBLEAdvertisedDevice { Address getAddress() { return {}; } };
int connectAttempts=0, clientCB=0;
constexpr int NIMBLE_MAX_CONNECTIONS=3;
struct NimBLEClient {
  bool connected=false;
  bool connect(NimBLEAdvertisedDevice*, bool=true) { ++connectAttempts; clockMs+=10000; return false; }
  bool isConnected() { return connected; }
  void setClientCallbacks(int*,bool) {}
  void setConnectionParams(int,int,int,int) {}
  void setConnectTimeout(int) {}
  Address getPeerAddress() { return {}; }
  int getRssi() { return -60; }
} bleClient;
struct NimBLEDevice {
  static int getClientListSize() { return 1; }
  static NimBLEClient* getClientByPeerAddress(Address) { return &bleClient; }
  static NimBLEClient* getDisconnectedClient() { return &bleClient; }
  static NimBLEClient* createClient() { return &bleClient; }
  static void deleteClient(NimBLEClient*) {}
};
'''
        program += block(source, 'class ProcessingScope') + ';\n'
        program += block(source, 'bool parseScanSeconds(') + '\n'
        program += block(source, 'bool waitForScanEnd(') + '\n'
        program += block(source, 'bool commandBudgetAvailable(') + '\n'
        program += block(source, 'bool busyRetryAllowed(') + '\n'
        program += block(source, 'bool connectToServer(NimBLEAdvertisedDevice * advDeviceToUse) {') + '\n'
        program += block(source, 'std::string haEntityName(std::string deviceName, const char * entityName) {') + '\n'
        program += block(source, 'void publishHomeAssistantDiscoveryCurtainConfig(') + '\n'
        program += 'void setupOTA() {\n' + ota + '\n}\n'
        program += r'''
void upload(int phase) { server.data.status=phase; server.chunk(); }
int main() {
  std::string curtain="curtain";
  publishHomeAssistantDiscoveryCurtainConfig(curtain,"aa:bb");
  auto light=published.at("homeassistant/sensor/curtain/illuminance/config");
  assert(light.find("\"dev_cla\"")==std::string::npos);
  assert(light.find("\"unit_of_meas\": \"Level\"")!=std::string::npos);
  assert(light.find("switchbot_AA:BB_illuminance")!=std::string::npos);
  int retries=0;
  for(int attempt=1; busyRetryAllowed(true,true,attempt); ++attempt) { ++retries; assert(retries<10); }
  assert(retries==5);
  assert(!busyRetryAllowed(false,true,1) && !busyRetryAllowed(true,false,1));
  commandStartedAt=UINT32_MAX-10000; clockMs=commandStartedAt;
  assert(commandBudgetAvailable()); clockMs+=19999; assert(commandBudgetAvailable());
  clockMs+=1; assert(!commandBudgetAvailable()); clockMs=0;
  commandStartedAt=0; NimBLEAdvertisedDevice missingDevice;
  assert(!connectToServer(&missingDevice)); // Both reconnect paths fail.
  assert(connectAttempts==2 && clockMs==20000);
  assert(!connectToServer(&missingDevice)); // Budget prevents a third attempt.
  assert(connectAttempts==2); clockMs=0;
  int seconds=99;
  for (auto text : {"", "0", "-1", "301", "999999999999999999999", "1.5", "true", "{}", "null"}) {
    assert(!parseScanSeconds(text, seconds)); assert(seconds==99);
  }
  assert(!parseScanSeconds(nullptr, seconds));
  assert(parseScanSeconds("1", seconds) && seconds==1);
  assert(parseScanSeconds("300", seconds) && seconds==300);
  auto earlyReturn=[] { ProcessingScope scope; assert(processing); return; };
  earlyReturn(); assert(!processing);
  processing=true; earlyReturn(); assert(processing); processing=false;
  scan.active=true; scan.ends=50; assert(waitForScanEnd(100)); assert(clockMs==50);
  scan.active=true; scan.ends=0; isRescanning=true;
  assert(!waitForScanEnd(100)); assert(!scan.active && !isRescanning && errors==1);
  clockMs=UINT32_MAX-20; scan.active=true;
  assert(!waitForScanEnd(100)); assert(!scan.active); // millis wrap
  setupOTA();
  upload(UPLOAD_FILE_START); upload(UPLOAD_FILE_WRITE); upload(UPLOAD_FILE_END); server.completed();
  assert(server.status==401 && Update.starts==0 && Update.writes==0 && Update.ends==0 && ESP.reboots==0);
  server.authorized=true;
  server.completed(); assert(server.status==400 && ESP.reboots==0); // POST without file
  upload(UPLOAD_FILE_START); upload(UPLOAD_FILE_WRITE); upload(UPLOAD_FILE_END); server.completed();
  assert(server.status==200 && Update.starts==1 && Update.writes==1 && ESP.reboots==1);
  upload(UPLOAD_FILE_START); upload(UPLOAD_FILE_ABORTED); server.completed();
  assert(server.status==400 && Update.aborts==1 && ESP.reboots==1);
  Update.success=false;
  upload(UPLOAD_FILE_START); upload(UPLOAD_FILE_WRITE); upload(UPLOAD_FILE_END); server.completed();
  assert(server.status==400 && ESP.reboots==1);
  Update.success=true; server.authorized=false;
  int starts=Update.starts;
  upload(UPLOAD_FILE_START); upload(UPLOAD_FILE_WRITE); upload(UPLOAD_FILE_END); server.completed();
  assert(Update.starts==starts && server.status==401 && ESP.reboots==1);
  server.authorized=true;server.token="wrong";
  upload(UPLOAD_FILE_START);upload(UPLOAD_FILE_WRITE);upload(UPLOAD_FILE_END);server.completed();
  assert(Update.starts==starts && server.status==403 && ESP.reboots==1);
  server.token="valid";adminTrial=true;
  upload(UPLOAD_FILE_START);upload(UPLOAD_FILE_WRITE);upload(UPLOAD_FILE_END);server.completed();
  assert(Update.starts==starts && ESP.reboots==1);
  adminTrial=false;adminAuthEnabled=false;server.authorized=false;server.token="wrong";
  upload(UPLOAD_FILE_START);upload(UPLOAD_FILE_WRITE);upload(UPLOAD_FILE_END);server.completed();
  assert(Update.starts==starts && server.status==403 && ESP.reboots==1);
  server.token="valid";
  upload(UPLOAD_FILE_START);upload(UPLOAD_FILE_WRITE);upload(UPLOAD_FILE_END);server.completed();
  assert(Update.starts==starts+1 && server.status==200 && ESP.reboots==2);
}
'''
        compiler = shutil.which('g++')
        self.assertIsNotNone(compiler, 'Install g++ to run firmware behaviour regressions')
        with tempfile.TemporaryDirectory() as temp:
            cpp = Path(temp) / 'regressions.cpp'
            exe = Path(temp) / 'regressions.exe'
            cpp.write_text(program, encoding='utf-8')
            subprocess.run([compiler, '-std=c++11', '-Wall', '-Wextra', '-Werror', str(cpp), '-o', str(exe)], check=True)
            subprocess.run([str(exe)], check=True)


if __name__ == '__main__':
    unittest.main()
