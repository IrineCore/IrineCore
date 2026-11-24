#pragma once
#include "windows.h"
#ifndef _DBTIMESTAMP_DEFINED
#define _DBTIMESTAMP_DEFINED
struct tagDBTIMESTAMP {
    SHORT  year;
    USHORT month;
    USHORT day;
    USHORT hour;
    USHORT minute;
    USHORT second;
    ULONG  fraction;
};
using DBTIMESTAMP = tagDBTIMESTAMP;
#endif
#ifndef _DBDATE_DEFINED
#define _DBDATE_DEFINED
struct tagDBDATE {
    SHORT  year;
    USHORT month;
    USHORT day;
};
using DBDATE = tagDBDATE;
#endif
