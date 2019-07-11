
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
}

BEGIN_MESSAGE_MAP(CGpsSimulatorDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_REFRESH, &CGpsSimulatorDlg::OnBnClickedRefresh)
	ON_BN_CLICKED(IDC_CONNECT, &CGpsSimulatorDlg::OnBnClickedConnect)
	ON_BN_CLICKED(IDC_LOAD, &CGpsSimulatorDlg::OnBnClickedLoad)
	ON_BN_CLICKED(IDCANCEL, &CGpsSimulatorDlg::OnBnClickedCancel)
	ON_CBN_SELCHANGE(IDC_ACTION, &CGpsSimulatorDlg::OnCbnSelchangeAction)

	ON_REGISTERED_MESSAGE(CSerialWnd::mg_nDefaultComMsg, OnSerialMessage)
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
	m_wndNmeaList.InsertColumn(0, _T("Time"), LVCFMT_LEFT, 100, 0);
	m_wndNmeaList.InsertColumn(1, _T("Latitude"), LVCFMT_LEFT, 100, 1);
	m_wndNmeaList.InsertColumn(2, _T("Longitude"), LVCFMT_LEFT, 100, 2);
	m_wndNmeaList.InsertColumn(3, _T("Altitude"), LVCFMT_LEFT, 100, 3);
	m_wndNmeaList.SetExtendedStyle(m_wndNmeaList.GetExtendedStyle() | LVS_EX_FULLROWSELECT);

	CButton * pButton = (CButton *)GetDlgItem(IDC_REFRESH);
	pButton->SetIcon((HICON)LoadImage(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDI_REFRESH), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR));


	//
	RefillSerialPort();
	UpdateData(FALSE);
	UpdateControls();
	UpdateTitle();

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
	// TODO: Add your control notification handler code here
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
		// ...
	}
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
			//
		}
	}

	return 0L;
}