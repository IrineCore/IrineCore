#pragma once

#include "ClientSocket.h"
#include "DNPacket.h"
#include "DNDWCCharPacket.h"
#include "ClientSessionManager.h"


inline void SendCheckVersion( BYTE cNation, BYTE cVersion, bool bCheck, USHORT nMajorVersion, USHORT nMinorVersion )
{
	FUNC_LOG();
	CSCheckVersion Check;
	memset(&Check, 0, sizeof(CSCheckVersion));

	Check.cNation = cNation;

	Check.cVersion = cVersion;
	Check.bCheck = bCheck;
	Check.nMajorVersion = nMajorVersion;
	Check.nMinorVersion = nMinorVersion;

	CClientSessionManager::GetInstance().SendPacket(CS_LOGIN, eLogin::CS_CHECKVERSION, (char*)&Check, sizeof(Check));
}

#if defined(PRE_MOD_SG_WITH_MACADDR)
#include "NetAdapters.h"
#endif		//#if defined (PRE_MOD_SG_WITH_MACADDR)
inline void SendLogin( const WCHAR *pId, const WCHAR *pPwd, const WCHAR *pIp )
{
	FUNC_LOG();

	CSCheckLogin Login;
	memset(&Login, 0, sizeof(CSCheckLogin));

	_wcscpy( Login.wszUserId, _countof(Login.wszUserId), pId, (int)wcslen(pId) );
	_wcscpy( Login.wszPassword, _countof(Login.wszPassword), pPwd, (int)wcslen(pPwd) );
	_wcscpy( Login.wszVirtualIp, _countof(Login.wszVirtualIp), pIp, (int)wcslen(pIp) );

	CClientSessionManager::GetInstance().SendPacket(CS_LOGIN, eLogin::CS_CHECKLOGIN, (char*)&Login, sizeof(CSCheckLogin));
	memset( &Login, 0, sizeof(CSCheckLogin));
}

inline void SendServerList()
{
	FUNC_LOG();
	//CClientSessionManager::GetInstance().GetTcpSocket()->AddSendData(CS_LOGIN, eLogin::CS_SERVERLIST, NULL, 0);
	CClientSessionManager::GetInstance().SendPacket(CS_LOGIN, eLogin::CS_SERVERLIST, NULL, 0);
}

#if defined(PRE_MOD_SELECT_CHAR)
inline void SendSelectChar(INT64 biCharacterDBID, int nSeed, int nValue[], bool isIgnoreAuthNotify)
#else	// #if defined(PRE_MOD_SELECT_CHAR)
inline void SendSelectChar(BYTE cCharIndex , int nSeed, int nValue[], bool isIgnoreAuthNotify )
#endif	// #if defined(PRE_MOD_SELECT_CHAR)
{
	FUNC_LOG();
	
	CSSelectChar Select;
	
	memset(&Select, 0, sizeof(CSSelectChar));

#if defined(PRE_MOD_SELECT_CHAR)
	Select.biCharacterDBID = biCharacterDBID;
#else	// #if defined(PRE_MOD_SELECT_CHAR)
	Select.cCharIndex = cCharIndex;
#endif	// #if defined(PRE_MOD_SELECT_CHAR)
	Select.nSeed = nSeed;
	memcpy( Select.nValue, nValue, sizeof(Select.nValue) );

#if defined(PRE_ADD_23829)
	Select.bSetIgnoreAuthNotify = isIgnoreAuthNotify;
#endif
	
	//CClientSessionManager::GetInstance().GetTcpSocket()->AddSendData(CS_LOGIN, eLogin::CS_SELECTCHAR, (char*)&Select, sizeof(CSSelectChar));
	CClientSessionManager::GetInstance().SendPacket(CS_LOGIN, eLogin::CS_SELECTCHAR, (char*)&Select, sizeof(CSSelectChar));

	memset(&Select, 0, sizeof(CSSelectChar));
#if defined(PRE_MOD_SELECT_CHAR)
	OutputDebug( "SendSelectChar, Character DBID : %I64d\n", biCharacterDBID );
#else	// #if defined(PRE_MOD_SELECT_CHAR)
	OutputDebug( "SendSelectChar, Character Index : %d\n", cCharIndex );
#endif	// #if defined(PRE_MOD_SELECT_CHAR)
}

