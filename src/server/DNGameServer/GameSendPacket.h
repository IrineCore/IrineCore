#pragma once

#include "MATransAction.h"
#include "DNPacket.h"
#include "RUDPCommonDef.h"
#include "DNUserSession.h"
#include "DnPlayerActor.h"

extern TGameConfig g_Config;

inline void SendConnectedGameServer(CDNUserSession * pSession, UINT nSessionID)
{
	SCConnectedResult packet;
	packet.nSessionID = nSessionID;
	_strcpy(packet.szServerVersion, _countof(packet.szServerVersion), g_Config.szVersion, (int)strlen(g_Config.szVersion));
	pSession->SendPacket( SC_SYSTEM, eSystem::SC_CONNECTED_RESULT, &packet, sizeof(packet), _RELIABLE);
}

inline void SendGameActorBundleMsg(CDNUserSession *pSession, USHORT nCount, BYTE *pBuf, int nSize, int nPrior )
{
	SCActorBundleMessage packet;

	packet.nSessionID = pSession->GetSessionID();
	memcpy(packet.cBuf, pBuf, nSize);

	pSession->SendPacket( SC_ACTORBUNDLE, nCount, &packet, sizeof(packet) - sizeof(packet.cBuf) + nSize, nPrior );
}

inline void SendGameSyncWait(CDNUserSession * pSession, UINT nSessionID )
{
	SCSyncWait packet;

	packet.nSessionID = nSessionID;
	
	pSession->SendPacket( SC_ROOM, eRoom::SC_SYNC_WAIT, &packet, sizeof(packet), _RELIABLE);
}

inline void SendGameSyncStart(CDNUserSession * pSession)
{
	SCSyncStart packet;

	packet.nSessionID = pSession->GetSessionID();

	pSession->SendPacket( SC_ROOM, eRoom::SC_SYNC_START, &packet, sizeof(packet), _RELIABLE);

#ifdef _DEBUG
	g_Log.Log(LogType::_NORMAL, pSession->GetWorldSetID(), pSession->GetAccountDBID(), pSession->GetCharacterDBID(), pSession->GetSessionID(), L"Send StartSync CharName:[%s]\n", pSession->GetCharacterName());
#endif
}

inline void SendGameDatumTick( CDNUserSession* pSession )
{
	_ASSERT( pSession );
	CDnPlayerActor* pPlayer = pSession->GetPlayerActor();
	_ASSERT( pPlayer );
	pPlayer->SetSyncDatumTick();
	pPlayer->SetSyncDatumSendTick();
	
	pSession->SendPacket( SC_ROOM, eRoom::SC_SYNC_DATUMTICK, NULL, 0, _RELIABLE );
}

inline void SendRefreshPartyMember(CDNUserSession * pSession, UINT * pMember, int nCount)
{
	SCRefreshMember packet;
	memset(&packet, 0, sizeof(packet));

	packet.cCount = (BYTE)nCount;
	memcpy(&packet.nMemberSessionID, pMember, sizeof(UINT) * nCount);

	pSession->SendPacket( SC_ROOM, eRoom::SC_REFRESH_MEMBER, &packet, sizeof(packet) - sizeof(packet.nMemberSessionID) + (sizeof(UINT) * nCount), _RELIABLE);
}

inline void SendGamePropMsg(CDNUserSession * pSession, UINT nUID, USHORT nProtocol, BYTE *pBuf, int nSize)
{
	SCPropMessage packet;

	packet.nUniqueID = nUID;
	memcpy(packet.cBuf, pBuf, nSize);

	pSession->SendPacket( SC_PROP, nProtocol, &packet, sizeof(packet) - sizeof(packet.cBuf) + nSize, _RELIABLE);
}

