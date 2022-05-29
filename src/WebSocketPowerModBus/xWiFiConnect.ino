//-------------------------------------------------------
// Config and Data
//-------------------------------------------------------
#define CONNECT_AP "ModBus"
#define CONNECT_PASS "ModBusAdmin"
#define WIFIPATH "/wifi.json"
IPAddress ConnectIP(192,168,1,1);
IPAddress ConnectGateway(192,168,1,1);
IPAddress ConnectSubnet(255,255,255,0);

#define PARAM_SSID "ssid"
#define PARAM_PASS "pass"
#define PARAM_IP "ip"
#define PARAM_GATEWAY "gateway"
#define PARAM_SUBNET "subnet"
#define PARAM_DHCP "dhcp"

#define WIFI_STATE_INIT 0
#define WIFI_STATE_CONNECT 1
#define WIFI_STATE_BUSY 2
#define WIFI_STATE_FAILED 3
#define WIFI_STATE_STA 4
#define WIFI_STATE_AP 5

#define WIFI_DELAY_FAILED 10000
#define WIFI_DELAY_RECONNECT 300000

String WiFiSSID;
String WiFiPass;
String WiFiIP;
String WiFiGateway;
String WiFiSubnet;
bool WiFiDHCP;

uint8_t WiFiState = WIFI_STATE_INIT;
uint32_t WiFiBusyTimer;
uint32_t WiFiReconnectTimer;

DynamicJsonDocument JWifiDoc(1000);
File ConfigFile;

bool ParamDHCP = false;
char ParamSSID[80];
char ParamPass[80];
IPAddress ParamIP;
IPAddress ParamGateway;
IPAddress ParamSubnet;

//-------------------------------------------------------
// static Webpage - WiFi
//-------------------------------------------------------
const char ArticleConnect[] PROGMEM = R"rawliteral(
<article>
<form action="/connect" method="POST">
<label for="ssid">SSID<input type="text" id ="ssid" name="ssid" value="%SSID%"></label>
<label for="pass">Password<input type="password" id ="pass" name="pass"></label>
<label for="dhcp"><input type="checkbox" id ="dhcp" name="dhcp" %DHCP%>DHCP</label>
<label for="ip">IP Address<input type="text" id ="ip" name="ip" value="%IP%"></label>
<label for="gateway">Gateway Address<input type="text" id ="gateway" name="gateway" value="%GATEWAY%"></label>
<label for="subnet">Subnet Address<input type="text" id ="subnet" name="subnet" value="%SUBNET%"></label>
<button type ="submit">Connect</button>
</form>
</article>
)rawliteral";


String procConnect(const String& var) {
  if(var == "TITLE"){
    return F("MB Connect");
  }
  if(var == "NAME"){
    return F("WiFi Connect");
  }
  if(var == "SSID"){
    return String(ParamSSID);
  }
  if(var == "DHCP" && ParamDHCP){
    return "checked";
  }
  if(var == "IP"){
    return ParamIP.toString();
  }
  if(var == "GATEWAY"){
    return ParamGateway.toString();
  }
  if(var == "SUBNET"){
    return ParamGateway.toString();
  }
  if(var == "STYLE"){
    return F(":root{--ColorWiFi:var(--contrast)}");
  }
  if(var == "SECTION"){
    return String(ArticleConnect);
  }
  if(var == "SVG_LOGO"){
    return String(SvgLogo);
  }
  if(var == "SVG_HOME"){
    return String(SvgHome);
  }
  if(var == "SVG_CONFIG"){
    return String(SvgConfig);
  }
  if(var == "SVG_WIFI"){
    return String(SvgWiFi);
  }
  if(var == "SVG_SYSTEM"){
    return String(SvgSystem);
  }
  return String();
}

