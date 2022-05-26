#pragma once

class Thread
{
	HANDLE m_hThread;
	UINT m_idThread;

public:
	Thread()
		: m_hThread(NULL), m_idThread(0)
	{
	}

	virtual ~Thread()
	{
		CloseHandle(m_hThread);
	}

	bool Start();
	unsigned Join();

protected:
	virtual unsigned Run() = 0;

private:
	static unsigned __stdcall StaticThreadRoutine(void *pParams)
	{
		// This shows a fairly common C++/Win32 idiom.  Most Windows API functions
		// that take a callback function also allow for a caller-defined parameter.
		// This can be used to hold a this-pointer.
		Thread *pThis = static_cast<Thread *>(pParams);
		return pThis->Run();
	}
};
