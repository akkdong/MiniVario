// TrackDialog.cpp : implementation file
//

#include "stdafx.h"
#include "GpsSimulator.h"
#include "TrackDialog.h"
#include "afxdialogex.h"


// CTrackDialog dialog

IMPLEMENT_DYNAMIC(CTrackDialog, CDialogEx)

CTrackDialog::CTrackDialog(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_TRACK, pParent)
{

}

CTrackDialog::~CTrackDialog()
{
}

void CTrackDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_FRAME, m_wndTrack);
}


BEGIN_MESSAGE_MAP(CTrackDialog, CDialogEx)
END_MESSAGE_MAP()


// CTrackDialog message handlers


BOOL CTrackDialog::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	//
	CRect rect;

	GetClientRect(rect);
	GetDlgItem(IDC_FRAME)->MoveWindow(rect);

	return TRUE;
}


void CTrackDialog::OnOK()
{
}


void CTrackDialog::OnCancel()
{
}

void CTrackDialog::AddPosition(double lat, double lon, double track)
{
	if (::IsWindow(m_wndTrack))
	{
		m_wndTrack.AddPosition(lat, lon, track);
		m_wndTrack.InvalidateRect(NULL);
	}
}

void CTrackDialog::ResetPosition()
{
	if (::IsWindow(m_wndTrack))
	{
		m_wndTrack.Reset();
		m_wndTrack.InvalidateRect(NULL);
	}
}