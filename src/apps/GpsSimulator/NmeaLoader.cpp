// NmeaLoader.cpp
//

#include "stdafx.h"
#include "NmeaLoader.h"





/////////////////////////////////////////////////////////////////////////////////////////////////
//

int Tokenizer(char * tok[], int tokLen, char * str, const char * delimiter)
{
	char * ptr = str;
	int i = 0;

	// 
	tok[i] = ptr++;

	//
	while (*ptr && i < tokLen)
	{
		if (strchr(delimiter, *ptr) != NULL)
		{
			tok[++i] = ptr + 1;
			*ptr = NULL;
		}

		ptr++;
	}

	return i + 1;
}


time_t TimeString2Seconds(const char * str)
{
	int num = atoi(str);

	int h = num / 10000;
	num = num - h * 10000;
	int m = num / 100;
	num = num - m * 100;
	int s = num;

	return h * 3600 + m * 60 + s;
}

double NmeaCoordi2Decimal(const char * angle, const char * dir)
{
	// [D]DDMM.mmmmm
	double nmea = atof(angle);
	int dd = (int)(nmea / 100);
	double ss = nmea - (dd * 100.0);
	int sign = 1;

	if (strcmp(dir, "N") != 0 && strcmp(dir, "E"))
		sign = -1;

	return sign * (dd + ss / 60.0);
}

int LoadNmea(LPCTSTR lpszFile, CPositionList & list)
{
	FILE * fp;

	USES_CONVERSION;
	if (fopen_s(&fp, T2A(lpszFile), "rt") == 0)
	{
		CString rmc, gga;
		time_t last = (time_t)-1;


		// reset list
		list.RemoveAll();

		while (!feof(fp))
		{
			//
			char line[256], temp[256];
			double lat, lon, alt, speed, track;

			memset(line, 0, sizeof(line));
			fgets(line, sizeof(line), fp);

			memset(temp, 0, sizeof(temp));
			strcpy_s(temp, sizeof(temp), line);

			//
#if 0
			char * tok[32], * context;
			int count = 0;

			for (int index = 0; index < sizeof(tok) / sizeof(tok[0]); index++)
			{
				if (index == 0)
					tok[index] = strtok_s(temp, ",", &context);
				else
					tok[index] = strtok_s(NULL, ",", &context);

				if (!tok[index])
					break;

				count = index + 1;
			}
#else
			char * tok[32];
			int count = Tokenizer(tok, sizeof(tok) / sizeof(tok[0]), temp, ",");
#endif

			if (count >= 10)
			{
				if (strcmp(tok[0], "$GPRMC") == 0 && strcmp(tok[2], "A") == 0) // fixed RMC only
				{
					time_t secs = TimeString2Seconds(tok[1]);

					if (gga.IsEmpty() || last == secs)
					{
						rmc = line;
						last = secs;
						speed = atof(tok[7]) * 1.852;
						track = atof(tok[8]);
					}
					else // mismatch with time of rmc
					{
						gga = _T("");
						rmc = _T("");
					}
				}
				else if (strcmp(tok[0], "$GPGGA") == 0 && strcmp(tok[6], "0") != 0)
				{
					time_t secs = TimeString2Seconds(tok[1]);

					if (rmc.IsEmpty() || last == secs)
					{
						gga = line;
						last = secs;
						lat = NmeaCoordi2Decimal(tok[2], tok[3]);
						lon = NmeaCoordi2Decimal(tok[4], tok[5]);
						alt = atof(tok[9]);
					}
					else // mismatch with time of rmc
					{
						gga = _T("");
						rmc = _T("");
					}
				}

				if (!gga.IsEmpty() && !rmc.IsEmpty())
				{
					//
					NmeaPosition * pos = new NmeaPosition;
					
					pos->rmc = rmc;
					pos->gga = gga;
					pos->_time = last;
					pos->latitude = lat;
					pos->longitude = lon;
					pos->altitude = alt;
					pos->speed = speed;
					pos->track = track;

					list.AddTail(pos);

					//
					gga = _T("");
					rmc = _T("");
				}
			}
		}

		fclose(fp);

		return list.GetCount();
	}

	return -1;
}