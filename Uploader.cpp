#include "StdAfx.h"
#include "Uploader.h"
#include "UploadProgress.h"
#include "UploadSettings.h"
#include "UploadResults.h"

CString Uploader::GenerateMultipartBoundary()
{
	CString multipartBoundary = _T("---------------------------");
	ASSERT(multipartBoundary.GetLength() == 27);

	// We need 12 hex digits.
	int r0 = rand() & 0xffff;
	int r1 = rand() & 0xffff;
	int r2 = rand() & 0xffff;
	multipartBoundary.AppendFormat(_T("%04X%04X%04X"), r0, r1, r2);

	return multipartBoundary;
}

CString Uploader::GenerateContentTypeHeader(CString strBoundary)
{
	CString contentTypeHeader;
	contentTypeHeader.Format(_T("Content-Type: multipart/form-data; boundary=%s"), LPCTSTR(strBoundary));

	return contentTypeHeader;
}

CStringA Uploader::GenerateFileHeader(CString strBoundary, LPCTSTR lpszFieldName, LPCTSTR lpszPathName, LPCTSTR lpszContentType)
{
	// Because we'll be sending this data in the raw (using InternetWriteFile), and the server is expecting
	// narrow strings (for the sake of argument, we'll assume straight ASCII), we need to use T2A to convert
	// the strings as relevant.  This also requires some care with format strings.
	USES_CONVERSION;

	CStringA boundary = T2CA(strBoundary);
	CStringA fieldName = T2CA(lpszFieldName);
	CStringA pathName = T2CA(lpszPathName);
	CStringA contentType = T2CA(lpszContentType);

	CStringA contentDispositionHeader;
	contentDispositionHeader.Format("Content-Disposition: form-data; name=\"%hs\"; filename=\"%hs\"", LPCSTR(fieldName), LPCSTR(pathName));

	CStringA contentTypeHeader;
	contentTypeHeader.Format("Content-Type: %hs", LPCSTR(contentType));

	CStringA fileHeader;
	fileHeader.Format("--%hs\r\n", LPCSTR(boundary));
	fileHeader.Append(contentDispositionHeader + "\r\n");
	fileHeader.Append(contentTypeHeader + "\r\n");
	fileHeader.Append("\r\n");

	return fileHeader;
}

CStringA Uploader::GenerateFileTrailer()
{
	return "\r\n";
}

CStringA Uploader::GenerateBodyTrailer(CString strBoundary)
{
	CStringA bodyTrailer;
	bodyTrailer.Format("--%s--\r\n", strBoundary);
	return bodyTrailer;
}

CStringA Uploader::GenerateFieldHeader(CString strBoundary, LPCTSTR lpszFieldName)
{
	USES_CONVERSION;

	CStringA boundary = T2CA(strBoundary);
	CStringA fieldName = T2CA(lpszFieldName);

	CStringA contentDispositionHeader;
	contentDispositionHeader.Format("Content-Disposition: form-data; name=\"%hs\"", LPCSTR(fieldName));

	CStringA fieldHeader;
	fieldHeader.Format("--%hs\r\n", LPCSTR(boundary));
	fieldHeader.Append(contentDispositionHeader + "\r\n");
	fieldHeader.Append("\r\n");

	return fieldHeader;
}

CStringA Uploader::GenerateFieldTrailer()
{
	return "\r\n";
}

DWORD Uploader::CalculateContentLength(CString strBoundary, LPCTSTR lpszFieldName, LPCTSTR lpszFieldValue)
{
	USES_CONVERSION;

	CStringA fieldHeader = GenerateFieldHeader(strBoundary, lpszFieldName);
	CStringA fieldTrailer = GenerateFieldTrailer();

	CStringA fieldValue = T2CA(lpszFieldValue);

	DWORD contentLength = fieldHeader.GetLength() + fieldValue.GetLength() + fieldTrailer.GetLength();
	return contentLength;
}

