# Einleitung
Das ModBus-Gateway ist ein konfigrierbares Modul zur Anbindung von ModBus-RTU (RS485) an Node-Red ober ähnliche Systeme die eine Websocket Kommunikation unterstützen.
Ziel ist es Herstellt unabhänig Daten Energie-Messdaten für Node-Red bereit zu stellten, jedoch ist die Anwendung nicht auf dieses begrenzt.
## Websocket und JSON
Das Modul sendet zyklisch an alle mit dem Websocket verbundenen Clients den kompletten Datensatz im JSON-Format.
Es ist zu beachten dass die verwendete Hardware nur eine begrenzte Anzahl Websocket-Verbindungen zur Verfügung stellt.
## WiFi
Als Netzwerkschnittstelle fungiert das standardmäsige WiFi des ESP. Dies hat den Vorteil dass kein extra Netzwerkkabel in den Verteilerschrank gezogen werden muss.
## Konfiguration
Alle Einstellungen können über die integrierte Webseite erfolgen.
# WiFi Konfiguration
Das Netzwerk lässt sich über eine eigene Webseite konfigurieren werden oder durch [hochladen der Konfiguration](#dateien-hochladen) als **wifi.json**.
```json
{
  "ssid":"your_ssid",
  "pass":"your_password",
  "dhcp":true,
  "ip":"x.x.x.x",
  "gateway":"x.x.x.x",
  "subnet":"x.x.x.x"
}
```
Die Konfiguration kann jedoch nicht zurück gelesen werden. Auf der WiFi-Webseite werden alle konfigurierten Informationen ausser das Passwort angezeigt.
## AP Mode
Wenn das Modul sich nicht mit dem angegebenen Netzwerk verbinden kann oder noch kein Netzwerk konfiguriert wurde, erstellt es einen SoftAP um die Konfiguration zu ermöglichen. Der SoftAP ist nicht für den Dauerbetrieb gedacht, denn nach einem 5 Minuten Timeout versucht das Modul erneut die Verbindung herzustellen. Dies soll dazu dienen dass sich das Modul selbstständig wieder mit dem Netzwerk verdinget auch wenn dieses zum Boot-Zeitpunkt nicht erreichbar war.  
> SSID: ModBus_*ESP-ChipID*  
> PASS: ModBusAdmin
# ModBus Konfiguration
Die Konfiguration der ModBus-Schnittstelle kann über die Webseite erfolgen oder durch [hochladen der Konfiguration](#dateien-hochladen) als **config.json**.
Die Modul spezifischen Einstellungen sind nur über die Konfigurationsdatei möglich, über die Webseite können nur die Slaves und Datenpunkte definiert werden.
## Modul
In der Modulkonfiguratio lassen sie der Hostname, die Baud-Rate und das Websocket-Updateinterval einstellen. Existiert keine Konfigurationsdatei werden die unten dargestellten Defaultwerte verwendet.
```json
  "device": {
    "hostname": "PowerGatewayServer",
    "baud": 9600,
    "update": 5000
  },
  "slaves": []
```
## Slaves
Der zweite Teil der Konfiguration definiert die ModBus-RTU Slaves und deren Datenpunkte.
```json
  "device": {},
  "slaves": [
    {
      "name": "Arbeitsplatz",
      "id": 1,
      "tags": [
        {
          "name": "Spannung",
          "type": "float",
          "be": true,
          "func": "ireg",
          "adr": 0,
          "unit":"V"
        }
      ]
    }
  ]
```
Der **name** des Slaves und der Datenpunkt ist frei wählbar und dient jeweis als Key im Aktualwert-Datensatz.

Die **id** definiert die ModBus-RTU Adresse des Slaves.

Da ModBus nur Register wird über den **type** der eigentliche Datentyp definiert. Das ewige Thema von Big-Ending und Little-Ending wird bei **be** angegenen, wobei true Big-Ending entspricht.

ModBus unterscheidet zwischen vier Funktionstypen, für **func** sind diese Werte gültig
- ists : Eingangs-Bit (1xxxxx)
- coli : Ein-/Ausgangs-Bit (2xxxxx)
- ireg : Eingangs-Register (3xxxxx)
- hreg : Ein-/Ausgangs-Register (4xxxxx)

Die Adresse wird in den meisten Dokumentationen als Adress-Register (z.B. 300001) angegeben, wobei es sich hierbei um das erste Register der Funktion 3xxxxx handelt. Beim Wert **adr** handelt es sich jedoch um den Offset, dieser beginnt bei 0.  
Das Adress-Register 30025 entsprich somit func = ireg, adr = 24.

# System Funktionen
Die System-Funktionen und auch die  WiFi Einstellungen sind Passwort geschützt. Das Passwort ist hart codiert und dient eher dazu dass keine Gäste im WLAN etwas "zerstören" können.  
> User: admin  
> Pass: admin
## Konfigration sichern
Hier läst sich die aktuelle ModBus Konfiguration als **config.json** herunterladen. Dies kann zur Datensicherung oder zum Editieren/Erweitern der Konfiguration dienen.
## Dateien hochladen
Über die Upload-Funktion können beliebige Dateien hochgeladen werden. Auf diesem Weg kann die Webseite oder deren Erscheinug angepasst werden oder auch Konfigurations-Dateien direkt geschrieben werden.
Es sollte darauf geachtet werden dass der Speicher des Kontrollers eingeschränkt ist, default sind 2MB.
## Firmware Update
Auf diesem Weg können einfach von der Arduino IDE exportierte Binarys hochgeladen werden und der Kontroller neu geflasht werden. Die Dateien und somit auch die Konfiguration sind davon nicht betroffen.
## Restart
Reboot tut immer Gut.
# Abhänigkeiten
Die verwendeten Bibliotheken können alle über den Libraray Manager der Arduino IDE bezogen werden
- [ArduinoJson](https://github.com/bblanchon/ArduinoJson)
- [ESPAsyncTCP](https://github.com/me-no-dev/ESPAsyncTCP) (ESP8266)
- [AsyncTCP](https://github.com/me-no-dev/AsyncTCP) (ESP32)
- [ESPAsyncWebServer](https://github.com/me-no-dev/ESPAsyncWebServer)
- [ModbusRTU](https://github.com/emelianov/modbus-esp8266)

Für das Designe der Webseite wurde auf das kompakte CSS-Framework [Pico.css](https://picocss.com/) zurück gegiffen. Wirklich ein einfachs und dennoch schön anzusehendes Framework mit dem sogar ein Nicht-Webentwickler wie ich gut klar kommt.

