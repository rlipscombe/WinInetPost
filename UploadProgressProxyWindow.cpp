// UploadProgressProxyWindow.cpp : implementation file
//

#include "stdafx.h"
#include "UploadProgressProxyWindow.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CUploadProgressProxyWindow
IMPLEMENT_DYNAMIC(CUploadProgressProxyWindow, CWnd)

CUploadProgressProxyWindow::CUploadProgressProxyWindow(BackgroundUploadProgress *pProgress)
	: m_pProgress(pProgress), m_dwNotifyFlags(0xffffffff)
{
	ASSERT(m_pProgress);
}

CUploadProgressProxyWindow::~CUploadProgressProxyWindow()
{
}

void CUploadProgressProxyWindow::SetNotifyFlags(DWORD dwFlags)
{
	m_dwNotifyFlags = dwFlags;
}

DWORD CUploadProgressProxyWindow::GetNotifyFlags() const
{
	return m_dwNotifyFlags;
}

#define WM_PROXY_PROGRESS (WM_APP + 42)

BEGIN_MESSAGE_MAP(CUploadProgressProxyWindow, CWnd)
	ON_MESSAGE(WM_PROXY_PROGRESS, OnProgressMessage)
END_MESSAGE_MAP()

BOOL CUploadProgressProxyWindow::Create()
{
	UINT nClassStyle = 0;
	HCURSOR hCursor = NULL;
	HBRUSH hbrBackground = NULL;
	HICON hIcon = NULL;

	LPCTSTR lpszClassName = AfxRegisterWndClass(nClassStyle, hCursor, hbrBackground, hIcon);
	return CWnd::CreateEx(0, lpszClassName, NULL, WS_POPUP, CRect(0,0,0,0), NULL, 0, NULL);
}

LRESULT CUploadProgressProxyWindow::OnProgressMessage(WPARAM wParam, LPARAM lParam)
{
	Command *commandObject = reinterpret_cast<Command *>(lParam);
	ASSERT(commandObject);

	commandObject->Execute();
	delete commandObject;

	return 0;
}

void CUploadProgressProxyWindow::PostCommand(Command *commandObject)
{
	PostMessage(WM_PROXY_PROGRESS, 0, (LPARAM)commandObject);
	Sleep(0);
}

void CUploadProgressProxyWindow::PostCommand(PROGRESS_VOID_FUNC pFunc)
{
	Command *commandObject = new VoidCommand(m_pProgress, pFunc);
	PostCommand(commandObject);
}

void CUploadProgressProxyWindow::PostCommand(void (BackgroundUploadProgress::*pFunc)(LPCTSTR), LPCTSTR lpsz)
{
	Command *commandObject = new StringCommand(m_pProgress, pFunc, lpsz);
	PostCommand(commandObject);
}

void CUploadProgressProxyWindow::PostCommand(PROGRESS_DWORD_FUNC pFunc, DWORD dw)
{
	Command *commandObject = new DwordCommand(m_pProgress, pFunc, dw);
	PostCommand(commandObject);
}

// CUploadProgressProxyWindow message handlers
void CUploadProgressProxyWindow::OnBackgroundUploadBegin()
{
	// This method is called from the background thread.  It's this
	// object's job to marshal the call to the foreground thread.

	// The simplest (most expedient) way to do this is to simply
	// post ourselves a message for each progress method: i.e.
	// OnBegin posts WM_ON_BEGIN, OnProgress posts WM_ON_PROGRESS
	// and so on.
	// The method handler can then call the appropriate
	// method on the stub.

	// This approach has some limitations:
	// 1. We're restricted to passing parameters in wParam and lParam.
	//    In practice, this isn't such a big deal: we can pass pointers
	//    to structs.
	//    This can lead to confusion, though: some methods don't need
	//    parameters, some need simple integers, some need more complicated
	//    things.
	//    We also need to be careful about the lifetime of the struct.
	//    This function will probably have exited by the time the message
	//    arrives, meaning that we can't pass an automatic (stack) variable.
	// 2. We need a message for each progress callback.
	//    In practice, we can use the same message and decide what's happening
	//    by looking at (e.g.) wParam.

	// Thus, at the other end of the scale (i.e. the least expedient), we
	// find the Command pattern.  It's a little more complicated than
	// strictly necessary, but it solves a number of problems for us, not least
	// in helping to ensure that all the methods can be handled in the same way.

	// As a bonus, we can hide most of the mess by using some
	// pointer-to-member-function magic.  See the header file for more info.

	PostCommand(&BackgroundUploadProgress::OnBackgroundUploadBegin);
}

