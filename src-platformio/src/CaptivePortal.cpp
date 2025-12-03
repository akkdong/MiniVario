// CaptivePortal.cpp
//

#include "CaptivePortal.h"
#include <SPIFFS.h>
#include <SD.h>
#include <AsyncJson.h>
#include <ArduinoJson.h>

#include <uri/UriBraces.h>
#include <uri/UriRegex.h>

/*
#include "DeviceRepository.h"
#include "Application.h"
#include "logger.h"
*/
#include "DeviceContext.h"


#define WIFI_getChipId()    (uint32_t)ESP.getEfuseMac()
#define WIFI_CHANNEL        6

#define DNS_TTL             (3600)
#define DNS_PORT            (53)

#define WEB_PORT            (80)

#define LOGe(x, ...)
#define LOGw(x, ...)
#define LOGi(x, ...)
#define LOGv(x, ...)
#define LOGd(x, ...)




////////////////////////////////////////////////////////////////////////////////////////////
//

class LogFinder
{
public:
    LogFinder(const char* rootDir) {
    }
    LogFinder(const LogFinder& finder) {
        LOGe("!!!!!!NO!!!!!!!");
    }
    ~LogFinder() {
        close();
    }


    bool open() {
        //
        close();

        //
        dir = SD.open("/TrackLogs");
        if (!dir.isDirectory())
            return false;

        data = "[";
        count = 0;
        eof = false;

        return true;
    }

    void close() {
        file.close();
        dir.close();
    }

    int read(uint8_t *buf, size_t maxLen) {
        uint8_t *ptr = buf;
        size_t totalSize = 0, remain = maxLen;

        while (remain > 0)
        {
            if (!eof)
                next();

            if (data.length() == 0)
                break;

            size_t size = data.length();
            if (size > remain)
                size = remain;
            memcpy((void *)ptr, data.c_str(), size);
            data = data.substring(size, -1);
            LOGd(" copy %d bytes", size);

            ptr += size;
            totalSize += size;
            remain -= size;
        }

        return totalSize;
    }

    void next() {
        // find next valid file
        file = dir.openNextFile();
        while (file)
        {
            if (!file.isDirectory() && file.size() > 0) {

                String name(file.name());
                if (name.endsWith(".igc"))
                    break;
            }

            file = dir.openNextFile();
        }

        if (file) {
            if (count > 0)
                data += ",";
                
            data += "{\"name\":\"";
            data += file.name();
            data += "\",\"size\":";
            data += file.size();
            data += ",\"date\":\"";
            data += getDateString(file.getLastWrite());
            data += "\"}";
            LOGd("  find: %s", file.name());

            ++count;
        } else {
            data += "]";
            eof = true;
        }
    }

    String getDateString(time_t t)
    {
        struct tm * _tm;
        _tm = gmtime(&t);

        char str[32]; // YYYY-MM-DD hh:mm:ssZ
        String date;
        
        sprintf(str, "%d-%d-%d %d:%d:%dZ", 
            _tm->tm_year + 1900, _tm->tm_mon + 1, _tm->tm_mday,
            _tm->tm_hour, _tm->tm_min, _tm->tm_sec);

        return String(str);
    }    

    File dir, file;
    String data;
    int count;
    bool eof;
};


LogFinder _lf("/TrackLogs");



////////////////////////////////////////////////////////////////////////////////////////////
//

CaptivePortal::CaptivePortal()
    : portalState(PORTAL_READY)
    , apSSID("CaptivePortal")
    , apPassword("") 
    , apIP(4, 3, 2, 1) // or 172.217.28.1
    , apEventID(0)
    #if USE_ASYNCWEBSERVER
    , webServer(WEB_PORT)
    #endif
{

}

CaptivePortal::~CaptivePortal()
{
    if (portalState != PORTAL_READY)
    {
        stop();
    }
}

