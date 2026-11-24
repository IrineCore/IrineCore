// stdafx.h : include file for standard system include files that are used frequently, but are changed infrequently
// Standard system include files and project-specific include files
// Included header file.
//

#pragma once


#ifndef _WIN32_WINNT		// Allow use of features specific to Windows XP or later.                   
#define _WIN32_WINNT 0x0501	// Change this to the appropriate value to target other versions of Windows.
#endif						

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

// TODO: Reference additional headers your program requires here.
#include <windows.h>
#include <stdio.h>

#include <vector>
#include <deque>
#include <list>
#include <set>
#include <map>
#include <unordered_map>
#include <algorithm>
#include <string>

using namespace std;

#include "DNDefine.h"
#include "Stream.h"
#include "SundriesFunc.h"

#include "EternityEngine.h"

// Lua header file
extern "C" 
{
	#include "lua.h"
	#include "lualib.h"
	#include "lauxlib.h"
};

#include "lua_tinker.h"
#include "EtResourceMng.h"