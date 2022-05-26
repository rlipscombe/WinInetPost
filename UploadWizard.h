#pragma once

class UploadSettings;
class UploadResults;

// CUploadWizard
class CUploadWizard : public CPropertySheet
{
	HICON m_hIcon;
	int m_deferredPressButton;

	DECLARE_DYNAMIC(CUploadWizard)

public:
	CUploadWizard(UploadSettings *pSettings, UploadResults *pResults, HBITMAP hbmWatermark, HPALETTE hpalWatermark, HBITMAP hbmHeader);
	virtual ~CUploadWizard();

	static INT_PTR DoWizard();

	virtual BOOL OnInitDialog();

protected:
	virtual void PreSubclassWindow();

	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg LRESULT OnPressButton(WPARAM wParam, LPARAM lParam);

	DECLARE_MESSAGE_MAP()
};
