
// GpsSimulatorDlg.cpp : implementation file
//

#include "stdafx.h"
#include "GpsSimulator.h"
#include "GpsSimulatorDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CGpsSimulatorDlg dialog

CGpsSimulatorDlg::CGpsSimulatorDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_GPSSIMULATOR_DIALOG, pParent)
{
	//
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	//
	m_sPortNum = AfxGetApp()->GetProfileInt(_T("SerialSettings"), _T("PortNum"), 1);
	m_sBaudRate = (CSerial::EBaudrate)AfxGetApp()->GetProfileInt(_T("SerialSettings"), _T("BaudRate"), CSerial::EBaud115200);
	m_sDataBits = CSerial::EData8;
	m_sParity = CSerial::EParNone;
	m_sStopBits = CSerial::EStop1;
	m_sHandshake = CSerial::EHandshakeOff;

	m_bConnected = FALSE;

	//
	m_nAction = 0;
	m_nPeriod = 4;

	//
	m_pDlgTrack = NULL;
	m_pDlgMonitor = NULL;
}

void CGpsSimulatorDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);

	//
	DDX_Control(pDX, IDC_SERIAL_PORT, m_wndSerialPort);
	DDX_Control(pDX, IDC_SERIAL_BAUDRATE, m_wndSerialBaudRate);

	//
	DDX_Control(pDX, IDC_LIST_NMEA, m_wndNmeaList);
	DDX_CBIndex(pDX, IDC_ACTION, m_nAction);
	DDX_Control(pDX, IDC_PERIOD, m_wndPeriod);
	DDX_CBIndex(pDX, IDC_PERIOD, m_nPeriod);

}

BEGIN_MESSAGE_MAP(CGpsSimulatorDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_REFRESH, &CGpsSimulatorDlg::OnBnClickedRefresh)
	ON_BN_CLICKED(IDC_CONNECT, &CGpsSimulatorDlg::OnBnClickedConnect)
	ON_BN_CLICKED(IDC_LOAD, &CGpsSimulatorDlg::OnBnClickedLoad)
	ON_BN_CLICKED(IDCANCEL, &CGpsSimulatorDlg::OnBnClickedCancel)
	ON_CBN_SELCHANGE(IDC_ACTION, &CGpsSimulatorDlg::OnCbnSelchangeAction)
	ON_CBN_SELCHANGE(IDC_PERIOD, &CGpsSimulatorDlg::OnCbnSelchangePeriod)

	ON_REGISTERED_MESSAGE(CSerialWnd::mg_nDefaultComMsg, OnSerialMessage)
	ON_MESSAGE(WM_ACTION_TRIGGER, OnActionTriggerEvent)
END_MESSAGE_MAP()


// CGpsSimulatorDlg message handlers

BOOL CGpsSimulatorDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	//
	m_wndNmeaList.InsertColumn(0, _T("Time"), LVCFMT_LEFT, 80, 0);
	m_wndNmeaList.InsertColumn(1, _T("Latitude"), LVCFMT_LEFT, 100, 1);
	m_wndNmeaList.InsertColumn(2, _T("Longitude"), LVCFMT_LEFT, 100, 2);
	m_wndNmeaList.InsertColumn(3, _T("Altitude"), LVCFMT_LEFT, 90, 3);
	m_wndNmeaList.InsertColumn(4, _T("Speed"), LVCFMT_LEFT, 90, 4);
	m_wndNmeaList.InsertColumn(5, _T("Track"), LVCFMT_LEFT, 90, 5);
	m_wndNmeaList.SetExtendedStyle(m_wndNmeaList.GetExtendedStyle() | LVS_EX_FULLROWSELECT);

	CButton * pButton = (CButton *)GetDlgItem(IDC_REFRESH);
	pButton->SetIcon((HICON)LoadImage(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDI_REFRESH), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR));

	//
	m_ActionTrigger.Create(this);

	//
	RefillSerialPort();
	UpdateData(FALSE);
	UpdateControls();
	UpdateTitle();

	//
	m_pDlgTrack = new CTrackDialog;
	m_pDlgTrack->Create(IDD_TRACK, this);

	m_pDlgMonitor = new CSerialMonitor;
	m_pDlgMonitor->Create(IDD_MONITOR, this);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CGpsSimulatorDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CGpsSimulatorDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CGpsSimulatorDlg::OnDestroy()
{
	if (m_pDlgTrack)
	{
		m_pDlgTrack->DestroyWindow();
		delete m_pDlgTrack;
		m_pDlgTrack = NULL;
	}

	if (m_pDlgMonitor)
	{
		m_pDlgMonitor->DestroyWindow();
		delete m_pDlgMonitor;
		m_pDlgMonitor = NULL;
	}

	CDialogEx::OnDestroy();
}


