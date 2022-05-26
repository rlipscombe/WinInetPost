#pragma once

class UploadSettings;
class UploadResults;

// CUploadWizard
class CUploadWizard : public CPropertySheet
{
	DECLARE_DYNAMIC(CUploadWizard)

public:
	CUploadWizard(UploadSettings *pSettings, UploadResults *pResults, HBITMAP hbmWatermark, HPALETTE hpalWatermark, HBITMAP hbmHeader);
	virtual ~CUploadWizard();

	static INT_PTR DoWizard();

protected:
	DECLARE_MESSAGE_MAP()
};


