
// GpsSimulatorDlg.h : header file
//

#pragma once

#include "SerialWnd.h"
#include "SerialHelper.h"
#include "NmeaLoader.h"
#include "ActionTrigger.h"
#include "TrackDialog.h"
#include "SerialMonitor.h"

enum SimulatorAction
{
	ACTION_NONE,
	ACTION_UNFIX,
	ACTION_READY,
	ACTION_RUN
};


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
	virtual void			DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

	void					RefillSerialPort();
	BOOL					UpdateData(BOOL bSaveAndValidate = TRUE);
	void					UpdateTitle();
	void					UpdateControls();

	void					ConnectSerial();
	void					DisconnectSerial();

	void					SendData(const char * nmea);


// Implementation
protected:
	//
	CSerialWnd				m_Serial;

	CString					m_strPortName;
	int						m_sPortNum;
	CSerial::EBaudrate		m_sBaudRate;
	CSerial::EDataBits		m_sDataBits;
	CSerial::EParity		m_sParity;
	CSerial::EStopBits		m_sStopBits;
	CSerial::EHandshake		m_sHandshake;
	BOOL					m_bConnected;
	std::list<SerialPort>	m_SerialPorts;

	//
	CPositionList			m_NmeaPosList;
	ActionTrigger			m_ActionTrigger;

	//
	CComboBox				m_wndSerialPort;
	CComboBox				m_wndSerialBaudRate;
	CListCtrl				m_wndNmeaList;
	int						m_nAction;
	CComboBox				m_wndPeriod;
	int						m_nPeriod;

	//
	std::string				m_strMonitor;

	//
	CTrackDialog *			m_pDlgTrack;
	CSerialMonitor *		m_pDlgMonitor;

	//
	HICON					m_hIcon;

	// Generated message map functions
	virtual BOOL			OnInitDialog();
	afx_msg void			OnPaint();
	afx_msg HCURSOR			OnQueryDragIcon();
	afx_msg void			OnDestroy();

	afx_msg void			OnBnClickedRefresh();
	afx_msg void			OnBnClickedConnect();
	afx_msg void			OnBnClickedLoad();
	afx_msg void			OnBnClickedCancel();
	afx_msg void			OnCbnSelchangeAction();
	afx_msg void			OnCbnSelchangePeriod();

	afx_msg LRESULT			OnSerialMessage(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT			OnActionTriggerEvent(WPARAM wParam, LPARAM lParam);

	DECLARE_MESSAGE_MAP()
};
