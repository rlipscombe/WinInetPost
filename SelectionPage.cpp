// SelectionPage.cpp : implementation file
//

#include "stdafx.h"
#include "SelectionPage.h"
#include "MultiFileDialog.h"
#include "UploadSettings.h"
#include "AddFieldDialog.h"

// CSelectionPage dialog
IMPLEMENT_DYNAMIC(CSelectionPage, CWizardPage)

CSelectionPage::CSelectionPage(UploadSettings *pSettings)
	: CWizardPage(CSelectionPage::IDD, IDS_SELECTION_PAGE_CAPTION, IDS_SELECTION_PAGE_TITLE, IDS_SELECTION_PAGE_SUBTITLE), m_pSettings(pSettings)
{
}

CSelectionPage::~CSelectionPage()
{
}

void CSelectionPage::DoDataExchange(CDataExchange* pDX)
{
	CWizardPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LISTCTRL, m_listCtrl);
}

BEGIN_MESSAGE_MAP(CSelectionPage, CWizardPage)
	ON_NOTIFY(LVN_INSERTITEM, IDC_LISTCTRL, OnLvnInsertitemListctrl)
	ON_NOTIFY(LVN_DELETEITEM, IDC_LISTCTRL, OnLvnDeleteitemListctrl)
	ON_BN_CLICKED(IDC_ADD_FILE, OnBnClickedAddFile)
	ON_BN_CLICKED(IDC_DELETE, OnBnClickedDelete)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LISTCTRL, OnLvnItemchangedListctrl)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_ADD_FIELD, OnBnClickedAddField)
END_MESSAGE_MAP()

// CSelectionPage message handlers
BOOL CSelectionPage::OnInitDialog()
{
	CWizardPage::OnInitDialog();

	InsertColumns();

	CImageList *pImageList = new CImageList;
	VERIFY(pImageList->Create(IDB_LIST_IMAGES, 16, 1, RGB(255,0,255)));
	m_listCtrl.SetImageList(pImageList, LVSIL_SMALL);

	return TRUE;
}

void CSelectionPage::InsertColumns()
{
	CRect rect;
	m_listCtrl.GetWindowRect(&rect);

	int totalColumnWidth = rect.Width() - 15;
	int fieldNameColumnWidth = totalColumnWidth / 4;
	int fieldValueColumnWidth = totalColumnWidth - fieldNameColumnWidth;

	CString fieldNameHeading;
	VERIFY(fieldNameHeading.LoadString(IDS_FIELDNAME_HEADING));
	m_listCtrl.InsertColumn(0, fieldNameHeading, LVCFMT_LEFT, fieldNameColumnWidth, 0);

	CString fieldValueHeading;
	VERIFY(fieldValueHeading.LoadString(IDS_FIELDVALUE_HEADING));
	m_listCtrl.InsertColumn(1, fieldValueHeading, LVCFMT_LEFT, fieldValueColumnWidth, 1);
}

void CSelectionPage::OnDestroy()
{
	delete m_listCtrl.GetImageList(LVSIL_SMALL);

	CWizardPage::OnDestroy();
}

BOOL CSelectionPage::OnSetActive()
{
	if (!CWizardPage::OnSetActive())
		return FALSE;

	VERIFY(m_listCtrl.DeleteAllItems());

	FileInfoCollection files;
	m_pSettings->GetFiles(&files);

	for (FileInfoCollection::const_iterator i = files.begin(); i != files.end(); ++i)
		AddListItem(i->GetFieldName(), i->GetPathName(), IMAGE_FILE, TRUE);

	FieldInfoCollection fields;
	m_pSettings->GetFields(&fields);

	for (FieldInfoCollection::const_iterator i = fields.begin(); i != fields.end(); ++i)
		AddListItem(i->GetFieldName(), i->GetFieldValue(), IMAGE_FIELD, FALSE);

	EnableControls();

	return TRUE;
}

BOOL CSelectionPage::GetListItem(int nItem, CString *pFieldName, CString *pFieldValue, BOOL *pbIsFile)
{
	LVITEM item;
	item.mask = LVIF_TEXT;
	item.iItem = nItem;
	item.iSubItem = 0;
	item.cchTextMax = _MAX_PATH;
	item.pszText = pFieldName->GetBuffer(_MAX_PATH);

	BOOL bResult = m_listCtrl.GetItem(&item);
	pFieldName->ReleaseBuffer();
	if (!bResult)
		return FALSE;

	item.iSubItem = 1;
	item.cchTextMax = _MAX_PATH;
	item.pszText = pFieldValue->GetBuffer(_MAX_PATH);

	bResult = m_listCtrl.GetItem(&item);
	pFieldValue->ReleaseBuffer();
	if (!bResult)
		return FALSE;

	*pbIsFile = m_listCtrl.GetItemData(nItem) ? TRUE : FALSE;
	return TRUE;
}

