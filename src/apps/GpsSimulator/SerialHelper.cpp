// SerialHelper.cpp
//

#include "stdafx.h"
#include "SerialWnd.h"
#include "SerialHelper.h"

#include <strsafe.h>
#include <initguid.h>
#include <devguid.h>
#include <setupapi.h>

#pragma comment(lib, "Setupapi.lib")



////////////////////////////////////////////////////////////////////////////////////
//

void ShowLastError(LPTSTR lpszFunction, DWORD dwLastError)
{
	// Retrieve the system error message for the last-error code
	LPVOID lpMsgBuf;
	LPVOID lpDisplayBuf;

	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		dwLastError,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf,
		0, NULL);

	// Display the error message and exit the process

	lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT, (lstrlen((LPCTSTR)lpMsgBuf) + lstrlen((LPCTSTR)lpszFunction) + 40) * sizeof(TCHAR));
	StringCchPrintf((LPTSTR)lpDisplayBuf,
		LocalSize(lpDisplayBuf) / sizeof(TCHAR),
		TEXT("%s failed with error %d: %s"),
		lpszFunction, dwLastError, lpMsgBuf);

	AfxMessageBox((LPCTSTR)lpDisplayBuf, MB_ICONERROR | MB_OK);

	LocalFree(lpMsgBuf);
	LocalFree(lpDisplayBuf);
}

int EnumSerialPorts(std::list<SerialPort> & ports)
{
	//
	HDEVINFO hDeviceInfo = SetupDiGetClassDevs(&GUID_DEVCLASS_PORTS,
		NULL,
		NULL,
		DIGCF_PRESENT);

	if (hDeviceInfo == INVALID_HANDLE_VALUE)
		return -1;

	//
	for (int nDevice = 0; ; nDevice++)
	{
		SP_DEVINFO_DATA devInfoData;

		ZeroMemory(&devInfoData, sizeof(devInfoData));
		devInfoData.cbSize = sizeof(devInfoData);

		BOOL bRet = SetupDiEnumDeviceInfo(hDeviceInfo, nDevice, &devInfoData);
		if (!bRet)
			break;

		DWORD dwSize = 0;
		CString strName;

		SetupDiGetDeviceRegistryProperty(hDeviceInfo, &devInfoData, SPDRP_FRIENDLYNAME, NULL, NULL, 0, &dwSize);
		SetupDiGetDeviceRegistryProperty(hDeviceInfo, &devInfoData, SPDRP_FRIENDLYNAME, NULL, (PBYTE)strName.GetBuffer(dwSize), dwSize, NULL);
		strName.ReleaseBuffer();

		HKEY hDeviceKey = SetupDiOpenDevRegKey(hDeviceInfo, &devInfoData, DICS_FLAG_GLOBAL, 0, DIREG_DEV, KEY_QUERY_VALUE);
		if (hDeviceKey)
		{
			TCHAR pszPortName[32];
			DWORD dwSize = sizeof(pszPortName);
			DWORD dwType = 0;

			if ((RegQueryValueEx(hDeviceKey, _T("PortName"), NULL, &dwType, (LPBYTE)pszPortName, &dwSize) == ERROR_SUCCESS) && (dwType == REG_SZ))
			{
				if (_tcsnicmp(pszPortName, _T("COM"), 3) == 0)
				{
					int nPort = _ttoi(pszPortName + 3);
					if (nPort != 0)
					{
						ports.push_back(SerialPort(nPort, strName));
					}
				}
			}

			RegCloseKey(hDeviceKey);
		}
	}

	SetupDiDestroyDeviceInfoList(hDeviceInfo);

	return ports.size();
}



////////////////////////////////////////////////////////////////////////////////////
//

CSerial::EBaudrate Map_BaudRate[7] =
{
	CSerial::EBaud9600,
	CSerial::EBaud14400,
	CSerial::EBaud19200,
	CSerial::EBaud38400,
	CSerial::EBaud56000,
	CSerial::EBaud57600,
	CSerial::EBaud115200,
};

CSerial::EDataBits Map_DataBits[4] =
{
	CSerial::EData5,
	CSerial::EData6,
	CSerial::EData7,
	CSerial::EData8,
};

CSerial::EParity Map_Parity[5] =
{
	CSerial::EParNone,
	CSerial::EParOdd,
	CSerial::EParEven,
	CSerial::EParMark,
	CSerial::EParSpace,
};

CSerial::EStopBits Map_StopBits[3] =
{
	CSerial::EStop1,
	CSerial::EStop1_5,
	CSerial::EStop2,
};

CSerial::EHandshake Map_FlowControl[3] =
{
	CSerial::EHandshakeOff,
	CSerial::EHandshakeSoftware,
	CSerial::EHandshakeHardware,
};
