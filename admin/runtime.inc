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