inline void SendGameCreateDropItem(CDNUserSession * pSession, UINT nUID, EtVector3 &vPos, UINT nItemID, UINT nRandomSeed, char cOption, USHORT nCount, USHORT nRotate, UINT nOwnerSessionID )
{
	SCCreateDropItem packet;

	packet.nSessionID = nUID;
	packet.fpos[0] = vPos.x;
	packet.fpos[1] = vPos.y;
	packet.fpos[2] = vPos.z;
	packet.nItemID = nItemID;
	packet.nRandomSeed = nRandomSeed;
	packet.nCount = nCount;
	packet.nRotate = nRotate;
	packet.nOwnerSessionID = nOwnerSessionID;
	packet.cOption = cOption;

	pSession->SendPacket( SC_ITEM, eItem::SC_CREATE_DROPITEM, &packet, sizeof(packet), _RELIABLE);
}

inline void SendGameDropItemList( CDNUserSession* pSession, EtVector3& vPos, DNVector(CDnItem::RealDropItemStruct)& vDropItem )
{
	SCCreateDropItemList TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	for( UINT i=0 ; i<vDropItem.size() ; ++i )
	{
		TxPacket.sDropItem[TxPacket.nCount].nSessionID		= vDropItem[i].dwUniqueID;
		TxPacket.sDropItem[TxPacket.nCount].fpos[0]			= vPos.x;
		TxPacket.sDropItem[TxPacket.nCount].fpos[1]			= vPos.y;
		TxPacket.sDropItem[TxPacket.nCount].fpos[2]			= vPos.z;
		TxPacket.sDropItem[TxPacket.nCount].nItemID			= vDropItem[i].nItemID;
		TxPacket.sDropItem[TxPacket.nCount].nRandomSeed		= vDropItem[i].nSeed;
		TxPacket.sDropItem[TxPacket.nCount].nCount			= vDropItem[i].nCount;
		TxPacket.sDropItem[TxPacket.nCount].nRotate			= vDropItem[i].nRotate;
		TxPacket.sDropItem[TxPacket.nCount].nOwnerSessionID = vDropItem[i].nOwnerSessionID;
		TxPacket.sDropItem[TxPacket.nCount].cOption			= vDropItem[i].cOption;

		++TxPacket.nCount;
		if( TxPacket.nCount >= _countof(TxPacket.sDropItem) )
		{
			// Send
			pSession->SendPacket( SC_ITEM, eItem::SC_CREATE_DROPITEMLIST, &TxPacket, sizeof(TxPacket), _RELIABLE );
			// Reset
			TxPacket.nCount = 0;
		}
	}

	if( TxPacket.nCount )
	{
		// Send
		pSession->SendPacket( SC_ITEM, eItem::SC_CREATE_DROPITEMLIST, &TxPacket, sizeof(TxPacket)-sizeof(TxPacket.sDropItem)+sizeof(SCCreateDropItem)*TxPacket.nCount, _RELIABLE );
	}
}

inline void SendGameDropItemMsg(CDNUserSession * pSession, UINT nUID, USHORT nProtocol, BYTE *pBuf, int nSize )
{
	SCDropItem packet;

	packet.nSessionID = nUID;
	packet.nItemProtocol= nProtocol;
	memcpy(packet.cBuf, pBuf, nSize);

	pSession->SendPacket(SC_ITEM, eItem::SC_DROPITEM_MSG, &packet, sizeof(packet) - sizeof(packet.cBuf) + nSize, _RELIABLE);
}

inline void SendGameGenerationMonsterMsg( CDNUserSession *pSession, int nSeed, UINT nMonsterID, 
										 UINT nUniqueID, EtVector3 &vPos, EtVector3 &vVel, EtVector3 &vResistance, 
										 int nEventAreaUniqueID, int nTeam, bool bRandomFrameSummon, EtQuat* pqRotation )
{
	SCGenerationMonster packet;

	packet.nSeed = nSeed;
	packet.nMonsterID = nMonsterID;
	packet.nSessionID = nUniqueID;
	packet.vPos = vPos;
	packet.vVel = vVel;
	packet.vResistance = vResistance;
	packet.nEventAreaUniqueID = nEventAreaUniqueID;
	packet.nTeam = nTeam;
	packet.bRandomFrameSummon = bRandomFrameSummon;

	if( pqRotation )
	{
		packet.bUseRotationQuat = true;
		packet.qRotation = *pqRotation;
	}
	else
		packet.bUseRotationQuat = false;

	pSession->SendPacket(SC_ROOM, eRoom::SC_GENERATION_MONSTER_MSG, &packet, sizeof(packet), _RELIABLE);
}

