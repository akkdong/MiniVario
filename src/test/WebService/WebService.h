// WebService.h
//

#ifndef __WEBSERVICE_H__
#define __WEBSERVICE_H__

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <SPIFFS.h>


///////////////////////////////////////////////////////////////////////////////////
// class WebServiceClass

class WebServiceClass
{
public:
    WebServiceClass();
    ~WebServiceClass();

public:
    int                 begin();
    void                end();

    void                update();

protected:
    const char *        getContentType(String filename);
    bool                checkExist(String path);

    bool                handleFileRead(String path);

    static void         onRequest();

protected:
    WebServer           mServer;
};


///////////////////////////////////////////////////////////////////////////////////
//

extern WebServiceClass  WebService;


#endif // __WEBSERVICE_H__
