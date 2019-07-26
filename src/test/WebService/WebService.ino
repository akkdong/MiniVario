// WebService.ino
//

#include "WebService.h"
#include <ArduinoJson.h>


void setup()
{
    Serial.begin(115200);
    Serial.println("Start WebService...");

    SPIFFS.begin();

    //
    {
        File conf = SPIFFS.open("/pref-data.json");
        if (conf)
        {
            const size_t capacity = JSON_OBJECT_SIZE(29) + 1024;
            DynamicJsonDocument doc(capacity);

            DeserializationError error = deserializeJson(doc, conf);
            if (error)
                Serial.println("Failed to read file, using default configuration");
            
            Serial.print("vario_climb_threshold: "); Serial.println((float)doc["vario_climb_threshold"]); // 0.2
            Serial.print("vario_sink_threshold: "); Serial.println((float)doc["vario_sink_threshold"]); // -3,
            Serial.print("vario_sensitivity: "); Serial.println((float)doc["vario_sensitivity"]); // 0.12,
            Serial.print("vario_ref_altitude_1: "); Serial.println((float)doc["vario_ref_altitude_1"]); // 0.0,
            Serial.print("vario_ref_altitude_2: "); Serial.println((float)doc["vario_ref_altitude_2"]); // 0.0,
            Serial.print("vario_ref_altitude_3: "); Serial.println((float)doc["vario_ref_altitude_3"]); // 0.0,
            Serial.print("vario_damping_factor: "); Serial.println((float)doc["vario_damping_factor"]); // 0.05,
            Serial.print("glider_type: "); Serial.println((int)doc["glider_type"]); // 1,
            Serial.print("glider_manufacture: "); Serial.println((const char *)doc["glider_manufacture"]); // "Ozone",
            Serial.print("glider_model: "); Serial.println((const char *)doc["glider_model"]); // "Zeno",
            Serial.print("igc_enable_logging: "); Serial.println((bool)doc["igc_enable_logging"]); // true,
            Serial.print("igc_takeoff_speed: "); Serial.println((int)doc["igc_takeoff_speed"]); // 6,
            Serial.print("igc_landing_timeout: "); Serial.println((int)doc["igc_landing_timeout"]); // 10000,
            Serial.print("igc_logging_interval: "); Serial.println((int)doc["igc_logging_interval"]); // 1000,
            Serial.print("igc_pilot: "); Serial.println((const char *)doc["igc_pilot"]); // "AKKDONG",
            Serial.print("igc_timezone: "); Serial.println((float)doc["igc_timezone"]); // 9.0,
            Serial.print("volume_vario_enabled: "); Serial.println((bool)doc["volume_vario_enabled"]); // false,
            Serial.print("volume_effect_enabled: "); Serial.println((bool)doc["volume_effect_enabled"]); // false,
            Serial.print("volume_auto_turnon: "); Serial.println((bool)doc["volume_auto_turnon"]); // true,
            Serial.print("threshold_low_battery: "); Serial.println((float)doc["threshold_low_battery"]); // 2.9,
            Serial.print("threshold_auto_shutdown: "); Serial.println((int)doc["threshold_auto_shutdown"]); // 600000,
            Serial.print("kalman_var_zmeas: "); Serial.println((float)doc["kalman_var_zmeas"]); // 400.0,
            Serial.print("kalman_var_zaccel: "); Serial.println((float)doc["kalman_var_zaccel"]); // 1000.0,
            Serial.print("kalman_var_abias: "); Serial.println((float)doc["kalman_var_abias"]); // 1.0,
            Serial.print("device_enable_bt: "); Serial.println((bool)doc["device_enable_bt"]); // true,
            Serial.print("device_enable_sound: "); Serial.println((bool)doc["device_enable_sound"]); // false,
            Serial.print("device_bt_name: "); Serial.println((const char *)doc["device_bt_name"]); // "MiniVario",
            Serial.print("device_enable_simulation: "); Serial.println((bool)doc["device_enable_simulation"]); // false,
            Serial.print("device_enable_nmea_logging: "); Serial.println((bool)doc["device_enable_nmea_logging"]); // false            
            conf.close();
        }
        else
        {
            Serial.println("pref-data.json not exist");
        }
        
    }
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