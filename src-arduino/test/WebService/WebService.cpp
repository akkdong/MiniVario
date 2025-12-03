// WebService.cpp
//

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
{
}

WebServiceClass::~WebServiceClass()
{
}

void WebServiceClass::start()
{
    //
    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(IPAddress(192, 168, 1, 1), IPAddress(192, 168, 1, 1), IPAddress(255, 255, 255, 0));
    WiFi.softAP("MiniVario", "123456789");
    IPAddress myIP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(myIP);

    //
    WebServer::on("/Update/{}", HTTP_POST, onUpdateRequest);
    WebServer::on("/TrackLogs/list", HTTP_GET, onRequestTrackLogs);
    WebServer::on("/TrackLogs/{}", HTTP_GET, onDownloadTrackLog);
    WebServer::on("/TrackLogs/{}", HTTP_DELETE, onDeleteTrackLog);
    WebServer::onNotFound(onRequest);

    //
    WebServer::begin(80);
}

void WebServiceClass::stop()
{
    WebServer::close();
    WiFi.softAPdisconnect(true);
}

void WebServiceClass::update()
{
    WebServer::handleClient();
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

    //if (filename.endsWith(".xml"))
    //    return "text/xml";

    //if (filename.endsWith(".zip"))
    //    return "application/x-zip";

    if (filename.endsWith(".gz"))
        return "application/x-gzip";

    //if (filename.endsWith(".pdf"))
    //    return "application/x-pdf";

    if (filename.endsWith(".igc"))
        return "application/octet-stream";

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
    Serial.print("handleFileRead: ");
    Serial.println(path);
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
            WebService.streamFile(file, contentType);
            file.close();

            return true;
        }
        
        Serial.println("  File open failed!");    
        WebService.send(400, "text/plain", "File Open Failed");
    }
    else
    {
        Serial.println("  File not exist!");
        WebService.send(404, "text/plain", "File Not Found");
    }

    return false;
}


void WebServiceClass::onUpdateRequest()
{
    //
    String target = "/" + WebService.pathArg(0);
    if (! SPIFFS.exists(target))
    {
        Serial.printf("File Not Exist: %s\n", target);
        WebService.send(404, "text/plain", "File Not Found");
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
        for (int i = 0; i < WebService.args(); i++)
        {
            Serial.printf("%s: %s\n", WebService.argName(i).c_str(), WebService.arg(i).c_str());

            String name = WebService.argName(i);
            String value = WebService.arg(i);
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
                WebService.send(200, "text/plain", "OK");
            }
            else
            {
                Serial.printf("Update Failed!\n");
                WebService.send(400, "text/plain", "File Write Failed");
            }
        }
    }
}

void WebServiceClass::onRequestTrackLogs()
{
    // WebService._prepareHeader(header, 200, "application/json", (size_t)-1 /*CONTENT_LENGTH_UNKNOWN*/);
    const char * hdr =  "HTTP/1.1 200 OK\r\n"
                        "Content-Type: application/json\r\n"
                        "Transfer-Encoding: chunked\r\n"
                        "Accept-Ranges: none\r\n"
                        "Connection: close\r\n"
                        "\r\n";
    WebService._chunked = true;
    WebService._currentClientWrite(hdr, strlen(hdr));

    WebService.sendContent("[");
    File dir = SPIFFS.open("/");
    if (dir.isDirectory())
    {
        File file = dir.openNextFile();
        int count = 0;

        while (file)
        {
            if (! file.isDirectory() && file.size() > 0)
            {
                String name(file.name());

                if (name.endsWith(".igc"))
                {
                    // { "name": "xxx", "size": nnn, "date": ddd }
                    String igc;

                    // 
                    if (count > 0)
                        igc += ",";
                        
                    igc += "{\"name\":\"";
                    igc += name.substring(1);
                    igc += "\",\"size\":";
                    igc += file.size();
                    igc += ",\"date\":";
                    igc += file.getLastWrite();
                    igc += "}";

                    WebService.sendContent(igc);
                    Serial.println(igc);

                    count += 1;
                }
            }

            file = dir.openNextFile();
        }
    }

    WebService.sendContent("]");
    WebService.sendContent("");
}

void WebServiceClass::onDownloadTrackLog()
{
    String target = "/" + WebService.pathArg(0);

    WebService.handleFileRead(target);
}

void WebServiceClass::onDeleteTrackLog()
{
    String target = "/" + WebService.pathArg(0);

    Serial.print("handleFileDelete: ");
    Serial.println(target);

    if (SPIFFS.remove(target.c_str()))
    {
        WebService.send(200, "text/plain", "OK");
    }
    else
    {
        WebService.send(400, "text/plain", "File Delete Failed");
    }
}

void WebServiceClass::onRequest()
{
    WebService.handleFileRead(WebService.uri());
}
