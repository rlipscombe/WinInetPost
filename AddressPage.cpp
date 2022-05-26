// AddressPage.cpp : implementation file
//

#include "stdafx.h"
#include "AddressPage.h"
#include "UploadSettings.h"

// CAddressPage dialog
IMPLEMENT_DYNAMIC(CAddressPage, CWizardPage)

CAddressPage::CAddressPage(UploadSettings *pSettings)
	: CWizardPage(CAddressPage::IDD, IDS_ADDRESS_PAGE_CAPTION, IDS_ADDRESS_PAGE_TITLE, IDS_ADDRESS_PAGE_SUBTITLE), m_pSettings(pSettings)
{
}

CAddressPage::~CAddressPage()
{
}

void CAddressPage::DoDataExchange(CDataExchange* pDX)
{
	CWizardPage::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CAddressPage, CWizardPage)
	ON_EN_CHANGE(IDC_ADDRESS, OnEnChangeAddress)
END_MESSAGE_MAP()

// CAddressPage message handlers
BOOL CAddressPage::OnSetActive()
{
	if (!CWizardPage::OnSetActive())
		return FALSE;

	SetDlgItemText(IDC_ADDRESS, m_pSettings->GetAddress());
	EnableControls();

	return TRUE;
}

LRESULT CAddressPage::OnWizardNext()
{
	CString str;
	if (GetDlgItemText(IDC_ADDRESS, str) != 0)
	{
		if (m_pSettings->SetAddress(str))
			return CWizardPage::OnWizardNext();
	}

	AfxMessageBox(IDP_INVALID_ADDRESS, MB_OK|MB_ICONWARNING);
	return -1;
}

void CAddressPage::EnableControls()
{
	CString str;
	if (GetDlgItemText(IDC_ADDRESS, str) != 0)
		SetWizardButtons(PSWIZB_BACK|PSWIZB_NEXT);
	else
		SetWizardButtons(PSWIZB_BACK);
}

void CAddressPage::OnEnChangeAddress()
{
	EnableControls();
}
