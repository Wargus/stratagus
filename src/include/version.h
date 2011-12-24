/// Name
#ifdef _WIN64
#define NAME "Stratagus (64 bit)"
#else
#define NAME "Stratagus"
#endif

// Description
#define DESCRIPTION NAME " - Strategy Gaming Engine"

/// Engine version shown
#define VERSION  "2.2.6"

/// Stratagus major version
#define StratagusMajorVersion  2

/// Stratagus minor version (maximal 99)
#define StratagusMinorVersion  2

/// Stratagus patch level (maximal 99)
#define StratagusPatchLevel    6

/// Stratagus patch level 2
#define StratagusPatchLevel2   0

/// Stratagus version (1,2,3) -> 10203
#define StratagusVersion (StratagusMajorVersion * 10000 + StratagusMinorVersion * 100 + StratagusPatchLevel)

/// Stratagus printf format string
#define StratagusFormatString   "%d.%d.%d"

/// Stratagus printf format arguments
#define StratagusFormatArgs(v)  (v) / 10000, ((v) / 100) % 100, (v) % 100

/// Homepage
#define HOMEPAGE "https://launchpad.net/stratagus"

/// License
#define LICENSE "GPL v2"

/// Copyright
#define COPYRIGHT "Copyright (c) 1998-2011 by The Stratagus Project and Pali Rohar"