inline void SendGameGateInfo( CDNUserSession *pSession, char cCount, char *pGateList, char *pPermitFlag )
{
	SCGateInfoMessage packet;

	packet.cCount = cCount;
	memcpy( packet.cGateIndex, pGateList, sizeof(char) * cCount );
	memcpy( packet.cPermitFlag, pPermitFlag, sizeof(char) * cCount );
	pSession->SendPacket(SC_ROOM, eRoom::SC_GATEINFO_MSG, &packet, sizeof(packet), _RELIABLE);
}

inline void SendGameGateEjectInfo( CDNUserSession *pSession, char cGateIndex, char cLessLevelCount, char cNotEnoughItemCount,char cExceedCount, char cNotRideVehicleCount , char *pActorIndex )
{
	SCGateEjectInfo packet;

	packet.cGateIndex = cGateIndex;
	packet.cLessLevelCount = cLessLevelCount;
	packet.cNotEnoughItemCount = cNotEnoughItemCount;
	packet.cExceedCount = cExceedCount;
	packet.cNotRideVehicleCount = cNotRideVehicleCount;

	int nCount = 0;
	if( cLessLevelCount > 0 ) {
		memcpy( packet.cActorIndex, pActorIndex, cLessLevelCount * sizeof(char) );
		nCount += cLessLevelCount;
	}
	if( cNotEnoughItemCount > 0 ) {
		int nOffset = nCount * sizeof(char);
		memcpy( packet.cActorIndex + nOffset, pActorIndex + nOffset, cNotEnoughItemCount * sizeof(char) );
		nCount += cNotEnoughItemCount;
	}
	if( cExceedCount > 0 ) {
		int nOffset = nCount * sizeof(char);
		memcpy( packet.cActorIndex + nOffset, pActorIndex + nOffset, cExceedCount * sizeof(char) );
		nCount += cExceedCount;
	}
	if( cNotRideVehicleCount > 0 ) {
		int nOffset = nCount * sizeof(char);
		memcpy( packet.cActorIndex + nOffset, pActorIndex + nOffset, cNotRideVehicleCount * sizeof(char) );
		nCount += cNotRideVehicleCount;
	}

	pSession->SendPacket(SC_ROOM, eRoom::SC_GATEEJECTINFO_MSG, &packet, sizeof(packet) - sizeof(packet.cActorIndex) + ( sizeof(char) * nCount ), _RELIABLE);
}

inline void SendGameDungeonClear( CDNUserSession *pSession, int nDungeonClearTableID, DWORD dwClearTime, char cCurLevel, int nCurExp, bool bIgnoreRewardItem, char cCount, std::vector<TDungeonClearInfo> & vInfo )
{
	SCDungeonClear packet;

	packet.cCount = cCount;
	packet.nDungeonClearTableID = nDungeonClearTableID;
	packet.nClearTime = dwClearTime;
	packet.cCurLevel = cCurLevel;
	packet.nCurExp = nCurExp;
	packet.bIgnoreRewardItem = bIgnoreRewardItem;

	for( DWORD itr = 0; itr < vInfo.size(); ++itr )
		memcpy( &packet.Info[itr], &vInfo[itr], sizeof(TDungeonClearInfo) );

	pSession->SendPacket(SC_ROOM, eRoom::SC_DUNGEONCLEAR_MSG, &packet, sizeof(packet) - sizeof(packet.Info) + ( sizeof(TDungeonClearInfo) * cCount ), _RELIABLE);
}

inline void SendGameDungeonClearSelectRewardItem( CDNUserSession *pSession )
{
	pSession->SendPacket(SC_ROOM, eRoom::SC_DUNGEONCLEAR_SELECTREWARDITEM, NULL, 0, _RELIABLE);
}