#if defined(PRE_MOD_SELECT_CHAR)
inline void SendCreateChar(int *EquipArr, WCHAR *pName, DWORD dwHairColor, DWORD dwEyeColor, DWORD dwSkinColor, BYTE cClass, BYTE cWorldID,bool bIsDark)
#else	// #if defined(PRE_MOD_SELECT_CHAR)
inline void SendCreateChar(int *EquipArr, WCHAR *pName, DWORD dwHairColor, DWORD dwEyeColor, DWORD dwSkinColor, BYTE cClass)
#endif	// #if defined(PRE_MOD_SELECT_CHAR)
{
	FUNC_LOG();
	CSCreateChar Create;
	memset(&Create, 0, sizeof(CSCreateChar));

	_wcscpy(Create.wszCharacterName, _countof(Create.wszCharacterName), pName, (int)wcslen(pName));
	for (int i = 0; i < EQUIPMAX; i++)
		Create.nEquipArray[i] = EquipArr[i];
	Create.cClass = cClass;
	Create.dwHairColor = dwHairColor;
	Create.dwEyeColor = dwEyeColor;
	Create.dwSkinColor = dwSkinColor;
#if defined(PRE_MOD_SELECT_CHAR)
	Create.cWorldID = cWorldID;
#endif	// #if defined(PRE_MOD_SELECT_CHAR)


	CClientSessionManager::GetInstance().SendPacket(CS_LOGIN, eLogin::CS_CREATECHAR, (char*)&Create, sizeof(CSCreateChar));
}

inline void SendCreateDWCChar(int *EquipArr, WCHAR *pName, DWORD dwHairColor, DWORD dwEyeColor, DWORD dwSkinColor,
							  BYTE cClass, BYTE cJobCode1, BYTE cJobCode2, BYTE cLevel, int nExp, int nMapID, int nCoin, int nSkillPoint, BYTE cWorldID)
{
	FUNC_LOG();
	CSCreateDWCChar Create;
	memset(&Create, 0, sizeof(CSCreateDWCChar));

	_wcscpy(Create.wszCharacterName, _countof(Create.wszCharacterName), pName, (int)wcslen(pName));
	for (int i = 0; i < EQUIPMAX; i++)
		Create.nEquipArray[i] = EquipArr[i];
	Create.cClass = cClass;
	Create.cJobCode1 = cJobCode1;
	Create.cJobCode2 = cJobCode2;
	Create.cLevel = cLevel;
	Create.nExp = nExp;
	Create.nMapID = nMapID;
	Create.nCoin = nCoin;
	Create.nSkillPoint = nSkillPoint;
	Create.dwHairColor = dwHairColor;
	Create.dwEyeColor = dwEyeColor;
	Create.dwSkinColor = dwSkinColor;
	Create.cWorldID = cWorldID;

	CClientSessionManager::GetInstance().SendPacket(CS_LOGIN, eLogin::CS_CREATEDWCCHAR, (char*)&Create, sizeof(CSCreateDWCChar));
}

#if defined(PRE_MOD_SELECT_CHAR)
inline void SendDeleteChar(INT64 biCharacterDBID, int nSeed, int nValue[])
#else	// #if defined(PRE_MOD_SELECT_CHAR)
inline void SendDeleteChar(BYTE cCharIndex , int nSeed, int nValue[] )
#endif	// #if defined(PRE_MOD_SELECT_CHAR)
{
	FUNC_LOG();
	CSDeleteChar Delete;
	memset(&Delete, 0, sizeof(CSDeleteChar));

#if defined(PRE_MOD_SELECT_CHAR)
	Delete.biCharacterDBID = biCharacterDBID;
#else	// #if defined(PRE_MOD_SELECT_CHAR)
	Delete.cCharIndex = cCharIndex;
#endif	// #if defined(PRE_MOD_SELECT_CHAR)
	Delete.nSeed = nSeed;
	memcpy( Delete.nValue, nValue, sizeof(Delete.nValue) );

	CClientSessionManager::GetInstance().SendPacket(CS_LOGIN, eLogin::CS_DELETECHAR, (char*)&Delete, sizeof(CSDeleteChar));
	memset(&Delete, 0, sizeof(CSDeleteChar));
}

