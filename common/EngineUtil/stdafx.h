// stdafx.h : include file for standard system include files that are used frequently, but are changed infrequently
// Standard system include files and project-specific include files
// Included header file.
//

#pragma once


#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers.

// TODO: Reference additional headers your program requires here.
#pragma warning( disable:4996 )

#define _WIN32_WINNT 0x0501

#include <windows.h>
#include <stdio.h>
#include <string.h>

#include <vector>
#include <map>
#include <string>
#include <xstring>
using namespace std;

typedef std::basic_string< TCHAR > tstring;

#if defined(_CLIENT) && defined(_WIN32)
#include <d3d9.h>
#include <d3dx9.h>
#include <dinput.h>
#else
#include "EtMathPortable.h"
#endif

#include "DNDefine.h"
#include "AssertX.h"
#include "Singleton.h"
#include "SmartPtr.h"
#include "Stream.h"
#include "SundriesFunc.h"


#include "EtMathWrapperD3D.h"
#include "EtPrimitive.h"
#include "EtCollisionPrimitive.h"