#if defined( PRE_ADD_NAMEDITEM_SYSTEM )
inline void SendGameDungeonClearRewardBoxType( CDNUserSession *pSession, char cCount, TRewardBoxTypeStruct *pInfo )
{
	SCDungeonClearBoxType packet;
	memset(&packet,0,sizeof(packet));

	packet.cCount = cCount;
	memcpy( packet.Info, pInfo, sizeof(TRewardBoxTypeStruct) * cCount );

	pSession->SendPacket(SC_ROOM, eRoom::SC_DUNGEONCLEAR_REWARDBOXTYPE, &packet, sizeof(packet) - sizeof(packet.Info) + ( sizeof(TRewardBoxTypeStruct) * cCount ), _RELIABLE);
}
#endif

inline void SendGameDungeonClearRewardItem( CDNUserSession *pSession, char cCount, TRewardItemStruct *pInfo )
{
	SCDungeonClearRewardItem packet;

	packet.cCount = cCount;
	memcpy( packet.Info, pInfo, sizeof(TRewardItemStruct) * cCount );

	pSession->SendPacket(SC_ROOM, eRoom::SC_DUNGEONCLEAR_REWARDITEM, &packet, sizeof(packet) - sizeof(packet.Info) + ( sizeof(TRewardItemStruct) * cCount ), _RELIABLE);
}

inline void SendGameDungeonClearRewardItemResult( CDNUserSession *pSession, char cCount, TRewardItemResultStruct *pInfo )
{
	SCDungeonClearRewardItemResult packet;

	packet.cCount = cCount;
	memcpy( packet.Info, pInfo, sizeof(TRewardItemResultStruct) * cCount );
	pSession->SendPacket(SC_ROOM, eRoom::SC_DUNGEONCLEAR_REWARDITEMRESULT, &packet, sizeof(packet) - sizeof(packet.Info) + ( sizeof(TRewardItemResultStruct) * cCount ), _RELIABLE );
}

inline void SendGameWarpDungeonClear( CDNUserSession *pSession )
{
	SCWarpDungeonClear packet;
#if defined(PRE_ADD_RETURN_VILLAGE_SHOWNAME)
	packet.nLastVillageMapIndex = pSession->GetLastVillageMapIndex();
#endif	// #if defined(PRE_ADD_RETURN_VILLAGE_SHOWNAME)
	pSession->SendPacket( SC_ROOM, eRoom::SC_WARP_DUNGEONCLEAR, &packet, sizeof(packet), _RELIABLE );
}

inline void SendGameDLDungeonClear( CDNUserSession *pSession, int nClearRound, DWORD dwClearTime, bool bClear, char cCount, TDLDungeonClearInfo *pInfo )
{
	SCDLDungeonClear packet;

	packet.cCount = cCount;
	packet.nClearRound = nClearRound;
	packet.nClearTime = dwClearTime;
	packet.bClear = bClear;
	memcpy( packet.Info, pInfo, sizeof(TDLDungeonClearInfo) * cCount );
	pSession->SendPacket(SC_ROOM, eRoom::SC_DLDUNGEONCLEAR_MSG, &packet, sizeof(packet) - sizeof(packet.Info) + ( sizeof(TDLDungeonClearInfo) * cCount ), _RELIABLE);
}

inline void SendGameDLRankResult( CDNUserSession *pSession, TDLRankHistoryPartyInfo *pCurrentScore, TDLRankHistoryPartyInfo *pTopScore, TDLRankHistoryPartyInfo *pHistoryScore )
{
	SCDLRankInfo packet;

	memset( &packet, 0, sizeof(packet) );

	packet.CurScore = *pCurrentScore;
	packet.TopScore = *pTopScore;
	memcpy( packet.HistoryScore, pHistoryScore, sizeof(TDLRankHistoryPartyInfo) * DarkLair::Rank::SelectTop );

	pSession->SendPacket(SC_ROOM, eRoom::SC_DLDUNGEONCLEAR_RANKINFO, &packet, sizeof(packet), _RELIABLE);
}

inline void SendGameDLChallengeRequest( CDNUserSession *pSession )
{
	pSession->SendPacket( SC_ROOM, eRoom::SC_DLCHALLENGE_REQUEST, NULL, 0, _RELIABLE );
}

inline void SendGameDLChallengeResponse( CDNUserSession *pSession, bool bAccept )
{
	SCDLChallengeResponse TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.bAccept = bAccept;

	pSession->SendPacket( SC_ROOM, eRoom::SC_DLCHALLENGE_RESPONSE, &TxPacket, sizeof(TxPacket), _RELIABLE );
}

