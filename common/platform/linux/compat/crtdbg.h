#pragma once
#include "windows.h"
#include <cassert>
#ifndef _CrtSetDbgFlag
#define _CrtSetDbgFlag(f) (0)
#define _CRTDBG_ALLOC_MEM_DF 0x01
#define _CRTDBG_LEAK_CHECK_DF 0x20
#define _NORMAL_BLOCK 1
#endif
#ifndef _RPT0
#define _RPT0(rptno, msg) ((void)0)
#define _RPT1(rptno, msg, a1) ((void)0)
#define _RPT2(rptno, msg, a1, a2) ((void)0)
#define _RPTF0(rptno, msg) ((void)0)
#define _RPTF1(rptno, msg, a1) ((void)0)
#endif
