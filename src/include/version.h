#include "version-generated.h"

/// Name
#ifdef _WIN64
#define NAME "Stratagus (64 bit)"
#else
#define NAME "Stratagus"
#endif

/// Description
#define DESCRIPTION NAME " - Strategy Gaming Engine"

#define _version_stringify_(s) #s
#define _version_stringify(s) _version_stringify_(s)

#define _version_str1 _version_stringify(StratagusMajorVersion) "." _version_stringify(StratagusMinorVersion) "." _version_stringify(StratagusPatchLevel)

#if StratagusPatchLevel2 > 0
#define _version_str2 _version_str1 "." _version_stringify(StratagusPatchLevel2)
#else
#define _version_str2 _version_str1
#endif

/// Engine version string
#ifdef StratagusBzrRev
#define VERSION _version_str2 "-bzr" _version_stringify(StratagusBzrRev)
#else
#define VERSION _version_str2
#endif

/// Stratagus version (1,2,3) -> 10203
#define StratagusVersion (StratagusMajorVersion * 10000 + StratagusMinorVersion * 100 + StratagusPatchLevel)

/// Homepage
#define HOMEPAGE "https://launchpad.net/stratagus"

/// License
#define LICENSE "GPL v2"

/// Copyright
#define COPYRIGHT "Copyright (c) 1998-2012 by The Stratagus Project and Pali Rohar"
