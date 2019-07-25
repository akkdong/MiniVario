// WebService.cpp
//

#include "WebService.h"
#include <ArduinoJson.h>


///////////////////////////////////////////////////////////////////////////////////
//

WebServiceClass  WebService;


///////////////////////////////////////////////////////////////////////////////////
// class WebServiceClass

WebServiceClass::WebServiceClass()
    : mServer(80)
{
}

WebServiceClass::~WebServiceClass()
{

}

int WebServiceClass::begin()
{
    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(IPAddress(192, 168, 1, 1), IPAddress(192, 168, 1, 1), IPAddress(255, 255, 255, 0));
    WiFi.softAP("esp32", "123456789");
    IPAddress myIP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(myIP);

    mServer.on("/update", HTTP_POST, onUpdateConf);
    mServer.onNotFound(onRequest);
    mServer.begin();

    return 0;
}

void WebServiceClass::end()
{
    mServer.close();
    WiFi.softAPdisconnect(true);
}

void WebServiceClass::update()
{
    mServer.handleClient();
}


const char * WebServiceClass::getContentType(String filename)
{
    if (filename.endsWith(".htm") || filename.endsWith(".html"))
        return "text/html";

    if (filename.endsWith(".css"))
        return "text/css";

    if (filename.endsWith(".js"))
        return "application/javascript";

    if (filename.endsWith(".json"))
        return "application/json";

    if (filename.endsWith(".png"))
        return "image/png";

    if (filename.endsWith(".gif"))
        return "image/gif";

    if (filename.endsWith(".jpg"))
        return "image/jpeg";

    if (filename.endsWith(".ico"))
        return "image/x-icon";

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

bool WebServiceClass::checkExist(String path)
{
    bool exist = false;

    File file = SPIFFS.open(path, "r");
    if (! file.isDirectory())
        exist = true;
    file.close();

    return exist;
}

bool WebServiceClass::handleFileRead(String path)
{
    Serial.println("handleFileRead: " + path);
    if (path.endsWith("/"))
        path += "index.html";

    String contentType = getContentType(path);
    Serial.println("  contentType: " + contentType);
    String path_gz = path + ".gz";

    if (checkExist(path_gz) || checkExist(path))
    {
        if (checkExist(path_gz)) {
            path += path + ".gz";
        }

        File file = SPIFFS.open(path, "r");
        if (file)
        {
            mServer.streamFile(file, contentType);
            file.close();

            return true;
        }
        
        Serial.println("  File open failed!");    
    }
    else
        Serial.println("  File not exist!");

    return false;
}

void WebServiceClass::onUpdateConf()
{
    //
    WebServer & server = WebService.mServer;

    /*
    Serial.printf("URL: %s\n", server.uri());
    Serial.printf("Headers: %d\n", server.headers());
    for (int i = 0; i < server.headers(); i++)
        Serial.printf("   %s: %s\n", server.headerName(i).c_str(), server.header(i).c_str());
    Serial.printf("Arguments: %d\n", server.args());
    for (int i = 0; i < server.args(); i++)
        Serial.printf("   %s: %s\n", server.argName(i).c_str(), server.arg(i).c_str());

     */

    //
    const size_t capacity = JSON_OBJECT_SIZE(29) + 1024;
    DynamicJsonDocument doc(capacity);

    for (int i = 0; i < server.args(); i++)
    {
        Serial.printf("%s: %s\n", server.argName(i).c_str(), server.arg(i).c_str());
        doc[server.argName(i)] = server.arg(i);
    }

    Serial.print("Serialize JSON to File: ");
    File conf = SPIFFS.open("/pref-data.json", FILE_WRITE);
    if (conf)
    {
        if (serializeJson(doc, conf) == 0)
            Serial.println(F("Failed to write to file"));
        conf.close();

        Serial.println("OK");

        server.sendHeader("Connection", "close");
        server.send(200, "text/plain", "OK");
    }
    else
    {
        Serial.println("FAILED");

        server.sendHeader("Connection", "close");
        server.send(200, "text/plain", "FAILED");
    }    
}

void WebServiceClass::onRequest()
{
    if (! WebService.handleFileRead(WebService.mServer.uri()))
        WebService.mServer.send(404, "text/plain", "FileNotFound");
}
