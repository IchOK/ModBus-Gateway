//-------------------------------------------------------
// Config and Data
//-------------------------------------------------------
JsonObject JSlave;
JsonArray JTags;
JsonObject JTag;

bool ModBusCoil;
uint16_t ModBusSlave = 0;
uint16_t ModBusTag = 0;
//Zur Konvertierung und für ByteSwap
union RawUnion{
  byte  RawByte[8];
  word RawWord[4];
  int16_t ValSigned16[4];
  uint16_t ValUnsigned16[4];
  int32_t ValSigned32[2];
  uint32_t ValUnsigned32[2];
  int64_t ValSigned64;
  uint64_t ValUnsigned64;
  float ValFloat[2];
  double ValDouble;
} ModBusRegs;

//-------------------------------------------------------
// Data-Handling
//-------------------------------------------------------
void readNextData() {
  #if DEBUG_SERIAL >= DEBUG_LOG
    Serial.printf("Station[%02d] - Tag[%02d]\r\n", ModBusSlave, ModBusTag);
  #endif
  if (JSlaves.size() >= ModBusSlave) {
    JSlave = JSlaves[ModBusSlave];
    if (JSlave.containsKey("tags") && JSlave.containsKey("id")) {
      uint16_t SlaveID = JSlave["id"].as<uint16_t>();
      #if DEBUG_SERIAL >= DEBUG_LOG
        Serial.printf("  [%03d] %s\r\n", SlaveID, JSlave["name"].as<String>());
      #endif
      JTags = JSlave["tags"].as<JsonArray>();
      if (JTags.size() >= ModBusTag) {
        JTag = JTags[ModBusTag];
        if (JTag.containsKey("func") && JTag.containsKey("adr") && JTag.containsKey("type")) {
          uint16_t TagAdr = JTag["adr"].as<uint16_t>();
          String TagFunc = JTag["func"].as<String>();
          String TagType = JTag["type"].as<String>();
          uint16_t TagLen = 0;
          if (TagType == "bool" || TagType == "word" || TagType == "int16" || TagType == "uint16") {
            TagLen = 1;
          } else if (TagType == "dword" || TagType == "int32" || TagType == "uint32" || TagType == "float") {
            TagLen = 2;
          } else if (TagType == "qword" || TagType == "int64" || TagType == "uint64" || TagType == "double") {
            TagLen = 4;
          }
          #if DEBUG_SERIAL >= DEBUG_LOG
            Serial.printf("  %s, (%s/%s), Adr:%d, Len:%d\r\n", JTag["name"].as<String>(), TagFunc, TagType, TagAdr, TagLen);
          #endif

          if (TagLen > 0) {
            if (TagFunc == "coil") {
              ModBus.readCoil(SlaveID, TagAdr, &ModBusCoil, TagLen, cbRead);
            } else if (TagFunc == "ists") {
              ModBus.readIsts(SlaveID, TagAdr, &ModBusCoil, TagLen, cbRead);
            } else if (TagFunc == "hreg") {
              ModBus.readHreg(SlaveID, TagAdr, ModBusRegs.RawWord, 4, cbRead);
            } else if (TagFunc == "ireg") {
              ModBus.readIreg(SlaveID, TagAdr, ModBusRegs.RawWord, 4, cbRead);
            } else {
              ModBusTag++;
            }
          } else {
            ModBusTag++;
          }
        } else {
          ModBusTag++;
        }
      } else {
        ModBusTag = 0;
        ModBusSlave++;
      }
    } else {
      ModBusSlave++;
    }
  } else {
    ModBusSlave = 0;
  }
}

//-------------------------------------------------------
// BigEnding Datatypes 32-Bit, swap Words
//-------------------------------------------------------
void bigEndingModBusReg32() {
  word SwapWord;
  SwapWord = ModBusRegs.RawWord[0];
  ModBusRegs.RawWord[0] = ModBusRegs.RawWord[1];
  ModBusRegs.RawWord[1] = SwapWord;
}

//-------------------------------------------------------
// BigEnding Datatypes 64-Bit, swap Words
//-------------------------------------------------------
void bigEndingModBusReg64() {
  word SwapWord;
  SwapWord = ModBusRegs.RawWord[0];
  ModBusRegs.RawWord[0] = ModBusRegs.RawWord[3];
  ModBusRegs.RawWord[3] = SwapWord;
  SwapWord = ModBusRegs.RawWord[1];
  ModBusRegs.RawWord[1] = ModBusRegs.RawWord[2];
  ModBusRegs.RawWord[2] = SwapWord;
}

