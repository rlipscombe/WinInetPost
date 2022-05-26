#include "StdAfx.h"
#include "FormatErrorMessage.h"
#include <psapi.h>	// for EnumProcessModules
#pragma comment(lib, "psapi.lib")

bool FormatErrorMessage(DWORD dwFlags, HMODULE hModule, DWORD dwMessageId, CString *pMessage)
{
	ASSERT(((dwFlags & FORMAT_MESSAGE_FROM_HMODULE) == 0) || (hModule != NULL));

	TCHAR *pBuffer = NULL;
	if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | dwFlags, hModule, dwMessageId, 0, (LPTSTR)&pBuffer, 0, NULL))
	{
		CString result(pBuffer);
		LocalFree(pBuffer);
		if (result.Right(2) == "\r\n")
			result = result.Left(result.GetLength()-2);
		*pMessage = result;
		return true;
	}

	return false;
}

bool EnumProcessModules(HANDLE hProcess, HMODULE **ppModules, DWORD *pModuleCount)
{
	*ppModules = NULL;
	*pModuleCount = 0;

	DWORD cb = sizeof(HMODULE) * 2;
	HMODULE *pModules = NULL;

	for (;;)
	{
		pModules = (HMODULE *)malloc(cb);
		if (!pModules)
			return false;

		DWORD cbNeeded;
		BOOL bResult = EnumProcessModules(GetCurrentProcess(), pModules, cb, &cbNeeded);
		if (!bResult)
		{
			free(pModules);
			return false;
		}

		if (cbNeeded > cb)
		{
			free(pModules);
			cb = cbNeeded;
		}
		else
		{
			*ppModules = pModules;
			*pModuleCount = cbNeeded / sizeof(HMODULE);
			return true;
		}
	}
}

CString FormatErrorMessage(DWORD dwMessageId)
{
	DWORD dwFacility = HRESULT_FACILITY(dwMessageId);
	if (dwFacility != FACILITY_ITF)
	{
		CString message;
		if (FormatErrorMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, dwMessageId, &message))
			return message;

		// If that still didn't work, have another go without the Win32 bits:
		if (dwFacility == FACILITY_WIN32)
		{
			if (FormatErrorMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, HRESULT_CODE(dwMessageId), &message))
				return message;
		}

		// That didn't work, so have a go against all the currently loaded modules.
		HMODULE *pModules;
		DWORD cModules;
		if (EnumProcessModules(GetCurrentProcess(), &pModules, &cModules))
		{
			for (DWORD i = 0; i < cModules; ++i)
			{
				HMODULE hModule = pModules[i];

				if (FormatErrorMessage(FORMAT_MESSAGE_FROM_HMODULE, hModule, dwMessageId, &message))
				{
					free(pModules);
					return message;
				}

				// If that still didn't work, have another go without the Win32 bits:
				if (dwFacility == FACILITY_WIN32)
				{
					if (FormatErrorMessage(FORMAT_MESSAGE_FROM_HMODULE, hModule, HRESULT_CODE(dwMessageId), &message))
					{
						free(pModules);
						return message;
					}
				}
			}

			free(pModules);
		}
	}

	CString result;
	result.Format(_T("Error 0x%08x"), dwMessageId);
	return result;
}

