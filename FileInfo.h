#pragma once

class FileInfo
{
	CString m_fieldName;
	CString m_pathName;
	CString m_contentType;

public:
	FileInfo(LPCTSTR lpszFieldName, LPCTSTR lpszPathName, LPCTSTR lpszContentType)
		: m_fieldName(lpszFieldName), m_pathName(lpszPathName), m_contentType(lpszContentType)
	{
	}

	CString GetFieldName() const { return m_fieldName; }
	CString GetPathName() const { return m_pathName; }
	CString GetContentType() const { return m_contentType; }
};

typedef std::vector< FileInfo > FileInfoCollection;
