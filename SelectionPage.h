#pragma once
#include "resource.h"
#include "WizardPage.h"

class UploadSettings;

// CSelectionPage dialog
class CSelectionPage : public CWizardPage
{
	UploadSettings *m_pSettings;
	CListCtrl m_listCtrl;

	DECLARE_DYNAMIC(CSelectionPage)

public:
	CSelectionPage(UploadSettings *pSettings);
	virtual ~CSelectionPage();

	virtual BOOL OnInitDialog();
	virtual BOOL OnSetActive();
	virtual LRESULT OnWizardNext();

// Dialog Data
	enum { IDD = IDD_SELECTION_PAGE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	afx_msg void OnDestroy();
	afx_msg void OnLvnInsertitemListctrl(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLvnDeleteitemListctrl(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLvnItemchangedListctrl(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedAddFile();
	afx_msg void OnBnClickedAddField();
	afx_msg void OnBnClickedDelete();

	DECLARE_MESSAGE_MAP()

private:
	void InsertColumns();
	void EnableControls();

	CString GenerateUnusedFieldName();

	void AddListItem(const CString &fieldName, const CString &fieldValue, int nImage, BOOL bIsFile);
	BOOL GetListItem(int nItem, CString *pFieldName, CString *pFieldValue, BOOL *pbIsFile);

	static const int IMAGE_FIELD = 0;
	static const int IMAGE_FILE = 1;

	static const int MAX_FIELD_INDEX = 100;
};
