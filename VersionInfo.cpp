/* VersionInfo.cpp
 *
 * Copyright (C) 2003 Roger Lipscombe
 *     http://www.differentpla.net/
 */

#include "StdAfx.h"
#include "VersionInfo.h"
#include <ShlWapi.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/** Check that the DLL has at least the version required.  It assumes that the DLL
 * is already loaded.
 */
bool VerifyDllVersionInfo(const char *dll, DWORD dwReqdVersion)
{
	HINSTANCE hInst = ::GetModuleHandleA(dll);
	if (!hInst)
		return false;

	DLLGETVERSIONPROC pfn = (DLLGETVERSIONPROC)GetProcAddress(hInst, "DllGetVersion");
	if (pfn == NULL)
		return false;

	DLLVERSIONINFO dvi;
	memset(&dvi, 0, sizeof(dvi));
	dvi.cbSize = sizeof(dvi);
	HRESULT hr = pfn(&dvi);
	if (FAILED(hr))
		return false;

	if (!(dvi.dwMajorVersion <= 0xFFFF) || !(dvi.dwMinorVersion <= 0xFFFF))
		return false;

	DWORD dwVersion = MAKELONG(dvi.dwMinorVersion, dvi.dwMajorVersion);
	return (dwVersion >= dwReqdVersion);
}
