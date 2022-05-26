// WinInetPost.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "WinInetPost.h"
#include "UploadWizard.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CWinInetPostApp
BEGIN_MESSAGE_MAP(CWinInetPostApp, CWinApp)
//	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

// CWinInetPostApp construction
CWinInetPostApp::CWinInetPostApp()
{
}

// The one and only CWinInetPostApp object
CWinInetPostApp theApp;

// CWinInetPostApp initialization
BOOL CWinInetPostApp::InitInstance()
{
	InitCommonControls();

	CWinApp::InitInstance();

	SetRegistryKey(_T("differentpla.net"));

	INT_PTR nResponse = CUploadWizard::DoWizard();
	return FALSE;
}
