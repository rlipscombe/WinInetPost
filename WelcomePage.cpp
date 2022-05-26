// WelcomePage.cpp : implementation file
//

#include "stdafx.h"
#include "WelcomePage.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CWelcomePage dialog
IMPLEMENT_DYNAMIC(CWelcomePage, CWizardPage)

CWelcomePage::CWelcomePage()
	: CWizardPage(CWelcomePage::IDD, IDS_WELCOME_PAGE_CAPTION)
{
    m_psp.dwFlags |= PSP_HIDEHEADER;
}

CWelcomePage::~CWelcomePage()
{
}

void CWelcomePage::DoDataExchange(CDataExchange* pDX)
{
	CWizardPage::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CWelcomePage, CWizardPage)
END_MESSAGE_MAP()

// CWelcomePage message handlers
BOOL CWelcomePage::OnInitDialog()
{
	CWizardPage::OnInitDialog();

	InflictBoldFont(IDC_TITLE);

	return TRUE;
}

BOOL CWelcomePage::OnSetActive()
{
	if (!CWizardPage::OnSetActive())
		return FALSE;

	SetWizardButtons(PSWIZB_NEXT);
	EnableCancelButton(TRUE);

	return TRUE;
}
