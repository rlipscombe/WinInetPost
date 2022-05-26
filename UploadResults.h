#pragma once

class UploadResults
{
	HRESULT m_hResult;
	DWORD m_dwStatusCode;

public:
	UploadResults()
		: m_hResult(S_OK), m_dwStatusCode(-1)
	{
	}

	void SetResult(HRESULT hr) { m_hResult = hr; }
	HRESULT GetResult() const { return m_hResult; }

	void SetStatusCode(DWORD dwStatusCode) { m_dwStatusCode = dwStatusCode; }
	DWORD GetStatusCode() const { return m_dwStatusCode; }

	bool GotHttpResult() const { return GetStatusCode() != -1; }
	bool IsHttpSuccess() const { return ((m_dwStatusCode / 100) == 2); }
};

#define E_USER_CANCELLED MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0x301)
