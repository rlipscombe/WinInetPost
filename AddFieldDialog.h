#pragma once
#include "resource.h"

// CAddFieldDialog dialog

class CAddFieldDialog : public CDialog
{
	CString m_strFieldName;
	CString m_strFieldValue;

	DECLARE_DYNAMIC(CAddFieldDialog)

public:
	CAddFieldDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~CAddFieldDialog();

	CString GetFieldName() const { return m_strFieldName; }
	CString GetFieldValue() const { return m_strFieldValue; }

// Dialog Data
	enum { IDD = IDD_ADD_FIELD };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
};
