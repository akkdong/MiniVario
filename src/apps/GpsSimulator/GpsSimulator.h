
// GpsSimulator.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CGpsSimulatorApp:
// See GpsSimulator.cpp for the implementation of this class
//

class CGpsSimulatorApp : public CWinApp
{
public:
	CGpsSimulatorApp();

// Overrides
public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CGpsSimulatorApp theApp;
