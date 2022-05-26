#pragma once

class CMultiFileDialog : public CFileDialog
{
	static const size_t BUFFER_SIZE_CHARS = 32 * 1024;

	TCHAR *m_pBuffer;
	TCHAR *m_pOldBuffer;

public:
	CMultiFileDialog()
		: CFileDialog(TRUE, NULL, NULL, OFN_HIDEREADONLY|OFN_ALLOWMULTISELECT)
	{
		m_pOldBuffer = GetOFN().lpstrFile;

		GetOFN().nMaxFile = BUFFER_SIZE_CHARS;
		m_pBuffer = (TCHAR *)malloc(BUFFER_SIZE_CHARS * sizeof(TCHAR));
		memset(m_pBuffer, 0, BUFFER_SIZE_CHARS * sizeof(TCHAR));
		GetOFN().lpstrFile = m_pBuffer;
	}

	~CMultiFileDialog()
	{
		GetOFN().lpstrFile = m_pOldBuffer;
		free(m_pBuffer);
	}
};