int CaptivePortal::start()
{
    if (portalState != PORTAL_READY)
        return -1;

	starSoftAP(apSSID.c_str(), apPassword.c_str(), apIP, apIP);
	startDNSServer(apIP);
    #if USE_ASYNCWEBSERVER
    startAsyncWebServer(apIP);
    #else
	startWebServer(apIP);
    #endif

    #if 0
    DeviceContext *contextPtr = DeviceRepository::instance().getContext();
    strcpy(contextPtr->deviceState.ipAddr, apIP.toString().c_str());
    Application::getApp()->sendMessage(MSG_WIFI_STATE_CHANGED);
    #else
    strcpy(__DeviceContext.deviceState.ipAddr, apIP.toString().c_str());
    //notify(MSG_WIFI_STATE_CHANGED);
    #endif

    portalState = PORTAL_WAIT_CONNECTION;
	return 0;
}

void CaptivePortal::stop()
{
    if (portalState != PORTAL_READY)
    {
        #if USE_ASYNCWEBSERVER
        webServer.end();
        #else
        webServer.close();
        #endif
        dnsServer.stop();

        WiFi.removeEvent(apEventID);
        WiFi.softAPdisconnect(true);

        portalState = PORTAL_READY;
        apEventID = 0;
    }
}

void CaptivePortal::update()
{
    if (portalState != PORTAL_READY)
    {
	    dnsServer.processNextRequest();
        #if !USE_ASYNCWEBSERVER
	    webServer.handleClient();
        #endif
    }
}

void CaptivePortal::starSoftAP(const char *ssid, const char *pass, const IPAddress &ip, const IPAddress &gw)
{
	// event handler
    apEventID = WiFi.onEvent(std::bind(&CaptivePortal::onWiFiEvent, this, std::placeholders::_1, std::placeholders::_2));
    LOGd("WiFi.onEvent() => event id: %u", apEventID);

	// set WIFI mode to AP
    WiFi.persistent(false);
	bool ret = WiFi.mode(WIFI_MODE_AP);
	LOGd("WiFi.mode() => ret(%d)", ret);

	// configure AP with specfic ip/gw/mask
	const IPAddress mask(255, 255, 255, 0);
	ret = WiFi.softAPConfig(ip, gw, mask);
	LOGd("WiFi.softAPConfig(\"%s\", \"%s\", \"%s\") => ret(%d)", ip.toString().c_str(), gw.toString().c_str(), mask.toString(), ret);

	// start AP with given ssid, password, channel
	ret = WiFi.softAP(ssid, pass && pass[0] ? pass : NULL, WIFI_CHANNEL);
	LOGv("WiFi.softAP(\"%s\", \"%s\") => %s, ret(%d)", ssid, pass && pass[0] ? pass : "", WiFi.softAPIP().toString().c_str(), ret);

	// Disable AMPDU RX on the ESP32 WiFi to fix a bug on Android
	/*
	esp_wifi_stop();
	esp_wifi_deinit();
	wifi_init_config_t my_config = WIFI_INIT_CONFIG_DEFAULT();
	my_config.ampdu_rx_enable = false;
	esp_wifi_init(&my_config);
	esp_wifi_start();
	vTaskDelay(100 / portTICK_PERIOD_MS);  // Add a small delay  
	*/
}

void CaptivePortal::startDNSServer(const IPAddress& ip)
{
  // set TTL for DNS & start it
  dnsServer.setTTL(DNS_TTL);
  bool ret = dnsServer.start(DNS_PORT, "*", ip);
  LOGv("DNSServer.start() => ret(%d)", ret);
}

#if !USE_ASYNCWEBSERVER
void CaptivePortal::redirect(String &uri)
{
	LOGd("-> Redirect to %s", uri.c_str());

    webServer.sendHeader("Location", uri, true); // @HTTPHEAD send redirect
    webServer.send(302, "text/plain", ""); // Empty content inhibits Content-length header so we have to close the socket ourselves.
    webServer.client().stop(); // Stop is needed because we sent no content length
}
#endif

