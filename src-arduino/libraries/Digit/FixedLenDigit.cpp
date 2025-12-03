// FixedLenDigit.cpp
//

#include <Arduino.h>

#include "FixedLenDigit.h"

/////////////////////////////////////////////////////////////////////////////
// class FixedLenDigit

int32_t FixedLenDigit::powTable[] = { 1, 10, 100, 1000, 10000, 100000, 1000000, 10000000, 100000000 };

FixedLenDigit::FixedLenDigit() : value(0), pos(0), length(0)
{
}

int32_t	FixedLenDigit::begin(int32_t v, int32_t n)
{
	if (n < 1 || 8 < n)
		return 0;
	
	if (v < 0)
	{
		value = 0 - v;
		length = n - 1;
		pos = -1;
	}
	else
	{
		value = v;
		length = n;
		pos = 0;
	}
	
	// truncate if value is too big
	int32_t div = powTable[length]; // powi(10, n);
	if (div < value)
		value = value - (value / div * div);
	
	return 1;
}

int32_t	FixedLenDigit::available()
{
	return pos < length ? true : false;
}

int32_t	FixedLenDigit::read()
{
	if (pos >= length)
		return -1;
	
	if (pos < 0)
	{
		pos = 0;
		
		return '-';
	}
	else
	{
		int32_t div = powTable[length - pos - 1]; // powi(10, length - pos - 1);
		int32_t num = value / div;
		
		value = value - (value / div * div);
		pos += 1;
		
		return '0' + num;
	}
}

int32_t FixedLenDigit::powi(int32_t base, int32_t exp)
{
	int32_t value = 1;
	
	for (int32_t i = 0; i < exp; i++)
		value = value * base;
	
	return value;
}