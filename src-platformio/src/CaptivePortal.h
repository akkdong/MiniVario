// CaptivePortal.h
//

#ifndef __CAPTIVE_PORTAL_H__
#define __CAPTIVE_PORTAL_H__

#ifndef USE_ASYNCWEBSERVER
#define USE_ASYNCWEBSERVER    1
#endif

#include <WiFi.h>
#include <DNSServer.h>
#if USE_ASYNCWEBSERVER
#include <AsyncTCP.h>
#include <AsyncJson.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#else
#include <WebServer.h>
#endif

////////////////////////////////////////////////////////////////////////////////////////////
//

class CaptivePortal
{
public:
  CaptivePortal();
  ~CaptivePortal();

  enum PortalState {
    PORTAL_READY,
    PORTAL_WAIT_CONNECTION,
    PORTAL_CONNECTED
  };

public:
  //
  void startWebServer(const IPAddress &ip) { startAsyncWebServer(ip); }

  //
  int start();
  void stop();

  void update();

  //
  PortalState getState() { return portalState; }

  void setSSID(const char *ssid) { apSSID = ssid; }
  void setPassword(const char *pass) { apPassword = pass; }
  void setAddress(const IPAddress &ip) { apIP = ip; }

protected:
  void starSoftAP(const char *ssid, const char *pass, const IPAddress &ip, const IPAddress &gw);
  void startDNSServer(const IPAddress& ip);
  #if USE_ASYNCWEBSERVER
  void startAsyncWebServer(const IPAddress &ip);

  void OnUpdateConfigJson(AsyncWebServerRequest *request, JsonVariant &json);
  void onUpdateRequest(AsyncWebServerRequest *request);
  void onRequestTrackLogs(AsyncWebServerRequest *request);
  void onDownloadTrackLog(AsyncWebServerRequest *request);
  void onDeleteTrackLog(AsyncWebServerRequest *request);
  void onRequest(AsyncWebServerRequest *request);

  bool handleFileRead(AsyncWebServerRequest *request, fs::FS & fs, String path);
  #else
  void startWebServer(const IPAddress &ip);

  void onUpdateRequest();
  void onRequestTrackLogs();
  void onDownloadTrackLog();
  void onDeleteTrackLog();
  void onRequest();

  bool handleFileRead(fs::FS & fs, String path);
  #endif

  void onWiFiEvent(WiFiEvent_t event,arduino_event_info_t info);

private:
  bool parseBoolean(String str);
  int parseInteger(String str);
  float parseFloat(String str);
  String getDateString(time_t t);

  const char * getContentType(String filename);
  bool checkExist(fs::FS & fs, String path);

  #if !USE_ASYNCWEBSERVER
  void redirect(String &uri);
  void redirect(const IPAddress &ip);
  #endif

protected:
  PortalState portalState;
  String apSSID;
  String apPassword;
  IPAddress apIP;
  wifi_event_id_t apEventID;
  String redirectUrl;

  #if USE_ASYNCWEBSERVER
  AsyncWebServer webServer;
  #else
  WebServer webServer;
  #endif
  DNSServer dnsServer;
};



#endif // __CAPTIVE_PORTAL_H__