void CGpsSimulatorDlg::RefillSerialPort()
{
	//
	m_wndSerialPort.ResetContent();

	m_SerialPorts.clear();
	EnumSerialPorts(m_SerialPorts);

	//
	int nPortIdx = -1;
	int nPortNum = -1;

	for (std::list<SerialPort>::iterator it = m_SerialPorts.begin(); it != m_SerialPorts.end(); it++)
	{
		SerialPort * pPort = (SerialPort *)&(*it);

		int nIndex = m_wndSerialPort.AddString(pPort->m_strName);
		m_wndSerialPort.SetItemDataPtr(nIndex, pPort);

		if (pPort->m_nPort == m_sPortNum)
		{
			nPortIdx = nIndex;
			nPortNum = pPort->m_nPort;
		}
	}

	if (nPortIdx < 0)
	{
		m_sPortNum = -1;

		if (m_wndSerialPort.GetCount() > 0)
		{
			//m_sPortNum = (*m_SerialPorts.begin()).m_nPort;
			m_sPortNum = ((SerialPort *)m_wndSerialPort.GetItemDataPtr(0))->m_nPort;
			m_wndSerialPort.SetCurSel(0);
		}
	}
	else
	{
		m_wndSerialPort.SetCurSel(nPortIdx);
		m_sPortNum = nPortNum;
	}
}

BOOL CGpsSimulatorDlg::UpdateData(BOOL bSaveAndValidate)
{
	if (!bSaveAndValidate)
	{
		// Port
		if (m_sPortNum)
		{
			for (int i = 0; i < m_wndSerialPort.GetCount(); i++)
			{
				if (((SerialPort *)m_wndSerialPort.GetItemDataPtr(i))->m_nPort == m_sPortNum)
				{
					m_wndSerialPort.SetCurSel(i);
					break;
				}
			}
		}

		// BaudRate
		for (int i = 0; i < sizeof(Map_BaudRate) / sizeof(Map_BaudRate[0]); i++)
		{
			if (Map_BaudRate[i] == m_sBaudRate)
			{
				m_wndSerialBaudRate.SetCurSel(i);
				break;
			}
		}

		//
		//UpdateTitle();
	}

	BOOL bRet = CDialog::UpdateData(bSaveAndValidate);

	if (bSaveAndValidate)
	{
		//
		int index;

		// Port
		if ((index = m_wndSerialPort.GetCurSel()) >= 0)
		{
			SerialPort * pPort = (SerialPort *)m_wndSerialPort.GetItemDataPtr(index);

			m_strPortName.Format(_T("\\\\.\\COM%d"), pPort->m_nPort);
			m_sPortNum = pPort->m_nPort;
		}

		// BaudRate
		if ((index = m_wndSerialBaudRate.GetCurSel()) >= 0)
			m_sBaudRate = Map_BaudRate[index];
	}

	return bRet;
}

void CGpsSimulatorDlg::UpdateTitle()
{
	CString strTitle;
	strTitle.Format(_T("GPS Simulator : Device %s"), m_bConnected ? _T("connected") : _T("disconnected"));

	SetWindowText(strTitle);
}

void CGpsSimulatorDlg::UpdateControls()
{
	GetDlgItem(IDC_SERIAL_PORT)->EnableWindow(m_bConnected ? FALSE : TRUE);
	GetDlgItem(IDC_SERIAL_BAUDRATE)->EnableWindow(m_bConnected ? FALSE : TRUE);
	GetDlgItem(IDC_ACTION)->EnableWindow(m_bConnected ? TRUE : FALSE);

	GetDlgItem(IDC_LOAD)->EnableWindow(m_nAction == ACTION_NONE ? TRUE : FALSE);
}


void CGpsSimulatorDlg::OnBnClickedRefresh()
{
	UpdateData(TRUE);
	RefillSerialPort();
}


void CGpsSimulatorDlg::OnBnClickedConnect()
{
	UpdateData();

	if (m_bConnected)
		DisconnectSerial();
	else
		ConnectSerial();
}


