// stdafx.h : include file for standard system include files that are used frequently, but are changed infrequently
// Standard system include files and project-specific include files
// Included header file.
//

#pragma once


#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers.

#ifndef WINVER
#define WINVER         0x0410
#endif
#ifndef _WIN32_WINDOWS
#define _WIN32_WINDOWS 0x0410 
#endif
#ifndef _WIN32_WINNT
#define _WIN32_WINNT   0x0500 
#endif


#pragma warning( disable:4819 )
#pragma warning( disable:4996 )

#include <stdio.h>
#include <assert.h>

#include <vector>
#include <deque>
#include <list>
#include <set>
#include <map>
#include <unordered_map>
#include <algorithm>
#include <string>

#include "DNDefine.h"
#include "EternityEngine.h"
#include "EtResourceMng.h"