LRESULT CSelectionPage::OnWizardNext()
{
	if (m_listCtrl.GetItemCount() == 0)
	{
		AfxMessageBox(IDP_MUST_ADD_FIELDS_OR_FILES, MB_OK | MB_ICONWARNING);
		return -1;	// Stay on this page.
	}

	m_pSettings->ClearFiles();
	m_pSettings->ClearFields();

	for (int nItem = m_listCtrl.GetNextItem(-1, LVNI_ALL); nItem != -1; nItem = m_listCtrl.GetNextItem(nItem, LVNI_ALL))
	{
		CString fieldName;
		CString fieldValue;
		BOOL bIsFile;

		if (GetListItem(nItem, &fieldName, &fieldValue, &bIsFile))
		{
			if (bIsFile)
				m_pSettings->AddFile(fieldName, fieldValue, _T("application/octet-stream"));
			else
				m_pSettings->AddField(fieldName, fieldValue);
		}
	}

	return CWizardPage::OnWizardNext();
}

void CSelectionPage::EnableControls()
{
	if (m_listCtrl.GetItemCount())
		SetWizardButtons(PSWIZB_BACK|PSWIZB_NEXT);
	else
		SetWizardButtons(PSWIZB_BACK);

	GetDlgItem(IDC_DELETE)->EnableWindow(m_listCtrl.GetSelectedCount() != 0);
	EnableCancelButton(TRUE);
}

void CSelectionPage::OnLvnInsertitemListctrl(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);

	EnableControls();

	*pResult = 0;
}

void CSelectionPage::OnLvnDeleteitemListctrl(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);

	EnableControls();

	*pResult = 0;
}

void CSelectionPage::OnLvnItemchangedListctrl(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);

	EnableControls();

	*pResult = 0;
}

CString CSelectionPage::GenerateUnusedFieldName()
{
	// Come up with a field name to use:
	CString fieldName;
	for (int i = 0; i < MAX_FIELD_INDEX; ++i)
	{
		fieldName.FormatMessage(IDS_FIELDNAME_FORMAT_FILE, i);

		LVFINDINFO findInfo;
		findInfo.flags = LVFI_STRING;
		findInfo.psz = fieldName;

		int nItem = m_listCtrl.FindItem(&findInfo);
		if (nItem == -1)
			break;
	}

	return fieldName;
}

void CSelectionPage::AddListItem(const CString &fieldName, const CString &fieldValue, int nImage, BOOL bIsFile)
{
	int nItem = m_listCtrl.InsertItem(m_listCtrl.GetItemCount(), fieldName, nImage);
	ASSERT(nItem != -1);
	VERIFY(m_listCtrl.SetItem(nItem, 1, LVIF_TEXT, fieldValue, 0, 0, 0, 0));
	VERIFY(m_listCtrl.SetItemData(nItem, bIsFile));
}

void CSelectionPage::OnBnClickedAddFile()
{
	CMultiFileDialog dlg;
	if (dlg.DoModal() == IDOK)
	{
		for (POSITION pos = dlg.GetStartPosition(); pos; )
		{
			CString fieldName = GenerateUnusedFieldName();
			CString pathName = dlg.GetNextPathName(pos);

			AddListItem(fieldName, pathName, IMAGE_FILE, TRUE);
		}
	}

	EnableControls();
}

void CSelectionPage::OnBnClickedDelete()
{
	for (int nItem = 0; nItem < m_listCtrl.GetItemCount(); )
	{
		if (m_listCtrl.GetItemState(nItem, LVIS_SELECTED) == LVIS_SELECTED)
			m_listCtrl.DeleteItem(nItem);
		else
			++nItem;
	}

	EnableControls();
}

void CSelectionPage::OnBnClickedAddField()
{
	CAddFieldDialog dlg;
	INT_PTR result = dlg.DoModal();
	if (result == IDOK)
	{
		CString fieldName = dlg.GetFieldName();
		CString fieldValue = dlg.GetFieldValue();

		AddListItem(fieldName, fieldValue, IMAGE_FIELD, FALSE);
	}
}
