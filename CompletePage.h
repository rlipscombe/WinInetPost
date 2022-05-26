#pragma once
#include "resource.h"
#include "WizardPage.h"

class UploadResults;

// CCompletePage dialog
class CCompletePage : public CWizardPage
{
	const UploadResults *m_pResults;

	DECLARE_DYNAMIC(CCompletePage)

public:
	CCompletePage(const UploadResults *pResults);
	virtual ~CCompletePage();

	virtual BOOL OnInitDialog();
	virtual BOOL OnSetActive();
	virtual LRESULT OnWizardBack();

// Dialog Data
	enum { IDD = IDD_COMPLETE_PAGE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

private:
	void SetCompletionIcon(HICON hIcon);
	void SetCompletionIcon(LPCTSTR lpszIconName);
};