#if defined(PRE_MOD_DARKLAIR_RECONNECT)
inline void SendGameDLRoundInfo( CDNUserSession *pSession, int nCurrentRound, int nTotalRound, bool bBossRound )
{
	SCDLRoundInfo TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.nCurrentRound = nCurrentRound;
	TxPacket.nTotalRound = nTotalRound;
	TxPacket.bBossRound = bBossRound;

	pSession->SendPacket( SC_ROOM, eRoom::SC_DLROUND_INFO, &TxPacket, sizeof(TxPacket), _RELIABLE );
}
#endif

inline void SendGameSyncDungeonTimeAttack( CDNUserSession* pSession, int iSec, int iOrgSec )
{
	if( pSession == NULL )
		return;

	SCSyncDungeonTimeAttack TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.iRemainSec = iSec;
	TxPacket.iOrgSec = iOrgSec;

	pSession->SendPacket( SC_ROOM, eRoom::SC_SYNC_DUNGEONTIMEATTACK, &TxPacket, sizeof(TxPacket), _RELIABLE );
}

inline void SendGameStopDungeonTimeAttack( CDNUserSession* pSession )
{
	if( pSession == NULL )
		return;

	pSession->SendPacket( SC_ROOM, eRoom::SC_STOP_DUNGEONTIMEATTACK, NULL, 0, _RELIABLE );
}

inline void SendGameDungeonWarpOutFailed(CDNUserSession* pSession, int errCode)
{
	SCDungeonClearWarpOutFail packet;
	packet.nRet = errCode;
	pSession->SendPacket(SC_ROOM, eRoom::SC_DUNGEONCLEAR_CANNOTWARP, &packet, sizeof(packet), _RELIABLE);
}

inline void SendGameEnableDungeonClearLeaderWarp(CDNUserSession* pSession, bool bEnable)
{
	SCDungeonClearLeaderWarp packet;
	packet.bEnable = bEnable;

	pSession->SendPacket(SC_ROOM, eRoom::SC_DUNGEONCLEAR_ENABLE_LEADERWARP, &packet, sizeof(packet), _RELIABLE);
}

inline void SendGameDungeonFailed( CDNUserSession *pSession, bool bCanWarpWorldMap, bool bTimeOut=false )
{
	SCDungeonFailed packet;
	packet.bCanWarpWorldMap = bCanWarpWorldMap;
	packet.bTimeOut = bTimeOut;
#if defined(PRE_ADD_RETURN_VILLAGE_SHOWNAME)
	packet.nLastVillageMapIndex = pSession->GetLastVillageMapIndex();
#endif	// #if defined(PRE_ADD_RETURN_VILLAGE_SHOWNAME)

	pSession->SendPacket(SC_ROOM, eRoom::SC_DUNGEONFAILED_MSG, &packet, sizeof(packet), _RELIABLE);
}

inline void SendGameSelectRewardItem( CDNUserSession *pSession, UINT nApplySessionID, char cItemIndex )
{
	SCSelectRewardItem packet;
	packet.nSessionID = nApplySessionID;
	packet.cItemIndex = cItemIndex;

	pSession->SendPacket( SC_ROOM, eRoom::SC_SELECT_REWARDITEM, &packet, sizeof(packet), _RELIABLE );
}

inline void SendGameIdentifyRewardItem( CDNUserSession *pSession, char cItemIndex, char cBoxType )
{
	SCIdentifyRewardItem packet;
	packet.cItemIndex = cItemIndex;
	packet.cBoxType = cBoxType;

	pSession->SendPacket( SC_ROOM, eRoom::SC_DUNGEONCLEAR_IDENTIFYREWARDITEM, &packet, sizeof(packet), _RELIABLE );
}

inline void SendGameOpenDungeonLevel( CDNUserSession *pSession, int nMapIndex, char cDifficulty )
{
	SCOpenDungeonOpenLevel packet;

	packet.nMapIndex = nMapIndex;
	packet.cDifficulty = cDifficulty;

	pSession->SendPacket(SC_ROOM, eRoom::SC_OPENDUNGEONLEVEL_MSG, &packet, sizeof(packet), _RELIABLE );
}

