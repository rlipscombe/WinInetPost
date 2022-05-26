#pragma once
#include "resource.h"
#include "WizardPage.h"

class UploadSettings;

// CAddressPage dialog
class CAddressPage : public CWizardPage
{
	UploadSettings *m_pSettings;

	DECLARE_DYNAMIC(CAddressPage)

public:
	CAddressPage(UploadSettings *pSettings);
	virtual ~CAddressPage();

	virtual BOOL OnSetActive();
	virtual LRESULT OnWizardNext();

// Dialog Data
	enum { IDD = IDD_ADDRESS_PAGE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	afx_msg void OnEnChangeAddress();

	DECLARE_MESSAGE_MAP()

private:
	void EnableControls();
};
