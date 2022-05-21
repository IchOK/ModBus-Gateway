#define ARDUINOJSON_USE_LONG_LONG 1

#define DEBUG_NONE 0
#define DEBUG_ERR 1
#define DEBUG_SETUP 2
#define DEBUG_LOG 5
#define DEBUG_DIAG 4
#define DEBUG_SERIAL DEBUG_NONE

#include <ArduinoJson.h>
#ifdef ESP8266
  #include <ESP8266WiFi.h>
  #include <ESPAsyncTCP.h>
  #include <ESP8266mDNS.h>
#elif ESP32
  #include <WiFi.h>
  #include <AsyncTCP.h>
  #include <ESPmDNS.h>
  #include <FS.h>
  #include <SPIFFS.h>
#endif
#include <ESPAsyncWebServer.h>
#include <SPIFFSEditor.h>
#include <WiFiUdp.h>
//#include <ArduinoOTA.h>
#include <ModbusRTU.h>

//-------------------------------------------------------
// OTA Web-Upload
//-------------------------------------------------------
bool SystemReboot = false;

//-------------------------------------------------------
// Config and Data
//-------------------------------------------------------
String ConfigHostname;
uint32_t ConfigBaud;
bool ConfigStaticIP = false;

uint32_t UpdateRate;
uint32_t lastMillis;
uint32_t updateMillis;
uint32_t loopMillis;

DynamicJsonDocument JConfig(8000);
DynamicJsonDocument JDoc(2000);
JsonArray JSlaves;
JsonObject JData;

//-------------------------------------------------------
// Webserver
//-------------------------------------------------------
AsyncWebServer WebServer(80);
AsyncWebSocket WebSocket("/ws");

//-------------------------------------------------------
// ModBus Interface
//-------------------------------------------------------
#define RS485_RX D5
#define RS485_TX D6
#define RS485_DIR D7

#if defined(ESP8266)
 #include <SoftwareSerial.h>
 SoftwareSerial S(RS485_RX, RS485_TX);
#endif

ModbusRTU ModBus;

//#######################################################
// SETUP
//#######################################################
void setup() {
  #if DEBUG_SERIAL > DEBUG_NONE
    Serial.begin(115200);
  #endif
  #if DEBUG_SERIAL >= DEBUG_DIAG
    Serial.setDebugOutput(true);
  #endif
  
  //Init FileSystem
  SPIFFS.begin();

  //Read Json Config
  readConfigFile();

  //Init WiFi
  getWifiStation();

  //ModBus
  #if defined(ESP8266)
    S.begin(ConfigBaud, SWSERIAL_8N1);
    ModBus.begin(&S, RS485_DIR);
  #elif defined(ESP32)
    Serial1.begin(ConfigBaud, SERIAL_8N1);
    ModBus.begin(&Serial1, RS485_DIR);
  #else
    Serial1.begin(ConfigBaud, SERIAL_8N1);
    ModBus.begin(&Serial1, RS485_DIR);
    ModBus.setBaudrate(ConfigBaud);
  #endif
  ModBus.master();
  //ModBus-Data
  JData = JDoc.createNestedObject();

  //Init DNS
  MDNS.begin(ConfigHostname.c_str());
  MDNS.addService("http", "tcp", 80);
  
  //Init WebSocket
  WebSocket.onEvent(WsEvent);
  WebServer.addHandler(&WebSocket);
  
  //Init WebServer
  initWebServer();
  initConnect();
  initSys();
  WebServer.begin();
  
  lastMillis = millis();
  updateMillis = 0;
  loopMillis = 0;
}

//#######################################################
// LOOP
//#######################################################
void loop() {
  if (SystemReboot) {
    #if DEBUG_SERIAL >= DEBUG_DIAG
      Serial.println("Rebooting...");
    #endif
    delay(100);
    ESP.restart();
  }
  
  uint32_t actMillis = millis();
  uint32_t diffMillis = actMillis - lastMillis;
  lastMillis = actMillis;

  // Check WiFi State;
  getWifiStation();
  
  // Handle Websocket
  WebSocket.cleanupClients();
  updateMillis += diffMillis;
  if (updateMillis >= UpdateRate) {
    updateMillis -= UpdateRate;
    notifyClients();
  }

  // Handle ModBus
  if (!ModBus.slave()) {
    readNextData();
  }
  ModBus.task();
  
  yield();
}
