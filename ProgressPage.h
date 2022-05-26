#pragma once
#include "resource.h"
#include "WizardPage.h"
#include "UploadProgress.h"

class UploadSettings;
class UploadResults;
class UploadThread;
class CUploadProgressProxyWindow;

// CProgressPage dialog
class CProgressPage : public CWizardPage, public BackgroundUploadProgress
{
	UploadSettings *m_pSettings;
	UploadResults *m_pResults;

	CProgressCtrl m_progressCtrl;

	UploadThread *m_uploadThread;
	CUploadProgressProxyWindow *m_progressProxy;

	bool m_bCancel;

	DECLARE_DYNAMIC(CProgressPage)

public:
	CProgressPage(UploadSettings *pSettings, UploadResults *pResults);
	virtual ~CProgressPage();

	virtual BOOL OnInitDialog();
	virtual BOOL OnSetActive();
	virtual BOOL OnQueryCancel();

// Dialog Data
	enum { IDD = IDD_PROGRESS_PAGE };

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
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
};
