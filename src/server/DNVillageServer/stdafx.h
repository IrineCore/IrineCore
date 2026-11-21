// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#ifndef _WIN32_WINNT		// Allow use of features specific to Windows XP or later.                   
#define _WIN32_WINNT 0x0501	// Change this to the appropriate value to target other versions of Windows.
#endif		

#define _CRT_RAND_S

#include "DNLocaleHeader.h"
#include "DNDefine.h"
#include "DNHeader.h"

#include "DNPacket.h"
#include "DNProtocol.h"

#include "EternityEngine.h"

#include "Random.h"

#define _rand() CRandom::GetInstance().rand()
#define _srand( nValue ) CRandom::GetInstance().srand( nValue )

#include "Log.h"

#include "EventControlHeader.h"

// Related to tinyxml; get it from common/tinyxml if missing.
#include "tinyxml.h"

#ifndef TIXML_USE_STL
#define TIXML_USE_STL
#endif 

#pragma comment (lib, "ole32.lib")
#pragma comment (lib, "oleaut32.lib")
#pragma comment( lib, "Odbc32.lib" )

#ifdef _USE_VOICECHAT
#pragma comment(lib, "ICE.lib")
#endif

// Lua header file
extern "C" 
{
	#include "lua.h"
	#include "lualib.h"
	#include "lauxlib.h"
};
#include "lua_tinker.h"
#include "StringUtil.h"

#include "EtResourceMng.h"
#include "DnTableDB.h"


// boost
#include <boost/format.hpp>

// Declared here to avoid typing it every time!
using namespace DB;

using namespace eSystem;
using namespace eChar;
using namespace eActor;
using namespace eProp;
using namespace eParty;
using namespace eItem;
using namespace eItemGoods;
using namespace eNpc;
using namespace eQuest;
using namespace eChat;
using namespace eTrade;
using namespace eSkill;
