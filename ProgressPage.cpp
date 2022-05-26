// ProgressPage.cpp : implementation file
//

#include "stdafx.h"
#include "ProgressPage.h"
#include "UploadThread.h"
#include "UploadProgressProxyWindow.h"
#include "FormatErrorMessage.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CProgressPage dialog
IMPLEMENT_DYNAMIC(CProgressPage, CWizardPage)

CProgressPage::CProgressPage(UploadSettings *pSettings, UploadResults *pResults)
	: CWizardPage(CProgressPage::IDD, IDS_PROGRESS_PAGE_CAPTION, IDS_PROGRESS_PAGE_TITLE, IDS_PROGRESS_PAGE_SUBTITLE),
		m_pSettings(pSettings), m_pResults(pResults), m_uploadThread(NULL), m_progressProxy(NULL), m_bCancel(false)
{
}

CProgressPage::~CProgressPage()
{
}


BOOL CProgressPage::OnInitDialog()
{
	CWizardPage::OnInitDialog();

	return TRUE;
}

void CProgressPage::DoDataExchange(CDataExchange* pDX)
{
	CWizardPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PROGRESSCTRL, m_progressCtrl);
}

BEGIN_MESSAGE_MAP(CProgressPage, CWizardPage)
END_MESSAGE_MAP()

// CProgressPage message handlers
BOOL CProgressPage::OnSetActive()
{
	if (!CWizardPage::OnSetActive())
		return FALSE;

	// At this point, the property page isn't fully created and displayed.
	// If we want to display our progress in this page, we can post
	// ourselves a message so that we can start the long-lived task
	// after the page has been displayed.
	// Alternatively, we can use a background thread, and kick it off
	// here.

	// Either way, we need to disable a few buttons until we've got
	// it all going.
	SetWizardButtons(0);
	EnableCancelButton(FALSE);
	
	// We also need to ensure that this flag is reset here.
	// It's tempting to reset it in OnBackgroundUploadBegin,
	// but due to using PostMessage for that, and a direct
	// look at the variable for this, there's a (slim)
	// possibility of a race condition.
	m_bCancel = false;

	// We're going to use a background thread.  This way, the foreground
	// thread won't block.  This leads to a more responsive app, as far
	// as the user's concerned.

	// We need to report progress from the background thread.  The normal
	// way to do that in C++ is to use an abstract Progress interface.
	// This is essentially the Observer pattern.

	// However, in Win32, there's an added wrinkle.  Because our progress
	// callback method will be updating dialog controls, and because it's 
	// being called from the wrong thread, the call to update the control
	// (which ultimately resolves to a call to SendMessage) will block
	// until the UI thread handles the message.
	// This way there be monsters (in the form of race conditions and
	// deadlocks).

	// The solution to this problem is to use a proxy.
	// Rather than pass 'this' as the progress observer, we pass another
	// object which will call PostMessage instead of SendMessage, thus
	// ensuring that the dialog controls are updated from the correct
	// thread.

	// One way to implement this is to have the proxy post messages
	// to the dialog window.  It can update the controls in the message
	// handlers.

	// The canonical Win32 solution, however, is to use a separate proxy
	// window, so that the dialog need not know about the kinds of magic going
	// on on its behalf.

	// Thus, the only piece of code that needs to know about the proxy
	// is the point that starts the background thread.

	ASSERT(!m_progressProxy);
	m_progressProxy = new CUploadProgressProxyWindow(this);

	// We can save CPU by not asking for some of the notifications.
	DWORD dwNotifyFlags = m_progressProxy->GetNotifyFlags();
	dwNotifyFlags &= ~(CUploadProgressProxyWindow::PROGRESS_SENDING_REQUEST | CUploadProgressProxyWindow::PROGRESS_REQUEST_SENT);
	m_progressProxy->SetNotifyFlags(dwNotifyFlags);

	VERIFY(m_progressProxy->Create());

	ASSERT(!m_uploadThread);
	m_uploadThread = new UploadThread(m_pSettings, m_pResults, m_progressProxy);
	m_uploadThread->Start();

	return TRUE;
}

// Even if we've disabled the Cancel button in OnSetActive, the user can still
// click on the close box (the 'X' in the caption).
BOOL CProgressPage::OnQueryCancel()
{
	// Double-check that the user can do this against the cancel button state:
	if (!IsCancelButtonEnabled())
		return FALSE;

	// The cancel button is enabled, but we don't want to let it through immediately,
	// so we'll just set the cancel flag:
	m_bCancel = true;

	// don't: return CWizardPage::OnQueryCancel();
	return FALSE;
}

// BackgroundUploadProgress: these methods are proxied, so they're called on the UI thread.
void CProgressPage::OnBackgroundUploadBegin()
{
	CString strUploadMessage;
	VERIFY(strUploadMessage.LoadString(IDS_UPLOAD_STARTING));
	SetDlgItemText(IDC_MESSAGE, strUploadMessage);

	SetDlgItemText(IDC_TIME_REMAINING, _T(""));

	// The long-lasting process is starting.  Reenable the cancel button:
	EnableCancelButton(TRUE);
}