#if USE_ASYNCWEBSERVER
void CaptivePortal::startAsyncWebServer(const IPAddress &ip)
{
    redirectUrl = String("http://") + ip.toString() + "/index.html";
    LOGd("Redirect URL : %s", redirectUrl.c_str());

	// Portal connection validation pages
	webServer.on("/connecttest.txt", [this](AsyncWebServerRequest *request) { LOGd("/connecttest.txt"); request->redirect(redirectUrl); });	// windows 11 captive portal workaround
	webServer.on("/wpad.dat", [this](AsyncWebServerRequest *request) { LOGd("/wpad.dat"); request->send(404); });				// Honestly don't understand what this is but a 404 stops win 10 keep calling this repeatedly and panicking the esp32 :)
	webServer.on("/generate_204", [this](AsyncWebServerRequest *request) { LOGd("/generate_204"); request->redirect(redirectUrl); });		   	// android captive portal redirect
	webServer.on("/redirect", [this](AsyncWebServerRequest *request) { LOGd("/redirect"); request->redirect(redirectUrl); });			   	    // microsoft redirect
	webServer.on("/hotspot-detect.html", [this](AsyncWebServerRequest *request) { LOGd("/hotspot-detect.html"); request->redirect(redirectUrl); });  // apple call home
	webServer.on("/canonical.html", [this](AsyncWebServerRequest *request) { LOGd("/canonical.html"); request->redirect(redirectUrl); });	   	// firefox captive portal call home
	webServer.on("/success.txt", [this](AsyncWebServerRequest *request) { LOGd("/success.txt"); request->send(200); });	    // firefox captive portal call home
	webServer.on("/ncsi.txt", [this](AsyncWebServerRequest *request) { LOGd("/ncsi.txt"); request->redirect(redirectUrl); });			   	    // windows call home

    // Portal service pages
    #if 0
	webServer.on("^\\/Update\\/([a-zA-Z0-9\\-_.]+)$", WebRequestMethod::HTTP_POST, std::bind(&CaptivePortal::onUpdateRequest, this, std::placeholders::_1));
    #else
    AsyncCallbackJsonWebHandler* handler = new AsyncCallbackJsonWebHandler("/Update/config.json", std::bind(&CaptivePortal::OnUpdateConfigJson, this, std::placeholders::_1, std::placeholders::_2));   
    webServer.addHandler(handler);
    #endif
	webServer.on("/TrackLogs/list", WebRequestMethod::HTTP_GET, std::bind(&CaptivePortal::onRequestTrackLogs, this, std::placeholders::_1));
	webServer.on("^\\/TrackLogs\\/([a-zA-Z0-9\\-_.]+)$", WebRequestMethod::HTTP_GET, std::bind(&CaptivePortal::onDownloadTrackLog, this, std::placeholders::_1));
	webServer.on("^\\/TrackLogs\\/([a-zA-Z0-9\\-_.]+)$", WebRequestMethod::HTTP_DELETE, std::bind(&CaptivePortal::onDeleteTrackLog, this, std::placeholders::_1));
    // Poral default request handler
    #if SERVING_STATIC_FILES
    webServer.serveStatic("/", SPIFFS, "/");
    webServer.onNotFound([](AsyncWebServerRequest *request) { request->send(404, "text/plain", "Not found"); });
    #else
	webServer.onNotFound(std::bind(&CaptivePortal::onRequest, this, std::placeholders::_1));
    #endif

	//
	webServer.begin();
    LOGv("WebServer.begin()");
}

