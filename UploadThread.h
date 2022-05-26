#pragma once
#include "Thread.h"

class UploadSettings;
class UploadResults;

class BackgroundUploadProgress;

class UploadThread : public Thread
{
	UploadSettings *m_pSettings;
	UploadResults *m_pResults;
	BackgroundUploadProgress *m_pProgress;

public:
	UploadThread(UploadSettings *pSettings, UploadResults *pResults, BackgroundUploadProgress *pProgress)
		: m_pSettings(pSettings), m_pResults(pResults), m_pProgress(pProgress)
	{
		ASSERT(m_pSettings);
		ASSERT(m_pResults);
		ASSERT(m_pProgress);
	}

	virtual unsigned Run();
};