//-------------------------------------------------------
// Byte-Swap
//-------------------------------------------------------
void byteSwapModBusReg() {
    byte SwapByte;
    SwapByte = ModBusRegs.RawByte[0];
    ModBusRegs.RawByte[0] = ModBusRegs.RawByte[1];
    ModBusRegs.RawByte[1] = SwapByte;
    SwapByte = ModBusRegs.RawByte[2];
    ModBusRegs.RawByte[2] = ModBusRegs.RawByte[3];
    ModBusRegs.RawByte[3] = SwapByte;
    SwapByte = ModBusRegs.RawByte[4];
    ModBusRegs.RawByte[4] = ModBusRegs.RawByte[5];
    ModBusRegs.RawByte[5] = SwapByte;
    SwapByte = ModBusRegs.RawByte[6];
    ModBusRegs.RawByte[6] = ModBusRegs.RawByte[7];
    ModBusRegs.RawByte[7] = SwapByte;
}

//-------------------------------------------------------
//-------------------------------------------------------
bool cbRead(Modbus::ResultCode event, uint16_t transactionId, void* data) {
  if (JSlaves.size() > ModBusSlave) {
    JSlave = JSlaves[ModBusSlave];
    if (JSlave.containsKey("swap")){
      if (JSlave["swap"].as<bool>()) {
        byteSwapModBusReg();
      }
    }
    if (JSlave.containsKey("tags") && JSlave.containsKey("name")) {
      String SlaveName = JSlave["name"].as<String>();
      JsonObject DataSlave;
      if (JData.containsKey(SlaveName)) {
        DataSlave = JData[SlaveName].as<JsonObject>();
      } else {
        DataSlave = JData.createNestedObject(SlaveName);
      }
      JTags = JSlave["tags"].as<JsonArray>();
      if (JTags.size() > ModBusTag) {
        JTag = JTags[ModBusTag];
        if (JTag.containsKey("type") && JTag.containsKey("name")) {
          String TagName = JTag["name"].as<String>();
          JsonObject DataTag;
          if (DataSlave.containsKey(TagName)) {
            DataTag = DataSlave[TagName].as<JsonObject>();
          } else {
            DataTag = DataSlave.createNestedObject(TagName);
          }
          DataTag["state"] = String(event, HEX);
          String TagType = JTag["type"].as<String>();
          bool BigEnding = false;
          if (JTag.containsKey("be")){
            BigEnding = JTag["be"].as<bool>();
          }
          if (TagType == "bool") {
            DataTag["value"] = ModBusCoil;
          } else if (TagType == "word") {
            DataTag["value"] = ModBusRegs.ValUnsigned16[0];
          } else if (TagType == "int16") {
            DataTag["value"] = ModBusRegs.ValSigned16[0];
          } else if (TagType == "uint16") {
            DataTag["value"] = ModBusRegs.ValUnsigned16[0];
          } else if (TagType == "dword") {
            if (BigEnding) {
              bigEndingModBusReg32();
            }
            DataTag["value"] = ModBusRegs.ValUnsigned32[0];
          } else if (TagType == "int32") {
            if (BigEnding) {
              bigEndingModBusReg32();
            }
            DataTag["value"] = ModBusRegs.ValSigned32[0];
          } else if (TagType == "uint32") {
            if (BigEnding) {
              bigEndingModBusReg32();
            }
            DataTag["value"] = ModBusRegs.ValUnsigned32[0];
          } else if (TagType == "float") {
            if (BigEnding) {
              bigEndingModBusReg32();
            }
            DataTag["value"] = ModBusRegs.ValFloat[0];
          } else if (TagType == "qword") {
            if (BigEnding) {
              bigEndingModBusReg64();
            }
            DataTag["value"] = ModBusRegs.ValUnsigned64;
          } else if (TagType == "int64") {
            if (BigEnding) {
              bigEndingModBusReg64();
            }
            DataTag["value"] = ModBusRegs.ValSigned64;
          } else if (TagType == "uint64") {
            if (BigEnding) {
              bigEndingModBusReg64();
            }
            DataTag["value"] = ModBusRegs.ValUnsigned64;
          } else if (TagType == "double") {
            if (BigEnding) {
              bigEndingModBusReg64();
            }
            DataTag["value"] = ModBusRegs.ValDouble;
          }
        }
        ModBusTag++;
        if (ModBusTag >= JTags.size()) {
          ModBusTag = 0;
          ModBusSlave++;
          if (ModBusSlave >= JSlaves.size()) {
            ModBusSlave = 0;
          }
        }
      }
    }
  }
  return true;
}