void CaptivePortal::OnUpdateConfigJson(AsyncWebServerRequest *request, JsonVariant &json)
{
    String target = "/config.json"; // + request->pathArg(0);
    LOGv("OnUpdateConfigJson: %s", target.c_str());
    
    if (SPIFFS.exists(target))
    {
        //
        const size_t capacity = JSON_OBJECT_SIZE(45) + 1240;
        DynamicJsonDocument doc(capacity);

        // read
        {
            File file = SPIFFS.open(target, FILE_READ);
            if (file)
            {
                deserializeJson(doc, file);
                file.close();
            }
        }

        // merge(or assign)
        auto jsonObj = json.as<JsonObject>();
        for (JsonPair pair : jsonObj)
        {
            String name = pair.key().c_str();
            JsonVariant value = pair.value();
            doc[name] = value;

            #if LOG_LEVEL >= LOG_LEVEL_VERBOSE
            if (value.is<bool>())
                LOGv("[%s] = %s", name.c_str(), value.as<bool>() ? "true" : "false");
            else if (value.is<int>())
                LOGv("[%s] = %d", name.c_str(), value.as<int>());
            else if (value.is<float>())
                LOGv("[%s] = %f", name.c_str(), value.as<float>());
            else
                LOGv("[%s] = %s", name.c_str(), value.as<const char *>());
            #endif
        }
        
        // save
        {
            File file = SPIFFS.open(target, FILE_WRITE);
            size_t size = 0;

            if (file)
            {
                size = serializeJson(doc, file);
                file.close();

                LOGv("Serialize %d bytes", size);
            }

            if (size != 0)
            {
                request->send(200, "text/plain", "OK");
                LOGv("Update Success.");
            }
            else
            {
                request->send(400, "text/plain", "File Write Failed");
                LOGw("Update Failed! : %s", target.c_str());
            }
        }
    }
    else
    {
        request->send(404, "text/plain", "File Not Found");
        LOGw("File Not Exist: %s", target.c_str());
    }
}

void CaptivePortal::onUpdateRequest(AsyncWebServerRequest *request)
{
    String target = "/" + request->pathArg(0);
    LOGv("onUpdateRequest: %s", target.c_str());
    
    if (!SPIFFS.exists(target))
    {
        request->send(404, "text/plain", "File Not Found");
        LOGw("File Not Exist: %s", target.c_str());
    }
    else
    {
        //
        const size_t capacity = JSON_OBJECT_SIZE(45) + 1240;
        DynamicJsonDocument doc(capacity);

        // read
        {
            File file = SPIFFS.open(target, FILE_READ);
            if (file)
            {
                deserializeJson(doc, file);
                file.close();
            }
        }

        // update
        for (int i = 0; i < request->args(); i++)
        {
            LOGv("%s: %s", request->argName(i).c_str(), request->arg(i).c_str());

            String name = request->argName(i);
            String value = request->arg(i);
            JsonVariant var = doc[name];

            if (! var.isNull())
            {
                if (var.is<bool>())
                    doc[name] = parseBoolean(value);
                else if (var.is<int>())
                    doc[name] = parseInteger(value);
                else if (var.is<float>())
                    doc[name] = parseFloat(value);
                else
                    doc[name] = value;
            }
            else
            {
                doc[name] = value;
            }
        }

        // save
        {
            File file = SPIFFS.open(target, FILE_WRITE);
            size_t size = 0;

            if (file)
            {
                size = serializeJson(doc, file);
                file.close();

                LOGv("Serialize %d bytes", size);
            }

            if (size != 0)
            {
                request->send(200, "text/plain", "OK");
                LOGv("Update Success.");
            }
            else
            {
                request->send(400, "text/plain", "File Write Failed");
                LOGw("Update Failed! : %s", target.c_str());
            }
        }
    }
}

void CaptivePortal::onRequestTrackLogs(AsyncWebServerRequest *request)
{
    if (_lf.open())
    {
        AsyncWebServerResponse *response = request->beginChunkedResponse("application/json", [] (uint8_t *buffer, size_t maxLen, size_t index) -> size_t {
            LOGd("read-chunk bufLen=%d, index=%d", maxLen, index);
            int size = _lf.read(buffer, maxLen);
            if (size == 0)
                _lf.close();

            LOGd("  total %d bytes", size);
            return size;
        });

        request->send(response);
    }
    else
    {
        request->send(500, "No TrackLogs directory");
    }
}

void CaptivePortal::onDownloadTrackLog(AsyncWebServerRequest *request)
{
    String target = "/TrackLogs/" + request->pathArg(0);
    LOGv("onDownloadTrackLog: %s", target.c_str());

    this->handleFileRead(request, SD, target);
}

