/* VersionInfo.h
 *
 * Copyright (C) 2003 Roger Lipscombe
 *     http://www.differentpla.net/
 */

bool VerifyDllVersionInfo(const char *dll, DWORD dwReqdVersion);

/** Ironically, Windows 98 (stock) doesn't support Wizard97.  Windows 98SE does.
 * Actually, we need IE5, so we'll check the comctl32.dll version here...
 */
inline bool SupportsWizard97()
{
#define WIZARD97_VERSION_NEEDED MAKELONG(80,5)
    return VerifyDllVersionInfo("comctl32.dll", WIZARD97_VERSION_NEEDED);
}

inline bool IsComCtl6()
{
#define COMCTL6_VERSION MAKELONG(0,6)
	return VerifyDllVersionInfo("comctl32.dll", COMCTL6_VERSION);
}
