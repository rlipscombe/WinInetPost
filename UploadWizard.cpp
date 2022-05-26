// UploadWizard.cpp : implementation file
//

#include "stdafx.h"
#include "UploadWizard.h"
#include "VersionInfo.h"
#include "UploadSettings.h"
#include "UploadResults.h"
#include "ApplicationProfile.h"

#include "WelcomePage.h"
#include "AddressPage.h"
#include "SelectionPage.h"
#include "ProgressPage.h"
#include "CompletePage.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CUploadWizard
IMPLEMENT_DYNAMIC(CUploadWizard, CPropertySheet)

CUploadWizard::CUploadWizard(UploadSettings *pSettings, UploadResults *pResults, HBITMAP hbmWatermark, HPALETTE hpalWatermark, HBITMAP hbmHeader)
	: CPropertySheet(IDS_UNUSED_CAPTION, NULL, 0, hbmWatermark, hpalWatermark, hbmHeader), m_deferredPressButton(-1)
{
 	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	AddPage(new CWelcomePage);
	AddPage(new CAddressPage(pSettings));
	AddPage(new CSelectionPage(pSettings));
	AddPage(new CProgressPage(pSettings, pResults));
	AddPage(new CCompletePage(pResults));

	SetWizardMode();

	if (SupportsWizard97())
		m_psh.dwFlags |= PSH_WIZARD97;
}

/* static */
INT_PTR CUploadWizard::DoWizard()
{
    HBITMAP hbmWatermark = ::LoadBitmap(AfxGetResourceHandle(), MAKEINTRESOURCE(IDB_WATERMARK));
    HPALETTE hpalWatermark = NULL;
    HBITMAP hbmHeader = ::LoadBitmap(AfxGetResourceHandle(), MAKEINTRESOURCE(IDB_HEADER));

	/* The implementation in MFC (and most of the samples) encourage using standard DDX/DDV to
	 * move values between the controls on each page and member variables in each page.
	 *
	 * Unfortunately, this leads to tight coupling between the various pages in the wizard.
	 * Following pages tend to go poking around in the member variables of preceding pages.
	 * Alternatively, the member variables could be stashed in the wizard (the CPropertySheet-derived
	 * object).  This isn't much nicer.
	 *
	 * My preferred solution is to pass each page a pointer to a "settings" object.
	 * In this way, each page is now decoupled from its neighbours.
	 * Moreover, the settings object can be passed directly to whatever's going
	 * to be doing the work.  It can also be saved and loaded (to the registry, for example),
	 * making it easier to present the user with the same settings they had last time.
	 *
	 * You might also decide to pass a const-pointer to some of the pages, thus ensuring that
	 * they can't change the settings.  You'd do this for a completion/summary page, for
	 * example.
	 */
	RegistryApplicationProfile profile(AfxGetApp()->GetAppRegistryKey());

	UploadSettings settings;
	settings.Load(&profile);

	UploadResults results;

	CUploadWizard dlg(&settings, &results, hbmWatermark, hpalWatermark, hbmHeader);
	INT_PTR nResult = dlg.DoModal();
	if (nResult == ID_WIZFINISH)
		settings.Save(&profile);

	return nResult;
}

CUploadWizard::~CUploadWizard()
{
	int pageCount = GetPageCount();
	for (int i = 0; i < pageCount; ++i)
	{
		CPropertyPage *pPage = GetPage(i);
		delete pPage;
	}
}

BEGIN_MESSAGE_MAP(CUploadWizard, CPropertySheet)
	ON_WM_SIZE()
	ON_MESSAGE(PSM_PRESSBUTTON, OnPressButton)
END_MESSAGE_MAP()

// CUploadWizard message handlers
void CUploadWizard::PreSubclassWindow()
{
	VERIFY(ModifyStyle(0, WS_MINIMIZEBOX));

	CPropertySheet::PreSubclassWindow();
}

BOOL CUploadWizard::OnInitDialog()
{
	BOOL bResult = CPropertySheet::OnInitDialog();

	SetIcon(m_hIcon, TRUE);
	SetIcon(m_hIcon, FALSE);

	m_deferredPressButton = -1;

	return bResult;
}

void CUploadWizard::OnSize(UINT nType, int cx, int cy)
{
	if (nType == SIZE_RESTORED && m_deferredPressButton != -1)
		PressButton(m_deferredPressButton);

	CPropertySheet::OnSize(nType, cx, cy);
}

LRESULT CUploadWizard::OnPressButton(WPARAM wParam, LPARAM /*lParam*/)
{
	if (IsIconic())
	{
		TRACE("Deferring button press.\n");
		m_deferredPressButton = (int)wParam;
		return 0;
	}
	else
		return Default();
}