void CaptivePortal::onDeleteTrackLog(AsyncWebServerRequest *request)
{
    String target = "/TrackLogs/" + request->pathArg(0);
    LOGv("onDeleteTrackLog: %s", target.c_str());

    if (SD.remove(target.c_str()))
    {
        request->send(200, "text/plain", "OK");
        LOGv("TrackLog deleted: %s", target.c_str());
    }
    else
    {
        request->send(400, "text/plain", "File Delete Failed");
        LOGw("TrackLog delete Failed: %s", target.c_str());
    }
}

void CaptivePortal::onRequest(AsyncWebServerRequest *request)
{
    this->handleFileRead(request, SPIFFS, request->url());
}

bool CaptivePortal::handleFileRead(AsyncWebServerRequest *request, fs::FS & fs, String path)
{
    LOGv("handleFileRead: %s", path.c_str());
    if (path.endsWith("/"))
        path += "index.html";

    if (fs.exists(path))
    {
        String contentType = getContentType(path);
        request->send(fs, path, contentType);
    }
    else
    {
        LOGv("Not Found: %s",path.c_str());
        request->send(404, "text/plain", "Not found");
    }

    return true;
}


#else
void CaptivePortal::startWebServer(const IPAddress &ip)
{
    redirectUrl = String("http://") + ip.toString() + "/index.html";
    LOGd("Redirect URL : %s", redirectUrl.c_str());

	// Portal connection validation pages
	webServer.on(Uri("/connecttest.txt"), [this]() { LOGd("/connecttest.txt"); redirect(redirectUrl); });	// windows 11 captive portal workaround
	webServer.on(Uri("/wpad.dat"), [this]() { LOGd("/wpad.dat"); webServer.send(404); });				// Honestly don't understand what this is but a 404 stops win 10 keep calling this repeatedly and panicking the esp32 :)
	webServer.on(Uri("/generate_204"), [this]() { LOGd("/generate_204"); redirect(redirectUrl); });		   	// android captive portal redirect
	webServer.on(Uri("/redirect"), [this]() { LOGd("/redirect"); redirect(redirectUrl); });			   	    // microsoft redirect
	webServer.on(Uri("/hotspot-detect.html"), [this]() { LOGd("/hotspot-detect.html"); redirect(redirectUrl); });  // apple call home
	webServer.on(Uri("/canonical.html"), [this]() { LOGd("/canonical.html"); redirect(redirectUrl); });	   	// firefox captive portal call home
	webServer.on(Uri("/success.txt"), [this]() { LOGd("/success.txt"); webServer.send(200); });	    // firefox captive portal call home
	webServer.on(Uri("/ncsi.txt"), [this]() { LOGd("/ncsi.txt"); redirect(redirectUrl); });			   	    // windows call home

    // Portal service pages
	webServer.on(UriBraces("/Update/{}"), HTTP_POST, std::bind(&CaptivePortal::onUpdateRequest, this));
	webServer.on(Uri("/TrackLogs/list"), HTTP_GET, std::bind(&CaptivePortal::onRequestTrackLogs, this));
	webServer.on(UriBraces("/TrackLogs/{}"), HTTP_GET, std::bind(&CaptivePortal::onDownloadTrackLog, this));
	webServer.on(UriBraces("/TrackLogs/{}"), HTTP_DELETE, std::bind(&CaptivePortal::onDeleteTrackLog, this));
    // Poral default request handler
	webServer.onNotFound(std::bind(&CaptivePortal::onRequest, this));

	//
	webServer.begin();
    LOGv("WebServer.begin()");
}


