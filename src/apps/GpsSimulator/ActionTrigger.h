// ActionTrigger.h
//

#pragma once


#define WM_ACTION_TRIGGER				(WM_USER+100)

/////////////////////////////////////////////////////////////////////////////////////////////
//

class ActionTrigger
{
public:
	ActionTrigger();
	~ActionTrigger();

	BOOL			Create(CWnd * pNotifyWnd, UINT nNotifyMsg = WM_ACTION_TRIGGER);
	void			Destroy();

	void			SetPeriod(UINT nPeriod) { m_nPeriod = nPeriod; }

protected:
	static UINT		ThreadProc(LPVOID lpParam);
	UINT			ThreadProc();

private:
	CWinThread *	m_pThread;
	BOOL			m_bRun;

	CWnd *			m_pNotifyWnd;
	UINT			m_nNotifyMsg;
	UINT			m_nPeriod;	// period in milli-seconds
};