//-------------------------------------------------------
// WiFiConnect init - call in Setup
//-------------------------------------------------------
void initConnect() {

  WebServer.on(CONNECT_PATH, HTTP_GET, [](AsyncWebServerRequest *request){
    if(!request->authenticate(http_username, http_password)){
      return request->requestAuthentication();
    }
    request->send_P(200, "text/html", MainPage, procConnect);
  });

  WebServer.on(CONNECT_PATH, HTTP_POST, [](AsyncWebServerRequest *request) {
    //Init Json-Doc
    JWifiDoc.clear();
    
    int params = request->params();
    for(int i=0;i<params;i++){
      AsyncWebParameter* p = request->getParam(i);
      if(p->isPost()){
        // HTTP POST ssid value
        if (p->name() == PARAM_SSID) {
          JWifiDoc[PARAM_SSID] = p->value();
          #if DEBUG_SERIAL >= DEBUG_SETUP
            Serial.print("initWiFiConnect - SSID set to: ");
            Serial.println(p->value().c_str());
          #endif
        }
        // HTTP POST pass value
        if (p->name() == PARAM_PASS) {
          JWifiDoc[PARAM_PASS] = p->value();
          #if DEBUG_SERIAL >= DEBUG_SETUP
            Serial.print("initWiFiConnect - Pass set to: ");
            Serial.println(p->value().c_str());
          #endif
        }
        // HTTP POST dhcp value
        if (p->name() == PARAM_DHCP) {
          if (p->value() == "on") {
            JWifiDoc[PARAM_DHCP] = true;
          }
          #if DEBUG_SERIAL >= DEBUG_SETUP
            Serial.print("initWiFiConnect - DHCP set to: ");
            Serial.println(p->value().c_str());
          #endif
        }
        // HTTP POST ip value
        if (p->name() == PARAM_IP) {
          JWifiDoc[PARAM_IP] = p->value();
          #if DEBUG_SERIAL >= DEBUG_SETUP
            Serial.print("initWiFiConnect - IP set to: ");
            Serial.println(p->value().c_str());
          #endif
        }
        // HTTP POST gateway value
        if (p->name() == PARAM_GATEWAY) {
          JWifiDoc[PARAM_GATEWAY] = p->value();
          #if DEBUG_SERIAL >= DEBUG_SETUP
            Serial.print("initWiFiConnect - Gateway set to: ");
            Serial.println(p->value().c_str());
          #endif
        }
        // HTTP POST subnet value
        if (p->name() == PARAM_SUBNET) {
          JWifiDoc[PARAM_SUBNET] = p->value();
          #if DEBUG_SERIAL >= DEBUG_SETUP
            Serial.print("initWiFiConnect - Subnet set to: ");
            Serial.println(p->value().c_str());
          #endif
        }
      }
    }

    //Write Config to File
    ConfigFile = SPIFFS.open(WIFIPATH, "w");
    serializeJson(JWifiDoc, ConfigFile);
    ConfigFile.close();
    
    WiFiState = WIFI_STATE_CONNECT;
    request->send(200, "text/plain", "Connect to Network");
  });
}