void CGpsSimulatorDlg::OnBnClickedLoad()
{
	CFileDialog dlg(TRUE, NULL, NULL, OFN_HIDEREADONLY, _T("NMEA log files(*.log)|*.log|All files(*.*)|*.*||"));

	if (dlg.DoModal() == IDOK)
	{
		if (LoadNmea(dlg.GetPathName(), m_NmeaPosList) > 0)
		{
			m_wndNmeaList.DeleteAllItems();

			POSITION pos = m_NmeaPosList.GetHeadPosition();
			while (pos)
			{
				NmeaPosition * pNmea = m_NmeaPosList.GetNext(pos);
				CString str;
				int t, h, m, s;

				t = (int)pNmea->_time;
				h = t / 3600;
				t = t - h * 3600;
				m = t / 60;
				s = t - m * 60;

				str.Format(_T("%02d:%02d:%02d"), h, m, s);
				int item = m_wndNmeaList.InsertItem(m_wndNmeaList.GetItemCount(), str);

				if (item != LB_ERR)
				{
					str.Format(_T("%.6f"), pNmea->latitude);
					m_wndNmeaList.SetItemText(item, 1, str);

					str.Format(_T("%.6f"), pNmea->longitude);
					m_wndNmeaList.SetItemText(item, 2, str);

					str.Format(_T("%.0f"), pNmea->altitude);
					m_wndNmeaList.SetItemText(item, 3, str);

					str.Format(_T("%.1f"), pNmea->speed);
					m_wndNmeaList.SetItemText(item, 4, str);

					str.Format(_T("%.0f"), pNmea->track);
					m_wndNmeaList.SetItemText(item, 5, str);

					m_wndNmeaList.SetItemData(item, (DWORD_PTR)pNmea);
				}
			}

			if (m_wndNmeaList.GetItemCount() > 0)
			{
				m_wndNmeaList.SetItemState(0, LVIS_SELECTED, LVIS_SELECTED);
				m_wndNmeaList.EnsureVisible(0, FALSE);
			}

			if (m_pDlgTrack)
				m_pDlgTrack->ResetPosition();

			if (m_pDlgMonitor)
			{
				CString str;
				str.Format(_T("Load %s"), dlg.GetPathName());
				m_pDlgMonitor->AddMessage(str);
			}
		}
		else
		{
			AfxMessageBox(_T("File open failed or No valid data!!"), MB_ICONSTOP);
		}
	}
}


void CGpsSimulatorDlg::OnBnClickedCancel()
{
	//
	if (m_bConnected)
		DisconnectSerial();

	//
	CDialogEx::OnCancel();
}


void CGpsSimulatorDlg::OnCbnSelchangeAction()
{
	int nActionOld = m_nAction;
	UpdateData(TRUE);

	if (m_nAction > ACTION_UNFIX && m_wndNmeaList.GetItemCount() == 0)
	{
		m_nAction = nActionOld;
		UpdateData(FALSE);
	}

	if (nActionOld != m_nAction)
	{
		switch (m_nAction)
		{
		case ACTION_UNFIX :
		case ACTION_READY :
		case ACTION_RUN :
			if (m_pDlgTrack)
				m_pDlgTrack->ShowWindow(SW_SHOWNORMAL);
			if (m_pDlgMonitor)
			{
				CString str;
				switch (m_nAction)
				{
				case ACTION_UNFIX: str = _T("Action UNFIX"); break;
				case ACTION_READY: str = _T("Action READY"); break;
				case ACTION_RUN: str = _T("Action RUN"); break;
				}
				m_pDlgMonitor->AddMessage(str);
			}
			break;

		default :
			if (m_pDlgTrack)
			{
				m_pDlgTrack->ShowWindow(SW_HIDE);
				m_pDlgTrack->ResetPosition();
			}
			if (m_pDlgMonitor)
				m_pDlgMonitor->AddMessage(_T("Action NONE"));
			break;
		}

		UpdateControls();
	}
}

void CGpsSimulatorDlg::OnCbnSelchangePeriod()
{
	UpdateData(TRUE);

	CString strText; 
	m_wndPeriod.GetLBText(m_nPeriod, strText);

	m_ActionTrigger.SetPeriod(_ttoi(strText));
}

void CGpsSimulatorDlg::ConnectSerial()
{
	//
	if (m_sPortNum > 0)
	{
		//
		AfxGetApp()->WriteProfileInt(_T("SerialSettings"), _T("PortNum"), m_sPortNum);
		AfxGetApp()->WriteProfileInt(_T("SerialSettings"), _T("BaudRate"), m_sBaudRate);

		//
		CWaitCursor wait;
		LONG lResult;

		if ((lResult = m_Serial.Open(m_strPortName, m_hWnd)) == ERROR_SUCCESS)
		{
			//
			m_Serial.Setup(m_sBaudRate, m_sDataBits, m_sParity, m_sStopBits);
			m_Serial.SetupHandshaking(m_sHandshake);
			m_Serial.SetupReadTimeouts(CSerial::EReadTimeoutNonblocking);

			//
			m_bConnected = TRUE;

			//
			UpdateControls();
			UpdateTitle();

			GetDlgItem(IDC_CONNECT)->SetWindowText(_T("Disconnect"));

			if (m_pDlgMonitor)
				m_pDlgMonitor->ShowWindow(SW_SHOWNORMAL);
		}
		else
		{
			//
			ShowLastError(_T("Serial port open"), lResult);
		}
	}
}

