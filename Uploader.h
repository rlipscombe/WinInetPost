#pragma once
#include <vector>

class UploadProgress;
class UploadSettings;
class UploadResults;

class Uploader
{
	UploadProgress *m_pProgress;
	UploadSettings *m_pSettings;
	UploadResults *m_pResults;

public:
	Uploader(UploadProgress *pProgress)
		: m_pProgress(pProgress), m_pSettings(NULL), m_pResults(NULL)
	{
	}

	HRESULT DoUpload(UploadSettings *pSettings, UploadResults *pResults);

private:
	CString GenerateMultipartBoundary();
	CString GenerateContentTypeHeader(CString strBoundary);
	DWORD CalculateContentLength(CString strBoundary, DWORD *pdwOverallBytesTotal);

	CStringA GenerateFieldHeader(CString strBoundary, LPCTSTR lpszFieldName);
	DWORD CalculateContentLength(CString strBoundary, LPCTSTR lpszFieldName, LPCTSTR lpszFieldValue);
	HRESULT UploadField(HINTERNET hRequest, CString strBoundary, LPCTSTR lpszFieldName, LPCTSTR lpszFieldValue);
	CStringA GenerateFieldTrailer();
	
	DWORD CalculateContentLength(CString strBoundary, LPCTSTR lpszFieldName, LPCTSTR lpszPathName, LPCTSTR lpszContentType, DWORD *pdwOverallBytesTotal);
	CStringA GenerateFileHeader(CString strBoundary, LPCTSTR lpszFieldName, LPCTSTR lpszPathName, LPCTSTR lpszContentType);
	HRESULT UploadFile(HINTERNET hRequest, CString strBoundary, LPCTSTR lpszFieldName, LPCTSTR lpszPathName, LPCTSTR lpszContentType, DWORD *pdwOverallBytesSent, DWORD dwOverallBytesTotal);
	HRESULT UploadFileContent(HINTERNET hRequest, LPCTSTR lpszPathName, HANDLE hFile, DWORD *pdwOverallBytesSent, DWORD dwOverallBytesTotal);
	CStringA GenerateFileTrailer();

	CStringA GenerateBodyTrailer(CString strBoundary);

	HRESULT ReadStatusCode(HINTERNET hRequest, DWORD *pStatusCode);
	HRESULT ReadResponseHeaders(HINTERNET hRequest);
	HRESULT ReadResponseBody(HINTERNET hRequest);

private:
	static void CALLBACK StaticInternetStatusCallback(HINTERNET hInternet, DWORD_PTR dwContext, DWORD dwInternetStatus, LPVOID lpvStatusInformation, DWORD dwStatusInformationLength);
	void InternetStatusCallback(HINTERNET hInternet, DWORD dwInternetStatus, LPVOID lpvStatusInformation, DWORD dwStatusInformationLength);
};
