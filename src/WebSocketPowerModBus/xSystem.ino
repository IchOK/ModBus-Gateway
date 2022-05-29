//-------------------------------------------------------
// Config and Data
//-------------------------------------------------------
#define SYS_PATH "/sys"

//-------------------------------------------------------
// static Webpage - System
//-------------------------------------------------------
const char ArticleSys[] PROGMEM = R"rawliteral(
<article>
<header>System Information</header>
Firmware Version: %FW_VERSION%<br/>
Firmware Build: %FW_BUILD%<br/>
Board Name: %BOARD_NAME%<br/>
Board Version: %BOARD_VERSION%<br/>
Board Variant: %BOARD_VARIANT%<br/>
Board MCU: %BOARD_MCU%<br/>
</article>
<article>
<header>Save Config</header>
<a href="/config.json" download="config.json"><button>Download Config</button></a>
</article>
<article>
<header>Upload Web-Content</header>
<form method="POST" action="/upload" enctype="multipart/form-data">
<label for="jsonUpload">Choose a config file or web content:<input type="file" id="jsonUpload" name="jsonUpload" accept=".json, .htm, .html, .js, .css"></label>
<button type="submit">Upload</button>
</form>
</article>
<article>
<header>Firmware</header>
<form method="POST" action="/update" enctype="multipart/form-data">
<label for="update">Choose a Firmware file:<input type="file" id="update" name="update" accept=".bin"></label>
<button type="submit">Update</button>
</form>
</article>
<article>
<header>Reset Controller</header>
<form action="/reset" method="POST">
<button type="submit">Reboot</button>
</form>
</article>
)rawliteral";


String procSys(const String& var) {
  if(var == "TITLE"){
    return F("MB System");
  }
  if(var == "NAME"){
    return F("System");
  }
  if(var == "STYLE"){
    return F(":root{--ColorSystem:var(--contrast)}");
  }
  if(var == "SECTION"){
    return String(ArticleSys);
  }
  if(var == "FW_VERSION"){
    return String(VERSION);
  }
  if(var == "FW_BUILD"){
    return String(BUILDCOUNT);
  }
  if(var == "BOARD_NAME"){
    return String(ARDUINO_BOARD);
  }
  if(var == "BOARD_VERSION"){
    return String(ARDUINO_ESP8266_RELEASE);
  }
  if(var == "BOARD_VARIANT"){
    return String(BOARD_VARIANT);
  }
  if(var == "BOARD_MCU"){
    return String(BOARD_MCU);
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
// System init - call in Setup
//-------------------------------------------------------
void initSys() {
  WebServer.on(SYS_PATH, HTTP_GET, [](AsyncWebServerRequest *request){
    if(!request->authenticate(http_username, http_password)){
      return request->requestAuthentication();
    }
    request->send_P(200, "text/html", MainPage, procSys);
  });
  
  // File Upload to Server
  WebServer.on("/upload", HTTP_POST, [](AsyncWebServerRequest *request){
    request->redirect(SYS_PATH);
  },[](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final){
    if(!request->authenticate(http_username, http_password)){
      return request->requestAuthentication();
    }
    #if DEBUG_SERIAL >= DEBUG_LOG
      String logmessage = "Client:" + request->client()->remoteIP().toString() + " " + request->url();
      Serial.println(logmessage);
    #endif
    if (!index) {
      #if DEBUG_SERIAL >= DEBUG_LOG
        logmessage = "Upload Start: " + String(filename);
        Serial.println(logmessage);
      #endif
      // open the file on first call and store the file handle in the request object
      request->_tempFile = SPIFFS.open("/" + filename, "w");
    }
    if (len) {
      #if DEBUG_SERIAL >= DEBUG_LOG
        logmessage = "Writing file: " + String(filename) + " index=" + String(index) + " len=" + String(len);
        Serial.println(logmessage);
      #endif
      // stream the incoming chunk to the opened file
      request->_tempFile.write(data, len);
    }
    if (final) {
      #if DEBUG_SERIAL >= DEBUG_LOG
        logmessage = "Upload Complete: " + String(filename) + ",size: " + String(index + len);
        Serial.println(logmessage);
      #endif
      // close the file handle as the upload is now done
      request->_tempFile.close();
    }
  });

  // Firmware Update
  WebServer.on("/update", HTTP_POST, [](AsyncWebServerRequest *request){
    SystemReboot = !Update.hasError();
    AsyncWebServerResponse *response = request->beginResponse(301);
    response->addHeader("Location",SYS_PATH);
    response->addHeader("Retry-After","60");
    request->send(response);
  },[](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final){
    if(!request->authenticate(http_username, http_password)){
      return request->requestAuthentication();
    }
    if(!index){
      #if DEBUG_SERIAL >= DEBUG_LOG
        Serial.printf("Update Start: %s\n", filename.c_str());
      #endif
      Update.runAsync(true);
      if(!Update.begin((ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000)){
        #if DEBUG_SERIAL >= DEBUG_ERR
          Update.printError(Serial);
        #endif
        ;
      }
    }
    if(!Update.hasError()){
      if(Update.write(data, len) != len){
        #if DEBUG_SERIAL >= DEBUG_ERR
          Update.printError(Serial);
        #endif
        ;
      }
    }
    if(final){
      if(Update.end(true)){
        #if DEBUG_SERIAL >= DEBUG_LOG
          Serial.printf("Update Success: %uB\n", index+len);
        #endif
        ;
      } else {
        #if DEBUG_SERIAL >= DEBUG_ERR
          Update.printError(Serial);
        #endif
        ;
      }
    }
  });

  // Roboot System
  WebServer.on("/reset", HTTP_POST, [](AsyncWebServerRequest *request){
    if(!request->authenticate(http_username, http_password)){
      return request->requestAuthentication();
    }
    SystemReboot = true;
    AsyncWebServerResponse* response = request->beginResponse(301);
    response->addHeader("Location",SYS_PATH);
    response->addHeader("Retry-After","60");
    request->send(response);
  });

}