void CProgressPage::OnBackgroundUploadComplete(HRESULT hr)
{
	CString strUploadMessage;
	VERIFY(strUploadMessage.LoadString(IDS_UPLOAD_COMPLETE));
	SetDlgItemText(IDC_MESSAGE, strUploadMessage);

	if (FAILED(hr))
		SetDlgItemText(IDC_MESSAGE, FormatErrorMessage(hr));

	SetDlgItemText(IDC_TIME_REMAINING, _T(""));

	// Wait for the background thread to finish.
	// TODO: Add comments explaining what happens to progress messages still
	// in the queue, the possibility for deadlocks, memory leaks, etc.
	// For now, it's enough to say that the call to OnComplete is the last
	// message posted, so we're fine.
	m_uploadThread->Join();
	delete m_uploadThread;
	m_uploadThread = NULL;

	// Once the background thread's stopped, we can delete the proxy window.
	m_progressProxy->DestroyWindow();
	delete m_progressProxy;
	m_progressProxy = NULL;
	
	// Sort out the buttons.
	EnableCancelButton(FALSE);	// Can't cancel something that's finished.
	SetWizardButtons(PSWIZB_NEXT);

	PressButton(PSBTN_NEXT);
}

// UploadProgress: these methods are proxied, so they're called on the UI thread.
void CProgressPage::OnResolvingName(LPCTSTR lpsz) { /* nothing */ }
void CProgressPage::OnNameResolved(LPCTSTR lpsz) { /* nothing */ }
void CProgressPage::OnConnectingToServer(LPCTSTR lpsz) { /* nothing */ }
void CProgressPage::OnConnectedToServer(LPCTSTR lpsz) { /* nothing */ }
void CProgressPage::OnSendingRequest() { /* nothing */ }
void CProgressPage::OnRequestSent(DWORD dwBytesSent) { /* nothing */ }
void CProgressPage::OnReceivingResponse() { /* nothing */ }
void CProgressPage::OnResponseReceived(DWORD dwBytesReceived) { /* nothing */ }
void CProgressPage::OnClosingConnection() { /* nothing */ }
void CProgressPage::OnConnectionClosed() { /* nothing */ }

void CProgressPage::OnFileBegin(LPCTSTR lpszPathName)
{
	CString str;
	str.FormatMessage(IDS_UPLOADING_FILE, lpszPathName);
	SetDlgItemText(IDC_MESSAGE, str);
}

void CProgressPage::OnFileProgress(LPCTSTR lpszPathName, DWORD dwFileBytesSent, DWORD dwFileBytesTotal, DWORD dwSecondsToFileCompletion, DWORD dwOverallBytesSent, DWORD dwOverallBytesTotal, DWORD dwSecondsToOverallCompletion, DWORD dwBytesPerSecond)
{
	TRACE("%s: %d/%d %d%%, %d seconds remaining\n", lpszPathName,
		dwFileBytesSent, dwFileBytesTotal, MulDiv(100, dwFileBytesSent, dwFileBytesTotal), dwSecondsToFileCompletion);

	TRACE("Total: %d/%d %d%%, %d seconds remaining; %d bytes/sec",
		dwOverallBytesSent, dwOverallBytesTotal, MulDiv(100, dwOverallBytesSent, dwOverallBytesTotal), dwSecondsToOverallCompletion,
		dwBytesPerSecond);

	m_progressCtrl.SetRange32(0, dwOverallBytesTotal);
	m_progressCtrl.SetPos(dwOverallBytesSent);

	CString strTransferRate;
	StrFormatKBSize(dwBytesPerSecond, strTransferRate.GetBuffer(_MAX_PATH), _MAX_PATH);
	strTransferRate.ReleaseBuffer();

	CString strUploadMessage;
	strUploadMessage.FormatMessage(IDS_UPLOADING_FILE_RATE, lpszPathName, LPCTSTR(strTransferRate));
	SetDlgItemText(IDC_MESSAGE, strUploadMessage);

	CString strTimeInterval;
	StrFromTimeInterval(strTimeInterval.GetBuffer(_MAX_PATH), _MAX_PATH, dwSecondsToOverallCompletion * 1000, 3);
	strTimeInterval.ReleaseBuffer();

	CString strTimeRemaining;
	strTimeRemaining.FormatMessage(IDS_TIME_REMAINING, LPCTSTR(strTimeInterval));

	SetDlgItemText(IDC_TIME_REMAINING, strTimeRemaining);
}

void CProgressPage::OnFileComplete(LPCTSTR lpszPathName, HRESULT hr)
{
}

bool CProgressPage::CheckCancel()
{
	return m_bCancel;
}
