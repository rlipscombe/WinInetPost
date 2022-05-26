#pragma once
#include <vector>
#include "FileInfo.h"
#include "FieldInfo.h"

class ApplicationProfile;

class UploadSettings
{
	CString m_strHostName;
	CString m_strUrlPath;

	FileInfoCollection m_files;
	FieldInfoCollection m_fields;

	DWORD m_dwCachedBytesPerSecond;

	// A random guess: 56Kbps modem (ish)
	static const DWORD INITIAL_ESTIMATED_TRANSFER_RATE = 5600;

public:
	UploadSettings()
		: m_dwCachedBytesPerSecond(INITIAL_ESTIMATED_TRANSFER_RATE)
	{
	}

	/* In order to decouple the Settings object from where it loads/saves its
	 * settings, we introduce an ApplicationProfile object, which is an
	 * abstract type.  This allows us to hide what's going on.
	 */
	void Load(ApplicationProfile *pProfile);
	void Save(ApplicationProfile *pProfile);

	bool SetAddress(LPCTSTR lpszAddress);

	CString GetAddress() const;
	CString GetHostName() const { return m_strHostName; }
	CString GetUrlPath() const { return m_strUrlPath; }

	void ClearFiles();
	void AddFile(LPCTSTR lpszFieldName, LPCTSTR lpszFileName, LPCTSTR lpszContentType);
	void GetFiles(FileInfoCollection *pFiles) const;

	void ClearFields();
	void AddField(LPCTSTR lpszFieldName, LPCTSTR lpszFieldValue);
	void GetFields(FieldInfoCollection *pFields) const;

	DWORD GetCachedBytesPerSecond() const { return m_dwCachedBytesPerSecond; }
	void SetCachedBytesPerSecond(DWORD dwBytesPerSecond) { m_dwCachedBytesPerSecond = dwBytesPerSecond; }
};
