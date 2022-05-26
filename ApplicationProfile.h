#pragma once

class ApplicationProfile
{
public:
	virtual CString GetString(LPCTSTR lpszSection, LPCTSTR lpszEntry, LPCTSTR lpszDefault) = 0;
	virtual void WriteString(LPCTSTR lpszSection, LPCTSTR lpszEntry, LPCTSTR lpszValue) = 0;

	virtual DWORD GetInteger(LPCTSTR lpszSection, LPCTSTR lpszEntry, DWORD dwDefault) = 0;
	virtual void WriteInteger(LPCTSTR lpszSection, LPCTSTR lpszEntry, DWORD dwValue) = 0;

	virtual void ClearSection(LPCTSTR lpszSection) = 0;

	class EntryCallback
	{
	public:
		virtual void OnEntry(LPCTSTR lpszEntry, LPCTSTR lpszValue) = 0;
	};

	virtual void EnumerateEntries(LPCTSTR lpszSection, EntryCallback *pCallback) = 0;
};

class RegistryApplicationProfile : public ApplicationProfile
{
	HKEY m_hKey;

public:
	RegistryApplicationProfile(HKEY hKey);

public:
	virtual CString GetString(LPCTSTR lpszSection, LPCTSTR lpszEntry, LPCTSTR lpszDefault);
	virtual void WriteString(LPCTSTR lpszSection, LPCTSTR lpszEntry, LPCTSTR lpszValue);

	virtual DWORD GetInteger(LPCTSTR lpszSection, LPCTSTR lpszEntry, DWORD dwDefault);
	virtual void WriteInteger(LPCTSTR lpszSection, LPCTSTR lpszEntry, DWORD dwValue);

	virtual void ClearSection(LPCTSTR lpszSection);
	virtual void EnumerateEntries(LPCTSTR lpszSection, EntryCallback *pCallback);

private:
	HKEY GetSectionKey(LPCTSTR lpszSection, bool bWriteAccess);
};