void CGpsSimulatorDlg::DisconnectSerial()
{
	m_Serial.Close();

	m_bConnected = FALSE;
	m_nAction = 0;

	UpdateData(FALSE);
	UpdateControls();
	UpdateTitle();

	GetDlgItem(IDC_CONNECT)->SetWindowText(_T("Connect"));

	//if (m_pDlgMonitor)
	//	m_pDlgMonitor->ShowWindow(SW_HIDE);

	if (m_pDlgTrack)
		m_pDlgTrack->ShowWindow(SW_HIDE);
}

LRESULT	CGpsSimulatorDlg::OnSerialMessage(WPARAM wParam, LPARAM lParam)
{
	CSerial::EEvent eEvent = CSerial::EEvent(LOWORD(wParam));
	CSerial::EError eError = CSerial::EError(HIWORD(wParam));

	if (eEvent & CSerial::EEventRecv)
	{
		CHAR ch;
		DWORD dwRead;

		while (m_Serial.Read(&ch, sizeof(ch), &dwRead) == ERROR_SUCCESS && dwRead == sizeof(ch))
		{
			if (ch == '\r' || ch == '\n')
			{
				if (m_strMonitor.size() > 0)
				{
#if 0
					if (m_pDlgMonitor)
						m_pDlgMonitor->AddMessage(CString(m_strMonitor.c_str()) + _T("\r\n"));
#else
					if (m_pDlgMonitor)
						m_pDlgMonitor->AddMessage(CString(m_strMonitor.c_str()));
#endif

					m_strMonitor.clear();
				}
			}
			else
			{
				m_strMonitor.push_back(ch);
			}
		}
	}

	return 0L;
}

void CGpsSimulatorDlg::SendData(const char * nmea)
{
	const char * ptr = nmea;

	while (*ptr)
	{
		char sz[2] = { 0, 0 };
		sz[0] = *ptr++;

		m_Serial.Write(sz);
	}
}

LRESULT CGpsSimulatorDlg::OnActionTriggerEvent(WPARAM wParam, LPARAM lParam)
{
	if (m_bConnected && m_nAction > ACTION_NONE)
	{
		POSITION pos;

		switch (m_nAction)
		{
		case ACTION_UNFIX:
			m_Serial.Write("$GPRMC,,V,,,,,,,,,,N*53\r\n");
			m_Serial.Write("$GPGGA,,,,,,0,00,99.99,,,,,,*48\r\n");
			break;

		case ACTION_READY:
			if ((pos = m_wndNmeaList.GetFirstSelectedItemPosition()) != NULL)
			{
				int nItem = m_wndNmeaList.GetNextSelectedItem(pos);
				NmeaPosition * pos = (NmeaPosition *)m_wndNmeaList.GetItemData(nItem);

				USES_CONVERSION;
				m_Serial.Write(T2A(pos->rmc));
				TRACE(T2A(pos->rmc));
				Sleep(100);
				m_Serial.Write(T2A(pos->gga));
				TRACE(T2A(pos->gga));

				if (m_pDlgTrack)
					m_pDlgTrack->AddPosition(pos->latitude, pos->longitude, pos->track);
			}
			break;

		case ACTION_RUN:
			if ((pos = m_wndNmeaList.GetFirstSelectedItemPosition()) != NULL)
			{
				int nItem = m_wndNmeaList.GetNextSelectedItem(pos);
				NmeaPosition * pos = (NmeaPosition *)m_wndNmeaList.GetItemData(nItem);

				USES_CONVERSION;
				m_Serial.Write(T2A(pos->rmc));
				TRACE(T2A(pos->rmc));
				Sleep(100);
				m_Serial.Write(T2A(pos->gga));
				TRACE(T2A(pos->gga));

				if (m_pDlgTrack)
					m_pDlgTrack->AddPosition(pos->latitude, pos->longitude, pos->track);

				if ((nItem + 1) < m_wndNmeaList.GetItemCount())
				{
					m_wndNmeaList.SetItemState(nItem + 1, LVIS_SELECTED, LVIS_SELECTED);
					m_wndNmeaList.EnsureVisible(nItem + 1, FALSE);
				}
			}
			break;
		}
	}

	return 0L;
}
