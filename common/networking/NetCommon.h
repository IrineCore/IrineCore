#pragma once

#include <windows.h>
#include <winsock2.h>
#include <winbase.h>
#include <process.h>
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <tchar.h>
#include <time.h>

#include <vector>
#include <queue>
#include <deque>
#include <list>
#include <map>
#include <string>
#include <utility>

#include "SundriesFunc.h"
#include "CriticalSection.h"

struct TUDP_HEADER 
{
	USHORT Len;
	USHORT Cmd;
	UINT   ConUID;		// Connection UID
	BYTE Mode;
};

/*
struct TSendPacketData
{
	USHORT Len;
	USHORT AckCount;
	USHORT State;
	DWORD SendTick;
};
*/

inline UINT GetPacketSequence()
{
	static UINT PacketSequence = 0;

	PacketSequence++;
	if (PacketSequence > 4000000000) PacketSequence = 0;
	return PacketSequence;
}