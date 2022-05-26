#pragma once

class FieldInfo
{
	CString m_fieldName;
	CString m_fieldValue;

public:
	FieldInfo(LPCTSTR lpszFieldName, LPCTSTR lpszFieldValue)
		: m_fieldName(lpszFieldName), m_fieldValue(lpszFieldValue)
	{
	}

	CString GetFieldName() const { return m_fieldName; }
	CString GetFieldValue() const { return m_fieldValue; }
};

typedef std::vector< FieldInfo > FieldInfoCollection;
