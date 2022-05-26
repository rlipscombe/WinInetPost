#pragma once

#include "UploadProgress.h"

class Command;

// CUploadProgressProxyWindow
class CUploadProgressProxyWindow : public CWnd, public BackgroundUploadProgress
{
	BackgroundUploadProgress *m_pProgress;
	DWORD m_dwNotifyFlags;

	DECLARE_DYNAMIC(CUploadProgressProxyWindow)

public:
	CUploadProgressProxyWindow(BackgroundUploadProgress *pProgress);
	virtual ~CUploadProgressProxyWindow();

	BOOL Create();

// Notification flags: you can turn off some of the messages.  This can save CPU.
public:
	static const int PROGRESS_SENDING_REQUEST = 0x00000001;
	static const int PROGRESS_REQUEST_SENT    = 0x00000002;

	void SetNotifyFlags(DWORD dwFlags);
	DWORD GetNotifyFlags() const;

// UploadProgress
public:
	virtual void OnResolvingName(LPCTSTR lpsz);
	virtual void OnNameResolved(LPCTSTR lpsz);
	virtual void OnConnectingToServer(LPCTSTR lpsz);
	virtual void OnConnectedToServer(LPCTSTR lpsz);
	virtual void OnSendingRequest();
	virtual void OnRequestSent(DWORD dwBytesSent);
	virtual void OnReceivingResponse();
	virtual void OnResponseReceived(DWORD dwBytesReceived);
	virtual void OnClosingConnection();
	virtual void OnConnectionClosed();

	virtual void OnFileBegin(LPCTSTR lpszPathName);
	virtual void OnFileProgress(LPCTSTR lpszPathName, DWORD dwFileBytesSent, DWORD dwFileBytesTotal, DWORD dwSecondsToFileCompletion, DWORD dwOverallBytesSent, DWORD dwOverallBytesTotal, DWORD dwSecondsToOverallCompletion, DWORD dwBytesPerSecond);
	virtual void OnFileComplete(LPCTSTR lpszPathName, HRESULT hr);

	virtual bool CheckCancel();

// BackgroundUploadProgress
public:
	virtual void OnBackgroundUploadBegin();
	virtual void OnBackgroundUploadComplete(HRESULT hr);

protected:
	afx_msg LRESULT OnProgressMessage(WPARAM wParam, LPARAM lParam);

	DECLARE_MESSAGE_MAP()

private:
	void PostCommand(Command *commandObject);

	typedef void (BackgroundUploadProgress::*PROGRESS_VOID_FUNC) ();
	typedef void (BackgroundUploadProgress::*PROGRESS_STRING_FUNC) (LPCTSTR);
	typedef void (BackgroundUploadProgress::*PROGRESS_DWORD_FUNC) (DWORD);

	void PostCommand(PROGRESS_VOID_FUNC pFunc);
	void PostCommand(PROGRESS_STRING_FUNC pFunc, LPCTSTR lpsz);
	void PostCommand(PROGRESS_DWORD_FUNC pFunc, DWORD dw);
};

/** This is the canonical C++ implementation of the GoF Command pattern.
 * We need a virtual destructor, because we normally end up deleting
 * through a pointer to the base class.
 * We need a pure virtual method that'll be defined by the derived class.
 */
class Command
{
public:
	virtual ~Command() { /* nothing */ }
	virtual void Execute() = 0;
};

class UploadProgressCommand : public Command
{
protected:
	BackgroundUploadProgress *m_pProgress;

	UploadProgressCommand(BackgroundUploadProgress *pProgress)
		: m_pProgress(pProgress)
	{
	}
};

/* Normally, the inside of a method on the proxy (using the Command pattern) would look
 * like this:

	void CProxyWindow::OnSomething(int arg1, char *arg2)
	{
		class OnSomethingCommand : public UploadProgressCommand {
			int m_arg1;
			CString m_arg2;

		public:
			OnSomethingCommand(Progress *pProgress, int arg1, char *arg2)
				: m_pProgress(pProgress), m_arg1(arg1), m_arg2(arg2)
			{
			}

			virtual void Execute() { m_pProgress->OnSomething(m_arg1, m_arg2); }
		};
		Command *commandObject = new OnSomethingCommand(m_pProgress, hr);
		PostMessage(WM_PROXY_PROGRESS, 0, (LPARAM)commandObject);
	}

 * Here we see one of the slightly unwieldy things about using the command
 * pattern: each parameter needs to be specified multiple times:
 * 1. As parameters to this function.
 * 2. As a member variable of the command class.
 * 3. As constructor parameters.
 * 4. In the initialiser list.
 * 5. In the call to the constructor.
 * 6. In the actual call inside Execute.
 *
 * Moreover, each progress function needs to cook up its own command object
 * to encapsulate the arguments (and, implicitly, which function is to be called).
 *
 * We can get around this by defining a selection of Command objects
 * that take pointer-to-member-function parameters.  This saves quite a lot of typing.
 *
 * We might also be able to use some template magic to simplify this still further.
 */
class VoidCommand : public UploadProgressCommand
{
public:
	typedef void (BackgroundUploadProgress::*PROGRESS_VOID_FUNC) ();

	VoidCommand(BackgroundUploadProgress *pProgress, PROGRESS_VOID_FUNC pFunc)
		: UploadProgressCommand(pProgress), m_pFunc(pFunc)
	{
	}

	virtual void Execute()
	{
		(m_pProgress->*m_pFunc)();
	}

private:
	PROGRESS_VOID_FUNC m_pFunc;
};

class StringCommand : public UploadProgressCommand
{
public:
	typedef void (BackgroundUploadProgress::*PROGRESS_STRING_FUNC) (LPCTSTR);

	StringCommand(BackgroundUploadProgress *pProgress, PROGRESS_STRING_FUNC pFunc, LPCTSTR lpsz)
		: UploadProgressCommand(pProgress), m_pFunc(pFunc), m_str(lpsz)
	{
	}

	virtual void Execute()
	{
		(m_pProgress->*m_pFunc)(m_str);
	}

private:
	PROGRESS_STRING_FUNC m_pFunc;
	CString m_str;
};

class DwordCommand : public UploadProgressCommand
{
public:
	typedef void (BackgroundUploadProgress::*PROGRESS_DWORD_FUNC) (DWORD);

	DwordCommand(BackgroundUploadProgress *pProgress, PROGRESS_DWORD_FUNC pFunc, DWORD dw)
		: UploadProgressCommand(pProgress), m_pFunc(pFunc), m_dw(dw)
	{
	}

	virtual void Execute()
	{
		(m_pProgress->*m_pFunc)(m_dw);
	}

private:
	PROGRESS_DWORD_FUNC m_pFunc;
	DWORD m_dw;
};

