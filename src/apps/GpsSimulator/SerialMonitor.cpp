// SerialMonitor.cpp : implementation file
//

#include "stdafx.h"
#include "GpsSimulator.h"
#include "SerialMonitor.h"
#include "afxdialogex.h"


// CSerialMonitor dialog

IMPLEMENT_DYNAMIC(CSerialMonitor, CDialogEx)

CSerialMonitor::CSerialMonitor(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_MONITOR, pParent)
{
	m_bAutoScroll = TRUE;
	m_strContent = _T("");
}

CSerialMonitor::~CSerialMonitor()
{
}

void CSerialMonitor::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_MESSAGE, m_wndMessage);
	DDX_Control(pDX, IDC_CONTENT, m_wndEdit);
	DDX_Check(pDX, IDC_AUTO_SCROLL, m_bAutoScroll);
}


BEGIN_MESSAGE_MAP(CSerialMonitor, CDialogEx)
	ON_BN_CLICKED(IDC_AUTO_SCROLL, OnClickAutoScroll)
END_MESSAGE_MAP()


// CSerialMonitor message handlers


BOOL CSerialMonitor::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	return TRUE;
}


void CSerialMonitor::OnOK()
{
	// CLEAR MESSAGEs
	m_wndMessage.ResetContent();

	m_strContent = _T("");
	m_wndEdit.SetWindowTextW(m_strContent);
}


void CSerialMonitor::OnCancel()
{
}

void CSerialMonitor::OnClickAutoScroll()
{
	UpdateData();
}

void CSerialMonitor::AddMessage(LPCTSTR message)
{
#if 1
	m_wndMessage.AddString(message);

	if (m_bAutoScroll)
		m_wndMessage.SetCurSel(m_wndMessage.GetCount() - 1);
#else

	m_strContent += message;
	m_wndEdit.SetWindowTextW(m_strContent);

#endif
}