void CaptivePortal::onUpdateRequest()
{
    String target = "/" + webServer.pathArg(0);
    LOGv("Update target: %s", target.c_str());
    
    if (!SPIFFS.exists(target))
    {
        webServer.send(404, "text/plain", "File Not Found");
        LOGw("File Not Exist: %s", target.c_str());
    }
    else
    {
        //
        const size_t capacity = JSON_OBJECT_SIZE(45) + 1240;
        DynamicJsonDocument doc(capacity);

        // read
        {
            File file = SPIFFS.open(target, FILE_READ);
            if (file)
            {
                deserializeJson(doc, file);
                file.close();
            }
        }

        // update
        for (int i = 0; i < webServer.args(); i++)
        {
            LOGd("%s: %s", webServer.argName(i).c_str(), webServer.arg(i).c_str());

            String name = webServer.argName(i);
            String value = webServer.arg(i);
            JsonVariant var = doc[name];

            if (! var.isNull())
            {
                if (var.is<bool>())
                    doc[name] = parseBoolean(value);
                else if (var.is<int>())
                    doc[name] = parseInteger(value);
                else if (var.is<float>())
                    doc[name] = parseFloat(value);
                else
                    doc[name] = value;
            }
            else
            {
                doc[name] = value;
            }
        }

        // save
        {
            File file = SPIFFS.open(target, FILE_WRITE);
            size_t size = 0;

            if (file)
            {
                size = serializeJson(doc, file);
                file.close();

                LOGd("Serialize %d bytes", size);
            }

            if (size != 0)
            {
                webServer.send(200, "text/plain", "OK");
                LOGv("Update Success.");
            }
            else
            {
                webServer.send(400, "text/plain", "File Write Failed");
                LOGw("Update Failed! : %s", target.c_str());
            }
        }
    }
}

void CaptivePortal::onRequestTrackLogs()
{
    #if 0 // write header directory. but not permitted
    const char * hdr =  "HTTP/1.1 200 OK\r\n"
                        "Content-Type: application/json\r\n"
                        "Transfer-Encoding: chunked\r\n"
                        "Accept-Ranges: none\r\n"
                        "Connection: close\r\n"
                        "\r\n";
    webServer._chunked = true;
    webServer._currentClientWrite(hdr, strlen(hdr));

    webServer.sendContent("[");
    #else
    webServer.setContentLength(CONTENT_LENGTH_UNKNOWN);
    webServer.send(200, "application/json", "[");
    #endif

    File dir = SD.open("/TrackLogs");
    if (dir.isDirectory())
    {
        File file = dir.openNextFile();
        int count = 0;

        LOGv("List up TrakcLogs");
        while (file)
        {
            if (! file.isDirectory() && file.size() > 0)
            {
                String name(file.name());

                if (name.endsWith(".igc"))
                {
                    // { "name": "xxx.igc", "size": nnn, "date": "YYYY-MM-DD hh:mm:ssZ" }
                    String igc;

                    // 
                    if (count > 0)
                        igc += ",";
                        
                    igc += "{\"name\":\"";
                    igc += name;
                    igc += "\",\"size\":";
                    igc += file.size();
                    igc += ",\"date\":\"";
                    igc += getDateString(file.getLastWrite());
                    igc += "\"}";

                    webServer.sendContent(igc);
                    LOGv(igc.c_str());

                    count += 1;
                }
            }

            file = dir.openNextFile();
        }
    }

    webServer.sendContent("]");
    webServer.sendContent("");
}

void CaptivePortal::onDownloadTrackLog()
{
    String target = "/TrackLogs/" + webServer.pathArg(0);
    LOGv("handleFileDowload: %s", target.c_str());

    this->handleFileRead(SD, target);
}

void CaptivePortal::onDeleteTrackLog()
{
    String target = "/TrackLogs/" + webServer.pathArg(0);
    LOGv("handleFileDelete: %s", target.c_str());

    if (SD.remove(target.c_str()))
    {
        webServer.send(200, "text/plain", "OK");
        LOGv("TrackLog deleted: %s", target.c_str());
    }
    else
    {
        webServer.send(400, "text/plain", "File Delete Failed");
        LOGw("TrackLog delete Failed: %s", target.c_str());
    }
}

void CaptivePortal::onRequest()
{
    this->handleFileRead(SPIFFS, webServer.uri());
}