inline void SendGameChangeGateState( CDNUserSession *pSession, char cGateIndex, int nPermitFlag )
{
	SCChangeGateState packet;

	packet.cGateInde = cGateIndex;
	packet.nPermitFlag = nPermitFlag;
	pSession->SendPacket(SC_ROOM, eRoom::SC_CHANGE_GATESTATE_MSG, &packet, sizeof(packet), _RELIABLE);
}

inline void SendGameGenerationPropMsg( CDNUserSession *pSession, int nSummonPropTableID, int nUniqueID, int nSeed, EtVector3 &vPosition, EtVector3 &vRotate, EtVector3 &vScale )
{
	SCGenerationProp packet;

	packet.nSummonPropTableID = nSummonPropTableID;
	packet.nPropUniqueID = nUniqueID;
	packet.nSeed = nSeed;
	packet.vPos = vPosition;
	packet.vRotate = vRotate;
	packet.vScale = vScale;

	pSession->SendPacket( SC_ROOM, eRoom::SC_GENERATION_PROP_MSG, &packet, sizeof(packet), _RELIABLE );
}

inline void SendGameChangeGameSpeed( CDNUserSession *pSession, float fPlaySpeed, DWORD dwDelay )
{
	SCChangeGameSpeed packet;

	packet.fSpeed = fPlaySpeed;
	packet.dwDelay = dwDelay;

	pSession->SendPacket( SC_ROOM, eRoom::SC_CHANGE_GAMESPEED, &packet, sizeof(packet), _RELIABLE );
};

inline void SendItemDisjointRes( CDNUserSession* pSession, int nItemID, bool bSuccess )
{
	SCItemDisjointRes Packet;
	Packet.nItemID = nItemID;
	Packet.bSuccess = bSuccess;

	pSession->SendPacket( SC_ITEM, eItem::SC_DISJOINT_RES, &Packet, sizeof(SCItemDisjointRes), _RELIABLE );
}

inline void SendActorShadowMsg(CDNUserSession * pSession, UINT nUID, USHORT nProtocol, BYTE *pBuf, int nSize)
{
	SCActorShadowMessage Packet;
	ZeroMemory( &Packet, sizeof(Packet) );

	Packet.nSessionID = nUID;
	memcpy(Packet.cBuf, pBuf, nSize);

	pSession->SendPacket(SC_ACTOR, nProtocol, &Packet, sizeof(Packet) - sizeof(Packet.cBuf) + nSize, _FAST);
}

inline void SendRoomSeqLevel(CDNUserSession * pSession, BYTE cSeqLevel)
{
	SCRoomSyncSeqLevel packet;
	memset(&packet, 0, sizeof(SCRoomSyncSeqLevel));

	packet.cSeqLevel = cSeqLevel;

	pSession->SendPacket(SC_ROOM, eRoom::SC_ROOMSYNC_SEQLEVEL, (char*)&packet, sizeof(packet), _RELIABLE);
}

#if defined(PRE_ADD_CP_RANK)
inline void SendAbyssStageClearBest(CDNUserSession* pSession, const TStageClearBest* sLegendClearBest, const TStageClearBest* sMonthlyClearBest)
{
	SCAbyssStageClearBest Packet;
	memset(&Packet, 0, sizeof(Packet));

	if( sLegendClearBest )
		memcpy(&Packet.sLegendClearBest, sLegendClearBest, sizeof(Packet.sLegendClearBest));
	if( sMonthlyClearBest )
		memcpy(&Packet.sMonthlyClearBest, sMonthlyClearBest, sizeof(Packet.sMonthlyClearBest));

	Packet.nMyBestCP = pSession->GetStageClearBestCP();
	Packet.cMyBestRank = pSession->GetStageClearBestRank();

	pSession->SendPacket(SC_ROOM, eRoom::SC_ABYSS_STAGE_CLEAR_BEST, (char*)&Packet, sizeof(Packet), _RELIABLE);
};
#endif //#if defined(PRE_ADD_CP_RANK)