// NmeaLoader.h
//

#ifndef __NMEALOADER_H__
#define __NMEALOADER_H__


/////////////////////////////////////////////////////////////////////////////////////////////////
//

struct NmeaPosition
{
	// NMEA raw data
	CString rmc;
	CString gga;

	//
	time_t	_time;

	double	latitude;
	double	longitude;
	double	altitude;
	double	speed;
	double	track;
};


class CPositionList : public CTypedPtrList<CPtrList, NmeaPosition *>
{
public:
	CPositionList() {}
	~CPositionList() { RemoveAll(); }

	void RemoveAll()
	{
		while (GetHeadPosition())
			delete RemoveHead();
	}
};




/////////////////////////////////////////////////////////////////////////////////////////////////
//

int LoadNmea(LPCTSTR lpszFile, CPositionList & list);



#endif // __NMEALOADER_H__

