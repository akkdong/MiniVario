// Util.h
//

#ifndef __UTIL_H__
#define __UTIL_H__

#include <Arduino.h>


////////////////////////////////////////////////////////////////////////////////////////////////
//

#define _MIN(x, y)              ((x) < (y) ? (x) : (y))
#define _MAX(x, y)              ((x) > (y) ? (x) : (y))
#define _CLAMP(x, min, max)     (((x) < (min)) ? (min) : (((x) > (max) ? (max) : (x))))
#define _ABS(x)                 ((x) > 0 ? (x) : (-(x)))


////////////////////////////////////////////////////////////////////////////////////////////////
//

inline double TO_RADIAN(double deg)
{
    return deg / 180.0 * PI;
}

inline double TO_DEGREE(double rad)
{
    return rad * (180.0 / PI);
}



////////////////////////////////////////////////////////////////////////////////////////////////
//

int16_t     GET_BEARING(double lat1, double lon1, double lat2, double lon2);
double      GET_DISTANCE(double lat1, double lon1, double lat2, double lon2);


#endif // __UTIL_H__