bool CaptivePortal::handleFileRead(fs::FS & fs, String path)
{
    LOGv("handleFileRead: %s", path.c_str());
    if (path.endsWith("/"))
        path += "index.html";

    String path_gz = path + ".gz";

    //if (checkExist(fs, path_gz) || checkExist(fs, path))
    if (fs.exists(path_gz) || fs.exists(path))
    {
        //if (checkExist(fs, path_gz))
        if (fs.exists(path_gz))
            path = path + ".gz";

        String contentType = getContentType(path);
        LOGd("  Open: %s(%s)", path.c_str(), contentType.c_str());
        File file = fs.open(path, "r");
        if (file)
        {
            webServer.streamFile(file, contentType);
            file.close();

            return true;
        }
        
        webServer.send(400, "text/plain", "File Open Failed");
        LOGw("  File open failed!: %s", path.c_str());    
    }
    else
    {
        webServer.send(404, "text/plain", "File Not Found");
        LOGv("  File not exist! : %s", path.c_str());
    }

    return false;
}

#endif

void CaptivePortal::onWiFiEvent(WiFiEvent_t event,arduino_event_info_t info)
{
    LOGv("[WiFi] onEvent: %s", WiFi.eventName(event));

    #if 0
    DeviceContext *contextPtr = DeviceRepository::instance().getContext();
    #endif
    //contextPtr->deviceState.wifiState = static_cast<int>(event);
    //Application::getApp()->sendMessage(MSG_WIFI_STATE_CHANGED);

    switch (event) 
    {
    case ARDUINO_EVENT_WIFI_READY:
        break;
    case ARDUINO_EVENT_WIFI_SCAN_DONE:
        break;
    case ARDUINO_EVENT_WIFI_STA_START:
        break;
    case ARDUINO_EVENT_WIFI_STA_STOP:
        break;
    case ARDUINO_EVENT_WIFI_STA_CONNECTED:
        break;
    case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
        break;
    case ARDUINO_EVENT_WIFI_STA_AUTHMODE_CHANGE:
        break;
    case ARDUINO_EVENT_WIFI_STA_GOT_IP:
        break;
    case ARDUINO_EVENT_WIFI_STA_GOT_IP6:
        break;
    case ARDUINO_EVENT_WIFI_STA_LOST_IP:
        break;
    case ARDUINO_EVENT_WIFI_AP_START:
        #if 0
        contextPtr->deviceState.wifiState = 1;
        Application::getApp()->sendMessage(MSG_WIFI_STATE_CHANGED);
        #else
        __DeviceContext.deviceState.wifiState = 1;
        //notify(MSG_WIFI_STATE_CHANGED);
        #endif
        break;
    case ARDUINO_EVENT_WIFI_AP_STOP:
        #if 0
        contextPtr->deviceState.wifiState = 0;
        Application::getApp()->sendMessage(MSG_WIFI_STATE_CHANGED);
        #else
        __DeviceContext.deviceState.wifiState = 0;
        //notify(MSG_WIFI_STATE_CHANGED);
        #endif
        break;
    case ARDUINO_EVENT_WIFI_AP_STACONNECTED:
        LOGv( "[WiFi] STA Connected");
        portalState = PORTAL_CONNECTED;
        #if 0
        contextPtr->deviceState.wifiState = 2;
        Application::getApp()->sendMessage(MSG_WIFI_STATE_CHANGED);
        #else
        __DeviceContext.deviceState.wifiState = 2;
        //notify(MSG_WIFI_STATE_CHANGED);
        #endif
        break;
    case ARDUINO_EVENT_WIFI_AP_STADISCONNECTED:
        LOGv( "[WiFi] STA Disconnected");
        portalState = PORTAL_WAIT_CONNECTION;
        #if 0
        contextPtr->deviceState.wifiState = 1;
        Application::getApp()->sendMessage(MSG_WIFI_STATE_CHANGED);
        #else
        __DeviceContext.deviceState.wifiState = 1;
        //notify(MSG_WIFI_STATE_CHANGED);
        #endif        
        break;
    case ARDUINO_EVENT_WIFI_AP_STAIPASSIGNED:
        LOGv( "[WiFi] STA IP: %s", IPAddress(info.wifi_ap_staipassigned.ip.addr).toString().c_str());
        break;
    case ARDUINO_EVENT_WIFI_AP_PROBEREQRECVED:
        break;
    case ARDUINO_EVENT_WIFI_AP_GOT_IP6:
        break;
    case ARDUINO_EVENT_WIFI_FTM_REPORT:
        break;
    case ARDUINO_EVENT_ETH_START:
        break;
    case ARDUINO_EVENT_ETH_STOP:
        break;
    case ARDUINO_EVENT_ETH_CONNECTED:
        break;
    case ARDUINO_EVENT_ETH_DISCONNECTED:
        break;
    case ARDUINO_EVENT_ETH_GOT_IP:
        break;
    case ARDUINO_EVENT_ETH_GOT_IP6:
        break;
    case ARDUINO_EVENT_WPS_ER_SUCCESS:
        break;
    case ARDUINO_EVENT_WPS_ER_FAILED:
        break;
    case ARDUINO_EVENT_WPS_ER_TIMEOUT:
        break;
    case ARDUINO_EVENT_WPS_ER_PIN:
        break;
    case ARDUINO_EVENT_WPS_ER_PBC_OVERLAP:
        break;
    case ARDUINO_EVENT_SC_SCAN_DONE:
        break;
    case ARDUINO_EVENT_SC_FOUND_CHANNEL:
        break;
    case ARDUINO_EVENT_SC_GOT_SSID_PSWD:
        break;
    case ARDUINO_EVENT_SC_SEND_ACK_DONE:
        break;
    case ARDUINO_EVENT_PROV_INIT:
        break;
    case ARDUINO_EVENT_PROV_DEINIT:
        break;
    case ARDUINO_EVENT_PROV_START:
        break;
    case ARDUINO_EVENT_PROV_END:
        break;
    case ARDUINO_EVENT_PROV_CRED_RECV:
        break;
    case ARDUINO_EVENT_PROV_CRED_FAIL:
        break;
    case ARDUINO_EVENT_PROV_CRED_SUCCESS:
        break;
    }
}


