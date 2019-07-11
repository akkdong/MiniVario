// SerialHelper.h
//

#pragma once

#include "SerialWnd.h"
#include <list>



////////////////////////////////////////////////////////////////////////////////////
//

class SerialPort
{
public:
	SerialPort(int nPort, LPCTSTR lpszName) : m_nPort(nPort), m_strName(lpszName) {}

public:
	int			m_nPort;
	CString		m_strName;
};



////////////////////////////////////////////////////////////////////////////////////
//

void	ShowLastError(LPTSTR lpszFunction, DWORD dwLastError);
int		EnumSerialPorts(std::list<SerialPort> & ports);


////////////////////////////////////////////////////////////////////////////////////
//

extern CSerial::EBaudrate	Map_BaudRate[7];
extern CSerial::EDataBits	Map_DataBits[4];
extern CSerial::EParity		Map_Parity[5];
extern CSerial::EStopBits	Map_StopBits[3];
extern CSerial::EHandshake	Map_FlowControl[3];

