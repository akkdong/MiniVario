
// GpsSimulatorDlg.h : header file
//

#pragma once

#include "SerialWnd.h"


// CGpsSimulatorDlg dialog
class CGpsSimulatorDlg : public CDialogEx
{
// Construction
public:
	CGpsSimulatorDlg(CWnd* pParent = nullptr);	// standard constructor

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_GPSSIMULATOR_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	CSerialWnd			m_Serial;

	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
};
