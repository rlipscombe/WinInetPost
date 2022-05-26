// WizardPage.cpp : implementation file
//

#include "stdafx.h"
#include "WizardPage.h"

// CWizardPage dialog
IMPLEMENT_DYNAMIC(CWizardPage, CPropertyPage)

CWizardPage::CWizardPage(UINT nIDTemplate, UINT nIDCaption, UINT nIDHeaderTitle, UINT nIDHeaderSubTitle)
	: CPropertyPage(nIDTemplate, nIDCaption, nIDHeaderTitle, nIDHeaderSubTitle)
{
}

CWizardPage::CWizardPage(UINT nIDTemplate, UINT nIDCaption)
	: CPropertyPage(nIDTemplate, nIDCaption)
{
}

CWizardPage::~CWizardPage()
{
}

void GetMessageBoxFont(LOGFONT *lf)
{
	NONCLIENTMETRICS ncm;
	ncm.cbSize = sizeof(NONCLIENTMETRICS);
	VERIFY(SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(ncm), &ncm, 0));
	*lf = ncm.lfMessageFont;
}

bool CWizardPage::CreateBoldFont()
{
	LOGFONT lf;
	GetMessageBoxFont(&lf);

	CDC *pDC = GetDC();
	lf.lfHeight = -MulDiv(12, GetDeviceCaps(pDC->m_hDC, LOGPIXELSY), 72);
	ReleaseDC(pDC);

	lf.lfWeight = FW_BOLD;

	if (m_boldFont.CreateFontIndirect(&lf))
		return true;
	return false;
}

void CWizardPage::InflictBoldFont(UINT nIDC)
{
	if (!m_boldFont.GetSafeHandle())
		VERIFY(CreateBoldFont());

	CWnd *pWnd = GetDlgItem(nIDC);
	if (pWnd)
		pWnd->SetFont(&m_boldFont);
}

void CWizardPage::SetWizardButtons(DWORD dwFlags)
{
	CPropertySheet *pParent = static_cast<CPropertySheet *>(GetParent());
	pParent->SetWizardButtons(dwFlags);
}

void CWizardPage::EnableCancelButton(BOOL bEnable)
{
	CWnd *pCancel = GetParent()->GetDlgItem(IDCANCEL);
	if (pCancel)
		pCancel->EnableWindow(bEnable);
}

BOOL CWizardPage::IsCancelButtonEnabled()
{
	CWnd *pCancel = GetParent()->GetDlgItem(IDCANCEL);
	if (pCancel)
		return pCancel->IsWindowEnabled();

	return FALSE;
}

void CWizardPage::PressButton(int nButton)
{
	CPropertySheet *pParent = static_cast<CPropertySheet *>(GetParent());
	pParent->PressButton(nButton);
}

BEGIN_MESSAGE_MAP(CWizardPage, CPropertyPage)
END_MESSAGE_MAP()

// CWizardPage message handlers
