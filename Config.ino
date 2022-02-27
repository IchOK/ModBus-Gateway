//-------------------------------------------------------
// Config and Data
//-------------------------------------------------------
#define DEF_HOSTNAME "ModBusMaster"
#define DEF_BAUD 9600
#define DEF_UPDATE 1000
#define CONFIGPATH "/config.json"

//-------------------------------------------------------
// Read Config File
//-------------------------------------------------------
void readConfigFile() {
  ConfigHostname = DEF_HOSTNAME;
  ConfigBaud = DEF_BAUD;
  UpdateRate = DEF_UPDATE;
  File ConfigFile = SPIFFS.open(CONFIGPATH, "r");
  if (ConfigFile) {
    #if DEBUG_SERIAL >= DEBUG_SETUP
      Serial.println("Config File found ...");
    #endif
    DeserializationError error = deserializeJson(JConfig, ConfigFile);
    if (!error) {
      #if DEBUG_SERIAL >= DEBUG_SETUP
        Serial.println("Deserialize Done ...");
      #endif
      // Config Device
      if (JConfig.containsKey("device")) {
        #if DEBUG_SERIAL >= DEBUG_SETUP
          Serial.println("Found Device Config ...");
        #endif
        JsonObject JDevice = JConfig["device"].as<JsonObject>();
        if (JDevice.containsKey("hostname")) {
          ConfigHostname = JDevice["hostname"].as<String>();
        }
        if (JDevice.containsKey("baud")) {
          ConfigBaud = JDevice["baud"].as<uint32_t>();
        }
        if (JDevice.containsKey("update")) {
          UpdateRate = JDevice["update"].as<uint32_t>();
        }
      }
      //Get Slave Config
      if (JConfig.containsKey("slaves")) {
        JSlaves = JConfig["slaves"].as<JsonArray>();
      }
    }
  ConfigFile.close();
  #if DEBUG_SERIAL >= DEBUG_ERR
    } else {
      Serial.println("Config File NOT found ...");
  #endif
  }
}

//-------------------------------------------------------
// Update Config
//-------------------------------------------------------
size_t updateConfig(JsonObject& ConfigMsg) {
  bool DoWrite = false;
  //Update Slave-Config
  if (ConfigMsg.containsKey("slaves")) {
    #if DEBUG_SERIAL >= DEBUG_DIAG
      Serial.print("Update Slaves ... ");
    #endif
    JConfig["slaves"].clear();
    JConfig.garbageCollect();
    if (!JConfig["slaves"].set(ConfigMsg["slaves"])) {
      #if DEBUG_SERIAL >= DEBUG_DIAG
        Serial.println("Failed");
      #endif
      return 0;
    }
    JSlaves = JConfig["slaves"].as<JsonArray>();
    DoWrite = true;
    #if DEBUG_SERIAL >= DEBUG_DIAG
      Serial.print("Done [used:");
      Serial.print(JConfig.memoryUsage());
      Serial.println("]");
    #endif
  }
  
  //Update Device-Config
  if (ConfigMsg.containsKey("device")) {
    #if DEBUG_SERIAL >= DEBUG_DIAG
      Serial.print("Update Device ... ");
    #endif
    JsonObject JDevice = ConfigMsg["device"].as<JsonObject>();
    if (JDevice.containsKey("hostname")) {
      JConfig["device"]["hostname"] = JDevice["hostname"].as<String>();
    }
    if (JDevice.containsKey("baud")) {
      JConfig["device"]["hostname"] = JDevice["baud"].as<uint32_t>();
    }
    if (JDevice.containsKey("update")) {
      JConfig["device"]["hostname"] = JDevice["update"].as<uint32_t>();
    }
    JConfig.garbageCollect();
    DoWrite = true;
    #if DEBUG_SERIAL >= DEBUG_DIAG
      Serial.print("Done [used:");
      Serial.print(JConfig.memoryUsage());
      Serial.println("]");
    #endif
  }
  if (DoWrite) {
    //Write Config-File
    File ConfigFile = SPIFFS.open(CONFIGPATH, "w");
    size_t RetVal = serializeJson(JConfig, ConfigFile);
    ConfigFile.close();
    #if DEBUG_SERIAL >= DEBUG_DIAG
      Serial.print("File written [Bytes:");
      Serial.print(RetVal);
      Serial.println("]");
    #endif
    return RetVal;
  } else {
    return 0;
  }
}
