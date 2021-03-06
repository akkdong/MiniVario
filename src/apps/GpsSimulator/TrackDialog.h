#pragma once

#include "TrackWnd.h"

// CTrackDialog dialog

class CTrackDialog : public CDialogEx
{
	DECLARE_DYNAMIC(CTrackDialog)

public:
	CTrackDialog(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CTrackDialog();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_TRACK };
#endif

	void		AddPosition(double lat, double lon, double track);
	void		ResetPosition();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	CTrackWnd	m_wndTrack;

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
};
