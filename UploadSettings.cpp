#include "StdAfx.h"
#include "UploadSettings.h"
#include "ApplicationProfile.h"

class FileInfoCollector : public ApplicationProfile::EntryCallback
{
	FileInfoCollection *m_pFiles;

public:
	FileInfoCollector(FileInfoCollection *pFiles)
		: m_pFiles(pFiles)
	{
	}

public:
	virtual void OnEntry(LPCTSTR lpszEntry, LPCTSTR lpszValue)
	{
		FileInfo info(lpszEntry, lpszValue, _T("application/octet-stream"));
		m_pFiles->push_back(info);
	}
};

class FieldInfoCollector : public ApplicationProfile::EntryCallback
{
	FieldInfoCollection *m_pFields;

public:
	FieldInfoCollector(FieldInfoCollection *pFields)
		: m_pFields(pFields)
	{
	}

public:
	virtual void OnEntry(LPCTSTR lpszEntry, LPCTSTR lpszValue)
	{
		FieldInfo info(lpszEntry, lpszValue);
		m_pFields->push_back(info);
	}
};

void UploadSettings::Load(ApplicationProfile *pProfile)
{
	m_strHostName = pProfile->GetString(_T("Address"), _T("Host Name"), NULL);
	m_strUrlPath = pProfile->GetString(_T("Address"), _T("Url Path"), NULL);

	m_dwCachedBytesPerSecond = pProfile->GetInteger(_T("Transfer"), _T("Estimated Transfer Rate"), INITIAL_ESTIMATED_TRANSFER_RATE);

	ClearFiles();

	FileInfoCollector fileCollector(&m_files);
	pProfile->EnumerateEntries(_T("Files"), &fileCollector);

	ClearFields();
	FieldInfoCollector fieldCollector(&m_fields);
	pProfile->EnumerateEntries(_T("Fields"), &fieldCollector);
}

void UploadSettings::Save(ApplicationProfile *pProfile)
{
	pProfile->WriteString(_T("Address"), _T("Host Name"), m_strHostName);
	pProfile->WriteString(_T("Address"), _T("Url Path"), m_strUrlPath);

	pProfile->WriteInteger(_T("Transfer"), _T("Estimated Transfer Rate"), m_dwCachedBytesPerSecond);

	pProfile->ClearSection(_T("Files"));
	for (FileInfoCollection::const_iterator i = m_files.begin(); i != m_files.end(); ++i)
		pProfile->WriteString(_T("Files"), i->GetFieldName(), i->GetPathName());

	pProfile->ClearSection(_T("Fields"));
	for (FieldInfoCollection::const_iterator i = m_fields.begin(); i != m_fields.end(); ++i)
		pProfile->WriteString(_T("Fields"), i->GetFieldName(), i->GetFieldValue());
}

bool UploadSettings::SetAddress(LPCTSTR lpszAddress)
{
	TCHAR szScheme[_MAX_PATH];
	TCHAR szUserName[_MAX_PATH];
	TCHAR szPassword[_MAX_PATH];
	TCHAR szHostName[_MAX_PATH];
	TCHAR szUrlPath[_MAX_PATH];
	TCHAR szExtraInfo[_MAX_PATH];

	URL_COMPONENTS url;
	memset(&url, 0, sizeof(URL_COMPONENTS));
	url.dwStructSize = sizeof(URL_COMPONENTS);
	url.lpszScheme = szScheme;
	url.dwSchemeLength = _MAX_PATH;
	url.lpszHostName = szHostName;
	url.dwHostNameLength = _MAX_PATH;
	url.lpszUserName = szUserName;
	url.dwUserNameLength = _MAX_PATH;
	url.lpszPassword = szPassword;
	url.dwPasswordLength = _MAX_PATH;
	url.lpszUrlPath = szUrlPath;
	url.dwUrlPathLength = _MAX_PATH;
	url.lpszExtraInfo = szExtraInfo;
	url.dwExtraInfoLength = _MAX_PATH;

	if (!InternetCrackUrl(lpszAddress, 0, 0, &url))
		return false;

	// Verify that none of the other bits got filled in:
	if (_tcscmp(url.lpszScheme, _T("http")) != 0)
		return false;
	if (url.nPort != INTERNET_DEFAULT_HTTP_PORT)
		return false;	// Not supported yet.
	if (url.lpszUserName[0])
		return false;
	if (url.lpszPassword[0])
		return false;
	if (url.lpszExtraInfo[0])
		return false;

	m_strHostName = szHostName;
	m_strUrlPath = szUrlPath;

	return true;
}

CString UploadSettings::GetAddress() const
{
	// TODO: Use InternetCreateUrl to put it back together.
	CString strAddress;
	strAddress.Format(_T("http://%s%s"), LPCTSTR(m_strHostName), LPCTSTR(m_strUrlPath));

	return strAddress;
}

void UploadSettings::AddFile(LPCTSTR lpszFieldName, LPCTSTR lpszPathName, LPCTSTR lpszContentType)
{
	// TODO: Open the file here, so that we can guarantee it's valid all the way through?
	m_files.push_back(FileInfo(lpszFieldName, lpszPathName, lpszContentType));
}

void UploadSettings::ClearFiles()
{
	m_files.clear();
}

void UploadSettings::GetFiles(FileInfoCollection *pFiles) const
{
	// TODO: If we start storing file handles in the FileInfo object, we ought to use DuplicateHandle?
	pFiles->clear();
	std::copy(m_files.begin(), m_files.end(), std::back_inserter(*pFiles));
}

void UploadSettings::ClearFields()
{
	m_fields.clear();
}

void UploadSettings::GetFields(FieldInfoCollection *pFields) const
{
	pFields->clear();
	std::copy(m_fields.begin(), m_fields.end(), std::back_inserter(*pFields));
}

void UploadSettings::AddField(LPCTSTR lpszFieldName, LPCTSTR lpszFieldValue)
{
	m_fields.push_back(FieldInfo(lpszFieldName, lpszFieldValue));
}