DWORD Uploader::CalculateContentLength(CString strBoundary, LPCTSTR lpszFieldName, LPCTSTR lpszPathName, LPCTSTR lpszContentType, DWORD *pdwOverallBytesTotal)
{
	CStringA fileHeader = GenerateFileHeader(strBoundary, lpszFieldName, lpszPathName, lpszContentType);

	HANDLE hFile = CreateFile(lpszPathName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	DWORD fileLength = GetFileSize(hFile, NULL);
	CloseHandle(hFile);

	*pdwOverallBytesTotal += fileLength;

	CStringA fileTrailer = GenerateFileTrailer();

	DWORD contentLength = fileHeader.GetLength() + fileLength + fileTrailer.GetLength();
	return contentLength;
}

DWORD Uploader::CalculateContentLength(CString strBoundary, DWORD *pdwOverallBytesTotal)
{
	DWORD contentLength = 0;
	*pdwOverallBytesTotal = 0;

	FieldInfoCollection fields;
	m_pSettings->GetFields(&fields);

	for (FieldInfoCollection::const_iterator i = fields.begin(); i != fields.end(); ++i)
		contentLength += CalculateContentLength(strBoundary, i->GetFieldName(), i->GetFieldValue());

	FileInfoCollection files;
	m_pSettings->GetFiles(&files);

	for (FileInfoCollection::const_iterator i = files.begin(); i != files.end(); ++i)
		contentLength += CalculateContentLength(strBoundary, i->GetFieldName(), i->GetPathName(), i->GetContentType(), pdwOverallBytesTotal);

	CStringA bodyTrailer = GenerateBodyTrailer(strBoundary);
	contentLength += bodyTrailer.GetLength();

	return contentLength;
}

HRESULT Uploader::DoUpload(UploadSettings *pSettings, UploadResults *pResults)
{
	m_pSettings = pSettings;
	m_pResults = pResults;

	LPCTSTR lpszAgent = _T("WinInetPost/0.2");
	DWORD dwOpenInternetFlags = 0;//INTERNET_FLAG_ASYNC;
	HINTERNET hInternet = InternetOpen(lpszAgent, INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, dwOpenInternetFlags);
	if (!hInternet)
	{
		TRACE("InternetOpen failed, error = %d (0x%x)\n", GetLastError(), GetLastError());
		return HRESULT_FROM_WIN32(GetLastError());
	}

	InternetSetStatusCallback(hInternet, StaticInternetStatusCallback);

	INTERNET_PORT nServerPort = INTERNET_DEFAULT_HTTP_PORT;
	LPCTSTR lpszUserName = NULL;
	LPCTSTR lpszPassword = NULL;
	DWORD dwConnectFlags = 0;
	DWORD_PTR dwConnectContext = reinterpret_cast<DWORD_PTR>(this);
	HINTERNET hConnect = InternetConnect(hInternet, m_pSettings->GetHostName(), nServerPort,
											lpszUserName, lpszPassword,
											INTERNET_SERVICE_HTTP,
											dwConnectFlags, dwConnectContext);
	if (!hConnect)
	{
		InternetCloseHandle(hInternet);
		TRACE("InternetConnect failed, error = %d (0x%x)\n", GetLastError(), GetLastError());
		return HRESULT_FROM_WIN32(GetLastError());
	}

	LPCTSTR lpszVerb = _T("POST");
	LPCTSTR lpszVersion = NULL;			// Use default.
	LPCTSTR lpszReferrer = NULL;		// No referrer.
	LPCTSTR *lplpszAcceptTypes = NULL;	// Whatever the server wants to give us.
	DWORD dwOpenRequestFlags = INTERNET_FLAG_IGNORE_REDIRECT_TO_HTTP |
								INTERNET_FLAG_IGNORE_REDIRECT_TO_HTTPS | 
								INTERNET_FLAG_KEEP_CONNECTION |
								INTERNET_FLAG_NO_AUTH |
								INTERNET_FLAG_NO_AUTO_REDIRECT |
								INTERNET_FLAG_NO_COOKIES |
								INTERNET_FLAG_NO_UI |
								INTERNET_FLAG_RELOAD;
	DWORD_PTR dwOpenRequestContext = reinterpret_cast<DWORD_PTR>(this);
	HINTERNET hRequest = HttpOpenRequest(hConnect, lpszVerb, m_pSettings->GetUrlPath(), lpszVersion,
											lpszReferrer, lplpszAcceptTypes,
											dwOpenRequestFlags, dwOpenRequestContext);
	if (!hRequest)
	{
		InternetCloseHandle(hConnect);
		InternetCloseHandle(hInternet);
		TRACE("HttpOpenRequest failed, error = %d (0x%x)\n", GetLastError(), GetLastError());
		return HRESULT_FROM_WIN32(GetLastError());
	}

	CString strBoundary = GenerateMultipartBoundary();
	CString contentTypeHeader = GenerateContentTypeHeader(strBoundary);
	BOOL bResult = HttpAddRequestHeaders(hRequest, contentTypeHeader, -1, HTTP_ADDREQ_FLAG_ADD);
	if (!bResult)
	{
		InternetCloseHandle(hConnect);
		InternetCloseHandle(hInternet);
		TRACE("HttpAddRequestHeaders failed, error = %d (0x%x)\n", GetLastError(), GetLastError());
		return HRESULT_FROM_WIN32(GetLastError());
	}

	DWORD dwOverallBytesTotal = 0;
	DWORD contentLength = CalculateContentLength(strBoundary, &dwOverallBytesTotal);

	INTERNET_BUFFERS buffersIn;
	memset(&buffersIn, 0, sizeof(INTERNET_BUFFERS));
	buffersIn.dwStructSize = sizeof(INTERNET_BUFFERS);
	buffersIn.dwBufferTotal = contentLength;

	bResult = HttpSendRequestEx(hRequest, &buffersIn, NULL, HSR_INITIATE, reinterpret_cast<DWORD_PTR>(this));
	if (!bResult)
	{
		InternetCloseHandle(hConnect);
		InternetCloseHandle(hInternet);
		TRACE("HttpSendRequestEx failed, error = %d (0x%x)\n", GetLastError(), GetLastError());
		return HRESULT_FROM_WIN32(GetLastError());
	}

	FieldInfoCollection fields;
	m_pSettings->GetFields(&fields);

	for (FieldInfoCollection::const_iterator i = fields.begin(); i != fields.end(); ++i)
	{
		HRESULT hr = UploadField(hRequest, strBoundary, i->GetFieldName(), i->GetFieldValue());
		if (FAILED(hr))
		{
			InternetCloseHandle(hConnect);
			InternetCloseHandle(hInternet);
			return hr;
		}
	}

	FileInfoCollection files;
	m_pSettings->GetFiles(&files);

	DWORD dwOverallBytesSent = 0;
	for (FileInfoCollection::const_iterator i = files.begin(); i != files.end(); ++i)
	{
		HRESULT hr = UploadFile(hRequest, strBoundary, i->GetFieldName(), i->GetPathName(), i->GetContentType(), &dwOverallBytesSent, dwOverallBytesTotal);
		if (FAILED(hr))
		{
			InternetCloseHandle(hConnect);
			InternetCloseHandle(hInternet);
			return hr;
		}
	}

	// After the last file:
	CStringA bodyTrailer = GenerateBodyTrailer(strBoundary);
	DWORD dwBytesWritten;
	bResult = InternetWriteFile(hRequest, bodyTrailer, bodyTrailer.GetLength(), &dwBytesWritten);
	if (!bResult)
	{
		InternetCloseHandle(hConnect);
		InternetCloseHandle(hInternet);
		TRACE("InternetWriteFile failed, error = %d (0x%x)\n", GetLastError(), GetLastError());
		return HRESULT_FROM_WIN32(GetLastError());
	}

	bResult = HttpEndRequest(hRequest, NULL, 0, reinterpret_cast<DWORD_PTR>(this));
	if (!bResult)
	{
		InternetCloseHandle(hConnect);
		InternetCloseHandle(hInternet);
		TRACE("HttpEndRequest failed, error = %d (0x%x)\n", GetLastError(), GetLastError());
		// TODO: return S_OK anyway?  It's probably too late to do anything about it now.
		return HRESULT_FROM_WIN32(GetLastError());
	}

	DWORD dwStatusCode;
	ReadStatusCode(hRequest, &dwStatusCode);
	m_pResults->SetStatusCode(dwStatusCode);

	ReadResponseHeaders(hRequest);
	ReadResponseBody(hRequest);

	InternetCloseHandle(hRequest);
	InternetCloseHandle(hConnect);
	InternetCloseHandle(hInternet);

	return S_OK;
}

HRESULT Uploader::ReadStatusCode(HINTERNET hRequest, DWORD *pStatusCode)
{
	DWORD dwStatusCodeSize = sizeof(DWORD);
	DWORD dwStatusCode;
	if (!HttpQueryInfo(hRequest, HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER, &dwStatusCode, &dwStatusCodeSize, NULL))
		return HRESULT_FROM_WIN32(GetLastError());

	*pStatusCode = dwStatusCode;
	return S_OK;
}

HRESULT Uploader::ReadResponseHeaders(HINTERNET hRequest)
{
	DWORD dwInfoLevel = HTTP_QUERY_RAW_HEADERS_CRLF;
	DWORD dwInfoBufferLength = 10;
	BYTE *pInfoBuffer = (BYTE *)malloc(dwInfoBufferLength+1);
	while (!HttpQueryInfo(hRequest, dwInfoLevel, pInfoBuffer, &dwInfoBufferLength, NULL))
	{
		DWORD dwError = GetLastError();
		if (dwError == ERROR_INSUFFICIENT_BUFFER)
		{
			free(pInfoBuffer);
			pInfoBuffer = (BYTE *)malloc(dwInfoBufferLength+1);
		}
		else
		{
			TRACE("HttpQueryInfo failed, error = %d (0x%x)\n", GetLastError(), GetLastError());
			return HRESULT_FROM_WIN32(GetLastError());
		}
	}

	pInfoBuffer[dwInfoBufferLength] = '\0';
	TRACE("%s", pInfoBuffer);
	free(pInfoBuffer);

	return S_OK;
}

HRESULT Uploader::ReadResponseBody(HINTERNET hRequest)
{
	DWORD dwBytesAvailable;
	while (InternetQueryDataAvailable(hRequest, &dwBytesAvailable, 0, 0))
	{
		BYTE *pMessageBody = (BYTE *)malloc(dwBytesAvailable+1);

		DWORD dwBytesRead;
		BOOL bResult = InternetReadFile(hRequest, pMessageBody, dwBytesAvailable, &dwBytesRead);
		if (!bResult)
		{
			free(pMessageBody);
			TRACE("InternetReadFile failed, error = %d (0x%x)\n", GetLastError(), GetLastError());
			return HRESULT_FROM_WIN32(GetLastError());
		}

		if (dwBytesRead == 0)
		{
			free(pMessageBody);
			break;	// End of File.
		}

		pMessageBody[dwBytesRead] = '\0';
		TRACE("%hs", pMessageBody);
		free(pMessageBody);
	}

	return S_OK;
}

HRESULT Uploader::UploadFile(HINTERNET hRequest, CString strBoundary, LPCTSTR lpszFieldName, LPCTSTR lpszPathName, LPCTSTR lpszContentType, DWORD *pdwOverallBytesSent, DWORD dwOverallBytesTotal)
{
	CStringA fileHeader = GenerateFileHeader(strBoundary, lpszFieldName, lpszPathName, lpszContentType);

	DWORD dwBytesWritten;
	if (!InternetWriteFile(hRequest, fileHeader, fileHeader.GetLength(), &dwBytesWritten))
	{
		TRACE("InternetWriteFile failed, error = %d (0x%x)\n", GetLastError(), GetLastError());
		return HRESULT_FROM_WIN32(GetLastError());
	}

	HANDLE hFile = CreateFile(lpszPathName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		TRACE("CreateFile failed, error = %d (0x%x)\n", GetLastError(), GetLastError());
		return HRESULT_FROM_WIN32(GetLastError());
	}

	m_pProgress->OnFileBegin(lpszPathName);
	HRESULT hr = UploadFileContent(hRequest, lpszPathName, hFile, pdwOverallBytesSent, dwOverallBytesTotal);
	if (FAILED(hr))
	{
		CloseHandle(hFile);
		m_pProgress->OnFileComplete(lpszPathName, hr);
		return hr;
	}

	m_pProgress->OnFileComplete(lpszPathName, S_OK);
	CloseHandle(hFile);

	// After the file:
	CStringA fileTrailer = GenerateFileTrailer();
	if (!InternetWriteFile(hRequest, fileTrailer, fileTrailer.GetLength(), &dwBytesWritten))
	{
		TRACE("InternetWriteFile failed, error = %d (0x%x)\n", GetLastError(), GetLastError());
		return HRESULT_FROM_WIN32(GetLastError());
	}

	return S_OK;
}

HRESULT Uploader::UploadField(HINTERNET hRequest, CString strBoundary, LPCTSTR lpszFieldName, LPCTSTR lpszFieldValue)
{
	USES_CONVERSION;

	CStringA fieldHeader = GenerateFieldHeader(strBoundary, lpszFieldName);

	DWORD dwBytesWritten;
	if (!InternetWriteFile(hRequest, fieldHeader, fieldHeader.GetLength(), &dwBytesWritten))
	{
		TRACE("InternetWriteFile failed, error = %d (0x%x)\n", GetLastError(), GetLastError());
		return HRESULT_FROM_WIN32(GetLastError());
	}

	CStringA fieldValue = T2CA(lpszFieldValue);
	if (!InternetWriteFile(hRequest, fieldValue, fieldValue.GetLength(), &dwBytesWritten))
	{
		TRACE("InternetWriteFile failed, error = %d (0x%x)\n", GetLastError(), GetLastError());
		return HRESULT_FROM_WIN32(GetLastError());
	}

	CStringA fieldTrailer = GenerateFieldTrailer();
	if (!InternetWriteFile(hRequest, fieldTrailer, fieldTrailer.GetLength(), &dwBytesWritten))
	{
		TRACE("InternetWriteFile failed, error = %d (0x%x)\n", GetLastError(), GetLastError());
		return HRESULT_FROM_WIN32(GetLastError());
	}

	return S_OK;
}

HRESULT Uploader::UploadFileContent(HINTERNET hRequest, LPCTSTR lpszPathName, HANDLE hFile, DWORD *pdwOverallBytesSent, DWORD dwOverallBytesTotal)
{
	const int PROGRESS_INTERVAL = 500;	// Report progress twice a second.

	DWORD dwLocalFileSize = GetFileSize(hFile, NULL);
	DWORD dwFileBytesSent = 0;
	DWORD dwBytesPerSecond = m_pSettings->GetCachedBytesPerSecond();
	DWORD dwTimeStarted = GetTickCount() / PROGRESS_INTERVAL;
	DWORD dwTimeLast = dwTimeStarted;

	DWORD cbBuffer = m_pSettings->GetBufferSize();
	BYTE *pBuffer = (BYTE *)malloc(cbBuffer);

	DWORD dwSecondsToFileCompletion = dwLocalFileSize / dwBytesPerSecond;
	DWORD dwSecondsToOverallCompletion = dwOverallBytesTotal / dwBytesPerSecond;
	m_pProgress->OnFileProgress(lpszPathName, 0, dwLocalFileSize, dwSecondsToFileCompletion, *pdwOverallBytesSent, dwOverallBytesTotal, dwSecondsToOverallCompletion, dwBytesPerSecond);
	for (;;)
	{
		if (m_pProgress->CheckCancel())
		{
			free(pBuffer);
			TRACE("User cancelled.\n");
			return E_USER_CANCELLED;
		}

		DWORD dwBytesRead;
		if (!ReadFile(hFile, pBuffer, cbBuffer, &dwBytesRead, NULL))
		{
			free(pBuffer);
			TRACE("ReadFile failed, error = %d (0x%x)\n", GetLastError(), GetLastError());
			return HRESULT_FROM_WIN32(GetLastError());
		}

		if (dwBytesRead == 0)
			break;

		// Write that to the other end:
		DWORD dwBytesWritten;
		if (!InternetWriteFile(hRequest, pBuffer, dwBytesRead, &dwBytesWritten))
		{
			free(pBuffer);

			/* One cause I've found for InternetWriteFile failing, when writing to a PHP script,
			 * is that max_execution_time is too small, and the file is too large.
			 *
			 * The PHP script gets killed, and we see ERROR_INTERNET_CONNECTION_ABORTED.
			 */
			TRACE("InternetWriteFile failed, error = %d (0x%x)\n", GetLastError(), GetLastError());

			// InternetGetLastResponseInfo doesn't do anything useful
			// for HTTP connections at this point.
			return HRESULT_FROM_WIN32(GetLastError());
		}

		dwFileBytesSent += dwBytesWritten;
		*pdwOverallBytesSent += dwBytesWritten;
		
		// Uncomment this to slow it down a bit, if you're testing against a local server:
		//Sleep(50);

		// Avoid reporting progress too often.
		DWORD dwTimeNow = GetTickCount() / PROGRESS_INTERVAL;
		if (dwTimeNow != dwTimeLast)
		{
			DWORD dwSecondsTaken = dwTimeNow - dwTimeStarted;
			dwBytesPerSecond = dwFileBytesSent / dwSecondsTaken;

			DWORD dwFileBytesRemaining = dwLocalFileSize - dwFileBytesSent;
			dwSecondsToFileCompletion = dwFileBytesRemaining / dwBytesPerSecond;

			DWORD dwOverallBytesRemaining = dwOverallBytesTotal - *pdwOverallBytesSent;
			dwSecondsToOverallCompletion = dwOverallBytesRemaining / dwBytesPerSecond;

			m_pProgress->OnFileProgress(lpszPathName, dwFileBytesSent, dwLocalFileSize, dwSecondsToFileCompletion, *pdwOverallBytesSent, dwOverallBytesTotal, dwSecondsToOverallCompletion, dwBytesPerSecond);
			dwTimeLast = dwTimeNow;
		}
	}

	m_pSettings->SetCachedBytesPerSecond(dwBytesPerSecond);
	m_pProgress->OnFileProgress(lpszPathName, dwLocalFileSize, dwLocalFileSize, dwSecondsToFileCompletion, *pdwOverallBytesSent, dwOverallBytesTotal, dwSecondsToOverallCompletion, dwBytesPerSecond);
	free(pBuffer);
	return S_OK;
}

/* static */
void CALLBACK Uploader::StaticInternetStatusCallback(HINTERNET hInternet, DWORD_PTR dwContext,
														 DWORD dwInternetStatus, LPVOID lpvStatusInformation, DWORD dwStatusInformationLength)
{
//	TRACE("StaticInternetStatusCallback(hInternet = %p, dwContext = %p, dwInternetStatus = %d, lpvStatusInformation = %p, dwStatusInformationLength = %d)\n",
//				hInternet, dwContext, dwInternetStatus, lpvStatusInformation, dwStatusInformationLength);

	Uploader *pThis = reinterpret_cast<Uploader *>(dwContext);
	pThis->InternetStatusCallback(hInternet, dwInternetStatus, lpvStatusInformation, dwStatusInformationLength);
}

void Uploader::InternetStatusCallback(HINTERNET hInternet, DWORD dwInternetStatus, LPVOID lpvStatusInformation, DWORD dwStatusInformationLength)
{
	switch (dwInternetStatus)
	{
	case INTERNET_STATUS_RESOLVING_NAME:
		// lpvStatusInformation is the server name.
		m_pProgress->OnResolvingName((LPCTSTR)lpvStatusInformation);
		break;
	case INTERNET_STATUS_NAME_RESOLVED:
		m_pProgress->OnNameResolved((LPCTSTR)lpvStatusInformation);
		break;
	case INTERNET_STATUS_CONNECTING_TO_SERVER:
		// The documentation says that lpvStatusInformation is a SOCKADDR structure.  It lies.
		m_pProgress->OnConnectingToServer((LPCTSTR)lpvStatusInformation);
		break;
	case INTERNET_STATUS_CONNECTED_TO_SERVER:
		// The documentation says that lpvStatusInformation is a SOCKADDR structure.  It lies.
		m_pProgress->OnConnectedToServer((LPCTSTR)lpvStatusInformation);
		break;
	case INTERNET_STATUS_SENDING_REQUEST:
		// lpvStatusInformation is NULL.
		m_pProgress->OnSendingRequest();
		break;
	case INTERNET_STATUS_REQUEST_SENT:
		{
			DWORD dwBytesSent = *(DWORD *)lpvStatusInformation;
//			TRACE("INTERNET_STATUS_REQUEST_SENT: %d bytes\n", dwBytesSent);
			m_pProgress->OnRequestSent(dwBytesSent);
		}
		break;
	case INTERNET_STATUS_RECEIVING_RESPONSE:
		// lpvStatusInformation is NULL.
		m_pProgress->OnReceivingResponse();
		break;
	case INTERNET_STATUS_RESPONSE_RECEIVED:
		{
			DWORD dwBytesReceived = *(DWORD *)lpvStatusInformation;
			m_pProgress->OnResponseReceived(dwBytesReceived);
		}
		break;
	case INTERNET_STATUS_CTL_RESPONSE_RECEIVED:
		TRACE("INTERNET_STATUS_CTL_RESPONSE_RECEIVED: Not implemented.\n");
		break;
	case INTERNET_STATUS_PREFETCH:
		TRACE("INTERNET_STATUS_PREFETCH:\n");
		break;
	case INTERNET_STATUS_CLOSING_CONNECTION:
		// lpvStatusInformation is NULL.
		m_pProgress->OnClosingConnection();
		break;
	case INTERNET_STATUS_CONNECTION_CLOSED:
		// lpvStatusInformation is NULL.
		m_pProgress->OnConnectionClosed();
		break;
	case INTERNET_STATUS_HANDLE_CREATED:
		{
			HINTERNET hCreated = *(HINTERNET *)lpvStatusInformation;
			TRACE("INTERNET_STATUS_HANDLE_CREATED: %p\n", hCreated);
		}
		break;
	case INTERNET_STATUS_HANDLE_CLOSING:
		{
			HINTERNET hClosing = *(HINTERNET *)lpvStatusInformation;
			TRACE("INTERNET_STATUS_HANDLE_CLOSING: %p\n", hClosing);
		}
		break;
	case INTERNET_STATUS_DETECTING_PROXY:
		TRACE("INTERNET_STATUS_DETECTING_PROXY: Notifies the client application that a proxy has been detected.\n");
		break;
	case INTERNET_STATUS_REQUEST_COMPLETE:
		TRACE("INTERNET_STATUS_REQUEST_COMPLETE:\n");
		break;
	case INTERNET_STATUS_REDIRECT:
		TRACE("INTERNET_STATUS_REDIRECT:\n");
		break;
	case INTERNET_STATUS_INTERMEDIATE_RESPONSE:
		TRACE("INTERNET_STATUS_INTERMEDIATE_RESPONSE:\n");
		break;
	case INTERNET_STATUS_USER_INPUT_REQUIRED:
		TRACE("INTERNET_STATUS_USER_INPUT_REQUIRED:\n");
		break;
	case INTERNET_STATUS_STATE_CHANGE:
		TRACE("INTERNET_STATUS_STATE_CHANGE:\n");
		break;
	case INTERNET_STATUS_COOKIE_SENT:
		TRACE("INTERNET_STATUS_COOKIE_SENT:\n");
		break;
	case INTERNET_STATUS_COOKIE_RECEIVED:
		TRACE("INTERNET_STATUS_COOKIE_RECEIVED:\n");
		break;
	case INTERNET_STATUS_PRIVACY_IMPACTED:
		TRACE("INTERNET_STATUS_PRIVACY_IMPACTED:\n");
		break;
	case INTERNET_STATUS_P3P_HEADER:
		TRACE("INTERNET_STATUS_P3P_HEADER:\n");
		break;
	case INTERNET_STATUS_P3P_POLICYREF:
		TRACE("INTERNET_STATUS_P3P_POLICYREF:\n");
		break;
	case INTERNET_STATUS_COOKIE_HISTORY:
		TRACE("INTERNET_STATUS_COOKIE_HISTORY:\n");
		break;
	}
}
