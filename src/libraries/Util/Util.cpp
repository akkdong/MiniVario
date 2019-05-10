// Util.cpp
//

#include "Util.h"


////////////////////////////////////////////////////////////////////////////////////////////////
//

int16_t GET_BEARING(double lat1, double lon1, double lat2, double lon2)
{
    double teta1 = TO_RADIAN(lat1);
    double teta2 = TO_RADIAN(lat2);
//  double delta1 = TO_RADIAN(lat2-lat1);
    double delta2 = TO_RADIAN(lon2-lon1);

    double y = sin(delta2) * cos(teta2);
    double x = cos(teta1)*sin(teta2) - sin(teta1)*cos(teta2)*cos(delta2);

    double bearing = atan2(y, x);
    bearing = TO_DEGREE(bearing);// radians to degrees

    return (((int16_t)bearing + 360) % 360); 

#if 0
    float dx = (lon2 - lon1);
    float dy = (lat2 - lat1);

    return (-(int16_t)TO_DEGREE(atan2(dx, dy)) + 360) % 360;
#endif
}

double GET_DISTANCE(double lat1, double lon1, double lat2, double lon2)
{
    double theta, dist;

    if ((lat1 == lat2) && (lon1 == lon2)) 
        return 0;

    theta = lon1 - lon2;
    dist = sin(TO_RADIAN(lat1)) * sin(TO_RADIAN(lat2)) + cos(TO_RADIAN(lat1)) * cos(TO_RADIAN(lat2)) * cos(TO_RADIAN(theta));
    dist = acos(dist);
    dist = TO_DEGREE(dist);
    dist = dist * 60 * 1.1515;

    return dist * 1609.344; // meter
}
