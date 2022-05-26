#include "StdAfx.h"
#include "ApplicationProfile.h"

RegistryApplicationProfile::RegistryApplicationProfile(HKEY hKey)
	: m_hKey(hKey)
{
}

HKEY RegistryApplicationProfile::GetSectionKey(LPCTSTR lpszSection, bool bWriteAccess)
{
	if (!m_hKey)
		return NULL;

	HKEY hk = NULL;
	LONG result = RegCreateKeyEx(m_hKey, lpszSection, NULL, NULL, 0, bWriteAccess ? KEY_WRITE : KEY_READ, NULL, &hk, NULL);
	if (result == ERROR_SUCCESS)
		return hk;

	return NULL;
}

CString RegistryApplicationProfile::GetString(LPCTSTR lpszSection, LPCTSTR lpszEntry, LPCTSTR lpszDefault)
{
	HKEY hSectionKey = GetSectionKey(lpszSection, false);
	if (hSectionKey)
	{
		DWORD type;
		DWORD cbData;
		LONG result = RegQueryValueEx(hSectionKey, lpszEntry, NULL, &type, NULL, &cbData);
		
		if (result == ERROR_SUCCESS || result == ERROR_MORE_DATA)
		{
			if (type == REG_SZ)
			{
				TCHAR *pData = (TCHAR *)malloc(cbData);
				result = RegQueryValueEx(hSectionKey, lpszEntry, NULL, &type, (BYTE *)pData, &cbData);

				CString value(pData);
				free(pData);
				return value;
			}
		}
	}

	return CString(lpszDefault);
}

void RegistryApplicationProfile::WriteString(LPCTSTR lpszSection, LPCTSTR lpszEntry, LPCTSTR lpszValue)
{
	HKEY hSectionKey = GetSectionKey(lpszSection, true);
	if (hSectionKey)
	{
		DWORD type = REG_SZ;
		const BYTE *pData = (const BYTE *)lpszValue;
		DWORD cbData = (DWORD)sizeof(TCHAR) * _tcslen(lpszValue);

		LONG result = RegSetValueEx(hSectionKey, lpszEntry, 0, type, pData, cbData);
		ASSERT(result == ERROR_SUCCESS);
	}
}

DWORD RegistryApplicationProfile::GetInteger(LPCTSTR lpszSection, LPCTSTR lpszEntry, DWORD dwDefault)
{
	HKEY hSectionKey = GetSectionKey(lpszSection, false);
	if (hSectionKey)
	{
		DWORD type;
		DWORD cbData;
		LONG result = RegQueryValueEx(hSectionKey, lpszEntry, NULL, &type, NULL, &cbData);

		if (result == ERROR_SUCCESS || result == ERROR_MORE_DATA)
		{
			if (type == REG_DWORD)
			{
				DWORD value;
				result = RegQueryValueEx(hSectionKey, lpszEntry, NULL, &type, (BYTE *)&value, &cbData);
				return value;
			}
		}
	}

	return dwDefault;
}

void RegistryApplicationProfile::WriteInteger(LPCTSTR lpszSection, LPCTSTR lpszEntry, DWORD dwValue)
{
	HKEY hSectionKey = GetSectionKey(lpszSection, true);
	if (hSectionKey)
	{
		DWORD type = REG_DWORD;
		const BYTE *pData = (const BYTE *)&dwValue;
		DWORD cbData = sizeof(DWORD);

		LONG result = RegSetValueEx(hSectionKey, lpszEntry, 0, type, pData, cbData);
		ASSERT(result == ERROR_SUCCESS);
	}
}

void RegistryApplicationProfile::ClearSection(LPCTSTR lpszSection)
{
	LONG result = RegDeleteKey(m_hKey, lpszSection);
}

void RegistryApplicationProfile::EnumerateEntries(LPCTSTR lpszSection, EntryCallback *pCallback)
{
	HKEY hSectionKey = GetSectionKey(lpszSection, false);

	DWORD cValues;
	DWORD cchMaxValueNameLen;
	DWORD cbMaxValueLen;
	LONG result = RegQueryInfoKey(hSectionKey, NULL, NULL, NULL, NULL, NULL, NULL, &cValues, &cchMaxValueNameLen, &cbMaxValueLen, NULL, NULL);
	if (result != ERROR_SUCCESS)
		return;

	// MSDN: "the size does not include the terminating NULL character".
	cchMaxValueNameLen += sizeof(TCHAR);
	cbMaxValueLen += sizeof(TCHAR);

	TCHAR *pszValueName = (TCHAR *)malloc(cchMaxValueNameLen * sizeof(TCHAR));
	BYTE *pData = (BYTE *)malloc(cbMaxValueLen);

	for (DWORD dwIndex = 0; dwIndex < cValues; ++dwIndex)
	{
		DWORD cchValueName = cchMaxValueNameLen;
		DWORD cbData = cbMaxValueLen;
		
		DWORD type;
		result = RegEnumValue(hSectionKey, dwIndex, pszValueName, &cchValueName, NULL, &type, pData, &cbData);
		if (result == ERROR_SUCCESS && type == REG_SZ)
		{
			pCallback->OnEntry(pszValueName, (LPCTSTR)pData);
		}
	}

	free(pData);
	free(pszValueName);
}
