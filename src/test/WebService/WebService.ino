// WebService.ino
//

#include "WebService.h"


void setup()
{
    Serial.begin(115200);
    Serial.println("Start WebService...");

    SPIFFS.begin();
}

void loop()
{
    //
    if (Serial.available())
    {
        int ch = Serial.read();

        if (ch == 'B' || ch == 'b')
        {
            Serial.println("start WebService");
            WebService.begin();
        }

        if (ch == 'E' || ch == 'e')
        {
            Serial.println("stop WebService");
            WebService.end();
        }
    }

    //
    WebService.update();
}