void CUploadProgressProxyWindow::OnBackgroundUploadComplete(HRESULT hr)
{
	// None of the pointer-to-member-function commands will suit, and we
	// only need the one special case, so we'll use a specific type here.
	class OnBackgroundUploadCompleteCommand : public UploadProgressCommand {
		HRESULT m_hr;

	public:
		OnBackgroundUploadCompleteCommand(BackgroundUploadProgress *pProgress, HRESULT hr)
			: UploadProgressCommand(pProgress), m_hr(hr)
		{
		}

		virtual void Execute() { m_pProgress->OnBackgroundUploadComplete(m_hr); }
	};
	Command *commandObject = new OnBackgroundUploadCompleteCommand(m_pProgress, hr);
	PostCommand(commandObject);
}

void CUploadProgressProxyWindow::OnResolvingName(LPCTSTR lpsz)
{
	PostCommand(&BackgroundUploadProgress::OnResolvingName, lpsz);
}

void CUploadProgressProxyWindow::OnNameResolved(LPCTSTR lpsz)
{
	PostCommand(&BackgroundUploadProgress::OnNameResolved, lpsz);
}

void CUploadProgressProxyWindow::OnConnectingToServer(LPCTSTR lpsz)
{
	PostCommand(&BackgroundUploadProgress::OnConnectingToServer, lpsz);
}

void CUploadProgressProxyWindow::OnConnectedToServer(LPCTSTR lpsz)
{
	PostCommand(&BackgroundUploadProgress::OnConnectedToServer, lpsz);
}

void CUploadProgressProxyWindow::OnSendingRequest()
{
	if (m_dwNotifyFlags & PROGRESS_SENDING_REQUEST)
		PostCommand(&BackgroundUploadProgress::OnSendingRequest);
}

void CUploadProgressProxyWindow::OnRequestSent(DWORD dwBytesSent)
{
	if (m_dwNotifyFlags & PROGRESS_REQUEST_SENT)
		PostCommand(&BackgroundUploadProgress::OnRequestSent, dwBytesSent);
}

void CUploadProgressProxyWindow::OnReceivingResponse()
{
	PostCommand(&BackgroundUploadProgress::OnReceivingResponse);
}

void CUploadProgressProxyWindow::OnResponseReceived(DWORD dwBytesReceived)
{
	PostCommand(&BackgroundUploadProgress::OnResponseReceived, dwBytesReceived);
}

void CUploadProgressProxyWindow::OnClosingConnection()
{
	PostCommand(&BackgroundUploadProgress::OnClosingConnection);
}

void CUploadProgressProxyWindow::OnConnectionClosed()
{
	PostCommand(&BackgroundUploadProgress::OnConnectionClosed);
}

void CUploadProgressProxyWindow::OnFileBegin(LPCTSTR lpszPathName)
{
	PostCommand(&BackgroundUploadProgress::OnFileBegin, lpszPathName);
}