const char * CaptivePortal::getContentType(String filename)
{
    if (filename.endsWith(".htm") || filename.endsWith(".html"))
        return "text/html";

    if (filename.endsWith(".css"))
        return "text/css";

    if (filename.endsWith(".js"))
        return "text/javascript";

    if (filename.endsWith(".json"))
        return "application/json";

    if (filename.endsWith(".png"))
        return "image/png";

    if (filename.endsWith(".gif"))
        return "image/gif";

    if (filename.endsWith(".jpg") || filename.endsWith(".jpeg"))
        return "image/jpeg";

    if (filename.endsWith(".ico"))
        return "image/x-icon";

    if (filename.endsWith(".igc"))
        return "application/octet-stream";

    if (filename.endsWith(".xml"))
        return "text/xml";

    if (filename.endsWith(".zip"))
        return "application/x-zip";

    if (filename.endsWith(".gz"))
        return "application/x-gzip";

    if (filename.endsWith(".pdf"))
        return "application/x-pdf";

    return "text/plain";
}

bool CaptivePortal::checkExist(fs::FS & fs, String path)
{
    bool exist = false;

    File file = fs.open(path, "r");
    if (! file.isDirectory())
        exist = true;
    file.close();

    return exist;
}

bool CaptivePortal::parseBoolean(String str)
{
    if (str == "true")
        return true;

    return false;
}

int CaptivePortal::parseInteger(String str)
{
    return atoi(str.c_str());
}

float CaptivePortal::parseFloat(String str)
{
    return atof(str.c_str());
}

String CaptivePortal::getDateString(time_t t)
{
    struct tm * _tm;
    _tm = gmtime(&t);

    char str[32]; // YYYY-MM-DD hh:mm:ssZ
    String date;
    
    sprintf(str, "%d-%d-%d %d:%d:%dZ", 
        _tm->tm_year + 1900, _tm->tm_mon + 1, _tm->tm_mday,
        _tm->tm_hour, _tm->tm_min, _tm->tm_sec);

    return String(str);
}


