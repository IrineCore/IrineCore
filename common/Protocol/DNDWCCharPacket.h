#pragma once

#include "DNPacket.h"

#pragma pack(push, 1)

struct CSCreateDWCChar
{
	WCHAR wszCharacterName[NAMELENMAX];
	int nEquipArray[EQUIPMAX];
	BYTE cClass;
	BYTE cJobCode1;
	BYTE cJobCode2;
	BYTE cLevel;
	int nExp;
	int nMapID;
	int nCoin;
	int nSkillPoint;
	DWORD dwHairColor;
	DWORD dwEyeColor;
	DWORD dwSkinColor;
	BYTE cWorldID;
};

#pragma pack(pop)
