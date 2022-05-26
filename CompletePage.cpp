// CompletePage.cpp : implementation file
//

#include "stdafx.h"
#include "CompletePage.h"
#include "UploadResults.h"
#include "FormatErrorMessage.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CCompletePage dialog
IMPLEMENT_DYNAMIC(CCompletePage, CWizardPage)

CCompletePage::CCompletePage(const UploadResults *pResults)
	: CWizardPage(CCompletePage::IDD, IDS_COMPLETE_PAGE_CAPTION), m_pResults(pResults)
{
    m_psp.dwFlags |= PSP_HIDEHEADER;
}

CCompletePage::~CCompletePage()
{
}

void CCompletePage::DoDataExchange(CDataExchange* pDX)
{
	CWizardPage::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CCompletePage, CWizardPage)
END_MESSAGE_MAP()

// CCompletePage message handlers
BOOL CCompletePage::OnInitDialog()
{
	CWizardPage::OnInitDialog();

	InflictBoldFont(IDC_TITLE);

	return TRUE;
}

void CCompletePage::SetCompletionIcon(HICON hIcon)
{
	CStatic *pIcon = static_cast<CStatic *>(GetDlgItem(IDC_COMPLETION_ICON));
	if (pIcon)
		pIcon->SetIcon(hIcon);
}

void CCompletePage::SetCompletionIcon(LPCTSTR lpszIconName)
{
	HICON hIcon = AfxGetApp()->LoadStandardIcon(lpszIconName);
	SetCompletionIcon(hIcon);
}

BOOL CCompletePage::OnSetActive()
{
	if (!CWizardPage::OnSetActive())
		return FALSE;
	
	// There are 5 cases to deal with here:
	// 1. The HTTP upload completed successfully.
	// 2. The upload failed, but it wasn't our fault -- e.g. a 500 response from the server.
	// 3. The user cancelled it.
	// 4. The upload failed, due to some other condition.
	// 5. The slightlier nastier one: With PHP, we get a 200 success code, but the file was larger
	//    than the maximum upload size.  We don't get any reliable indication that there
	//    was a problem in this case.
	if (m_pResults->GotHttpResult())
	{
		if (m_pResults->IsHttpSuccess())
		{
			CString title;
			VERIFY(title.LoadString(IDS_SUCCESSFUL_UPLOAD_TITLE));
			SetDlgItemText(IDC_TITLE, title);

			SetCompletionIcon(MAKEINTRESOURCE(IDI_INFORMATION));

			CString message;
			VERIFY(message.LoadString(IDS_SUCCESSFUL_UPLOAD_MESSAGE));
			SetDlgItemText(IDC_MESSAGE, message);
		}
		else 
		{
			CString title;
			title.LoadString(IDS_FAILED_UPLOAD_TITLE);
			SetDlgItemText(IDC_TITLE, title);

			SetCompletionIcon(MAKEINTRESOURCE(IDI_ERROR));

			CString message;
			message.FormatMessage(IDS_HTTP_FAILED_UPLOAD_MESSAGE, m_pResults->GetStatusCode());
			SetDlgItemText(IDC_MESSAGE, message);
		}
	}
	else
	{
		if (m_pResults->GetResult() == E_USER_CANCELLED)
		{
			CString title;
			title.LoadString(IDS_CANCELLED_UPLOAD_TITLE);
			SetDlgItemText(IDC_TITLE, title);

			SetCompletionIcon(MAKEINTRESOURCE(IDI_WARNING));

			CString message;
			VERIFY(message.LoadString(IDS_CANCELLED_UPLOAD_MESSAGE));
			SetDlgItemText(IDC_MESSAGE, message);
		}
		else
		{
			CString title;
			title.LoadString(IDS_FAILED_UPLOAD_TITLE);
			SetDlgItemText(IDC_TITLE, title);

			SetCompletionIcon(MAKEINTRESOURCE(IDI_ERROR));

			CString message;
			message.FormatMessage(IDS_FAILED_UPLOAD_MESSAGE, (LPCTSTR)FormatErrorMessage(m_pResults->GetResult()));
			SetDlgItemText(IDC_MESSAGE, message);
		}
	}

	SetWizardButtons(PSWIZB_BACK|PSWIZB_FINISH);

	return TRUE;
}

LRESULT CCompletePage::OnWizardBack()
{
	return IDD_WELCOME_PAGE;
//	return CWizardPage::OnWizardBack();
}