#if defined(PRE_MOD_SELECT_CHAR)
inline void SendReviveDeleteChar(INT64 biCharacterDBID)
#else	// #if defined(PRE_MOD_SELECT_CHAR)
inline void SendReviveDeleteChar(BYTE cCharIndex)
#endif	// #if defined(PRE_MOD_SELECT_CHAR)
{
	FUNC_LOG();
	CSReviveChar Revive;
	memset(&Revive, 0, sizeof(CSReviveChar));

#if defined(PRE_MOD_SELECT_CHAR)
	Revive.biCharacterDBID = biCharacterDBID;
#else	// #if defined(PRE_MOD_SELECT_CHAR)
	Revive.cCharIndex = cCharIndex;
#endif	// #if defined(PRE_MOD_SELECT_CHAR)

	CClientSessionManager::GetInstance().SendPacket(CS_LOGIN, eLogin::CS_REVIVECHAR, (char*)&Revive, sizeof(CSReviveChar));
}

inline void SendSelectedServer( BYTE cWorldID )
{
	FUNC_LOG();
	CSSelectServer selectServer;
	memset(&selectServer, 0, sizeof(CSSelectServer));

	selectServer.cWorldID = cWorldID;

	CClientSessionManager::GetInstance().SendPacket(CS_LOGIN, eLogin::CS_SELECTSERVER, (char*)&selectServer, sizeof(CSSelectServer));
}

inline void SendSelectedChannel( int nChannelID, bool bSkipTutorial )
{
	FUNC_LOG();
	CSSelectChannel selectChannel;
	memset(&selectChannel, 0, sizeof(CSSelectChannel));

	selectChannel.nChannelID = nChannelID;

	CClientSessionManager::GetInstance().SendPacket(CS_LOGIN, eLogin::CS_SELECTCHANNEL, (char*)&selectChannel, sizeof(CSSelectChannel));
}

inline void SendBackButton()
{
	FUNC_LOG();
	CClientSessionManager::GetInstance().SendPacket(CS_LOGIN, eLogin::CS_BACKBUTTON, NULL, 0);
}

inline void SendBackButtonLogin()
{
	FUNC_LOG();
	CClientSessionManager::GetInstance().SendPacket(CS_LOGIN, eLogin::CS_BACKBUTTON_LOGIN, NULL, 0);
}

inline void SendReqChannelList()
{
	CClientSessionManager::GetInstance().SendPacket(CS_LOGIN, eLogin::CS_CHANNELLIST, NULL, 0);
	FUNC_LOG();
}

#ifdef PRE_ADD_DOORS
inline void SendDoorsMobileReqAuthKey()
{
	CClientSessionManager::GetInstance().SendPacket(CS_DOORS, eDoors::CS_GET_AUTHKEY, NULL, 0);
	FUNC_LOG();
}

inline void SendDoorsMobileReqCancelAuth()
{
	CClientSessionManager::GetInstance().SendPacket(CS_DOORS, eDoors::CS_CANCEL_AUTH, NULL, 0);
	FUNC_LOG();
}

inline void SendDoorsMobileReqAuthFlag()
{
	CClientSessionManager::GetInstance().SendPacket(CS_DOORS, eDoors::CS_GET_MOBILEAUTHFLAG, NULL, 0);
	FUNC_LOG();
}
#endif

inline void SendCharacterSlotCode(BYTE cSlotCode)
{
	FUNC_LOG();
	CSCharacterSortCode Packet;
	memset(&Packet, 0, sizeof(Packet));

	Packet.cCharacterSortCode = cSlotCode;

	CClientSessionManager::GetInstance().SendPacket(CS_LOGIN, eLogin::CS_CHARACTERSORTCODE, (char*)&Packet, sizeof(Packet));
}
