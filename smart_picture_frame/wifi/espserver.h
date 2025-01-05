#include <Arduino.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <WiFiManager.h>
#include "displayhandler.h"

#ifdef HTTP_UPLOAD_BUFLEN //if the macro MEDIAN_MAX_SIZE is defined 
#undef HTTP_UPLOAD_BUFLEN //un-define it
#define HTTP_UPLOAD_BUFLEN 200000 //redefine it with the new value
#endif 

class EspServer {
public:
  EspServer(void);
  ~EspServer(void);

  int  Init(void);
  void SetDisplay(DisplayHandler* ptr);
  void Loop(void);
private:
  DisplayHandler* dspPtr;
  WebServer server;
  StaticJsonDocument<1024> jsonDocument;
  char buffer[1024];

  void CreateJson(char *tag, char *value);
  void SendJsonResponse(int code, char *tag, char *value);

  void GetStatus(void);
  void ClearDisplay(void);
  void FinalizeImageUpload(void);
  void UploadImageChunk(void);
};