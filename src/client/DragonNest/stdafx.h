// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//
#pragma warning(disable:4250)
#pragma warning(disable:4819)
#pragma warning(disable:4482)

// Modify the following defines if you have to target a platform prior to the ones specified below.
// Refer to MSDN for the latest info on corresponding values for different platforms.
#ifndef WINVER				// Allow use of features specific to Windows XP or later.
#define WINVER 0x0501		// Change this to the appropriate value to target other versions of Windows.
#endif

#ifndef _WIN32_WINNT		// Allow use of features specific to Windows XP or later.                   
#define _WIN32_WINNT 0x0501	// Change this to the appropriate value to target other versions of Windows.
#endif						

#ifndef _WIN32_WINDOWS		// Allow use of features specific to Windows 98 or later.
#define _WIN32_WINDOWS 0x0410 // Change this to the appropriate value to target Windows Me or later.
#endif

#ifndef _WIN32_IE			// Allow use of features specific to IE 6.0 or later.
#define _WIN32_IE 0x0600	// Change this to the appropriate value to target other versions of IE.
#endif

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

// Windows Header Files:
#include <windows.h>

// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

#include <vector>
#include <map>
#include <set>
#include <string>
#include <xstring>
#include <stack>

#include "DNLocaleHeader.h"
#include "DNDefine.h"

using namespace std;

typedef std::basic_string< TCHAR > tstring;

#include "Random.h"

#define _rand() g_Random.rand()
#define _srand( nValue ) g_Random.srand( nValue )
#define GetRandom() g_Random

// D3D
#pragma warning(disable:4819)
#define DIRECTINPUT_VERSION 0x0800

#if defined(_DEBUG) || defined(_RDEBUG)
#ifndef D3D_DEBUG_INFO
#define D3D_DEBUG_INFO
#endif
#ifndef SMARTPTR_VALID_CHECK		// Exclude from final build due to performance load.
#define SMARTPTR_VALID_CHECK
#endif
#endif
#include <d3d9.h>
#include <d3dx9.h>
#include <dinput.h>

#pragma warning(default:4819)
// DirectX Lib 
#pragma comment(lib,"d3d9.lib")
#ifdef _DEBUG
#pragma comment(lib,"d3dx9d.lib")
#else 
#pragma comment(lib,"d3dx9.lib")
#endif // _DEBUG
#pragma comment(lib, "ddraw.lib")
#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dsound.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "dbghelp.lib")
#pragma comment(lib, "imm32.lib")

#ifdef _USE_VOICECHAT
#ifdef _DEBUG
#pragma comment(lib, "..\\..\\Common\\VoiceChat\\DolbyAxon\\ClientLib\\iceclientlib_d.lib")
#else
#pragma comment(lib, "..\\..\\Common\\VoiceChat\\DolbyAxon\\ClientLib\\iceclientlib.lib")
#endif
#endif


// Related to tinyxml; get it from common/tinyxml if missing.
#include "tinyxml.h"

#ifndef TIXML_USE_STL
#define TIXML_USE_STL
#endif 

// Lua header file
extern "C" 
{
	#include "lua.h"
	#include "lualib.h"
	#include "lauxlib.h"
};
#include "lua_tinker.h"

#include "Version.h"

#if !defined(_FINAL_BUILD)
#define SW_MSDT_ASSERT	// assert() occurs
#define SW_MSDT_NOTINCLUDE_COMPILESAFETY
#define SW_MSDT_ERRORCHECK_ORIGINVALUE
#endif	// _FINAL_BUILD
#include "MemoryShuffleDataType.h"

// Engine include
#include "EternityEngine.h"
#include "EngineUtil.h"
// Sound Engine
#include "EtSoundEngine.h"

// Game Global member
#include "GlobalValue.h"
#include "EtResourceMng.h"
#include "EtUI.h"
#include "EtUIXML.h"
#include "SmartPtrDef.h"
#include "DnMouseCursor.h"

#include "DnEtcObject.h"
#include "DnActionSignalImp.h"

// Log window
#include "LogWnd.h"
#include "StringUtil.h"
#include "DnCommonUtil.h"

#include "Log.h"

using namespace GlobalEnum;

// Network include
#include "NetCommon.h"
#include "DNCommonDef.h"

// boost
#pragma warning( disable:4996 )
#pragma warning( disable:4819 )

#include <boost/algorithm/string.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/tuple/tuple.hpp>

#pragma warning( default:4819 )

// Network Link
#pragma comment(lib,"ws2_32.lib")
#pragma comment(lib,"Mswsock.lib")

const static string __NULL_STR("");


