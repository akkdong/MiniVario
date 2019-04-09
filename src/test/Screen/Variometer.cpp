// Variometer.cpp
//

#include "Variometer.h"


////////////////////////////////////////////////////////////////////////////////////////////////
// class Variometer

Variometer::Variometer()
{
}

const char * Variometer::getLabel(VarioDisplayType type)
{
	switch (type)
	{
	case VarioData_Speed_Ground :
		return "Speed";
	case VarioData_Speed_Air :
		return "Air Speed";
	case VarioData_Heading :
		return "Heading";
//	case VarioData_Heading_GPS :
//		return "Heading";
//	case VarioData_Heading_Compass :
//		return "Heading";
//	case VarioData_Bearing :
//		return "Heading";
	case VarioData_Longitude :
		return "Long";
	case VarioData_Latitude :
		return "Lat";
	case VarioData_Altitude_GPS :		// QNH
		return "Altitude";
	case VarioData_Altitude_Baro :	// QNE
		return "Altitude";
	case VarioData_Altitude_AGL :		// QFE
		return "Altitude";
	case VarioData_Altitude_Ref1 :	// QFE
		return "Altitude";
	case VarioData_Altitude_Ref2 :	// QFE
		return "Altitude";
	case VarioData_Altitude_Ref3 :	// QFE
		return "Altitude";
	case VarioData_Glide_Ratio :
		return "LD";
	case VarioData_Vario_Active :
		return "Vario";
	case VarioData_Vario_Lazy :		// 1s average
		return "Vario";
	case VarioData_DateTime :
		return "Date";
	case VarioData_Time_Current :
		return "Time";
	case VarioData_Time_Flight :
		return "Fly";
	case VarioData_Pressure :
		return "Prs";
	case VarioData_Temperature :
		return "Temp";
//	case VarioData_Thermal_Time :
//		return "";
//	case VarioData_Thermal_Gain :
//		return "";
	}
	
	return "";
}

const char * Variometer::getUnit(VarioDisplayType type)
{
	switch (type)
	{
	case VarioData_Speed_Ground :
		return "Km/h";
	case VarioData_Speed_Air :
		return "Km/h";
	case VarioData_Heading :
		return "Deg";
//	case VarioData_Heading_GPS :
//		return "Deg";
//	case VarioData_Heading_Compass :
//		return "Deg";
//	case VarioData_Bearing :
//		return "Deg";
	case VarioData_Longitude :
		return "";
	case VarioData_Latitude :
		return "";
	case VarioData_Altitude_GPS :		// QNH
		return "m";
	case VarioData_Altitude_Baro :	// QNE
		return "m";
	case VarioData_Altitude_AGL :		// QFE
		return "m";
	case VarioData_Altitude_Ref1 :	// QFE
		return "m";
	case VarioData_Altitude_Ref2 :	// QFE
		return "m";
	case VarioData_Altitude_Ref3 :	// QFE
		return "m";
	case VarioData_Glide_Ratio :
		return "";
	case VarioData_Vario_Active :
		return "m/s";
	case VarioData_Vario_Lazy :		// 1s average
		return "m/s";
	case VarioData_DateTime :
		return "";
	case VarioData_Time_Current :
		return "";
	case VarioData_Time_Flight :
		return "";
	case VarioData_Pressure :
		return "hPa";
	case VarioData_Temperature :
		return "C";
//	case VarioData_Thermal_Time :
//		return "";
//	case VarioData_Thermal_Gain :
//		return "";
	}
	
	return "";
}

const char * Variometer::getString(VarioDisplayType type)
{
	switch (type)
	{
	case VarioData_Speed_Ground :
		return "";
	case VarioData_Speed_Air :
		return "";
	case VarioData_Heading :
		return "";
//	case VarioData_Heading_GPS :
//		return "";
//	case VarioData_Heading_Compass :
//		return "";
//	case VarioData_Bearing :
//		return "";
	case VarioData_Longitude :
		return "";
	case VarioData_Latitude :
		return "";
	case VarioData_Altitude_GPS :		// QNH
		return "";
	case VarioData_Altitude_Baro :	// QNE
		return "";
	case VarioData_Altitude_AGL :		// QFE
		return "";
	case VarioData_Altitude_Ref1 :	// QFE
		return "";
	case VarioData_Altitude_Ref2 :	// QFE
		return "";
	case VarioData_Altitude_Ref3 :	// QFE
		return "";
	case VarioData_Glide_Ratio :
		return "";
	case VarioData_Vario_Active :
		return "";
	case VarioData_Vario_Lazy :		// 1s average
		return "";
	case VarioData_DateTime :
		return "";
	case VarioData_Time_Current :
		return "";
	case VarioData_Time_Flight :
		return "";
	case VarioData_Pressure :
		return "";
	case VarioData_Temperature :
		return "";
//	case VarioData_Thermal_Time :
//		return "";
//	case VarioData_Thermal_Gain :
//		return "";
	}
	
	return "";
}
