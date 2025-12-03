// ActionTrigger.cpp
//

#include "stdafx.h"
#include "ActionTrigger.h"

/////////////////////////////////////////////////////////////////////////////////////////////
//

ActionTrigger::ActionTrigger() 
	: m_pThread(NULL)
	, m_pNotifyWnd(NULL)
	, m_nNotifyMsg(0)
	, m_nPeriod(1000)
	, m_bRun(FALSE)
{
}

ActionTrigger::~ActionTrigger()
{
	Destroy();
}

BOOL ActionTrigger::Create(CWnd * pNotifyWnd, UINT nNotifyMsg)
{
	if (m_pThread)
		return FALSE;

	m_pThread = AfxBeginThread(ActionTrigger::ThreadProc, (LPVOID)this, THREAD_PRIORITY_NORMAL, 0, CREATE_SUSPENDED, NULL);
	m_bRun = TRUE;
	m_pNotifyWnd = pNotifyWnd;
	m_nNotifyMsg = nNotifyMsg;

	m_pThread->ResumeThread();

	return TRUE;
}

void ActionTrigger::Destroy()
{
	if (m_pThread)
	{
		m_bRun = FALSE;

		::WaitForSingleObject(m_pThread->m_hThread, INFINITE);
		m_pThread = NULL;
	}
}

UINT ActionTrigger::ThreadProc(LPVOID lpParam)
{
	return ((ActionTrigger *)lpParam)->ThreadProc();
}

UINT ActionTrigger::ThreadProc()
{
	DWORD dwCount = 0;

	while (m_bRun)
	{
		m_pNotifyWnd->PostMessageW(m_nNotifyMsg, 0, dwCount++);

		Sleep(m_nPeriod);
	}

	return 0;
}