#if defined(_DEBUG) || defined(_RDEBUG)
	template <bool T> struct ASSERT_STRUCT {};
	template<> struct ASSERT_STRUCT<true> { static void static_assertion_failed() {} };
	#define StaticAssert(expr) { const bool __b = (expr) ? true : false; ASSERT_STRUCT<__b>::static_assertion_failed(); }
#else
	#define StaticAssert(expr) ((void)0)
#endif

#define CHECK_ALIGNMENT()  \
	struct __alignment_static_test {char _;int __;double ___;}; \
	StaticAssert( sizeof(__alignment_static_test) == 16 ); \
	StaticAssert( __alignof(__alignment_static_test) == 8 ); 



// Added by Hanki. Macro for video playback testing
//#define TEST_CUTSCENE

#define INSTANCE(cls) cls::GetInstance()
#define INSTANCE_(cls,func) cls::func()
#define STATICFUNC() 
#define GETROOM(obj)
#define ARG_STATIC_FUNC( object )



// Function that forces a NULL-terminated string.
// Added nDestLen due to a previous overrun issue.
// Check because string may be truncated if nLen >= nDestLen. // Kimbap
inline void _wcscpy( WCHAR* pDest, const int nDestLen, const WCHAR* pSrc, const int nLen )
{
	if(pDest == pSrc) return;
	memset(pDest, 0, nDestLen*sizeof(WCHAR));

	if( nLen <= 0 )
		return;
	wmemcpy_s( pDest, nDestLen, pSrc, nDestLen < nLen ? nDestLen : nLen );
	if( nLen >= nDestLen )
	{
		if( nLen > nDestLen )
			g_Log.Log( LogType::_ERROR, L"_wcscpy err %s Len:%d DestLen:%d\n", pSrc, nLen, nDestLen );
		pDest[nDestLen-1] = '\0';
	}
	else
		pDest[nLen] = '\0';
}

inline void _strcpy( char* pDest, const int nDestLen, const char* pSrc, const int nLen )
{
	if(pDest == pSrc) return;
	memset(pDest, 0, nDestLen);

	if( nLen <= 0 )
		return;
	memcpy_s( pDest, nDestLen, pSrc, nDestLen < nLen ? nDestLen : nLen );
	if( nLen >= nDestLen )
	{
		if( nLen > nDestLen )
			g_Log.Log( LogType::_ERROR, "_strcpy err %s\n", pSrc );
		pDest[nDestLen-1] = '\0';
	}
	else
		pDest[nLen] = '\0';
}

#ifndef DEL_TEMPLATE
#define DEL_TEMPLATE
template <class T>
class DeleteData
{
public:
	void operator() ( T elem )
	{
		if( elem )
		{
			delete elem;
			elem = NULL;
		}
	}

};
#endif
#define WS_DRAGONNEST      ( WS_POPUP | WS_CAPTION | WS_MINIMIZEBOX | /*WS_MAXIMIZEBOX |*/ WS_BORDER | WS_SYSMENU )

#if defined(_FILELOG_PACKET) || defined(_FILELOG_SPOS)
void _WriteLogEx( const char* pszFile, int nLine, const char* pszFunc, int nLogType, const char *szStr, ... );
void _WriteLog( int nLogType, const char *szStr, ... );
void _WriteLogA( const char *szStr, ... );
void _WriteDisplayLogEx( const char* pszFile, int nLine, const char* pszFunc, int Logtype, const char *szStr, ... );
void _WriteDisplayLog( int Logtype, const char *szStr, ... );
extern bool g_bInvalidDisconnect;
#define WriteLog( type, ... ) _WriteLogEx(__FILE__, __LINE__, __FUNCTION__, type, __VA_ARGS__)

#define InfoLog( ... )    _WriteDisplayLogEx(__FILE__, __LINE__, __FUNCTION__, 1, __VA_ARGS__)
#define ErrorLog( ... )   _WriteDisplayLogEx(__FILE__, __LINE__, __FUNCTION__, 3, __VA_ARGS__)

#ifndef _FINAL_BUILD
#define DebugLog( ... )   _WriteDisplayLogEx(__FILE__, __LINE__, __FUNCTION__, 4, __VA_ARGS__)
#define WarningLog( ... ) _WriteDisplayLogEx(__FILE__, __LINE__, __FUNCTION__, 2, __VA_ARGS__)
#else
#define DebugLog( ... )
#define WarningLog( ... )
#endif
#else
#define WriteLog( type, ... )
#define InfoLog( ... )
#define ErrorLog( ... )
#define DebugLog( ... )
#define WarningLog( ... )
#endif

#include "CommonMacros.h"
#include "BaseSet.h"
#include "DebugUtil.h"
#include "SafeStringMacros.h"

