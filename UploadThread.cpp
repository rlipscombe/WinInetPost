#include "StdAfx.h"
#include "UploadThread.h"
#include "Uploader.h"
#include "UploadProgress.h"
#include "UploadSettings.h"
#include "UploadResults.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

unsigned UploadThread::Run()
{
	m_pProgress->OnBackgroundUploadBegin();

	Uploader uploader(m_pProgress);

	HRESULT hr = uploader.DoUpload(m_pSettings, m_pResults);
	m_pResults->SetResult(hr);
	m_pProgress->OnBackgroundUploadComplete(hr);

	return hr;
}
