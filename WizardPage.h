#pragma once

// CWizardPage dialog
class CWizardPage : public CPropertyPage
{
	DECLARE_DYNAMIC(CWizardPage)

public:
	CWizardPage(UINT nIDTemplate, UINT nIDCaption, UINT nIDHeaderTitle, UINT nIDHeaderSubTitle);
	CWizardPage(UINT nIDTemplate, UINT nIDCaption);
	virtual ~CWizardPage();

protected:
	void SetWizardButtons(DWORD dwFlags);
	void EnableCancelButton(BOOL bEnable);
	BOOL IsCancelButtonEnabled();
	void PressButton(int nButton);

	void InflictBoldFont(UINT nIDC);

protected:
	DECLARE_MESSAGE_MAP()

private:
	CFont m_boldFont;

	bool CreateBoldFont();
};
