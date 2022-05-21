//-------------------------------------------------------
// Config and Data
//-------------------------------------------------------
String WsMsgText = "";

//-------------------------------------------------------
// Connection Response
//-------------------------------------------------------
void echoClients(AsyncWebSocketClient *_Client, const char *_TypeStr, const char *_DataStr) {
  StaticJsonDocument<200> JMsg;
  
  JMsg[_TypeStr] = _DataStr;
  size_t Len = measureJson(JMsg) + 1;
  char *Buffer = (char*)malloc(Len);
  if (Buffer != NULL) {
    Len = serializeJson(JMsg, Buffer, Len);
    _Client->text(Buffer);
    free(Buffer);
  }
}

//-------------------------------------------------------
// Data telegramm for all connected Clients
//-------------------------------------------------------
void notifyClients() {
  size_t Len = measureJson(JData) + 1;
  char *Buffer = (char*)malloc(Len);
  if (Buffer != NULL) {
    Len = serializeJson(JData, Buffer, Len);
    WebSocket.textAll(Buffer);
    free(Buffer);
  #if DEBUG_SERIAL >= DEBUG_ERR
    } else {
      Serial.printf("Buffer failed, Len=%d\r\n", Len);
  #endif
  }
}

//-------------------------------------------------------
// Empfangene Daten verarbeiten
//-------------------------------------------------------
void handleWebSocketMessage(AsyncWebSocketClient *_Client, void *_Arg, uint8_t *_Data, size_t _Len) {
  AwsFrameInfo *Info = (AwsFrameInfo*)_Arg;
  if (Info->opcode == WS_TEXT) {
    //Initialise Message-String on first Frame
    if (Info->index == 0) {
      WsMsgText = "";
    }
    //Append Data to Message-Sting
    for(size_t i=0; i < _Len; i++) {
      WsMsgText += (char) _Data[i];
    }
    //Handle Message if last Frame ist received
    if (Info->final && Info->index + _Len == Info->len) {
      DynamicJsonDocument JWsDoc(8000);
      DeserializationError Err = deserializeJson(JWsDoc, WsMsgText);
      if (Err) {
        echoClients(_Client, "error", Err.c_str());
        return;
      }
      JsonObject JWsMsg = JWsDoc.as<JsonObject>();
      if (JWsMsg.containsKey("type")) {
        String WsMsgType = JWsMsg["type"].as<String>();
        if (WsMsgType == "config") {
          size_t ByteLen = updateConfig(JWsMsg);
          String EchoText = "Bytes written:" + ByteLen;
          echoClients(_Client, "config", EchoText.c_str());
          
        } else if (WsMsgType == "write") {
          echoClients(_Client, "write", "ModBus write currently not supported");
          
        } else {
          echoClients(_Client, "error", "type not supported. use config/write");
        }
      } else {
        echoClients(_Client, "error", "type key missing");
      }
    }
  }
  if (Info->final && Info->index == 0 && Info->len == _Len && Info->opcode == WS_TEXT) {
    
    StaticJsonDocument<200> JMsg;
    DeserializationError Err = deserializeJson(JMsg, _Data);
    if (Err) {
      echoClients(_Client, "error", Err.c_str());
      return;
    }
    echoClients(_Client, "data", "passed");
  }
}

//-------------------------------------------------------
// Event-Handling
//-------------------------------------------------------
void WsEvent(AsyncWebSocket *_WebServer, AsyncWebSocketClient *_Client, AwsEventType _Type, void *_Arg, uint8_t *_Data, size_t _Len) {
    #if DEBUG_SERIAL >= DEBUG_LOG
      Serial.println("WsEvent - Start");
    #endif
    switch (_Type) {
      case WS_EVT_CONNECT:
        echoClients(_Client, "connect", "hello");
        break;
      case WS_EVT_DISCONNECT:
        break;
      case WS_EVT_DATA:
        handleWebSocketMessage(_Client, _Arg, _Data, _Len);
        break;
      case WS_EVT_PONG:
        #if DEBUG_SERIAL >= DEBUG_LOG
          Serial.println("Ping-Pong");
        #endif
        break;
      case WS_EVT_ERROR:
        break;
  }
}
