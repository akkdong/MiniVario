// DateTime.ino
//

#include <sys/time.h>

void setup()
{
    Serial.begin(115200);
    Serial.println("DateTime Test....");


    struct tm tm;
    tm.tm_year = 2018 - 1900;
    tm.tm_mon = 10;
    tm.tm_mday = 15;
    tm.tm_hour = 14;
    tm.tm_min = 10;
    tm.tm_sec = 10;

    time_t t = mktime(&tm);
    Serial.print("Setting time: "); Serial.println(asctime(&tm));

    struct timeval now = { t, 0 };
    settimeofday(&now, NULL);
}

void loop()
{
    time_t now = time(0);

    // Convert now to tm struct for local timezone
    tm* localtm = localtime(&now);
    Serial.print("The local date and time is: "); Serial.println(asctime(localtm));

    delay(1000);
    delay(1000);
    delay(1000);
    delay(1000);
    delay(1000);
}
