#pragma once


// CSerialMonitor dialog

class CSerialMonitor : public CDialogEx
{
	DECLARE_DYNAMIC(CSerialMonitor)

public:
	CSerialMonitor(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CSerialMonitor();


	void AddMessage(LPCTSTR message);
	void Cleanup() { OnOK(); }

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MONITOR };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	CListBox	m_wndMessage;
	CEdit		m_wndEdit;
	BOOL		m_bAutoScroll;

	CString		m_strContent;

	DECLARE_MESSAGE_MAP()

	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();

	afx_msg void OnClickAutoScroll();
};
