// WebService.cpp
//

#include "DeviceContext.h"
#include "WebService.h"
#include <ArduinoJson.h>


///////////////////////////////////////////////////////////////////////////////////
//

bool parseBoolean(String str)
{
    if (str == "true")
        return true;

    return false;
}

int parseInteger(String str)
{
    return atoi(str.c_str());
}

float parseFloat(String str)
{
    return atof(str.c_str());
}


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
    DeviceContext & context = __DeviceContext;

    //
    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(IPAddress(192, 168, 1, 1), IPAddress(192, 168, 1, 1), IPAddress(255, 255, 255, 0));
    WiFi.softAP(context.deviceDefault.wifiSSID, context.deviceDefault.wifiPassword);
    IPAddress myIP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(myIP);

    //
    mServer.on("/update/{}", HTTP_POST, onUpdateRequest);
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


void WebServiceClass::onUpdateRequest()
{
    //
    WebServer & server = WebService.mServer;

    String target = "/" + server.pathArg(0);
    if (! SPIFFS.exists(target))
    {
        Serial.printf("File Not Exist: %s\n", target);

        server.sendHeader("Connection", "close");
        server.send(200, "text/plain", "NOT EXIST");
    }
    else
    {
        //
        const size_t capacity = JSON_OBJECT_SIZE(29) + 1024;
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
        for (int i = 0; i < server.args(); i++)
        {
            Serial.printf("%s: %s\n", server.argName(i).c_str(), server.arg(i).c_str());

            String name = server.argName(i);
            String value = server.arg(i);
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

                Serial.printf("Serialize %d bytes\n", size);
            }

            if (size != 0)
            {
                Serial.printf("Update Success.\n");

                server.sendHeader("Connection", "close");
                server.send(200, "text/plain", "OK");
            }
            else
            {
                Serial.printf("Update Failed!\n");

                server.sendHeader("Connection", "close");
                server.send(200, "text/plain", "FAILED");
            }
        }

        // update context
        {
            DeviceContext & context = __DeviceContext;
            context.set(doc);
        }
    }
}

void WebServiceClass::onRequest()
{
    if (! WebService.handleFileRead(WebService.mServer.uri()))
        WebService.mServer.send(404, "text/plain", "FileNotFound");
}
