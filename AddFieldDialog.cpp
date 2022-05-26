// AddFieldDialog.cpp : implementation file
//

#include "stdafx.h"
#include "AddFieldDialog.h"


// CAddFieldDialog dialog
IMPLEMENT_DYNAMIC(CAddFieldDialog, CDialog)

CAddFieldDialog::CAddFieldDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CAddFieldDialog::IDD, pParent)
	, m_strFieldName(_T(""))
	, m_strFieldValue(_T(""))
{
}

CAddFieldDialog::~CAddFieldDialog()
{
}

void CAddFieldDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_FIELD_NAME, m_strFieldName);
	DDX_Text(pDX, IDC_FIELD_VALUE, m_strFieldValue);
}


BEGIN_MESSAGE_MAP(CAddFieldDialog, CDialog)
END_MESSAGE_MAP()


// CAddFieldDialog message handlers