// None of the pointer-to-member-function commands will suit, and we
// only need the one special case, so we'll use a specific type here.
class OnFileProgressCommand : public UploadProgressCommand
{
	CString m_strPathName;
	DWORD m_dwFileBytesSent;
	DWORD m_dwFileBytesTotal;
	DWORD m_dwSecondsToFileCompletion;
	DWORD m_dwOverallBytesSent;
	DWORD m_dwOverallBytesTotal;
	DWORD m_dwSecondsToOverallCompletion;
	DWORD m_dwBytesPerSecond;

public:
	OnFileProgressCommand(BackgroundUploadProgress *pProgress, LPCTSTR lpszPathName,
							DWORD dwFileBytesSent, DWORD dwFileBytesTotal, DWORD dwSecondsToFileCompletion,
							DWORD dwOverallBytesSent, DWORD dwOverallBytesTotal, DWORD dwSecondsToOverallCompletion,
							DWORD dwBytesPerSecond)
		: UploadProgressCommand(pProgress),
			m_strPathName(lpszPathName),
			m_dwFileBytesSent(dwFileBytesSent),
			m_dwFileBytesTotal(dwFileBytesTotal),
			m_dwSecondsToFileCompletion(dwSecondsToFileCompletion),
			m_dwOverallBytesSent(dwOverallBytesSent),
			m_dwOverallBytesTotal(dwOverallBytesTotal),
			m_dwSecondsToOverallCompletion(dwSecondsToOverallCompletion),
			m_dwBytesPerSecond(dwBytesPerSecond)
	{
	}

	virtual void Execute()
	{
		m_pProgress->OnFileProgress(m_strPathName, m_dwFileBytesSent, m_dwFileBytesTotal, m_dwSecondsToFileCompletion,
			m_dwOverallBytesSent, m_dwOverallBytesTotal, m_dwSecondsToOverallCompletion, m_dwBytesPerSecond);
	}
};

void CUploadProgressProxyWindow::OnFileProgress(LPCTSTR lpszPathName, DWORD dwFileBytesSent, DWORD dwFileBytesTotal, DWORD dwSecondsToFileCompletion, DWORD dwOverallBytesSent, DWORD dwOverallBytesTotal, DWORD dwSecondsToOverallCompletion, DWORD dwBytesPerSecond)
{
	Command *commandObject = new OnFileProgressCommand(m_pProgress, lpszPathName,
														dwFileBytesSent, dwFileBytesTotal, dwSecondsToFileCompletion,
														dwOverallBytesSent, dwOverallBytesTotal, dwSecondsToOverallCompletion,
														dwBytesPerSecond);
	PostCommand(commandObject);
}

class OnFileCompleteCommand : public UploadProgressCommand
{
	CString m_strPathName;
	HRESULT m_hr;

public:
	OnFileCompleteCommand(BackgroundUploadProgress *pProgress, LPCTSTR lpszPathName, HRESULT hr)
		: UploadProgressCommand(pProgress),
			m_strPathName(lpszPathName), m_hr(hr)
	{
	}

	virtual void Execute()
	{
		m_pProgress->OnFileComplete(m_strPathName, m_hr);
	}
};

void CUploadProgressProxyWindow::OnFileComplete(LPCTSTR lpszPathName, HRESULT hr)
{
	Command *commandObject = new OnFileCompleteCommand(m_pProgress, lpszPathName, hr);
	PostCommand(commandObject);
}

bool CUploadProgressProxyWindow::CheckCancel()
{
	// There are several different ways to implement cancellation of a
	// task running on a background thread.
	//
	// Having a CheckCancel callback is traditional: cf. AbortProc
	// in the Windows API.
	//
	// It does lead to difficulties in a multithreaded scenario:
	// Race conditions, deadlocks, etc.
	//
	// This particular implementation is OK, because CProgressPage
	// only checks a boolean to decide whether to cancel or not.
	// This is safe because one thread reads the value and the
	// other thread writes the value, so it's safe.
	//
	// The only problem with this approach is that the UI class
	// needs to implement its CheckCancel function in a thread-safe
	// manner.
	//
	// An alternative implementation has a Cancel function that
	// stops the background task.  It's then the background
	// task's responsibility to ensure it's implemented in a thread-safe
	// way.
	//
	// Again, considering that my Uploader object doesn't even know that
	// it's running on a background thread, this is a bit more complicated
	// than we'd want.
	//
	// Another alternative is to have the Cancel function implemented on
	// the proxy object, which already knows how to do the marshalling.
	//
	// In the end, we'll just settle for the callback implementation.
	//
	return m_pProgress->CheckCancel();
}
