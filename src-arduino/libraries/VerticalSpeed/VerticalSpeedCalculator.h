// VerticalSpeedCalculator.h
//

#ifndef __VERTICALSPEEDCALCULATOR_H__
#define __VERTICALSPEEDCALCULATOR_H__

#include <Arduino.h>

#define MAX_PERIOD          (10)        // 10s
#define DEF_FREQUENCY       (50)        // 50Hz

#define AVG_PEROID_1        (10)        // 10s
#define AVG_PEROID_2        (2)         // 2s
#define AVG_PEROID_3        (1)         // 1s


////////////////////////////////////////////////////////////////////////////////////////////////
// class VerticalSpeedCalculator

class VerticalSpeedCalculator
{
public:
    VerticalSpeedCalculator();

public:
    void                add(float vSpeed);
    void                reset();

public:
    float               mVertSpeedAVG1;
    float               mVertSpeedAVG2;
    float               mVertSpeedAVG3;

protected:
    float               mData[MAX_PERIOD * DEF_FREQUENCY];

    int                 mNext;

    float               mVertSpeedSUM1;
    float               mVertSpeedSUM2;
    float               mVertSpeedSUM3;
};

#endif // __VERTICALSPEEDCALCULATOR_H__
