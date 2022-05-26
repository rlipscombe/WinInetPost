#include "StdAfx.h"
#include "Thread.h"
#include <process.h>	// for _beginthreadex

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

bool Thread::Start()
{
	ASSERT(!m_hThread);
	m_hThread = (HANDLE)_beginthreadex(NULL, 0, StaticThreadRoutine, this, CREATE_SUSPENDED, &m_idThread);
	if (!m_hThread)
		return false;

	ResumeThread(m_hThread);
	return true;
}

unsigned Thread::Join()
{
	WaitForSingleObject(m_hThread, INFINITE);
	
	DWORD result;
	VERIFY(GetExitCodeThread(m_hThread, &result));

	return result;
}
