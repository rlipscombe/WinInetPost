#pragma once
#include "resource.h"
#include "WizardPage.h"

// CWelcomePage dialog

class CWelcomePage : public CWizardPage
{
	DECLARE_DYNAMIC(CWelcomePage)

public:
	CWelcomePage();
	virtual ~CWelcomePage();

	virtual BOOL OnInitDialog();
	virtual BOOL OnSetActive();

// Dialog Data
	enum { IDD = IDD_WELCOME_PAGE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
};
