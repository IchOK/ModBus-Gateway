//-------------------------------------------------------
// Config and Data
//-------------------------------------------------------
const char* http_username = "admin";
const char* http_password = "admin";
#define HOME_PATH "/home.htm"
#define CONFIG_PATH "/config.htm"
#define CONNECT_PATH "/connect"

//-------------------------------------------------------
// static Webpage - Main Page
//-------------------------------------------------------
const char MainPage[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<title>%TITLE%</title>
<link rel="stylesheet" type="text/css" href="/style.css">
<link rel="stylesheet" type="text/css" href="/styleMobile.css">
<link rel="stylesheet" type="text/css" href="/styleAddon.css">
<style>:root{--ColorHome:var(--secondary);--ColorConfig:var(--secondary);--ColorWiFi:var(--secondary);--ColorSystem:var(--secondary)}</style>
<style>%STYLE%</style>
</head>
<body>
<nav class="container-fluid">
<ul>
<li>%SVG_LOGO%</li>
<li>%NAME%</li>
</ul><ul>
<li><a href="/home.htm">%SVG_HOME%</a></li>
<li><a href="/config.htm">%SVG_CONFIG%</a></li>
<li><a href="/connect">%SVG_WIFI%</a></li>
<li><a href="/sys">%SVG_SYSTEM%</a></li>
</ul>
</nav>
<main class="container">
<section>
%SECTION%
</section>
</main>
</body>
</html>
)rawliteral";

//-------------------------------------------------------
// static Webpage - GetFile
//-------------------------------------------------------
String procFile(const String& var) {
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
// WebServer init - call in Setup
//-------------------------------------------------------
void initWebServer() {
  //Block access to WiFi-Config
  WebServer.on("/heap", HTTP_GET, [](AsyncWebServerRequest *request){
    AsyncWebServerResponse* response = request->beginResponse(403);
    response->addHeader("Location",CONNECT_PATH);
    response->addHeader("Retry-After","30");
    request->send(response);
  });

  //Path to Main-Page
  WebServer.on(HOME_PATH, HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, HOME_PATH, String(), false, procFile);
  });

  //Path to Config-Page
  WebServer.on(CONFIG_PATH, HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, CONFIG_PATH, String(), false, procFile);
  });
  
  WebServer.serveStatic("/", SPIFFS, "/").setDefaultFile(HOME_PATH);

  WebServer.onNotFound([](AsyncWebServerRequest *request){
    request->send(404);
  });
}
