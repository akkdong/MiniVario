// VerticalSpeedCalculator.cpp
//

#include "VerticalSpeedCalculator.h"
#include <string.h>


////////////////////////////////////////////////////////////////////////////////////////////////
// class VerticalSpeedCalculator

VerticalSpeedCalculator::VerticalSpeedCalculator()
{
    reset();
}

void VerticalSpeedCalculator::add(float vSpeed)
{
    int i1 = (mNext + DEF_FREQUENCY * (MAX_PERIOD - AVG_PEROID_1) + 1) % (MAX_PERIOD * DEF_FREQUENCY);
    int i2 = (mNext + DEF_FREQUENCY * (MAX_PERIOD - AVG_PEROID_2) + 1) % (MAX_PERIOD * DEF_FREQUENCY);
    int i3 = (mNext + DEF_FREQUENCY * (MAX_PERIOD - AVG_PEROID_3) + 1) % (MAX_PERIOD * DEF_FREQUENCY);

    mVertSpeedSUM1 -= mData[i1];
    mVertSpeedSUM2 -= mData[i2];
    mVertSpeedSUM3 -= mData[i3];

    mData[mNext] = vSpeed;
    mNext = (mNext + 1) % (MAX_PERIOD * DEF_FREQUENCY);

    mVertSpeedSUM1 += vSpeed;
    mVertSpeedSUM2 += vSpeed;
    mVertSpeedSUM3 += vSpeed;

    mVertSpeedAVG1 = mVertSpeedSUM1 / (DEF_FREQUENCY * AVG_PEROID_1);
    mVertSpeedAVG2 = mVertSpeedSUM2 / (DEF_FREQUENCY * AVG_PEROID_2);
    mVertSpeedAVG3 = mVertSpeedSUM3 / (DEF_FREQUENCY * AVG_PEROID_3);
}

void VerticalSpeedCalculator::reset()
{
    #if 0
    mVertSpeedAVG1 = mVertSpeedAVG2 = mVertSpeedAVG3 = 0.0;
    mVertSpeedSUM1 = mVertSpeedSUM2 = mVertSpeedSUM3 = 0.0;

    mNext = 0;

    memset(&mData[0], 0, sizeof(mData));
    #else
    memset(this, 0, sizeof(VerticalSpeedCalculator));
    #endif
}