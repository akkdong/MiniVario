// AGL.cpp
//

#include "AGL.h"
#include "Util.h"


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

AGL::AGL() : mLat(0), mLon(0)
{
}

AGL::~AGL()
{
	closeFile();
}

double AGL::getGroundLevel(double _lat, double _lon)
{
	if (! mFile || (int)_lat != mLat || (int)_lon != mLon)
	{
		mLat = (int16_t)_lat;
		mLon = (int16_t)_lon;

		int16_t n_lat = _ABS(mLat), n_lon = _ABS(mLon);
		char c_lat, c_lon;

		if (mLat >= 0)
		{
			c_lat = 'N';
		}
		else
		{
			c_lat = 'S';
			n_lat = n_lat + 1;
		}

		if (mLon >= 0)
		{
			c_lon = 'E';
		}
		else
		{
			c_lon = 'W';
			n_lon = n_lon + 1;
		}

		char name[16];
		sprintf(name, "%c%02d%c%03d.HGT", c_lat, n_lat, c_lon, n_lon);

		closeFile();
		openFile(name);
	}

	if (! mFile)
		return 0.0;

	return getGroundLevel(&mFile, _lat, _lon);
}


double AGL::getGroundLevel(File * file, double lat, double lon)
{
	uint16_t num_points_x;
	uint16_t num_points_y;

	lat = lat - (int)lat;
	lon = lon - (int)lon;

	if (lat < 0)
		lat = 1 + lat;

	if (lon < 0)
		lon = 1 + lon;

	// support 1201x1201 only : 3601x3601
    num_points_x = num_points_y = 1201;

	double coord_div_x = 1.0 / (1201 - 2); // 1.0 / (num_points_x - 2);
	double coord_div_y = 1.0 / (1201 - 2); // 1.0 / (num_points_y - 2);

	uint16_t y = (uint16_t)(lat / coord_div_y);
	uint16_t x = (uint16_t)(lon / coord_div_x);

	int16_t alt11, alt12, alt21, alt22;
	uint8_t temp[4];

	//seek to position
	uint32_t pos = ((uint32_t)x + num_points_x * (uint32_t)((num_points_y - y) - 1)) * 2;

	file->seek(pos, SeekSet);
    file->read(temp, sizeof(temp));

	// big-endian --> little endian
	alt11 = (temp[0] << 8) + temp[1];
	alt21 = (temp[2] << 8) + temp[3];

	// opposite position
	pos -= num_points_x * 2;

	file->seek(pos, SeekSet);
    file->read(temp, sizeof(temp));

	// big-endian --> little endian
	alt12 = (temp[0] << 8) + temp[1];
	alt22 = (temp[2] << 8) + temp[3];

	//get point displacement
	double lat_dr = lat * coord_div_y;
	double lon_dr = lon * coord_div_x;

	//compute height by using bilinear interpolation
	double alt1 = alt11 + (alt12 - alt11) * lat_dr;
	double alt2 = alt21 + (alt22 - alt21) * lat_dr;

#if 0
	/* Find the minimum altitude: */
	int16_t alt_min = min(alt11, alt12);
	alt_min = min(alt_min, alt21);
	alt_min = min(alt_min, alt22);

	/* Find the maximum altitude: */
	int16_t alt_max = max(alt11, alt12);
	alt_max = max(alt_max, alt21);
	alt_max = max(alt_max, alt22);
#endif

	return alt1 + (alt2 - alt1) * lon_dr;
}

int AGL::openFile(const char * name)
{
	if (mFile)
		return -1;

	char file[32]; // /agl/NXXEXXX.HGT

	strcpy(file, "/agl/");
	strcat(file, name);

    mFile = SD_MMC.open(file, FILE_READ);
	if (mFile)
        return 0;

    return -1;
}

void AGL::closeFile()
{
	if (mFile)
		mFile.close();
}