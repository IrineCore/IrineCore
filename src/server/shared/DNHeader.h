#pragma once

#include "Platform.h"
#include "IrineApp.h"
#include <winsock2.h>
#include <windows.h>
#include <winbase.h>
#include <process.h>
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <tchar.h>
#include <time.h>
#include <sql.h>
#include <sqlucode.h>
#include <sqlext.h>
#include <sqltypes.h>
#include <assert.h>
#include <conio.h>
#include <unordered_map>
#include <mswsock.h>


#include <set>
#include <vector>
#include <queue>
#include <deque>
#include <list>
#include <map>
#include <string>
#include <utility>

#include "SundriesFunc.h"
#include "CriticalSection.h"

#define DNHEADER	__DEFINED__		// Prevent errors when sub-sources (.h etc.) in the project include 'DNPrjSafe.h' without the base header

#include "DNConfig.h"
#include "DNCommonDef.h"
#include "DNServerDef.h"
#include "DNServerPacket.h"
#if defined( _SERVER )
#include "DNServerPacketDB.h"
#include "DNServerPacketGame.h"
#include "DNServerPacketLogin.h"
#include "DNServerPacketMaster.h"
#include "DNServerPacketVillage.h"
#include "DNServerPacketCash.h"
#endif // #if defined( _SERVER )
#include "DNServerProtocol.h"

using namespace std;

#pragma warning( disable:4819 )
#pragma warning( disable:4996 )

// Portable Server 3D Math (EtMathPortable)
#if defined(_VILLAGESERVER) || defined(_GAMESERVER)
#include "EtMathPortable.h"
#endif // _VILLAGESERVER, _GAMESERVER

#pragma comment(lib,"winmm.lib")

// Network Link
#pragma comment(lib,"ws2_32.lib")
#pragma comment(lib,"Mswsock.lib")
#pragma comment(lib,"advapi32.lib")
#pragma comment(lib, "Gdi32.lib")

#pragma comment(lib, "user32.lib")