//-------------------------------------------------------
// WiFi Connection Handling
//-------------------------------------------------------
bool getWifiStation() {
  bool UseStation = false;
  char ConnectAP[40];
  switch (WiFiState) {
    case WIFI_STATE_INIT:
      //-----------------------------
      // Init Connection
      //-----------------------------
      WiFi.persistent(true);
      #if DEBUG_SERIAL >= DEBUG_SETUP
        Serial.print("getWifiStation - Begin ");
      #endif
      WiFiBusyTimer = millis();
      WiFiState = WIFI_STATE_CONNECT;
      break;
      
    case WIFI_STATE_CONNECT:
      //-----------------------------
      // Connect to new SSID
      //-----------------------------
      //Disable AP to change to Station
      WiFi.softAPdisconnect(true);
      WiFi.disconnect();
      delay(1000);
      
      // Get Wifi-Config from File
      ConfigFile = SPIFFS.open(WIFIPATH, "r");
      if (ConfigFile) {
        #if DEBUG_SERIAL >= DEBUG_SETUP
          Serial.println("WiFi File found ...");
        #endif
        DeserializationError error = deserializeJson(JWifiDoc, ConfigFile);
        if (!error) {
          #if DEBUG_SERIAL >= DEBUG_SETUP
            Serial.println("Deserialize Done ...");
          #endif
          JsonObject JWifi = JWifiDoc.as<JsonObject>();
          // Get Network Config
          if (JWifi.containsKey(PARAM_SSID) && JWifi.containsKey(PARAM_PASS)) {
            #if DEBUG_SERIAL >= DEBUG_SETUP
              Serial.println("Found WiFi Config ...");
            #endif
            strcpy(ParamSSID, JWifi[PARAM_SSID].as<const char*>());
            strcpy(ParamPass, JWifi[PARAM_PASS].as<const char*>());
            UseStation = true;
    
            ParamDHCP = false;
            //IP not exist or invalid, use DHCP
            if (JWifi.containsKey(PARAM_IP)) {
              if (!ParamIP.fromString(JWifi[PARAM_IP].as<const char*>())) {
                ParamDHCP = true;
              }
            } else {
              ParamDHCP = true;
            }
            //Gateway not exist or invalid, use DHCP
            if (JWifi.containsKey(PARAM_GATEWAY)) {
              if (!ParamGateway.fromString(JWifi[PARAM_GATEWAY].as<const char*>())) {
                ParamDHCP = true;
              }
            } else {
              ParamDHCP = true;
            }
            //Subnet not exist or invalid, use DHCP
            if (JWifi.containsKey(PARAM_SUBNET)) {
              if (!ParamGateway.fromString(JWifi[PARAM_SUBNET].as<const char*>())) {
                ParamDHCP = true;
              }
            } else {
              ParamDHCP = true;
            }
            //If DHCP ist set, force DHCP
            if (JWifi.containsKey(PARAM_DHCP)) {
              if (JWifi[PARAM_DHCP].as<bool>()) {
                ParamDHCP = true;
              }
            }
          }
          JWifiDoc.clear();
        }
        ConfigFile.close();
      }
      #if DEBUG_SERIAL >= DEBUG_ERR
      else {
        Serial.println("Config File NOT found ...");
      }
      #endif
      
      if (UseStation) {
        //Set static IP
        if (!ParamDHCP) {
          #if DEBUG_SERIAL >= DEBUG_SETUP
            Serial.println("initWiFiConnect - Set static IP");
          #endif
          if (!WiFi.config(ParamIP, ParamGateway, ParamSubnet)) {
            #if DEBUG_SERIAL >= DEBUG_ERR
              Serial.println("initWiFiConnect - Static IP failed");
            #endif
          }
        }
  
        // Connect to Network
        #if DEBUG_SERIAL >= DEBUG_SETUP
          Serial.print("getWifiStation - Change ");
        #endif
        WiFi.mode(WIFI_STA);
        WiFi.begin(ParamSSID, ParamPass);
        WiFiBusyTimer = millis();
        WiFiState = WIFI_STATE_BUSY;
      } else {
        WiFiState = WIFI_STATE_FAILED;
      }
      break;

    case WIFI_STATE_BUSY:
      //-----------------------------
      // Wait for Connection
      //-----------------------------
      if (WiFi.status() == WL_CONNECTED && WiFi.getMode() == WIFI_STA) {
        #if DEBUG_SERIAL >= DEBUG_SETUP
          Serial.println(" DONE");
          WiFi.printDiag(Serial);
        #endif
        WebServer.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
          request->redirect(HOME_PATH);
        });
        WiFiState = WIFI_STATE_STA;
      } else {
        delay(500);
        #if DEBUG_SERIAL >= DEBUG_SETUP
          Serial.print(".");
        #endif
        digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
        if (millis() - WiFiBusyTimer > WIFI_DELAY_FAILED) {
          #if DEBUG_SERIAL >= DEBUG_SETUP
            Serial.println(" FAILED");
          #endif
          WiFiState = WIFI_STATE_FAILED;
        }
      }
      break;

    case WIFI_STATE_FAILED:
      //-----------------------------
      // Connection Failt, setup AP
      //-----------------------------
      sprintf(ConnectAP,"%s_%08X", CONNECT_AP, ESP.getChipId());
      #if DEBUG_SERIAL >= DEBUG_SETUP
        Serial.print("getWifiStation - Start AP: ");
        Serial.println(ConnectAP);
      #endif
      WiFi.mode(WIFI_AP);
      WiFi.softAPConfig(ConnectIP, ConnectGateway, ConnectSubnet);
      WiFi.softAP(ConnectAP, CONNECT_PASS);
      WebServer.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->redirect(CONNECT_PATH);
      });
      WiFiState = WIFI_STATE_AP;
      WiFiReconnectTimer = millis();
      break;
      
    case WIFI_STATE_STA:
      //-----------------------------
      // Connected to STA
      //-----------------------------
      digitalWrite(LED_BUILTIN, HIGH);
      break;
      
    case WIFI_STATE_AP:
      //-----------------------------
      // Connected to AP
      //-----------------------------
      digitalWrite(LED_BUILTIN, LOW);
      if (WiFi.softAPgetStationNum() > 0) {
        WiFiReconnectTimer = millis();
      } else if (millis() - WiFiReconnectTimer > WIFI_DELAY_RECONNECT) {
        WiFiBusyTimer = millis();
        WiFiState = WIFI_STATE_CONNECT;
      }
      break;
      
  }
  return WiFiState == WIFI_STATE_STA;
}
