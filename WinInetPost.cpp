/* WinInetPost.cpp
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <WinInet.h>
#pragma comment(lib, "WinInet.lib")
#include <stdio.h>
#include <malloc.h>
#include <string>
#include <time.h>
#include <assert.h>

std::string GenerateMultipartBoundary();
std::string GenerateRequestBody(const std::string &filename, const std::string &boundary);

int main(int argc, char *argv[])
{
	if (argc < 4)
	{
		printf("Usage:\n\tWinInetPost server object localfile\ne.g.\n\tWinInetPost vague /~roger/cgi-bin/upload2 C:\\Temp\\Foo\n");
		return 1;
	}

	srand((int)time(NULL));

	LPCTSTR lpszServerName = argv[1];
	LPCTSTR lpszObjectName = argv[2];
	LPCTSTR lpszLocalFile = argv[3];

	LPCTSTR lpszAgent = "WinInetPost/0.1";
	HINTERNET hInternet = InternetOpen(lpszAgent, INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
	if (!hInternet)
	{
		fprintf(stderr, "InternetOpen failed, error = %d (0x%x)\n", GetLastError(), GetLastError());
		return 1;
	}

	INTERNET_PORT nServerPort = INTERNET_DEFAULT_HTTP_PORT;
	LPCTSTR lpszUserName = NULL;
	LPCTSTR lpszPassword = NULL;
	DWORD dwConnectFlags = 0;
	DWORD dwConnectContext = 0;
	HINTERNET hConnect = InternetConnect(hInternet, lpszServerName, nServerPort,
											lpszUserName, lpszPassword,
											INTERNET_SERVICE_HTTP,
											dwConnectFlags, dwConnectContext);
	if (!hConnect)
	{
		InternetCloseHandle(hInternet);
		fprintf(stderr, "InternetConnect failed, error = %d (0x%x)\n", GetLastError(), GetLastError());
		return 1;
	}

	LPCTSTR lpszVerb = "POST";
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
	DWORD dwOpenRequestContext = 0;
	HINTERNET hRequest = HttpOpenRequest(hConnect, lpszVerb, lpszObjectName, lpszVersion,
											lpszReferrer, lplpszAcceptTypes,
											dwOpenRequestFlags, dwOpenRequestContext);
	if (!hRequest)
	{
		InternetCloseHandle(hConnect);
		InternetCloseHandle(hInternet);
		fprintf(stderr, "HttpOpenRequest failed, error = %d (0x%x)\n", GetLastError(), GetLastError());
		return 1;
	}

	// Using POST to send a file:
	// Add the Content-Type header:
	std::string boundary = GenerateMultipartBoundary();
	std::string contentTypeHeader = "Content-Type: multipart/form-data; boundary=" + boundary;
	HttpAddRequestHeaders(hRequest, contentTypeHeader.c_str(), -1, HTTP_ADDREQ_FLAG_ADD);

	std::string requestBody = GenerateRequestBody(lpszLocalFile, boundary);
	LPVOID lpOptional = const_cast<char *>(requestBody.data());
	DWORD dwOptionalLength = (DWORD)requestBody.size();

	BOOL bResult = HttpSendRequest(hRequest, NULL, 0, lpOptional, dwOptionalLength);
	if (!bResult)
	{
		InternetCloseHandle(hRequest);
		InternetCloseHandle(hConnect);
		InternetCloseHandle(hInternet);
		fprintf(stderr, "HttpSendRequest failed, error = %d (0x%x)\n", GetLastError(), GetLastError());
		return 1;
	}

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
			fprintf(stderr, "HttpQueryInfo failed, error = %d (0x%x)\n", GetLastError(), GetLastError());
			break;
		}
	}

	pInfoBuffer[dwInfoBufferLength] = '\0';
	printf("%s", pInfoBuffer);
	free(pInfoBuffer);

	DWORD dwBytesAvailable;
	while (InternetQueryDataAvailable(hRequest, &dwBytesAvailable, 0, 0))
	{
		BYTE *pMessageBody = (BYTE *)malloc(dwBytesAvailable+1);

		DWORD dwBytesRead;
		BOOL bResult = InternetReadFile(hRequest, pMessageBody, dwBytesAvailable, &dwBytesRead);
		if (!bResult)
		{
			fprintf(stderr, "InternetReadFile failed, error = %d (0x%x)\n", GetLastError(), GetLastError());
			break;
		}

		if (dwBytesRead == 0)
			break;	// End of File.

		pMessageBody[dwBytesRead] = '\0';
		printf("%s", pMessageBody);
		free(pMessageBody);
	}

	InternetCloseHandle(hRequest);
	InternetCloseHandle(hConnect);
	InternetCloseHandle(hInternet);

	return 0;
}

std::string GenerateMultipartBoundary()
{
	std::string boundaryPrefix(27, '-');

	// We now need 12 hex digits.
	int r0 = rand() & 0xffff;
	int r1 = rand() & 0xffff;
	int r2 = rand() & 0xffff;

	char temp[13];
	sprintf(temp, "%04X%04X%04X", r0, r1, r2);

	return boundaryPrefix + temp;
}

std::string GetFileContents(const std::string &filename)
{
	HANDLE hFile = CreateFile(filename.c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
	assert(hFile != INVALID_HANDLE_VALUE);

	DWORD dwFileSize = GetFileSize(hFile, NULL);
	char *p = (char *)malloc(dwFileSize);

	DWORD dwBytesRead;
	ReadFile(hFile, p, dwFileSize, &dwBytesRead, NULL);

	std::string fileContents;
	fileContents.assign(p, dwFileSize);
	free(p);

	return fileContents;
}

std::string GenerateRequestBody(const std::string &filename, const std::string &boundary)
{
	std::string requestBody;

	// For each file...
	std::string field_name = "upload_file";
	std::string contentDisposition = "Content-Disposition: form-data; name=\"" + field_name + "\"; filename=\"" + filename + "\"";
	std::string contentType = "Content-Type: application/octet-stream";

	requestBody.append("--" + boundary + "\r\n");
	requestBody.append(contentDisposition + "\r\n");
	requestBody.append(contentType + "\r\n");
	requestBody.append("\r\n");
	requestBody.append(GetFileContents(filename) + "\r\n");

	// After the last file:
	requestBody.append("--" + boundary + "--\r\n");

	return requestBody;
}
