#include "stdafx.h"
#include "DNUserSendManager.h"
#include "DNGameDataManager.h"
#include "Log.h"
#include "Util.h"
#include "DNWorldUserState.h"
#include "DNDBConnectionManager.h"
#include "DNLogConnection.h"
#include "DNUserSession.h"
#include "DNGuildSystem.h"
#include "DNDBConnection.h"
#include <bitset>
#include "DNEvent.h"

#if defined(_VILLAGESERVER)
#include "DNFieldDataManager.h"
#include "DNPvPRoom.h"
#include "DNLadderRoom.h"
#include "DNLadderSystemManager.h"
extern TVillageConfig g_Config;

#include "DNGuildWarManager.h"
#include "DNGuildVillage.h"
#elif defined(_GAMESERVER) // #if defined(_VILLAGESERVER)
#include "DNGameRoom.h"
#include "DNUserTcpConnection.h"
extern TGameConfig g_Config;
#include "DNMasterConnectionManager.h"
#endif // #if defined(_VILLAGESERVER)

#include "MasterSystemCacheRepository.h"
#if defined( PRE_ADD_SECONDARY_SKILL )
#include "SecondarySkillRecipe.h"
#endif // #if defined( PRE_ADD_SECONDARY_SKILL )

#include "DNGesture.h"
#include "DNPeriodQuestSystem.h"
#include "TimeSet.h"
#include "DNGuildRecruitCacheRepository.h"
#if defined (PRE_ADD_BESTFRIEND)
#include "DNBestFriend.h"
#endif

CDNUserSendManager::CDNUserSendManager(CDNUserSession *pUserSession)
{
	m_pSession = pUserSession;
}

CDNUserSendManager::~CDNUserSendManager(void)
{
}

// System


void CDNUserSendManager::Send( int iMainCmd, int iSubCmd, char* pData, int iLen )
{
	if( !m_pSession )
		return;
	m_pSession->AddSendData( iMainCmd, iSubCmd, pData, iLen );
}
#if defined(PRE_MOD_DUPLICATION_LOGIN_MES)
void CDNUserSendManager::SendCompleteDetachMsg(int nRetCode, const WCHAR * pwszDetachReason, bool IsDisConnectByClient)
#else	// #if defined(PRE_MOD_DUPLICATION_LOGIN_MES)
void CDNUserSendManager::SendCompleteDetachMsg(int nRetCode, const WCHAR * pwszDetachReason)
#endif	// #if defined(PRE_MOD_DUPLICATION_LOGIN_MES)
{
	SCDetachMessage packet;
	memset(&packet, 0, sizeof(SCDetachMessage));

	packet.nRetCode = nRetCode;
#if defined(PRE_MOD_DUPLICATION_LOGIN_MES)
	packet.bDisConnectByClient = IsDisConnectByClient;
#endif		// #if defined(PRE_MOD_DUPLICATION_LOGIN_MES)
	if (!m_pSession)
		return;

	m_pSession->AddSendData(SC_SYSTEM, eSystem::SC_DETACHCONNECTION_MSG, (char*)&packet, sizeof(packet));
#ifdef _GAMESERVER
	if( m_pSession->GetTcpConnection() )
		m_pSession->GetTcpConnection()->DelayDetachConnection(const_cast<wchar_t*>(pwszDetachReason));
#else
	m_pSession->DelayDetachConnection(const_cast<wchar_t*>(pwszDetachReason));
#endif
}

void CDNUserSendManager::SendConnectVillage(int nRet, int nAttr, bool bWithOutLoading )
{
	SCConnectVillage ConnectVillage;
	memset(&ConnectVillage, 0, sizeof(SCConnectVillage));
	ConnectVillage.nRet = nRet;
	_strcpy(ConnectVillage.szServerVersion, _countof(ConnectVillage.szServerVersion), g_Config.szVersion, (int)strlen(g_Config.szVersion));
	ConnectVillage.nChannelAttr = nAttr;
	ConnectVillage.cWithoutLoading = bWithOutLoading == true ? 1 : 0;
	
	m_pSession->AddSendData(SC_SYSTEM, eSystem::SC_CONNECTVILLAGE, (char*)&ConnectVillage, sizeof(ConnectVillage));
}

void CDNUserSendManager::SendGameInfo( GameTaskType::eType GameTaskType, BYTE cReqGameIDType, ULONG nIP, USHORT nPort, USHORT nTcpPort, UINT nAccountDBID, INT64 biCertifyingKey)
{
	SCGameInfo TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.GameTaskType		= GameTaskType;
	TxPacket.cReqGameIDType		= cReqGameIDType;
	TxPacket.nGameServerIP		= nIP;
	TxPacket.nGameServerPort	= nPort;
	TxPacket.nGameServerTcpPort = nTcpPort;
	TxPacket.nAccountDBID = nAccountDBID;
	TxPacket.biCertifyingKey = biCertifyingKey;

	m_pSession->AddSendData( SC_SYSTEM, eSystem::SC_GAMEINFO, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket));
}

void CDNUserSendManager::SendReConnectReq()
{
	m_pSession->AddSendData(SC_SYSTEM, eSystem::SC_RECONNECT_REQ, NULL, 0);
}

void CDNUserSendManager::SendReconnectLogin(int nRet, UINT nAccountDBID, INT64 biCertifyingKey)
{
	SCReconnectLogin Reconnect = { 0, };
	Reconnect.nRet = nRet;
	Reconnect.nAccountDBID = nAccountDBID;
	Reconnect.biCertifyingKey = biCertifyingKey;

	m_pSession->AddSendData(SC_SYSTEM, eSystem::SC_RECONNECTLOGIN, (char*)&Reconnect, sizeof(SCReconnectLogin));
}

void CDNUserSendManager::SendCountDownMsg(bool bIsStart, int nCountTime, UINT nCode, BYTE msgboxType)
{
	SCCountDownMsg packet;
	memset(&packet, 0, sizeof(packet));

	packet.cIsStart = bIsStart == true ? 1 : 0;
	packet.nCountTime = nCountTime;
	packet.nCode = nCode;
	packet.msgboxType = msgboxType;

	m_pSession->AddSendData(SC_SYSTEM, eSystem::SC_COUNTDOWNMSG, (char*)&packet, sizeof(packet));
}

#if defined(PRE_ADD_VIP)
void CDNUserSendManager::SendVIPAutomaticPay(bool bAuto)
{
	SCVIPAutomaticPay System;
	System.bAutomaticPay = bAuto;
	m_pSession->AddSendData(SC_SYSTEM, eSystem::SC_VIPAUTOMATICPAY, (char*)&System, sizeof(System));
}
#endif	// #if defined(PRE_ADD_VIP)

// Char
void CDNUserSendManager::SendMapInfo(UINT nMapIndex, BYTE cMapArrayIndex, BYTE cEnviIndex, BYTE cEnviArrayIndex, int nChannelID, int nChannelIdx, int nMeritID )
{
	SCMapInfo MapInfo;
	memset(&MapInfo, 0, sizeof(SCMapInfo));

	MapInfo.nMapIndex = nMapIndex;
	MapInfo.cMapArrayIndex = cMapArrayIndex;
	MapInfo.cEnviIndex = cEnviIndex;
	MapInfo.cEnviArrayIndex = cEnviArrayIndex;
	MapInfo.nChannelID = nChannelID;
	MapInfo.wChannelIdx = nChannelIdx;
	if(nMeritID > 0)
	{
		const TMeritInfo * pMerit = g_pDataManager->GetMeritInfo(nMeritID);
		if(pMerit)
			memcpy(&MapInfo.MeritInfo, pMerit, sizeof(TMeritInfo));
	}

	m_pSession->AddSendData(SC_CHAR, eChar::SC_MAPINFO, (char*)&MapInfo, sizeof(MapInfo));
}

void CDNUserSendManager::SendEnter(CDNUserSession *pSession)
{
	SCEnter Enter;
	memset(&Enter, 0, sizeof(SCEnter));

#if defined(_VILLAGESERVER)
	Enter.Position.x = (float)(pSession->GetCurrentPos().nX / 1000.f);
	Enter.Position.y = (float)(pSession->GetCurrentPos().nY / 1000.f);
	Enter.Position.z = (float)(pSession->GetCurrentPos().nZ / 1000.f);
	Enter.fRotate = pSession->GetRotate();

	if( !pSession->m_bFirst )
		g_pFieldDataManager->GetStartPositionAngle( pSession->GetMapIndex(), pSession->GetLastVillageGateNo(), Enter.fRotate );

#elif defined(_GAMESERVER)
	Enter.Position.x = (float)(pSession->GetPosX() / 1000.f);
	Enter.Position.y = (float)(pSession->GetPosY() / 1000.f);
	Enter.Position.z = (float)(pSession->GetPosZ() / 1000.f);
	Enter.fRotate = 0;
#endif

	Enter.nVehicleObjectID = pSession->m_nVehicleObjectID;
	memcpy(Enter.wszCharacterName, pSession->GetCharacterName(), sizeof(Enter.wszCharacterName));
	Enter.cClassID = pSession->GetClassID();
	Enter.cAccountLevel = pSession->GetAccountLevel();	// Account level
	memcpy(&Enter.cJobArray, pSession->GetStatusData()->cJobArray, sizeof(Enter.cJobArray));
	Enter.cLevel = pSession->GetLevel();
	Enter.nExp = pSession->GetExp();
	Enter.nCoin = pSession->GetCoin();
	Enter.cRebirthCoin = pSession->GetRebirthCoin();
	Enter.cPCBangRebirthCoin = pSession->GetPCBangRebirthCoin();
	Enter.cPartyState = 1;
	Enter.wSkillPoint[DualSkill::Type::Primary] = pSession->GetSkillPoint(DualSkill::Type::Primary);
	Enter.wSkillPoint[DualSkill::Type::Secondary] = pSession->GetSkillPoint(DualSkill::Type::Secondary);
	Enter.nDefaultBody = pSession->GetDefaultBody();
	Enter.nDefaultLeg = pSession->GetDefaultLeg();
	Enter.nDefaultHand = pSession->GetDefaultHand();
	Enter.nDefaultFoot = pSession->GetDefaultFoot();
	Enter.dwHairColor = pSession->GetHairColor();
	Enter.dwEyeColor = pSession->GetEyeColor();
	Enter.dwSkinColor = pSession->GetSkinColor();
	Enter.nAppellation = pSession->GetSelectAppellation();
	Enter.nCoverAppellation = pSession->GetCoverAppellation();
	Enter.wFatigue = pSession->GetFatigue();
	Enter.wWeekFatigue = pSession->GetWeeklyFatigue();
	Enter.wPCBangFatigue = pSession->GetPCBangFatigue();
	Enter.wEventFatigue = pSession->GetEventFatigue();
	Enter.wMaxEventFatigue = pSession->GetMaxEventFatigue();
	Enter.wRebirthCashCoin = pSession->GetRebirthCashCoin();
#if defined(PRE_ADD_VIP)
	Enter.bVIP = pSession->IsVIP();
	Enter.cVIPRebirthCoin = pSession->GetVIPRebirthCoin();
	Enter.nVIPPoint = pSession->m_nVIPTotalPoint;
	Enter.wVIPFatigue = pSession->GetVIPFatigue();
	Enter.tVIPExpirationDate = pSession->m_tVIPEndDate;
	Enter.bVIPAutomaticPay = pSession->IsVIPAutoPay();
#endif	// #if defined(PRE_ADD_VIP)

	memcpy(Enter.cViewCashEquipBitmap, pSession->GetViewCashEquipBitmap(), sizeof(Enter.cViewCashEquipBitmap));
	// TPvPGroup
	memcpy( &Enter.sPvPInfo, pSession->GetPvPData(), sizeof(TPvPGroup) );
	// Guild
	Enter.GuildSelfView = pSession->GetGuildSelfView();

#if defined(_VILLAGESERVER)
	if(g_pGuildWarManager)
	{
		Enter.cCurrentGuildWarEventStep = g_pGuildWarManager->GetStepIndex();
		Enter.wCurrentGuldWarScheduleID = g_pGuildWarManager->GetScheduleID();
	}
	else
		_DANGER_POINT();
#endif // #if defined(_VILLAGESERVER)
	Enter.cSkillPage = pSession->GetSkillPage();
	Enter.cAge = pSession->GetAge();

#if defined( PRE_ADD_NAMEDITEM_SYSTEM )
	TItem* NameItem = pSession->GetItem()->GetItemRank(ITEMRANK_SSS);
	if(NameItem)
		Enter.nNamedItemID = NameItem->nItemID;
#endif

	m_pSession->AddSendData( SC_CHAR, eChar::SC_ENTER, (char*)&Enter, sizeof(SCEnter) );
}

void CDNUserSendManager::SendEnterUser(CDNUserSession *pSession)
{
#if defined(_VILLAGESERVER)
	SCEnterUser EnterUser;
	memset(&EnterUser, 0, sizeof(SCEnterUser));

	EnterUser.nSessionID = pSession->GetSessionID();
	EnterUser.nVehicleObjectID = pSession->m_nVehicleObjectID;
	memcpy(EnterUser.wszCharacterName, pSession->GetCharacterName(), sizeof(EnterUser.wszCharacterName));
	EnterUser.bHide = pSession->GetHide();
	EnterUser.cAccountLevel = pSession->GetAccountLevel();
	EnterUser.cClassID		= pSession->GetClassID();
	EnterUser.cLevel		= pSession->GetLevel();
	EnterUser.cPvPLevel		= pSession->GetPvPData()->cLevel;
	EnterUser.cJob			= pSession->GetUserJob();
	EnterUser.cPartyState	= 1;
	EnterUser.Position.x	= (float)(pSession->GetCurrentPos().nX / 1000.f);
	EnterUser.Position.y	= (float)(pSession->GetCurrentPos().nY / 1000.f);
	EnterUser.Position.z	= (float)(pSession->GetCurrentPos().nZ / 1000.f);
	EnterUser.fRotate		= pSession->GetRotate();
	EnterUser.bBattleMode	= pSession->IsBattleMode();
	EnterUser.nDefaultBody	= pSession->GetDefaultBody();
	EnterUser.nDefaultLeg	= pSession->GetDefaultLeg();
	EnterUser.nDefaultHand	= pSession->GetDefaultHand();
	EnterUser.nDefaultFoot	= pSession->GetDefaultFoot();
	EnterUser.dwHairColor	= pSession->GetHairColor();
	EnterUser.dwEyeColor	= pSession->GetEyeColor();
	EnterUser.dwSkinColor	= pSession->GetSkinColor();
	EnterUser.nAppellation	= pSession->GetSelectAppellation();
	EnterUser.nCoverAppellation = pSession->GetCoverAppellation();
	EnterUser.GuildSelfView = pSession->GetGuildSelfView();
	memcpy(EnterUser.cViewCashEquipBitmap, pSession->GetViewCashEquipBitmap(), sizeof(EnterUser.cViewCashEquipBitmap));
#if defined(PRE_ADD_VIP)
	EnterUser.bVIP			= pSession->IsVIP();
#endif	// #if defined(PRE_ADD_VIP)
	if( pSession->GetItem()->GetVehicleEquip() )
		EnterUser.VehicleEquip = *(pSession->GetItem()->GetVehicleEquip());
	if( pSession->GetItem()->GetPetEquip() )
		EnterUser.PetEquip = *(pSession->GetItem()->GetPetEquip());
	EnterUser.bIsPartyLeader = (pSession->GetPartyID() > 0 && pSession->m_boPartyLeader) ? true : false;

#if defined( PRE_ADD_NAMEDITEM_SYSTEM )
	// Client needs to show source items when creating actor in village task, so send here; if sent later before actor creation, client ignores it.
	TEffectSkillData* EffectSkill = pSession->GetItem()->GetEffectSkillItem( EffectSkillNameSpace::SearchType::ItemType, ITEMTYPE_SOURCE);
	if( EffectSkill )
		EnterUser.nSourceItemID = EffectSkill->nItemID;
	TItem* NameItem = pSession->GetItem()->GetItemRank(ITEMRANK_SSS);
	if(NameItem)
		EnterUser.nNamedItemID = NameItem->nItemID;
#else
	EnterUser.nSourceItemID = pSession->GetItem()->GetSource().nItemID;
#endif
#if defined (PRE_ADD_BESTFRIEND)
	EnterUser.biBestFriendItemSerial = pSession->GetBestFriend()->GetInfo().biItemSerial;
	WCHAR* pwszBFName = pSession->GetBestFriend()->GetInfo().wszName;
	if (pwszBFName)
		_wcscpy(EnterUser.wszBestFriendName, _countof(EnterUser.wszBestFriendName), pwszBFName, (int)wcslen(pwszBFName));
#endif
#if defined(PRE_ADD_TRANSFORM_POTION)
	EnterUser.nTransformID = pSession->GetTransformID();
#endif //#if defined(PRE_ADD_TRANSFORM_POTION)


#if defined(PRE_ADD_TALISMAN_SYSTEM)
	pSession->GetItem()->GetEquipList(EnterUser.cCount, EnterUser.cCashCount, EnterUser.cGlyphCount, EnterUser.cTalismanCount, EnterUser.EquipArray);
	m_pSession->AddSendData(SC_CHAR, eChar::SC_ENTERUSER, (char*)&EnterUser, (sizeof(SCEnterUser) - sizeof(EnterUser.EquipArray) +(sizeof(TItemInfo) *( EnterUser.cCount + EnterUser.cCashCount + EnterUser.cGlyphCount + EnterUser.cTalismanCount ) )));
#else
	pSession->GetItem()->GetEquipList(EnterUser.cCount, EnterUser.cCashCount, EnterUser.cGlyphCount, EnterUser.EquipArray);
	m_pSession->AddSendData(SC_CHAR, eChar::SC_ENTERUSER, (char*)&EnterUser, (sizeof(SCEnterUser) - sizeof(EnterUser.EquipArray) +(sizeof(TItemInfo) *( EnterUser.cCount + EnterUser.cCashCount + EnterUser.cGlyphCount ) )));
#endif

#endif	// #if defined(_VILLAGESERVER)
}

void CDNUserSendManager::SendLeaveUser(UINT nSessionID, char cType, int nAccountLevel)
{
	SCLeaveUser Leave;
	memset(&Leave, 0, sizeof(SCLeaveUser));
	Leave.nSessionID = nSessionID;
	Leave.cLeaveType = cType;
	Leave.cAccountLevel = nAccountLevel;
	m_pSession->AddSendData(SC_CHAR, eChar::SC_LEAVEUSER, (char*)&Leave, sizeof(SCLeaveUser));
}

void CDNUserSendManager::SendEnterNpc(UINT nNpcObjectID, int nNpcID, float x, float y, float z, float fRotate)
{
	SCEnterNpc EnterNpc;
	memset(&EnterNpc, 0, sizeof(SCEnterNpc));

	EnterNpc.nNpcObjectID = nNpcObjectID;
	EnterNpc.nNpcID = nNpcID;
	EnterNpc.Position.x = x;
	EnterNpc.Position.y = y;
	EnterNpc.Position.z = z;

	EnterNpc.fRotate = fRotate;

	m_pSession->AddSendData(SC_CHAR, eChar::SC_ENTERNPC, (char*)&EnterNpc, sizeof(SCEnterNpc));
}

void CDNUserSendManager::SendLeaveNpc(UINT nObjectID)
{
	SCLeaveNpc Leave;
	memset(&Leave, 0, sizeof(SCLeaveNpc));
	Leave.nObjectID = nObjectID;
	m_pSession->AddSendData(SC_CHAR, eChar::SC_LEAVENPC, (char*)&Leave, sizeof(SCLeaveNpc));
}

void CDNUserSendManager::SendEnterPropNpc(UINT nNpcObjectID, int nPropUniqueID)
{
	SCEnterPropNpc EnterNpc;
	memset(&EnterNpc, 0, sizeof(SCEnterPropNpc));

	EnterNpc.nNpcObjectID = nNpcObjectID;
	EnterNpc.nPropUniqueID = nPropUniqueID;

	m_pSession->AddSendData(SC_CHAR, eChar::SC_ENTERPROPNPC, (char*)&EnterNpc, sizeof(SCEnterPropNpc));
}

void CDNUserSendManager::SendLeavePropNpc(int nPropUniqueID)
{
	SCLeavePropNpc Leave;
	memset(&Leave, 0, sizeof(SCLeavePropNpc));
	Leave.nPropUniqueID = nPropUniqueID;
	m_pSession->AddSendData(SC_CHAR, eChar::SC_LEAVEPROPNPC, (char*)&Leave, sizeof(SCLeavePropNpc));
}

void CDNUserSendManager::SendQuickSlotList(TQuickSlot *QuickSlotArray)
{
	SCQuickSlotList Slot;
	memset(&Slot, 0, sizeof(SCQuickSlotList));

	int nCount = 0;
	for (int i = 0; i < QUICKSLOTMAX; i++){
		if(QuickSlotArray[i].nID <= 0) continue;
		Slot.QuickSlot[nCount].cIndex = i;
		Slot.QuickSlot[nCount].cType = QuickSlotArray[i].cType;
		Slot.QuickSlot[nCount].nID = QuickSlotArray[i].nID;
		nCount++;
	}
	Slot.cSlotCount = nCount;

	int nSize = sizeof(BYTE) +( sizeof(TQuickSlotEx) * nCount );
	m_pSession->AddSendData(SC_CHAR, eChar::SC_QUICKSLOTLIST, (char*)&Slot, nSize);
}

void CDNUserSendManager::SendFatigueResult(UINT nSessionID, int iResult)
{
	SCFatigue Fatigue = { 0, };
	Fatigue.nSessionID = nSessionID;
	Fatigue.iResult = iResult;

	m_pSession->AddSendData(SC_CHAR, eChar::SC_FATIGUE, (char*)&Fatigue, sizeof(SCFatigue));
}

void CDNUserSendManager::SendFatigue(UINT nSessionID, short wFatigue, short wWeekFatigue, short wPCBangFatigue, short wEventFatigue, short wVIPFatigue)
{
	SCFatigue Fatigue = { 0, };

	Fatigue.nSessionID = nSessionID;
	Fatigue.wFatigue = wFatigue;
	Fatigue.wWeekFatigue = wWeekFatigue;
	Fatigue.wPCBangFatigue = wPCBangFatigue;
	Fatigue.wEventFatigue = wEventFatigue;
#if defined(PRE_ADD_VIP)
	Fatigue.wVIPFatigue = wVIPFatigue;
#endif	// #if defined(PRE_ADD_VIP)

	m_pSession->AddSendData(SC_CHAR, eChar::SC_FATIGUE, (char*)&Fatigue, sizeof(SCFatigue));
}

void CDNUserSendManager::SendHide(UINT nSessionID, bool bHide)
{
	SCHide Hide = { 0, };

	Hide.nSessionID = nSessionID;
	Hide.bHide = bHide;

	m_pSession->AddSendData(SC_CHAR, eChar::SC_HIDE, (char*)&Hide, sizeof(SCHide));
}

void CDNUserSendManager::SendPCBang(char cPCBangGrade, MAPCBangResult *pPacket)
{
	SCPCBang PCBang = { 0, };
	PCBang.cPCBangGrade = cPCBangGrade;

	if (pPacket){
	}

	m_pSession->AddSendData(SC_CHAR, eChar::SC_PCBANG, (char*)&PCBang, sizeof(SCPCBang));
}

void CDNUserSendManager::SendNestClear(TNestClearData *NestClearList)
{
	SCNestClear Nest = { 0, };

	Nest.cCount = 0;
	for (int i = 0; i < NESTCLEARMAX; i++){
		if (NestClearList[i].nMapIndex <= 0) continue;

		Nest.NestClear[Nest.cCount] = NestClearList[i];
		Nest.cCount++;
	}

	int nSize = sizeof(SCNestClear) - sizeof(Nest.NestClear) +(sizeof(TNestClearData) * Nest.cCount);
	m_pSession->AddSendData(SC_CHAR, eChar::SC_NESTCLEAR, (char*)&Nest, nSize);
}

void CDNUserSendManager::SendUpdateNestClear(TNestClearData *pNestData)
{
	SCUpdateNestClear Nest = { 0, };

	if (pNestData)
		Nest.Update = *pNestData;

	m_pSession->AddSendData(SC_CHAR, eChar::SC_UPDATENESTCLEAR, (char*)&Nest, sizeof(SCUpdateNestClear));
}

void CDNUserSendManager::SendKeySetting( TKeySetting* pKeySetting )
{
	SCGameOptionSelectKeySetting TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.bIsDefault = true;

	for( UINT i=0 ;i<sizeof(TKeySetting) ; ++i ) 
	{
		char* pBuf = reinterpret_cast<char*>(pKeySetting)+i;
		if( *pBuf != 0 )
		{
			TxPacket.bIsDefault = false;
			break;
		}
	}

	int iSize = sizeof(TxPacket);
	if( !TxPacket.bIsDefault )
		memcpy( &TxPacket.sKeySetting, pKeySetting, sizeof(TKeySetting) );
	else
		iSize = sizeof(TxPacket)-sizeof(TxPacket.sKeySetting);

	m_pSession->AddSendData( SC_GAMEOPTION, eGameOption::SC_GAMEOPTION_SELECT_KEYSETTING, reinterpret_cast<char*>(&TxPacket), iSize );
}

void CDNUserSendManager::SendPadSetting( TPadSetting* pPadSetting )
{
	SCGameOptionSelectPadSetting TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.bIsDefault = true;

	for( UINT i=0 ;i<sizeof(TPadSetting) ; ++i ) 
	{
		char* pBuf = reinterpret_cast<char*>(pPadSetting)+i;
		if( *pBuf != 0 )
		{
			TxPacket.bIsDefault = false;
			break;
		}
	}

	int iSize = sizeof(TxPacket);
	if( !TxPacket.bIsDefault )
		memcpy( &TxPacket.sPadSetting, pPadSetting, sizeof(TPadSetting) );
	else
		iSize = sizeof(TxPacket)-sizeof(TxPacket.sPadSetting);

	m_pSession->AddSendData( SC_GAMEOPTION, eGameOption::SC_GAMEOPTION_SELECT_PADSETTING, reinterpret_cast<char*>(&TxPacket), iSize );
}
void CDNUserSendManager::SendPvPFatigueReward( int nGainExp, int nGainMedal )
{
	SCPVP_FATIGUE_REWARD TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.nGainExp = nGainExp;
	TxPacket.nGainMedal = nGainMedal;	

	m_pSession->AddSendData( SC_PVP, ePvP::SC_FATIGUE_REWARD, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}
void CDNUserSendManager::SendPvPLadderScoreInfo( const TPvPLadderScoreInfo* pLadderScoreInfo )
{
	LadderSystem::SC_SCOREINFO TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	int iSize = sizeof(TxPacket)-sizeof(TxPacket.Data.LadderScore)+pLadderScoreInfo->cLadderTypeCount*sizeof(TxPacket.Data.LadderScore[0]);
	if( iSize > sizeof(TxPacket) )
		return;
	memcpy( &TxPacket, pLadderScoreInfo, iSize );

	m_pSession->AddSendData( SC_PVP, ePvP::SC_LADDER_SCOREINFO, reinterpret_cast<char*>(&TxPacket), iSize );

}

void CDNUserSendManager::SendPvPLadderScoreInfoByJob( TAGetListPvPLadderScoreByJob* pA )
{
	LadderSystem::SC_SCOREINFO_BYJOB TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	int iSize = sizeof(TxPacket)-sizeof(TxPacket.Data);
	TxPacket.iRet = pA->nRetCode;
	if( TxPacket.iRet == ERROR_NONE )
	{
		int iAddSize = sizeof(pA->Data)-sizeof(pA->Data.LadderScoreByJob)+pA->Data.cJobCount*sizeof(pA->Data.LadderScoreByJob[0]);
		iSize += iAddSize;

		TxPacket.Data.cJobCount = pA->Data.cJobCount;
		for( int i=0 ; i<TxPacket.Data.cJobCount ; ++i )
			TxPacket.Data.LadderScoreByJob[i] = pA->Data.LadderScoreByJob[i];
	}

	m_pSession->AddSendData( SC_PVP, ePvP::SC_LADDER_SCOREINFO_BYJOB, reinterpret_cast<char*>(&TxPacket), iSize );

}

void CDNUserSendManager::SendPvPLadderPointRefresh( int iPoint )
{
	LadderSystem::SC_LADDERPOINT_REFRESH TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.iPoint= iPoint;

	m_pSession->AddSendData( SC_PVP, ePvP::SC_LADDER_POINT_REFRESH, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );

}

void CDNUserSendManager::SendPvPLadderInviteUser( int iRet, const WCHAR* pwszCharName )
{
	LadderSystem::SC_INVITE TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.iRet = iRet;
	_wcscpy( TxPacket.wszCharName, _countof(TxPacket.wszCharName), pwszCharName, (int)wcslen(pwszCharName) );

	m_pSession->AddSendData( SC_PVP, ePvP::SC_LADDER_INVITE, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );

}

void CDNUserSendManager::SendPvPLadderInviteConfirmReq( const WCHAR* pwszCharName, LadderSystem::MatchType::eCode MatchType, int iCurUserCount, int iAvgGradePoint )
{
	LadderSystem::SC_INVITE_CONFIRM_REQ TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	_wcscpy( TxPacket.wszCharName, _countof(TxPacket.wszCharName), pwszCharName, (int)wcslen(pwszCharName) );
	TxPacket.MatchType		= MatchType;
	TxPacket.iCurUserCount	= iCurUserCount;
	TxPacket.iAvgGradePoint	= iAvgGradePoint;

	m_pSession->AddSendData( SC_PVP, ePvP::SC_LADDER_INVITE_CONFIRM_REQ, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );

}

void CDNUserSendManager::SendPvPLadderInviteConfirm( int iRet, const WCHAR* pwszCharName )
{
	LadderSystem::SC_INVITE_CONFIRM TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.iRet = iRet;
	_wcscpy( TxPacket.wszCharName, _countof(TxPacket.wszCharName), pwszCharName, (int)wcslen(pwszCharName) );

	m_pSession->AddSendData( SC_PVP, ePvP::SC_LADDER_INVITE_CONFIRM, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );

}

void CDNUserSendManager::SendPvPLadderKickOut( int iRet )
{
	LadderSystem::SC_KICKOUT TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.iRet = iRet;

	m_pSession->AddSendData( SC_PVP, ePvP::SC_LADDER_KICKOUT, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );

}

void CDNUserSendManager::SendPvPGhoulScores( const TPvPGhoulScores* pGhoulSocres)
{
	SCPVP_GHOUL_SCORES TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	memcpy( &TxPacket, pGhoulSocres, sizeof(SCPVP_GHOUL_SCORES));

	m_pSession->AddSendData( SC_PVP, ePvP::SC_PVP_GHOULSCORES, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}
void CDNUserSendManager::SendPvPHolyWaterKillCount(UINT nSessionID ,  UINT nKillerSessionID )
{
	SCPVP_HOLYWATER_KILLCOUNT TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket));
	TxPacket.nSessionID = nSessionID;
	TxPacket.nKillerSessionID = nKillerSessionID;

	m_pSession->AddSendData( SC_PVP, ePvP::SC_PVP_HOLYWATER_KILLCOUNT, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
};

#ifdef PRE_ADD_COLOSSEUM_BEGINNER
void CDNUserSendManager::SendPvPChangeChannelResult(BYTE cType, int nRetCode)
{
	SCPVP_CHANGE_CHANNEL packet;
	memset(&packet, 0, sizeof(SCPVP_CHANGE_CHANNEL));

	packet.cType = cType;
	packet.nRetCode = nRetCode;

	m_pSession->AddSendData( SC_PVP, ePvP::SC_PVP_CHANGE_CHANNEL, reinterpret_cast<char*>(&packet), sizeof(packet) );
}
#endif		//#ifdef PRE_ADD_COLOSSEUM_BEGINNER

// Actor
#if defined(_VILLAGESERVER)
void CDNUserSendManager::SendActorMsg(UINT nSessionID, USHORT wActorProtocol, int nSize, char *pData, TParamData* pParamData )
{
	SCActorMessage ActorMsg;
	memset(&ActorMsg, 0, sizeof(SCActorMessage));

	ActorMsg.nSessionID = nSessionID;

	if(nSize > 128) nSize = 128;
	memcpy(ActorMsg.cBuf, pData, nSize);

	m_pSession->AddSendData(SC_ACTOR, wActorProtocol, (char*)&ActorMsg, sizeof(UINT) + nSize, pParamData );
}

void CDNUserSendManager::SendAddExp(UINT nSessionID, int nExp, int nAddExp, int nEventBonusExp, int nPcBangExp)
{
	SCActorMessage ActorMsg;
	memset(&ActorMsg, 0, sizeof(SCActorMessage));

	CPacketCompressStream Stream( ActorMsg.cBuf, 128 );

	Stream.Write( &nExp, sizeof(int) );
	Stream.Write( &nAddExp, sizeof(int) );
	Stream.Write( &nEventBonusExp, sizeof(int) );
	Stream.Write( &nPcBangExp, sizeof(int) );
#if defined(PRE_ADD_VIP)
	int nVIPExp = 0;
	Stream.Write( &nVIPExp, sizeof(int) );
#endif	// #if defined(PRE_ADD_VIP)

	int nPromoExp = 0;
	Stream.Write( &nPromoExp, sizeof(int) );


	ActorMsg.nSessionID = nSessionID;
	int nSize = (USHORT)Stream.Tell();

	m_pSession->AddSendData(SC_ACTOR, SC_ADDEXP, (char*)&ActorMsg, sizeof(SCActorMessage) -(USHORT)(128 - nSize));
}

void CDNUserSendManager::SendLevelUp(UINT nSessionID, int nLevel, int nExp)
{
	SCActorMessage ActorMsg;
	memset(&ActorMsg, 0, sizeof(SCActorMessage));

	CPacketCompressStream Stream( ActorMsg.cBuf, 128 );

	Stream.Write( &nLevel, sizeof(int), CPacketCompressStream::INTEGER_CHAR );
	Stream.Write( &nExp, sizeof(int) );

	ActorMsg.nSessionID = nSessionID;
	int nSize = (USHORT)Stream.Tell();

	m_pSession->AddSendData(SC_ACTOR, SC_LEVELUP, (char*)&ActorMsg, sizeof(SCActorMessage) -(USHORT)(128 - nSize));
}

#endif

void CDNUserSendManager::SendChangeJob(UINT nSessionID, int nJobID)
{
	SCActorMessage ActorMsg;
	memset(&ActorMsg, 0, sizeof(SCActorMessage));

	CPacketCompressStream Stream( ActorMsg.cBuf, 128 );

	Stream.Write( &nJobID, sizeof(int) );

	ActorMsg.nSessionID = nSessionID;
	int nSize = (USHORT)Stream.Tell();

	m_pSession->AddSendData(SC_ACTOR, eActor::SC_CHANGEJOB, (char*)&ActorMsg, sizeof(SCActorMessage) -(USHORT)(128 - nSize));
}

void CDNUserSendManager::SendAddCoin(UINT nSessionID, INT64 nTotalCoin, INT64 nAddCoin)
{
	SCActorMessage ActorMsg;
	memset(&ActorMsg, 0, sizeof(SCActorMessage));

	CPacketCompressStream Stream( ActorMsg.cBuf, 128 );

	Stream.Write( &nTotalCoin, sizeof(INT64) );
	Stream.Write( &nAddCoin, sizeof(INT64) );

	ActorMsg.nSessionID = nSessionID;
	int nSize = (USHORT)Stream.Tell();

	m_pSession->AddSendData(SC_ACTOR, eActor::SC_ADDCOIN, (char*)&ActorMsg, sizeof(SCActorMessage) -(USHORT)(128 - nSize));
}

void CDNUserSendManager::SendGhostType( UINT nSessionID, Ghost::Type::eCode Type )
{
	SCActorMessage ActorMsg;
	memset(&ActorMsg, 0, sizeof(SCActorMessage));

	CPacketCompressStream Stream( ActorMsg.cBuf, 128 );

	Stream.Write( &Type, sizeof(Type) );

	ActorMsg.nSessionID = nSessionID;
	int nSize = (USHORT)Stream.Tell();

	m_pSession->AddSendData( SC_ACTOR, eActor::SC_GHOST_TYPE, reinterpret_cast<char*>(&ActorMsg), sizeof(SCActorMessage) -(USHORT)(128 - nSize));
}

void CDNUserSendManager::SendUdpPing( UINT nSessionID, DWORD dwTick )
{
	SCActorMessage ActorMsg;
	memset(&ActorMsg, 0, sizeof(SCActorMessage));

	CPacketCompressStream Stream( ActorMsg.cBuf, 128 );

	Stream.Write( &dwTick, sizeof(dwTick) );

	ActorMsg.nSessionID = nSessionID;
	int nSize = (USHORT)Stream.Tell();

	m_pSession->AddSendData( SC_ACTOR, eActor::SC_UDP_PING, reinterpret_cast<char*>(&ActorMsg), sizeof(SCActorMessage) -(USHORT)(128 - nSize));
}

// Party
void CDNUserSendManager::SendCreateParty(int nRetCode, TPARTYID PartyID, bool bIsAutoCreate, ePartyType PartyType)
{
	SCCreateParty CreateParty;
	memset(&CreateParty, 0, sizeof(SCCreateParty));
	
	CreateParty.nRetCode = nRetCode;
	CreateParty.PartyID = PartyID;
	CreateParty.cIsAutoCreate = bIsAutoCreate == true ? 1 : 0;
	CreateParty.PartyType = PartyType;

	m_pSession->AddSendData(SC_PARTY, eParty::SC_CREATEPARTY, (char*)&CreateParty, sizeof(SCCreateParty));
}

void CDNUserSendManager::SendJoinParty(int nRetCode, TPARTYID PartyID, const WCHAR *pwszPartyName)
{
	SCJoinParty Join;
	memset(&Join, 0, sizeof(SCJoinParty));

	Join.nRetCode = nRetCode;
	Join.PartyID = PartyID;
	if( pwszPartyName ) 
	{
		if( wcslen(pwszPartyName) >= PARTYNAMELENMAX )
		{
			_DANGER_POINT();
			return;
		}

		_wcscpy(Join.wszPartyName, _countof(Join.wszPartyName), pwszPartyName, (int)wcslen(pwszPartyName));
	}

	m_pSession->AddSendData(SC_PARTY, eParty::SC_JOINPARTY, (char*)&Join, sizeof(SCJoinParty));
}

#if defined( PRE_WORLDCOMBINE_PARTY )
void CDNUserSendManager::SendRefreshParty(UINT nLeaderSessionID, int nCount, bool bHasVoiceChat, SPartyMemberInfo *pMemberInfo, short nRetCode, Party::Data *PartyData, int nWorldCombinePartyTableIndex)
{
	SCRefreshParty packet;
	memset(&packet, 0, sizeof(SCRefreshParty));

	packet.nLeaderSessionID = nLeaderSessionID;
	packet.cCurrentUserCount = (BYTE)nCount;
	packet.cPartyMemberMax = (BYTE)PartyData->nPartyMaxCount;
	packet.cUserLvLimitMin = PartyData->cMinLevel;
	packet.ItemLootRule = PartyData->LootRule;
	packet.ItemLootRank = PartyData->LootItemRank;
	packet.cVoiceChatAvailable = bHasVoiceChat == true ? 1 : 0;
	packet.nTargetMapIdx = PartyData->iTargetMapIndex;
	packet.Difficulty = PartyData->TargetMapDifficulty;
	packet.cPartyRefreshSubJect = (BYTE)nRetCode;
	_wcscpy(packet.wszName, _countof(packet.wszName), PartyData->wszPartyName, (int)wcslen(PartyData->wszPartyName));
	packet.iBitFlag = PartyData->iBitFlag;
	packet.PartyType = PartyData->Type;
	packet.iPassword = PartyData->iPassword;
	packet.nWorldCombinePartyTableIndex = nWorldCombinePartyTableIndex;

	if(nCount > 0)
		memcpy(packet.Member, pMemberInfo, sizeof(SPartyMemberInfo) * nCount);

	int nLen = sizeof(SCRefreshParty) - sizeof(packet.Member) +(sizeof(SPartyMemberInfo) * nCount);
	m_pSession->AddSendData(SC_PARTY, eParty::SC_REFRESHPARTY, (char*)&packet, nLen);
}
#else	//	#if defined( PRE_WORLDCOMBINE_PARTY )
#if defined( PRE_PARTY_DB )
void CDNUserSendManager::SendRefreshParty(UINT nLeaderSessionID, const WCHAR * pName, BYTE cPartyMemberMax, BYTE cUserLvLimitMin, int nCount, TPARTYITEMLOOTRULE ItemLootRule, TITEMRANK ItemLootRank, bool bHasVoiceChat, int nTargetMapIdx, TDUNGEONDIFFICULTY Difficulty, SPartyMemberInfo *pMemberInfo, short nRetCode, ePartyType PartyType, int iBitFlag, int iPassword )
#else
void CDNUserSendManager::SendRefreshParty(UINT nLeaderSessionID, const WCHAR * pName, const WCHAR * pPass, BYTE cPartyMemberMax, BYTE cUserLvLimitMin, BYTE cUserLvLimitMax, int nCount, TPARTYITEMLOOTRULE ItemLootRule, TITEMRANK ItemLootRank, bool bHasVoiceChat, int nTargetMapIdx, TDUNGEONDIFFICULTY Difficulty, SPartyMemberInfo *pMemberInfo, short nRetCode, BYTE cIsJobDice, BYTE cPartyType)
#endif // #if defined( PRE_PARTY_DB )
{
	SCRefreshParty packet;
	memset(&packet, 0, sizeof(SCRefreshParty));

	packet.nLeaderSessionID = nLeaderSessionID;
	packet.cCurrentUserCount = (BYTE)nCount;
	packet.cPartyMemberMax = cPartyMemberMax;
	packet.cUserLvLimitMin = cUserLvLimitMin;
	packet.ItemLootRule = ItemLootRule;
	packet.ItemLootRank = ItemLootRank;
	packet.cVoiceChatAvailable = bHasVoiceChat == true ? 1 : 0;
	packet.nTargetMapIdx = nTargetMapIdx;
	packet.Difficulty = Difficulty;
	packet.cPartyRefreshSubJect = (BYTE)nRetCode;
	_wcscpy(packet.wszName, _countof(packet.wszName), pName, (int)wcslen(pName));
#if defined( PRE_PARTY_DB )
	packet.iBitFlag = iBitFlag;
	packet.PartyType = PartyType;
	packet.iPassword =iPassword;
#else
	packet.cUserLvLimitMax = cUserLvLimitMax;
	packet.cIsJobDice = cIsJobDice;
	packet.cPartyType = cPartyType;
	_wcscpy(packet.wszPass, _countof(packet.wszPass), pPass, (int)wcslen(pPass));
#endif // #if defined( PRE_PARTY_DB )

	if(nCount > 0)
		memcpy(packet.Member, pMemberInfo, sizeof(SPartyMemberInfo) * nCount);

	int nLen = sizeof(SCRefreshParty) - sizeof(packet.Member) +(sizeof(SPartyMemberInfo) * nCount);
	m_pSession->AddSendData(SC_PARTY, eParty::SC_REFRESHPARTY, (char*)&packet, nLen);
}
#endif	//	#if defined( PRE_WORLDCOMBINE_PARTY )

void CDNUserSendManager::SendPartyOut(int nRetCode, char cKickKind, UINT nSessionID)
{
	SCPartyOut Out;
	memset(&Out, 0, sizeof(SCPartyOut));

	Out.nSessionID = nSessionID;
	Out.nRetCode = nRetCode;
	Out.cKickKind = cKickKind;

	m_pSession->AddSendData(SC_PARTY, eParty::SC_PARTYOUT, (char*)&Out, sizeof(SCPartyOut));
}

void CDNUserSendManager::SendKickPartymember(UINT nSessionID, int nRet)
{
	SCPartyKickedMember packet;

	packet.nSessionID = nSessionID;
	packet.nRetCode = nRet;

	m_pSession->AddSendData(SC_PARTY, eParty::SC_PARTYMEMBERKICKED, (char*)&packet, sizeof(packet));
}

void CDNUserSendManager::SendSwapPartyLeader(UINT nSessionID, int nRetCode)
{
	SCSwapPartyLeader packet;
	
	packet.nSessionID = nSessionID;
	packet.nRetCode = nRetCode;

	m_pSession->AddSendData(SC_PARTY, eParty::SC_PARTYLEADERSWAP, (char*)&packet, sizeof(packet));
}

#if defined( PRE_PARTY_DB )

#if defined( _VILLAGESERVER )
void CDNUserSendManager::SendPartyList( int iRet, const CSPartyListInfo* pPacket, std::list<CDNParty*>& PartyList )
{
	SCPartyListInfo TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.nRetCode = iRet;
	TxPacket.unReqPage = pPacket->unReqPage;
	TxPacket.iTotalCount = static_cast<int>(PartyList.size());

	if( iRet == ERROR_NONE )
	{
		if( (int)(PartyList.size()) >= pPacket->unReqPage*PARTYLISTOFFSET )
		{
			std::list<CDNParty*>::iterator itor = PartyList.begin();
			std::advance( itor, pPacket->unReqPage*PARTYLISTOFFSET );

			for( ; itor!=PartyList.end() ; ++itor )
			{
				if( (*itor)->GetPartyInfo( &TxPacket.PartyListInfo[TxPacket.iListCount] ) != ERROR_NONE )
					continue;

				++TxPacket.iListCount;
				if( TxPacket.iListCount >= _countof(TxPacket.PartyListInfo) )
					break;
			}
		}
	}
	
	int iSize = sizeof(TxPacket)-sizeof(TxPacket.PartyListInfo)+(sizeof(TxPacket.PartyListInfo[0])*TxPacket.iListCount);
	m_pSession->AddSendData( SC_PARTY, eParty::SC_PARTYLISTINFO, reinterpret_cast<char*>(&TxPacket), iSize );
}

#else

void CDNUserSendManager::SendPartyList( int iRet, const CSPartyListInfo* pPacket, std::list<CDNParty*>& PartyList )
{
#pragma message( "SendPartyList() is currently unused on GameServer; implement it if needed.")
}

#endif // #if defined( _VILLAGESERVER )

#else
void CDNUserSendManager::SendPartyList(std::list <SPartyListInfo> * pList, int nWholeCnt, int nRet)
{
	SCPartyListInfo packet;
	memset(&packet, 0, sizeof(SCPartyListInfo));

	packet.cCount = (BYTE)pList->size();
	packet.nWholeCount = nWholeCnt;
	packet.nRetCode = nRet;

	if(packet.cCount > PARTYLISTOFFSET * 2)
	{
		_DANGER_POINT();
		return;
	}

	int nCnt;
	std::list <SPartyListInfo>::iterator ii;
	for(ii = pList->begin(), nCnt = 0; ii != pList->end(); ii++, nCnt++)
		memcpy(&packet.PartyListInfo[nCnt], &(*ii), sizeof(SPartyListInfo));

	m_pSession->AddSendData(SC_PARTY, eParty::SC_PARTYLISTINFO, (char*)&packet, sizeof(packet) - sizeof(packet.PartyListInfo) + \
		(sizeof(SPartyListInfo) * packet.cCount));
}
#endif // #if defined( PRE_PARTY_DB )

void CDNUserSendManager::SendRefreshGateInfo(char *pGateNo, int nCount, int nRet)
{
	SCRefreshGateInfo Enter;
	memset(&Enter, 0, sizeof(SCRefreshGateInfo));
	Enter.nRet = nRet;

	if(nCount > PARTYMAX) nCount = PARTYMAX;
	for (int i = 0; i < nCount; i++){
		Enter.GateNo[i] = pGateNo[i];
	}

	Enter.cCount = nCount;

	m_pSession->AddSendData(SC_PARTY, eParty::SC_REFRESHGATEINFO, (char*)&Enter, (int)(sizeof(SCRefreshGateInfo) - sizeof(Enter.GateNo) +(sizeof(char) * nCount)));
}

void CDNUserSendManager::SendGateInfo(UINT nLeaderUID, char cGateNo, char cMapCount, char *cCanEnter, char *cPermitFlag)
{
	SCGateInfo Info;
	memset(&Info, 0, sizeof(SCGateInfo));

	Info.nLeaderUID = nLeaderUID;
	Info.cGateNo = cGateNo;
	Info.cMapCount = cMapCount;
	int nSize = sizeof(char) * 5;
	for( int i=0; i<cMapCount; i++ ) {
		memcpy( Info.cCanEnter[i], cCanEnter +( i * nSize ), nSize );
		Info.cPermitFlag[i] = cPermitFlag[i];
	}

	nSize = sizeof(SCGateInfo) - sizeof(Info.cCanEnter) +( nSize * cMapCount );
	m_pSession->AddSendData(SC_PARTY, eParty::SC_GATEINFO, (char*)&Info, nSize);
}


#if defined( PRE_ADD_VIP_FARM )

void CDNUserSendManager::SendFarmStart( int iRet )
{
	SCFarmStart TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.iRet = iRet;

	m_pSession->AddSendData(SC_FARM, eFarm::SC_START, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

void CDNUserSendManager::SendEffectItemInfo( int iItemID, __time64_t tExpireDate )
{
	SCEffectItemInfo TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.iItemID		= iItemID;
	TxPacket.tExpireDate	= tExpireDate;

	m_pSession->AddSendData(SC_ITEM, eItem::SC_EFFECTITEM_INFO, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

#endif // #if defined( PRE_ADD_VIP_FARM )

void CDNUserSendManager::SendFarmOpenList()
{
	m_pSession->AddSendData(SC_FARM, eFarm::SC_OPEN_FARMLIST, NULL, 0);
}

void CDNUserSendManager::SendFarmInfo(TFarmItem * pFarmItems, BYTE cFarmCount, TFarmFieldPartial * pFieldPartial, BYTE cFieldCount, bool bRefreshGate)
{
	if(pFarmItems == NULL || pFieldPartial == NULL)
	{
		_DANGER_POINT();
		return;
	}

	if(cFarmCount >= Farm::Max::FARMCOUNT || cFieldCount >= Farm::Max::FIELDCOUNT )
	{
		_DANGER_POINT();
		return;
	}

	SCFarmInfo Farm;
	memset(&Farm, 0, sizeof(SCFarmInfo));
	Farm.bRefreshGate = bRefreshGate;
	Farm.cCount = cFarmCount;
	memcpy(Farm.Farms, pFarmItems, sizeof(TFarmItem) * cFarmCount);

	m_pSession->AddSendData(SC_FARM, eFarm::SC_FARMINFO, (char*)&Farm, sizeof(SCFarmInfo) - sizeof(Farm.Farms) +(sizeof(TFarmItem) * Farm.cCount));

	//
	SendFarmFieldList( pFieldPartial, cFieldCount );
}

void CDNUserSendManager::SendFarmFieldList( const TFarmFieldPartial * pFieldPartial, BYTE cFieldCount )
{
	SCFarmPlantedInfo Planted;
	memset(&Planted, 0, sizeof(SCFarmPlantedInfo));

	Planted.cCount = cFieldCount;
	memcpy(Planted.FarmFields, pFieldPartial, sizeof(TFarmFieldPartial) * cFieldCount);

	m_pSession->AddSendData(SC_FARM, eFarm::SC_FARMPLANTEDINFO, (char*)&Planted, sizeof(SCFarmPlantedInfo) - sizeof(Planted.FarmFields) +(sizeof(TFarmFieldPartial) * Planted.cCount));

}

void CDNUserSendManager::SendDefaultPartsData( const std::vector<TPartyMemberDefaultParts>& vDefaultParts )
{
	SCDefaultPartsData TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	UINT uiSendCount = 0;
	for( UINT i=0 ; i<vDefaultParts.size() ; ++i )
	{
		TxPacket.DefaultParts[TxPacket.cCount] = vDefaultParts[i];
		++TxPacket.cCount;

		// Page completed
		if( TxPacket.cCount%_countof(TxPacket.DefaultParts) == 0 )
		{
#ifdef PRE_MOD_SYNCPACKET
#else		//#ifdef PRE_MOD_SYNCPACKET
			TxPacket.unStartIndex	= uiSendCount;
#endif		//#ifdef PRE_MOD_SYNCPACKET

			m_pSession->AddSendData( SC_PARTY, eParty::SC_DEFAULTPARTSDATA, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );

			uiSendCount		+= _countof(TxPacket.DefaultParts);
			TxPacket.cCount	 = 0;
		}
	}
	
	if( TxPacket.cCount > 0 )
	{
#ifdef PRE_MOD_SYNCPACKET
		int iSize = sizeof(TxPacket)-sizeof(TxPacket.DefaultParts)+TxPacket.cCount*sizeof(TPartyMemberDefaultParts);
#else		//#ifdef PRE_MOD_SYNCPACKET
		TxPacket.unStartIndex	= uiSendCount;
		int iSize = sizeof(TxPacket)-sizeof(TxPacket.DefaultParts)+TxPacket.cCount*sizeof(TxPacket.DefaultParts[0]);
#endif		//#ifdef PRE_MOD_SYNCPACKET

		m_pSession->AddSendData( SC_PARTY, eParty::SC_DEFAULTPARTSDATA, reinterpret_cast<char*>(&TxPacket), iSize );
	}
}

void CDNUserSendManager::SendWeaponOrderData( const std::vector<TPartyMemberWeaponOrder>& vWeaponOrder )
{
	SCWeaponOrderData TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	UINT uiSendCount = 0;
	for( UINT i=0 ; i<vWeaponOrder.size() ; ++i )
	{
		TxPacket.WeaponOrder[i-uiSendCount] = vWeaponOrder[i];

		// Page completed
		if((i+1)%_countof(TxPacket.WeaponOrder) == 0 )
		{
			TxPacket.cCount			= _countof(TxPacket.WeaponOrder);
#ifdef PRE_MOD_SYNCPACKET
#else		//#ifdef PRE_MOD_SYNCPACKET
			TxPacket.unStartIndex	= uiSendCount;
#endif		//#ifdef PRE_MOD_SYNCPACKET

			m_pSession->AddSendData( SC_PARTY, eParty::SC_WEAPONORDERDATA, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );

			uiSendCount += _countof(TxPacket.WeaponOrder);
		}
	}

	if( uiSendCount < vWeaponOrder.size() )
	{
		TxPacket.cCount			= static_cast<BYTE>(vWeaponOrder.size()-uiSendCount);
#ifdef PRE_MOD_SYNCPACKET
		int iSize = sizeof(TxPacket)-sizeof(TxPacket.WeaponOrder)+TxPacket.cCount*sizeof(TPartyMemberWeaponOrder);
#else		//#ifdef PRE_MOD_SYNCPACKET
		TxPacket.unStartIndex	= uiSendCount;
		int iSize = sizeof(TxPacket)-sizeof(TxPacket.WeaponOrder)+TxPacket.cCount*sizeof(TxPacket.WeaponOrder[0]);
#endif		//#ifdef PRE_MOD_SYNCPACKET

		m_pSession->AddSendData( SC_PARTY, eParty::SC_WEAPONORDERDATA, reinterpret_cast<char*>(&TxPacket), iSize );
	}
}

void CDNUserSendManager::SendEquipData( const std::vector<TPartyMemberEquip>& vEquip )
{
	SCEquipData TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

#ifdef PRE_MOD_SYNCPACKET
	UINT	uiSendCount = 0;
	for( UINT i=0 ; i<vEquip.size() ; ++i )
	{
		TxPacket.MemberEquip[i - uiSendCount] = vEquip[i];

		// Page completed
		if((i+1)%_countof(TxPacket.MemberEquip) == 0 )
		{
			TxPacket.cCount			= _countof(TxPacket.MemberEquip);
			m_pSession->AddSendData( SC_PARTY, eParty::SC_EQUIPDATA, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );

			uiSendCount += _countof(TxPacket.MemberEquip);
		}
	}

	if( uiSendCount < vEquip.size() )
	{
		TxPacket.cCount			= static_cast<BYTE>(vEquip.size()-uiSendCount);
		int iSize = sizeof(TxPacket)-sizeof(TxPacket.MemberEquip)+TxPacket.cCount*sizeof(TPartyMemberEquip);

		m_pSession->AddSendData( SC_PARTY, eParty::SC_EQUIPDATA, reinterpret_cast<char*>(&TxPacket), iSize );
	}
#else		//#ifdef PRE_MOD_SYNCPACKET
	UINT	uiSendCount = 0;
	int		iOffset		= sizeof(TxPacket)-sizeof(TxPacket.MemberEquip);

	for( UINT i=0 ; i<vEquip.size() ; ++i )
	{
		int iSize = ( vEquip[i].cCount * sizeof(TItemInfo) ) + sizeof(char);
		memcpy( reinterpret_cast<char*>(&TxPacket) + iOffset, &vEquip[i], iSize );
		iOffset += iSize;

		// Page completed
		if((i+1)%_countof(TxPacket.MemberEquip) == 0 )
		{
			TxPacket.cCount			= _countof(TxPacket.MemberEquip);
			TxPacket.unStartIndex	= uiSendCount;

			m_pSession->AddSendData( SC_PARTY, eParty::SC_EQUIPDATA, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );

			uiSendCount += _countof(TxPacket.MemberEquip);
			iOffset		 = sizeof(TxPacket)-sizeof(TxPacket.MemberEquip);
		}
	}

	if( uiSendCount < vEquip.size() )
	{
		TxPacket.cCount			= static_cast<BYTE>(vEquip.size()-uiSendCount);
		TxPacket.unStartIndex	= uiSendCount;
		int iSize = sizeof(TxPacket)-sizeof(TxPacket.MemberEquip)+TxPacket.cCount*sizeof(TxPacket.MemberEquip[0]);

		m_pSession->AddSendData( SC_PARTY, eParty::SC_EQUIPDATA, reinterpret_cast<char*>(&TxPacket), iSize );
	}
#endif		//#ifdef PRE_MOD_SYNCPACKET
}

void CDNUserSendManager::SendCashEquipData( const std::vector<TPartyMemberCashEquip>& vEquip )
{
	SCCashEquipData TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

#ifdef PRE_MOD_SYNCPACKET
	UINT	uiSendCount = 0;
	for( UINT i=0 ; i<vEquip.size() ; ++i )
	{
		TxPacket.MemberEquip[i - uiSendCount] = vEquip[i];

		// Page completed
		if((i+1)%_countof(TxPacket.MemberEquip) == 0 )
		{
			TxPacket.cCount			= _countof(TxPacket.MemberEquip);
			m_pSession->AddSendData( SC_PARTY, eParty::SC_EQUIPCASHDATA, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );

			uiSendCount += _countof(TxPacket.MemberEquip);
		}
	}

	if( uiSendCount < vEquip.size() )
	{
		TxPacket.cCount			= static_cast<BYTE>(vEquip.size()-uiSendCount);
		int iSize = sizeof(TxPacket)-sizeof(TxPacket.MemberEquip)+TxPacket.cCount*sizeof(TPartyMemberCashEquip);

		m_pSession->AddSendData( SC_PARTY, eParty::SC_EQUIPCASHDATA, reinterpret_cast<char*>(&TxPacket), iSize );
	}
#else		//#ifdef PRE_MOD_SYNCPACKET
	UINT	uiSendCount = 0;
	int		iOffset		= sizeof(TxPacket)-sizeof(TxPacket.MemberEquip);

	for( UINT i=0 ; i<vEquip.size() ; ++i )
	{
		int iSize = ( vEquip[i].cCount * sizeof(TItemInfo) ) + sizeof(char);
		memcpy( reinterpret_cast<char*>(&TxPacket) + iOffset, &vEquip[i], iSize );
		iOffset += iSize;

		// Page completed
		if((i+1)%_countof(TxPacket.MemberEquip) == 0 )
		{
			TxPacket.cCount			= _countof(TxPacket.MemberEquip);
			TxPacket.unStartIndex	= uiSendCount;

			m_pSession->AddSendData( SC_PARTY, eParty::SC_EQUIPCASHDATA, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );

			uiSendCount += _countof(TxPacket.MemberEquip);
			iOffset		 = sizeof(TxPacket)-sizeof(TxPacket.MemberEquip);
		}
	}

	if( uiSendCount < vEquip.size() )
	{
		TxPacket.cCount			= static_cast<BYTE>(vEquip.size()-uiSendCount);
		TxPacket.unStartIndex	= uiSendCount;
		int iSize = sizeof(TxPacket)-sizeof(TxPacket.MemberEquip)+TxPacket.cCount*sizeof(TxPacket.MemberEquip[0]);

		m_pSession->AddSendData( SC_PARTY, eParty::SC_EQUIPCASHDATA, reinterpret_cast<char*>(&TxPacket), iSize );
	}
#endif		//#ifdef PRE_MOD_SYNCPACKET
}

void CDNUserSendManager::SendGlyphEquipData( const std::vector<TPartyMemberGlyphEquip>& vGlyphEquip )
{
	SCGlyphEquipData TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

#ifdef PRE_MOD_SYNCPACKET
	UINT	uiSendCount = 0;
	for( UINT i=0 ; i<vGlyphEquip.size() ; ++i )
	{
		TxPacket.MemberEquip[i - uiSendCount] = vGlyphEquip[i];

		// Page completed
		if((i+1)%_countof(TxPacket.MemberEquip) == 0 )
		{
			TxPacket.cCount			= _countof(TxPacket.MemberEquip);
			m_pSession->AddSendData( SC_PARTY, eParty::SC_GLYPHDATA, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );

			uiSendCount += _countof(TxPacket.MemberEquip);
		}
	}

	if( uiSendCount < vGlyphEquip.size() )
	{
		TxPacket.cCount			= static_cast<BYTE>(vGlyphEquip.size()-uiSendCount);
		int iSize = sizeof(TxPacket)-sizeof(TxPacket.MemberEquip)+TxPacket.cCount*sizeof(TPartyMemberGlyphEquip);

		m_pSession->AddSendData( SC_PARTY, eParty::SC_GLYPHDATA, reinterpret_cast<char*>(&TxPacket), iSize );
	}
#else		//#ifdef PRE_MOD_SYNCPACKET
	UINT	uiSendCount = 0;
	int		iOffset		= sizeof(TxPacket)-sizeof(TxPacket.MemberEquip);

	for( UINT i=0 ; i<vGlyphEquip.size() ; ++i )
	{
		int iSize = ( vGlyphEquip[i].cCount * sizeof(TItemInfo) ) + sizeof(char);
		memcpy( reinterpret_cast<char*>(&TxPacket) + iOffset, &vGlyphEquip[i], iSize );
		iOffset += iSize;

		// Page completed
		if((i+1)%_countof(TxPacket.MemberEquip) == 0 )
		{
			TxPacket.cCount			= _countof(TxPacket.MemberEquip);
			TxPacket.unStartIndex	= uiSendCount;

			m_pSession->AddSendData( SC_PARTY, eParty::SC_GLYPHDATA, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );

			uiSendCount += _countof(TxPacket.MemberEquip);
			iOffset		 = sizeof(TxPacket)-sizeof(TxPacket.MemberEquip);
		}
	}

	if( uiSendCount < vGlyphEquip.size() )
	{
		TxPacket.cCount			= static_cast<BYTE>(vGlyphEquip.size()-uiSendCount);
		TxPacket.unStartIndex	= uiSendCount;
		int iSize = sizeof(TxPacket)-sizeof(TxPacket.MemberEquip)+TxPacket.cCount*sizeof(TxPacket.MemberEquip[0]);

		m_pSession->AddSendData( SC_PARTY, eParty::SC_GLYPHDATA, reinterpret_cast<char*>(&TxPacket), iSize );
	}
#endif		//#ifdef PRE_MOD_SYNCPACKET
}

#if defined(PRE_ADD_TALISMAN_SYSTEM)
void CDNUserSendManager::SendTalismanEquipData( const std::vector<TPartyMemberTalismanEquip>& vTalismanEquip )
{
	SCTalismanEquipData TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

#ifdef PRE_MOD_SYNCPACKET
	UINT uiSendCount = 0;
	for( UINT i=0 ; i<vTalismanEquip.size() ; ++i )
	{
		TxPacket.MemberEquip[i - uiSendCount] = vTalismanEquip[i];

		if( (i + 1) % _countof(TxPacket.MemberEquip) == 0 )
		{
			TxPacket.cCount = _countof(TxPacket.MemberEquip);
			m_pSession->AddSendData( SC_PARTY, eParty::SC_TALISMANDATA, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
			uiSendCount += _countof(TxPacket.MemberEquip);
		}
	}

	if( uiSendCount < vTalismanEquip.size() )
	{
		TxPacket.cCount = static_cast<BYTE>(vTalismanEquip.size() - uiSendCount);
		int iSize = static_cast<int>(sizeof(TxPacket) - sizeof(TxPacket.MemberEquip) + TxPacket.cCount * sizeof(TPartyMemberTalismanEquip));
		m_pSession->AddSendData( SC_PARTY, eParty::SC_TALISMANDATA, reinterpret_cast<char*>(&TxPacket), iSize );
	}
#else
	UINT uiSendCount = 0;
	int iOffset = sizeof(TxPacket) - sizeof(TxPacket.MemberEquip);

	for( UINT i=0 ; i<vTalismanEquip.size() ; ++i )
	{
		int iSize = ( vTalismanEquip[i].cCount * sizeof(TItemInfo) ) + sizeof(char);
		memcpy( reinterpret_cast<char*>(&TxPacket) + iOffset, &vTalismanEquip[i], iSize );
		iOffset += iSize;

		if( (i + 1) % _countof(TxPacket.MemberEquip) == 0 )
		{
			TxPacket.cCount = _countof(TxPacket.MemberEquip);
			TxPacket.unStartIndex = uiSendCount;
			m_pSession->AddSendData( SC_PARTY, eParty::SC_TALISMANDATA, reinterpret_cast<char*>(&TxPacket), iOffset );
			uiSendCount += _countof(TxPacket.MemberEquip);
			iOffset = sizeof(TxPacket) - sizeof(TxPacket.MemberEquip);
		}
	}

	if( uiSendCount < vTalismanEquip.size() )
	{
		TxPacket.cCount = static_cast<BYTE>(vTalismanEquip.size() - uiSendCount);
		TxPacket.unStartIndex = uiSendCount;
		m_pSession->AddSendData( SC_PARTY, eParty::SC_TALISMANDATA, reinterpret_cast<char*>(&TxPacket), iOffset );
	}
#endif
}
#endif	// #if defined(PRE_ADD_TALISMAN_SYSTEM)


#ifdef PRE_MOD_SYNCPACKET
void CDNUserSendManager::SendVehicleEquipData( const std::vector<TPartyVehicle>& vVehicle )
{
	SCVehicleEquipData TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	UINT	uiSendCount = 0;
	for( UINT i=0 ; i<vVehicle.size() ; ++i )
	{
		TVehicleCompact VehicleCompact;
		memset(&VehicleCompact, 0, sizeof(VehicleCompact));

		VehicleCompact.SetCompact( vVehicle[i].tVehicle );
		VehicleCompact.nSessionID = vVehicle[i].nSessionID;
		TxPacket.MemberVehicle[i - uiSendCount] = VehicleCompact;

		// Page completed
		if((i+1)%_countof(TxPacket.MemberVehicle) == 0 )
		{
			TxPacket.cCount			= _countof(TxPacket.MemberVehicle);
			m_pSession->AddSendData( SC_PARTY, eParty::SC_VEHICLEEQUIPDATA, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );

			uiSendCount += _countof(TxPacket.MemberVehicle);
		}
	}

	if( uiSendCount < vVehicle.size() )
	{
		TxPacket.cCount			= static_cast<BYTE>(vVehicle.size()-uiSendCount);
		int iSize = sizeof(TxPacket)-sizeof(TxPacket.MemberVehicle)+TxPacket.cCount*sizeof(TVehicleCompact);

		m_pSession->AddSendData( SC_PARTY, eParty::SC_VEHICLEEQUIPDATA, reinterpret_cast<char*>(&TxPacket), iSize );
	}
}
#else		//#ifdef PRE_MOD_SYNCPACKET
void CDNUserSendManager::SendVehicleEquipData( const std::vector<TVehicle>& vVehicle )
{
	SCVehicleEquipData TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	UINT	uiSendCount = 0;
	int		iOffset		= sizeof(TxPacket)-sizeof(TxPacket.MemberVehicle);

	for( UINT i=0 ; i<vVehicle.size() ; ++i )
	{
		int iSize = sizeof(TVehicleCompact);

		TVehicleCompact VehicleCompact;
		memset(&VehicleCompact, 0, sizeof(VehicleCompact));
		VehicleCompact.SetCompact( vVehicle[i] );

		memcpy( reinterpret_cast<char*>(&TxPacket) + iOffset, &VehicleCompact, iSize );
		iOffset += iSize;

		// Page completed
		if((i+1)%_countof(TxPacket.MemberVehicle) == 0 )
		{
			TxPacket.cCount			= _countof(TxPacket.MemberVehicle);
			TxPacket.unStartIndex	= uiSendCount;

			m_pSession->AddSendData( SC_PARTY, eParty::SC_VEHICLEEQUIPDATA, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );

			uiSendCount += _countof(TxPacket.MemberVehicle);
			iOffset		 = sizeof(TxPacket)-sizeof(TxPacket.MemberVehicle);
		}
	}

	if( uiSendCount < vVehicle.size() )
	{
		TxPacket.cCount			= static_cast<BYTE>(vVehicle.size()-uiSendCount);
		TxPacket.unStartIndex	= uiSendCount;
		int iSize = sizeof(TxPacket)-sizeof(TxPacket.MemberVehicle)+TxPacket.cCount*sizeof(TxPacket.MemberVehicle[0]);

		m_pSession->AddSendData( SC_PARTY, eParty::SC_VEHICLEEQUIPDATA, reinterpret_cast<char*>(&TxPacket), iSize );
	}
}
#endif		//#ifdef PRE_MOD_SYNCPACKET

#ifdef PRE_MOD_SYNCPACKET
void CDNUserSendManager::SendPetEquipData( const std::vector<TPartyVehicle>& vPet )
{
	SCVehicleEquipData TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	UINT	uiSendCount = 0;
	for( UINT i=0 ; i<vPet.size() ; ++i )
	{
		TVehicleCompact VehicleCompact;
		memset(&VehicleCompact, 0, sizeof(TVehicleCompact));

		VehicleCompact.SetCompact( vPet[i].tVehicle );
		VehicleCompact.nSessionID = vPet[i].nSessionID;

		TxPacket.MemberVehicle[i - uiSendCount] = VehicleCompact;

		// Page completed
		if((i+1)%_countof(TxPacket.MemberVehicle) == 0 )
		{
			TxPacket.cCount			= _countof(TxPacket.MemberVehicle);
			m_pSession->AddSendData( SC_PARTY, eParty::SC_PETEQUIPDATA, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );

			uiSendCount += _countof(TxPacket.MemberVehicle);
		}
	}

	if( uiSendCount < vPet.size() )
	{
		TxPacket.cCount			= static_cast<BYTE>(vPet.size()-uiSendCount);
		int iSize = sizeof(TxPacket)-sizeof(TxPacket.MemberVehicle)+TxPacket.cCount*sizeof(TVehicleCompact);

		m_pSession->AddSendData( SC_PARTY, eParty::SC_PETEQUIPDATA, reinterpret_cast<char*>(&TxPacket), iSize );
	}
}
#else		//#ifdef PRE_MOD_SYNCPACKET
void CDNUserSendManager::SendPetEquipData( const std::vector<TVehicle>& vPet )
{
	SCVehicleEquipData TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	UINT	uiSendCount = 0;
	int		iOffset		= sizeof(TxPacket)-sizeof(TxPacket.MemberVehicle);

	for( UINT i=0 ; i<vPet.size() ; ++i )
	{
		int iSize = sizeof(TVehicleCompact);

		TVehicleCompact VehicleCompact;
		memset(&VehicleCompact, 0, sizeof(TVehicleCompact));

		VehicleCompact.SetCompact( vPet[i] );

		memcpy( reinterpret_cast<char*>(&TxPacket) + iOffset, &VehicleCompact, iSize );
		iOffset += iSize;

		// Page completed
		if((i+1)%_countof(TxPacket.MemberVehicle) == 0 )
		{
			TxPacket.cCount			= _countof(TxPacket.MemberVehicle);
			TxPacket.unStartIndex	= uiSendCount;

			m_pSession->AddSendData( SC_PARTY, eParty::SC_PETEQUIPDATA, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );

			uiSendCount += _countof(TxPacket.MemberVehicle);
			iOffset		 = sizeof(TxPacket)-sizeof(TxPacket.MemberVehicle);
		}
	}

	if( uiSendCount < vPet.size() )
	{
		TxPacket.cCount			= static_cast<BYTE>(vPet.size()-uiSendCount);
		TxPacket.unStartIndex	= uiSendCount;
		int iSize = sizeof(TxPacket)-sizeof(TxPacket.MemberVehicle)+TxPacket.cCount*sizeof(TxPacket.MemberVehicle[0]);

		m_pSession->AddSendData( SC_PARTY, eParty::SC_PETEQUIPDATA, reinterpret_cast<char*>(&TxPacket), iSize );
	}
}
#endif		//#ifdef PRE_MOD_SYNCPACKET

void CDNUserSendManager::SendSkillData( const std::vector<TPartyMemberSkill>& vSkill )
{
	SCSkillData TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

#ifdef PRE_MOD_SYNCPACKET
	UINT	uiSendCount = 0;

	for( UINT i=0 ; i<vSkill.size() ; ++i )
	{
		TxPacket.MemberSkill[i - uiSendCount] = vSkill[i];

		// Page completed
		if((i+1)%_countof(TxPacket.MemberSkill) == 0 )
		{
			TxPacket.cCount			= _countof(TxPacket.MemberSkill);
			m_pSession->AddSendData( SC_PARTY, eParty::SC_SKILLDATA, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );

			uiSendCount += _countof(TxPacket.MemberSkill);
		}
	}

	if( uiSendCount < vSkill.size() )
	{
		TxPacket.cCount			= static_cast<BYTE>(vSkill.size()-uiSendCount);
		int iSize = sizeof(TxPacket)-sizeof(TxPacket.MemberSkill)+TxPacket.cCount*sizeof(TPartyMemberSkill);

		m_pSession->AddSendData( SC_PARTY, eParty::SC_SKILLDATA, reinterpret_cast<char*>(&TxPacket), iSize );
	}
#else		//#ifdef PRE_MOD_SYNCPACKET
	UINT	uiSendCount = 0;
	int		iOffset		= sizeof(TxPacket)-sizeof(TxPacket.MemberSkill);

	for( UINT i=0 ; i<vSkill.size() ; ++i )
	{
		int iSize = ( vSkill[i].cCount * sizeof(TSkill) ) + sizeof(char);

		memcpy( reinterpret_cast<char*>(&TxPacket) + iOffset, &vSkill[i], iSize );
		iOffset += iSize;

		// Page completed
		if((i+1)%_countof(TxPacket.MemberSkill) == 0 )
		{
			TxPacket.cCount			= _countof(TxPacket.MemberSkill);
			TxPacket.unStartIndex	= uiSendCount;

			m_pSession->AddSendData( SC_PARTY, eParty::SC_SKILLDATA, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );

			uiSendCount += _countof(TxPacket.MemberSkill);
			iOffset		 = sizeof(TxPacket)-sizeof(TxPacket.MemberSkill);
		}
	}

	if( uiSendCount < vSkill.size() )
	{
		TxPacket.cCount			= static_cast<BYTE>(vSkill.size()-uiSendCount);
		TxPacket.unStartIndex	= uiSendCount;
		int iSize = sizeof(TxPacket)-sizeof(TxPacket.MemberSkill)+TxPacket.cCount*sizeof(TxPacket.MemberSkill[0]);

		m_pSession->AddSendData( SC_PARTY, eParty::SC_SKILLDATA, reinterpret_cast<char*>(&TxPacket), iSize );
	}
#endif		//#ifdef PRE_MOD_SYNCPACKET
}

void CDNUserSendManager::SendPartyEtcData( const std::vector<TPartyEtcData>& vEtcData )
{
	SCEtcData TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

#ifdef PRE_MOD_SYNCPACKET
	UINT uiSendCount = 0;

	for( UINT i=0 ; i<vEtcData.size() ; ++i )
	{
		TxPacket.EtcData[i - uiSendCount] = vEtcData[i];

		// Page completed
		if((i+1)%_countof(TxPacket.EtcData) == 0 )
		{
			TxPacket.cCount			= _countof(TxPacket.EtcData);
			m_pSession->AddSendData( SC_PARTY, eParty::SC_ETCDATA, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );

			uiSendCount += _countof(TxPacket.EtcData);
		}
	}

	if( uiSendCount < vEtcData.size() )
	{
		TxPacket.cCount			= static_cast<BYTE>(vEtcData.size()-uiSendCount);
		int iSize = sizeof(TxPacket)-sizeof(TxPacket.EtcData)+TxPacket.cCount*sizeof(TPartyEtcData);

		m_pSession->AddSendData( SC_PARTY, eParty::SC_ETCDATA, reinterpret_cast<char*>(&TxPacket), iSize );
	}
#else		//#ifdef PRE_MOD_SYNCPACKET
	UINT uiSendCount = 0;

	for( UINT i=0 ; i<vEtcData.size() ; ++i )
	{
		TxPacket.EtcData[i-uiSendCount] = vEtcData[i];

		// Page completed
		if((i+1)%_countof(TxPacket.EtcData) == 0 )
		{
			TxPacket.cCount			= _countof(TxPacket.EtcData);
			TxPacket.unStartIndex	= uiSendCount;

			m_pSession->AddSendData( SC_PARTY, eParty::SC_ETCDATA, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );

			uiSendCount += _countof(TxPacket.EtcData);
		}
	}

	if( uiSendCount < vEtcData.size() )
	{
		TxPacket.cCount			= static_cast<BYTE>(vEtcData.size()-uiSendCount);
		TxPacket.unStartIndex	= uiSendCount;
		int iSize = sizeof(TxPacket)-sizeof(TxPacket.EtcData)+TxPacket.cCount*sizeof(TxPacket.EtcData[0]);

		m_pSession->AddSendData( SC_PARTY, eParty::SC_ETCDATA, reinterpret_cast<char*>(&TxPacket), iSize );
	}
#endif		//#ifdef PRE_MOD_SYNCPACKET
}

void CDNUserSendManager::SendPartyGuildData( const std::vector<TPartyMemberGuild::TPartyMemberGuildView>& vMemberGuildView )
{
	TPartyMemberGuild TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	UINT uiSendCount = 0;

	for( UINT i=0 ; i<vMemberGuildView.size() ; ++i )
	{
		TxPacket.List[TxPacket.nCount] = vMemberGuildView[i];
		++TxPacket.nCount;

		// Page completed
		if( TxPacket.nCount%_countof(TxPacket.List) == 0 )
		{
			TxPacket.unStartIndex = uiSendCount;

			m_pSession->AddSendData( SC_PARTY, eParty::SC_GUILDDATA, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );

			uiSendCount		 += _countof(TxPacket.List);
			TxPacket.nCount = 0;
		}
	}

	if( TxPacket.nCount > 0 )
	{
		TxPacket.unStartIndex = uiSendCount;
		int iSize = sizeof(TxPacket)-sizeof(TxPacket.List)+TxPacket.nCount*sizeof(TxPacket.List[0]);
		m_pSession->AddSendData( SC_PARTY, eParty::SC_GUILDDATA, reinterpret_cast<char*>(&TxPacket),iSize );
	}
}

#if defined (PRE_ADD_BESTFRIEND)
void CDNUserSendManager::SendPartyBestFriend( const std::vector<TPartyBestFriend>& vMemberBestFriend )
{
	SCPartyBestFriend TxPacket;
	memset (&TxPacket, 0, sizeof(TxPacket));

	for( UINT i=0 ; i<vMemberBestFriend.size() ; ++i )
	{
		TxPacket.List[TxPacket.nCount] = vMemberBestFriend[i];
		++TxPacket.nCount;
	}

	if (TxPacket.nCount > 0)
	{
		int iSize = sizeof(TxPacket) - sizeof(TxPacket.List) + (TxPacket.nCount * sizeof(TxPacket.List[0]));
		m_pSession->AddSendData( SC_PARTY, eParty::SC_BESTFRIENDDATA, reinterpret_cast<char*>(&TxPacket),iSize );
	}
}
#endif // #if defined (PRE_ADD_BESTFRIEND)

void CDNUserSendManager::SendSyncEquipData( TPartyMemberEquip* EquipArray, int iCount )
{
	SCROOM_SYNC_MEMBEREQUIP TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.nCount = iCount;

#ifdef PRE_MOD_SYNCPACKET
#else		//#ifdef PRE_MOD_SYNCPACKET
	int iSize = 0;
	int iOffset = sizeof(short);
#endif		//#ifdef PRE_MOD_SYNCPACKET
	if( iCount > 0 ) 
	{
		for( int i=0 ; i<iCount ; ++i ) 
		{
#ifdef PRE_MOD_SYNCPACKET
			TxPacket.MemberEquip[i] = EquipArray[i];
#else		//#ifdef PRE_MOD_SYNCPACKET
			iSize = ( EquipArray[i].cCount * sizeof(TItemInfo) ) + sizeof(char);
			memcpy(((char*)&TxPacket)+iOffset, &EquipArray[i], iSize );
			iOffset += iSize;
#endif		//#ifdef PRE_MOD_SYNCPACKET
		}
	}	

#ifdef PRE_MOD_SYNCPACKET
	m_pSession->AddSendData( SC_ROOM, eRoom::SC_SYNC_MEMBEREQUIP, (char*)&TxPacket, sizeof(SCROOM_SYNC_MEMBEREQUIP) - sizeof(TxPacket.MemberEquip) + (sizeof(TPartyMemberEquip) * iCount) );
#else		//#ifdef PRE_MOD_SYNCPACKET
	m_pSession->AddSendData( SC_ROOM, eRoom::SC_SYNC_MEMBEREQUIP, (char*)&TxPacket, iOffset );
#endif		//#ifdef PRE_MOD_SYNCPACKET
}

void CDNUserSendManager::SendSyncSkillData(TPartyMemberSkill *pSkillArray, int iCount)
{
	SCROOM_SYNC_MEMBERSKILL TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.nCount = iCount;
	int iSize	= 0;
	int iOffset = sizeof(short);
	if( iCount > 0 ) 
	{
		for( int i=0; i<iCount; ++i ) 
		{
			iSize = ( pSkillArray[i].cCount * sizeof(TSkill) ) + sizeof(char);
			memcpy(((char*)&TxPacket)+iOffset, &pSkillArray[i], iSize );
			iOffset += iSize;
		}
	}

	m_pSession->AddSendData( SC_ROOM, eRoom::SC_SYNC_MEMBERSKILL, (char*)&TxPacket, iOffset );
}

void CDNUserSendManager::SendStartStage( bool bDirectConnect, int nMapIndex, char cGateNo, int nRandomSeed, TDUNGEONDIFFICULTY Difficulty, int nExtendDropRate )
{
	if( bDirectConnect ) {
		SCStartStageDirect Dungeon;
		memset(&Dungeon, 0, sizeof(SCStartStageDirect));

		Dungeon.nMapIndex = nMapIndex;
		Dungeon.cGateNo = cGateNo;
		Dungeon.nRandomSeed = nRandomSeed;
		Dungeon.Difficulty = Difficulty;
		Dungeon.nExtendDropRate = nExtendDropRate;

		m_pSession->AddSendData(SC_PARTY, eParty::SC_STARTSTAGEDIRECT, (char*)&Dungeon, sizeof(SCStartStageDirect));
	}
	else {
		SCStartStage Stage;
		memset(&Stage, 0, sizeof(SCStartStage));

		Stage.nRandomSeed = nRandomSeed;
		Stage.Difficulty = Difficulty;
		Stage.nExtendDropRate = nExtendDropRate;

		m_pSession->AddSendData(SC_PARTY, eParty::SC_STARTSTAGE, (char*)&Stage, sizeof(SCStartStage));
	}

#ifdef _DEBUG
	g_Log.Log(LogType::_NORMAL, m_pSession, L"[ADBID:%u, CDBID:%I64d, SID:%u] SendStartStage\r\n", m_pSession->GetAccountDBID(), m_pSession->GetCharacterDBID(), m_pSession->GetSessionID());
#endif
}

void CDNUserSendManager::SendSelectStage( char cSelectIndex )
{
	SCSelectStage Stage;
	memset(&Stage, 0, sizeof(SCSelectStage));

	Stage.cSelectMapIndex = cSelectIndex;
	m_pSession->AddSendData(SC_PARTY, eParty::SC_SELECTSTAGE, (char*)&Stage, sizeof(SCSelectStage));
}

void CDNUserSendManager::SendVillageInfo(const char * pIP, USHORT nPort, short nRet, UINT nAccountDBID, INT64 biCertifyingKey)
{
	SCVillageInfo packet;
	memset( &packet, 0, sizeof(packet) );

	packet.nSessionID = m_pSession->GetSessionID();
	if( pIP )
		_strcpy(packet.szVillageIp, _countof(packet.szVillageIp), pIP, (int)strlen(pIP));
	packet.wVillagePort = nPort;
	packet.nRet	= nRet;
	packet.nAccountDBID = nAccountDBID;
	packet.biCertifyingKey = biCertifyingKey;
	m_pSession->AddSendData(SC_SYSTEM, eSystem::SC_VILLAGEINFO, (char*)&packet, sizeof(SCVillageInfo));
}

void CDNUserSendManager::SendStartVillage()
{
	m_pSession->AddSendData(SC_PARTY, eParty::SC_STARTVILLAGE, NULL, 0);
}

void CDNUserSendManager::SendCancelStage( int nRetCode )
{
	SCCancelStage CancelStage;
	memset(&CancelStage, 0, sizeof(SCCancelStage));

	CancelStage.nRetCode = nRetCode;

	m_pSession->AddSendData(SC_PARTY, eParty::SC_CANCELSTAGE, (char*)&CancelStage, sizeof(SCCancelStage));
}

void CDNUserSendManager::SendChangeLeader(UINT nSessionID)
{
	SCPartyChangeLeader packet;
	packet.nNewLeaderSessionID = nSessionID;
	m_pSession->AddSendData(SC_PARTY, eParty::SC_CHANGEPARTYLEADER, (char*)&packet, sizeof(packet));
}

#if defined( PRE_PARTY_DB )
void CDNUserSendManager::SendPartyInviteMsg(TPARTYID PartyID, const WCHAR * pPartyName, int iPassword, const WCHAR * pSrcName, BYTE cMaxUserCount, BYTE cCurUserCount, USHORT nAvrLevel, int nGameServerID, int nGameMapIdx)
#else
void CDNUserSendManager::SendPartyInviteMsg(TPARTYID PartyID, const WCHAR * pPartyName, const WCHAR * pPass, const WCHAR * pSrcName, BYTE cMaxUserCount, BYTE cCurUserCount, USHORT nAvrLevel, int nGameServerID, int nGameMapIdx)
#endif // #if defined( PRE_PARTY_DB )
{
	SCPartyInvite packet;
	memset(&packet, 0, sizeof(packet));

	packet.PartyID = PartyID;
	packet.cMaxUserCount = cMaxUserCount;
	packet.cCurUserCount = cCurUserCount;
	packet.nAvrLevel = nAvrLevel;
	packet.nGameServerID = nGameServerID;
	packet.nGameMapIdx = nGameMapIdx;

	if( pPartyName )
	{
		packet.nPartyNameLen = (USHORT)wcslen(pPartyName);
		if( packet.nPartyNameLen >= PARTYNAMELENMAX )
		{
			_DANGER_POINT();
			return;
		}

		_wcscpy(packet.wszBuf, _countof(packet.wszBuf), pPartyName, (int)wcslen(pPartyName));
	}

#if defined( PRE_PARTY_DB )
	packet.iPassword = iPassword;
#else
	if(pPass != NULL && pPass[0] != '\0')
	{
		packet.cPartyPassLen = (BYTE)wcslen(pPass);
		if( packet.cPartyPassLen > PARTYPASSWORDMAX )
		{
			_DANGER_POINT();
			return;
		}

		_wcscpy(packet.wszBuf + packet.nPartyNameLen, _countof(packet.wszBuf)-packet.nPartyNameLen , pPass, (int)wcslen(pPass));
	}
#endif // #if defined( PRE_PARTY_DB )
	if( pSrcName )
	{
		packet.cPartyInviterNameLen = (BYTE)wcslen(pSrcName);
		if( packet.cPartyInviterNameLen > NAMELENMAX )
		{
			_DANGER_POINT();
			return;
		}

#if defined( PRE_PARTY_DB )
		_wcscpy(packet.wszBuf + packet.nPartyNameLen, _countof(packet.wszBuf) - packet.nPartyNameLen , pSrcName, (int)wcslen(pSrcName));
#else
		_wcscpy(packet.wszBuf + packet.nPartyNameLen + packet.cPartyPassLen, _countof(packet.wszBuf) - packet.nPartyNameLen - packet.cPartyPassLen , pSrcName, (int)wcslen(pSrcName));
#endif // #if defined( PRE_PARTY_DB )
	}
	
#if defined( PRE_PARTY_DB )
	m_pSession->AddSendData(SC_PARTY, eParty::SC_PARTYINVITE, (char*)&packet, sizeof(packet) - sizeof(packet.wszBuf) + \
		((packet.nPartyNameLen + packet.cPartyInviterNameLen ) * sizeof(WCHAR)));
#else
	m_pSession->AddSendData(SC_PARTY, eParty::SC_PARTYINVITE, (char*)&packet, sizeof(packet) - sizeof(packet.wszBuf) + \
		((packet.nPartyNameLen + packet.cPartyInviterNameLen + packet.cPartyPassLen) * sizeof(WCHAR)));
#endif // #if defined( PRE_PARTY_DB )
}

void CDNUserSendManager::SendPartyIniviteNotice(const WCHAR * pwszInvitedName, int nRetCode)
{
	SCPartyInviteNotice packet;
	memset(&packet, 0, sizeof(SCPartyInviteNotice));

	packet.nRetCode = nRetCode;
	_wcscpy(packet.wszInvitedCharacterName, _countof(packet.wszInvitedCharacterName), pwszInvitedName, (int)wcslen(pwszInvitedName));

	m_pSession->AddSendData(SC_PARTY, eParty::SC_PARTYINVITENOTICE, (char*)&packet, sizeof(packet));
}

void CDNUserSendManager::SendPartyInviteFail(int nRet)
{
	SCPartyInviteFail packet;
	packet.nRetCode = nRet;
	m_pSession->AddSendData(SC_PARTY, eParty::SC_PARTYINVITEFAIL, (char*)&packet, sizeof(packet));
}

void CDNUserSendManager::SendPartyInviteDenied(const WCHAR * pDest, int nRet)
{
	SCPartyInviteDenied packet;
	memset(&packet, 0, sizeof(packet));

	packet.nRetCode = nRet;
	_wcscpy(packet.wszCharName, _countof(packet.wszCharName), pDest, (int)wcslen(pDest));

	m_pSession->AddSendData(SC_PARTY, eParty::SC_PARTYINVITEDENIED, (char*)&packet, sizeof(packet));	
}

void CDNUserSendManager::SendPartyMemberInfo(SCPartyMemberInfo * pPacket)
{
#if defined( PRE_PARTY_DB )
	int iSize = sizeof(SCPartyMemberInfo)-sizeof(pPacket->MemberData)+(pPacket->nCount*sizeof(pPacket->MemberData[0]));
	m_pSession->AddSendData(SC_PARTY, eParty::SC_PARTYMEMBER_INFO, (char*)pPacket, iSize );
#else
	m_pSession->AddSendData(SC_PARTY, eParty::SC_PARTYMEMBER_INFO, (char*)pPacket, sizeof(SCPartyMemberInfo) - sizeof(pPacket->Info) +(sizeof(TPartyMemberInfo) * pPacket->cCount));
#endif // #if defined( PRE_PARTY_DB )
}

void CDNUserSendManager::SendChannelInfo(SCChannelList * pPacket)
{
	int iSize = sizeof(SCChannelList)-sizeof(pPacket->Info)+pPacket->cCount*sizeof(pPacket->Info[0]);
	m_pSession->AddSendData( SC_PARTY, eParty::SC_CHANNELLIST, (char*)pPacket, iSize );
}

void CDNUserSendManager::SendMoveChannelFail(int nRet)
{
	SCMoveChannelFail packet;
	packet.nRetCode = nRet;
	m_pSession->AddSendData(SC_PARTY, eParty::SC_MOVECHANNELFAIL, NULL, 0);
}

void CDNUserSendManager::SendPartyInfoErr(short nRetCode)
{
	SCPartyInfoErr packet;
	memset(&packet, 0, sizeof(packet));
	packet.nRetCode = nRetCode;
	m_pSession->AddSendData(SC_PARTY, eParty::SC_PARTYINFO_ERROR, (char*)&packet, sizeof(packet));
}

void CDNUserSendManager::SendSelectDungeonInfo(int nMapIndex, BYTE cDiffyculty)
{
	SCSelectDunGeon packet;
	memset(&packet, 0, sizeof(packet));

	packet.cDiffyculty = cDiffyculty;
	packet.nMapIndex = nMapIndex;
	
	m_pSession->AddSendData(SC_PARTY, eParty::SC_SELECTDUNGEON, (char*)&packet, sizeof(packet));
}

void CDNUserSendManager::SendPartyModify(short nRetCode)
{
	SCPartyInfoModify packet;
	memset(&packet, 0, sizeof(packet));

	packet.nRet = nRetCode;

	m_pSession->AddSendData(SC_PARTY, eParty::SC_PARTYMODIFY, (char*)&packet, sizeof(packet));
}

void CDNUserSendManager::SendVerifyJoinParty(int nPartyIdx, int nChannelID, short nRetCode)
{
	SCVerifyJoinParty packet;
	memset(&packet, 0, sizeof(packet));

	packet.nPartyIdx = nPartyIdx;
	packet.nChannelID = nChannelID;
	packet.nRet = nRetCode;

	m_pSession->AddSendData(SC_PARTY, eParty::SC_VERIFYJOINPARTY, (char*)&packet, sizeof(packet));
}

void CDNUserSendManager::SendPartyMemberMove(UINT nSessionID, EtVector3 vPos)
{
	SCPartyMemberMove Move;
	memset(&Move, 0, sizeof(SCPartyMemberMove));

	Move.nSessionID = nSessionID;
	Move.Position = vPos;

	m_pSession->AddSendData(SC_PARTY, eParty::SC_PARTYMEMBER_MOVE, (char*)&Move, sizeof(SCPartyMemberMove));
}

void CDNUserSendManager::SendPartyMemberPart(CDNUserSession *pSession)
{
	SCPartyMemberPart Part;
	memset(&Part, 0, sizeof(SCPartyMemberPart));

	Part.nSessionID = pSession->GetSessionID();
	Part.nExp = pSession->GetExp();
	Part.cLevel = pSession->GetLevel();
	memcpy( Part.cJobArray, pSession->GetStatusData()->cJobArray, sizeof(Part.cJobArray) );
	Part.nHairID = (pSession->GetItem()->GetEquip(EQUIP_HAIR) == NULL) ? 0 : pSession->GetItem()->GetEquip(EQUIP_HAIR)->nItemID;
	Part.nFaceID = (pSession->GetItem()->GetEquip(EQUIP_FACE) == NULL) ? 0 : pSession->GetItem()->GetEquip(EQUIP_FACE)->nItemID;
	Part.dwHairColor = pSession->GetHairColor();
	Part.dwEyeColor = pSession->GetEyeColor();
	Part.dwSkinColor = pSession->GetSkinColor();
	Part.wFatigue = pSession->GetFatigue() + pSession->GetWeeklyFatigue() + pSession->GetPCBangFatigue() + pSession->GetEventFatigue();

	int nEquipID = 0;
	CDNUserItem* pUserItem = pSession->GetItem();
	if (pUserItem == NULL)
		return;

	const TItem* pHelmetEquipItemData = pUserItem->GetEquip(EQUIP_HELMET);
	if (pHelmetEquipItemData != NULL)
		nEquipID = (pHelmetEquipItemData->nLookItemID != 0) ? pHelmetEquipItemData->nLookItemID : pHelmetEquipItemData->nItemID;

	int nCashEquipID = 0;
	const TItem* pCashHelmetEquipItemData = pUserItem->GetCashEquip(CASHEQUIP_HELMET);
	if (pCashHelmetEquipItemData != NULL)
		nCashEquipID = (pCashHelmetEquipItemData->nLookItemID != 0) ? pCashHelmetEquipItemData->nLookItemID : pCashHelmetEquipItemData->nItemID;
	Part.nHelmetID = ( pSession->IsExistViewCashEquipBitmap(CASHEQUIP_HELMET) ) ? nCashEquipID : nEquipID;
	if( pSession->IsExistViewCashEquipBitmap(HIDEHELMET_BITINDEX) ) Part.nHelmetID = 0;

	nEquipID = (pSession->GetItem()->GetEquip(EQUIP_EARRING) == NULL) ? 0 : pSession->GetItem()->GetEquip(EQUIP_EARRING)->nItemID;
	nCashEquipID = (pSession->GetItem()->GetCashEquip(CASHEQUIP_EARRING) == NULL) ? 0 : pSession->GetItem()->GetCashEquip(CASHEQUIP_EARRING)->nItemID;
	Part.nEarringID = ( pSession->IsExistViewCashEquipBitmap(CASHEQUIP_EARRING) ) ? nCashEquipID : nEquipID;

	m_pSession->AddSendData(SC_PARTY, eParty::SC_PARTYMEMBER_PART, (char*)&Part, sizeof(SCPartyMemberPart));
}

void CDNUserSendManager::SendRequestJoinGetReversionItem(const TItem& reversionItem, const DWORD& dropUniqueID)
{
	SCPartyJoinGetReversionItemInfo info;
	memset(&info, 0, sizeof(SCPartyJoinGetReversionItemInfo));

	info.itemInfo = reversionItem;
	info.dropItemUniqueId = dropUniqueID;

	m_pSession->AddSendData(SC_PARTY, eParty::SC_REQUEST_JOINGETREVERSIONITEM, (char*)&info, sizeof(SCPartyJoinGetReversionItemInfo));
}

void CDNUserSendManager::SendAskJoinResult(int nRet)
{
	SCPartyAskJoinResult packet;
	memset(&packet, 0, sizeof(SCPartyAskJoinResult));

	packet.nRet = nRet;

	m_pSession->AddSendData(SC_PARTY, eParty::SC_ASKJOINPARTYRESULT, (char*)&packet, sizeof(SCPartyAskJoinResult));
}

void CDNUserSendManager::SendAskJoinToLeader(const WCHAR * pwszCharacterName, char cJob, BYTE cLevel)
{
	SCPartyAskJoinToLeader packet;
	memset(&packet, 0, sizeof(SCPartyAskJoinToLeader));

	packet.cJob = cJob;
	packet.cLevel = cLevel;
	_wcscpy(packet.wszAskChracterName, _countof(packet.wszAskChracterName), pwszCharacterName, (int)wcslen(pwszCharacterName));

	m_pSession->AddSendData(SC_PARTY, eParty::SC_ASKJOINPARTYTOLEADER, (char*)&packet, sizeof(SCPartyAskJoinToLeader));
}

#if defined( PRE_PARTY_DB )
void CDNUserSendManager::SendAskJoinAgreeInfo(TPARTYID PartyID, int iPassword )
#else
void CDNUserSendManager::SendAskJoinAgreeInfo(TPARTYID PartyID, const WCHAR * pwszPassword)
#endif // #if defined( PRE_PARTY_DB )
{
	SCPartyAskJoinDecisionInfo packet;
	memset(&packet, 0, sizeof(SCPartyAskJoinDecisionInfo));

	packet.PartyID = PartyID;
#if defined( PRE_PARTY_DB )
	packet.iPassword = iPassword;
#else
	_wcscpy(packet.wszPartyPassword, _countof(packet.wszPartyPassword), pwszPassword, (int)wcslen(pwszPassword));
#endif // #if defined( PRE_PARTY_DB )

	m_pSession->AddSendData(SC_PARTY, eParty::SC_ASKJOINPARTYDECISIONINFO, (char*)&packet, sizeof(SCPartyAskJoinDecisionInfo));
}

void CDNUserSendManager::SendUpdatePartyUI(UINT nSessionID, bool bLeader)
{
	SCPartyUpdateFieldUI packet;
	memset(&packet, 0, sizeof(SCPartyUpdateFieldUI));

	packet.nSessionID = nSessionID;
	packet.cLeader = bLeader == true ? 1 : 0;

	m_pSession->AddSendData(SC_PARTY, eParty::SC_PARTYUPDATEFIELDUI, (char*)&packet, sizeof(SCPartyUpdateFieldUI));
}

void CDNUserSendManager::SendPartySwapMemberIndex(CSPartySwapMemberIndex * pPacket, int nRetCode)
{	
	SCPartySwapMemberIndex packet;
	memset(&packet, 0, sizeof(SCPartySwapMemberIndex));

	packet.nRetCode = nRetCode;
	if(nRetCode == ERROR_NONE && pPacket)
	{
		packet.cCount = pPacket->cCount;
		memcpy(packet.Index, pPacket->Index, sizeof(packet.Index));
		if(packet.cCount <= 0)
			packet.nRetCode = ERROR_PARTY_SWAPINDEX_FAIL;		//This should not happen.
	}

	m_pSession->AddSendData(SC_PARTY, eParty::SC_SWAPMEMBERINDEX, (char*)&packet, sizeof(SCPartySwapMemberIndex) - sizeof(packet.Index) +(sizeof(TSwapMemberIndex) * packet.cCount));
}

void CDNUserSendManager::SendRollDiceForGetReversionItem(UINT nSessionID, bool bRoll, int diceValue)
{
	SCRollDiceForGetReverseItem rollInfo;
	memset(&rollInfo, 0, sizeof(SCRollDiceForGetReverseItem));

	rollInfo.nSessionID = nSessionID;
	rollInfo.bRoll = bRoll;
	rollInfo.nDiceValue = diceValue;

	m_pSession->AddSendData(SC_PARTY, eParty::SC_NOTIFY_ROLLGETREVERSIONITEM, (char*)&rollInfo, sizeof(SCRollDiceForGetReverseItem));
}

void CDNUserSendManager::SendResultGetReversionItem(UINT nSessionID, const TItem& itemInfo, DWORD dropItemUniqueId)
{
	SCResultGetReverseItem resultInfo;
	memset(&resultInfo, 0, sizeof(SCResultGetReverseItem));

	resultInfo.nSessionID = nSessionID;
	resultInfo.itemInfo = itemInfo;
	resultInfo.dropItemUniqueId = dropItemUniqueId;

	m_pSession->AddSendData(SC_PARTY, eParty::SC_NOTIFY_RESULTGETREVERSIONITEM, (char*)&resultInfo, sizeof(SCResultGetReverseItem));
}

void CDNUserSendManager::SendReadyRequest()
{
	m_pSession->AddSendData(SC_PARTY, eParty::SC_PARTYREQUEST, NULL, 0);
}

void CDNUserSendManager::SendMemberAbsent(UINT nSessionID, BYTE cAbsentFlag)
{
	SCMemberAbsent packet;
	memset(&packet, 0, sizeof(SCMemberAbsent));

	packet.nSessionID = nSessionID;
	packet.cAbsentFlag = cAbsentFlag;

	m_pSession->AddSendData(SC_PARTY, eParty::SC_ABSENT, (char*)&packet, sizeof(SCMemberAbsent));
}

void CDNUserSendManager::SendVerifyDungeonAlloed(int nMapIdx, BYTE cDifficulty, bool * pAllowed)
{
	SCVerifyDungeon packet;
	memset(&packet, 0, sizeof(SCVerifyDungeon));

	for (int i = 0 ; i < sizeof(packet.cVerifyFlag)/sizeof(*packet.cVerifyFlag); i++)
		packet.cVerifyFlag[i] = (pAllowed[i] == true) ? 1 : 0;

	m_pSession->AddSendData(SC_PARTY, eParty::SC_VERIFYTARGETDUNGEON, (char*)&packet, sizeof(SCMemberAbsent));
}

void CDNUserSendManager::SendPartyBonusValue(int nUpkeepBonusRate, int nFriendBonus, bool bUpkeepMax, int nBeginnerGuildBonus)
{
	SCPartyBonusValue packet;
	memset(&packet, 0, sizeof(SCPartyBonusValue));
	
	packet.cUpKeepBonusLimit = bUpkeepMax == true ? 1 : 0;
	packet.cUpkeepBonus = (BYTE)nUpkeepBonusRate;
	if(m_pSession->GetFriendBonus())
		packet.cFriendBonus = (BYTE)nFriendBonus;
#ifdef PRE_ADD_BEGINNERGUILD
	if (nBeginnerGuildBonus > 0 && m_pSession->CheckBegginerGuild())
		packet.cBeginnerGuildBonus = (BYTE)nBeginnerGuildBonus;
#endif		//#ifdef PRE_ADD_BEGINNERGUILD

	m_pSession->AddSendData(SC_PARTY, eParty::SC_PARTYBONUSVALUE, (char*)&packet, sizeof(SCPartyBonusValue));
}

#if defined(PRE_MOD_REQ_JOIN_PARTY_ANSWER_MSG_APP) && defined(_VILLAGESERVER)
void CDNUserSendManager::SendPartyInfo( int nRetCode, CDNParty* pParty )
{
	SCPartyInfo packet;
	memset(&packet, 0, sizeof(SCPartyInfo));
	packet.nRetCode = ERROR_NONE;

	if(nRetCode == ERROR_NONE)
	{
		if( !pParty || pParty->GetPartyInfo(&packet.PartyInfo) != ERROR_NONE )
			packet.nRetCode = ERROR_PARTY_ASKJOIN_PARTY_NOTFOUND;
	}
	else 
		packet.nRetCode = nRetCode;

	m_pSession->AddSendData( SC_PARTY, eParty::SC_PARTYINFO, reinterpret_cast<char*>(&packet), sizeof(packet) );	
}
#endif

// Item
void CDNUserSendManager::SendEquipList(CDNUserItem *pItem)
{
	SCEquipList Equip;
	memset(&Equip, 0, sizeof(SCEquipList));

#if defined(PRE_ADD_TALISMAN_SYSTEM)
	Equip.nTalismanOpenFlag = pItem->GetTalismanOpenFlag();
	pItem->GetEquipList(Equip.cCount, Equip.cCashCount, Equip.cGlyphCount, Equip.cTalismanCount, Equip.EquipArray);
	m_pSession->AddSendData( SC_ITEM, eItem::SC_EQUIPLIST, (char*)&Equip, sizeof(SCEquipList) - sizeof(Equip.EquipArray) +(sizeof(TItemInfo) *(Equip.cCount+Equip.cCashCount+Equip.cGlyphCount+Equip.cTalismanCount)) );
#else
	pItem->GetEquipList(Equip.cCount, Equip.cCashCount, Equip.cGlyphCount, Equip.EquipArray);
	m_pSession->AddSendData( SC_ITEM, eItem::SC_EQUIPLIST, (char*)&Equip, sizeof(SCEquipList) - sizeof(Equip.EquipArray) +(sizeof(TItemInfo) *(Equip.cCount+Equip.cCashCount+Equip.cGlyphCount)) );
#endif
}

void CDNUserSendManager::SendInvenList(CDNUserItem *pItem)
{
	SCInvenList Inven;
	memset(&Inven, 0, sizeof(SCInvenList));

	Inven.cInvenMaxCount = pItem->GetInventoryCount();
	pItem->GetInventoryList(Inven.Inven, Inven.cInvenCount);

	int nSize = sizeof(SCInvenList) - sizeof(Inven.Inven) + (sizeof(TItemInfo) * Inven.cInvenCount);
	m_pSession->AddSendData(SC_ITEM, eItem::SC_INVENLIST, (char*)&Inven, nSize);
}

void CDNUserSendManager::SendCashInvenList(int nTotalCount, int nCount, TItem *CashItemList)
{
	if(nTotalCount == 0) return;

	SCCashInvenList Inven;
	memset(&Inven, 0, sizeof(SCCashInvenList));

	Inven.nTotalCashInventoryCount = nTotalCount;
	if(CashItemList){
		Inven.cInvenCount = nCount;
		memcpy(Inven.Inven, CashItemList, sizeof(TItem) * Inven.cInvenCount);
	}

	int nSize = sizeof(SCCashInvenList) - sizeof(Inven.Inven) + (sizeof(TItem) * Inven.cInvenCount);
	m_pSession->AddSendData( SC_ITEM, eItem::SC_CASHINVENLIST, (char*)&Inven, nSize );
}

#if defined(_VILLAGESERVER) && defined(PRE_ADD_CASH_REFUND)
void CDNUserSendManager::SendPaymentItemList(CDNUserItem *pItem)
{
	if( pItem->IsPaymentItemEmpty() )
		return;

	int nTotalSize = pItem->GetPaymentItemCount();
	int nTotalPage = nTotalSize/CASHINVENTORYMAX;
	if( nTotalSize%CASHINVENTORYMAX > 0 )
		++nTotalPage;

	SCPaymentList PaymentList;
	for( int i=0; i<nTotalPage; ++ i)
	{
		memset(&PaymentList, 0, sizeof(SCPaymentList));
		pItem->MakeSendPaymentItem(&PaymentList, i);
		int nSize = sizeof(int) + sizeof(BYTE) +(sizeof(TPaymentItemInfo)*PaymentList.cInvenCount);
		m_pSession->AddSendData( SC_CASHSHOP, eCashShop::SC_PAYMENT_LIST, (char*)&PaymentList, nSize);
	}
}

void CDNUserSendManager::SendPaymentPackageItemList(CDNUserItem *pItem)
{
	if( pItem->IsPaymentPackageItemEmpty() )
		return;

	int nTotalSize = pItem->GetPaymentPackageItemCount();
	int nTotalPage = nTotalSize/CASHINVENTORYMAX;
	if( nTotalSize%CASHINVENTORYMAX > 0 )
		++nTotalPage;

	SCPaymentPackageList PaymentPackageList;
	for( int i=0; i<nTotalPage; ++ i)
	{
		memset(&PaymentPackageList, 0, sizeof(SCPaymentPackageList));
		pItem->MakeSendPaymentPackageItem(&PaymentPackageList, i);
		int nSize = sizeof(int) + sizeof(BYTE) +(sizeof(TPaymentPackageItemInfo)*PaymentPackageList.cInvenCount);
		m_pSession->AddSendData( SC_CASHSHOP, eCashShop::SC_PAYMENT_PACKAGELIST, (char*)&PaymentPackageList, nSize);
	}
}
#endif

void CDNUserSendManager::SendMoveItem(char cType, BYTE cSrcIndex, BYTE cDestIndex, const TItem *pSrcItem, const TItem *pDestItem, int nRetCode)
{
	SCMoveItem Move;
	memset(&Move, 0, sizeof(SCMoveItem));

	Move.cMoveType = cType;
	Move.nRetCode = nRetCode;

	if(nRetCode == ERROR_NONE){
		Move.SrcItem.cSlotIndex = cSrcIndex;
		if(pSrcItem) Move.SrcItem.Item = *pSrcItem;

		Move.DestItem.cSlotIndex = cDestIndex;
		if(pDestItem) Move.DestItem.Item = *pDestItem;

		m_pSession->AddSendData(SC_ITEM, eItem::SC_MOVEITEM, (char*)&Move, sizeof(SCMoveItem));
	}
	else
		m_pSession->AddSendData(SC_ITEM, eItem::SC_MOVEITEM, (char*)&Move, (int)(sizeof(Move) -(sizeof(Move.DestItem) + sizeof(Move.SrcItem))));	
}

void CDNUserSendManager::SendMoveCashItem(char cType, BYTE cEquipIndex, const TItem *pEquipItem, INT64 biInvenSerial, const TItem *pInvenItem, int nRetCode)
{
	SCMoveCashItem Move;
	memset(&Move, 0, sizeof(SCMoveCashItem));

	Move.cMoveType = cType;
	Move.nRetCode = nRetCode;

	if(nRetCode == ERROR_NONE){
		Move.CashEquipItem.cSlotIndex = cEquipIndex;
		if(pEquipItem)
			Move.CashEquipItem.Item = *pEquipItem;
		if(pInvenItem)
			Move.CashInvenItem = *pInvenItem;
		else
			Move.CashInvenItem.nSerial = biInvenSerial;

		m_pSession->AddSendData(SC_ITEM, eItem::SC_MOVECASHITEM, (char*)&Move, sizeof(SCMoveCashItem));
	}
	else
		m_pSession->AddSendData(SC_ITEM, eItem::SC_MOVECASHITEM, (char*)&Move, (int)(sizeof(SCMoveCashItem) -(sizeof(Move.CashEquipItem) + sizeof(Move.CashInvenItem))));	
}

#if defined(PRE_ADD_SERVER_WAREHOUSE)
void CDNUserSendManager::SendServerWareHouseList(CDNUserItem* pItem)
{
	SCServerWareList Ware;
	memset(&Ware, 0, sizeof(SCServerWareList));
	Ware.cWareCount = pItem->CopySendServerWare(&Ware.WareItems[0]);

	int nSize = sizeof(SCServerWareList) - sizeof(Ware.WareItems) +(sizeof(TItem) * Ware.cWareCount);
	m_pSession->AddSendData( SC_ITEM, eItem::SC_SERVERWARE_LIST, (char*)&Ware, nSize );
}

void CDNUserSendManager::SendServerWareHouseCashList(CDNUserItem* pItem)
{
	SCCashServerWareList Ware;
	memset(&Ware, 0, sizeof(SCCashServerWareList));
	Ware.cWareCount = pItem->CopySendServerWareCash(&Ware.WareCashItems[0]);

	int nSize = sizeof(SCCashServerWareList) - sizeof(Ware.WareCashItems) +(sizeof(TItem) * Ware.cWareCount);
	m_pSession->AddSendData( SC_ITEM, eItem::SC_CASHSERVERWARE_LIST, (char*)&Ware, nSize );
}

void CDNUserSendManager::SendMoveServerWareItem(char cType, BYTE cInvenIndex, INT64 biWareSerial, const TItem *pInvenItem, const TItem *pServerWareItem, int nRetCode)
{
	SCMoveServerWare Move;
	memset(&Move, 0, sizeof(SCMoveServerWare));

	Move.cMoveType = cType;
	Move.nRetCode = nRetCode;

	if(nRetCode == ERROR_NONE)
	{
		Move.InvenItem.cSlotIndex = cInvenIndex;
		if(pInvenItem) Move.InvenItem.Item = *pInvenItem;		
		
		if(pServerWareItem) Move.ServerWareItem = *pServerWareItem;
		else
			Move.ServerWareItem.nSerial = biWareSerial;

		m_pSession->AddSendData(SC_ITEM, eItem::SC_MOVE_SERVERWARE, (char*)&Move, sizeof(SCMoveServerWare));
	}
	else
		m_pSession->AddSendData(SC_ITEM, eItem::SC_MOVE_SERVERWARE, (char*)&Move, (int)(sizeof(Move) -(sizeof(Move.InvenItem) + sizeof(Move.ServerWareItem))));	
}

void CDNUserSendManager::SendMoveServerWareCashItem(char cType, INT64 biSrcItemSerial, const TItem *pSrcItem, const TItem *pDstItem, int nRetCode)
{
	SCMoveCashServerWare Move;
	memset(&Move, 0, sizeof(SCMoveCashServerWare));

	Move.cMoveType = cType;
	Move.nRetCode = nRetCode;

	if(nRetCode == ERROR_NONE){		
		if(pSrcItem)		
			Move.SrcItem = *pSrcItem;
		else
			Move.biSrcItemSerial = biSrcItemSerial;

		if(pDstItem)
			Move.DestItem = *pDstItem;		

		m_pSession->AddSendData(SC_ITEM, eItem::SC_MOVE_CASHSERVERWARE, (char*)&Move, sizeof(SCMoveCashServerWare));
	}
	else
		m_pSession->AddSendData(SC_ITEM, eItem::SC_MOVE_CASHSERVERWARE, (char*)&Move, (int)(sizeof(SCMoveCashServerWare) -(sizeof(Move.SrcItem) + sizeof(Move.DestItem))));
}
#endif //#if defined(PRE_ADD_SERVER_WAREHOUSE)

void CDNUserSendManager::SendRefreshInvenPickUpByType(char cType, int nRetCode, BYTE cInvenIndex, TItem *pPickUpItem, short wCount)
{
	switch(cType) {
		case CREATEINVEN_PICKUP:
		case CREATEINVEN_PICKUP_DICE:
			{
				SendPickUp(nRetCode, cInvenIndex, pPickUpItem, wCount);
			}
			break;

		case CREATEINVEN_QUEST:
		case CREATEINVEN_ETC:
		default:
			{
				SendRefreshInven(cInvenIndex, pPickUpItem, true);
			}
			break;
	}
}

void CDNUserSendManager::SendPickUp(int nRetCode, BYTE cInvenIndex, TItem *pPickUpItem, short wCount )
{
	SCPickUp PickUp;
	memset(&PickUp, 0, sizeof(SCPickUp));

	PickUp.nRetCode = nRetCode;
	PickUp.wCount = wCount;

	if(pPickUpItem){
		PickUp.PickUpItem.cSlotIndex = cInvenIndex;
		PickUp.PickUpItem.Item = *pPickUpItem;

		m_pSession->AddSendData(SC_ITEM, eItem::SC_PICKUP, (char*)&PickUp, sizeof(SCPickUp));
#if defined(_GAMESERVER)
		m_pSession->GetGameRoom()->OnDispatchMessage( m_pSession, IN_SC_ITEM, eItem::SC_PICKUP, (char*)&PickUp, sizeof(SCPickUp), 0 );
#endif
	}
	else{
		m_pSession->AddSendData(SC_ITEM, eItem::SC_PICKUP, (char*)&PickUp, (int)(sizeof(short)));
#if defined(_GAMESERVER)
		m_pSession->GetGameRoom()->OnDispatchMessage( m_pSession, IN_SC_ITEM, eItem::SC_PICKUP, (char*)&PickUp, (int)(sizeof(short)), 0 );
#endif
	}
}

void CDNUserSendManager::SendRemoveItem(char cType, BYTE cSrcIndex, TItem *pRemoveItemItem, int nRetCode)
{
	SCRemoveItem RemoveItem;
	memset(&RemoveItem, 0, sizeof(SCRemoveItem));

	RemoveItem.cType = cType;
	RemoveItem.nRetCode = nRetCode;
	
	if(nRetCode == ERROR_NONE){
		RemoveItem.ItemInfo.cSlotIndex = cSrcIndex;
		RemoveItem.ItemInfo.Item = *pRemoveItemItem;

		m_pSession->AddSendData(SC_ITEM, eItem::SC_REMOVEITEM, (char*)&RemoveItem, sizeof(SCRemoveItem));

#if defined(_GAMESERVER)
		if(cType == ITEMPOSITION_QUESTINVEN)	// Only when it is quest inventory
		{
			m_pSession->GetGameRoom()->OnDispatchMessage( m_pSession, IN_SC_ITEM, eItem::SC_REMOVEQUESTITEM, (char*)&RemoveItem, sizeof(SCRemoveItem), 0 );
		}
#endif
	}
	else 
		m_pSession->AddSendData(SC_ITEM, eItem::SC_REMOVEITEM, (char*)&RemoveItem, (int)(sizeof(short)+sizeof(char)));
}

void CDNUserSendManager::SendChangeEquip(UINT nSessionID, TItemInfo &ItemInfo)
{
	SCChangeEquip ChangeEquip;
	memset(&ChangeEquip, 0, sizeof(SCChangeEquip));

	ChangeEquip.nSessionID = nSessionID;
	ChangeEquip.ItemInfo = ItemInfo;

	m_pSession->AddSendData(SC_ITEM, eItem::SC_CHANGEEQUIP, (char*)&ChangeEquip, sizeof(SCChangeEquip));
}

void CDNUserSendManager::SendChangeCashEquip(UINT nSessionID, TItemInfo &ItemInfo)
{
	SCChangeCashEquip ChangeEquip;
	memset(&ChangeEquip, 0, sizeof(SCChangeCashEquip));

	ChangeEquip.nSessionID = nSessionID;
	ChangeEquip.ItemInfo = ItemInfo;

	m_pSession->AddSendData(SC_ITEM, eItem::SC_CHANGECASHEQUIP, (char*)&ChangeEquip, sizeof(SCChangeCashEquip));
}

void CDNUserSendManager::SendRefreshInven(BYTE cInvenIndex, const TItem *pInvenItem, bool bNewSign)
{
	SCRefreshInven RefreshInven;
	memset(&RefreshInven, 0, sizeof(SCRefreshInven));

	RefreshInven.bNewSign = bNewSign;
	RefreshInven.ItemInfo.cSlotIndex = cInvenIndex;
	if(pInvenItem) RefreshInven.ItemInfo.Item = *pInvenItem;

	int nLen = sizeof(SCRefreshInven);
	m_pSession->AddSendData(SC_ITEM, eItem::SC_REFRESHINVEN, (char*)&RefreshInven, nLen);

#if defined(_GAMESERVER)
	m_pSession->GetGameRoom()->OnDispatchMessage( m_pSession, IN_SC_ITEM, eItem::SC_REFRESHINVEN, (char*)&RefreshInven, nLen, 0 );
#endif
}

void CDNUserSendManager::SendRefreshCashInven(const TItem &InvenItem, bool bNewSign/* = true*/)
{
	SCRefreshCashInven RefreshInven;
	memset(&RefreshInven, 0, sizeof(SCRefreshCashInven));

	RefreshInven.bNewSign = bNewSign;
	RefreshInven.nCount = 1;
	RefreshInven.ItemList[0] = InvenItem;

	int nLen = sizeof(SCRefreshCashInven) - sizeof(RefreshInven.ItemList) +(sizeof(TItem) * RefreshInven.nCount);
	m_pSession->AddSendData(SC_ITEM, eItem::SC_REFRESHCASHINVEN, (char*)&RefreshInven, nLen);

#if defined(_GAMESERVER)
	m_pSession->GetGameRoom()->OnDispatchMessage( m_pSession, IN_SC_ITEM, eItem::SC_REFRESHCASHINVEN, (char*)&RefreshInven, nLen, 0 );
#endif
}

void CDNUserSendManager::SendRefreshCashInven(std::vector<TItem> VecItemList, bool bNewSign/* = true*/)
{
	SCRefreshCashInven RefreshInven;
	memset(&RefreshInven, 0, sizeof(SCRefreshCashInven));

	RefreshInven.bNewSign = bNewSign;
	RefreshInven.nCount = (int)VecItemList.size();
	for (int i = 0; i < RefreshInven.nCount; i++){
		RefreshInven.ItemList[i] = VecItemList[i];
	}

	int nLen = sizeof(SCRefreshCashInven) - sizeof(RefreshInven.ItemList) +(sizeof(TItem) * RefreshInven.nCount);
	m_pSession->AddSendData(SC_ITEM, eItem::SC_REFRESHCASHINVEN, (char*)&RefreshInven, nLen);

#if defined(_GAMESERVER)
	m_pSession->GetGameRoom()->OnDispatchMessage( m_pSession, IN_SC_ITEM, eItem::SC_REFRESHCASHINVEN, (char*)&RefreshInven, nLen, 0 );
#endif
}

#if defined(PRE_ADD_EQUIPED_ITEM_ENCHANT)
void CDNUserSendManager::SendRefreshEquip(char cEquipIndex, const TItem *pEquipItem, bool bRefreshtate/*=false*/)
#else
void CDNUserSendManager::SendRefreshEquip(char cEquipIndex, const TItem *pEquipItem)
#endif
{
	SCRefreshEquip RefreshEquip;
	memset( &RefreshEquip, 0, sizeof(SCRefreshEquip) );

	RefreshEquip.ItemInfo.cSlotIndex = cEquipIndex;
	if(pEquipItem) RefreshEquip.ItemInfo.Item = *pEquipItem;
#if defined(PRE_ADD_EQUIPED_ITEM_ENCHANT)
	RefreshEquip.bRefreshState = bRefreshtate;
#endif
	m_pSession->AddSendData(SC_ITEM, eItem::SC_REFRESHEQUIP, (char*)&RefreshEquip, sizeof(SCRefreshEquip));

#if defined(_GAMESERVER)
	m_pSession->GetGameRoom()->OnDispatchMessage( m_pSession, IN_SC_ITEM, eItem::SC_REFRESHEQUIP, (char*)&RefreshEquip, sizeof(SCRefreshEquip), 0 );
#endif
}

void CDNUserSendManager::SendRefreshWarehouse(BYTE cWarehouseIndex, const TItem *pWarehouseItem)
{
	SCRefreshWarehouse Refreshwarehouse;
	memset(&Refreshwarehouse, 0, sizeof(SCRefreshWarehouse));

	Refreshwarehouse.ItemInfo.cSlotIndex = cWarehouseIndex;
	if(pWarehouseItem) Refreshwarehouse.ItemInfo.Item = *pWarehouseItem;

	int nLen = sizeof(SCRefreshWarehouse);
	m_pSession->AddSendData(SC_ITEM, eItem::SC_REFRESHWAREHOUSE, (char*)&Refreshwarehouse, nLen);
}

void CDNUserSendManager::SendWarehouseList(INT64 nWarehouseCoin, CDNUserItem *pItem)
{
	SCWarehouseList Ware;
	memset(&Ware, 0, sizeof(SCWarehouseList));

	Ware.nWarehouseCoin = nWarehouseCoin;
	Ware.cWareMaxCount = pItem->GetWarehouseCount();

	pItem->GetWarehouseList(Ware.Warehouse, Ware.cWareCount);

	int nSize = sizeof(SCWarehouseList) - sizeof(Ware.Warehouse) + (sizeof(TItemInfo) * Ware.cWareMaxCount);
	m_pSession->AddSendData(SC_ITEM, eItem::SC_WAREHOUSELIST, (char*)&Ware, nSize);
}

void CDNUserSendManager::SendUseItem(int nInvenType, BYTE cInvenIndex, INT64 biItemSerial)
{
	SCUseItem UseItem = { 0, };
	UseItem.cInvenType = nInvenType;
	UseItem.cInvenIndex = cInvenIndex;
	UseItem.biInvenSerial = biItemSerial;
	m_pSession->AddSendData( SC_ITEM, eItem::SC_USEITEM, (char*)&UseItem, sizeof(SCUseItem) );
}

void CDNUserSendManager::SendEmblemCompoundOpen( int nRet )
{
	SCEmblemCompundOpenRes Result;
	Result.nResult = nRet;
	m_pSession->AddSendData( SC_ITEM, eItem::SC_EMBLEM_COMPOUND_OPEN_RES, (char*)&Result, sizeof(SCEmblemCompundOpenRes) );
}

void CDNUserSendManager::SendItemCompoundOpen( int nRet )
{
	SCItemCompoundOpenRes Result;
	Result.nResult = nRet;
	m_pSession->AddSendData( SC_ITEM, eItem::SC_ITEM_COMPOUND_OPEN_RES, (char*)&Result, sizeof(SCItemCompoundOpenRes) );
}


void CDNUserSendManager::SendEmblemCompoundRes( int nSuccess, int nResultItemID )
{
	SCCompoundEmblemRes packet;
	packet.sResult = (short)nSuccess;
	packet.nResultItemID = nResultItemID;

	m_pSession->AddSendData( SC_ITEM, eItem::SC_EMBLEM_COMPOUND_RES, (char*)&packet, sizeof(SCCompoundEmblemRes) );
}


void CDNUserSendManager::SendItemCompoundRes( int nSuccess, int nResultItemID, char cLevel, char cOption )
{
	SCCompoundItemRes packet;
	packet.sResult = (short)nSuccess;
	packet.nResultItemID = nResultItemID;
	packet.cLevel = cLevel;
	packet.cOption = cOption;

	m_pSession->AddSendData( SC_ITEM, eItem::SC_ITEM_COMPOUND_RES, (char*)&packet, sizeof(SCCompoundItemRes) );
}


void CDNUserSendManager::SendEmblemCompoundCancel( int nRet )
{
	SCEmblemCompoundCancelRes Result;
	Result.nResult = nRet;
	m_pSession->AddSendData( SC_ITEM, eItem::SC_EMBLEM_COMPOUND_CANCEL_RES, (char*)&Result, sizeof(SCEmblemCompoundCancelRes) );
}


void CDNUserSendManager::SendItemCompoundCancel( int nRet )
{
	SCItemCompoundCancelRes Result;
	Result.nResult = nRet;
	m_pSession->AddSendData( SC_ITEM, eItem::SC_ITEM_COMPOUND_CANCEL_RES, (char*)&Result, sizeof(SCItemCompoundCancelRes) );
}

void CDNUserSendManager::SendInventoryMaxCount(BYTE cInvenMaxCount)
{
	SCInventoryMaxCount MaxCount = { 0, };
	MaxCount.cInvenMaxCount = cInvenMaxCount;
	m_pSession->AddSendData(SC_ITEM, eItem::SC_INVENTORYMAXCOUNT, (char*)&MaxCount, sizeof(SCInventoryMaxCount));
}

void CDNUserSendManager::SendGlyphExtendCount( int iCount )
{
	SCGlyphExtendCount TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.cCount = static_cast<BYTE>(iCount);

	m_pSession->AddSendData(SC_ITEM, eItem::SC_GLYPHEXTENDCOUNT, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

void CDNUserSendManager::SendGlyphExpireData( TCashGlyphData *GlyphExpireData )
{
	SCGlyphExpireData TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	BYTE count = 0;
	for (int i=0;i<CASHGLYPHSLOTMAX;i++)
	{
		if(GlyphExpireData[i].cActiveGlyph)
		{
			TxPacket.TGlyphData[count].cActiveGlyph = GlyphExpireData[i].cActiveGlyph;
			TxPacket.TGlyphData[count].tGlyphExpireDate = GlyphExpireData[i].tGlyphExpireDate;
			count++;
		}				
	}	

	TxPacket.cCount = count;

	int nSize = sizeof(SCGlyphExpireData) - sizeof(TxPacket.TGlyphData) +(sizeof(TCashGlyphData) * TxPacket.cCount);	

	m_pSession->AddSendData(SC_ITEM, eItem::SC_GLYPHEXPIREDATA, reinterpret_cast<char*>(&TxPacket), nSize );
}

void CDNUserSendManager::SendWarehouseMaxCount(BYTE cWareMaxCount)
{
	SCWarehouseMaxCount MaxCount = { 0, };
	MaxCount.cWareMaxCount = cWareMaxCount;
	m_pSession->AddSendData(SC_ITEM, eItem::SC_WAREHOUSEMAXCOUNT, (char*)&MaxCount, sizeof(SCWarehouseMaxCount));
}

void CDNUserSendManager::SendRebirthCoin(int nRet, int nUsableRebirthCoin, eRebirthType type, UINT nSessionID)
{
	SCRebirthCoin Coin = { 0, };

	Coin.nRet = nRet;
	Coin.nUsableRebirthCoin = nUsableRebirthCoin;
	Coin.cRebirthCoin = m_pSession->GetRebirthCoin();
	Coin.cPCBangRebirthCoin = m_pSession->GetPCBangRebirthCoin();
	Coin.wRebirthCashCoin = m_pSession->GetRebirthCashCoin();
	Coin.nRebirthType = type;
	Coin.nSessionID = nSessionID;
#if defined(PRE_ADD_VIP)
	Coin.cVIPRebirthCoin = m_pSession->GetVIPRebirthCoin();
#endif	// #if defined(PRE_ADD_VIP)

	m_pSession->AddSendData(SC_ITEM, eItem::SC_REBIRTH_COIN, (char*)&Coin, sizeof(SCRebirthCoin));
#ifdef _GAMESERVER
	if(nRet == ERROR_NONE && m_pSession->GetGameRoom())
		m_pSession->GetGameRoom()->SendRefreshRebirthCoin(m_pSession->GetSessionID(), m_pSession->GetRebirthCoin(), m_pSession->GetPCBangRebirthCoin(), m_pSession->GetRebirthCashCoin(), m_pSession->GetVIPRebirthCoin());
#endif	// _GAMESERVER
}

void CDNUserSendManager::SendRefreshRebirthCoin(UINT nSessionID, BYTE cRebirthCoin, BYTE cPCBangRebirthCoin, short nCashRebirthCoin, BYTE cVIPRebirthCoin)
{
	SCRefreshRebirthCoin packet;
	memset(&packet, 0, sizeof(packet));

	packet.nSessionID = nSessionID;
	packet.cRebirthCoin = cRebirthCoin;
	packet.cPCBangRebirthCoin = cPCBangRebirthCoin;
	packet.nCashRebirthCoin = nCashRebirthCoin;
#if defined(PRE_ADD_VIP)
	packet.cVIPRebirthCoin = cVIPRebirthCoin;
#endif	// #if defined(PRE_ADD_VIP)

	m_pSession->AddSendData(SC_PARTY, eParty::SC_REFRESHREBIRTHCOIN, (char*)&packet, sizeof(packet));
}

#if defined(PRE_ADD_REBIRTH_EVENT)
void CDNUserSendManager::SendRebirthMaxCoin(int nRebirthMaxCoin)
{
	SCRebirthMaxCoin packet;
	memset(&packet, 0, sizeof(packet));

	packet.nRebirthMaxCoin = nRebirthMaxCoin;

	m_pSession->AddSendData(SC_CHAR, eChar::SC_REBIRTH_MAXCOIN, (char*)&packet, sizeof(packet));
}
#endif

void CDNUserSendManager::SendQuestInvenList(CDNUserItem *pItem)
{
	SCQuestInvenList QuestInvenList = { 0, };

	int nCount = 0;
	for (int i = 0; i < QUESTINVENTORYMAX; i++){
		if(!pItem->GetQuestInventory(i)) continue;
		QuestInvenList.QuestInventory[nCount].cSlotIndex = i;
		QuestInvenList.QuestInventory[nCount].Item.nItemID = pItem->GetQuestInventory(i)->nItemID;
		QuestInvenList.QuestInventory[nCount].Item.nSerial = pItem->GetQuestInventory(i)->nSerial;
		QuestInvenList.QuestInventory[nCount].Item.wCount = pItem->GetQuestInventory(i)->wCount;
		nCount++;
	}
	QuestInvenList.cQuestInvenCount = nCount;

	int nSize = sizeof(char) +(sizeof(TItemInfo) * nCount);
	m_pSession->AddSendData(SC_ITEM, eItem::SC_QUESTINVENLIST, (char*)&QuestInvenList, nSize);
}

void CDNUserSendManager::SendRefreshQuestInven(BYTE cIndex, TQuestItem &QuestItem)
{
	SCRefreshQuestInven RefreshInven;
	memset(&RefreshInven, 0, sizeof(SCRefreshQuestInven));

	RefreshInven.ItemInfo.cSlotIndex = cIndex;
	RefreshInven.ItemInfo.Item.nItemID = QuestItem.nItemID;
	RefreshInven.ItemInfo.Item.nSerial = QuestItem.nSerial;
	RefreshInven.ItemInfo.Item.wCount = QuestItem.wCount;

	int nLen = sizeof(SCRefreshQuestInven);
	m_pSession->AddSendData(SC_ITEM, eItem::SC_REFRESHQUESTINVEN, (char*)&RefreshInven, nLen);
#if defined(_GAMESERVER)
	m_pSession->GetGameRoom()->OnDispatchMessage( m_pSession, IN_SC_ITEM, eItem::SC_REFRESHQUESTINVEN, (char*)&RefreshInven, nLen, 0 );
#endif
}

void CDNUserSendManager::SendRepairEquip(int nRet)
{
	SCRepairResult Result;
	memset(&Result, 0, sizeof(SCRepairResult));

	Result.nRet = nRet;
	m_pSession->AddSendData(SC_ITEM, eItem::SC_REPAIR_EQUIP, (char*)&Result, sizeof(SCRepairResult));
}

void CDNUserSendManager::SendRepairAll(int nRet)
{
	SCRepairResult Result;
	memset(&Result, 0, sizeof(SCRepairResult));

	Result.nRet = nRet;
	m_pSession->AddSendData(SC_ITEM, eItem::SC_REPAIR_ALL, (char*)&Result, sizeof(SCRepairResult));
}

void CDNUserSendManager::SendMoveCoin(char cType, INT64 nAddCoin, INT64 nInventoryCoin, INT64 nWarehouseCoin, int nRet)
{
	SCMoveCoin Coin;
	memset(&Coin, 0, sizeof(SCMoveCoin));

	Coin.cMoveType = cType;
	Coin.nMoveCoin = nAddCoin;
	Coin.nInventoryCoin = nInventoryCoin;
	Coin.nWarehouseCoin = nWarehouseCoin;
	Coin.nRet = nRet;

	m_pSession->AddSendData(SC_ITEM, eItem::SC_MOVECOIN, (char*)&Coin, sizeof(SCMoveCoin));
}

void CDNUserSendManager::SendRequestRandomItem(int nInvenType, BYTE cInvenIndex, INT64 biItemSerial, short nRetCode)
{
	SCRequestRandomItem RandomItem;
	memset( &RandomItem, 0, sizeof(SCRequestRandomItem) );

	RandomItem.cInvenType = nInvenType;
	RandomItem.cInvenIndex = cInvenIndex;
	RandomItem.biInvenSerial = biItemSerial;
	RandomItem.nRet = nRetCode;

	m_pSession->AddSendData( SC_ITEM, eItem::SC_REQUEST_RANDOMITEM, (char*)&RandomItem, sizeof(SCRequestRandomItem) );
}

void CDNUserSendManager::SendCompleteRandomItem( int nInvenType, char cLevel, int nResultItemID, int nItemCount, short nRetCode )
{
	SCCompleteRandomItem RandomItem;
	memset( &RandomItem, 0, sizeof(SCCompleteRandomItem) );

	RandomItem.cInvenType = nInvenType;
	RandomItem.cLevel = cLevel;
	RandomItem.nResultItemID = nResultItemID;
	RandomItem.nItemCount = nItemCount;
	RandomItem.nRet = nRetCode;

	m_pSession->AddSendData( SC_ITEM, eItem::SC_COMPLETE_RANDOMITEM, (char*)&RandomItem, sizeof(SCCompleteRandomItem) );
}

// Enhance
#if defined(PRE_ADD_EQUIPED_ITEM_ENCHANT)
void CDNUserSendManager::SendEnchant(int nItemIndex, bool bEnchantEquipedItem, int nRet)
#else
void CDNUserSendManager::SendEnchant(int nInvenIndex, int nRet)
#endif
{
	SCEnchantItem Result;
	memset(&Result, 0, sizeof(SCEnchantItem));
#if defined(PRE_ADD_EQUIPED_ITEM_ENCHANT)
	Result.cItemIndex = nItemIndex;
	Result.bEnchantEquipedItem = bEnchantEquipedItem;
#else
	Result.cInvenIndex = nInvenIndex;
#endif
	Result.nRet = nRet;
	m_pSession->AddSendData(SC_ITEM, eItem::SC_ENCHANT, (char*)&Result, sizeof(SCEnchantItem));	
}

void CDNUserSendManager::SendEnchantComplete(int nItemID, char cLevel, char cOption, int nRet)
{
	SCEnchantComplete Result;
	memset(&Result, 0, sizeof(SCEnchantComplete));
	Result.nItemID = nItemID;
	Result.cLevel = cLevel;
	Result.cOption = cOption;
	Result.nRet = nRet;
	m_pSession->AddSendData(SC_ITEM, eItem::SC_ENCHANTCOMPLETE, (char*)&Result, sizeof(SCEnchantComplete));	
}

void CDNUserSendManager::SendEnchantCancel(int nRet)
{
	SCEnchantCancel Result;
	memset(&Result, 0, sizeof(SCEnchantCancel));
	Result.nRet = nRet;
	m_pSession->AddSendData(SC_ITEM, eItem::SC_ENCHANTCANCEL, (char*)&Result, sizeof(SCEnchantCancel));	
}

void CDNUserSendManager::SendCosMixOpen(int nRet)
{
	SCCosMixOpen Result;
	memset(&Result, 0, sizeof(SCCosMixOpen));
	Result.nRet = nRet;
	m_pSession->AddSendData(SC_ITEMGOODS, eItemGoods::SC_COSMIXOPEN, (char*)&Result, sizeof(SCCosMixOpen));
}

void CDNUserSendManager::SendCosMixClose(int nRet, int nSubCmd)
{
	SCCosMixClose Result;
	memset(&Result, 0, sizeof(SCCosMixClose));
	Result.nRet = nRet;

	m_pSession->AddSendData(SC_ITEMGOODS, eItemGoods::SC_COSMIXCLOSE, (char*)&Result, sizeof(SCCosMixClose));
}

void CDNUserSendManager::SendCosMixComplete(int nRet, int nItemId, char cOption)
{
	SCCosMix result;
	memset(&result, 0, sizeof(SCCosMix));
	result.nRet = nRet;
	result.nResultItemID = nItemId;
	result.cOption = cOption;
	m_pSession->AddSendData(SC_ITEMGOODS, eItemGoods::SC_COSMIXCOMPLETE, (char*)&result, sizeof(SCCosMix));
}

void CDNUserSendManager::SendCosDesignMixOpen(int nRet)
{
	SCCosDesignMixOpen Result;
	memset(&Result, 0, sizeof(SCCosDesignMixOpen));
	Result.nRet = nRet;
	m_pSession->AddSendData(SC_ITEMGOODS, eItemGoods::SC_DESIGNMIXOPEN, (char*)&Result, sizeof(SCCosDesignMixOpen));
}

void CDNUserSendManager::SendCosDesignMixClose(int nRet, int nSubCmd)
{
	SCCosDesignMixClose Result;
	memset(&Result, 0, sizeof(SCCosDesignMixClose));
	Result.nRet = nRet;

	eItemGoods::eSCItemGoods prtcl = (nSubCmd == eItemGoods::CS_DESIGNMIXCOMPLETECLOSE) ? eItemGoods::SC_DESIGNMIXCOMPLETECLOSE : eItemGoods::SC_DESIGNMIXCLOSE;
	m_pSession->AddSendData(SC_ITEMGOODS, prtcl, (char*)&Result, sizeof(SCCosDesignMixClose));
}

void CDNUserSendManager::SendCosDesignMixComplete(int nRet, INT64 biItemSertial, int nLookItemID)
{
	SCCosDesignMix result;
	memset(&result, 0, sizeof(SCCosDesignMix));
	result.nRet = nRet;
	result.biItemSertial = biItemSertial;
	result.nLookItemID = nLookItemID;
	m_pSession->AddSendData(SC_ITEMGOODS, eItemGoods::SC_DESIGNMIXCOMPLETE, (char*)&result, sizeof(SCCosDesignMix));
}

#ifdef PRE_ADD_COSRANDMIX

#ifdef PRE_ADD_COSRANDMIX_ACCESSORY
void CDNUserSendManager::SendCosRandomMixOpen(int nRet, CostumeMix::RandomMix::eOpenType type)
#else
void CDNUserSendManager::SendCosRandomMixOpen(int nRet)
#endif
{
	SCCosRandomMixOpen Result;
	memset(&Result, 0, sizeof(SCCosRandomMixOpen));
	Result.nRet = nRet;
#ifdef PRE_ADD_COSRANDMIX_ACCESSORY
	Result.nOpenType = (int)type;
#endif
	m_pSession->AddSendData(SC_ITEMGOODS, eItemGoods::SC_RANDOMMIXOPEN, (char*)&Result, sizeof(SCCosRandomMixOpen));
}

void CDNUserSendManager::SendCosRandomMixClose(int nRet, int nSubCmd)
{
	SCCosRandomMixClose Result;
	memset(&Result, 0, sizeof(SCCosRandomMixClose));
	Result.nRet = nRet;

	eItemGoods::eSCItemGoods prtcl = (nSubCmd == eItemGoods::CS_RANDOMMIXCOMPLETECLOSE) ? eItemGoods::SC_RANDOMMIXCOMPLETECLOSE : eItemGoods::SC_RANDOMMIXCLOSE;
	m_pSession->AddSendData(SC_ITEMGOODS, prtcl, (char*)&Result, sizeof(SCCosRandomMixClose));
}

void CDNUserSendManager::SendCosRandomMixComplete(int nRet, int nResultItemId)
{
	SCCosRandomMix result;
	memset(&result, 0, sizeof(SCCosRandomMix));
	result.nRet = nRet;
	result.nResultItemID = nResultItemId;
	m_pSession->AddSendData(SC_ITEMGOODS, eItemGoods::SC_RANDOMMIXCOMPLETE, (char*)&result, sizeof(SCCosRandomMix));
}
#endif // PRE_ADD_COSRANDMIX

void CDNUserSendManager::SendPotentialItemResult( int nRet, int nSlotIndex )
{
	SCPotentialItem result;
	result.nRet = nRet;
	result.cInvenIndex = nSlotIndex;

	m_pSession->AddSendData(SC_ITEMGOODS, eItemGoods::SC_POTENTIALJEWEL, (char*)&result, sizeof(SCPotentialItem));
}
#if defined(PRE_MOD_POTENTIAL_JEWEL_RENEWAL)
void CDNUserSendManager::SendPotentialItemRollbackResult(int nRet)
{
	SCPotentialItemRollback result;
	result.nRet = nRet;

	m_pSession->AddSendData(SC_ITEMGOODS, eItemGoods::SC_POTENTIALJEWEL_ROLLBACK, (char*)&result, sizeof(SCPotentialItemRollback));
}
#endif	// #if defined(PRE_MOD_POTENTIAL_JEWEL_RENEWAL
void CDNUserSendManager::SendCharmItemRequest(int nInvenType, short sInvenIndex, INT64 biItemSerial, short nRetCode, std::vector<TCharmItem> * pvRandomGiveItemList)
{
	SCCharmItemRequest Item = {0,};

	Item.biInvenSerial = biItemSerial;
	Item.sInvenIndex = sInvenIndex;
	Item.cInvenType = nInvenType;
	Item.nRet = nRetCode;
#ifdef PRE_ADD_GETITEM_WITH_RANDOMVALUE
	if (pvRandomGiveItemList && (!pvRandomGiveItemList->empty()))
	{
		std::vector<TCharmItem>::iterator ii;
		for (ii = pvRandomGiveItemList->begin(); ii != pvRandomGiveItemList->end(); ii++)
		{
			Item.RandomGiveItems[Item.cRandomGiveCount].nItemID = (*ii).nItemID;
			Item.RandomGiveItems[Item.cRandomGiveCount].nCount = (*ii).nCount;
			Item.RandomGiveItems[Item.cRandomGiveCount].nPeriod = (*ii).nPeriod;
			Item.RandomGiveItems[Item.cRandomGiveCount].nGold = (*ii).nGold;
			Item.cRandomGiveCount++;
			if (Item.cRandomGiveCount >= RANDOMGIVE_CHARMITEM_MAX)
				break;
		}
	}
#endif		//#ifdef PRE_ADD_GETITEM_WITH_RANDOMVALUE

	m_pSession->AddSendData(SC_ITEMGOODS, eItemGoods::SC_CHARMITEMREQUEST, (char*)&Item, sizeof(SCCharmItemRequest));
}


void CDNUserSendManager::SendCharmItemComplete(int nInvenType, int nResultItemID, int nResultItemCount, int nResultPeriod, short nRetCode)
{
	SCCharmItemComplete Item = {0,};

	Item.cInvenType = nInvenType;
	Item.nResultItemID = nResultItemID;
	Item.nItemCount = nResultItemCount; // ItemCount or Gold
	Item.nPeriod = nResultPeriod;
	Item.nRet = nRetCode;

	m_pSession->AddSendData(SC_ITEMGOODS, eItemGoods::SC_CHARMITEMCOMPLETE, (char*)&Item, sizeof(SCCharmItemComplete));
}

#if defined (PRE_ADD_BESTFRIEND)
void CDNUserSendManager::SendBestFriendItemRequest(INT64 biItemSerial, short nRetCode)
{
	SCBestFriendItemRequest Item = {0,};

	Item.biGiftSerial = biItemSerial;
	Item.nRet = nRetCode;

	m_pSession->AddSendData(SC_ITEMGOODS, eItemGoods::SC_BESTFRIENDREQUEST, (char*)&Item, sizeof(SCBestFriendItemRequest));
}

void CDNUserSendManager::SendBestFriendItemComplete(int nResultItemID, int nResultItemCount, int nResultPeriod, short nRetCode)
{
	SCBestFriendItemComplete Item = {0,};

	Item.nResultItemID = nResultItemID;
	Item.nItemCount = nResultItemCount;
	Item.nPeriod = nResultPeriod;
	Item.nRet = nRetCode;

	m_pSession->AddSendData(SC_ITEMGOODS, eItemGoods::SC_BESTFRIENDCOMPLETE, (char*)&Item, sizeof(SCBestFriendItemComplete));
}
#endif

#if defined (PRE_ADD_CHAOSCUBE)
void CDNUserSendManager::SendChaosCubeRequest(char cInvenType, int nCount, TChaosItem ChaosItem[MAX_CHAOSCUBE_STUFF], short nRetCode)
{
	SCChaosCubeRequest Item = {0,};

	Item.nRet = nRetCode;
	Item.cInvenType = cInvenType;
	Item.nCount = nCount;
	if (ChaosItem)
		memcpy (Item.ChaosItem, ChaosItem, sizeof(TChaosItem)*nCount);

	int nSize = sizeof(SCChaosCubeRequest) - sizeof (Item.ChaosItem) + (sizeof(TChaosItem)*nCount);
	m_pSession->AddSendData(SC_ITEMGOODS, eItemGoods::SC_CHAOSCUBEREQUEST, (char*)&Item, nSize);
}

void CDNUserSendManager::SendChaosCubeComplete(char cInvenType, int nResultItemID, int nResultItemCount, int nResultPeriod, short nRetCode)
{
	SCChaosCubeComplete Item = {0,};

	Item.cInvenType = cInvenType;
	Item.nResultItemID = nResultItemID;
	Item.nItemCount = nResultItemCount; // ItemCount or Gold
	Item.nPeriod = nResultPeriod;
	Item.nRet = nRetCode;

	m_pSession->AddSendData(SC_ITEMGOODS, eItemGoods::SC_CHAOSCUBECOMPLETE, (char*)&Item, sizeof(SCChaosCubeComplete));
}

void CDNUserSendManager::SendChaosGaugeCubeAddStuff(int nChaosGaugeType, int nCurrentGauge, int nRetCode)
{
	SCChaosGaugeCubeAddStuff Packet = { 0, };
	Packet.nRet = nRetCode;
	Packet.nChaosGaugeType = nChaosGaugeType;
	Packet.nCurrentGauge = nCurrentGauge;
	m_pSession->AddSendData(SC_ITEMGOODS, eItemGoods::SC_CHAOSGAUGECUBE_ADDSTUFF, (char*)&Packet, sizeof(SCChaosGaugeCubeAddStuff));
}

void CDNUserSendManager::SendChaosGaugeCubeComplete(int nChaosGaugeType, int nResultItemID, int nResultItemCount, int nResultPeriod, int nRetCode)
{
	SCChaosGaugeCubeComplete Packet = { 0, };
	Packet.nRet = nRetCode;
	Packet.nChaosGaugeType = nChaosGaugeType;
	Packet.nResultItemID = nResultItemID;
	Packet.nItemCount = nResultItemCount;
	Packet.nPeriod = nResultPeriod;
	m_pSession->AddSendData(SC_ITEMGOODS, eItemGoods::SC_CHAOSGAUGECUBE_COMPLETE, (char*)&Packet, sizeof(SCChaosGaugeCubeComplete));
}

void CDNUserSendManager::SendChaosGaugeCubeList(const std::map<int, int> &mapChaosGauge)
{
	SCChaosGaugeCubeList Packet = { 0, };
	int nIdx = 0;
	for (std::map<int, int>::const_iterator it = mapChaosGauge.begin(); it != mapChaosGauge.end() && nIdx < 20; ++it)
	{
		Packet.GaugeList[nIdx].nChaosGaugeType = it->first;
		Packet.GaugeList[nIdx].nItemGauge = it->second;
		++nIdx;
	}
	Packet.nCount = nIdx;
	int nSize = sizeof(SCChaosGaugeCubeList) - sizeof(Packet.GaugeList) + (sizeof(TChaosGaugeInfo) * nIdx);
	m_pSession->AddSendData(SC_ITEMGOODS, eItemGoods::SC_CHAOSGAUGECUBE_LIST, (char*)&Packet, nSize);
}
#endif

void CDNUserSendManager::SendPetalTokenResult(int nUseItemID, int nTotalPetal)
{
	SCPetalTokenResult Item = { 0, };

	Item.nUseItemID = nUseItemID;
	Item.nTotalPetal = nTotalPetal;

	m_pSession->AddSendData(SC_ITEMGOODS, eItemGoods::SC_PETALTOKENRESULT, (char*)&Item, sizeof(SCPetalTokenResult));
}

void CDNUserSendManager::SendAppellationGainResult(int nAppellationArrayIndex, int nRet)
{
	SCAppellationGainResult Item = { 0, };

	Item.nArrayIndex = nAppellationArrayIndex;
	Item.nRet = nRet;

	m_pSession->AddSendData(SC_ITEMGOODS, eItemGoods::SC_APPELLATIONGAINRESULT, (char*)&Item, sizeof(SCAppellationGainResult));
}

void CDNUserSendManager::SendEnchantJewelItemResult( int nRet, int nSlotIndex )
{
	SCEnchantJewelItem result;
	result.nRet = nRet;
	result.cInvenIndex = nSlotIndex;

	m_pSession->AddSendData(SC_ITEMGOODS, eItemGoods::SC_ENCHANTJEWEL, (char*)&result, sizeof(SCEnchantJewelItem));
}

void CDNUserSendManager::SendExpandSkillPageResult( int nRet )
{
	SCExpandSkillPageItem result;
	result.nRet = nRet;	

	m_pSession->AddSendData(SC_ITEMGOODS, eItemGoods::SC_EXPANDSKILLPAGERESULT, (char*)&result, sizeof(SCExpandSkillPageItem));
}

void CDNUserSendManager::SendGlyphTimeInfo(int nDelayArray, int nRemainArray)
{
	SCGlyphTimeInfo Glyph;
	memset(&Glyph, 0, sizeof(SCGlyphTimeInfo));

	Glyph.nDelayTime = nDelayArray;
	Glyph.nRemainTime = nRemainArray;

	m_pSession->AddSendData(SC_ITEM, eItem::SC_GLYPHTIMEINFO, (char*)&Glyph, sizeof(SCGlyphTimeInfo));	
}

void CDNUserSendManager::SendSortInventory(int nRet)
{
	SCSortInventory Sort = { 0, };
	Sort.nRet = nRet;
	m_pSession->AddSendData(SC_ITEM, eItem::SC_SORTINVENTORY, (char*)&Sort, sizeof(SCSortInventory));	
}

void CDNUserSendManager::SendDecreaseDurabilityInventory( char cType, void *pValue )
{
	SCDecreaseDurabilityInventory Durability;
	memset( &Durability, 0, sizeof(SCDecreaseDurabilityInventory) );

	Durability.cType = cType;
	switch( cType ) {
		case 0:	Durability.nAbsolute = *(int*)pValue; break;
		case 1: Durability.fRatio = *(float*)pValue; break;

	}
	m_pSession->AddSendData(SC_ITEM, eItem::SC_DECREASE_DURABILITY_INVENTORY, (char*)&Durability, sizeof(SCDecreaseDurabilityInventory));	
}

void CDNUserSendManager::SendBroadcastingEffect(UINT nSessionID, char cType, char cState)
{
	SCBroadcastingEffect Effect;
	memset(&Effect, 0, sizeof(SCBroadcastingEffect));

	Effect.nSessionID = nSessionID;
	Effect.cType = cType;
	Effect.cState = cState;

	m_pSession->AddSendData(SC_ITEM, eItem::SC_BROADCASTINGEFFECT, (char*)&Effect, sizeof(SCBroadcastingEffect));
}

void CDNUserSendManager::SendChangeGlyph(UINT nSessionID, TItemInfo &ItemInfo)
{
	SCChangeGlyph ChangeGlyph;
	memset(&ChangeGlyph, 0, sizeof(SCChangeGlyph));

	ChangeGlyph.nSessionID = nSessionID;
	ChangeGlyph.ItemInfo = ItemInfo;

	m_pSession->AddSendData(SC_ITEM, eItem::SC_CHANGEGLYPH, (char*)&ChangeGlyph, sizeof(SCChangeGlyph));
}


// Mount
void CDNUserSendManager::SendVehicleEquipList(TVehicle *pVehicleEquip)
{
	if(!pVehicleEquip) return;

	SCVehicleEquipList Vehicle;
	memset(&Vehicle, 0, sizeof(Vehicle));

	Vehicle.VehicleEquip.SetCompact(*pVehicleEquip);

	m_pSession->AddSendData(SC_ITEM, eItem::SC_VEHICLEEQUIPLIST, reinterpret_cast<char*>(&Vehicle), sizeof(Vehicle) );
}

void CDNUserSendManager::SendVehicleInvenList(int nTotalCount, int nCount, TVehicleCompact *VehicleList)
{
	if(nTotalCount == 0) return;

	SCVehicleInvenList Vehicle;
	memset(&Vehicle, 0, sizeof(Vehicle));

	Vehicle.nTotalInventoryCount = nTotalCount;
	Vehicle.cInvenCount = nCount;
	for (int i = 0; i < Vehicle.cInvenCount; i++)
	{
		if( m_pSession->GetItem()->GetVehicleInventory( VehicleList[i].Vehicle[Vehicle::Slot::Body].nSerial ) == NULL )
		{
			if( Vehicle.cInvenCount > 0 )
				--Vehicle.cInvenCount;
			else
				_ASSERT(0);
			continue;
		}
		Vehicle.VehicleInven[i] = VehicleList[i];
	}

	int nSize = sizeof(SCVehicleInvenList) - sizeof(Vehicle.VehicleInven) +(sizeof(TVehicleCompact) * Vehicle.cInvenCount);
	m_pSession->AddSendData(SC_ITEM, eItem::SC_VEHICLEINVENLIST, (char*)&Vehicle, nSize);
}

void CDNUserSendManager::SendChangeVehicleParts(UINT nSessionID, int nIndex, const TItem &Item)
{
	SCChangeVehicleParts Vehicle;
	memset(&Vehicle, 0, sizeof(Vehicle));

	Vehicle.nSessionID = nSessionID;
	Vehicle.cSlotIndex = nIndex;
	Vehicle.Equip.SetItem(Item);

	m_pSession->AddSendData(SC_ITEM, eItem::SC_CHANGEVEHICLEPARTS, reinterpret_cast<char*>(&Vehicle), sizeof(Vehicle) );
}

void CDNUserSendManager::SendChangeVehicleColor(UINT nSessionID, INT64 biSerial, DWORD dwColor, char cPetPartsColor)
{
	SCChangeVehicleColor Vehicle;
	memset(&Vehicle, 0, sizeof(Vehicle));

	Vehicle.nSessionID = nSessionID;
	Vehicle.dwColor = dwColor;
	Vehicle.cSlotIndex = cPetPartsColor;
	Vehicle.biSerial = biSerial;

	m_pSession->AddSendData(SC_ITEM, eItem::SC_CHANGEVEHICLECOLOR, reinterpret_cast<char*>(&Vehicle), sizeof(Vehicle) );
}

void CDNUserSendManager::SendRefreshVehicleInven(TVehicle &InvenItem, bool bNewSign/* = true*/)
{
	SCRefreshVehicleInven Vehicle;
	memset(&Vehicle, 0, sizeof(Vehicle));

	Vehicle.bNewSign = bNewSign;
	Vehicle.nCount = 1;
	Vehicle.ItemList[0].SetCompact(InvenItem);

	int nLen = sizeof(SCRefreshVehicleInven) - sizeof(Vehicle.ItemList) +(sizeof(TVehicleCompact) * Vehicle.nCount);
	m_pSession->AddSendData(SC_ITEM, eItem::SC_REFRESHVEHICLEINVEN, (char*)&Vehicle, nLen);

#if defined(_GAMESERVER)
	m_pSession->GetGameRoom()->OnDispatchMessage( m_pSession, IN_SC_ITEM, eItem::SC_REFRESHVEHICLEINVEN, (char*)&Vehicle, nLen, 0 );
#endif
}

void CDNUserSendManager::SendSortWarehouse(int nRet)
{
	SCSortWarehouse packet;
	memset(&packet, 0, sizeof(SCSortWarehouse));

	packet.nRet = nRet;

	m_pSession->AddSendData(SC_ITEM, eItem::SC_SORTWAREHOUSE, (char*)&packet, sizeof(SCSortWarehouse));	
}

void CDNUserSendManager::SendRefreshVehicleInven(std::vector<TVehicle> VecItemList, bool bNewSign/* = true*/)
{
	SCRefreshVehicleInven Vehicle;
	memset(&Vehicle, 0, sizeof(Vehicle));

	Vehicle.bNewSign = bNewSign;
	Vehicle.nCount = (int)VecItemList.size();
	for (int i = 0; i < Vehicle.nCount; i++){
		Vehicle.ItemList[i].SetCompact(VecItemList[i]);
	}

	int nLen = sizeof(SCRefreshVehicleInven) - sizeof(Vehicle.ItemList) +(sizeof(TVehicleCompact) * Vehicle.nCount);
	m_pSession->AddSendData(SC_ITEM, eItem::SC_REFRESHVEHICLEINVEN, (char*)&Vehicle, nLen);

#if defined(_GAMESERVER)
	m_pSession->GetGameRoom()->OnDispatchMessage( m_pSession, IN_SC_ITEM, eItem::SC_REFRESHVEHICLEINVEN, (char*)&Vehicle, nLen, 0 );
#endif
}

void CDNUserSendManager::SendPetEquipList(const TVehicle *pPetEquip)
{
	if(!pPetEquip) return;

	SCVehicleEquipList Pet;
	memset(&Pet, 0, sizeof(Pet));

	Pet.VehicleEquip.SetCompact(*pPetEquip);

	m_pSession->AddSendData(SC_ITEM, eItem::SC_PETEQUIPLIST, reinterpret_cast<char*>(&Pet), sizeof(Pet) );
}

// Change Pet
void CDNUserSendManager::SendChangePetParts(UINT nSessionID, int nIndex, TItem &Item)
{
	SCChangeVehicleParts Pet;
	memset(&Pet, 0, sizeof(Pet));

	Pet.nSessionID = nSessionID;
	Pet.cSlotIndex = nIndex;
	Pet.Equip.SetItem(Item);

	m_pSession->AddSendData(SC_ITEM, eItem::SC_CHANGEPETPARTS, reinterpret_cast<char*>(&Pet), sizeof(Pet) );
}

void CDNUserSendManager::SendChangePetBody(UINT nSessionID, TVehicleCompact &Pet)
{
	SCChangePetBody Packet;
	memset(&Packet, 0, sizeof(Packet));

	Packet.nSessionID = nSessionID;
	Packet.PetInfo = Pet;

	m_pSession->AddSendData(SC_ITEM, eItem::SC_CHANGEPETBODY, reinterpret_cast<char*>(&Packet), sizeof(Packet));
}

void CDNUserSendManager::SendAddPetExp(UINT nSessionID, INT64 biPetSerial, int nExp)
{
	SCAddPetExp Packet = {0,};
	Packet.nSessionID = nSessionID;
	Packet.biPetSerial = biPetSerial;
	Packet.nExp = nExp;

	m_pSession->AddSendData(SC_ITEM, eItem::SC_ADDPETEXP, reinterpret_cast<char*>(&Packet), sizeof(Packet));
}

void CDNUserSendManager::SendPetCurrentSatiety(INT64 biPetSerial, int nCurrentSatiety)
{
	SCPetCurrentSatiety Packet = {0,};
	Packet.biPetSerial = biPetSerial;
	Packet.nCurrentSatiety = nCurrentSatiety;

 	m_pSession->AddSendData(SC_ITEM, eItem::SC_PET_CURRENT_SATIETY, reinterpret_cast<char*>(&Packet), sizeof(Packet));
}
void CDNUserSendManager::SendPetFoodEat(int nResult, int nIncSatiety )
{
	SCPetFoodEat Packet = {0,};
	Packet.nResult = nResult;
	Packet.nIncSatiety = nIncSatiety;

	m_pSession->AddSendData(SC_ITEM, eItem::SC_PET_FOOD_EAT,  reinterpret_cast<char*>(&Packet), sizeof(Packet));
}

#if defined(PRE_ADD_EXPUP_ITEM)
void CDNUserSendManager::SendDailyLimitItemErr(int nWeightTableIndex)
{
	SCDailyLimitItemError Packet = {0,};
	Packet.nWeightTableIndex = nWeightTableIndex;
	m_pSession->AddSendData(SC_ITEM, eItem::SC_DAILY_LIMIT_ITEM, reinterpret_cast<char*>(&Packet), sizeof(Packet));
}
#endif

void CDNUserSendManager::SendRemoveCash(const TItem *pItem, int nRet)
{
	SCRemoveCash Packet = {0,};

	if (pItem){
		Packet.biItemSerial = pItem->nSerial;
		Packet.bEternity = pItem->bEternity;
		Packet.bExpireComplete = pItem->bExpireComplete;
		Packet.tExpireDate = pItem->tExpireDate;
	}
	Packet.nRet = nRet;

	m_pSession->AddSendData(SC_ITEM, eItem::SC_REMOVECASH, (char*)&Packet, sizeof(SCRemoveCash));
}

#if defined( PRE_ADD_LIMITED_SHOP )
#if defined( PRE_FIX_74404 )
void CDNUserSendManager::SendLimitedShopItemData(std::list<LimitedShop::LimitedItemData> &listLimitedShop, bool bAllClear/*=false*/)
{
	if (listLimitedShop.empty() && !bAllClear)
		return;

	SCLimitedShopItemData packet;
	memset(&packet, 0, sizeof(packet));

	packet.count = static_cast<short>(listLimitedShop.size());
	packet.bAllClear = bAllClear;

	int i = 0;
	for( std::list<LimitedShop::LimitedItemData>::iterator itor = listLimitedShop.begin();itor!=listLimitedShop.end();itor++ )
	{
		packet.items[i] = *itor;
		++i;
	}

	int length = sizeof(packet) - sizeof(packet.items) + (sizeof(packet.items[0]) * packet.count);
	m_pSession->AddSendData(SC_ITEM, eItem::SC_LIMITEDSHOPITEMDATA, reinterpret_cast<char*>(&packet), length);
}

void CDNUserSendManager::SendLimitedShopItemData(int nShopID, int nItemID, int nBuyCount, int nResetCycle)
{
	SCLimitedShopItemData packet;
	memset(&packet, 0, sizeof(packet));

	packet.count = 1;	
	packet.items[0].nShopID = nShopID;
	packet.items[0].nItemID = nItemID;	
	packet.items[0].nBuyCount = nBuyCount;
	packet.items[0].nResetCycle = nResetCycle;

	int length = sizeof(packet) - sizeof(packet.items) + (sizeof(packet.items[0]) * packet.count);
	m_pSession->AddSendData(SC_ITEM, eItem::SC_LIMITEDSHOPITEMDATA, reinterpret_cast<char*>(&packet), length);
}
#else // #if defined( PRE_FIX_74404 )
void CDNUserSendManager::SendLimitedShopItemData(std::map<int, LimitedShop::LimitedItemData> &mapLimitedShop, bool bAllClear/*=false*/)
{
	if (mapLimitedShop.empty() && !bAllClear)
		return;

	SCLimitedShopItemData packet;
	memset(&packet, 0, sizeof(packet));

	packet.count = static_cast<short>(mapLimitedShop.size());
	packet.bAllClear = bAllClear;

	size_t i = 0;
	for (std::map<int, LimitedShop::LimitedItemData>::value_type v : mapLimitedShop)
	{		
		packet.items[i] = v.second;
		++i;
	}

	int length = sizeof(packet) - sizeof(packet.items) + (sizeof(packet.items[0]) * packet.count);
	m_pSession->AddSendData(SC_ITEM, eItem::SC_LIMITEDSHOPITEMDATA, reinterpret_cast<char*>(&packet), length);
}
void CDNUserSendManager::SendLimitedShopItemData(int nItemID, int nBuyCount, int nResetCycle)
{
	SCLimitedShopItemData packet;
	memset(&packet, 0, sizeof(packet));

	packet.count = 1;	
	packet.items[0].nItemID = nItemID;	
	packet.items[0].nBuyCount = nBuyCount;
	packet.items[0].nResetCycle = nResetCycle;
	
	int length = sizeof(packet) - sizeof(packet.items) + (sizeof(packet.items[0]) * packet.count);
	m_pSession->AddSendData(SC_ITEM, eItem::SC_LIMITEDSHOPITEMDATA, reinterpret_cast<char*>(&packet), length);
}
#endif // #if defined( PRE_FIX_74404 )
#endif // #if defined( PRE_ADD_LIMITED_SHOP )

#if defined(PRE_PERIOD_INVENTORY)
void CDNUserSendManager::SendPeriodInventory(bool bEnable, __time64_t tExpireDate)
{
	SCPeriodInventory Packet = {0,};
	Packet.bEnable = bEnable;
	Packet.tExpireDate = tExpireDate;
	m_pSession->AddSendData(SC_ITEM, eItem::SC_PERIODINVENTORY, (char*)&Packet, sizeof(SCPeriodInventory));
}

void CDNUserSendManager::SendPeriodWarehouse(bool bEnable, __time64_t tExpireDate)
{
	SCPeriodWarehouse Packet = {0,};
	Packet.bEnable = bEnable;
	Packet.tExpireDate = tExpireDate;
	m_pSession->AddSendData(SC_ITEM, eItem::SC_PERIODWAREHOUSE, (char*)&Packet, sizeof(SCPeriodWarehouse));
}

void CDNUserSendManager::SendSortPeriodInventory(int nRet)
{
	SCSortInventory Packet = {0,};
	Packet.nRet = nRet;
	m_pSession->AddSendData(SC_ITEM, eItem::SC_SORTPERIODINVENTORY, (char*)&Packet, sizeof(SCSortInventory));
}

void CDNUserSendManager::SendSortPeriodWarehouse(int nRet)
{
	SCSortWarehouse Packet = {0,};
	Packet.nRet = nRet;
	m_pSession->AddSendData(SC_ITEM, eItem::SC_SORTPERIODWAREHOUSE, (char*)&Packet, sizeof(SCSortWarehouse));
}
#endif	// #if defined(PRE_PERIOD_INVENTORY)

#if defined(PRE_ADD_TALISMAN_SYSTEM)
void CDNUserSendManager::SendTalismanOpenSlot(int nRet, int nTalismanOpenFlag)
{
	SCTalismanOpenSlot Packet;
	memset(&Packet, 0, sizeof(SCTalismanOpenSlot));
	Packet.nRet = nRet;
	Packet.nTalismanOpenFlag = nTalismanOpenFlag;
	m_pSession->AddSendData(SC_ITEM, eItem::SC_TALISMAN_OPENSLOT, (char*)&Packet, sizeof(SCTalismanOpenSlot));
}

void CDNUserSendManager::SendChangeTalisman(UINT nSessionID, char cSlotIndex, TItem &TalismanItem)
{
	SCChangeTalisman Packet;
	memset(&Packet, 0, sizeof(SCChangeTalisman));
	Packet.nSessionID = nSessionID;
	Packet.cSlotIndex = cSlotIndex;
	Packet.Item = TalismanItem;
	m_pSession->AddSendData(SC_ITEM, eItem::SC_CHANGETALISMAN, (char*)&Packet, sizeof(SCChangeTalisman));
}

void CDNUserSendManager::SendTalismanExpireData(bool bOpenFlag, __time64_t tExpireDate)
{
	SCTalismanExpireData Packet;
	memset(&Packet, 0, sizeof(SCTalismanExpireData));
	Packet.bOpenFlag = bOpenFlag;
	Packet.tExpireDate = tExpireDate;
	m_pSession->AddSendData(SC_ITEM, eItem::SC_TALISMANEXPIREDATA, (char*)&Packet, sizeof(SCTalismanExpireData));
}
#endif	// #if defined(PRE_ADD_TALISMAN_SYSTEM)

#if defined( PRE_ADD_STAGE_USECOUNT_ITEM )
void CDNUserSendManager::SendStageUseLimitItem( int nItemID, int nUseCount )
{
	SCStageUseLimitItem Packet;
	Packet.nItemID = nItemID;
	Packet.nUseCount = nUseCount;
	m_pSession->AddSendData(SC_ITEM, eItem::SC_STAGE_USELIMITITEM, (char*)&Packet, sizeof(SCStageUseLimitItem));
}
#endif

void CDNUserSendManager::SendSpecialItemList( const int* pItemIDs, int nCount )
{
	SCSpecialItemList Packet;
	memset(&Packet, 0, sizeof(SCSpecialItemList));
	if (pItemIDs && nCount > 0)
	{
		int nCopy = (nCount > 10) ? 10 : nCount;
		for (int i = 0; i < nCopy; ++i)
			Packet.nItemIDs[i] = pItemIDs[i];
	}
	m_pSession->AddSendData(SC_ITEM, eItem::SC_SPECIAL_ITEM_LIST, (char*)&Packet, sizeof(SCSpecialItemList));
}

void CDNUserSendManager::SendAutoUnpackResult( int nRet )
{
	SCAutoUnpackResult Packet;
	Packet.nRet = nRet;
	m_pSession->AddSendData(SC_ITEM, eItem::SC_AUTO_UNPACK_RESULT, (char*)&Packet, sizeof(SCAutoUnpackResult));
}

void CDNUserSendManager::SendDeleteExpireItemList( const std::vector<INT64>& vecSerials )
{
	if (vecSerials.empty())
		return;
	SCDeleteExpireItemList Packet;
	memset(&Packet, 0, sizeof(SCDeleteExpireItemList));
	WORD wMax = static_cast<WORD>((vecSerials.size() > CASHINVENTORYMAX) ? CASHINVENTORYMAX : vecSerials.size());
	Packet.wCount = wMax;
	for (WORD i = 0; i < wMax; ++i)
		Packet.biSerial[i] = vecSerials[i];
	int nSize = static_cast<int>(sizeof(WORD) + sizeof(INT64) * wMax);
	m_pSession->AddSendData(SC_ITEM, eItem::SC_DELETE_EXPIRE_ITEM_LIST, (char*)&Packet, nSize);
}

// Item Goods
void CDNUserSendManager::SendChangePetNameResult(int nRet, UINT nUserSessionID, INT64 biItemSerial, const WCHAR* wszName)
{
	SCChangePetName result = {0, };
	result.nRet = nRet;
	result.nUserSessionID = nUserSessionID;
	result.biItemSerial = biItemSerial;

	if(wszName)
		_wcscpy(result.wszPetName, _countof(result.wszPetName), wszName, (int)wcslen(wszName));

	m_pSession->AddSendData(SC_ITEMGOODS, eItemGoods::SC_CHANGEPETNAME, (char*)&result, sizeof(SCChangePetName));
}

void CDNUserSendManager::SendRequestCashWorldMsg(INT64 biItemSerial, char cType)
{
	SCRequestCashWorldMsg WorldMsg; 
	memset( &WorldMsg, 0, sizeof(SCRequestCashWorldMsg) );

	WorldMsg.biItemSerial = biItemSerial;
	WorldMsg.cType = cType;

	m_pSession->AddSendData( SC_ITEMGOODS, eItemGoods::SC_REQUEST_CASHWORLDMSG, (char*)&WorldMsg, sizeof(SCRequestCashWorldMsg) );
}

void CDNUserSendManager::SendChangeColor(UINT nSessionID, int nItemType, DWORD dwColor)
{
	SCChangeColor Color;
	memset(&Color, 0, sizeof(SCChangeColor));

	Color.nSessionID = nSessionID;
	Color.cItemType = nItemType;
	Color.dwColor = dwColor;

	m_pSession->AddSendData(SC_ITEMGOODS, eItemGoods::SC_CHANGECOLOR, (char*)&Color, sizeof(SCChangeColor));
}

void CDNUserSendManager::SendUnsealItemResult(int nRet, int nSlotCount)
{
	SCUnSealItem result;
	result.nRet = nRet;
	result.cInvenIndex = nSlotCount;

	m_pSession->AddSendData(SC_ITEMGOODS, eItemGoods::SC_UNSEAL, (char*)&result, sizeof(SCUnSealItem));
}

void CDNUserSendManager::SendSealItemResult(int nRet, int nSlotCount)
{
	SCSealItem result;
	result.nRet = nRet;
	result.cInvenIndex = nSlotCount;

	m_pSession->AddSendData(SC_ITEMGOODS, eItemGoods::SC_SEAL, (char*)&result, sizeof(SCSealItem));
}

void CDNUserSendManager::SendGuildRenameResult(int nRet)
{
	SCGuildRename result;
	result.nRet = nRet;

	m_pSession->AddSendData(SC_ITEMGOODS, eItemGoods::SC_GUILDRENAME, (char*)&result, sizeof(SCGuildRename));
}

void CDNUserSendManager::SendGuildMarkResult(int nRet, short wGuildMark, short wGuildMarkBG, short wGuildMarkBorder)
{
	SCGuildMark result;
	result.nRet = nRet;
	result.wGuildMark = wGuildMark;
	result.wGuildMarkBG = wGuildMarkBG;
	result.wGuildMarkBorder = wGuildMarkBorder;

	m_pSession->AddSendData(SC_ITEMGOODS, eItemGoods::SC_GUILDMARK, (char*)&result, sizeof(SCGuildMark));
}

void CDNUserSendManager::SendCharacterRenameResult(int nRet, UINT nUserSessionID, const WCHAR* wszName, __time64_t tNextChangeDate /*= 0*/)
{
	SCCharacterRename result;
	result.nRet = nRet;
	result.nUserSessionID = nUserSessionID;
	result.tNextChangeDate = tNextChangeDate;

	if(wszName)
		_wcscpy(result.wszCharacterName, _countof(result.wszCharacterName), wszName, (int)wcslen(wszName));

	m_pSession->AddSendData(SC_ITEMGOODS, eItemGoods::SC_CHARACTERRENAME, (char*)&result, sizeof(SCCharacterRename));
}

#ifdef PRE_ADD_GACHA_JAPAN
void CDNUserSendManager::SendGachaShopOpen_JP( int nGachaShopIndex )
{
	SCGachaShopOpen_JP GachaOpen;
	GachaOpen.nGachaShopIndex = nGachaShopIndex;

	m_pSession->AddSendData( SC_ITEMGOODS, eItemGoods::SC_JP_GACHAPON_OPEN_SHOP_RES, (char*)&GachaOpen, sizeof(SCGachaShopOpen_JP) );
}

void CDNUserSendManager::SendGachaResult_JP( int nResult, int nResultItemID )
{
	SCGachaRes_JP GachaResult;
	GachaResult.nRet = nResult;
	GachaResult.nResultItemID = nResultItemID;
	m_pSession->AddSendData( SC_ITEMGOODS, eItemGoods::SC_JP_GACHAPON_RES, (char*)&GachaResult, sizeof(GachaResult) );
}
#endif // PRE_ADD_GACHA_JAPAN

// Reputation
#if defined( PRE_ADD_NPC_REPUTATION_SYSTEM )
void CDNUserSendManager::SendOpenGiveNpcPresent( UINT nNpcID )
{
	SCOpenGiveNpcPresent GiveNpcPresent;
	GiveNpcPresent.nNpcID = nNpcID;

	m_pSession->AddSendData( SC_REPUTATION, eReputation::SC_OPENGIVENPCPRESENT, (char*)&GiveNpcPresent, sizeof(GiveNpcPresent) );
}

void CDNUserSendManager::SendRequestSendSelectedPresent( UINT nNpcID )
{
	SCRequestSendSelectedPresent RequestSendSelectedPresent;
	RequestSendSelectedPresent.nNpcID = nNpcID;

	m_pSession->AddSendData( SC_REPUTATION, eReputation::SC_REQUESTSENDSELECTEDPRESENT, (char*)&RequestSendSelectedPresent, sizeof(RequestSendSelectedPresent) );
}

void CDNUserSendManager::SendShowNpcEffect( UINT nNpcID, int nEffectIndex )
{
	SCShowNpcEffect ShowNpcEffect;
	ShowNpcEffect.nNpcID = nNpcID;
	ShowNpcEffect.nEffectIndex = nEffectIndex;

	m_pSession->AddSendData( SC_REPUTATION, eReputation::SC_SHOWNPCEFFECT, (char*)&ShowNpcEffect, sizeof(ShowNpcEffect) );
}
#endif // #if defined( PRE_ADD_NPC_REPUTATION_SYSTEM )


void CDNUserSendManager::SendOpenDarkLairRankBoard( UINT uiNpcID, int iMapIndex, int iPlayerCount )
{
	SCOpenDarkLairRankBoard TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );
	
	TxPacket.uiNpcID		= uiNpcID;
	TxPacket.iMapIndex		= iMapIndex;
	TxPacket.cPlayerCount	= static_cast<BYTE>(iPlayerCount);

	m_pSession->AddSendData( SC_NPC, eNpc::SC_OPEN_DARKLAIRRANKBOARD, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

void CDNUserSendManager::SendDarkLairRankBoard( TAGetDarkLairRankBoard* pPacket )
{
	SCDarkLairRankBoard TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	int iSize = (pPacket->nRetCode == ERROR_NONE) ? sizeof(TxPacket) : sizeof(TxPacket.iRet);

	TxPacket.iRet = pPacket->nRetCode;
#if defined(PRE_ADD_CHALLENGE_DARKLAIR)
	TxPacket.iMapIndex = pPacket->iMapIndex;
#endif
	if( TxPacket.iRet == ERROR_NONE )
	{
		// Best
		TxPacket.sBestHistory.nRank				= pPacket->sBestHistory.iRank;
		TxPacket.sBestHistory.nPlayRound		= pPacket->sBestHistory.unPlayRound;
		TxPacket.sBestHistory.nPlaySec			= pPacket->sBestHistory.uiPlaySec;
		TxPacket.sBestHistory.cPartyUserCount	= pPacket->sBestHistory.cPartyUserCount;
		_wcscpy( TxPacket.sBestHistory.wszPartyName, _countof(TxPacket.sBestHistory.wszPartyName), pPacket->sBestHistory.wszPartyName, (int)wcslen(pPacket->sBestHistory.wszPartyName) );
		if( TxPacket.sBestHistory.cPartyUserCount == 0 )
			TxPacket.sBestHistory.cPartyUserCount = pPacket->cPartyUserCount;
		
		for( int i=0 ; i<TxPacket.sBestHistory.cPartyUserCount ; ++i )
		{
			TxPacket.sBestHistory.Info[i].nLevel			= pPacket->sBestHistory.sUserData[i].unLevel;
			TxPacket.sBestHistory.Info[i].nJobIndex			= pPacket->sBestHistory.sUserData[i].iJobIndex;
			_wcscpy( TxPacket.sBestHistory.Info[i].wszCharacterName, _countof(TxPacket.sBestHistory.Info[i].wszCharacterName), 
				pPacket->sBestHistory.sUserData[i].wszCharacterName, (int)wcslen(pPacket->sBestHistory.sUserData[i].wszCharacterName) );
		}

		// Select-Top
		for( int i=0 ; i<_countof(TxPacket.sHistoryTop) ; ++i )
		{
			TxPacket.sHistoryTop[i].nRank			= i+1;
			TxPacket.sHistoryTop[i].nPlayRound		= pPacket->sHistoryTop[i].unPlayRound;
			TxPacket.sHistoryTop[i].nPlaySec		= pPacket->sHistoryTop[i].uiPlaySec;
			TxPacket.sHistoryTop[i].cPartyUserCount	= pPacket->sHistoryTop[i].cPartyUserCount;
			_wcscpy( TxPacket.sHistoryTop[i].wszPartyName, _countof(TxPacket.sHistoryTop[i].wszPartyName), pPacket->sHistoryTop[i].wszPartyName, (int)wcslen(pPacket->sHistoryTop[i].wszPartyName) );

			for( int j=0 ; j<TxPacket.sHistoryTop[i].cPartyUserCount ; ++j )
			{
				TxPacket.sHistoryTop[i].Info[j].nLevel		= pPacket->sHistoryTop[i].sUserData[j].unLevel;
				TxPacket.sHistoryTop[i].Info[j].nJobIndex	= pPacket->sHistoryTop[i].sUserData[j].iJobIndex;
				_wcscpy( TxPacket.sHistoryTop[i].Info[j].wszCharacterName, _countof(TxPacket.sHistoryTop[i].Info[j].wszCharacterName), 
					pPacket->sHistoryTop[i].sUserData[j].wszCharacterName, (int)wcslen(pPacket->sHistoryTop[i].sUserData[j].wszCharacterName) );
			}
		}
	}

	m_pSession->AddSendData( SC_ETC, eEtc::SC_DARKLAIR_RANK_BOARD, reinterpret_cast<char*>(&TxPacket), iSize );
}

void CDNUserSendManager::SendOpenPvPLadderRankBoard( UINT uiNpcID, LadderSystem::MatchType::eCode MatchType )
{
	SCOpenPvPLadderRankBoard TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.uiNpcID	= uiNpcID;
	TxPacket.MatchType	= MatchType;

	m_pSession->AddSendData( SC_NPC, eNpc::SC_OPEN_PVPLADDERRANKBOARD, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

void CDNUserSendManager::SendPvPLadderRankBoard( TAGetListPvPLadderRanking* pPacket )
{
	SCPvPLadderRankBoard TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	int iSize = (pPacket->nRetCode == ERROR_NONE) ? sizeof(TxPacket) : sizeof(TxPacket.iRet);

	TxPacket.iRet = pPacket->nRetCode;
	if( TxPacket.iRet == ERROR_NONE )
	{
		memcpy( &TxPacket.MyRanking, &pPacket->MyRanking, sizeof(TxPacket.MyRanking) );
		memcpy( TxPacket.Top, &pPacket->Top, sizeof(TxPacket.Top) );
	}

	m_pSession->AddSendData( SC_ETC, eEtc::SC_PVPLADDER_RANK_BOARD, reinterpret_cast<char*>(&TxPacket), iSize );
}

#if defined(PRE_ADD_PVP_RANKING)
void CDNUserSendManager::SendPvPRankBoard(TAGetPvPRankBoard* pPacket)
{
	SCPvPRankBoard TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	int iSize = (pPacket->nRetCode == ERROR_NONE) ? sizeof(TxPacket) : sizeof(TxPacket.iRet);

	TxPacket.iRet = pPacket->nRetCode;
	if( TxPacket.iRet == ERROR_NONE )
	{
		memcpy( &TxPacket.MyRanking, &pPacket->MyRanking, sizeof(TxPacket.MyRanking) );
	}

	m_pSession->AddSendData( SC_ETC, eEtc::SC_PVP_RANK_BOARD, reinterpret_cast<char*>(&TxPacket), iSize );
}

void CDNUserSendManager::SendPvPRankList( TAGetPvPRankList* pPacket )
{
	SCPvPRankList TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	int iSize = (pPacket->nRetCode == ERROR_NONE) ? sizeof(TxPacket) : sizeof(TxPacket.iRet);

	TxPacket.iRet = pPacket->nRetCode;
	if( TxPacket.iRet == ERROR_NONE )
	{
		TxPacket.cRankingCount = pPacket->cRankingCount;
		TxPacket.nRankingTotalCount = pPacket->biRankingTotalCount;
		if(pPacket->cRankingCount > 0 )
			memcpy(&TxPacket.RankingInfo, &pPacket->RankingInfo, sizeof(TPvPRanking) * pPacket->cRankingCount);
	}

	int nLen = sizeof(SCPvPRankList) - sizeof(TxPacket.RankingInfo) +(sizeof(TPvPRanking) * pPacket->cRankingCount);
	m_pSession->AddSendData(SC_ETC, eEtc::SC_PVP_RANK_LIST, reinterpret_cast<char*>(&TxPacket), nLen);
}

void CDNUserSendManager::SendPvPRankInfo(TAGetPvPRankInfo* pPacket)
{
	SCPvPRankInfo TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	int iSize = (pPacket->nRetCode == ERROR_NONE) ? sizeof(TxPacket) : sizeof(TxPacket.iRet);

	TxPacket.iRet = pPacket->nRetCode;
	if( TxPacket.iRet == ERROR_NONE )
	{
		memcpy( &TxPacket.RankingInfo, &pPacket->RankingInfo, sizeof(TxPacket.RankingInfo) );
	}

	m_pSession->AddSendData( SC_ETC, eEtc::SC_PVP_RANK_INFO, reinterpret_cast<char*>(&TxPacket), iSize );
}

void CDNUserSendManager::SendPvPLadderRankBoard(TAGetPvPLadderRankBoard* pPacket)
{
	SCPvPLadderRankBoard2 TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	int iSize = (pPacket->nRetCode == ERROR_NONE) ? sizeof(TxPacket) : sizeof(TxPacket.iRet);

	TxPacket.iRet = pPacket->nRetCode;
	TxPacket.MatchType = (LadderSystem::MatchType::eCode)pPacket->cPvPLadderCode;
	if( TxPacket.iRet == ERROR_NONE )
		memcpy( &TxPacket.MyRanking, &pPacket->MyRanking, sizeof(TxPacket.MyRanking) );

	m_pSession->AddSendData( SC_ETC, eEtc::SC_PVP_LADDER_RANK_BOARD, reinterpret_cast<char*>(&TxPacket), iSize );
}

void CDNUserSendManager::SendPvPLadderRankList( TAGetPvPLadderRankList* pPacket )
{
	SCPvPLadderRankList TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	int iSize = (pPacket->nRetCode == ERROR_NONE) ? sizeof(TxPacket) : sizeof(TxPacket.iRet);

	TxPacket.iRet = pPacket->nRetCode;
	if( TxPacket.iRet == ERROR_NONE )
	{
		TxPacket.cRankingCount = pPacket->cRankingCount;
		TxPacket.nRankingTotalCount = pPacket->biRankingTotalCount;
		TxPacket.MatchType = (LadderSystem::MatchType::eCode)pPacket->cPvPLadderCode;
		if(pPacket->cRankingCount > 0 )
			memcpy(&TxPacket.RankingInfo, &pPacket->RankingInfo, sizeof(TPvPLadderRanking2) * pPacket->cRankingCount);
	}

	int nLen = sizeof(SCPvPLadderRankList) - sizeof(TxPacket.RankingInfo) +(sizeof(TPvPLadderRanking2) * pPacket->cRankingCount);
	m_pSession->AddSendData(SC_ETC, eEtc::SC_PVP_LADDER_RANK_LIST, reinterpret_cast<char*>(&TxPacket), nLen);
}

void CDNUserSendManager::SendPvPLadderRankInfo(TAGetPvPLadderRankInfo* pPacket)
{
	SCPvPLadderRankInfo TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	int iSize = (pPacket->nRetCode == ERROR_NONE) ? sizeof(TxPacket) : sizeof(TxPacket.iRet);

	TxPacket.iRet = pPacket->nRetCode;
	TxPacket.MatchType = (LadderSystem::MatchType::eCode)pPacket->cPvPLadderCode;
	if( TxPacket.iRet == ERROR_NONE )
		memcpy( &TxPacket.RankingInfo, &pPacket->RankingInfo, sizeof(TxPacket.RankingInfo) );

	m_pSession->AddSendData( SC_ETC, eEtc::SC_PVP_LADDER_RANK_INFO, reinterpret_cast<char*>(&TxPacket), iSize );
}

#endif

// Npc
void CDNUserSendManager::SendTalkEnd(UINT nNpcObjectID)
{
	SCNpcTalk NpcTalk;
	memset(&NpcTalk, 0, sizeof(SCNpcTalk));
	
	NpcTalk.nNpcUniqueID = nNpcObjectID;
	NpcTalk.bIsEnd = true;
	NpcTalk.nCount = 0;
	
	m_pSession->AddSendData(SC_NPC, eNpc::SC_NPCTALK, (char*)&NpcTalk, sizeof(SCNpcTalk) -(sizeof(TalkParam) *(TALK_PARAM_MAX-NpcTalk.nCount)));
}

void CDNUserSendManager::SendNextTalk(UINT nNpcObjectID, WCHAR* wszIndex, WCHAR* wszTarget, std::vector<TalkParam>& talkParam )
{
	SCNpcTalk NpcTalk;
	memset(&NpcTalk, 0, sizeof(SCNpcTalk));
	
	NpcTalk.nNpcUniqueID = nNpcObjectID;

	NpcTalk.dwIndexHashCode = g_pDataManager->GetStringHashCode(wszIndex);
	NpcTalk.dwTargetHashCode = g_pDataManager->GetStringHashCode(wszTarget);
	NpcTalk.bIsEnd = false;

	if( talkParam.size() <=  TALK_PARAM_MAX )
	{
		for (int i = 0; i <(int)talkParam.size(); i++)
		{
			memcpy(&(NpcTalk.TalkParamArray[i]), &(talkParam[i]), sizeof(TalkParam));
		}
		NpcTalk.nCount = (int)talkParam.size();
	}
	
	m_pSession->AddSendData(SC_NPC, eNpc::SC_NPCTALK, (char*)&NpcTalk, sizeof(SCNpcTalk) -(sizeof(TalkParam) *(TALK_PARAM_MAX-NpcTalk.nCount)));
}

void CDNUserSendManager::SendNextTalkError()
{
	SCNpcTalk NpcTalk;
	memset(&NpcTalk, 0, sizeof(SCNpcTalk));
	
	NpcTalk.nNpcUniqueID = 0xffffffff;
	NpcTalk.bIsEnd = true;
	NpcTalk.nCount = 0;

	m_pSession->AddSendData(SC_NPC, eNpc::SC_NPCTALK, (char*)&NpcTalk, sizeof(SCNpcTalk) -(sizeof(TalkParam) *(TALK_PARAM_MAX-NpcTalk.nCount)));
}

void CDNUserSendManager::SendShowWarehouse( int iItemID/*=0*/ )
{
	SCShowWareHouse TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.iItemID = iItemID;

	m_pSession->AddSendData(SC_NPC, eNpc::SC_SHOWWAREHOUSE, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}


void CDNUserSendManager::SendOpenQuestReward(int nTableIndex, bool bActivate)
{
	// SC_QUEST / SC_QUEST_REWARD 
	SCQuestReward packet;
	memset(&packet, 0, sizeof(SCQuestReward));

	packet.nRewardTableIndex = nTableIndex;
	packet.bActivate = bActivate;

	m_pSession->AddSendData(SC_QUEST, eQuest::SC_QUEST_REWARD, (char*)&packet, sizeof(packet));
}

void CDNUserSendManager::SendOpenCompoundEmblem()
{
	m_pSession->AddSendData(SC_NPC, eNpc::SC_OPENCOMPOUND_EMBLEM, NULL, 0);
}

void CDNUserSendManager::SendOpenUpdagrageJewel()
{
	m_pSession->AddSendData(SC_NPC, eNpc::SC_OPENUPGRADE_JEWEL, NULL, 0);
}

void CDNUserSendManager::SendOpenMailBox()
{
	m_pSession->AddSendData(SC_NPC, eNpc::SC_OPENMAILBOX, NULL, 0);
}

void CDNUserSendManager::SendOpenGlyphLift()
{
	m_pSession->AddSendData(SC_NPC, eNpc::SC_OPEN_GLYPH_LIFT, NULL, 0);
}


void CDNUserSendManager::SendOpenFarmWareHouse()
{
	m_pSession->AddSendData(SC_NPC, eNpc::SC_OPEN_FARMWAREHOUSE, NULL, 0);
}


void CDNUserSendManager::SendOpenDisjointItem()
{
	m_pSession->AddSendData(SC_NPC, eNpc::SC_OPENDISJOINT_ITEM, NULL, 0);
}

void CDNUserSendManager::SendOpenUpgradeItem( int iItemID/*=0*/ )
{
	SCOpenUpgradeItem TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.iItemID = iItemID;

	m_pSession->AddSendData(SC_NPC, eNpc::SC_OPENUPGRADE_ITEM, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

#ifdef PRE_ADD_EXCHANGE_ENCHANT
void CDNUserSendManager::SendOpenExchangeEnchant()
{
	m_pSession->AddSendData(SC_NPC, eNpc::SC_OPEN_EXCHANGE_ENCHANT, NULL, 0);
}
#endif

void CDNUserSendManager::SendOpenCompoundItem(int nCompoundShopID)
{
	SCOpenCompoundItem packet;
	memset(&packet, 0, sizeof(packet));

	packet.nCompoundShopID = nCompoundShopID;

	m_pSession->AddSendData(SC_NPC, eNpc::SC_OPENCOMPOUND_ITEM, (char*)&packet, sizeof(packet));
}

void CDNUserSendManager::SendOpenCompound2Item(int nCompoundGroupID, int iItemID )
{
	SCOpenCompound2Item packet;
	memset(&packet, 0, sizeof(packet));

	packet.nCompoundGroupID = nCompoundGroupID;
	packet.iItemID = iItemID;

	m_pSession->AddSendData(SC_NPC, eNpc::SC_OPENCOMPOUND2_ITEM, (char*)&packet, sizeof(packet));
}

void CDNUserSendManager::SendOpenCashShop()
{
	m_pSession->AddSendData(SC_NPC, eNpc::SC_OPENCASHSHOP, NULL, 0);
}

void CDNUserSendManager::SendOpenGuildMgrBox(int nGuildMgrNo)
{
	SCOpenGuildMgrBox packet;
	memset(&packet, 0, sizeof(packet));

	packet.nGuildMgrNo = nGuildMgrNo;

	m_pSession->AddSendData(SC_NPC, eNpc::SC_OPENGUILDMGRBOX, (char*)&packet, sizeof(packet));
}


void CDNUserSendManager::SendOpenInventory()
{
	m_pSession->AddSendData(SC_NPC, eNpc::SC_OPEN_INVENTORY, NULL, 0);
}

#ifdef PRE_ADD_OPEN_QUEST_TEXTURE_DIALOG
void CDNUserSendManager::SendOpenTextureDialog(SCOpenTextureDialog sDialogData)
{
	SCOpenTextureDialog packet;
	memset(&packet, 0, sizeof(SCOpenTextureDialog));
	memcpy(&packet, &sDialogData, sizeof(SCOpenTextureDialog));

	m_pSession->AddSendData(SC_NPC, eNpc::SC_OPEN_TEXTURE_DIALOG, (char*)&packet, sizeof(packet));
}

void CDNUserSendManager::SendCloseTextureDialog(int nDlgID)
{
	SCCloseTextureDialog packet;
	memset(&packet, 0, sizeof(SCCloseTextureDialog));
	packet.nDialogID = nDlgID;

	m_pSession->AddSendData(SC_NPC, eNpc::SC_CLOSE_TEXTURE_DIALOG, (char*)&packet, sizeof(packet));
}
#endif

// Quest
void CDNUserSendManager::SendQuestInfo(TUserData &UserData)
{
	SendQuestInfo(&UserData.Quest);
}

void CDNUserSendManager::SendQuestInfo(TQuestGroup* pQuestGroup)
{
	DN_ASSERT(NULL != pQuestGroup,	"Invalid!");

	SCQuestInfo Info;
	memset( &Info, 0, sizeof(Info) );

	int nCount = 0;
	for( UINT i=0 ; i<DNNotifier::RegisterCount::Total ; ++i )
		Info.NotifierData[i] = m_pSession->m_NotifierRepository.GetData( i );
	for (int i = 0; i < MAX_PLAY_QUEST; i++){
		if(pQuestGroup->Quest[i].nQuestID <= 0) continue;
		Info.Quest[nCount] = pQuestGroup->Quest[i];
		nCount++;
	}
	Info.cCount = nCount;

	m_pSession->AddSendData(SC_QUEST, eQuest::SC_QUESTINFO, (char*)&Info, sizeof(SCQuestInfo) - sizeof(Info.Quest) +(sizeof(TQuest) * Info.cCount));
}

void CDNUserSendManager::SendQuestCompleteInfo(TQuestGroup &Quest)
{
	SCQuestCompleteInfo Info = { 0, };

	memcpy(Info.CompleteQuest, Quest.CompleteQuest, sizeof(Info.CompleteQuest));

	m_pSession->AddSendData(SC_QUEST, eQuest::SC_QUESTCOMPLETEINFO, (char*)&Info, sizeof(SCQuestCompleteInfo));
}

void CDNUserSendManager::SendQuestResult(int nRet)
{
	SCQuestResult packet;
	packet.nRetCode = nRet;
	m_pSession->AddSendData(SC_QUEST, eQuest::SC_QUESTRESULT, (char*)&packet, sizeof(packet));
}

void CDNUserSendManager::SendQuestRefresh(char cSlot, int nRefreshType, int nQuestID, TQuest& Quest)
{
	SCRefreshQuest refreshQuest;
	memset(&refreshQuest, 0, sizeof(SCRefreshQuest));
	
	refreshQuest.nRefreshType = (short)nRefreshType;
	refreshQuest.cQuestSlot = cSlot;
	refreshQuest.nQuestID = nQuestID;
	CopyMemory(&(refreshQuest.Quest), &Quest, sizeof(TQuest));

	m_pSession->AddSendData(SC_QUEST, eQuest::SC_REFRESH_QUEST, (char*)&refreshQuest, sizeof(SCRefreshQuest));
}

void CDNUserSendManager::SendCompleteQuest(bool bDeletePlayList, char cSlot, UINT nQuestID, bool bNeedMarking)
{
	SCCompleteQuest completeQuest;
	memset(&completeQuest, 0, sizeof(SCCompleteQuest));
	
	completeQuest.bDeletePlayList = bDeletePlayList;
	completeQuest.cQuestSlot = cSlot;
	completeQuest.nQuestID = nQuestID;
	completeQuest.bNeedMarking = bNeedMarking;

	m_pSession->AddSendData(SC_QUEST, eQuest::SC_COMPLETE_QUEST, (char*)&completeQuest, sizeof(SCCompleteQuest));
}

void CDNUserSendManager::SendAssginPeriodQuest(UINT nQuestID, bool bFlag)
{
	SCAssignPeriodQuest periodQuest;
	memset(&periodQuest, 0, sizeof(periodQuest));
	periodQuest.nQuestID = nQuestID;
	periodQuest.bFlag = bFlag;

	m_pSession->AddSendData(SC_QUEST, eQuest::SC_ASSIGN_PERIODQUEST, (char*)&periodQuest, sizeof(SCAssignPeriodQuest));
}

void CDNUserSendManager::SendPeriodQuestNotice(int nItemID, int nNoticeCount)
{
	SCPeriodQuestNotice periodQuest;
	memset(&periodQuest, 0, sizeof(periodQuest));
	periodQuest.nItemID = nItemID;
	periodQuest.nNoticeCount = nNoticeCount;

	m_pSession->AddSendData(SC_QUEST, eQuest::SC_NOTICE_PERIODQUEST, (char*)&periodQuest, sizeof(SCPeriodQuestNotice));
}

void CDNUserSendManager::SendScorePeriodQuest(int nCurrentCount, int nMaxCount)
{
	SCScorePeriodQuest periodQuest;
	memset(&periodQuest, 0, sizeof(periodQuest));
	periodQuest.nCurrentCount = nCurrentCount;
	periodQuest.nMaxCount = nMaxCount;

	m_pSession->AddSendData(SC_QUEST, eQuest::SC_SCORE_PERIODQUEST, (char*)&periodQuest, sizeof(SCScorePeriodQuest));
}

void CDNUserSendManager::SendMarkingCompleteQuest(UINT nQuestID)
{
	SCMarkingCompleteQuest completeQuest;
	memset(&completeQuest, 0, sizeof(SCMarkingCompleteQuest));
	completeQuest.nQuestID = nQuestID;

	m_pSession->AddSendData(SC_QUEST, eQuest::SC_MARKING_COMPLETE_QUEST, (char*)&completeQuest, sizeof(SCMarkingCompleteQuest));
}


void CDNUserSendManager::SendRemoveQuest(char cSlot, UINT nQuestID, short nRet)
{
	SCRemoveQuest removeQuest;
	memset(&removeQuest, 0, sizeof(SCRemoveQuest));
	
	removeQuest.cQuestSlot = cSlot;
	removeQuest.nQuestID = nQuestID;
	removeQuest.nRetCode = nRet;

	m_pSession->AddSendData(SC_QUEST, eQuest::SC_REMOVE_QUEST, (char*)&removeQuest, sizeof(SCRemoveQuest));
}

void CDNUserSendManager::SendPlayCutScene( int nCutSceneTableID, int nQuestIndex, int nQuestStep, UINT nNpcObjectID, bool bQuestAutoFadeIn, DWORD dwQuestPlayerUniqueID )
{
	SCPlayCutScene packet;
	memset(&packet, 0, sizeof(SCPlayCutScene));

	packet.nCutSceneTableID = nCutSceneTableID;
	packet.nQuestIndex = nQuestIndex;
	packet.nQuestStep = nQuestStep;
	packet.nNpcObjectID = nNpcObjectID;
	packet.bQuestAutoFadeIn = bQuestAutoFadeIn;
	packet.dwQuestPlayerUniqueID = dwQuestPlayerUniqueID;

	m_pSession->AddSendData(SC_QUEST, eQuest::SC_PLAYCUTSCENE, (char*)&packet, sizeof(SCPlayCutScene));

	m_pSession->m_nCutSceneID = nCutSceneTableID;
	//g_pLogConnection->QueryLogCutScene(m_pSession->GetWorldSetID(), m_pSession->GetAccountDBID(), m_pSession->GetCharacterDBID(), m_pSession->GetSessionID(), nCutSceneTableID, CUTSCENELOG_START);
}

void CDNUserSendManager::SendCompleteCutScene( bool bFadeIn )
{
	SCCompleteCutScene packet;
	memset( &packet, 0, sizeof(SCCompleteCutScene) );

	packet.bFadeIn = bFadeIn;

	m_pSession->AddSendData( SC_QUEST, eQuest::SC_COMPLETE_CUTSCENE, (char*)&packet, sizeof(SCCompleteCutScene) );

	if(m_pSession->m_nCutSceneID > 0){
		//g_pLogConnection->QueryLogCutScene(m_pSession->GetWorldSetID(), m_pSession->GetAccountDBID(), m_pSession->GetCharacterDBID(), m_pSession->GetSessionID(), m_pSession->m_nCutSceneID, CUTSCENELOG_COMPLETE);
		m_pSession->m_nCutSceneID = 0;
	}
}

void CDNUserSendManager::SendSkipCutScene( UINT nSessionID )
{
	SCSkipCutScene packet;

	memset( &packet, 0, sizeof(SCSkipCutScene) );

	packet.nSessionID = nSessionID;

	m_pSession->AddSendData( SC_QUEST, eQuest::SC_SKIP_CUTSCENE, (char*)&packet, sizeof(SCSkipCutScene) );

	if(m_pSession->m_nCutSceneID > 0){
		//g_pLogConnection->QueryLogCutScene(m_pSession->GetWorldSetID(), m_pSession->GetAccountDBID(), m_pSession->GetCharacterDBID(), m_pSession->GetSessionID(), m_pSession->m_nCutSceneID, CUTSCENELOG_SKIP);
		m_pSession->m_nCutSceneID = 0;
	}
}

void CDNUserSendManager::SendSkipAllCutScene()
{
	m_pSession->AddSendData( SC_QUEST, eQuest::SC_SKIPALL_CUTSCENE, NULL, 0 );

	if(m_pSession->m_nCutSceneID > 0){
		//g_pLogConnection->QueryLogCutScene(m_pSession->GetWorldSetID(), m_pSession->GetAccountDBID(), m_pSession->GetCharacterDBID(), m_pSession->GetSessionID(), m_pSession->m_nCutSceneID, CUTSCENELOG_SKIP);
		m_pSession->m_nCutSceneID = 0;
	}
}

#if defined(PRE_ADD_REMOTE_QUEST)
void CDNUserSendManager::SendGainRemoteQuest( int nQuestID)
{
	SCGainRemoteQuest packet;
	memset(&packet, 0, sizeof(packet));

	packet.nQuestID = nQuestID;
	m_pSession->AddSendData(SC_QUEST, eQuest::SC_GAIN_REMOTE_QUEST, (char*)&packet, sizeof(packet));
}

void CDNUserSendManager::SendRemoveRemoteQuest( int nQuestID)
{
	SCRemoveRemoteQuest packet;
	memset(&packet, 0, sizeof(packet));

	packet.nQuestID = nQuestID;
	m_pSession->AddSendData(SC_QUEST, eQuest::SC_REMOVE_REMOTE_QEUST, (char*)&packet, sizeof(packet));
}

void CDNUserSendManager::SendCompleteRemoteQuest( int nQuestID)
{
	SCCompleteRemoteQuest packet;
	memset(&packet, 0, sizeof(packet));

	packet.nQuestID = nQuestID;
	m_pSession->AddSendData(SC_QUEST, eQuest::SC_COMPLETE_REMOTE_QEUST, (char*)&packet, sizeof(packet));
}

#endif

// Chat
void CDNUserSendManager::SendChat(char cType, int cLen, const WCHAR *pwszCharacterName, const WCHAR *pwszChatMsg, TParamData* pParamData, int nRet )
{
	if( pwszCharacterName == NULL || pwszChatMsg == NULL || wcslen( pwszCharacterName ) >= NAMELENMAX || cLen > CHATLENMAX )
		return;

	SCChat Chat;
	memset(&Chat, 0, sizeof(SCChat));

	Chat.eType = (eChatType)cType;
	_wcscpy(Chat.wszFromCharacterName, _countof(Chat.wszFromCharacterName), pwszCharacterName, (int)wcslen(pwszCharacterName));
	Chat.nRet = nRet;

	if(nRet == ERROR_NONE){
		_wcscpy(Chat.wszChatMsg, _countof(Chat.wszChatMsg), pwszChatMsg, (int)wcslen(pwszChatMsg));
		Chat.nLen = cLen;
#if defined( _VILLAGESERVER )
		m_pSession->AddSendData(SC_CHAT, eChat::SC_CHATMSG, (char*)&Chat, (int)(sizeof(SCChat) - sizeof(Chat.wszChatMsg) +(cLen * 2)), pParamData );
#else
		m_pSession->AddSendData(SC_CHAT, eChat::SC_CHATMSG, (char*)&Chat, (int)(sizeof(SCChat) - sizeof(Chat.wszChatMsg) +(cLen * 2)) );
#endif
	}
	else {
		m_pSession->AddSendData(SC_CHAT, eChat::SC_CHATMSG, (char*)&Chat, (int)(sizeof(SCChat) - sizeof(Chat.wszChatMsg)));
	}
}

void CDNUserSendManager::SendNotice(const WCHAR * pMsg, int nLen, int nSlideShowSec)
{
	if(pMsg == NULL) return;
	SCNotice packet;
	memset(&packet, 0, sizeof(packet));

	packet.nShowSec = nSlideShowSec;
	packet.nLen = nLen;
	_wcscpy(packet.wszNoticeMsg, _countof(packet.wszNoticeMsg), pMsg, (int)wcslen(pMsg));

	m_pSession->AddSendData(SC_CHAT, eChat::SC_NOTICE, (char*)&packet, sizeof(SCNotice));
}

void CDNUserSendManager::SendWorldSystemMsg(char cType, int nID, const WCHAR *pwszCharacterName, int nValue, const WCHAR* pwszToCharacterName)
{
	if( pwszCharacterName == NULL || wcslen( pwszCharacterName ) >= NAMELENMAX ) return;


	SCWorldSystemMsg Msg;
	memset(&Msg, 0, sizeof(SCWorldSystemMsg));

	_wcscpy(Msg.wszFromCharacterName, _countof(Msg.wszFromCharacterName), pwszCharacterName, (int)wcslen(pwszCharacterName));
	Msg.cType = cType;
	Msg.nID = nID;
	Msg.nValue = nValue;
	
	if (pwszToCharacterName)
		_wcscpy(Msg.wszToCharacterName, _countof(Msg.wszToCharacterName), pwszToCharacterName, (int)wcslen(pwszToCharacterName));

	m_pSession->AddSendData(SC_CHAT, eChat::SC_WORLDSYSTEMMSG, (char*)&Msg, sizeof(SCWorldSystemMsg));
}

void CDNUserSendManager::SendDebugChat(const WCHAR* szMsg, eChatType eType/*=CHATTYPE_NORMAL*/)
{
	std::wstring wszString;
	wszString = szMsg;
	SendChat(eType, (int)wszString.size()*sizeof(WCHAR), L"", (WCHAR*)wszString.c_str());
}

void CDNUserSendManager::SendNoticeCancel()
{
	m_pSession->AddSendData(SC_CHAT, eChat::SC_CANCELNOTICE, NULL, 0);
}

void CDNUserSendManager::SendServerMsg( char cType, char cStringType, int nMsgIndex, char cCaptionType, BYTE cFadeTime )
{
	SCChatServerMsg packet;
	memset(&packet, 0, sizeof(SCChatServerMsg));

	packet.cType = cType;
	packet.cStringType = cStringType;
	packet.nMsgIndex = nMsgIndex;
	packet.cCaption = cCaptionType;

	m_pSession->AddSendData(SC_CHAT, eChat::SC_SERVERMSG, (char*)&packet, sizeof(SCChatServerMsg));
}

void CDNUserSendManager::SendCloseService(__time64_t _tNow, __time64_t _tOderedTime, __time64_t _tCloseTime)
{
	SCServiceClose packet;
	memset(&packet, 0, sizeof(SCServiceClose));

	packet._tNow = _tNow;
	packet._tOderedTime = _tOderedTime;
	packet._tCloseTime = _tCloseTime;

	m_pSession->AddSendData(SC_SYSTEM, eSystem::SC_SERVICECLOSE, (char*)&packet, sizeof(SCServiceClose));
}


void CDNUserSendManager::SendTcpPing( DWORD dwTick )
{
	SCTCPPing TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.dwTick = dwTick;

	m_pSession->AddSendData( SC_SYSTEM, eSystem::SC_TCP_PING, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

void CDNUserSendManager::SendClientPing( DWORD dwTick )
{
	SCClientPing TxPacket;
	TxPacket.dwTick = dwTick;
	m_pSession->AddSendData( SC_SYSTEM, eSystem::SC_CLIENT_PING, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}


// RadioMessage
void CDNUserSendManager::SendUseRadio(UINT nSessionID, USHORT nRadioID)
{
	SCUseRadio packet;
	memset(&packet, 0, sizeof(packet));

	packet.nSessionID = nSessionID;
	packet.nID = nRadioID;

	m_pSession->AddSendData(SC_RADIO, eRadio::SC_USERADIO, (char*)&packet, sizeof(packet));
}

//Gesture
void CDNUserSendManager::SendCashGestureList()
{
	TGesture  GestureArr[GESTUREMAX];
	memset(&GestureArr, 0, sizeof(GestureArr));

	if( m_pSession->GetGesture() == NULL )	return;
	m_pSession->GetGesture()->GetCashGestureList( GestureArr );

	SCGestureList packet;
	memset(&packet, 0, sizeof(packet));

	for (int i = 0; i < GESTUREMAX; i++)
	{
		if(GestureArr[i].nGestureID <= 0) continue;
		packet.nGestureID[packet.cGestureCount] = GestureArr[i].nGestureID;
		packet.cGestureCount++;
	}

	m_pSession->AddSendData(SC_GESTURE, eGesture::SC_CASHGESTURELIST, (char*)&packet, sizeof(packet) - sizeof(packet.nGestureID) +(sizeof(USHORT) * packet.cGestureCount));
}

void CDNUserSendManager::SendCashGestureAdd( USHORT nGestureID )
{
	if (nGestureID <= 0) return;	

	SCGestureAdd GestureAdd;
	GestureAdd.nGestureID = nGestureID;
	m_pSession->AddSendData( SC_GESTURE, eGesture::SC_CASHGESTUREADD, (char*)&GestureAdd, sizeof(GestureAdd) );
}

void CDNUserSendManager::SendUseGesture( USHORT nGestureID )
{
	SCUseGesture UseGesture;

	UseGesture.nGestureID = nGestureID;

	m_pSession->AddSendData( SC_GESTURE, eGesture::SC_USEGESTURE, (char*)&UseGesture, sizeof(UseGesture) );
}

void CDNUserSendManager::SendEffectItemGestureList()
{
	TGesture  GestureArr[GESTUREMAX];
	memset(&GestureArr, 0, sizeof(GestureArr));
	if( m_pSession->GetGesture() == NULL )	return;
	m_pSession->GetGesture()->GetEffectItemGestureList( GestureArr );

	SCGestureList packet;
	memset(&packet, 0, sizeof(packet));

	for (int i = 0; i < GESTUREMAX; i++)
	{
		if(GestureArr[i].nGestureID <= 0) continue;
		packet.nGestureID[packet.cGestureCount] = GestureArr[i].nGestureID;
		packet.cGestureCount++;
	}

	m_pSession->AddSendData(SC_GESTURE, eGesture::SC_EFFECTITEMGESTURELIST, (char*)&packet, sizeof(packet) - sizeof(packet.nGestureID) +(sizeof(USHORT) * packet.cGestureCount));
}

void CDNUserSendManager::SendEffectItemGestureAdd( USHORT nGestureID )
{
	if (nGestureID <= 0) return;	

	SCGestureAdd GestureAdd;
	GestureAdd.nGestureID = nGestureID;
	m_pSession->AddSendData( SC_GESTURE, eGesture::SC_EFFECTITEMGESTUREADD, (char*)&GestureAdd, sizeof(GestureAdd) );
}

// Skill
void CDNUserSendManager::SendSkill(TSkill *SkillArray, char cSkillPage)
{
	SCSkillList SkillList;
	memset(&SkillList, 0, sizeof(SCSkillList));

	int nCount = 0;
	for (int i = 0; i < SKILLMAX; i++){
		if(SkillArray[i].nSkillID <= 0) continue;
		SkillList.Skill[i] = SkillArray[i];
		nCount++;
	}
	SkillList.cSkillCount = nCount;
	SkillList.cSkillPage = cSkillPage;
	int nSize = (sizeof(SkillList) - sizeof(SkillList.Skill)) +(sizeof(TSkill) * nCount);
	m_pSession->AddSendData(SC_SKILL, eSkill::SC_SKILLLIST, (char*)&SkillList, nSize);
}

void CDNUserSendManager::SendAddSkill(int nSkillID, int nRet)
{
	SCAddSkill AddSkill;
	memset(&AddSkill, 0, sizeof(SCAddSkill));

	AddSkill.nSkillID = nSkillID;
	AddSkill.nRet = nRet;

	m_pSession->AddSendData(SC_SKILL, eSkill::SC_ADDSKILL, (char*)&AddSkill, sizeof(SCAddSkill));
}

void CDNUserSendManager::SendDelSkill(int nSkillID, int nRet)
{
	SCDelSkill DelSkill;
	memset(&DelSkill, 0, sizeof(SCDelSkill));

	DelSkill.nSkillID = nSkillID;
	DelSkill.nRet = nRet;

	m_pSession->AddSendData(SC_SKILL, eSkill::SC_DELSKILL, (char*)&DelSkill, sizeof(SCDelSkill));
}

void CDNUserSendManager::SendSkillLevelUp(int nSkillID, BYTE cLevel, int nRet)
{
	SCSkillLevelUp LevelUp;
	memset(&LevelUp, 0, sizeof(SCSkillLevelUp));

	LevelUp.nSkillID = nSkillID;
	LevelUp.cLevel = cLevel;
	LevelUp.nRetCode = nRet;

	m_pSession->AddSendData(SC_SKILL, eSkill::SC_SKILLLEVELUP, (char*)&LevelUp, sizeof(SCSkillLevelUp));
}


void CDNUserSendManager::SendOtherPlayerSkillLevelUp( int nOtherPlayerIndex, int nSkillID, char cLevel )
{
	SCOtherPlayerSkillLevelUp LevelUp;
	memset( &LevelUp, 0, sizeof(SCOtherPlayerSkillLevelUp) );
	
	LevelUp.nPartyIndex = nOtherPlayerIndex;
	LevelUp.nSkillID = nSkillID;
	LevelUp.cLevel = cLevel;

	m_pSession->AddSendData( SC_SKILL, eSkill::SC_OTHERPLAYERSKILLLEVELUP, (char*)&LevelUp, sizeof(SCOtherPlayerSkillLevelUp) );
}


void CDNUserSendManager::SendPushSkillPoint( unsigned short usSkillPoint )
{
	SCPushSkillPoint PushSkillPoint;
	memset( &PushSkillPoint, 0, sizeof(SCPushSkillPoint) );

	PushSkillPoint.usSkillPoint = usSkillPoint;

	m_pSession->AddSendData( SC_SKILL, eSkill::SC_PUSHSKILLPOINT, (char*)&PushSkillPoint, sizeof(SCPushSkillPoint) );
}


void CDNUserSendManager::SendUnlockSkill( int nSkillID, int nRetCode )
{
	SCUnlockSkill UnlockSkill;
	UnlockSkill.nSkillID = nSkillID;
	UnlockSkill.nRetCode = (short)nRetCode;

	m_pSession->AddSendData( SC_SKILL, eSkill::SC_UNLOCKSKILL, (char*)&UnlockSkill, sizeof(SCUnlockSkill) );
}

void CDNUserSendManager::SendAcquireSkill( int nSkillID, int nRetCode )
{
	SCAcquireSkill AcquireSkill;
	AcquireSkill.nSkillID = nSkillID;
	AcquireSkill.nRetCode = (short)nRetCode;

	m_pSession->AddSendData( SC_SKILL, eSkill::SC_ACQUIRESKILL, (char*)&AcquireSkill, sizeof(SCAcquireSkill) );
}

void CDNUserSendManager::SendSkillReset( int nResultSkillPoint, char cSkillPage )
{
	// Send the final reset skill points together.
	SCSkillReset SkillReset;
	SkillReset.nResultSkillPoint = nResultSkillPoint;
	SkillReset.cSkillPage = cSkillPage;

	m_pSession->AddSendData( SC_SKILL, eSkill::SC_SKILL_RESET, (char*)&SkillReset, sizeof(SCSkillReset) );
}

void CDNUserSendManager::SendCanUseSkillResetCashItem( INT64 biSerial, int nRewardSP, int nRetCode )
{
	// Can use the requested skill reset item.
	SCCanUseSkillResetCashItem CanUse;
	memset( &CanUse, 0, sizeof(CanUse) );
	CanUse.biItemSerial = biSerial;
	CanUse.nRewardSP = nRewardSP;
	CanUse.nRet = nRetCode;
	m_pSession->AddSendData( SC_SKILL, eSkill::SC_CANUSE_SKILLRESET_CASH_ITEM, (char*)&CanUse, sizeof(SCCanUseSkillResetCashItem) );
}

void CDNUserSendManager::SendUseSkillResetCashItemRes( int nRetCode, int nResultSP, char cSkillPage )
{
	SCUseSkillResetCashItemRes Res;
	Res.nRet = nRetCode;
	Res.nResultSP = nResultSP;
	Res.cSkillPage = cSkillPage;
	m_pSession->AddSendData( SC_SKILL, eSkill::SC_USE_SKILLRESET_CASHITEM_RES, (char*)&Res, sizeof(SCUseSkillResetCashItemRes) );

}

void CDNUserSendManager::SendSkillPageCount( char cSkillPageCount )
{
	SCSkillPageCount Res;
	Res.cSkillPageCount = cSkillPageCount;	
	m_pSession->AddSendData( SC_SKILL, eSkill::SC_SKILLPAGE_COUNT, (char*)&Res, sizeof(SCSkillPageCount) );
}
void CDNUserSendManager::SendChangeSkillPage( char cSkillPage )
{
	SCChangeSkillPage Res;
	Res.cSkillPage = cSkillPage;	
	m_pSession->AddSendData( SC_SKILL, eSkill::SC_CHANGE_SKILLPAGE_RES, (char*)&Res, sizeof(SCChangeSkillPage) );
}

// Trade - Shop
void CDNUserSendManager::SendShopOpen(int nShopID, Shop::Type::eCode Type/*=Shop::Type::Normal*/ )
{
	SCShopOpen ShopOpen;
	memset(&ShopOpen, 0, sizeof(SCShopOpen));
	ShopOpen.nShopID = nShopID;
	ShopOpen.Type = Type;

	m_pSession->AddSendData(SC_TRADE, eTrade::SC_SHOP_OPEN, (char*)&ShopOpen, sizeof(SCShopOpen));
}

void CDNUserSendManager::SendShopBuyResult(int nRet)
{
	SCShopBuyResult Result;
	memset(&Result, 0, sizeof(SCShopBuyResult));
	Result.nRet = nRet;
	m_pSession->AddSendData(SC_TRADE, eTrade::SC_SHOP_BUY, (char*)&Result, sizeof(SCShopBuyResult));
}

void CDNUserSendManager::SendShopSellResult(int nRet)
{
	SCShopSellResult Result;
	memset(&Result, 0, sizeof(SCShopSellResult));
	Result.nRet = nRet;
	m_pSession->AddSendData(SC_TRADE, eTrade::SC_SHOP_SELL, (char*)&Result, sizeof(SCShopSellResult));
}

void CDNUserSendManager::SendShopRepurchaseList( int nRet, BYTE cCount/*=0*/, const TRepurchaseItemInfo* pList/*=NULL*/ )
{
	SCShopRepurchaseList TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.nRet = nRet;
	if( nRet == ERROR_NONE && cCount > 0 && pList )
	{
		for( int i=0 ; i<cCount ; ++i )
		{
			TxPacket.List[i] = *pList;
			++pList;
			++TxPacket.cCount;
		}
	}

	int iSize = sizeof(TxPacket)-sizeof(TxPacket.List)+(sizeof(TxPacket.List[0])*TxPacket.cCount);
	m_pSession->AddSendData(SC_TRADE, eTrade::SC_SHOP_GETLIST_REPURCHASE,reinterpret_cast<char*>(&TxPacket), iSize );
}

void CDNUserSendManager::SendShopRepurchase( int iRet, int iRepurchaseID/*=-1*/ )
{
	SCShopRepurchase TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.iRet =iRet;
	TxPacket.iRepurchaseID =iRepurchaseID;

	m_pSession->AddSendData(SC_TRADE, eTrade::SC_SHOP_REPURCHASE,reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

// Trade - SkillShop
void CDNUserSendManager::SendSkillShopOpen( void )
{
	SCSkillShopOpen SkillShopOpen;
	memset( &SkillShopOpen, 0, sizeof(SCSkillShopOpen) );

	SkillShopOpen.nRet = ERROR_NONE;

	m_pSession->AddSendData( SC_TRADE, eTrade::SC_SKILLSHOP_OPEN, (char*)&SkillShopOpen, sizeof(SCSkillShopOpen) );
}

// Trade - Mail
void CDNUserSendManager::SendMailBox(const TMailBoxInfo *MailArray, short wTotalCount, short wUnreadCount, char cMailCount, char cDailyMailCount, int nRet)
{
	SCMailBox MailBox;
	memset(&MailBox, 0, sizeof(SCMailBox));

	MailBox.nRetCode = nRet;
	int nCount = (int)(CGlobalWeightTable::GetInstance().GetValue(CGlobalWeightTable::DailyMailCount));
	MailBox.cDailyMailCount = nCount - cDailyMailCount;
	if(MailBox.cDailyMailCount < 0) MailBox.cDailyMailCount = 0;
	MailBox.wTotalCount = wTotalCount;
	MailBox.wUnreadCount = wUnreadCount;
	MailBox.cMailBoxCount = cMailCount;

	if(cMailCount > 0 && MailArray){
		memcpy(MailBox.MailBox, MailArray, sizeof(TMailBoxInfo) * cMailCount);
	}

	int nLen = sizeof(SCMailBox) - sizeof(MailBox.MailBox) +(sizeof(TMailBoxInfo) * cMailCount);
	m_pSession->AddSendData(SC_TRADE, eTrade::SC_MAILBOX, (char*)&MailBox, nLen);
}

void CDNUserSendManager::SendMailSendResult(int nRet, int nDailyMailCount)
{
	SCSendMail Result;
	memset(&Result, 0, sizeof(SCSendMail));
	int nCount = (int)(CGlobalWeightTable::GetInstance().GetValue(CGlobalWeightTable::DailyMailCount));
	Result.cDailyMailCount = nCount - nDailyMailCount;
	if(Result.cDailyMailCount < 0) Result.cDailyMailCount = 0;
	Result.nRet = nRet;
	m_pSession->AddSendData(SC_TRADE, eTrade::SC_MAIL_SEND, (char*)&Result, sizeof(SCSendMail));
}

void CDNUserSendManager::SendReadMailResult(const TAReadMail *pRead, int nAttachCoin)
{
	SCReadMail ReadMail;
	memset(&ReadMail, 0, sizeof(SCReadMail));

	if (pRead){
		ReadMail.nRet = pRead->nRetCode;
		ReadMail.wNotReadMailCount = pRead->iNotReadMailCount;
		ReadMail.nMailDBID = pRead->nMailDBID;
		ReadMail.bNewFlag = pRead->bNewFlag;
	
		if (ReadMail.nRet == ERROR_NONE)
		{
			_wcscpy(ReadMail.wszText, MAILTEXTLENMAX, pRead->wszContent, MAILTEXTLENMAX);
			ReadMail.tSendDate = pRead->SendDate;
			ReadMail.nAttachCoin = nAttachCoin;
			ReadMail.cDeliveryType = pRead->cDeliveryType;

			int nCount = 0;
			for (int i = 0; i < MAILATTACHITEMMAX; i++){
				if (pRead->sAttachItem[i].nItemID <= 0) continue;
				ReadMail.AttachItem[nCount].cSlotIndex = i;
				ReadMail.AttachItem[nCount].Item = pRead->sAttachItem[i];
				nCount++;
			}
			ReadMail.cAttachCount = nCount;
		}
	}
	else
		ReadMail.nRet = ERROR_GENERIC_INVALIDREQUEST;

	int nLen = sizeof(SCReadMail) - sizeof(ReadMail.AttachItem) + (sizeof(TItemInfo) * ReadMail.cAttachCount);
	m_pSession->AddSendData(SC_TRADE, eTrade::SC_MAIL_READ, (char*)&ReadMail, nLen);
}

#if defined(PRE_ADD_CADGE_CASH)
void CDNUserSendManager::SendReadCadgeMailResult(const TAReadWishMail *pRead)
{
	SCReadCadgeMail ReadMail;
	memset(&ReadMail, 0, sizeof(SCReadCadgeMail));

	if (pRead){
		ReadMail.nRet = pRead->nRetCode;
		ReadMail.bNewFlag = pRead->bNewFlag;
		ReadMail.wNotReadMailCount = pRead->nNotReadMailCount;
		ReadMail.nMailDBID = pRead->nMailDBID;

		if (ReadMail.nRet == ERROR_NONE)
		{
			_wcscpy(ReadMail.wszText, MAILTEXTLENMAX, pRead->wszContent, MAILTEXTLENMAX);
			ReadMail.tSendDate = pRead->SendDate;

			if (pRead->biPurchaseOrderID > 0)
				ReadMail.bCadgeComplete = true;

			ReadMail.nPackageSN = pRead->nPackageSN;

			ReadMail.cWishListCount = pRead->cWishListCount;
			for (int i = 0; i < pRead->cWishListCount; i++){
				ReadMail.WishList[i].nItemSN = pRead->WishList[i].nItemSN;
				ReadMail.WishList[i].nItemID = pRead->WishList[i].nItemID;
				ReadMail.WishList[i].nOptionIndex = pRead->WishList[i].cOption;
			}
		}
	}
	else
		ReadMail.nRet = ERROR_GENERIC_INVALIDREQUEST;

	int nLen = sizeof(SCReadCadgeMail) - sizeof(ReadMail.WishList) + (sizeof(TWishItemInfo) * ReadMail.cWishListCount);
	m_pSession->AddSendData(SC_TRADE, eTrade::SC_MAIL_READCADGE, (char*)&ReadMail, nLen);
}

#endif	// #if defined(PRE_ADD_CADGE_CASH)

void CDNUserSendManager::SendDeleteMailResult(int nRet)
{
	SCMailResult Result;
	memset(&Result, 0, sizeof(SCMailResult));
	Result.nRet = nRet;
	m_pSession->AddSendData(SC_TRADE, eTrade::SC_MAIL_DELETE, (char*)&Result, sizeof(SCMailResult));
}

void CDNUserSendManager::SendAttachAllMailResult(int *nMailDBIDArray, int nRet)
{
	SCAttachAllMailResult Result;
	memset(&Result, 0, sizeof(SCAttachAllMailResult));
	Result.nRet = nRet;
	if(nMailDBIDArray) memcpy(Result.nMailDBID, nMailDBIDArray, sizeof(Result.nMailDBID));
	m_pSession->AddSendData(SC_TRADE, eTrade::SC_MAIL_ATTACHALL, (char*)&Result, sizeof(SCAttachAllMailResult));
}

void CDNUserSendManager::SendAttachMailResult(int nRet)
{
	SCMailResult Result;
	memset(&Result, 0, sizeof(SCMailResult));
	Result.nRet = nRet;
	m_pSession->AddSendData(SC_TRADE, eTrade::SC_MAIL_ATTACH, (char*)&Result, sizeof(SCMailResult));
}

void CDNUserSendManager::SendNotifyMail(int nTotalMailCount, int nNotReadMailCount, int wExpirationCount, bool bNewMail)
{
	SCNotifyMail Mail = { 0, };
	Mail.wTotalMailCount = nTotalMailCount;
	Mail.wNotReadMailCount = nNotReadMailCount;
	if(wExpirationCount > 0) Mail.bExpiration = true;
	Mail.bNewMail = bNewMail;

	m_pSession->AddSendData(SC_TRADE, eTrade::SC_MAIL_NOTIFY, (char*)&Mail, sizeof(SCNotifyMail));
}

// Trade - Exchange (Personal Trade)
void CDNUserSendManager::SendExchangeRequest(UINT nSenderSessionID, int nRet)
{
	SCExchangeRequest Request;
	memset(&Request, 0, sizeof(SCExchangeRequest));
	Request.nSenderSessionID = nSenderSessionID;
	Request.nRet = nRet;
	m_pSession->AddSendData(SC_TRADE, eTrade::SC_EXCHANGE_REQUEST, (char*)&Request, sizeof(SCExchangeRequest));
}

void CDNUserSendManager::SendExchangeReject(UINT nSessionID, short nRetCode)
{
	SCExchangeReject Reject;
	memset(&Reject, 0, sizeof(SCExchangeReject));
	Reject.nSessionID = nSessionID;
	Reject.nRetCode = nRetCode;
	m_pSession->AddSendData(SC_TRADE, eTrade::SC_EXCHANGE_REJECT, (char*)&Reject, sizeof(SCExchangeReject));
}

void CDNUserSendManager::SendExchangeStart(UINT nTargetSessionID, int nRet)
{
	SCExchangeStart Start;
	memset(&Start, 0, sizeof(SCExchangeStart));
	Start.nTargetSessionID = nTargetSessionID;
	Start.nRet = nRet;
	m_pSession->AddSendData(SC_TRADE, eTrade::SC_EXCHANGE_START, (char*)&Start, sizeof(SCExchangeStart));
}

void CDNUserSendManager::SendExchangeAddItem(UINT nSessionID, char cExchangeIndex, BYTE cInvenIndex, TItem &InvenItem, int nRet)
{
	SCExchangeAddItem AddItem;
	memset(&AddItem, 0, sizeof(SCExchangeAddItem));

	AddItem.nSessionID = nSessionID;
	AddItem.cExchangeIndex = cExchangeIndex;
	AddItem.ItemInfo.cSlotIndex = cInvenIndex;
	AddItem.ItemInfo.Item = InvenItem;
	AddItem.nRet = nRet;

	m_pSession->AddSendData(SC_TRADE, eTrade::SC_EXCHANGE_ADDITEM, (char*)&AddItem, sizeof(SCExchangeAddItem));
}

void CDNUserSendManager::SendExchangeDeleteItem(UINT nSessionID, char cExchangeIndex, int nRet)
{
	SCExchangeDeleteItem DeleteItem;
	memset(&DeleteItem, 0, sizeof(SCExchangeDeleteItem));

	DeleteItem.nSessionID = nSessionID;
	DeleteItem.cExchangeIndex = cExchangeIndex;
	DeleteItem.nRet = nRet;

	m_pSession->AddSendData(SC_TRADE, eTrade::SC_EXCHANGE_DELETEITEM, (char*)&DeleteItem, sizeof(SCExchangeDeleteItem));
}

void CDNUserSendManager::SendExchangeAddCoin(UINT nSessionID, INT64 nCoin, int nRet)
{
	SCExchangeAddCoin AddCoin;
	memset(&AddCoin, 0, sizeof(SCExchangeAddCoin));

	AddCoin.nSessionID = nSessionID;
	AddCoin.nCoin = nCoin;
	AddCoin.nRet = nRet;

	m_pSession->AddSendData(SC_TRADE, eTrade::SC_EXCHANGE_ADDCOIN, (char*)&AddCoin, sizeof(SCExchangeAddCoin));
}

void CDNUserSendManager::SendExchangeConfirm(UINT nSessionID, char cType)
{
	SCExchangeConfirm Confirm;
	memset(&Confirm, 0, sizeof(SCExchangeConfirm));
	Confirm.cType = cType;
	Confirm.nSessionID = nSessionID;
	m_pSession->AddSendData(SC_TRADE, eTrade::SC_EXCHANGE_CONFIRM, (char*)&Confirm, sizeof(SCExchangeConfirm));
}

void CDNUserSendManager::SendExchangeCancel()
{
	m_pSession->AddSendData(SC_TRADE, eTrade::SC_EXCHANGE_CANCEL, NULL, 0);
}

void CDNUserSendManager::SendExchangeComplete(int nRet)
{
	SCExchangeComplete Complete;
	memset(&Complete, 0, sizeof(SCExchangeComplete));
	Complete.nRet = nRet;
	m_pSession->AddSendData(SC_TRADE, eTrade::SC_EXCHANGE_COMPLETE, (char*)&Complete, sizeof(SCExchangeComplete));	
}

// Trade - Market
void CDNUserSendManager::SendMarketList(TMarketInfo *MarketArray, char cMarketCount, int nMarketTotalCount, int nRet)
{
	SCMarketList MarketList;
	memset(&MarketList, 0, sizeof(SCMarketList));	

	MarketList.nRetCode = nRet;
	MarketList.nMarketTotalCount = nMarketTotalCount;
	MarketList.cMarketCount = cMarketCount;
	if(MarketList.cMarketCount < 0) MarketList.cMarketCount = 0;
	if(MarketList.cMarketCount > MARKETMAX) MarketList.cMarketCount = MARKETMAX;
	if(cMarketCount > 0 && MarketArray != NULL)
		memcpy(MarketList.MarketInfo, MarketArray, sizeof(TMarketInfo) * cMarketCount);

	int nLen = sizeof(SCMarketList) - sizeof(MarketList.MarketInfo) +(sizeof(TMarketInfo) * cMarketCount);
	m_pSession->AddSendData(SC_TRADE, eTrade::SC_MARKETLIST, (char*)&MarketList, nLen);
}

void CDNUserSendManager::SendMyMarketList(TMyMarketInfo *MarketArray, char cMarketCount, int nSellingCount, int nClosingCount, int nWeeklyRegisterCount, int nRegisterItemCount, int nRet, bool bPremiumTrade )
{
	SCMyMarketList MarketList;
	memset(&MarketList, 0, sizeof(SCMyMarketList));

	MarketList.nRetCode = nRet;
	MarketList.wSellingCount = nSellingCount;
	MarketList.wClosingCount = nClosingCount;
	MarketList.wWeeklyRegisterCount = nWeeklyRegisterCount;
	MarketList.wRegisterItemCount = nRegisterItemCount;
	MarketList.bPremiumTrade = bPremiumTrade;
	MarketList.cMarketCount = cMarketCount;
	if(MarketList.cMarketCount < 0) MarketList.cMarketCount = 0;
	if(MarketList.cMarketCount > MYMARKETMAX) MarketList.cMarketCount = MYMARKETMAX;
	if(cMarketCount > 0 && MarketArray)
		memcpy(MarketList.MarketInfo, MarketArray, sizeof(TMyMarketInfo) * cMarketCount);

	int nLen = sizeof(SCMyMarketList) - sizeof(MarketList.MarketInfo) +(sizeof(TMyMarketInfo) * cMarketCount);
	m_pSession->AddSendData(SC_TRADE, eTrade::SC_MYMARKETLIST, (char*)&MarketList, nLen);
}

void CDNUserSendManager::SendMarketRegister(int nMarketDBID, int nRet)
{
	SCMarketResult Result;
	memset(&Result, 0, sizeof(SCMarketResult));
	Result.nMarketDBID = nMarketDBID;
	Result.nRet = nRet;
	m_pSession->AddSendData(SC_TRADE, eTrade::SC_MARKET_REGISTER, (char*)&Result, sizeof(SCMarketResult));
}

void CDNUserSendManager::SendMarketInterrupt(int nMarketDBID, int nRegisterCount, int nRet)
{
	SCMarketInterrupt Result;
	memset(&Result, 0, sizeof(SCMarketInterrupt));
	Result.nMarketDBID = nMarketDBID;
	Result.wRegisterItemCount = nRegisterCount;
	Result.nRet = nRet;
	m_pSession->AddSendData(SC_TRADE, eTrade::SC_MARKET_INTERRUPT, (char*)&Result, sizeof(SCMarketInterrupt));
}

void CDNUserSendManager::SendMarketBuy(int nMarketDBID, int nRet, bool bMini)
{
	SCMarketBuyResult Result;
	memset(&Result, 0, sizeof(SCMarketBuyResult));
	Result.nMarketDBID = nMarketDBID;
	Result.nRet = nRet;
#if defined(PRE_ADD_DIRECT_BUY_UPGRADEITEM)
	Result.bMini = bMini;
#endif	// #if defined(PRE_ADD_DIRECT_BUY_UPGRADEITEM)
	m_pSession->AddSendData(SC_TRADE, eTrade::SC_MARKET_BUY, (char*)&Result, sizeof(SCMarketBuyResult));
}

void CDNUserSendManager::SendMarketCalculationList(TMarketCalculationInfo *MarketArray, char cMarketCount, int nRet)
{
	SCMarketCalculationList MarketList;
	memset(&MarketList, 0, sizeof(SCMarketCalculationList));

	MarketList.nRetCode = nRet;
	MarketList.cMarketCount = cMarketCount;
	if(MarketList.cMarketCount < 0) MarketList.cMarketCount = 0;
	if(MarketList.cMarketCount > MYMARKETMAX) MarketList.cMarketCount = MYMARKETMAX;
	if(cMarketCount > 0 && MarketArray)
		memcpy(MarketList.MarketInfo, MarketArray, sizeof(TMarketCalculationInfo) * cMarketCount);

	int nLen = sizeof(SCMarketCalculationList) - sizeof(MarketList.MarketInfo) +(sizeof(TMarketCalculationInfo) * cMarketCount);
	m_pSession->AddSendData(SC_TRADE, eTrade::SC_MARKET_CALCULATIONLIST, (char*)&MarketList, nLen);
}

void CDNUserSendManager::SendMarketCalculation(int nMarketDBID, int nRet)
{
	SCMarketResult Result;
	memset(&Result, 0, sizeof(SCMarketResult));
	Result.nMarketDBID = nMarketDBID;
	Result.nRet = nRet;
	m_pSession->AddSendData(SC_TRADE, eTrade::SC_MARKET_CALCULATION, (char*)&Result, sizeof(SCMarketResult));
}

void CDNUserSendManager::SendMarketCalculationAll(int nRet)
{
	SCMarketCalculationAll Market;
	memset(&Market, 0, sizeof(SCMarketCalculationAll));
	Market.nRet = nRet;
	m_pSession->AddSendData(SC_TRADE, eTrade::SC_MARKET_CALCULATIONALL, (char*)&Market, sizeof(SCMarketCalculationAll));
}

void CDNUserSendManager::SendNotifyMarket(int nItemID, short wCalculationCount)
{
	SCNotifyMarket Market;
	memset(&Market, 0, sizeof(SCNotifyMarket));
	Market.nItemID = nItemID;
	Market.wCalculationCount = wCalculationCount;
	m_pSession->AddSendData(SC_TRADE, eTrade::SC_MARKET_NOTIFY, (char*)&Market, sizeof(SCNotifyMarket));
}

void CDNUserSendManager::SendMarketPetalBalance(int nPetalBalance, int nRet)
{
	SCMarketPetalBalance Market;
	memset(&Market, 0, sizeof(SCMarketPetalBalance));
	Market.nPetalBalance = nPetalBalance;
	Market.nRetCode = nRet;
	m_pSession->AddSendData(SC_TRADE, eTrade::SC_MARKET_PETALBALANCE, (char*)&Market, sizeof(SCMarketPetalBalance));
}

void CDNUserSendManager::SendMarketPrice(int nRet, int nMarketDBID, TMarketPrice * pPrice)
{
	SCMarketPrice Market;
	memset(&Market, 0, sizeof(SCMarketPrice));

	Market.nRetCode = nRet;
	Market.nMarketDBID = nMarketDBID;
	memcpy(Market.ItemPrices, pPrice, sizeof(Market.ItemPrices));

	m_pSession->AddSendData(SC_TRADE, eTrade::SC_MARKET_PRICE, (char*)&Market, sizeof(SCMarketPrice));
}

void CDNUserSendManager::SendMarketMiniList(TMarketInfo *MarketArray, char cMarketCount, int nMarketTotalCount, int nRet)
{
	SCMarketMiniList MarketList;
	memset(&MarketList, 0, sizeof(SCMarketMiniList));	

	MarketList.nRetCode = nRet;
	MarketList.nMarketTotalCount = nMarketTotalCount;
	MarketList.cMarketCount = cMarketCount;
	if (MarketList.cMarketCount < 0) MarketList.cMarketCount = 0;
	if (MarketList.cMarketCount > MARKETMAX) MarketList.cMarketCount = MARKETMAX;
	if (cMarketCount > 0 && MarketArray != NULL)
		memcpy(MarketList.MarketInfo, MarketArray, sizeof(TMarketInfo) * cMarketCount);

	int nLen = sizeof(SCMarketMiniList) - sizeof(MarketList.MarketInfo) + (sizeof(TMarketInfo) * cMarketCount);
	m_pSession->AddSendData(SC_TRADE, eTrade::SC_MARKETMINILIST, (char*)&MarketList, nLen);
}

#if defined(PRE_SPECIALBOX)
// Trade - SpecialBox
void CDNUserSendManager::SendSpecialBoxList(int nRet, char cCount, TSpecialBoxInfo *InfoList)
{
	SCSpecialBoxList Packet;
	memset(&Packet, 0, sizeof(SCSpecialBoxList));

	Packet.nRet = nRet;
	Packet.cCount = cCount;
	if (Packet.cCount < 0) Packet.cCount = 0;
	if (Packet.cCount > SpecialBox::Common::ListMax) Packet.cCount = SpecialBox::Common::ListMax;

	if (InfoList)
		memcpy(Packet.BoxInfo, InfoList, sizeof(TSpecialBoxInfo) * Packet.cCount);

	int nLen = sizeof(SCSpecialBoxList) - sizeof(Packet.BoxInfo) + (sizeof(TSpecialBoxInfo) * Packet.cCount);
	m_pSession->AddSendData(SC_TRADE, eTrade::SC_SPECIALBOX_LIST, (char*)&Packet, nLen);
}

void CDNUserSendManager::SendSpecialBoxItemList(int nRet, char cCount, TSpecialBoxItemInfo *ItemList, INT64 biRewardCoin)
{
	SCSpecialBoxItemList Packet;
	memset(&Packet, 0, sizeof(SCSpecialBoxItemList));

	Packet.nRet = nRet;
	Packet.biRewardCoin = biRewardCoin;
	Packet.cCount = cCount;
	if (Packet.cCount < 0) Packet.cCount = 0;
	if (Packet.cCount > SpecialBox::Common::RewardSelectMax) Packet.cCount = SpecialBox::Common::RewardSelectMax;

	if (ItemList)
		memcpy(Packet.BoxItem, ItemList, sizeof(TSpecialBoxItemInfo) * Packet.cCount);

	int nLen = sizeof(SCSpecialBoxItemList) - sizeof(Packet.BoxItem) + (sizeof(TSpecialBoxItemInfo) * Packet.cCount);
	m_pSession->AddSendData(SC_TRADE, eTrade::SC_SPECIALBOX_ITEMLIST, (char*)&Packet, nLen);
}

void CDNUserSendManager::SendReceiveSpecialBoxItem(int nRet)
{
	SCReceiveSpecialBoxItem Packet = {0,};
	Packet.nRet = nRet;
	m_pSession->AddSendData(SC_TRADE, eTrade::SC_SPECIALBOX_RECEIVEITEM, (char*)&Packet, sizeof(SCReceiveSpecialBoxItem));
}

void CDNUserSendManager::SendNotifySpecialBoxCount(int nNotifyCount, bool bNew)
{
	SCNotifySpecialBox Packet = {0,};
	Packet.nNotifyCount = nNotifyCount;
	Packet.bNew = bNew;
	m_pSession->AddSendData(SC_TRADE, eTrade::SC_SPECIALBOX_NOTIFY, (char*)&Packet, sizeof(SCNotifySpecialBox));
}
#endif	// #if defined(PRE_SPECIALBOX)

//friend
void CDNUserSendManager::SendFriendGroupList(SCFriendGroupList * pPacket, int nSize)
{
	if(pPacket == NULL)
		return;

	m_pSession->AddSendData(SC_FRIEND, eFriend::SC_FRIEND_GROUP_LIST, (char*)pPacket, sizeof(*pPacket) - sizeof(pPacket->wszBuf) +(nSize * sizeof(WCHAR)));
}

void CDNUserSendManager::SendFriendList(SCFriendList * pPacket)
{
	if(pPacket == NULL)
		return;

	m_pSession->AddSendData(SC_FRIEND, eFriend::SC_FRIEND_LIST, (char*)pPacket, sizeof(*pPacket) - sizeof(pPacket->Info) +(pPacket->cCount * sizeof(TFriendInfo)));
}

void CDNUserSendManager::SendFriendLocationList(SCFriendLocationList * pPacket)
{
	if(pPacket == NULL)
		return;

	m_pSession->AddSendData(SC_FRIEND, eFriend::SC_FRIEND_LOCATIONLIST, (char*)pPacket, sizeof(*pPacket) - sizeof(pPacket->FriendLocation) +(pPacket->cCount * sizeof(TFriendLocation)));
}

void CDNUserSendManager::SendFriendGroupAdded(UINT nGroupDBID, const WCHAR * pName, int nRet)
{
	SCFriendGroupAdded packet;		//SC_FRIEND / SC_FRIEND_GROUP_ADDED
	memset(&packet, 0, sizeof(packet));
	
	packet.nRetCode = nRet;
	packet.nGroupDBID = nGroupDBID;
	if( pName )
	{
		packet.cNameLen = (BYTE)wcslen(pName);
		if( packet.cNameLen > FRIEND_GROUP_NAMELENMAX )
		{
			_DANGER_POINT();
			return;
		}

		_wcscpy(packet.wszBuf, _countof(packet.wszBuf), pName, (int)wcslen(pName));
	}

	int nSize = packet.cNameLen * sizeof(WCHAR);
	m_pSession->AddSendData(SC_FRIEND, eFriend::SC_FRIEND_GROUP_ADDED, (char*)&packet, sizeof(packet) - sizeof(packet.wszBuf) + nSize);
}

void CDNUserSendManager::SendFriendAdded(INT64 biFriendCharDBID, UINT nGroupDBID, const WCHAR * pName, const sWorldUserState * pState, int nRet)
{
	SCFriendAdded packet;
	memset(&packet, 0, sizeof(packet));

	packet.nRetCode = nRet;
	packet.Info.biFriendCharacterDBID = biFriendCharDBID;
	packet.Info.nGroupDBID = nGroupDBID;

	if(packet.nRetCode == ERROR_NONE)
	{
		if(pState)
		{
			packet.Info.Location.cServerLocation = pState->nLocationState;
			packet.Info.Location.nChannelID = pState->nLocationState == _LOCATION_VILLAGE ? pState->nChannelID : -1;
			packet.Info.Location.nMapIdx = pState->nMapIdx;
		}
		else
			packet.Info.Location.cServerLocation = _LOCATION_NONE;
	}
	if(pName)
		_wcscpy(packet.Info.wszFriendName, _countof(packet.Info.wszFriendName), pName, (int)wcslen(pName));
	
	m_pSession->AddSendData(SC_FRIEND, eFriend::SC_FRIEND_ADDED, (char*)&packet, sizeof(packet));
}

void CDNUserSendManager::SendFriendGroupDeleted(UINT nGroupDBID, int nRet)
{
	SCFriendGroupDeleted	 packet;	//SC_FRIEND / SC_FRIEND_GROUP_DELETED
	memset(&packet, 0, sizeof(packet));

	packet.nGroupDBID = nGroupDBID;
	packet.nRetCode = nRet;

	m_pSession->AddSendData(SC_FRIEND, eFriend::SC_FRIEND_GROUP_DELETED, (char*)&packet, sizeof(packet));
}

void CDNUserSendManager::SendFriendGroupUpdated(UINT nGroupDBID, const WCHAR * pName, int nRet)
{
	SCFriendGroupUpdated packet;	//SC_FRIEND / SC_FRIEND_GROUP_UPDATED
	memset(&packet, 0, sizeof(packet));

	packet.nRetCode = nRet;
	packet.nGroupDBID = nGroupDBID;
	if(pName)
	{
		packet.cNameLen = (BYTE)wcslen(pName);
		if( packet.cNameLen > FRIEND_GROUP_NAMELENMAX )
		{
			_DANGER_POINT();
			return;
		}

		_wcscpy(packet.wszBuf, _countof(packet.wszBuf), pName, (int)wcslen(pName));
	}

	int nSize = packet.cNameLen * sizeof(WCHAR);
	m_pSession->AddSendData(SC_FRIEND, eFriend::SC_FRIEND_GROUP_UPDATED, (char*)&packet, sizeof(packet) - sizeof(packet.wszBuf) + nSize);
}

void CDNUserSendManager::SendFriendDeleted(BYTE cCount, const INT64 * biFriendCharacterDBID, int nRet)
{
	SCFriendDeleted packet;
	memset(&packet, 0, sizeof(SCFriendDeleted));

	packet.nRetCode = nRet;
	if(cCount > 0 && biFriendCharacterDBID != NULL)
	{
		if(cCount >= FRIEND_MAXCOUNT)
		{
			_DANGER_POINT();
			return;
		}

		packet.cCount = cCount;
		memcpy(packet.biFriendCharacterDBID, biFriendCharacterDBID, sizeof(INT64) * packet.cCount);
	}

	m_pSession->AddSendData(SC_FRIEND, eFriend::SC_FRIEND_ADDED_DELETED, (char*)&packet, sizeof(packet) - sizeof(packet.biFriendCharacterDBID) +(sizeof(INT64) * packet.cCount));
}

void CDNUserSendManager::SendFriendUpdated(BYTE cCount, INT64 * biFriendDBID, UINT nGroupDBID, int nRet)
{
	if(biFriendDBID == NULL) return;

	SCFriendUpdated packet;			//SC_FRIEND / SC_FRIEND_UPDATED
	memset(&packet, 0, sizeof(packet));

	packet.nRetCode = nRet;
	packet.nGroupDBID = nGroupDBID;
	if(cCount > 0)
	{
		if(cCount >= FRIEND_MAXCOUNT)
		{
			_DANGER_POINT();
			return;
		}

		packet.cCount = cCount;
		memcpy(packet.biFriendCharacterDBID, biFriendDBID, sizeof(INT64) * packet.cCount);
	}

	m_pSession->AddSendData(SC_FRIEND, eFriend::SC_FRIEND_UPDATED, (char*)&packet, sizeof(packet) - sizeof(packet.biFriendCharacterDBID) +(sizeof(INT64) * packet.cCount));
}

void CDNUserSendManager::SendFriendDetailInfo(INT64 biFriendCharacterDBID, UINT nBelongGroupDBID, int nClass, int nLevel, int nJob, \
		const sWorldUserState * pState, const WCHAR * pGuildName, const WCHAR * pMemo)
{
	int nSize = 0;
	SCFriendDetailInfo packet;
	memset(&packet, 0, sizeof(packet));

	packet.biFriendCharacterDBID = biFriendCharacterDBID;
	packet.nGroupDBID = nBelongGroupDBID;
	packet.nClass = nClass;
	packet.nJob = nJob;
	packet.cCharacterLevel = (BYTE)nLevel;

	if(pState)
	{
		packet.Location.cServerLocation = pState->nLocationState;
		packet.Location.nChannelID = pState->nChannelID;
		packet.Location.nMapIdx = pState->nMapIdx;
	}
	
	m_pSession->AddSendData(SC_FRIEND, eFriend::SC_FRIEND_INFO, (char*)&packet, sizeof(packet));
}

void CDNUserSendManager::SendFriendResult(int nRet)
{
	SCFriendResult packet;
	packet.nRet = nRet;
	m_pSession->AddSendData(SC_FRIEND, eFriend::SC_FRIEND_RESULT, (char*)&packet, sizeof(packet));
}

void CDNUserSendManager::SendFriendAddNotice(const WCHAR * pName)
{
	SCFriendAddNotice packet;
	memset(&packet, 0, sizeof(packet));

	_wcscpy(packet.wszName, _countof(packet.wszName), pName, (int)wcslen(pName));

	m_pSession->AddSendData(SC_FRIEND, eFriend::SC_FRIEND_ADDNOTICE, (char*)&packet, sizeof(packet));
}

//Guild
void CDNUserSendManager::SendCreateGuild(UINT nSessionID, LPCWSTR lpwszGuildName, int iErrNo, const TGuildUID* pGuildUID, const TGuild* pInfo, UINT nAccountDBID, INT64 nCharacterDBID, TP_JOB nJob, CHAR cLevel, TCommunityLocation* pLocation)
{
 	SCCreateGuild Packet;
	memset(&Packet, 0, sizeof(Packet));

	Packet.nSessionID = nSessionID;
	_wcscpy(Packet.wszGuildName, _countof(Packet.wszGuildName), lpwszGuildName, (int)wcslen(lpwszGuildName));
	Packet.nAccountDBID = nAccountDBID;
	Packet.nCharacterDBID = nCharacterDBID;
	Packet.nJob = nJob;
	Packet.cLevel = cLevel;
	Packet.iErrNo = iErrNo;
	if(pGuildUID) {
		Packet.GuildUID = (*pGuildUID);
	}
	if(pInfo) {
		Packet.Info = (*pInfo);
	}
	if(pLocation) {
		Packet.Location = (*pLocation);
	}

	m_pSession->AddSendData(SC_GUILD, eGuild::SC_CREATEGUILD, reinterpret_cast<char*>(&Packet), sizeof(Packet));
}

void CDNUserSendManager::SendDismissGuild(UINT nSessionID, int iErrNo, const TGuildUID* pGuildUID)
{
 	SCDismissGuild Packet;
	memset(&Packet, 0, sizeof(Packet));

	Packet.nSessionID = nSessionID;
	Packet.iErrNo = iErrNo;
	if(pGuildUID) {
		Packet.GuildUID = (*pGuildUID);
	}

	m_pSession->AddSendData(SC_GUILD, eGuild::SC_DISMISSGUILD, reinterpret_cast<char*>(&Packet), sizeof(Packet));
}

void CDNUserSendManager::SendInviteGuildMemberReq(const TGuildUID pGuildUID, UINT nFromAccountDBID, UINT nFromSessionID, LPCWSTR lpwszFromCharacterName, int iErrNo, UINT nToAccountDBID, UINT nToSessionID, LPCWSTR lpwszGuildName)
{
	SCInviteGuildMemberReq Packet;
	memset(&Packet, 0, sizeof(Packet));

	Packet.GuildUID = pGuildUID;
	Packet.nFromAccountDBID = nFromAccountDBID;
	Packet.nFromSessionID = nFromSessionID;
	Packet.nToAccountDBID = nToAccountDBID;
	Packet.nToSessionID = nToSessionID;
	Packet.iErrNo = iErrNo;
	_wcscpy(Packet.wszFromCharacterName, _countof(Packet.wszFromCharacterName), lpwszFromCharacterName, (int)wcslen(lpwszFromCharacterName));
	if(lpwszGuildName) 
		_wcscpy(Packet.wszGuildName, _countof(Packet.wszGuildName), lpwszGuildName, (int)wcslen(lpwszGuildName));

	m_pSession->AddSendData(SC_GUILD, eGuild::SC_INVITEGUILDMEMBREQ, reinterpret_cast<char*>(&Packet), sizeof(Packet));
}

void CDNUserSendManager::SendInviteGuildMemberAck(UINT nToAccountDBID, INT64 nToCharacterDBID, LPCWSTR lpwszToCharacterName, int iErrNo, UINT nFromAccountDBID, UINT nFromSessionID, TP_JOB nJob, CHAR cLevel, TCommunityLocation* pLocation, const TGuildUID* pGuildUID)
{
	SCInviteGuildMemberAck Packet;
	memset(&Packet, 0, sizeof(Packet));

	Packet.nToAccountDBID = nToAccountDBID;
	Packet.nToCharacterDBID = nToCharacterDBID;
	Packet.nFromAccountDBID = nFromAccountDBID;
	Packet.nFromCharacterDBID = nFromSessionID;
	Packet.nJob = nJob;
	Packet.cLevel = cLevel;
	Packet.iErrNo = iErrNo;
	_wcscpy(Packet.wszToCharacterName, _countof(Packet.wszToCharacterName), lpwszToCharacterName, (int)wcslen(lpwszToCharacterName));
	if(pLocation) {
		Packet.Location = (*pLocation);
	}
	if(pGuildUID) {
		Packet.GuildUID = (*pGuildUID);
	}

	m_pSession->AddSendData(SC_GUILD, eGuild::SC_INVITEGUILDMEMBACK, reinterpret_cast<char*>(&Packet), sizeof(Packet));
}

void CDNUserSendManager::SendLeaveGuildMember(UINT nAccountDBID, INT64 nCharacterDBID, int iErrNo, const TGuildUID* pGuildUID, bool bGraduateBeginnerGuild)
{
	SCLeaveGuildMember Packet;
	memset(&Packet, 0, sizeof(Packet));

	Packet.nAccountDBID = nAccountDBID;
	Packet.nCharacterDBID = nCharacterDBID;
	Packet.iErrNo = iErrNo;
	if(pGuildUID) {
		Packet.GuildUID = (*pGuildUID);
	}
#ifdef PRE_ADD_BEGINNERGUILD
	Packet.bGraduateBeginnerGuild = bGraduateBeginnerGuild;
#endif		//#ifdef PRE_ADD_BEGINNERGUILD

	m_pSession->AddSendData(SC_GUILD, eGuild::SC_LEAVEGUILDMEMB, reinterpret_cast<char*>(&Packet), sizeof(Packet));
}

void CDNUserSendManager::SendExileGuildMember(UINT nAccountDBID, INT64 nCharacterDBID, int iErrNo, const TGuildUID* pGuildUID)
{
	SCExileGuildMember Packet;
	memset(&Packet, 0, sizeof(Packet));

	Packet.nAccountDBID = nAccountDBID;
	Packet.nCharacterDBID = nCharacterDBID;
	Packet.iErrNo = iErrNo;
	if(pGuildUID) {
		Packet.GuildUID = (*pGuildUID);
	}

	m_pSession->AddSendData(SC_GUILD, eGuild::SC_EXILEGUILDMEMB, reinterpret_cast<char*>(&Packet), sizeof(Packet));
}

void CDNUserSendManager::SendChangeGuildInfo(UINT nAccountDBID, INT64 nCharacterDBID, BYTE btGuildUpdate, int iErrNo, int iInt1, int iInt2, INT64 biInt64, LPCWSTR pTextL, const TGuildUID *pGuildUID)
{
	SCChangeGuildInfo Packet;
	memset(&Packet, 0, sizeof(Packet));

	Packet.nAccountDBID = nAccountDBID;
	Packet.nCharacterDBID = nCharacterDBID;
	Packet.btGuildUpdate = btGuildUpdate;
	Packet.Int1 = iInt1;
	Packet.Int2 = iInt2;
	Packet.Int64 = biInt64;
	Packet.iErrNo = iErrNo;
	if(pTextL) {
		_wcscpy(Packet.Text, _countof(Packet.Text), pTextL, (int)wcslen(pTextL));
	}
	if(pGuildUID) {
		Packet.GuildUID = (*pGuildUID);
	}

	m_pSession->AddSendData(SC_GUILD, eGuild::SC_CHANGEGUILDINFO, reinterpret_cast<char*>(&Packet), sizeof(Packet));
}

void CDNUserSendManager::SendChangeGuildInfoEx(UINT nAccountDBID, INT64 nCharacterDBID, BYTE btGuildUpdate, int iErrNo, int iInt1, int iInt2, int iInt3, int iInt4, INT64 biInt64, LPCWSTR pTextL, const TGuildUID *pGuildUID)
{
	SCChangeGuildInfo Packet;
	memset(&Packet, 0, sizeof(Packet));

	Packet.nAccountDBID = nAccountDBID;
	Packet.nCharacterDBID = nCharacterDBID;
	Packet.btGuildUpdate = btGuildUpdate;
	Packet.Int1 = iInt1;
	Packet.Int2 = iInt2;
	Packet.Int64 = biInt64;
	Packet.iErrNo = iErrNo;
	if(pTextL) {
		_wcscpy(Packet.Text, _countof(Packet.Text), pTextL, (int)wcslen(pTextL));
	}
	if(pGuildUID) {
		Packet.GuildUID = (*pGuildUID);
	}
	Packet.Int3 = iInt3;
	Packet.Int4 = iInt4;

	m_pSession->AddSendData(SC_GUILD, eGuild::SC_CHANGEGUILDINFO, reinterpret_cast<char*>(&Packet), sizeof(Packet));
}

void CDNUserSendManager::SendMoveGuildItem(char cType, INT64 biSrcSerial, INT64 biDestSerial, const TItemInfo *pSrcInfo, const TItemInfo *pDestInfo, int nRetCode, int nTakeItemCount)
{
	SCMoveGuildItem Move;
	memset(&Move, 0, sizeof(SCMoveGuildItem));

	Move.cMoveType = cType;
	Move.nRetCode = nRetCode;
	Move.biSrcSerial = biSrcSerial;
	Move.biDestSerial = biDestSerial;
	Move.nTakeItemCount = nTakeItemCount;

	if(nRetCode == ERROR_NONE)
	{
		if(pSrcInfo){
			Move.SrcItem = *pSrcInfo;
			if(pSrcInfo->Item.wCount <= 0)
				Move.SrcItem.Item.nItemID = 0;
		}

		if(pDestInfo){
			Move.DestItem = *pDestInfo;
			if(pDestInfo->Item.wCount <= 0)
				Move.DestItem.Item.nItemID = 0;
		}

		m_pSession->AddSendData(SC_ITEM, eItem::SC_MOVEGUILDITEM, (char*)&Move, sizeof(SCMoveGuildItem));
	}
	else
		m_pSession->AddSendData(SC_ITEM, eItem::SC_MOVEGUILDITEM, (char*)&Move, (int)(sizeof(Move) -(sizeof(Move.SrcItem) + sizeof(Move.DestItem))));	
}

void CDNUserSendManager::SendRefreshGuildItem(char cType, INT64 biSrcSerial, INT64 biDestSerial, const TItemInfo *pSrcInfo, const TItemInfo *pDestInfo)
{
	SCRefreshGuildItem Refresh;
	memset(&Refresh, 0, sizeof(Refresh));

	Refresh.cMoveType = cType;
	Refresh.biSrcSerial = biSrcSerial;
	Refresh.biDestSerial = biDestSerial;

	if(pSrcInfo){
		Refresh.SrcItem = *pSrcInfo;
		if(pSrcInfo->Item.wCount <= 0)
			Refresh.SrcItem.Item.nItemID = 0;
	}

	if(pDestInfo){
		Refresh.DestItem = *pDestInfo;
		if(pDestInfo->Item.wCount <= 0)
			Refresh.DestItem.Item.nItemID = 0;
	}

	m_pSession->AddSendData(SC_ITEM, eItem::SC_REFRESH_GUILDITEM, (char*)&Refresh, sizeof(SCRefreshGuildItem));
}

void CDNUserSendManager::SendRefreshGuildCoin(INT64 biTotalCoin)
{
	SCRefreshGuildCoin Refresh;
	Refresh.biTotalCoin = biTotalCoin;

	m_pSession->AddSendData(SC_ITEM, eItem::SC_REFRESH_GUILDCOIN, (char*)&Refresh, sizeof(SCRefreshGuildCoin));
}

void CDNUserSendManager::SendMoveGuildCoin(char cType, INT64 nAddCoin, INT64 nInventoryCoin, INT64 nWarehouseCoin, INT64 nWithdrawCoin, int nRet)
{
	SCMoveGuildCoin Coin;

	Coin.cMoveType = cType;
	Coin.nMoveCoin = nAddCoin;
	Coin.nInventoryCoin = nInventoryCoin;
	Coin.nWarehouseCoin = nWarehouseCoin;
	Coin.nWithdrawCoin = nWithdrawCoin;
	Coin.nRet = nRet;

	m_pSession->AddSendData(SC_ITEM, eItem::SC_MOVEGUILDCOIN, (char*)&Coin, sizeof(SCMoveGuildCoin));
}

void CDNUserSendManager::SendGetGuildWareHistoryList(const TGuildUID pGuildUID, bool bDirection, int nCurCount, int nTotCount, int iErrNo, const TGuildWareHistory* pHistoryList, int nCount)
{
	SCGetGuildWareHistory Packet;
	memset(&Packet, NULL, sizeof(Packet));
	
	int nSize = 0;

	Packet.nErrNo = iErrNo;
	Packet.nCurrCount = nCurCount;
	Packet.nTotalCount = nTotCount;
	nSize = sizeof(SCGetGuildWareHistory) - sizeof(Packet.HistoryList);

	if(nCount > 0)
	{	
		Packet.nCount = nCount;	
		memcpy(Packet.HistoryList, pHistoryList, sizeof(TGuildWareHistory)*nCount);
		nSize += sizeof(TGuildWareHistory)*nCount;
	}
	
	m_pSession->AddSendData(SC_GUILD, eGuild::SC_GET_GUILDWARE_HISTORY, reinterpret_cast<char*>(&Packet), nSize);
}

void CDNUserSendManager::SendGetGuildInfo(CDNGuildBase *pGuild, int nRet)
{
	SCGetGuildInfo Packet;
	memset(&Packet, 0, sizeof(Packet));

	Packet.iErrNo = nRet;
	if(pGuild){
		if(pGuild->GetInfo()) Packet.Info = (*(pGuild->GetInfo()));
	}

	// Return true if Guild War is applied for and guild join cannot be approved
#if defined( _VILLAGESERVER )
	if( pGuild->IsEnrollGuildWar() && g_pGuildWarManager->GetRestriction(eGuild::CS_INVITEGUILDMEMBREQ, pGuild->GetUID()) )
		Packet.bNotRecruitMember = true;
#endif // #if defined( _VILLAGESERVER )

#if defined (_VILLAGESERVER)
	if (pGuild->IsEnrollGuildWar() && g_pGuildWarManager->GetStepIndex() == GUILDWAR_STEP_REWARD)
		Packet.Info.iWarPoint = g_pGuildWarManager->GetGuildWarPoint(m_pSession->GetGuildUID().nDBID);
	else
		Packet.Info.iWarPoint = 0;
#endif // #if defined (_VILLAGESERVER)

	// Send to the user
	m_pSession->AddSendData(SC_GUILD, eGuild::SC_GETGUILDINFO, reinterpret_cast<char*>(&Packet), sizeof(Packet));
}

void CDNUserSendManager::SendGetGuildMember(CDNGuildBase *pGuild, int nRet)
{
	TGuildMember MemberList[GUILDSIZE_MAX];
	memset(&MemberList, 0, sizeof(MemberList));
	SCGetGuildMember Packet;
	memset(&Packet, 0, sizeof(Packet));

	Packet.iErrNo = nRet;
	if(pGuild){
		pGuild->SetMemberList(MemberList);
	}

	int nPacketSize = (sizeof(SCGetGuildMember) - sizeof(Packet.MemberList));
	int nMaxSendMember = ((SERVERDETACHPACKETSIZE)-nPacketSize) / sizeof(Packet.MemberList[0]);
	if(nMaxSendMember > SENDGUILDMEMBER_MAX)
		nMaxSendMember = SENDGUILDMEMBER_MAX;
	
	for(int i=0;i<GUILDSIZE_MAX;i++)
	{
		if(MemberList[i].nCharacterDBID == 0)
			break;
		Packet.MemberList[Packet.nCount++] = MemberList[i];
		if(Packet.nCount >= nMaxSendMember)
		{
			if( i+1 >= GUILDSIZE_MAX || (i+1 < GUILDSIZE_MAX && MemberList[i+1].nCharacterDBID == 0) )
				Packet.bEndMember = true;
			int iSize = static_cast<int>(sizeof(Packet) - sizeof(Packet.MemberList) + sizeof(Packet.MemberList[0]) * Packet.nCount);
			m_pSession->AddSendData(SC_GUILD, eGuild::SC_GET_GUILDMEMBER, reinterpret_cast<char*>(&Packet), iSize);
			Packet.nCount = 0;
			Packet.nPage++;
			memset(Packet.MemberList, 0, sizeof(Packet.MemberList));
		}
	}
	// Send to the user
	if(Packet.nCount > 0)
	{
		Packet.bEndMember = true;
		int iSize = static_cast<int>(sizeof(Packet) - sizeof(Packet.MemberList) + sizeof(Packet.MemberList[0]) * Packet.nCount);
		m_pSession->AddSendData(SC_GUILD, eGuild::SC_GET_GUILDMEMBER, reinterpret_cast<char*>(&Packet), iSize);

	}	
}

void CDNUserSendManager::SendEnrollGuildWar(int iErrNo, short wScheduleID, BYTE cTeamColorCode)
{
	SCEnrollGuildWar Packet;
	Packet.iErrorNo = iErrNo;
	Packet.wScheduleID = wScheduleID;
	Packet.cTeamColorCode = cTeamColorCode;

	m_pSession->AddSendData(SC_GUILD, eGuild::SC_ENROLL_GUILDWAR, reinterpret_cast<char*>(&Packet), sizeof(Packet));
}

void CDNUserSendManager::SendGuildWarEvent(short wScheduleID, char cEventStep, char cEventType)
{
	SCGuildWarEvent TxPacket;
	TxPacket.wScheduleID = wScheduleID;
	TxPacket.cEventStep = cEventStep;
	TxPacket.cEventType = cEventType;

	m_pSession->AddSendData(SC_GUILD, eGuild::SC_CHANGE_WAREVENT, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket));
}

void CDNUserSendManager::SendGuildWarFestivalPoint()
{
	SCGuildWarFestivalPoint GuildWarFestivalPoint;
	memset( &GuildWarFestivalPoint, 0, sizeof(GuildWarFestivalPoint) );

	GuildWarFestivalPoint.biGuildWarFestivalPoint = m_pSession->GetGuildWarFestivalPoint();
	m_pSession->AddSendData( SC_GUILD, eGuild::SC_GUILDWAR_FESTIVAL_POINT, reinterpret_cast<char*>(&GuildWarFestivalPoint), sizeof(GuildWarFestivalPoint) );
}

void CDNUserSendManager::SendGuildWarPreWinGuild(bool bPreWin)
{
	SCGuildWarPreWin GuildWarPreWin;
	memset( &GuildWarPreWin, 0, sizeof(GuildWarPreWin) );

	GuildWarPreWin.bPreWin = bPreWin;
	m_pSession->AddSendData( SC_GUILD, eGuild::SC_GUILDWAR_PRE_WIN, reinterpret_cast<char*>(&GuildWarPreWin), sizeof(GuildWarPreWin) );
}

void CDNUserSendManager::SendGuildWarTournamentWin(SCGuildWarTournamentWin* pPacket)
{	
	m_pSession->AddSendData( SC_GUILD, eGuild::SC_GUILDWAR_TOURNAMENT_WIN, reinterpret_cast<char*>(pPacket), sizeof(SCGuildWarTournamentWin) );
}

#if defined(_VILLAGESERVER)
void CDNUserSendManager::SendGuildWarEventTime(TGuildWarEventInfo* pGuildWarTime, TGuildWarEventInfo* pFinalPartTime, bool bFinalProgress)
{
	SCGuildWarEventTime GuildWarEventTime;
	memset( &GuildWarEventTime, 0, sizeof(GuildWarEventTime));

	memcpy(GuildWarEventTime.tFinalPartTime, pFinalPartTime, sizeof(GuildWarEventTime.tFinalPartTime));
	memcpy(GuildWarEventTime.tGuildWarTime, pGuildWarTime, sizeof(GuildWarEventTime.tGuildWarTime));
	GuildWarEventTime.bFinalProgress = bFinalProgress;
	m_pSession->AddSendData( SC_GUILD, eGuild::SC_GUILDWAR_EVENT_TIME, reinterpret_cast<char*>(&GuildWarEventTime), sizeof(GuildWarEventTime) );
}

void CDNUserSendManager::SendGuildWarStatusOpen(int nBluePoint, int nRedPoint, int nMyPoint, int nMyGuildPoint)
{
	SCGuildWarStatus GuildWarStatus;
	memset(&GuildWarStatus, 0, sizeof(SCGuildWarStatus));

	GuildWarStatus.nBluePoint = nBluePoint;
	GuildWarStatus.nRedPoint = nRedPoint;
	GuildWarStatus.nMyPoint = nMyPoint;
	GuildWarStatus.nMyGuildPoint = nMyGuildPoint;

	if( g_pGuildWarManager->GetFinalProgress() )
	{
		GuildWarStatus.nRankingCount = g_pGuildWarManager->GetGuildWarTrialRankingCount();
		memcpy(GuildWarStatus.sGuildWarPointTrialRanking, g_pGuildWarManager->GetGuildWarTrialRankingInfo(), sizeof(SGuildWarRankingInfo)*GuildWarStatus.nRankingCount);
	}
	m_pSession->AddSendData(SC_GUILD, eGuild::SC_GUILDWAR_STATUS_OPEN, reinterpret_cast<char*>(&GuildWarStatus)
		, sizeof(GuildWarStatus)-(sizeof(SGuildWarRankingInfo)*(GUILDWAR_TRIAL_POINT_TEAM_MAX-GuildWarStatus.nRankingCount)) );
}

void CDNUserSendManager::SendGuildWarTrialResultOpen()
{
	SCGuildWarTrialResult GuildWarTrialResult;
	memset(&GuildWarTrialResult, 0, sizeof(SCGuildWarTrialResult));
	GuildWarTrialResult.nBluePoint = g_pGuildWarManager->GetBlueTeamPoint();
	GuildWarTrialResult.nRedPoint = g_pGuildWarManager->GetRedTeamPoint();	
	memcpy(GuildWarTrialResult.sGuildWarRankingInfo, g_pGuildWarManager->GetGuildWarRankingInfo(), sizeof(GuildWarTrialResult.sGuildWarRankingInfo));
	memcpy(GuildWarTrialResult.sGuildWarMissionGuildRankingInfo, g_pGuildWarManager->GetGuildWarMissionGuildRankingInfo(), sizeof(GuildWarTrialResult.sGuildWarMissionGuildRankingInfo));
	memcpy(GuildWarTrialResult.sGuildWarMissionRankingInfo, g_pGuildWarManager->GetGuildWarMissionRankingInfo(), sizeof(GuildWarTrialResult.sGuildWarMissionRankingInfo));
	memcpy(GuildWarTrialResult.sGuildWarDailyAward, g_pGuildWarManager->GetGuildDailyAward(), sizeof(GuildWarTrialResult.sGuildWarDailyAward));
	
	int nPacketSize = sizeof(SCGuildWarTrialResult);
	GuildWarTrialResult.bGuildWar = false;
	GuildWarTrialResult.bReward = false;

	CDNGuildBase* pGuild = g_pGuildManager->At(m_pSession->GetGuildUID());
	if(pGuild)
	{
#if !defined( PRE_ADD_NODELETEGUILD )
		CDetachAutoEx<CDNGuildBase>	AutoDetach(pGuild);
		if(true == pGuild->IsEnable() )
		{
#endif
			if( pGuild->IsEnrollGuildWar() )
			{
				CDNGuildVillage* pGuildVillage = static_cast<CDNGuildVillage *>(pGuild);
				GuildWarTrialResult.bGuildWar = true;
				// Set guild info and own info			
				GuildWarTrialResult.nFestivalPoint = m_pSession->GetGuildWarPoint();
				GuildWarTrialResult.nFestivalPointAdd = g_pGuildWarManager->CalcGuildWarFestivalPoint(m_pSession->GetGuildSelfView().cTeamColorCode,
					m_pSession->GetCharacterDBID(), GuildWarTrialResult.nFestivalPoint) - GuildWarTrialResult.nFestivalPoint;

				memcpy(&GuildWarTrialResult.sMyGuildWarRankingInfo, pGuildVillage->GetGuildWarMyRankingInfo(), sizeof(GuildWarTrialResult.sMyGuildWarRankingInfo));
				memcpy(&GuildWarTrialResult.sMyGuildWarMissionGuildRankingInfo, pGuildVillage->GetGuildWarTotalRankingInfo(), sizeof(GuildWarTrialResult.sMyGuildWarMissionGuildRankingInfo));
				memcpy(&GuildWarTrialResult.sMyGuildWarMissionRaningInfo, m_pSession->GetGuildWarMissionRankingInfo(), sizeof(GuildWarTrialResult.sMyGuildWarMissionRaningInfo));
				if( m_pSession->GetGuildWarPoint() > 0 )  // Only if participated in preliminaries..
					GuildWarTrialResult.bReward = m_pSession->IsGuildWarReward();
				m_pSession->AddSendData(SC_GUILD, eGuild::SC_GUILDWAR_TRIAL_RESULT_OPEN, reinterpret_cast<char*>(&GuildWarTrialResult), nPacketSize);
				return;
			}
#if !defined( PRE_ADD_NODELETEGUILD )
		}		
#endif
	}	
	nPacketSize = sizeof(SCGuildWarTrialResult) - sizeof(int) - sizeof(int) -
		(sizeof(GuildWarTrialResult.sMyGuildWarRankingInfo)+sizeof(GuildWarTrialResult.sMyGuildWarMissionGuildRankingInfo)+sizeof(GuildWarTrialResult.sMyGuildWarMissionRaningInfo));			
	m_pSession->AddSendData(SC_GUILD, eGuild::SC_GUILDWAR_TRIAL_RESULT_OPEN, reinterpret_cast<char*>(&GuildWarTrialResult), nPacketSize);
}

void CDNUserSendManager::SendGuildWarTrialResultUIOpen()
{
	m_pSession->AddSendData(SC_GUILD, eGuild::SC_GUILDWAR_TRIAL_RESULT_UIOPEN, NULL, 0);
}

void CDNUserSendManager::SendGuildWarTournamentInfoUIOpen()
{
	m_pSession->AddSendData(SC_GUILD, eGuild::SC_GUILDWAR_TOURNAMENT_INFO_UIOPEN, NULL, 0);
}

void CDNUserSendManager::SendGuildWarTournamentInfoOpen(bool bPopularityVote)
{
	SCGuildTournamentInfo GuildTournamentInfo;
	memset(&GuildTournamentInfo, 0, sizeof(SCGuildTournamentInfo));

	memcpy(GuildTournamentInfo.TournamentInfo, g_pGuildWarManager->GetGuildTournamentInfo(), sizeof(GuildTournamentInfo.TournamentInfo));	
	GuildTournamentInfo.bPopularityVote = bPopularityVote;
	GuildTournamentInfo.cMatchTypeCode = g_pGuildWarManager->GetGuildWarFinalPart();
	GuildTournamentInfo.tStartTime = g_pGuildWarManager->GetGuildWarFinalPartBeginTime();
	m_pSession->AddSendData(SC_GUILD, eGuild::SC_GUILDWAR_TOURNAMENT_INFO_OPEN, reinterpret_cast<char*>(&GuildTournamentInfo), sizeof(SCGuildTournamentInfo));
}

void CDNUserSendManager::SendGuildWarVote(int nRetCode, int nAddFestivalPoint)
{
	SCGuildWarVote GuildWarVote;
	memset(&GuildWarVote, 0, sizeof(SCGuildWarVote));

	GuildWarVote.nRetCode = nRetCode;
	GuildWarVote.nAddFestivalPoint = nAddFestivalPoint;
	m_pSession->AddSendData(SC_GUILD, eGuild::SC_GUILDWAR_VOTE,reinterpret_cast<char*>(&GuildWarVote), sizeof(SCGuildWarVote));
}

void CDNUserSendManager::SendGuildWarVoteTop(int nVoteCount, const WCHAR* pwszVoteGuildName)
{
	SCGuildWarVoteTop GuildWarVoteTop;
	memset(&GuildWarVoteTop, 0, sizeof(SCGuildWarVoteTop));

	GuildWarVoteTop.nVoteCount = nVoteCount;
	memcpy(GuildWarVoteTop.wszGuildName, pwszVoteGuildName, sizeof(GuildWarVoteTop.wszGuildName));
	m_pSession->AddSendData(SC_GUILD, eGuild::SC_GUILDWAR_VOTE_TOP,reinterpret_cast<char*>(&GuildWarVoteTop), sizeof(SCGuildWarVoteTop));
}

void CDNUserSendManager::SendGuildWarReward(int nRetCode)
{
	SCGuildWarCompensation GuildWarCompensation;
	memset(&GuildWarCompensation, 0, sizeof(SCGuildWarCompensation));

	GuildWarCompensation.nRetCode = nRetCode;	
	m_pSession->AddSendData(SC_GUILD, eGuild::SC_GUILDWAR_COMPENSATION,reinterpret_cast<char*>(&GuildWarCompensation), sizeof(SCGuildWarCompensation));
}

void CDNUserSendManager::SendGuildWarWinSkill(int nRetCode, DWORD dwCoolTime)
{
	SCGuildWarWinSkill GuildWarWinSkill;
	memset(&GuildWarWinSkill, 0, sizeof(SCGuildWarWinSkill));

	GuildWarWinSkill.nRetCode = nRetCode;
	GuildWarWinSkill.dwCoolTime = dwCoolTime;
	m_pSession->AddSendData(SC_GUILD, eGuild::SC_GUILDWAR_WIN_SKILL,reinterpret_cast<char*>(&GuildWarWinSkill), sizeof(SCGuildWarWinSkill));
}

void CDNUserSendManager::SendGuildWarUserWinSkill(TGuildUID GuildUID, EtVector3 vPos, int nSkillID)
{
	SCGuildWarUserWinSkill GuildWarUserWinSkill;
	memset(&GuildWarUserWinSkill, 0, sizeof(SCGuildWarUserWinSkill));

	GuildWarUserWinSkill.GuildUID = GuildUID;
	GuildWarUserWinSkill.vPos = vPos;
	GuildWarUserWinSkill.nSkillID = nSkillID;
	m_pSession->AddSendData(SC_GUILD, eGuild::SC_GUILDWAR_USER_WINSKILL,reinterpret_cast<char*>(&GuildWarUserWinSkill), sizeof(SCGuildWarUserWinSkill));
}

void CDNUserSendManager::SendGuildWarBuyedItem(int itemID, int count)
{
	SCBuyedItemCount packet;
	memset(&packet, 0, sizeof(packet));

	packet.count = 1;
	packet.items[0].id = itemID;
	packet.items[0].count = count;

	int length = sizeof(packet) - sizeof(packet.items) + (sizeof(ItemCount) * packet.count);
	m_pSession->AddSendData(SC_GUILD, eGuild::SC_GUILDWAR_BUYED_ITEM_COUNT, reinterpret_cast<char*>(&packet), length);
}

void CDNUserSendManager::SendGuildWarBuyedItemList(const std::map<int, int>& items)
{
	if (items.empty())
		return;

	SCBuyedItemCount packet;
	memset(&packet, 0, sizeof(packet));

	packet.count = static_cast<short>(items.size());

	size_t i = 0;
	for (std::map<int, int>::value_type v : items)
	{
		packet.items[i].id = v.first;
		packet.items[i].count = v.second;
		++i;
	}

	int length = sizeof(packet) - sizeof(packet.items) + (sizeof(ItemCount) * packet.count);
	m_pSession->AddSendData(SC_GUILD, eGuild::SC_GUILDWAR_BUYED_ITEM_COUNT_LIST, reinterpret_cast<char*>(&packet), length);
}
#endif // #if defined(_VILLAGESERVER)

void CDNUserSendManager::SendChangeGuildName(const wchar_t* pwszGuildName)
{
	SCChangeGuildName Packet;
	memset(&Packet, 0, sizeof(SCChangeGuildName));	
	_wcscpy(Packet.wszGuildName, _countof(Packet.wszGuildName), pwszGuildName, (int)wcslen(pwszGuildName));

	m_pSession->AddSendData(SC_GUILD, eGuild::SC_CHANGEGUILDNAME, reinterpret_cast<char*>(&Packet), sizeof(SCChangeGuildName));
}

void CDNUserSendManager::SendChangeGuildMark(const MAGuildChangeMark *pPacket)
{
	SCChangeGuildMark Packet;
	memset(&Packet, 0, sizeof(SCChangeGuildMark));	
	Packet.wMark = pPacket->wMark;
	Packet.wMarkBG = pPacket->wMarkBG;
	Packet.wMarkBorder = pPacket->wMarkBorder;

	m_pSession->AddSendData(SC_GUILD, eGuild::SC_CHANGEGUILDMARK, reinterpret_cast<char*>(&Packet), sizeof(SCChangeGuildMark));
}

void CDNUserSendManager::SendGuildLevelUp(int nLevel)
{
	SCGuildLevelUp Packet;
	memset (&Packet, 0, sizeof(SCGuildLevelUp));
	Packet.nLevel = nLevel;

	m_pSession->AddSendData(SC_GUILD, eGuild::SC_GUILDLEVELUP, reinterpret_cast<char*>(&Packet), sizeof(SCGuildLevelUp));
}

void CDNUserSendManager::SendUpdateGuildExp(int nError, BYTE cPointType, int nPointValue, int nGuildExp, INT64 biCharacterDBID, int nMissionID)
{
	SCUpdateGuildExp Packet;
	memset (&Packet, 0, sizeof(SCUpdateGuildExp));
	Packet.nError = nError;
	Packet.cPointType = cPointType;
	Packet.nPointValue = nPointValue;
	Packet.nGuildExp = nGuildExp;
	Packet.biCharacterDBID = biCharacterDBID;
	Packet.nMissionID = nMissionID;

	m_pSession->AddSendData(SC_GUILD, eGuild::SC_UPDATEGUILDEXP, reinterpret_cast<char*>(&Packet), sizeof(SCUpdateGuildExp));
}

void CDNUserSendManager::SendExtendGuildWare(short wTotalSize)
{
	SCExtendGuildWare Packet;
	Packet.wTotalSize = wTotalSize;
	
	m_pSession->AddSendData(SC_GUILD, eGuild::SC_EXTEND_GUILDWARESIZE, reinterpret_cast<char*>(&Packet), sizeof(SCExtendGuildWare));
}

void CDNUserSendManager::SendChangeGuildMemberInfo(UINT nReqAccountDBID, INT64 nReqCharacterDBID, UINT nChgAccountDBID, INT64 nChgCharacterDBID, BYTE btGuildMemberUpdate, int iErrNo, int iInt1, int iInt2, INT64 biInt64, LPCWSTR pText, const TGuildUID *pGuildUID)
{
	SCChangeGuildMemberInfo Packet;
	memset(&Packet, 0, sizeof(Packet));

	Packet.nReqAccountDBID = nReqAccountDBID;
	Packet.nReqCharacterDBID = nReqCharacterDBID;
	Packet.nChgAccountDBID = nChgAccountDBID;
	Packet.nChgCharacterDBID = nChgCharacterDBID;
	Packet.btGuildMemberUpdate = btGuildMemberUpdate;
	Packet.Int1 = iInt1;
	Packet.Int2 = iInt2;
	Packet.Int64 = biInt64;
	Packet.iErrNo = iErrNo;
	if(pText) {
		_wcscpy(Packet.Text, _countof(Packet.Text), pText, (int)wcslen(pText));
	}
	if(pGuildUID) {
		Packet.GuildUID = (*pGuildUID);
	}

	m_pSession->AddSendData(SC_GUILD, eGuild::SC_CHANGEGUILDMEMBINFO, reinterpret_cast<char*>(&Packet), sizeof(Packet));
}

void CDNUserSendManager::SendGetGuildHistoryList(const TGuildUID pGuildUID, INT64 biIndex, bool bDirection, int nCurCount, int nTotCount, int iErrNo, const TGuildHistory* pHistoryList, int nCount)
{
	SCGetGuildHistoryList Packet;
	memset(&Packet, 0, sizeof(Packet));

	Packet.GuildUID = pGuildUID;
	Packet.biIndex = biIndex;
	Packet.bDirection = bDirection;
	Packet.nCurCount = nCurCount;
	Packet.nTotCount = nTotCount;
	Packet.iErrNo = iErrNo;

	if(pHistoryList && CHECK_RANGE(nCount, 1, GUILDSIZE_MAX)) {
		for (int iIndex = 0 ; nCount > iIndex ; ++iIndex) {
			Packet.HistoryList[Packet.nCount] = pHistoryList[iIndex];
			++Packet.nCount;
		}
	}

	int iSize = static_cast<int>(sizeof(Packet) - sizeof(Packet.HistoryList) + sizeof(Packet.HistoryList[0]) * Packet.nCount);
	m_pSession->AddSendData(SC_GUILD, eGuild::SC_GETGUILDHISTORYLIST, reinterpret_cast<char*>(&Packet), iSize);
}

void CDNUserSendManager::SendGuildSelfView(UINT nSessionID, const TGuildSelfView& pGuildSelfView)
{
	SCChangeGuildSelfView Packet;
	memset(&Packet, 0, sizeof(Packet));

	Packet.nSessionID = nSessionID;
	Packet.GuildSelfView = pGuildSelfView;

	m_pSession->AddSendData(SC_GUILD, eGuild::SC_CHANGEGUILDSELFVIEW, reinterpret_cast<char*>(&Packet), sizeof(Packet));
}

void CDNUserSendManager::SendGuildChat(const TGuildUID pGuildUID, UINT nAccountDBID, INT64 nCharacterDBID, LPCWSTR lpwszChatMsg, short nLen)
{
	SCGuildChat Packet;
	memset(&Packet, 0, sizeof(Packet));

	Packet.GuildUID = pGuildUID;
	Packet.nAccountDBID = nAccountDBID;
	Packet.nCharacterDBID = nCharacterDBID;
	for(short aIndex = 0 ; _countof(Packet.wszChatMsg) > aIndex ; ++aIndex) {
		Packet.wszChatMsg[aIndex] = lpwszChatMsg[aIndex];
	}
	Packet.nLen = nLen;

	int iSize = static_cast<int>(sizeof(Packet) - sizeof(Packet.wszChatMsg) + sizeof(Packet.wszChatMsg[0]) * Packet.nLen);
	m_pSession->AddSendData(SC_CHAT, eChat::SC_GUILDCHAT, reinterpret_cast<char*>(&Packet), iSize);
}

#ifdef PRE_ADD_DOORS_GUILDCHAT_DISCONNECT
void CDNUserSendManager::SendDoorsGuildChat(const TGuildUID pGuildUID, INT64 biCharacterDBID, LPCWSTR lpwszChatMsg, short nLen)
{
	SCDoorsGuildChat packet;
	memset(&packet, 0, sizeof(SCDoorsGuildChat));

	packet.GuildUID = pGuildUID;
	packet.biCharacterDBID = biCharacterDBID;
	_wcscpy(packet.wszChatMsg, CHATLENMAX, lpwszChatMsg, nLen);
	packet.nLen = nLen;

	int iSize = static_cast<int>(sizeof(packet) - sizeof(packet.wszChatMsg) + sizeof(packet.wszChatMsg[0]) * packet.nLen);
	m_pSession->AddSendData(SC_CHAT, eChat::SC_DOORS_GUILDCHAT, reinterpret_cast<char*>(&packet), iSize);
}
#endif		//#ifdef PRE_ADD_DOORS_GUILDCHAT_DISCONNECT

void CDNUserSendManager::SendGuildMemberLoginList(SCGuildMemberLoginList* pPacket)
{
	int iSize = static_cast<int>(sizeof(SCGuildMemberLoginList) - sizeof(pPacket->List) + sizeof(pPacket->List[0]) * pPacket->nCount);
	m_pSession->AddSendData(SC_GUILD, eGuild::SC_GUILDMEMBLOGINLIST, reinterpret_cast<char*>(pPacket), iSize);
}

void CDNUserSendManager::SendGuildMemberLoginList(TGuildUID GuildUID, int nCount, INT64 *List)
{
	SCGuildMemberLoginList Packet;
	memset(&Packet, 0, sizeof(Packet));

	Packet.GuildUID = GuildUID;
	for (int i = 0 ; std::min<int>(GUILDSIZE_MAX, nCount) > i; ++i) {
		if(GUILDSIZE_MAX <= Packet.nCount) {
			break;
		}

		Packet.List[Packet.nCount] = List[i];
		++Packet.nCount;
	}

	int iSize = static_cast<int>(sizeof(Packet) - sizeof(Packet.List) + sizeof(Packet.List[0]) * Packet.nCount);
	m_pSession->AddSendData(SC_GUILD, eGuild::SC_GUILDMEMBLOGINLIST, reinterpret_cast<char*>(&Packet), iSize);
}

void CDNUserSendManager::SendGuildRewardItem(TGuildRewardItem* GuildRewardItem)
{
	SCGetGuildRewardItem Packet;
	memset(&Packet, 0, sizeof(Packet));	
	if(GuildRewardItem)
		memcpy( Packet.GuildRewardItem, GuildRewardItem, sizeof(Packet.GuildRewardItem) );
	
	m_pSession->AddSendData(SC_GUILD, eGuild::SC_GUILD_GET_REWARDITEM, reinterpret_cast<char*>(&Packet), sizeof(Packet));
}

void CDNUserSendManager::SendAddGuildRewardItem( TGuildRewardItem GuildRewardItem )
{
	SCAddGUildRewardItem Packet;
	memset(&Packet, 0, sizeof(Packet));
	memcpy( &Packet.GuildRewardItem, &GuildRewardItem, sizeof(Packet.GuildRewardItem) );

	m_pSession->AddSendData(SC_GUILD, eGuild::SC_GUILD_ADD_REWARDITEM, reinterpret_cast<char*>(&Packet), sizeof(Packet));
}

void CDNUserSendManager::SendBuyGuildRewardItem( int iRet, int nItemID )
{
	SCBuyGuildRewardItem Packet;
	memset(&Packet, 0, sizeof(Packet));	
	Packet.iRet = iRet;
	Packet.nItemID = nItemID;
	
	m_pSession->AddSendData(SC_GUILD, eGuild::SC_GUILD_BUY_REWARDITEM, reinterpret_cast<char*>(&Packet), sizeof(Packet));
}

void CDNUserSendManager::SendExtendGuildSize(short nGuildSize)
{
	SCExtendGuildSize Packet;
	memset(&Packet, 0, sizeof(Packet));	
	Packet.nGuildSize = nGuildSize;

	m_pSession->AddSendData(SC_GUILD, eGuild::SC_GUILD_EXTEND_GUILDSIZE, reinterpret_cast<char*>(&Packet), sizeof(SCExtendGuildSize));
}

void CDNUserSendManager::SendPlayerGuildInfo( int nRet, UINT nSessionID, LPCWSTR lpwszGuildName, LPCWSTR lpwszMasterName, int nGuildLevel, int nMemberSize, int nGuildSize, TGuildRewardItem* GuildRewardItem )
{
	SCPlayerGuildInfo Packet;
	memset(&Packet, 0, sizeof(Packet));	
	Packet.nSessionID = nSessionID;
	if(lpwszGuildName) 
		_wcscpy(Packet.wszGuildName, _countof(Packet.wszGuildName), lpwszGuildName, (int)wcslen(lpwszGuildName));
	if(lpwszMasterName)
		_wcscpy(Packet.wszMasterName, _countof(Packet.wszMasterName), lpwszMasterName, (int)wcslen(lpwszMasterName));
	Packet.nGuildLevel = nGuildLevel;  
	Packet.nMemberSize = nMemberSize;
	Packet.nGuildSize = nGuildSize;
	if(GuildRewardItem)
		memcpy( Packet.GuildRewardItem, GuildRewardItem, sizeof(Packet.GuildRewardItem) );

	m_pSession->AddSendData(SC_GUILD, eGuild::SC_PLAYER_REQUEST_GUILDINFO, reinterpret_cast<char*>(&Packet), sizeof(SCPlayerGuildInfo));
}

void CDNUserSendManager::SendPvPVillageToLobby()
{
	if( !m_pSession ) return;

	m_pSession->AddSendData( SC_SYSTEM, eSystem::SC_MOVE_PVPVILLAGETOLOBBY, NULL, 0 );
}

void CDNUserSendManager::SendPvPCreateRoom( const short nRetCode, const UINT uiPvPIndex/*=0*/, const CSPVP_CREATEROOM* pPacket/*=NULL*/ )
{
	if( !m_pSession )
		return;

	SCPVP_CREATEROOM TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.nRetCode = nRetCode;

	int nSize = sizeof(TxPacket.nRetCode);

	if( nRetCode == ERROR_NONE )
	{
		TxPacket.uiPvPIndex	= uiPvPIndex;

		memcpy( &TxPacket.sCSPVP_CREATEROOM, pPacket, sizeof(CSPVP_CREATEROOM)-sizeof(TxPacket.sCSPVP_CREATEROOM.wszBuf)+sizeof(WCHAR)*(pPacket->cRoomNameLen+pPacket->cRoomPWLen) );
		nSize += sizeof(UINT);
		nSize += sizeof(CSPVP_CREATEROOM)-sizeof(TxPacket.sCSPVP_CREATEROOM.wszBuf)+sizeof(WCHAR)*(pPacket->cRoomNameLen+pPacket->cRoomPWLen);
	}
	
	m_pSession->AddSendData( SC_PVP, ePvP::SC_CREATEROOM, reinterpret_cast<char*>(&TxPacket), nSize );
}

void CDNUserSendManager::SendPvPModifyRoom( const short nRetCode, const CSPVP_MODIFYROOM* pPacket/*=NULL*/ )
{
	if( !m_pSession )
		return;

	SCPVP_MODIFYROOM TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.nRetCode = nRetCode;

	int nSize = sizeof(TxPacket.nRetCode);

	if( nRetCode == ERROR_NONE )
	{
		int iDummySize = sizeof(CSPVP_MODIFYROOM)-sizeof(TxPacket.sCSPVP_MODIFYROOM.sCSPVP_CREATEROOM.wszBuf)+sizeof(WCHAR)*(pPacket->sCSPVP_CREATEROOM.cRoomNameLen+pPacket->sCSPVP_CREATEROOM.cRoomPWLen);
		memcpy( &TxPacket.sCSPVP_MODIFYROOM, pPacket, iDummySize );
		nSize += iDummySize;
	}

	m_pSession->AddSendData( SC_PVP, ePvP::SC_MODIFYROOM, reinterpret_cast<char*>(&TxPacket), nSize );
}

void CDNUserSendManager::SendPvPLeaveRoom( const short nRetCode, PvPCommon::LeaveType::eLeaveType Type, const UINT uiLeaveUserSessionID )
{
	if( !m_pSession )
		return;

	SCPVP_LEAVEROOM TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.nRetCode				= nRetCode;
	TxPacket.uiLeaveUserSessionID	= uiLeaveUserSessionID;
	TxPacket.eType					= Type;

	m_pSession->AddSendData( SC_PVP, ePvP::SC_LEAVEROOM, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

#if defined(_VILLAGESERVER)

void CDNUserSendManager::SendPvPRoomList( const short nRetCode )
{
	if( !m_pSession )
		return;

	SCPVP_ROOMLIST TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.nRetCode		= nRetCode;
	TxPacket.cRoomCount		= 0;
	TxPacket.uiMaxPage		= 0;
	WCHAR* pBufferOffset	= TxPacket.RoomInfoList.wszBuffer;

	int iSize = static_cast<int>(sizeof(TxPacket)-sizeof(TxPacket.RoomInfoList.wszBuffer)+(sizeof(WCHAR)*(pBufferOffset-TxPacket.RoomInfoList.wszBuffer)) );
	m_pSession->AddSendData( SC_PVP, ePvP::SC_ROOMLIST, reinterpret_cast<char*>(&TxPacket), iSize );
}

void CDNUserSendManager::SendPvPRoomList( const short nRetCode, std::vector<CDNPvPRoom*>& vPvPRoom, const UINT uiMaxPage )
{
	if( !m_pSession )
		return;

	SCPVP_ROOMLIST TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.nRetCode		= nRetCode;
	TxPacket.cRoomCount		= static_cast<BYTE>( vPvPRoom.size() );
	TxPacket.uiMaxPage		= uiMaxPage;
	WCHAR* pBufferOffset	= TxPacket.RoomInfoList.wszBuffer;
	for( UINT i=0 ; i<vPvPRoom.size() ; ++i )
		pBufferOffset = vPvPRoom[i]->MakeRoomInfoList( this, &TxPacket.RoomInfoList, i, pBufferOffset );

	int iSize = static_cast<int>(sizeof(TxPacket)-sizeof(TxPacket.RoomInfoList.wszBuffer)+(sizeof(WCHAR)*(pBufferOffset-TxPacket.RoomInfoList.wszBuffer)) );
	m_pSession->AddSendData( SC_PVP, ePvP::SC_ROOMLIST, reinterpret_cast<char*>(&TxPacket), iSize );
}
#if defined(PRE_ADD_PVP_VILLAGE_ACCESS)
void CDNUserSendManager::SendPvPRoomList( SCPVP_ROOMLIST* pPacket, int nSize)
{
	if( !m_pSession )
		return;

	m_pSession->AddSendData( SC_PVP, ePvP::SC_ROOMLIST, reinterpret_cast<char*>(pPacket), nSize);
}
#endif

void CDNUserSendManager::SendPvPWaitUserList( const short nRetCode )
{
	if( !m_pSession )
		return;

	SCPVP_WAITUSERLIST TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.nRetCode = nRetCode;

	m_pSession->AddSendData( SC_PVP, ePvP::SC_WAITUSERLIST, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket.nRetCode) );
}

void CDNUserSendManager::SendPvPWaitUserList( const short nRetCode, std::vector<CDNUserSession*>& vUserList, const UINT uiMaxPage )
{
	if( !m_pSession )
		return;

	SCPVP_WAITUSERLIST TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.nRetCode		= nRetCode;
	TxPacket.cUserCount		= static_cast<BYTE>(vUserList.size());
	TxPacket.unMaxPage		= static_cast<USHORT>(uiMaxPage);
	for( UINT i=0 ; i<vUserList.size() ; ++i )
	{
		TxPacket.Repository.WaitUserList[i].cPvPLevel	= vUserList[i]->GetPvPData()->cLevel;
		TxPacket.Repository.WaitUserList[i].cJob		= vUserList[i]->GetUserJob();
		TxPacket.Repository.WaitUserList[i].cLevel		= vUserList[i]->GetLevel();
#ifdef PRE_ADD_COLOSSEUM_BEGINNER
		TxPacket.Repository.WaitUserList[i].cType		= vUserList[i]->GetPvPChannelType();
#endif		//#ifdef PRE_ADD_COLOSSEUM_BEGINNER
		_wcscpy( TxPacket.Repository.WaitUserList[i].wszCharName, _countof(TxPacket.Repository.WaitUserList[i].wszCharName), 
			vUserList[i]->GetCharacterName(), (int)wcslen(vUserList[i]->GetCharacterName()) );
	}

	int iSize = static_cast<int>(sizeof(TxPacket)-sizeof(TxPacket.Repository)+(sizeof(TxPacket.Repository.WaitUserList[0])*TxPacket.cUserCount ));
	m_pSession->AddSendData( SC_PVP, ePvP::SC_WAITUSERLIST, reinterpret_cast<char*>(&TxPacket), iSize );
}

void CDNUserSendManager::SendPvPJoinRoom( const short nRetCode, CDNPvPRoom* pPvPRoom/*=NULL*/ )
{
	if( !m_pSession )
		return;
	
	SCPVP_JOINROOM TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.nRetCode = nRetCode;
	int iSize = sizeof(short);
	if( nRetCode == ERROR_NONE )
	{	
		if( pPvPRoom )
		{
			if( !pPvPRoom->bMakeJoinRoom( &TxPacket, iSize ) )
			{
				_DANGER_POINT();
				return;
			}

		}
		else
			_DANGER_POINT();
	}

	m_pSession->AddSendData( SC_PVP, ePvP::SC_JOINROOM, reinterpret_cast<char*>(&TxPacket), iSize );
}

void CDNUserSendManager::SendPvPStart( const short nRetCode )
{
	if( !m_pSession )
		return;

	SCPVP_START TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.nRetCode = nRetCode;

	m_pSession->AddSendData( SC_PVP, ePvP::SC_START, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

void CDNUserSendManager::SendPvPChangeTeam( const short nRetCode )
{
	if( !m_pSession )
		return;

	SCPVP_CHANGETEAM TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.nRetCode = nRetCode;

	m_pSession->AddSendData( SC_PVP, ePvP::SC_CHANGETEAM, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

void CDNUserSendManager::SendPvPRoomInfo( MAVIPVP_ROOMINFO* pPacket )
{
	if( !m_pSession )
		return;
	
	SCPVP_ROOMINFO TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.uiIndex				= pPacket->uiIndex;
	TxPacket.uiWinCondition			= pPacket->uiWinCondition;
	TxPacket.uiGameModeTableID		= pPacket->uiGameModeTableID;
	TxPacket.uiPlayTimeSec			= pPacket->uiPlayTimeSec;
	TxPacket.cMaxUser				= pPacket->cMaxUser;
	TxPacket.bIsGuildWar			= pPacket->bIsGuildWar;
#if defined( PRE_PVP_GAMBLEROOM )
	TxPacket.cGambleType = pPacket->cGambleType;
	TxPacket.nGamblePrice = pPacket->nGamblePrice;
#endif
	m_pSession->AddSendData( SC_PVP, ePvP::SC_ROOMINFO, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}
#endif

void CDNUserSendManager::SendPvPData( const TPvPGroup& PvP )
{
	if( !m_pSession )
		return;

	SCPvPInfo TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	memcpy( &TxPacket.PvP, &PvP, sizeof(TPvPGroup) );

	m_pSession->AddSendData( SC_CHAR, eChar::SC_PVPDATA, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

void CDNUserSendManager::SendPvPMemberIndex(USHORT nTeam, BYTE cCount, const TSwapMemberIndex * pIndex, int nRetCode)
{
	if( !m_pSession ) return;

	SCPvPMemberIndex packet;
	memset(&packet, 0, sizeof(SCPvPMemberIndex));
	
	packet.nTeam = nTeam;
	packet.nRetCode = nRetCode;
	if(pIndex)
	{
		packet.cCount = cCount;
		memcpy(packet.Index, pIndex, sizeof(packet.Index));
	}

	m_pSession->AddSendData( SC_PVP, ePvP::SC_PVP_MEMBERINDEX, reinterpret_cast<char*>(&packet), sizeof(packet) - sizeof(packet.Index) +(sizeof(TSwapMemberIndex) * cCount) );
}

void CDNUserSendManager::SendPvPGuildWarMemberGrade(UINT uiUserState, UINT nSessionID, int nRetCode)
{
	if( !m_pSession ) return;

	SCPvPMemberGrade packet;
	memset(&packet, 0, sizeof(SCPvPMemberGrade));

	packet.nRetCode = nRetCode;
	packet.uiUserState = uiUserState;
	packet.nSessionID = nSessionID;
	
	m_pSession->AddSendData(SC_PVP, ePvP::SC_PVP_MEMBERGRADE, reinterpret_cast<char*>(&packet), sizeof(packet));
}

void CDNUserSendManager::SendPvPTryAcquirePoint(int nAreaID, int nRetCode)
{
	if( !m_pSession ) return;

	SCPvPTryAcquirePoint packet;
	memset(&packet, 0, sizeof(SCPvPTryAcquirePoint));

	packet.nAreaID = nAreaID;
	packet.nRetCode = nRetCode;

	m_pSession->AddSendData(SC_PVP, ePvP::SC_PVP_TRYACQUIREPOINT, reinterpret_cast<char*>(&packet), sizeof(packet));
}

void CDNUserSendManager::SendPvPTryAcquireSkill(int nSkillID, int nLevel, int nRetCode)
{
	if(m_pSession == NULL) return;

	SCPvPTryAcquireSkill packet;
	memset(&packet, 0, sizeof(SCPvPTryAcquireSkill));

	packet.nSkillID = nSkillID;
	packet.nLevel = nLevel;
	packet.nRetCode = nRetCode;

	m_pSession->AddSendData(SC_PVP, ePvP::SC_PVP_TRYACQUIRESKILL, reinterpret_cast<char*>(&packet), sizeof(packet));
}

void CDNUserSendManager::SendPvPInitSkill(int nRetCode)
{
	if(m_pSession == NULL) return;

	SCPvPInitSkill packet;
	memset(&packet, 0, sizeof(SCPvPInitSkill));

	packet.nRetCode = nRetCode;

	m_pSession->AddSendData(SC_PVP, ePvP::SC_PVP_INITSKILL, reinterpret_cast<char*>(&packet), sizeof(packet));
}

void CDNUserSendManager::SendPvPUseSkill(UINT nSessionID, int nSkillID, int nRetCode)
{
	if(m_pSession == NULL) return;

	SCPvPUseSkill packet;
	memset(&packet, 0, sizeof(SCPvPUseSkill));

	packet.nSessionID = nSessionID;
	packet.nSkillID = nSkillID;
	packet.nRetCode = nRetCode;

	m_pSession->AddSendData(SC_PVP, ePvP::SC_PVP_USESKILL, reinterpret_cast<char*>(&packet), sizeof(packet));
}

void CDNUserSendManager::SendPvPSwapSkillIndex(char cFrom, char cTo, int nRetCode)
{
	if(m_pSession == NULL) return;

	SCPvPSwapSkillIndex packet;
	memset(&packet, 0, sizeof(SCPvPSwapSkillIndex));

	packet.cFrom = cFrom;
	packet.cTo = cTo;
	packet.nRetCode = nRetCode;

	m_pSession->AddSendData(SC_PVP, ePvP::SC_PVP_SWAPSKILLINDEX, reinterpret_cast<char*>(&packet), sizeof(packet));
}

void CDNUserSendManager::SendPvPUserState(UINT nSessionID, UINT uiUserState)
{
	if(m_pSession == NULL) return;

	SCPVP_USERSTATE TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.uiSessionID = nSessionID;
	TxPacket.uiUserState = uiUserState;
	
	m_pSession->AddSendData(SC_PVP, ePvP::SC_USERSTATE, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket));
}

#if defined(_GAMESERVER) ||(defined(_WINDOWS) && !defined(_LAUNCHER))
void CDNUserSendManager::SendGuildWarConcentrateOrder(UINT nSessionID, EtVector3 vPosition)
{
	SCConcentrateOrder packet;
	memset(&packet, 0, sizeof(SCConcentrateOrder));

	packet.nSessionID = nSessionID;
	packet.vPosition = vPosition;

	m_pSession->AddSendData( SC_PVP, ePvP::SC_CONCENTRATE_ORDER, reinterpret_cast<char*>(&packet), sizeof(SCConcentrateOrder) );
}
#endif

void CDNUserSendManager::SendPvPAllKillGroupCaptain( UINT uiGroupCaptainSessionID )
{
	SCPVP_ALLKILL_GROUPCAPTAIN TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.uiGroupCaptainSessionID = uiGroupCaptainSessionID;

	m_pSession->AddSendData( SC_PVP, ePvP::SC_PVP_ALLKILL_GROUPCAPTAIN, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

void CDNUserSendManager::SendPvPAllKillActiveActor( UINT uiActiveActorSessionID )
{
	SCPVP_ALLKILL_ACTIVEPLAYER TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.uiActivePlayerSessionID = uiActiveActorSessionID;

	m_pSession->AddSendData( SC_PVP, ePvP::SC_PVP_ALLKILL_ACTIVEPLAYER, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

#if defined(PRE_ADD_PVP_TOURNAMENT)
void CDNUserSendManager::SendPvPSwapTournamentIndex(int nRetCode, char cSourceIndex, char cDestIndex)
{
	SCPvPSwapTournamentIndex TxPacket;
	memset(&TxPacket, 0, sizeof(TxPacket));

	TxPacket.nRetCode = nRetCode;
	TxPacket.cSourceIndex = cSourceIndex;
	TxPacket.cDestIndex = cDestIndex;
	m_pSession->AddSendData( SC_PVP, ePvP::SC_PVP_SWAP_TOURNAMENT_INDEX, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}
#endif // #if defined(PRE_ADD_PVP_TOURNAMENT)

#if defined(PRE_ADD_QUICK_PVP)
void CDNUserSendManager::SendQuickPvPInvite(UINT nSenderSessionID)
{
	SCQuickPvPInvite packet;
	packet.nSenderSessionID = nSenderSessionID;

	m_pSession->AddSendData( SC_PVP, ePvP::SC_QUICKPVP_INVITE, reinterpret_cast<char*>(&packet), sizeof(SCQuickPvPInvite));
}

void CDNUserSendManager::SendQuickPvPResult(int nResult)
{
	SCQuickPvPResult packet;
	packet.nResult = nResult;

	m_pSession->AddSendData( SC_PVP, ePvP::SC_QUICKPVP_RESULT, reinterpret_cast<char*>(&packet), sizeof(SCQuickPvPInvite));
}
#endif //#if defined(PRE_ADD_QUICK_PVP)
#if defined(PRE_ADD_PVP_VILLAGE_ACCESS)
void CDNUserSendManager::SendPvPListOpenUI(int nResult, int nLimitLevel)
{
	SCPvPListOpenUI packet;
	packet.nResult = nResult;
	packet.nLimitLevel = nLimitLevel;

	m_pSession->AddSendData( SC_PVP, ePvP::SC_PVP_LIST_OPEN_UI, reinterpret_cast<char*>(&packet), sizeof(SCPvPListOpenUI));
}
#endif

void CDNUserSendManager::SendLadderEnterChannel( int iRet, LadderSystem::MatchType::eCode Type )
{
	LadderSystem::SC_ENTER_CHANNEL TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.iRet		= iRet;
	TxPacket.MatchType	= Type;

	m_pSession->AddSendData( SC_PVP, ePvP::SC_ENTER_LADDERCHANNEL, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );

}

void CDNUserSendManager::SendLadderLeaveChannel( int iRet )
{
	LadderSystem::SC_LEAVE_CHANNEL TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.iRet = iRet;

	m_pSession->AddSendData( SC_PVP, ePvP::SC_LEAVE_LADDERCHANNEL, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );

}

void CDNUserSendManager::SendLadderMatching( int iRet, bool bIsCancel )
{
	LadderSystem::SC_LADDER_MATCHING TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.iRet		= iRet;
	TxPacket.bIsCancel	= bIsCancel;

	m_pSession->AddSendData( SC_PVP, ePvP::SC_LADDER_MATCHING, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );

}

void CDNUserSendManager::SendLadderNotifyLeaveUser( INT64 biCharDBID, const WCHAR* pwszCharName, LadderSystem::Reason::eCode Type )
{
	LadderSystem::SC_NOTIFY_LEAVEUSER TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.biCharDBID = biCharDBID;
	TxPacket.Reason		= Type;

	m_pSession->AddSendData( SC_PVP, ePvP::SC_LADDER_NOTIFY_LEAVEUSER, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );

}

void CDNUserSendManager::SendLadderNotifyJoinUser( INT64 biCharDBID, const WCHAR* pwszCharName, int iGradePoint, BYTE cJobID )
{
	LadderSystem::SC_NOTIFY_JOINUSER TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.sUserInfo.biCharDBID	= biCharDBID;
	TxPacket.sUserInfo.iGradePoint	= iGradePoint;
	TxPacket.sUserInfo.cJob			= cJobID;
	_wcscpy( TxPacket.sUserInfo.wszCharName, _countof(TxPacket.sUserInfo.wszCharName), pwszCharName, (int)wcslen(pwszCharName) );

	m_pSession->AddSendData( SC_PVP, ePvP::SC_LADDER_NOTIFY_JOINUSER, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );

}

void CDNUserSendManager::SendLadderNotifyLeader( const WCHAR* pwszCharName )
{
	LadderSystem::SC_NOTIFY_LEADER TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	_wcscpy( TxPacket.wszLeaderName, _countof(TxPacket.wszLeaderName), pwszCharName, (int)wcslen(pwszCharName) );

	m_pSession->AddSendData( SC_PVP, ePvP::SC_LADDER_NOTIFY_LEADER, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );

}

void CDNUserSendManager::SendLadderNotifyRoomState( LadderSystem::RoomState::eCode State, LadderSystem::RoomStateReason::eCode Reason )
{
	LadderSystem::SC_NOTIFY_ROOMSTATE TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.State	= State;
	TxPacket.Reason	= Reason;

	m_pSession->AddSendData( SC_PVP, ePvP::SC_LADDER_NOTIFY_ROOMSTATE, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );

}

#if defined( _VILLAGESERVER )
void CDNUserSendManager::SendLadderPlayingRoomList( int iRet, const std::vector<LadderSystem::CRoom*>& vRoomList/*=std::vector<LadderSystem::CRoom>()*/, UINT uiMaxPage/*=0*/ )
{
	LadderSystem::SC_PLAYING_ROOMLIST TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	int iSize = sizeof(TxPacket)-sizeof(TxPacket.wszCharName);
	TxPacket.iRet		= iRet;
	TxPacket.unMaxPage	= static_cast<USHORT>(uiMaxPage);
	if( iRet == ERROR_NONE )
	{
		for( UINT i=0 ; i<vRoomList.size() ; ++i )
		{
			if( TxPacket.unNameCount >= _countof(TxPacket.wszCharName) )
			{
				_ASSERT(0);
				break;
			}

			if( vRoomList[i]->bIsValidUserInfo() == false )
			{
				_ASSERT(0);
				continue;
			}
			LadderSystem::CRoom* pOpponentRoom = LadderSystem::CManager::GetInstance().GetRoomPtr( vRoomList[i]->GetOpponentRoomIndex() );
			if( pOpponentRoom == NULL )
			{
				_ASSERT(0);
				continue;
			}
			if( pOpponentRoom->bIsValidUserInfo() == false )
			{
				_ASSERT(0);
				continue;
			}
			if( LadderSystem::CManager::GetInstance().bIsValidPairRoom( vRoomList[i]->GetRoomIndex(), pOpponentRoom->GetRoomIndex() ) == false )
			{
				_ASSERT(0);
				continue;
			}
			
			for( int j=0 ; j<vRoomList[i]->GetMatchType() ; ++j )
			{
				TxPacket.cJob[TxPacket.unNameCount] = vRoomList[i]->GetUserInfo()[j].cJob;
				USHORT unNameCount = TxPacket.unNameCount;
				_wcscpy( TxPacket.wszCharName[unNameCount], _countof(TxPacket.wszCharName[unNameCount]),
					vRoomList[i]->GetUserInfo()[j].wszCharName, (int)wcslen(vRoomList[i]->GetUserInfo()[j].wszCharName) );
				TxPacket.unNameCount++;
			}
			for( int j=0 ; j<pOpponentRoom->GetMatchType() ; ++j )
			{
				TxPacket.cJob[TxPacket.unNameCount] = pOpponentRoom->GetUserInfo()[j].cJob;
				USHORT unNameCount = TxPacket.unNameCount;
				_wcscpy( TxPacket.wszCharName[unNameCount], _countof(TxPacket.wszCharName[unNameCount]),
					pOpponentRoom->GetUserInfo()[j].wszCharName, (int)wcslen(pOpponentRoom->GetUserInfo()[j].wszCharName) );
				TxPacket.unNameCount++;
			}
		}
	}

	iSize += (TxPacket.unNameCount*sizeof(TxPacket.cJob[0]));
	iSize += (TxPacket.unNameCount*sizeof(TxPacket.wszCharName[0]));
	m_pSession->AddSendData( SC_PVP, ePvP::SC_LADDER_PLAYING_ROOMLIST, reinterpret_cast<char*>(&TxPacket), iSize );

}

void CDNUserSendManager::SendLadderObserver( int iRet )
{
	LadderSystem::SC_OBSERVER TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.iRet = iRet;

	m_pSession->AddSendData( SC_PVP, ePvP::SC_LADDER_OBSERVER, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );

}

#endif // #if defined( _VILLAGESERVER )

void CDNUserSendManager::SendOccupationModeState(int nState)
{
	SCPvPOccupationModeState packet;
	memset(&packet, 0, sizeof(SCPvPOccupationModeState));

	packet.nState = nState;

	m_pSession->AddSendData(SC_PVP, ePvP::SC_PVP_OCCUPATION_MODESTATE, reinterpret_cast<char*>(&packet), sizeof(packet));
}

void CDNUserSendManager::SendOccupationState(BYTE cCount, const PvPCommon::OccupationStateInfo * pInfo)
{
	if(pInfo == NULL)
	{
		_DANGER_POINT();
		return;
	}

	SCPvPOccupationState packet;
	memset(&packet, 0, sizeof(SCPvPOccupationState));

	packet.cCount = cCount;

	for (int i = 0; i < cCount || i < PARTYMAX; i++)
	{
		packet.Info[i].nAreaID = pInfo[i].nAreaID;
		packet.Info[i].nOccupationState = pInfo[i].nOccupationState;

		packet.Info[i].nOwnedUniqueID = pInfo[i].nOwnedUniqueID;
		packet.Info[i].nOwnedTemID = pInfo[i].nOwnedTemID;

		packet.Info[i].nTryUniqueID = pInfo[i].nTryUniqueID;
		packet.Info[i].nTryTeamID = pInfo[i].nTryTeamID;

		packet.Info[i].nMoveWaitUniqueID = pInfo[i].nMoveWaitUniqueID;
		packet.Info[i].nMoveWaitTeamID = pInfo[i].nMoveWaitTeamID;
	}

	m_pSession->AddSendData( SC_PVP, ePvP::SC_PVP_OCCUPATION_POINTSTATE, reinterpret_cast<char*>(&packet), sizeof(packet) - sizeof(packet.Info) +(sizeof(PvPCommon::OccupationStateInfo) * cCount) );
}

void CDNUserSendManager::SendOccupationTeamState(int nTeam, int nResource, int nResourceSum)
{
	SCPvPOccupationTeamState packet;
	memset(&packet, 0, sizeof(SCPvPOccupationTeamState));

	packet.nTeam = nTeam;
	packet.nResource = nResource;
	packet.nResourceSum = nResourceSum;

	m_pSession->AddSendData( SC_PVP, ePvP::SC_PVP_OCCUPATION_TEAMSTATE, reinterpret_cast<char*>(&packet), sizeof(packet) );
}

void CDNUserSendManager::SendOccupationSkillState(const PvPCommon::OccupationSkillSlotInfo * pInfo)
{
	if(pInfo == NULL)
		return;

	SCPvPOccupationSkillState packet;
	memset(&packet, 0, sizeof(SCPvPOccupationSkillState));

	memcpy_s(&packet.Info, sizeof(SCPvPOccupationSkillState), pInfo, sizeof(PvPCommon::OccupationSkillSlotInfo));

	m_pSession->AddSendData( SC_PVP, ePvP::SC_PVP_OCCUPATION_SKILLSTATE, reinterpret_cast<char*>(&packet), sizeof(packet) );
}

void CDNUserSendManager::SendOccupationClimaxMode()
{
	m_pSession->AddSendData( SC_PVP, ePvP::SC_PVP_CLIMAXMODE, NULL, 0);
}

#if defined(PRE_ADD_REVENGE)
void CDNUserSendManager::SendPvPSetRevengeTarget( UINT uiSessionID, UINT uiRevengeTargetSessionID, Revenge::TargetReason::eCode eReason )
{
	SCPvPSetRevengeTarget packet;
	packet.uiSessionID = uiSessionID;
	packet.uiRevengeTargetSessionID = uiRevengeTargetSessionID;
	packet.eReason = eReason;

	m_pSession->AddSendData( SC_PVP, ePvP::SC_PVP_SET_REVENGE_TARGET, reinterpret_cast<char*>(&packet), sizeof(packet) );
}

#endif

void CDNUserSendManager::SendTriggerCallAction( int nSectorIndex, int nTriggerIndex, int nRandomSeed )
{
	SCTriggerCallAction packet;
	packet.wSectorIndex = (short)nSectorIndex;
	packet.wObjectIndex = (short)nTriggerIndex;
	packet.nRandomSeed = nRandomSeed;

	m_pSession->AddSendData(SC_TRIGGER, eTrigger::SC_TRIGGER_CALLACTION, (char*)&packet, sizeof(SCTriggerCallAction));
}

void CDNUserSendManager::SendTriggerChangeMyBGM( int nBGM, int nFadeDelta )
{
	SCChangeMyBGM packet;
	packet.nBGM = nBGM;
	packet.nFadeDelta = nFadeDelta;
	packet.nSessionID = m_pSession->GetSessionID();

	m_pSession->AddSendData(SC_TRIGGER, eTrigger::SC_CHANGE_MY_BGM, (char*)&packet, sizeof(SCChangeMyBGM));
}

void CDNUserSendManager::SendTriggerFileTableBGMOff()
{
	SCFileTableBGMOff packet;
	packet.nSessionID = m_pSession->GetSessionID();

	m_pSession->AddSendData( SC_TRIGGER, eTrigger::SC_FILE_TABLE_BGM_OFF, (char*)&packet, sizeof(SCFileTableBGMOff) );
}

void CDNUserSendManager::SendTriggerRadioImage( UINT nSessionID, int nFileID, DWORD nTime )
{
	SCRadioImage packet;
	memset(&packet, 0, sizeof(packet));

	packet.nSessionID = nSessionID;
	packet.nFileID = nFileID;
	packet.nTime = nTime;

	m_pSession->AddSendData(SC_TRIGGER, eTrigger::SC_RADIO_IMAGE, (char*)&packet, sizeof(SCRadioImage));
}

void CDNUserSendManager::SendTriggerForceEnableRide( const UINT nSessionID, const bool bForceEnableRide )
{
	SCForceEnableRide packet;
	memset( &packet, 0, sizeof(packet) );

	packet.nSessionID = nSessionID;
	packet.bForceEnableRide = bForceEnableRide;

	m_pSession->AddSendData(SC_TRIGGER, eTrigger::SC_TRIGGER_FORCE_ENABLE_RIDE, (char*)&packet, sizeof(SCForceEnableRide));
}

void CDNUserSendManager::SendVoiceChatInfo(char *pIp, USHORT wControlPort, USHORT wAudioPort, UINT m_nAccountDBID)
{
	SCVoiceChatInfo Info = { 0, };

	_strcpy(Info.szVoiceChatIp, _countof(Info.szVoiceChatIp), pIp, (int)strlen(pIp));
	Info.wControlPort = wControlPort;
	Info.wAudioPort = wAudioPort;
	Info.nAccountDBID = m_nAccountDBID;

	m_pSession->AddSendData(SC_VOICECHAT, eVoiceChat::SC_VOICECHATINFO, (char*)&Info, sizeof(SCVoiceChatInfo));
}

void CDNUserSendManager::SendTalkingUpdate(int nCount, TTalkingInfo * pInfo)
{
	SCTalkingInfo packet;
	memset(&packet, 0, sizeof(SCTalkingInfo));

	packet.cCount = (BYTE)nCount;
	memcpy(packet.Talking, pInfo, sizeof(TTalkingInfo) * nCount);

	m_pSession->AddSendData(SC_VOICECHAT, eVoiceChat::SC_TALKINGINFO, (char*)&packet, sizeof(SCTalkingInfo) - sizeof(packet.Talking) +(sizeof(TTalkingInfo) * packet.cCount));
}

void CDNUserSendManager::SendVoiceMemberInfoRefresh(int nCount, UINT * pMuteSessionList, TVoiceMemberInfo * pInfo)
{
	SCVoiceMemberInfo packet;
	memset(&packet, 0, sizeof(SCVoiceMemberInfo));

	packet.cCount = (BYTE)nCount;
	memcpy(packet.Info, pInfo, sizeof(TVoiceMemberInfo) * nCount);
	memcpy(packet.nVoiceMutedList, pMuteSessionList, sizeof(packet.nVoiceMutedList));

	m_pSession->AddSendData(SC_VOICECHAT, eVoiceChat::SC_VOICEMEMBERINFO, (char*)&packet, sizeof(SCVoiceMemberInfo) - sizeof(packet.Info) +(sizeof(TVoiceMemberInfo) * packet.cCount));
}


void CDNUserSendManager::SendIsolateResult(int nRetCode)
{
	SCIsolateResult packet;
	memset(&packet, 0, sizeof(packet));

	packet.nRetCode = nRetCode;

	m_pSession->AddSendData(SC_ISOLATE, eIsolate::SC_ISOLATE_RESULT, (char*)&packet, sizeof(packet));
}

void CDNUserSendManager::SendIsolateList(SCIsolateList * pPacket, int nSize)
{
	m_pSession->AddSendData(SC_ISOLATE, eIsolate::SC_ISOLATE_LIST, (char*)pPacket, sizeof(SCIsolateList) - sizeof(pPacket->wszIsolateName) + nSize);
}

void CDNUserSendManager::SendIsolateAdd(const WCHAR * pAddIsolateName)
{
	SCIsolateAdd packet;
	memset(&packet, 0, sizeof(packet));

	_wcscpy(packet.wszIsolateName, _countof(packet.wszIsolateName), pAddIsolateName, (int)wcslen(pAddIsolateName));

	m_pSession->AddSendData(SC_ISOLATE, eIsolate::SC_ISOLATE_ADD, (char*)&packet, sizeof(packet));
}

void CDNUserSendManager::SendIsolateDel(std::vector <std::wstring> * vList)
{
	SCIsolateDelete packet;
	memset(&packet, 0, sizeof(SCIsolateDelete));

	int nSize = 0;
	std::vector <std::wstring>::iterator ii;
	for(ii = vList->begin(); ii != vList->end(); ii++)
	{
		packet.cNameLen[packet.cCount] = (BYTE)wcslen((*ii).c_str());
		wmemcpy_s( packet.wszIsolateName+nSize, _countof(packet.wszIsolateName)-nSize, (*ii).c_str(), packet.cNameLen[packet.cCount]);
		nSize += packet.cNameLen[packet.cCount];
		packet.cCount++;
	}
	nSize = nSize * sizeof(WCHAR);

	m_pSession->AddSendData(SC_ISOLATE, eIsolate::SC_ISOLATE_DELETE, (char*)&packet, sizeof(packet) - sizeof(packet.wszIsolateName) + nSize);
}

void CDNUserSendManager::SendIsolateDel(const WCHAR * pDelIsolateName)
{
	if(pDelIsolateName == NULL) return;

	SCIsolateDelete packet;
	memset(&packet, 0, sizeof(SCIsolateDelete));

	_wcscpy(packet.wszIsolateName, _countof(packet.wszIsolateName), pDelIsolateName, (int)wcslen(pDelIsolateName));
	packet.cNameLen[0] = (BYTE)wcslen(pDelIsolateName);
	packet.cCount = 1;

	m_pSession->AddSendData(SC_ISOLATE, eIsolate::SC_ISOLATE_DELETE, (char*)&packet, sizeof(packet) - sizeof(packet.wszIsolateName) +(packet.cNameLen[0] * sizeof(WCHAR)));
}

void CDNUserSendManager::SendMissionList(int nScore, char *pGain, char *pAchieve, short *pLastUpdate)
{
	SCMissionList packet;
	memset(&packet, 0, sizeof(packet));

	packet.nMissionScore = nScore;
	memcpy(packet.cGain, pGain, MISSIONMAX_BITSIZE);
	memcpy(packet.cAchieve, pAchieve, MISSIONMAX_BITSIZE);
	memcpy(packet.wLastUpdate, pLastUpdate, sizeof(short)*LASTMISSIONACHIEVEMAX);

	/*
	int nOffset;
	for( int i=0; i<MISSIONMAX_BITSIZE; i++ ) {
		nOffset = i * 8;
		packet.cGain[i] = ((pGain[nOffset+0]<<7)&0xFF ) |((pGain[nOffset+1]<<6)&0xFF ) |
			((pGain[nOffset+2]<<5)&0xFF ) |((pGain[nOffset+3]<<4)&0xFF ) |
			((pGain[nOffset+4]<<3)&0xFF ) |((pGain[nOffset+5]<<2)&0xFF ) |
			((pGain[nOffset+6]<<1)&0xFF ) |( pGain[nOffset+7]&0xFF );

		packet.cAchieve[i] = ((pAchieve[nOffset+0]<<7)&0xFF ) |((pAchieve[nOffset+1]<<6)&0xFF ) |
			((pAchieve[nOffset+2]<<5)&0xFF ) |((pAchieve[nOffset+3]<<4)&0xFF ) |
			((pAchieve[nOffset+4]<<3)&0xFF ) |((pAchieve[nOffset+5]<<2)&0xFF ) |
			((pAchieve[nOffset+6]<<1)&0xFF ) |( pAchieve[nOffset+7]&0xFF );
	}
	*/

	m_pSession->AddSendData(SC_MISSION, eMission::SC_MISSION_LIST, (char*)&packet, sizeof(packet));
}

void CDNUserSendManager::SendMissionGain( int nArrayIndex )
{
	SCMissionGain packet;
	memset(&packet, 0, sizeof(packet));

	packet.nArrayIndex = nArrayIndex;
	m_pSession->AddSendData(SC_MISSION, eMission::SC_GAIN_MISSION, (char*)&packet, sizeof(packet));
}

void CDNUserSendManager::SendMissionAchieve( int nArrayIndex )
{
	SCMissionAchieve packet;
	memset(&packet, 0, sizeof(packet));

	packet.nArrayIndex = nArrayIndex;
	m_pSession->AddSendData(SC_MISSION, eMission::SC_ACHIEVE_MISSION, (char*)&packet, sizeof(packet));
}

void CDNUserSendManager::SendDailyMissionList( char cType, TDailyMission *pMission )
{
	SCDailyMissionList packet;
	memset(&packet, 0, sizeof(packet));

	packet.cType = cType;

	int nCountMax = 0;
	switch( cType )
	{
		case 0: nCountMax = DAILYMISSIONMAX; break;
		case 1: nCountMax = WEEKLYMISSIONMAX; break;
		case 2: nCountMax = GUILDWARMISSIONMAX; break;
		case 4: nCountMax = GUILDCOMMONMISSIONMAX; break;
		case 5: nCountMax = WEEKENDEVENTMISSIONMAX; break;
		case 6: nCountMax = WEEKENDREPEATMISSIONMAX; break;
#if defined(PRE_ADD_MONTHLY_MISSION)
		case 7: nCountMax = MONTHLYMISSIONMAX; break;
#endif	// #if defined(PRE_ADD_MONTHLY_MISSION)
	}

	int nCount = 0;
	for( int i=0; i<nCountMax; i++ ) {
		if( pMission[i].nMissionID > 0 ){
			nCount++;
		}
	};
	packet.nCount = nCount;

	if(nCount > 0)
		memcpy( packet.Mission, pMission, sizeof(TDailyMission) * nCount );

	m_pSession->AddSendData(SC_MISSION, eMission::SC_DAILYMISSION_LIST, (char*)&packet, sizeof(SCDailyMissionList) - sizeof(packet.Mission) +( sizeof(TDailyMission) * nCount ));
}

void CDNUserSendManager::SendDailyMissionCount( char cType, int nArrayIndex, int nCount )
{
	SCDailyMissionCount packet;
	memset(&packet, 0, sizeof(packet));

	packet.cType = cType;
	packet.nArrayIndex = nArrayIndex;
	packet.nCount = nCount;

	m_pSession->AddSendData(SC_MISSION, eMission::SC_COUNT_DAILYMISSION, (char*)&packet, sizeof(SCDailyMissionCount));
}
void CDNUserSendManager::SendDailyMissionAchieve( char cType, int nArrayIndex, bool bSecret/*=false*/)
{
	SCDailyMissionAchieve packet;
	memset(&packet, 0, sizeof(packet));

	packet.cType = cType;
	packet.nArrayIndex = nArrayIndex;
	packet.bSecret = bSecret;
	m_pSession->AddSendData(SC_MISSION, eMission::SC_ACHIEVE_DAILYMISSION, (char*)&packet, sizeof(SCDailyMissionAchieve));
}

void CDNUserSendManager::SendDailyMissionAlarm( char cType )
{
	SCDailyMissionAlarm packet;
	memset(&packet, 0, sizeof(packet));

	packet.cType = cType;

	m_pSession->AddSendData(SC_MISSION, eMission::SC_ALARM_DAILYMISSION, (char*)&packet, sizeof(SCDailyMissionAlarm));
}

// Event
void CDNUserSendManager::SendMissionEventPopUp( int nArrayIndex )
{
	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TMISSION );

	if(!pSox)
	{
		g_Log.Log( LogType::_FILELOG, L"MissionTable.ext failed\r\n");
		return;
	}

	SCMissionEventPopUp packet;
	memset(&packet, 0, sizeof(packet));
	
	// Allow finding the record in PopUpTable by the item ID.
	packet.nItemID = pSox->GetItemID(nArrayIndex);
	m_pSession->AddSendData(SC_MISSION, eMission::SC_EVENT_POPUP, (char*)&packet, sizeof(packet));
}

void CDNUserSendManager::SendMissionHelpAlarm( int nArrayIndex )
{
	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TMISSION );

	if(!pSox)
	{
		g_Log.Log( LogType::_FILELOG, L"MissionTable.ext failed\r\n");
		return;
	}

	SCMissionHelpAlarm packet;
	memset(&packet, 0, sizeof(packet));

	// Allow finding the record in PopUpTable by the item ID.
	packet.nItemID = pSox->GetItemID(nArrayIndex);
	m_pSession->AddSendData(SC_MISSION, eMission::SC_HELP_ALARM, (char*)&packet, sizeof(packet));
}

#if defined(PRE_ADD_ACTIVEMISSION)
void CDNUserSendManager::SendActiveMissionGain( int nIndex )
{
	SCActiveMissionGain packet;
	memset(&packet, 0, sizeof(packet));

	packet.nIndex = nIndex;
	m_pSession->AddSendData(SC_MISSION, eMission::SC_GAIN_ACTIVE_MISSION, (char*)&packet, sizeof(packet));
}

void CDNUserSendManager::SendActiveMissionAchieve( int nIndex )
{
	SCActiveMissionAchieve packet;
	memset(&packet, 0, sizeof(packet));

	packet.nIndex = nIndex;
	m_pSession->AddSendData(SC_MISSION, eMission::SC_ACHIEVE_ACTIVE_MISSION, (char*)&packet, sizeof(packet));
}
#endif

void CDNUserSendManager::SendAppellationList( char *pGain )
{
	SCAppellationList packet;
	memset(&packet, 0, sizeof(packet));

	memcpy( packet.cGain, pGain, APPELLATIONMAX_BITSIZE );

	m_pSession->AddSendData(SC_APPELLATION, eAppellation::SC_APPELLATION_LIST, (char*)&packet, sizeof(packet));
}

void CDNUserSendManager::SendAppellationGain( int nArrayIndex )
{
	SCAppellationGain packet;
	memset( &packet, 0, sizeof(packet) );

	packet.nArrayIndex = nArrayIndex;

	m_pSession->AddSendData(SC_APPELLATION, eAppellation::SC_GAIN_APPELLATION, (char*)&packet, sizeof(packet));
}

void CDNUserSendManager::SendAppellationTime( BYTE nCount, TAppellationTime *AppellationTIme)
{
	if(nCount == 0) return;

	SCPeriodAppellationTIme PeriodAppellation;
	memset(&PeriodAppellation, 0, sizeof(SCPeriodAppellationTIme));
	
	if(AppellationTIme){
		PeriodAppellation.cPeriodCount = nCount;
		memcpy(PeriodAppellation.AppellationTime, AppellationTIme, sizeof(TAppellationTime) * PeriodAppellation.cPeriodCount);
	}

	int nSize = sizeof(SCPeriodAppellationTIme) - sizeof(PeriodAppellation.AppellationTime) +(sizeof(TAppellationTime) * PeriodAppellation.cPeriodCount);
	m_pSession->AddSendData( SC_APPELLATION, eAppellation::SC_PERIOD_APPELLATION, (char*)&PeriodAppellation, nSize );
}

void CDNUserSendManager::SendResetPcbangAppellation()
{
	m_pSession->AddSendData(SC_APPELLATION, eAppellation::SC_RESET_PCBANGAPPELLATION, NULL, 0);
}

void CDNUserSendManager::SendGameOption(UINT nSessionID, const TGameOptions * pOption, TPARTYID PartyID)
{
	SCGameOptionCommOption packet;
	memset(&packet, 0, sizeof(packet));

	packet.nSessionID = nSessionID;
	memcpy_s(packet.cCommunityOption, sizeof(packet.cCommunityOption), pOption->cCommunityOption, sizeof(pOption->cCommunityOption));
	packet.PartyID = PartyID;

	m_pSession->AddSendData(SC_GAMEOPTION, eGameOption::SC_GAMEOPTION_USERCOMMOPTION, (char*)&packet, sizeof(packet));
}

void CDNUserSendManager::SendGameOptionRefreshNotifier()
{
	SCGameOptionRefreshNotifier TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	for( UINT i=0 ; i<DNNotifier::RegisterCount::Total ; ++i )
	{
		TxPacket.data[i] = m_pSession->m_NotifierRepository.GetData(i);
	}

	m_pSession->AddSendData( SC_GAMEOPTION, eGameOption::SC_GAMEOPTION_REFRESH_NOTIFIER, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

void CDNUserSendManager::SendSecondAuthInfo( bool bPW, bool bLock )
{
	SCGameOptionNotifySecondAuthInfo TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.bIsSetSecondAuthPW	= bPW;
	TxPacket.bIsSetSecondAuthLock = bLock;
	TxPacket.tSecondAuthResetDate = -1;

	m_pSession->AddSendData( SC_GAMEOPTION, eGameOption::SC_NOTIFY_SECONDAUTHINFO, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

void CDNUserSendManager::SendMaxLevelCharacterCount( int iCount )
{
	SCMaxLevelCharacterCount TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.iCount = iCount;

	m_pSession->AddSendData(SC_CHAR, eChar::SC_MAXLEVEL_CHARACTER_COUNT, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );

}

void CDNUserSendManager::SendSecondAuthValidate( int nRet, int nAuthCheckType, BYTE cFailCount )
{
	SCGameOptionValidateSecondAuth TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.nRet			= nRet;
	TxPacket.nAuthCheckType	= nAuthCheckType;
	TxPacket.cFailCount		= cFailCount;

	m_pSession->AddSendData( SC_GAMEOPTION, eGameOption::SC_GAMEOPTION_VALIDATE_SECONDAUTH, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

void CDNUserSendManager::SendProfile( TProfile & profile )
{
	SCGameOptionGetProfile TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	memcpy_s(&TxPacket.sProfile, sizeof(TxPacket.sProfile), &profile, sizeof(profile) );

	m_pSession->AddSendData( SC_GAMEOPTION, eGameOption::SC_GAMEOPTION_GET_PROFILE, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

void CDNUserSendManager::SendDisplayProfile( int nSessionID, TProfile & profile )
{
	SCGameOptionDisplayProfile TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.nSessionID = nSessionID;
	memcpy_s(&TxPacket.sProfile, sizeof(TxPacket.sProfile), &profile, sizeof(profile) );

	m_pSession->AddSendData( SC_GAMEOPTION, eGameOption::SC_GAMEOPTION_DISPLAY_PROFILE, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

void CDNUserSendManager::SendSelectAppellation( int nArrayIndex, int nCoverArrayIndex )
{
	SCSelectAppellation packet;
	memset(&packet, 0, sizeof(packet));

	packet.nArrayIndex = nArrayIndex;
	packet.nCoverArrayIndex = nCoverArrayIndex;
	m_pSession->AddSendData(SC_APPELLATION, eAppellation::SC_SELECT_APPELLATION, (char*)&packet, sizeof(packet));

#if defined(_GAMESERVER)
	m_pSession->GetGameRoom()->OnDispatchMessage( m_pSession, SC_APPELLATION, eAppellation::SC_SELECT_APPELLATION, (char*)&packet, (int)sizeof(packet), 0 );
#endif
}

void CDNUserSendManager::SendChangeAppellation( UINT nSessionID, int nArrayIndex, int nCoverArrayIndex )
{
	SCChangeAppellation packet;
	memset(&packet, 0, sizeof(packet));

	packet.nSessionID = nSessionID;
	packet.nArrayIndex = nArrayIndex;
	packet.nCoverArrayIndex = nCoverArrayIndex;
	m_pSession->AddSendData(SC_APPELLATION, eAppellation::SC_CHANGE_APPELLATION, (char*)&packet, sizeof(packet));
}

void CDNUserSendManager::SendDecreaseDurabilityInventory( const int iValue )
{
	SCDecreaseDurabilityInventory TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.cType		= 0;
	TxPacket.nAbsolute	= iValue;

	m_pSession->AddSendData( SC_ITEM, eItem::SC_DECREASE_DURABILITY_INVENTORY, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

void CDNUserSendManager::SendDecreaseDurabilityInventory( const float fValue )
{
	SCDecreaseDurabilityInventory TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.cType	= 1;
	TxPacket.fRatio	= fValue;

	m_pSession->AddSendData( SC_ITEM, eItem::SC_DECREASE_DURABILITY_INVENTORY, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

void CDNUserSendManager::SendRestrinatList(TRestraintData * pRestraint)
{
	SCRestraintList packet;
	memset(&packet, 0, sizeof(packet));

	for (int i = 0; i < RESTRAINTMAX; i++)
	{
		if(pRestraint->Restraint[i].nRestraintKey <= 0) continue;
		memcpy(&packet.restraint[i], &pRestraint->Restraint[i], sizeof(TRestraint));
		packet.cCount++;
	}

	m_pSession->AddSendData(SC_RESTRAINT, eRestraint::SC_RESTRAINTLIST, (char*)&packet, sizeof(packet) - sizeof(packet.restraint) +(sizeof(TRestraint) * packet.cCount));
}

void CDNUserSendManager::SendRestraint(TRestraint * pRestraint)
{
	SCRestraintAdd packet;
	memset(&packet, 0, sizeof(SCRestraintAdd));

	memcpy(&packet.Restraint, pRestraint, sizeof(TRestraint));

	m_pSession->AddSendData(SC_RESTRAINT, eRestraint::SC_RESTRAINTADD, (char*)&packet, sizeof(packet));
}

void CDNUserSendManager::SendRestraintDel(UINT nRestraintKey)
{
	SCRestraintDel packet;
	memset(&packet, 0, sizeof(SCRestraintDel));

	packet.nRestraintKey = nRestraintKey;

	m_pSession->AddSendData(SC_RESTRAINT, eRestraint::SC_RESTRAINTDEL, (char*)&packet, sizeof(packet));
}

void CDNUserSendManager::SendRestraintRetCode(short nRetCode)
{
	SCRestraintRet packet;
	memset(&packet, 0, sizeof(SCRestraintRet));

	packet.nRestraintRet = nRetCode;

	m_pSession->AddSendData(SC_RESTRAINT, eRestraint::SC_RESTRAINTRETCODE, (char*)&packet, sizeof(packet));
}

// CashShop
void CDNUserSendManager::SendCashShopBalanceInquiry(int nRet, int nPetal, int nCash)
{
	SCCashShopBalanceInquiry CashShop;
	memset(&CashShop, 0, sizeof(SCCashShopBalanceInquiry));

	CashShop.nRet = nRet;
	CashShop.nCash = nCash;
	CashShop.nPetal = nPetal;

	m_pSession->AddSendData(SC_CASHSHOP, eCashShop::SC_BALANCEINQUIRY, (char*)&CashShop, sizeof(SCCashShopBalanceInquiry));
}

void CDNUserSendManager::SendCashShopOpen(UINT nCash, int nReserve, int nRet, WCHAR *pAccountName)
{
	SCCashShopOpen CashShop;
	memset(&CashShop, 0, sizeof(SCCashShopOpen));
	CashShop.nCashAmount = nCash;
	CashShop.nReserveAmount = nReserve;
	CashShop.nRet = nRet;
	if(pAccountName) _wcscpy(CashShop.wszAccountName, _countof(CashShop.wszAccountName), pAccountName, (int)wcslen(pAccountName));
	m_pSession->AddSendData(SC_CASHSHOP, eCashShop::SC_SHOPOPEN, (char*)&CashShop, sizeof(SCCashShopOpen));
}

void CDNUserSendManager::SendCashShopClose(int nRet)
{
	SCCashShopClose CashShop;
	memset(&CashShop, 0, sizeof(SCCashShopClose));
	CashShop.nRet = nRet;
	m_pSession->AddSendData(SC_CASHSHOP, eCashShop::SC_SHOPCLOSE, (char*)&CashShop, sizeof(SCCashShopClose));
}

void CDNUserSendManager::SendCashShopBuy(UINT nCash, int nReserve, int nRet, char cType, char cCount, TCashShopInfo *CashShopArray, TPaymentItemInfo* pPaymentItem/* = NULL*/, INT64 nSeedPoint/*=0*/)
{
	SCCashShopBuy CashShop;
	memset(&CashShop, 0, sizeof(SCCashShopBuy));

	CashShop.nCashAmount = nCash;
	CashShop.nReserveAmount = nReserve;
	CashShop.nRet = nRet;
	CashShop.cType = cType;
#if defined(PRE_ADD_CASH_REFUND)
	if( pPaymentItem )
		memcpy(&CashShop.ItemList, pPaymentItem, sizeof(CashShop.ItemList));
#endif
	if(CashShopArray){
		CashShop.cCount = cCount;

		switch(CashShop.cType)
		{
		case 3:
		case 7:
			if(CashShop.cCount > PREVIEWCARTLISTMAX) CashShop.cCount = PREVIEWCARTLISTMAX;
			break;

		default:
			if(CashShop.cCount > CARTLISTMAX) CashShop.cCount = CARTLISTMAX;
			break;
		}

		memcpy(CashShop.BuyList, CashShopArray, sizeof(TCashShopInfo) * CashShop.cCount);
	}
	int nLen = sizeof(SCCashShopBuy) - sizeof(CashShop.BuyList) +(sizeof(TCashShopInfo) * CashShop.cCount);

	m_pSession->AddSendData(SC_CASHSHOP, eCashShop::SC_BUY, (char*)&CashShop, nLen);
}

void CDNUserSendManager::SendCashShopPackageBuy(int nPackageSN, int nCash, int nReserve, int nRet, const TPaymentPackageItemInfo* pPaymentPackageItem/* = NULL*/, INT64 nSeedPoint/*=0*/)
{
	SCCashShopPackageBuy CashShop;
	memset(&CashShop, 0, sizeof(SCCashShopPackageBuy));

	CashShop.nPackageSN = nPackageSN;
	CashShop.nCashAmount = nCash;
	CashShop.nReserveAmount = nReserve;
	CashShop.nRet = nRet;
#if defined(PRE_ADD_CASH_REFUND)
	if( pPaymentPackageItem )
		memcpy(&CashShop.PackageItem, pPaymentPackageItem, sizeof(TPaymentPackageItemInfo));
#endif

	m_pSession->AddSendData(SC_CASHSHOP, eCashShop::SC_PACKAGEBUY, (char*)&CashShop, sizeof(SCCashShopPackageBuy));
}

#if defined(PRE_ADD_CASH_REFUND)
void CDNUserSendManager::SendCashShopMoveCashinven( INT64 biDBID, int nRet, int nReservePetal, INT64 nSeedPoint )
{
	SCMoveCashInven CashShop;
	memset(&CashShop, 0, sizeof(SCMoveCashInven));

	CashShop.biDBID = biDBID;
	CashShop.nRet = nRet;
	CashShop.nReserveAmount = nReservePetal;

	m_pSession->AddSendData(SC_CASHSHOP, eCashShop::SC_MOVE_CASHINVEN, (char*)&CashShop, sizeof(SCMoveCashInven));
}

void CDNUserSendManager::SendCashShopCashRefund( INT64 biDBID, int nRet, int nCashAmount)
{
	SCCashRefund CashShop;
	memset(&CashShop, 0, sizeof(SCCashRefund));

	CashShop.biDBID = biDBID;
	CashShop.nRet = nRet;
	CashShop.nCashAmount = nCashAmount;

	m_pSession->AddSendData(SC_CASHSHOP, eCashShop::SC_CASH_REFUND, (char*)&CashShop, sizeof(SCCashRefund));
}
#endif

void CDNUserSendManager::SendCashShopCheckReceiver(BYTE cLevel, BYTE cJob, int nRet)
{
	SCCashShopCheckReceiver CashShop;
	memset(&CashShop, 0, sizeof(SCCashShopCheckReceiver));

	CashShop.cLevel = cLevel;
	CashShop.cJob = cJob;
	CashShop.nRet = nRet;

	m_pSession->AddSendData(SC_CASHSHOP, eCashShop::SC_CHECKRECEIVER, (char*)&CashShop, sizeof(SCCashShopCheckReceiver));
}

#if defined(PRE_ADD_CADGE_CASH)
void CDNUserSendManager::SendCashShopCadge(int nRet)
{
	SCCashShopCadge CashShop;

	CashShop.nRet = nRet;

	m_pSession->AddSendData(SC_CASHSHOP, eCashShop::SC_CADGE, (char*)&CashShop, sizeof(SCCashShopCadge));
}
#endif	// #if defined(PRE_ADD_CADGE_CASH)

void CDNUserSendManager::SendCashShopGift(int nCash, int nReserve, int nRet, char cType, int nCount, TCashShopInfo *GiftList, INT64 nSeedPoint/*=0*/)
{
	SCCashShopGift CashShop;
	memset(&CashShop, 0, sizeof(SCCashShopGift));

	CashShop.nCashAmount = nCash;
	CashShop.nReserveAmount = nReserve;
	CashShop.nRet = nRet;
	CashShop.cType = cType;


	if(GiftList){
		CashShop.cCount = nCount;

		switch(CashShop.cType)
		{
		case 3:
		case 7:
			if(CashShop.cCount > PREVIEWCARTLISTMAX) CashShop.cCount = PREVIEWCARTLISTMAX;
			break;

		default:
			if(CashShop.cCount > CARTLISTMAX) CashShop.cCount = CARTLISTMAX;
			break;
		}

		memcpy(CashShop.GiftList, GiftList, sizeof(TCashShopInfo) * CashShop.cCount);
	}

	int nLen = sizeof(SCCashShopGift) - sizeof(CashShop.GiftList) +(sizeof(TCashShopInfo) * CashShop.cCount);

	m_pSession->AddSendData(SC_CASHSHOP, eCashShop::SC_GIFT, (char*)&CashShop, nLen);
}

void CDNUserSendManager::SendCashShopPackageGift(int nPackageSN, int nCash, int nReserve, int nRet, INT64 nSeedPoint/*=0*/)
{
	SCCashShopPackageGift CashShop;
	memset(&CashShop, 0, sizeof(SCCashShopPackageGift));

	CashShop.nPackageSN = nPackageSN;
	CashShop.nCashAmount = nCash;
	CashShop.nReserveAmount = nReserve;
	CashShop.nRet = nRet;


	m_pSession->AddSendData(SC_CASHSHOP, eCashShop::SC_PACKAGEGIFT, (char*)&CashShop, sizeof(SCCashShopPackageGift));
}

void CDNUserSendManager::SendCashShopGiftList(int nRet, int nCount, TGiftInfo *GiftInfoArray)
{
	SCCashShopGiftList CashShop;
	memset(&CashShop, 0, sizeof(SCCashShopGiftList));

	CashShop.nRet = nRet;
	CashShop.cCount = nCount;
	if(GiftInfoArray) memcpy(CashShop.GiftInfo, GiftInfoArray, nCount * sizeof(TGiftInfo));

	int nLen = sizeof(SCCashShopGiftList) - sizeof(CashShop.GiftInfo) + (nCount * sizeof(TGiftInfo));

	m_pSession->AddSendData(SC_CASHSHOP, eCashShop::SC_GIFTLIST, (char*)&CashShop, nLen);
}

void CDNUserSendManager::SendCashShopReceiveGift(INT64 nGiftDBID, int nRet)
{
	SCCashShopReceiveGift CashShop;
	memset(&CashShop, 0, sizeof(SCCashShopReceiveGift));

	CashShop.nGiftDBID = (int)nGiftDBID;
	CashShop.nRet = nRet;
	m_pSession->AddSendData(SC_CASHSHOP, eCashShop::SC_RECEIVEGIFT, (char*)&CashShop, sizeof(SCCashShopReceiveGift));
}

void CDNUserSendManager::SendCashShopReceiveGiftAll(std::vector<INT64> &VecGiftDBIDList, int nRet)
{
	SCCashShopReceiveGiftAll CashShop;
	memset(&CashShop, 0, sizeof(SCCashShopReceiveGiftAll));

	for (int i = 0; i < (int)VecGiftDBIDList.size(); i++){
		CashShop.nGiftDBID[CashShop.cCount] = VecGiftDBIDList[i];
		CashShop.cCount++;
	}
	CashShop.nRet = nRet;

	m_pSession->AddSendData(SC_CASHSHOP, eCashShop::SC_RECEIVEGIFTALL, (char*)&CashShop, sizeof(SCCashShopReceiveGiftAll));
}

#if defined(PRE_ADD_GIFT_RETURN)
void CDNUserSendManager::SendCashShopGiftReturn(INT64 nGiftDBID, int nRet)
{
	SCCashShopGiftReturn CashShop;
	memset(&CashShop, 0, sizeof(SCCashShopGiftReturn));

	CashShop.nGiftDBID = (int)nGiftDBID;
	CashShop.nRet = nRet;
	m_pSession->AddSendData(SC_CASHSHOP, eCashShop::SC_GIFTRETURN, (char*)&CashShop, sizeof(SCCashShopGiftReturn));
}
#endif

void CDNUserSendManager::SendCashshopNotifyGift(bool bNew, int nGiftCount)
{
	SCNotifyGift CashShop;
	CashShop.bNew = bNew;
	CashShop.nGiftCount = nGiftCount;
	m_pSession->AddSendData(SC_CASHSHOP, eCashShop::SC_NOTIFYGIFT, (char*)&CashShop, sizeof(SCNotifyGift));
}

void CDNUserSendManager::SendCoupon(int nRet)
{
	SCCashShopCoupon CashShop;
	CashShop.nRet = nRet;
	m_pSession->AddSendData(SC_CASHSHOP, eCashShop::SC_COUPON, (char*)&CashShop, sizeof(SCCashShopCoupon));
}

#if defined(PRE_ADD_VIP)
void CDNUserSendManager::SendVIPBuy(UINT nCashAmount, int nReserveAmount, int nRet, int nVIPPoint, __time64_t tEndDate, bool bAutoPay, INT64 nSeedPoint)
{
	SCVIPBuy Result;

	Result.nCashAmount = nCashAmount;
	Result.nReserveAmount = nReserveAmount;
	Result.nRet = nRet;
	Result.nVIPPoint = nVIPPoint;
	Result.tVIPExpirationDate = tEndDate;
	Result.bAutoPay = bAutoPay;

	m_pSession->AddSendData(SC_CASHSHOP, eCashShop::SC_VIPBUY, (char*)&Result, sizeof(SCVIPBuy));
}

void CDNUserSendManager::SendVIPGift(UINT nCashAmount, int nReserveAmount, int nRet, INT64 nSeedPoint)
{
	SCVIPGift Result;

	Result.nCashAmount = nCashAmount;
	Result.nReserveAmount = nReserveAmount;
	Result.nRet = nRet;

	m_pSession->AddSendData(SC_CASHSHOP, eCashShop::SC_VIPGIFT, (char*)&Result, sizeof(SCVIPGift));
}

void CDNUserSendManager::SendVIPInfo(int nVIPPoint, __time64_t tEndDate, bool bAutoPay, bool bVIP)
{
	SCVIPInfo Result;

	Result.bVIP = bVIP;
	Result.nVIPPoint = nVIPPoint;
	Result.tVIPExpirationDate = tEndDate;
	Result.bAutoPay = bAutoPay;

	m_pSession->AddSendData(SC_CASHSHOP, eCashShop::SC_VIPINFO, (char*)&Result, sizeof(SCVIPInfo));
}

#endif	// #if defined(PRE_ADD_VIP)

void CDNUserSendManager::SendSaleAbortList(BYTE cCount, int *nAbortList)
{
	SCSaleAbortList Result;
	memset(&Result, 0, sizeof(SCSaleAbortList));

	Result.cCount = cCount;
	memcpy(Result.nAbortList, nAbortList, sizeof(int) * Result.cCount);

	m_pSession->AddSendData(SC_CASHSHOP, eCashShop::SC_SALEABORTLIST, (char*)&Result, sizeof(SCSaleAbortList) - sizeof(nAbortList) +(sizeof(int) * Result.cCount));
}

void CDNUserSendManager::SendCashShopServerTimeForCharge(__time64_t tTime)
{
	SCCashShopChargeTime Result;
	memset(&Result, 0, sizeof(SCCashShopChargeTime));

	Result.tTime = tTime;

	m_pSession->AddSendData(SC_CASHSHOP, eCashShop::SC_CHARGE_TIME, (char*)&Result, sizeof(SCCashShopChargeTime));
}

void CDNUserSendManager::SendSecretCashShopNotify(int nHiddenShopID)
{
	SCSecretCashShopNotify Packet;
	Packet.nHiddenShopID = nHiddenShopID;
	m_pSession->AddSendData(SC_CASHSHOP, eCashShop::SC_SECRETCASHSHOP_NOTIFY, (char*)&Packet, sizeof(SCSecretCashShopNotify));
}

void CDNUserSendManager::SendSecretCashShopOpen(int nRet, int nHiddenShopID)
{
	SCSecretCashShopOpen Packet;
	Packet.nRet = nRet;
	Packet.nHiddenShopID = nHiddenShopID;
	m_pSession->AddSendData(SC_CASHSHOP, eCashShop::SC_SECRETCASHSHOP_OPEN, (char*)&Packet, sizeof(SCSecretCashShopOpen));
}

#ifdef PRE_ADD_LIMITED_CASHITEM
void CDNUserSendManager::SendQuantityLimitedItemList(std::vector<LimitedCashItem::TLimitedQuantityCashItem> &vList)
{
	SCQuantityLimitedItem packet;
	memset(&packet, 0, sizeof(SCQuantityLimitedItem));

	for (int i = 0; i < (int)vList.size(); i++)
	{
		packet.Limited[packet.cCount++] = vList[i];
		if (packet.cCount >= LimitedCashItem::Common::DefaultPacketCount)
		{
			m_pSession->AddSendData(SC_CASHSHOP, eCashShop::SC_LIMITEDCASHITEM, (char*)&packet, sizeof(SCQuantityLimitedItem) - sizeof(packet.Limited) + \
				(sizeof(LimitedCashItem::TLimitedQuantityCashItem) * packet.cCount));
			memset(&packet, 0, sizeof(SCQuantityLimitedItem));
		}
	}

	packet.cIsLast = 1;
	m_pSession->AddSendData(SC_CASHSHOP, eCashShop::SC_LIMITEDCASHITEM, (char*)&packet, sizeof(SCQuantityLimitedItem) - sizeof(packet.Limited) + \
		(sizeof(LimitedCashItem::TLimitedQuantityCashItem) * packet.cCount));
}

void CDNUserSendManager::SendChangedQuantityItemList(std::vector<LimitedCashItem::TChangedLimitedQuantity> &vList)
{
	SCChangedQuantityLimitedItem packet;
	memset(&packet, 0, sizeof(SCChangedQuantityLimitedItem));

	for (int i = 0; i < (int)vList.size(); i++)
	{
		packet.Changed[packet.cCount++] = vList[i];
		if (packet.cCount >= LimitedCashItem::Common::DefaultPacketCount)
			break;
	}

	m_pSession->AddSendData(SC_CASHSHOP, eCashShop::SC_CHANGEDLIMITEDCASHITEM, (char*)&packet, sizeof(SCChangedQuantityLimitedItem) - sizeof(packet.Changed) + \
		(sizeof(LimitedCashItem::TChangedLimitedQuantity) * packet.cCount));
}
#endif		//#ifdef PRE_ADD_LIMITED_CASHITEM

void CDNUserSendManager::SendZeroPopulation(bool bZero)
{
	SCZeroPopulation packet;
	memset(&packet, 0, sizeof(packet));

	packet.bZeroPopulation = bZero;

	m_pSession->AddSendData(SC_SYSTEM, eSystem::SC_ZEROPOPULATION, (char*)&packet, sizeof(packet));
}

void CDNUserSendManager::SendChatRoomCreateChatRoom( int nRet, TChatRoomView & RoomView )
{
	SCCreateChatRoom CreateChatRoom;

	CreateChatRoom.nRet = nRet;
	CreateChatRoom.ChatRoomView.Set( RoomView.m_nChatRoomID, RoomView.m_wszChatRoomName, RoomView.m_nChatRoomType, RoomView.m_bHasPassword,
									 RoomView.m_wszChatRoomPRLine1, RoomView.m_wszChatRoomPRLine2, RoomView.m_wszChatRoomPRLine3 );

	m_pSession->AddSendData( SC_CHATROOM, eChatRoom::SC_CREATECHATROOM, reinterpret_cast<char*>(&CreateChatRoom), sizeof(CreateChatRoom));
}

void CDNUserSendManager::SendChatRoomEnterRoom( int nRet, int nLeaderSID, int * pUserSIDs, USHORT nUserCount )
{
	SCChatRoomEnterRoom EnterRoom;

	EnterRoom.nRet = nRet;
	EnterRoom.LeaderSID = nLeaderSID;
	USHORT nCount = min( nUserCount, CHATROOMMAX );
	for( int i=0; i<nCount; i++ )
		EnterRoom.UserSessionIDs[i] = pUserSIDs[i];

	m_pSession->AddSendData( SC_CHATROOM, eChatRoom::SC_ENTERCHATROOM, reinterpret_cast<char*>(&EnterRoom), sizeof(EnterRoom));
}

void CDNUserSendManager::SendChatRoomEnterUser( UINT nChatRoomID, BYTE cChatRoomType, int nUserSessionID)
{
	SCChatRoomEnterUser EnterUser;

	EnterUser.nChatRoomID = nChatRoomID;
	EnterUser.cChatRoomType = cChatRoomType;
	EnterUser.nUserSessionID = nUserSessionID;

	m_pSession->AddSendData( SC_CHATROOM, eChatRoom::SC_ENTERUSERCHATROOM, reinterpret_cast<char*>(&EnterUser), sizeof(EnterUser));
}

void CDNUserSendManager::SendChatRoomChangeRoomOption( int nRet, TChatRoomView & RoomView )
{
	SCChatRoomChangeRoomOption ChangeRoomOption;
	ChangeRoomOption.nRet = nRet;
	ChangeRoomOption.ChatRoomView.Set( RoomView );

	m_pSession->AddSendData( SC_CHATROOM, eChatRoom::SC_CHANGEROOMOPTION, reinterpret_cast<char*>(&ChangeRoomOption), sizeof(ChangeRoomOption));
}

void CDNUserSendManager::SendChatRoomKickUser( int nRet, int UserSessionID )
{
	SCChatRoomKickUser KickUser;

	KickUser.nRet = nRet; 
	KickUser.nKickedSessionID = UserSessionID;

	m_pSession->AddSendData( SC_CHATROOM, eChatRoom::SC_KICKUSER, reinterpret_cast<char*>(&KickUser), sizeof(KickUser));
}

void CDNUserSendManager::SendChatRoomLeaveUser( int UserSessionID, BYTE nLeaveReason )
{
	SCChatRoomLeaveUser LeaveUser;

	LeaveUser.nUserSessionID	= UserSessionID;
	LeaveUser.nLeaveReason		= nLeaveReason;

	m_pSession->AddSendData( SC_CHATROOM, eChatRoom::SC_LEAVEUSER, reinterpret_cast<char*>(&LeaveUser), sizeof(LeaveUser));
}

void CDNUserSendManager::SendChatRoomView( int SessionID, TChatRoomView & RoomView )
{
	SCChatRoomView ChatRoomView;

	ChatRoomView.nSessionID	= SessionID;
	ChatRoomView.ChatRoomView.Set( RoomView );

	m_pSession->AddSendData( SC_CHATROOM, eChatRoom::SC_CHATROOMVIEW, reinterpret_cast<char*>(&ChatRoomView), sizeof(ChatRoomView));
}

#if defined( PRE_ADD_NPC_REPUTATION_SYSTEM )

void CDNUserSendManager::SendReputationList( std::vector<TNpcReputation>& vData )
{
	SCReputationList TxPacket;

	if( vData.empty() )
	{
		memset( &TxPacket, 0, sizeof(TxPacket) );
		int iSize = sizeof(TxPacket)-sizeof(TxPacket.ReputationArr);
		m_pSession->AddSendData( SC_REPUTATION, eReputation::SC_REPUTATIONLIST, reinterpret_cast<char*>(&TxPacket), iSize );
		return;
	}

	for( UINT i=0 ; i<vData.size() ; i+=_countof(TxPacket.ReputationArr) )
	{
		memset( &TxPacket, 0, sizeof(TxPacket) );

		if( vData.size()-i > _countof(TxPacket.ReputationArr) )
			TxPacket.cCount = _countof(TxPacket.ReputationArr);
		else
			TxPacket.cCount = static_cast<BYTE>(vData.size()-i);

		for( UINT j=0 ; j<TxPacket.cCount ; ++j )
		{
			TxPacket.ReputationArr[j] = vData[i+j];
		}

		int iSize = sizeof(TxPacket)-sizeof(TxPacket.ReputationArr)+(sizeof(TNpcReputation)*TxPacket.cCount);
		m_pSession->AddSendData( SC_REPUTATION, eReputation::SC_REPUTATIONLIST, reinterpret_cast<char*>(&TxPacket), iSize );
	}

}

void CDNUserSendManager::SendModReputation( std::vector<TNpcReputation>& vData )
{
	SCModReputation TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.cCount = static_cast<BYTE>(vData.size());
	for( UINT i=0 ; i<TxPacket.cCount ; ++i )
		TxPacket.UpdateArr[i] = vData[i];

	int iSize = sizeof(TxPacket)-sizeof(TxPacket.UpdateArr)+(sizeof(TxPacket.UpdateArr[0])*TxPacket.cCount);
	m_pSession->AddSendData( SC_REPUTATION, eReputation::SC_MODREPUTATION, reinterpret_cast<char*>(&TxPacket), iSize );

}

#endif // #if defined( PRE_ADD_NPC_REPUTATION_SYSTEM )

void CDNUserSendManager::SendUnionPoint()
{
	SCUnionPoint TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	for (int i=NpcReputation::UnionType::Commercial; i<NpcReputation::UnionType::Etc; i++)
		TxPacket.biUnionPoint[i] = m_pSession->GetUnionReputePointInfoPtr()->GetUnionReputePoint(i);

	m_pSession->AddSendData( SC_REPUTATION, eReputation::SC_UNIONPOINT, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

void CDNUserSendManager::SendUnionMembershipList( CDNUserItem *pItem )
{
	bool bSend = false;

	CTimeSet CurrentTimeSet;
	SCUnionMembershipList Packet;
	memset( &Packet, 0, sizeof(SCUnionMembershipList) );

	for (int i=0; i<NpcReputation::UnionType::Etc; i++)
	{
		TUnionMembership* pMembership = pItem->GetUnionMembership(i);
		if(pMembership->nItemID <=0 ) continue;

		Packet.nItemID[i] = pMembership->nItemID;

		CTimeSet ExpireTimeSet(pMembership->tExpireDate, true);
		Packet.nLeftUseTime[i] = (int)((ExpireTimeSet-CurrentTimeSet)/60);
		bSend = true;
	}

	if(!bSend) return;

	m_pSession->AddSendData( SC_REPUTATION, eReputation::SC_UNIONMEMBERSHIPLIST, (char*)&Packet, sizeof(SCUnionMembershipList) );
}

void CDNUserSendManager::SendBuyUnionMembershipResult( TUnionMembership* pMembership )
{
	SCBuyUnionMembershipResult Packet;
	memset( &Packet, 0, sizeof(SCBuyUnionMembershipResult) );

	Packet.cType = pMembership->cType;
	Packet.nItemID = pMembership->nItemID;

	CTimeSet CurrentTimeSet;
	CTimeSet ExpireTimeSet(pMembership->tExpireDate, true);
	Packet.nLeftUseTime = (int)((ExpireTimeSet-CurrentTimeSet)/60);

	m_pSession->AddSendData( SC_REPUTATION, eReputation::SC_BUYUNIONMEMBERSHIPRESULT, (char*)&Packet, sizeof(SCBuyUnionMembershipResult) );
}

void CDNUserSendManager::SendMasterSystemSimpleInfo( TMasterSystemSimpleInfo& SimpleInfo, bool bRefresh /*= false*/)
{
	MasterSystem::SCSimpleInfo TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );
	
	TxPacket.SimpleInfo = SimpleInfo;
	TxPacket.bRefresh = bRefresh;

	int iSize = sizeof(TxPacket.SimpleInfo)-sizeof(TxPacket.SimpleInfo.OppositeInfo)+TxPacket.SimpleInfo.cCharacterDBIDCount*sizeof(TxPacket.SimpleInfo.OppositeInfo[0]);
	m_pSession->AddSendData( SC_MASTERSYSTEM, eMasterSystem::SC_SIMPLEINFO, reinterpret_cast<char*>(&TxPacket), iSize );

}

void CDNUserSendManager::SendMasterSystemCountInfo( int iMasterCount, int iPupilCount, int iClassmateCount )
{
	MasterSystem::SCCountInfo TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.iMasterCount		= iMasterCount;
	TxPacket.iPupilCount		= iPupilCount;
	TxPacket.iClassmateCount	= iClassmateCount;

	m_pSession->AddSendData( SC_MASTERSYSTEM, eMasterSystem::SC_COUNTINFO, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );

}

void CDNUserSendManager::SendMasterSystemMasterList( int iRet, const std::vector<TMasterInfo>& vMasterList )
{
	MasterSystem::SCMasterList TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.iRet	= iRet;
	TxPacket.cCount = static_cast<BYTE>(vMasterList.size());
	for( UINT i=0 ; i<TxPacket.cCount ; ++i )
		TxPacket.MasterInfoList[i] = vMasterList[i];

	int iSize = sizeof(TxPacket)-sizeof(TxPacket.MasterInfoList)+(TxPacket.cCount*sizeof(TxPacket.MasterInfoList[0]));
	m_pSession->AddSendData( SC_MASTERSYSTEM, eMasterSystem::SC_MASTERLIST, reinterpret_cast<char*>(&TxPacket), iSize );

}

void CDNUserSendManager::SendMasterSystemMasterCharacterInfo( int iRet, TMasterCharacterInfo* pMasterCharacterInfo/*=NULL*/ )
{
	MasterSystem::SCMasterCharacterInfo TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.iRet = iRet;
	if( iRet != ERROR_NONE )
	{
		m_pSession->AddSendData( SC_MASTERSYSTEM, eMasterSystem::SC_MASTERCHARACTERINFO, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket.iRet) );
		return;
	}
	
	if( pMasterCharacterInfo == NULL )
	{
		_DANGER_POINT();
		return;
	}

	TxPacket.MasterCharacterInfo = *pMasterCharacterInfo;
	m_pSession->AddSendData( SC_MASTERSYSTEM, eMasterSystem::SC_MASTERCHARACTERINFO, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );

}

void CDNUserSendManager::SendMasterSystemPupilList( int iRet, INT64 biCharacterDBID, const std::vector<TPupilInfo>& vPupilList )
{
	MasterSystem::SCPupilList TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.iRet					= iRet;
	TxPacket.biMasterCharacterDBID	= biCharacterDBID;
	if( iRet != ERROR_NONE )
	{
		m_pSession->AddSendData( SC_MASTERSYSTEM, eMasterSystem::SC_PUPILLIST, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket.iRet)+sizeof(TxPacket.biMasterCharacterDBID) );
		return;
	}

	TxPacket.cCount = static_cast<BYTE>(vPupilList.size());
	if( TxPacket.cCount > MasterSystem::Max::PupilCount )
	{
		_DANGER_POINT();
		TxPacket.cCount = MasterSystem::Max::PupilCount;
	}

	for( UINT i=0 ; i<TxPacket.cCount ; ++i )
	{
		static_cast<TPupilInfo&>(TxPacket.PupilInfoList[i]) = vPupilList[i];
		TxPacket.PupilInfoList[i].Location	= MasterSystem::CCacheRepository::GetInstance().GetLocationInfo( vPupilList[i].biCharacterDBID );
	}

	int iSize = sizeof(TxPacket)-sizeof(TxPacket.PupilInfoList)+TxPacket.cCount*sizeof(TxPacket.PupilInfoList[0]);
	m_pSession->AddSendData( SC_MASTERSYSTEM, eMasterSystem::SC_PUPILLIST, reinterpret_cast<char*>(&TxPacket), iSize );

}

void CDNUserSendManager::SendMasterSystemIntroductionOnOff( int iRet, bool bRegister, WCHAR* pwszText )

{
	MasterSystem::SCIntroduction TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.iRet		= iRet;
	TxPacket.bRegister	= bRegister;
	if( bRegister && iRet == ERROR_NONE )
		_wcscpy( TxPacket.wszSelfIntroduction, _countof(TxPacket.wszSelfIntroduction), pwszText, (int)wcslen(pwszText) );

	int iSize = (bRegister && iRet == ERROR_NONE) ? sizeof(TxPacket) : sizeof(TxPacket)-sizeof(TxPacket.wszSelfIntroduction);
	m_pSession->AddSendData( SC_MASTERSYSTEM, eMasterSystem::SC_INTRODUCTION_ONOFF, reinterpret_cast<char*>(&TxPacket), iSize );

}

void CDNUserSendManager::SendMasterSystemMasterApplication( int iRet )
{
	MasterSystem::SCMasterApplication TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.iRet = iRet;

	m_pSession->AddSendData( SC_MASTERSYSTEM, eMasterSystem::SC_MASTER_APPLICATION, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );

}

void CDNUserSendManager::SendMasterSystemJoin( int iRet, bool bIsDirectMenu, bool bIsAddPupil, bool bIsTransactor )
{
	MasterSystem::SCJoin TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.iRet			= iRet;
	TxPacket.bIsDirectMenu	= bIsDirectMenu;
	TxPacket.bIsAddPupil	= bIsAddPupil;
	TxPacket.bIsTransactor	= bIsTransactor;

	m_pSession->AddSendData( SC_MASTERSYSTEM, eMasterSystem::SC_JOIN, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );

}

void CDNUserSendManager::SendMasterSystemMasterAndClassmateInfo( int iRet, const std::vector<TMasterAndClassmateInfo>& vList )
{
	MasterSystem::SCMasterClassmate TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.iRet = iRet;
	if( iRet != ERROR_NONE )
	{
		m_pSession->AddSendData( SC_MASTERSYSTEM, eMasterSystem::SC_MASTERANDCLASSMATE, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket.iRet) );
		return;
	}

	TxPacket.cCount = static_cast<BYTE>(vList.size());
	if( TxPacket.cCount > _countof(TxPacket.DataList) )
	{
		_DANGER_POINT();
		TxPacket.cCount = _countof(TxPacket.DataList);
	}

	for( UINT i=0 ; i<TxPacket.cCount ; ++i )
	{
		static_cast<TMasterAndClassmateInfo&>(TxPacket.DataList[i]) = vList[i];
		TxPacket.DataList[i].Location	= MasterSystem::CCacheRepository::GetInstance().GetLocationInfo( vList[i].biCharacterDBID );
	}

	int iSize = sizeof(TxPacket)-sizeof(TxPacket.DataList)+TxPacket.cCount*sizeof(TxPacket.DataList[0]);
	m_pSession->AddSendData( SC_MASTERSYSTEM, eMasterSystem::SC_MASTERANDCLASSMATE, reinterpret_cast<char*>(&TxPacket), iSize );

}

void CDNUserSendManager::SendMasterSystemClassmateInfo( int iRet, TClassmateInfo* pClassmateInfo/*=NULL*/ )
{
#if defined( _VILLAGESERVER )
	MasterSystem::SCClassmateInfo TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	int iSize = sizeof(TxPacket.iRet);

	TxPacket.iRet = iRet;

	if( iRet == ERROR_NONE )
	{
		_ASSERT( pClassmateInfo );
		iSize = sizeof(TxPacket);

		if( pClassmateInfo )
		{
			TxPacket.ClassmateInfo			= *pClassmateInfo;
			TxPacket.ClassmateInfo.Location	= MasterSystem::CCacheRepository::GetInstance().GetLocationInfo( pClassmateInfo->biCharacterDBID );
		}
	}

	m_pSession->AddSendData( SC_MASTERSYSTEM, eMasterSystem::SC_CLASSMATEINFO, reinterpret_cast<char*>(&TxPacket), iSize );

#endif // #if defined( _VILLAGESERVER )
}

void CDNUserSendManager::SendMasterSystemMyMasterInfo( int iRet, TMyMasterInfo* pMyMasterInfo/*=NULL*/ )
{
#if defined( _VILLAGESERVER )
	MasterSystem::SCMyMasterInfo TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	int iSize = sizeof(TxPacket.iRet);

	TxPacket.iRet = iRet;

	if( iRet == ERROR_NONE )
	{
		_ASSERT( pMyMasterInfo );
		iSize = sizeof(TxPacket);

		if( pMyMasterInfo )
		{
			TxPacket.MasterInfo				= *pMyMasterInfo;
			TxPacket.MasterInfo.Location	= MasterSystem::CCacheRepository::GetInstance().GetLocationInfo( pMyMasterInfo->biCharacterDBID );
		}
	}

	m_pSession->AddSendData( SC_MASTERSYSTEM, eMasterSystem::SC_MYMASTERINFO, reinterpret_cast<char*>(&TxPacket), iSize );

#endif // #if defined( _VILLAGESERVER )
}

void CDNUserSendManager::SendMasterSystemLeave( int iRet, bool bIsDelPupil/*=false*/, int iPenaltyRespectPoint/*=0*/ )
{
	MasterSystem::SCLeave TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	int iSize = sizeof(TxPacket.iRet);
	TxPacket.iRet = iRet;

	if( iRet == ERROR_NONE )
	{
		iSize = sizeof(TxPacket);

		TxPacket.bIsDelPupil			= bIsDelPupil;
		TxPacket.iPenaltyRespectPoint	= iPenaltyRespectPoint;
	}

	m_pSession->AddSendData( SC_MASTERSYSTEM, eMasterSystem::SC_LEAVE, reinterpret_cast<char*>(&TxPacket), iSize );

}

void CDNUserSendManager::SendMasterSystemGraduate( WCHAR* pwszCharName )
{
	MasterSystem::SCGraduate TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	_wcscpy( TxPacket.wszCharName, _countof(TxPacket.wszCharName), pwszCharName, (int)wcslen(pwszCharName) );

	m_pSession->AddSendData( SC_MASTERSYSTEM, eMasterSystem::SC_GRADUATE, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );

}

void CDNUserSendManager::SendMasterSystemConnect( bool bIsConnect, WCHAR* pwszCharName )
{
	MasterSystem::SCConnect TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.bIsConnect = bIsConnect;
	_wcscpy( TxPacket.wszCharName, _countof(TxPacket.wszCharName), pwszCharName, (int)wcslen(pwszCharName) );

	m_pSession->AddSendData( SC_MASTERSYSTEM, eMasterSystem::SC_CONNECT, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );

}

void CDNUserSendManager::SendMasterSystemRespectPoint( int iRespectPoint )
{
	MasterSystem::SCRespectPoint TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.iRespectPoint = iRespectPoint;

	m_pSession->AddSendData( SC_MASTERSYSTEM, eMasterSystem::SC_RESPECTPOINT, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );

}

void CDNUserSendManager::SendMasterSystemFavorPoint( INT64 biMasterCharacterDBID, INT64 biPupilCharacterDBID, int iFavorPoint )
{
	MasterSystem::SCFavorPoint TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.biMasterCharacterDBID	= biMasterCharacterDBID;
	TxPacket.biPupilCharacterDBID	= biPupilCharacterDBID;
	TxPacket.iFavorPoint			= iFavorPoint;

	m_pSession->AddSendData( SC_MASTERSYSTEM, eMasterSystem::SC_FAVORPOINT, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );

}

void CDNUserSendManager::SendMasterSystemInvitePupil( int iRet, WCHAR* pwszName )
{
	MasterSystem::SCInvitePupil TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.iRet	= iRet;
	_wcscpy( TxPacket.wszCharName, _countof(TxPacket.wszCharName), pwszName, (int)wcslen(pwszName) );

	m_pSession->AddSendData( SC_MASTERSYSTEM, eMasterSystem::SC_INVITE_PUPIL, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );

}

void CDNUserSendManager::SendMasterSystemInvitePupilConfirm( int iRet, WCHAR* pwszCharName )
{
	MasterSystem::SCInvitePupilConfirm TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.iRet = iRet;
	_wcscpy( TxPacket.wszMasterCharName, _countof(TxPacket.wszMasterCharName), pwszCharName, (int)wcslen(pwszCharName) );

	m_pSession->AddSendData( SC_MASTERSYSTEM, eMasterSystem::SC_INVITE_PUPIL_CONFIRM, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );

}

void CDNUserSendManager::SendMasterSystemJoinDirect( int iRet, WCHAR* pwszCharName )
{
	MasterSystem::SCJoinDirect TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.iRet	= iRet;
	_wcscpy( TxPacket.wszMasterCharName, _countof(TxPacket.wszMasterCharName), pwszCharName, (int)wcslen(pwszCharName) );

	m_pSession->AddSendData( SC_MASTERSYSTEM, eMasterSystem::SC_JOIN_DIRECT, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );

}

void CDNUserSendManager::SendMasterSystemJoinDirectConfirm( int iRet, WCHAR* pwszCharName )
{
	MasterSystem::SCJoinDirectConfirm TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.iRet	= iRet;
	_wcscpy( TxPacket.wszPupilCharName, _countof(TxPacket.wszPupilCharName), pwszCharName, (int)wcslen(pwszCharName) );

	m_pSession->AddSendData( SC_MASTERSYSTEM, eMasterSystem::SC_JOIN_DIRECT_CONFIRM, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );

}

void CDNUserSendManager::SendMasterSystemRecallMaster( int iRet, WCHAR* pwszCharName, bool bIsConfirm )
{
	MasterSystem::SCRecallMaster TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.iRet		= iRet;
	TxPacket.bIsConfirm	= bIsConfirm;
	_wcscpy( TxPacket.wszCharName, _countof(TxPacket.wszCharName), pwszCharName, (int)wcslen(pwszCharName) );

	m_pSession->AddSendData( SC_MASTERSYSTEM, eMasterSystem::SC_RECALL_MASTER, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );

}

void CDNUserSendManager::SendMasterSystemJoinComfirm( BYTE cLevel, BYTE cJob, WCHAR* pwszCharName )
{
	MasterSystem::SCJoinComfirm TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );
	
	TxPacket.cLevel = cLevel;
	TxPacket.cJob = cJob;
	_wcscpy( TxPacket.wszPupilCharName, _countof(TxPacket.wszPupilCharName), pwszCharName, (int)wcslen(pwszCharName) );
	m_pSession->AddSendData( SC_MASTERSYSTEM, eMasterSystem::SC_JOIN_COMFIRM, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

#if defined( PRE_ADD_SECONDARY_SKILL )

void CDNUserSendManager::SendCreateSecondarySkill( int iRet, int iSkillID )
{
	SecondarySkill::SCCreate TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.iRet		= iRet;
	TxPacket.iSkillID	= iSkillID;

	m_pSession->AddSendData( SC_SECONDARYSKILL, eSecondarySkill::SC_CREATE, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );

}

void CDNUserSendManager::SendDelSecondarySkill( int iRet, int iSkillID )
{
	SecondarySkill::SCDelete TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.iRet		= iRet;
	TxPacket.iSkillID	= iSkillID;

	m_pSession->AddSendData( SC_SECONDARYSKILL, eSecondarySkill::SC_DELETE, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );

}

void CDNUserSendManager::SendUpdateSecondarySkillExp( int iRet, int iSkillID, SecondarySkill::Grade::eType Grade, int iExp, int iLevel )
{
	SecondarySkill::SCUpdateExp TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.iRet		= iRet;
	TxPacket.iSkillID	= iSkillID;
	TxPacket.Grade		= Grade;
	TxPacket.iExp		= iExp;
	TxPacket.iLevel		= iLevel;

	m_pSession->AddSendData( SC_SECONDARYSKILL, eSecondarySkill::SC_UPDATE_EXP, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );

}

void CDNUserSendManager::SendAddSecondarySkillRecipe( int iRet, CSecondarySkillRecipe* pRecipe )
{
	SecondarySkill::SCAddRecipe TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.iRet = iRet;
	if( TxPacket.iRet == ERROR_NONE )
	{
		_ASSERT( pRecipe );

		TxPacket.iSkillID		= pRecipe->GetSkillID();
		TxPacket.iItemID		= pRecipe->GetItemID();
		TxPacket.biItemSerial	= pRecipe->GetItemSerial();
		TxPacket.nExp			= pRecipe->GetExp();

		m_pSession->AddSendData( SC_SECONDARYSKILL, eSecondarySkill::SC_ADD_RECIPE, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );

	}
	else
	{
		_ASSERT( pRecipe == NULL );

		m_pSession->AddSendData( SC_SECONDARYSKILL, eSecondarySkill::SC_ADD_RECIPE, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket.iRet) );

	}
}

void CDNUserSendManager::SendDeleteSecondarySkillRecipe( int iRet, int iSkillID, int iItemID )
{
	SecondarySkill::SCDeleteRecipe TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.iRet		= iRet;
	TxPacket.iSkillID	= iSkillID;
	TxPacket.iItemID	= iItemID;

	m_pSession->AddSendData( SC_SECONDARYSKILL, eSecondarySkill::SC_DELETE_RECIPE, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );

}

void CDNUserSendManager::SendExtractSecondarySkillRecipe( int iRet, int iSkillID, int iItemID )
{
	SecondarySkill::SCExtractRecipe TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.iRet		= iRet;
	TxPacket.iSkillID	= iSkillID;
	TxPacket.iItemID	= iItemID;

	m_pSession->AddSendData( SC_SECONDARYSKILL, eSecondarySkill::SC_EXTRACT_RECIPE, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );

}

void CDNUserSendManager::SendUpdateSecondarySkillRecipeExp( int iRet, int iSkillID, int iItemID, int iExp )
{
	SecondarySkill::SCUpdateRecipeExp TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.iRet		= iRet;
	TxPacket.iSkillID	= iSkillID;
	TxPacket.iItemID	= iItemID;
	TxPacket.iExp		= iExp;

	m_pSession->AddSendData( SC_SECONDARYSKILL, eSecondarySkill::SC_UPDATE_RECIPE_EXP, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );

}

void CDNUserSendManager::SendManufactureSecondarySkill( int iRet, bool bIsStart, int iSkillID, int iItemID )
{
	SecondarySkill::SCManufacture TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.iRet		= iRet;
	TxPacket.bIsStart	= bIsStart;
	TxPacket.iSkillID	= iSkillID;
	TxPacket.iItemID	= iItemID;

	m_pSession->AddSendData( SC_SECONDARYSKILL, eSecondarySkill::SC_MANUFACTURE, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );

}

void CDNUserSendManager::SendCancleManufactureSecondarySkill( int iRet, int iSkillID )
{
	SecondarySkill::SCCancelManufacture TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.iRet		= iRet;
	TxPacket.iSkillID	= iSkillID;

	m_pSession->AddSendData( SC_SECONDARYSKILL, eSecondarySkill::SC_CANCEL_MANUFACTURE, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );

}

#endif // #if defined( PRE_ADD_SECONDARY_SKILL )


void CDNUserSendManager::SendFarmPlantSeed( int iRet, Farm::ActionType::eType ActionType, int iAreaIndex, int iItemID, int iAttachItemID )
{
	SCFarmPlant TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.iRet			= iRet;
	TxPacket.ActionType		= ActionType;
	TxPacket.iAreaIndex		= iAreaIndex;
	TxPacket.iItemID		= iItemID;
	TxPacket.iAttachItemID	= iAttachItemID;

	m_pSession->AddSendData( SC_FARM, eFarm::SC_PLANT, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );

}

void CDNUserSendManager::SendFarmHarvest( int iRet, Farm::ActionType::eType ActionType, int iAreaIndex, const std::vector<int>& vResultItem/*=std::vector<int>()*/ )
{
	SCFarmHarvest TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.iRet		= iRet;
	TxPacket.ActionType	= ActionType;
	TxPacket.iAreaIndex	= iAreaIndex;
	for( UINT i=0 ; i<vResultItem.size() ; ++i )
	{
		if( i >= _countof(TxPacket.iResultItemIDs) )
		{
			_ASSERT(0);
			break;
		}
		TxPacket.iResultItemIDs[i] = vResultItem[i];
	}

	int iSize = sizeof(TxPacket)-sizeof(TxPacket.iResultItemIDs);
	if( iRet == ERROR_NONE && ActionType == Farm::ActionType::COMPLETE )
		iSize = sizeof(TxPacket);

	m_pSession->AddSendData( SC_FARM, eFarm::SC_HARVEST, reinterpret_cast<char*>(&TxPacket), iSize );

}

void CDNUserSendManager::SendFarmAddWater( int iRet, Farm::ActionType::eType ActionType, int iAreaIndex )
{
	SCFarmAddWater TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.iRet		= iRet;
	TxPacket.ActionType	= ActionType;
	TxPacket.iAreaIndex	= iAreaIndex;

	m_pSession->AddSendData( SC_FARM, eFarm::SC_ADDWATER, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );

}

void CDNUserSendManager::SendFarmWareHouseList( int iRet, INT64 biLastUniqueID, const std::map<INT64,TFarmWareHouseItem>& mList )
{
	SCFarmWareHouseList TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.iRet = iRet;

	if( iRet == ERROR_NONE )
	{
		for( std::map<INT64,TFarmWareHouseItem>::const_iterator itor=mList.begin() ; itor!=mList.end() ; ++itor )
		{
			if((*itor).second.biUniqueID <= biLastUniqueID  )
				continue;
			TxPacket.Items[TxPacket.cCount] = (*itor).second;
			++TxPacket.cCount;

			if( TxPacket.cCount >= Farm::Max::HARVESTDEPOT_COUNT )
				break;
		}
	}

	int iSize = sizeof(TxPacket)-sizeof(TxPacket.Items)+TxPacket.cCount*sizeof(TxPacket.Items[0]);
	m_pSession->AddSendData( SC_FARM, eFarm::SC_WAREHOUSE_LIST, reinterpret_cast<char*>(&TxPacket), iSize );

}

void CDNUserSendManager::SendFarmTakeWareHouseItem( int iRet, INT64 biUniqueID )
{
	SCFarmTakeWareHouseItem TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.iRet		= iRet;
	TxPacket.biUniqueID	= biUniqueID;

	m_pSession->AddSendData( SC_FARM, eFarm::SC_TAKE_WAREHOUSE_ITEM, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );

}

void CDNUserSendManager::SendFarmFieldCountInfo( int iCount )
{
	SCFarmFieldCountInfo TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.iCount = iCount;

	m_pSession->AddSendData( SC_FARM, eFarm::SC_FIELDCOUNTIFNO, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );

}

void CDNUserSendManager::SendFarmWareHouseItemCount( int iCount )
{
	SCFarmWareHouseItemCount TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.iCount = iCount;

	m_pSession->AddSendData( SC_FARM, eFarm::SC_WAREHOUSE_ITEMCOUNT, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );

}

void CDNUserSendManager::SendFarmAddWaterAnotherUser( WCHAR* pwszCharName )
{
	SCFarmAddWaterAnotherUser TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	_wcscpy( TxPacket.wszCharName, _countof(TxPacket.wszCharName), pwszCharName, (int)wcslen(pwszCharName) );

	m_pSession->AddSendData( SC_FARM, eFarm::SC_ADDWATER_ANOTHERUSER, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );

}

void CDNUserSendManager::SendFishingReadyResult(int nRet)
{
	SCFishingReady packet;
	memset(&packet, 0, sizeof(SCFishingReady));

	packet.nRetCode = nRet;

	m_pSession->AddSendData(SC_FISHING, eFishing::SC_READYFISHING, (char*)&packet, sizeof(SCFishingReady));
}

void CDNUserSendManager::SendFishingCastBaitResult(int nRet)
{
	SCCastBait packet;
	memset(&packet, 0, sizeof(SCCastBait));

	packet.nRetCode = nRet;

	m_pSession->AddSendData(SC_FISHING, eFishing::SC_CASTBAITRESULT, (char*)&packet, sizeof(SCCastBait));
}

void CDNUserSendManager::SendFishingStopFishingResult(int nRet)
{
	SCStopFishing packet;
	memset(&packet, 0, sizeof(SCStopFishing));

	packet.nRetCode = nRet;

	m_pSession->AddSendData(SC_FISHING, eFishing::SC_STOPFISHINGRESULT, (char*)&packet, sizeof(SCStopFishing));
}

#ifdef PRE_ADD_CASHFISHINGITEM
void CDNUserSendManager::SendFishingPattern(int nPatternID, int nFishingAutoMaxTime)
{
	SCFishingPattern packet;
	memset(&packet, 0, sizeof(SCFishingPattern));

	packet.nPatternID = nPatternID;
	packet.nFishingAutoMaxTime = nFishingAutoMaxTime;

	m_pSession->AddSendData(SC_FISHING, eFishing::SC_FISHINGPATTERN, (char*)&packet, sizeof(SCFishingPattern));
}
#else		//#ifdef PRE_ADD_CASHFISHINGITEM
void CDNUserSendManager::SendFishingPattern(int nPatternID)
{
	SCFishingPattern packet;
	memset(&packet, 0, sizeof(SCFishingPattern));

	packet.nPatternID = nPatternID;

	m_pSession->AddSendData(SC_FISHING, eFishing::SC_FISHINGPATTERN, (char*)&packet, sizeof(SCFishingPattern));
}
#endif		//#ifdef PRE_ADD_CASHFISHINGITEM

void CDNUserSendManager::SendFishingEnd()
{
	m_pSession->AddSendData(SC_FISHING, eFishing::SC_FISHINGEND, NULL, 0);
}

//void CDNUserSendManager::SendFishingResult(UINT nSessionID, int nRetCode, int nFishingRewardID)
//{
//	SCFishingReward packet;
//	memset(&packet, 0, sizeof(SCFishingReward));
//
//	packet.nSessionID = nSessionID;
//	packet.nRetCode = nRetCode;
//	packet.nRewardItemID = nFishingRewardID;
//
//	m_pSession->AddSendData(SC_FISHING, eFishing::SC_FISHINGREWARD, (char*)&packet, sizeof(SCFishingPattern));
//}

void CDNUserSendManager::SendFishingSync(int nReduceTimeTick, int nFishingGauge)
{
	SCFishingSync packet;
	memset(&packet, 0, sizeof(SCFishingSync));
	
	packet.nReduceTick = nReduceTimeTick;
	packet.nFishingGauge = nFishingGauge;

	m_pSession->AddSendData(SC_FISHING, eFishing::SC_FISHINGSYNC, (char*)&packet, sizeof(SCFishingSync));
}


void CDNUserSendManager::SendOpenMovieBrowser()
{
	// note by kalliste : Since calling NPC Movie Browser is currently limited to China, decided to set url and type on the client instead of sending over the network.
	m_pSession->AddSendData(SC_NPC, eNpc::SC_OPEN_MOVIEBROWSER, NULL, 0);
}

void CDNUserSendManager::SendOpenBrowser(const char* szUrl, int nX, int nY, int width, int height)
{
	SCOpenBrowser packet;
	memset(&packet, 0, sizeof(SCOpenBrowser));

	_strcpy(packet.url, _countof(packet.url), szUrl, (int)strlen(szUrl));
	packet.nX = nX;
	packet.nY = nY;
	packet.nWidth = width;
	packet.nHeight = height;

	m_pSession->AddSendData(SC_NPC, eNpc::SC_OPEN_BROWSER, (char*)&packet, sizeof(SCOpenBrowser));
}

void CDNUserSendManager::SendStartDragonNest( eDragonNestType Type, int nRebirthCount )
{
	SCStartDragonNest TxPacket;
	TxPacket.Type= Type;
	TxPacket.nRebirthCount = nRebirthCount;
	m_pSession->AddSendData( SC_ROOM, eRoom::SC_START_DRAGONNEST, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

void CDNUserSendManager::SendTimeEventList( TTimeEventGroup *pEvent )
{
	SCTimeEventList Packet;
	memset( &Packet, 0, sizeof(SCTimeEventList) );

	int nCount = 0;
	for( int i=0; i<TIMEEVENTMAX; i++ ) {
		if( pEvent->Event[i].nItemID < 1 ) continue;
		Packet.Event[nCount] = pEvent->Event[i];
		nCount++;
	}
	Packet.nCount = nCount;

	m_pSession->AddSendData( SC_TIMEEVENT, eTimeEvent::SC_TIMEEVENTLIST, (char*)&Packet, sizeof(SCTimeEventList) -((sizeof(TTimeEvent) *(TIMEEVENTMAX-nCount) ) ) );
}

void CDNUserSendManager::SendTimeEventAchieve( int nEventID, INT64 nRemainTime )
{
	SCTimeEventAchieve Packet;
	memset( &Packet, 0, sizeof(SCTimeEventAchieve) );

	Packet.nItemID = nEventID;
	Packet.nRemainTime = nRemainTime;

	m_pSession->AddSendData( SC_TIMEEVENT, eTimeEvent::SC_ACHIEVE_EVENT, (char*)&Packet, sizeof(SCTimeEventAchieve) );
}

void CDNUserSendManager::SendTimeEventExpire( int nEventID )
{
	SCTimeEventExpire Packet;
	memset( &Packet, 0, sizeof(SCTimeEventExpire) );

	Packet.nItemID = nEventID;

	m_pSession->AddSendData( SC_TIMEEVENT, eTimeEvent::SC_EXPIRE_EVENT, (char*)&Packet, sizeof(SCTimeEventExpire) );
}

#ifdef PRE_ADD_CHANGEJOB_CASHITEM
void CDNUserSendManager::SendChangeJobCashItemRes( int nRet, USHORT wTotalSkillPoint, int nChangeFirstJobID, int nChangeSecondJobID )
{
	SCUseChangeJobCashItemRes Packet;
	memset( &Packet, 0, sizeof(SCUseChangeJobCashItemRes) );

	Packet.nRet = nRet;
	Packet.nFirstJobID = nChangeFirstJobID;
	Packet.nSecondJobID = nChangeSecondJobID;
	Packet.nResultSkillPoint = wTotalSkillPoint;

	m_pSession->AddSendData( SC_ITEMGOODS, eItemGoods::SC_USE_CHANGEJOB_CASHITEM_RES, (char*)&Packet, sizeof(SCUseChangeJobCashItemRes) );
}
#endif // #ifdef PRE_ADD_CHANGEJOB_CASHITEM

void CDNUserSendManager::SendCommonVariableList( const std::map<CommonVariable::Type::eCode, CommonVariable::Data>& mList )
{
	SCCommonVariableList TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	for( std::map<CommonVariable::Type::eCode, CommonVariable::Data>::const_iterator itor=mList.begin() ; itor!=mList.end() ; ++itor )
	{
		TxPacket.List[TxPacket.cCount++] = (*itor).second;
	}

	int iSize = sizeof(TxPacket)-sizeof(TxPacket.List)+TxPacket.cCount*sizeof(TxPacket.List[0]);
	m_pSession->AddSendData( SC_CHAR, eChar::SC_COMMONVARIABLE_LIST, reinterpret_cast<char*>(&TxPacket), iSize );

}

void CDNUserSendManager::SendModCommonVariableData( const CommonVariable::Data& Data )
{
	SCModCommonVariable TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.Data = Data;

	m_pSession->AddSendData( SC_CHAR, eChar::SC_MOD_COMMONVARIABLE, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );

}

void CDNUserSendManager::SendCheckLastDungeonInfo( const WCHAR* pwszPartyName )
{
	SCPartyCheckLastDungeonInfo TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );
	
	if( pwszPartyName )
		_wcscpy( TxPacket.wszPartyName, _countof(TxPacket.wszPartyName), pwszPartyName, static_cast<int>(wcslen(pwszPartyName)) );

	m_pSession->AddSendData( SC_PARTY, eParty::SC_CHECK_LASTDUNGEONINFO, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );

}

void CDNUserSendManager::SendConfirmLastDungeonInfo( int iRet )
{
	SCPartyConfirmLastDungeonInfo TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.iRet = iRet;

	m_pSession->AddSendData( SC_PARTY, eParty::SC_CONFIRM_LASTDUNGEONINFO, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );

}

void CDNUserSendManager::SendAttendanceEventResult( bool bCheckAttendanceFirst )
{
	SCAttendanceEvent TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.bCheckAttendanceFirst = bCheckAttendanceFirst;

	m_pSession->AddSendData( SC_CHAR, eChar::SC_ATTENDANCEEVENT, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );

}

void CDNUserSendManager::SendIncreaseLife( char cType, int nIncreaseLife )
{	
	SCIncreaseLife Packet;
	memset( &Packet, 0, sizeof(SCIncreaseLife) );

	Packet.cType = cType;
	Packet.nIncreaseLife = nIncreaseLife;	

	m_pSession->AddSendData( SC_ITEM, eItem::SC_INCREASELIFE, (char*)&Packet, sizeof(SCIncreaseLife) );
}


#if defined(PRE_ADD_REMOVE_PREFIX)
void CDNUserSendManager::SendRemovePrefixItemResult( int nRet, int nSlotIndex )
{
	SCEnchantJewelItem result;
	result.nRet = nRet;
	result.cInvenIndex = nSlotIndex;

	m_pSession->AddSendData(SC_ITEMGOODS, eItemGoods::SC_REMOVE_PREFIX, (char*)&result, sizeof(SCEnchantJewelItem));
}
#endif // PRE_ADD_REMOVE_PREFIX

void CDNUserSendManager::SendSourceData(int nSessonID, const TSourceData& sourceData, bool bUsedInGameServer)
{
	if (sourceData.nItemID <= 0)
		return;

	SCSourceItem packet;
	memset(&packet, 0, sizeof(packet));

	packet.nSessionID = nSessonID;
	packet.nItemID = sourceData.nItemID;
	packet.nRemainTime = sourceData.nRemainTime;
	packet.bUsedInGameServer = bUsedInGameServer;

	m_pSession->AddSendData(SC_ITEM, eItem::SC_USE_SOURCE, (char*)&packet, sizeof(packet));
}

#if defined( PRE_ADD_NAMEDITEM_SYSTEM )
void CDNUserSendManager::SendEffectSkillItemData(int nSessionID, std::vector<TEffectSkillData>& EffectSkillList, bool bUsedInGameServer)
{
	if( EffectSkillList.empty() || EffectSkillList.size() > EFFECTSKILLMAX )
		return;

	SCEffectSkillItem packet;
	memset(&packet, 0, sizeof(packet));	
	packet.nSessionID = nSessionID;
	packet.bUsedInGameServer = bUsedInGameServer;	
#if defined( _VILLAGESERVER )
	for( int i=0;i<(int)(EffectSkillList.size());i++ )
	{		
		packet.EffectSkill[i] = EffectSkillList[i];
		packet.nCount++;		
	} 
#elif defined( _GAMESERVER )
	for( int i=0;i<(int)(EffectSkillList.size());i++ )
	{
		if( EffectSkillList[i].bApplySkill )
		{
			packet.EffectSkill[packet.nCount] = EffectSkillList[i];
			packet.nCount++;
		}		
	} 
#endif
	int nSize = ( sizeof(packet) - sizeof(packet.EffectSkill) ) + ( sizeof(TEffectSkillData) * packet.nCount );
	m_pSession->AddSendData(SC_ITEM, eItem::SC_EFFECTSKILLITEM, (char*)&packet, nSize);
}

void CDNUserSendManager::SendEffectSkillItemData(int nSessionID, TEffectSkillData EffectSkill, bool bUsedInGameServer)
{	
	SCEffectSkillItem packet;
	memset(&packet, 0, sizeof(packet));	
	packet.nSessionID = nSessionID;
	packet.bUsedInGameServer = bUsedInGameServer;
	packet.EffectSkill[0] = EffectSkill;
	packet.nCount++;
	int nSize = ( sizeof(packet) - sizeof(packet.EffectSkill) ) + ( sizeof(TEffectSkillData) * packet.nCount );
	m_pSession->AddSendData(SC_ITEM, eItem::SC_EFFECTSKILLITEM, (char*)&packet, nSize);
}

void CDNUserSendManager::SendDelEffectSkillItemData(int nSessionID, int nItemID)
{	
	SCDelEffectSkillItem packet;
	memset(&packet, 0, sizeof(packet));	
	packet.nSessionID = nSessionID;
	packet.nItemID = nItemID;
	
	m_pSession->AddSendData(SC_ITEM, eItem::SC_DELEFFECTSKILLITEM, (char*)&packet, sizeof(SCDelEffectSkillItem));
}

void CDNUserSendManager::SendShowEffect(int nItemID, int nSessionID, BYTE cEffectType)
{		
	SCShowEffect packet;
	memset(&packet, 0, sizeof(packet));	
	packet.nItemID = nItemID;
	packet.nSessionID = nSessionID;

	if(cEffectType == EffectSkillNameSpace::ShowEffectType::BUFFEFFECT)
		packet.bUseSession = false;		
	else if(cEffectType == EffectSkillNameSpace::ShowEffectType::SPELLEFFECT)
		packet.bUseSession = true;	
	
	m_pSession->AddSendData(SC_ITEM, eItem::SC_SHOWEFFECT, (char*)&packet, sizeof(SCShowEffect));
}

void CDNUserSendManager::SendNamedItemID(int nSessionID, int nItemID)
{		
	SCNamedItemID packet;
	memset(&packet, 0, sizeof(packet));	
	packet.nSessionID = nSessionID;
	packet.nItemID = nItemID;
	
	m_pSession->AddSendData(SC_ITEM, eItem::SC_SENDNAMEDITEMID, (char*)&packet, sizeof(SCNamedItemID));
}
#endif
#if defined(PRE_ADD_TRANSFORM_POTION)
void CDNUserSendManager::SendChangeTransform(int nSessionID, int nTranformID, int nExpireTIme)
{
	SCChangeTransform packet;
	memset(&packet, 0, sizeof(packet));
	packet.nSessionID = nSessionID;
	packet.nTransformID = nTranformID;
	packet.nExpireTime = nExpireTIme;

	m_pSession->AddSendData(SC_ITEM, eItem::SC_CHANGE_TRANSFORM, (char*)&packet, sizeof(SCChangeTransform));
}
#endif

void CDNUserSendManager::SendPetSkillItemResult( int nRet, char cSlotNum, int nSkillID )
{
	SCPetSkillItem result;
	memset(&result, 0, sizeof(result));

	result.nRet = nRet;
	result.cSlotNum = cSlotNum;
	result.nSkillID = nSkillID;

	m_pSession->AddSendData(SC_ITEMGOODS, eItemGoods::SC_PETSKILLITEM, (char*)&result, sizeof(SCPetSkillItem));
}
void CDNUserSendManager::SendPetSkillExpandResult( int nRet )
{
	SCPetSkillExpand result;
	memset(&result, 0, sizeof(result));

	result.nRet = nRet;
	m_pSession->AddSendData(SC_ITEMGOODS, eItemGoods::SC_PETSKILLEXPAND, (char*)&result, sizeof(SCPetSkillExpand));
}
void CDNUserSendManager::SendPetSkillDelete( int nRet, char cSlotNum)
{
	SCPetSkillDelete result;
	memset(&result, 0, sizeof(result));

	result.nRet = nRet;
	result.cSlotNum = cSlotNum;
	m_pSession->AddSendData(SC_ITEM, eItem::SC_DELETE_PET_SKILL, (char*)&result, sizeof(SCPetSkillDelete));
}

void CDNUserSendManager::SendSpecialRebirthItem( int nItemID, int nRebirthCount )
{
	SCSpecialRebirthItem TxPacket;
	TxPacket.nItemID= nItemID;
	TxPacket.nRebirthItemCount = nRebirthCount;
	m_pSession->AddSendData( SC_ROOM, eRoom::SC_SPECIAL_REBIRTHITEM, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

void CDNUserSendManager::SendPlayerCustomEventUI(SCGetPlayerCustomEventUI * pPacket)
{
	if (pPacket == NULL) return;
	m_pSession->AddSendData(SC_CUSTOMEVENTUI, ePlayerCustomEventUI::SC_GETPLAYERUI, reinterpret_cast<char*>(pPacket), sizeof(SCGetPlayerCustomEventUI) - sizeof(pPacket->UIs) + (sizeof(TCustomEventInfo) * pPacket->cCount));
}

void CDNUserSendManager::SendGuildRecruitList( int iRet, UINT uiPage, const std::vector<TGuildRecruitInfo>& vGuildRecruitList )
{
	GuildRecruitSystem::SCGuildRecruitList TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.iRet	= iRet;
	TxPacket.uiPage	= uiPage;
	TxPacket.cCount = static_cast<BYTE>(vGuildRecruitList.size());
	for( UINT i=0 ; i<TxPacket.cCount ; ++i )
		TxPacket.GuildRecruitList[i] = vGuildRecruitList[i];

	int iSize = sizeof(TxPacket)-sizeof(TxPacket.GuildRecruitList)+(TxPacket.cCount*sizeof(TxPacket.GuildRecruitList[0]));
	m_pSession->AddSendData( SC_GUILDRECRUIT, eGuildRecruit::SC_GUILDRECRUIT_LIST, reinterpret_cast<char*>(&TxPacket), iSize );
}

void CDNUserSendManager::SendGuildRecruitCharacter( int iRet, const std::vector<TGuildRecruitCharacter>& vGuildRecruitCharacterList )
{
	GuildRecruitSystem::SCGuildRecruitCharacterList TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.iRet	= iRet;
	TxPacket.cCount = static_cast<BYTE>(vGuildRecruitCharacterList.size());
	for( UINT i=0 ; i<TxPacket.cCount ; ++i )
		TxPacket.GuildRecruitCharacterList[i] = vGuildRecruitCharacterList[i];

	int iSize = sizeof(TxPacket)-sizeof(TxPacket.GuildRecruitCharacterList)+(TxPacket.cCount*sizeof(TxPacket.GuildRecruitCharacterList[0]));
	m_pSession->AddSendData( SC_GUILDRECRUIT, eGuildRecruit::SC_GUILDREQUEST_CHARACTER, reinterpret_cast<char*>(&TxPacket), iSize );
}

void CDNUserSendManager::SendMyGuildRecruit( int iRet, const std::vector<TGuildRecruitInfo>& vMyGuildRecruitList )
{
	GuildRecruitSystem::SCMyGuildRecruitList TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.iRet	= iRet;
	TxPacket.cCount = static_cast<BYTE>(vMyGuildRecruitList.size());
	for( UINT i=0 ; i<TxPacket.cCount ; ++i )
		TxPacket.MyGuildRecruitList[i] = vMyGuildRecruitList[i];

	int iSize = sizeof(TxPacket)-sizeof(TxPacket.MyGuildRecruitList)+(TxPacket.cCount*sizeof(TxPacket.MyGuildRecruitList[0]));
	m_pSession->AddSendData( SC_GUILDRECRUIT, eGuildRecruit::SC_GUILDRECRUIT_MYLIST, reinterpret_cast<char*>(&TxPacket), iSize );
}

void CDNUserSendManager::SendGuildRecruitRequestCount( int iRet, int cRequestCount, GuildRecruitSystem::Max::eType type )
{
	GuildRecruitSystem::SCGuildRecruitRequestCount TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.iRet	= iRet;
	TxPacket.cRequestCount = cRequestCount;
	TxPacket.cMaxRequestCount = type;

	m_pSession->AddSendData( SC_GUILDRECRUIT, eGuildRecruit::SC_GUILDRECRUIT_REQUESTCOUNT, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

void CDNUserSendManager::SendRegisterGuildRecruitList( int iRet, GuildRecruitSystem::RegisterType::eType type )
{
	GuildRecruitSystem::SCGuildRecruitRegister TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.iRet	= iRet;	
	TxPacket.cRegisterType = (BYTE)type;
	
	m_pSession->AddSendData( SC_GUILDRECRUIT, eGuildRecruit::SC_GUILDRECRUIT_REGISTER, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

void CDNUserSendManager::SendGuildRecruitRequest( int iRet, GuildRecruitSystem::RequestType::eType type )
{
	GuildRecruitSystem::SCGuildRecruitRequest TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.iRet	= iRet;	
	TxPacket.cRequestType = type;

	m_pSession->AddSendData( SC_GUILDRECRUIT, eGuildRecruit::SC_GUILDRECRUIT_REQUEST, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

void CDNUserSendManager::SendGuildRecruitAccept( int iRet, INT64 biAcceptCharacterDBID, bool bDelGuildRecruit, GuildRecruitSystem::AcceptType::eType type )
{
	GuildRecruitSystem::SCGuildRecruitAccept TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.iRet	= iRet;	
	TxPacket.cAcceptType = type;
	TxPacket.biAcceptCharacterDBID	= biAcceptCharacterDBID;
	TxPacket.bDelGuildRecruit = bDelGuildRecruit;

	m_pSession->AddSendData( SC_GUILDRECRUIT, eGuildRecruit::SC_GUILDRECRUIT_ACCEPRESULT, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

void CDNUserSendManager::SendGuildRecruitMemberResult( int iRet, WCHAR* wszGuildName, GuildRecruitSystem::AcceptType::eType type )
{
	GuildRecruitSystem::SCGuildRecruitMemberResult TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.iRet	= iRet;	
	TxPacket.cAcceptType = type;	
	_wcscpy( TxPacket.wszGuildName, _countof(TxPacket.wszGuildName), wszGuildName, (int)wcslen(wszGuildName) );

	m_pSession->AddSendData( SC_GUILDRECRUIT, eGuildRecruit::SC_GUILDRECRUIT_MEMBERRESULT, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

#if defined( PRE_ADD_GUILD_EASYSYSTEM )
void CDNUserSendManager::SendRegisterGuildRecruitInfo( BYTE* cClassCode, int nMinLevel, int nMaxLevel, WCHAR* wszGuildIntroduction, BYTE cPurposeCode, bool bCheckHomePage )
#else
void CDNUserSendManager::SendRegisterGuildRecruitInfo( BYTE* cClassCode, int nMinLevel, int nMaxLevel, WCHAR* wszGuildIntroduction )
#endif
{
	GuildRecruitSystem::SCGuildRecruitRegisterInfo TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );
	if(cClassCode) 
		memcpy(TxPacket.cClassGrade, cClassCode, sizeof(TxPacket.cClassGrade));
	TxPacket.nMinLevel = nMinLevel;
	TxPacket.nMaxLevel = nMaxLevel;
	_wcscpy( TxPacket.wszGuildIntroduction, _countof(TxPacket.wszGuildIntroduction), wszGuildIntroduction, (int)wcslen(wszGuildIntroduction) );

#if defined( PRE_ADD_GUILD_EASYSYSTEM )
	TxPacket.cPurposeCode = cPurposeCode;
	TxPacket.bCheckHomePage = bCheckHomePage;	
#endif

	m_pSession->AddSendData( SC_GUILDRECRUIT, eGuildRecruit::SC_GUILDRECRUIT_REGISTERINFO, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

void CDNUserSendManager::SendWarpVillageResult(int nResult)
{
	SCWarpVillage packet;
	ZeroMemory(&packet, sizeof(packet));
	packet.nResult = nResult;

	m_pSession->AddSendData(SC_ITEMGOODS, eItemGoods::SC_WARP_VILLAGE, reinterpret_cast<char*>(&packet), sizeof(packet));
}

void CDNUserSendManager::SendWarpVillageList(const vector<WarpVillage::WarpVillageInfo>& vList)
{
	SCWarpVillageList packet;
	ZeroMemory(&packet, sizeof(packet));
	packet.btCount = static_cast<BYTE>(vList.size());
	copy(vList.begin(), vList.end(), packet.VillageList);

	m_pSession->AddSendData(SC_ITEMGOODS, eItemGoods::SC_WARP_VILLAGE_LIST, reinterpret_cast<char*>(&packet), sizeof(packet) - sizeof(packet.VillageList) + sizeof(WarpVillage::WarpVillageInfo) * packet.btCount);
}

#if defined (PRE_ADD_DONATION)
void CDNUserSendManager::SendOpenDonation()
{
	m_pSession->AddSendData(SC_DONATION, eDonation::SC_DONATION_OPEN, NULL, 0);
}

void CDNUserSendManager::SendDonationResult(int nResult)
{
	SCDonate packet;
	ZeroMemory(&packet, sizeof(packet));
	packet.nResult = nResult;

	m_pSession->AddSendData(SC_DONATION, eDonation::SC_DONATE, reinterpret_cast<char*>(&packet), sizeof(packet));
}

void CDNUserSendManager::SendDonationRanking(const Donation::Ranking vRanking[], int nMyRanking, INT64 nMyCoin)
{
	SCDonationRanking packet;
	ZeroMemory(&packet, sizeof(packet));
	memcpy(packet.Ranking, vRanking, sizeof(Donation::Ranking) * Donation::MAX_RANKING);
	packet.nMyRanking = nMyRanking;
	packet.nMyCoin = nMyCoin;

	m_pSession->AddSendData(SC_DONATION, eDonation::SC_DONATION_RANKING, reinterpret_cast<char*>(&packet), sizeof(packet));
}
#endif // #if defined (PRE_ADD_DONATION)

#ifdef PRE_ADD_AUTOUNPACK
void CDNUserSendManager::SendAutoUnpack(INT64 serial, int nRetCode)
{
	SCAutoUnpack packet;
	memset(&packet, 0, sizeof(SCAutoUnpack));

	packet.nRetCode = nRetCode;
	packet.serial = serial;

	m_pSession->AddSendData(SC_ITEM, eItem::SC_USE_AUTOUNPACK_CASHITEM, (char*)&packet, sizeof(SCAutoUnpack));
}
#endif

#ifdef PRE_ADD_EXCHANGE_POTENTIAL
void CDNUserSendManager::SendExchangePotoential(int nRetCode)
{
	SCExchangePotential packet;
	memset(&packet, 0, sizeof(SCExchangePotential));

	packet.nRetCode = nRetCode;

	m_pSession->AddSendData(SC_ITEMGOODS, eItemGoods::SC_EXCHANGE_POTENTIAL, (char*)&packet, sizeof(SCExchangePotential));
}
#endif		//#ifdef PRE_ADD_EXCHANGE_POTENTIAL

#if defined(PRE_ADD_EXCHANGE_ENCHANT)
void CDNUserSendManager::SendExchangeEnchant(int nRetCode)
{
	SCExchangeEnchant packet;
	memset(&packet, 0, sizeof(SCExchangeEnchant));

	packet.nRetCode = nRetCode;

	m_pSession->AddSendData(SC_ITEMGOODS, eItemGoods::SC_EXCHANGE_ENCHANT, (char*)&packet, sizeof(SCExchangeEnchant));
}
#endif //#if defined(PRE_ADD_EXCHANGE_ENCHANT)

#if defined(PRE_ADD_PCBANG_RENTAL_ITEM)
void CDNUserSendManager::SendPcCafeRentItem(int nRetCode, int nNeedCount)
{
	SCPcCafeRentItem packet;
	memset(&packet, 0, sizeof(SCPcCafeRentItem));

	packet.nRetCode = nRetCode;
	packet.nNeedCount = nNeedCount;

	m_pSession->AddSendData(SC_ITEMGOODS, eItemGoods::SC_PCCAFE_RENT_ITEM, (char*)&packet, sizeof(SCPcCafeRentItem));
}
#endif

#if defined (PRE_ADD_BESTFRIEND)
void CDNUserSendManager::SendGetBestFriend(int nRetCode, TBestFriendInfo& Info)
{
	BestFriend::SCGetInfo packet;
	memset(&packet, 0, sizeof(packet));

	packet.iRet = nRetCode;
	packet.Info = Info;

	m_pSession->AddSendData(SC_BESTFRIEND, eBestFriend::SC_GETINFO, (char*)&packet, sizeof(packet));
}
void CDNUserSendManager::SendSearchBestFriendResult(int nRetCode, char cLevel, char cJob, LPCWSTR lpwszCharacterName)
{
	BestFriend::SCSearch packet;
	memset(&packet, 0, sizeof(packet));

	packet.iRet = nRetCode;
	packet.cLevel = cLevel;
	packet.cJob = cJob;
	
	if(lpwszCharacterName) 
		_wcscpy(packet.wszName, _countof(packet.wszName), lpwszCharacterName, _countof(packet.wszName));

	m_pSession->AddSendData(SC_BESTFRIEND, eBestFriend::SC_SEARCH, (char*)&packet, sizeof(packet));
}

void CDNUserSendManager::SendRegistBestFriendReq(int nRetCode, UINT nFromAccountDBID, INT64 biFromCharacterDBID, LPCWSTR lpwszFromCharacterName, UINT nToAccountDBID, INT64 biToCharacterDBID, LPCWSTR lpwszToCharacterName)
{

	BestFriend::SCRegistReq packet;
	memset(&packet, 0, sizeof(packet));

	packet.iRet = nRetCode;
	packet.nFromAccountDBID = nFromAccountDBID;
	packet.biFromCharacterDBID = biFromCharacterDBID;
	packet.nToAccountDBID = nToAccountDBID;
	packet.biToCharacterDBID = biToCharacterDBID;

	if(lpwszFromCharacterName) 
		_wcscpy(packet.wszFromName, _countof(packet.wszFromName), lpwszFromCharacterName, _countof(packet.wszFromName));

	if(lpwszToCharacterName) 
		_wcscpy(packet.wszToName, _countof(packet.wszToName), lpwszToCharacterName, _countof(packet.wszToName));

	m_pSession->AddSendData(SC_BESTFRIEND, eBestFriend::SC_REGISTREQ, (char*)&packet, sizeof(packet));
}	

void CDNUserSendManager::SendRegistBestFriendAck(int nRetCode, UINT nFromAccountDBID, LPCWSTR lpwszFromCharacterName, UINT nToAccountDBID, LPCWSTR lpwszToCharacterName)
{
	BestFriend::SCRegistAck packet;
	memset(&packet, 0, sizeof(packet));

	packet.iRet = nRetCode;
	packet.nFromAccountDBID = nFromAccountDBID;
	packet.nToAccountDBID = nToAccountDBID;

	if(lpwszFromCharacterName) 
		_wcscpy(packet.wszFromName, _countof(packet.wszFromName), lpwszFromCharacterName, _countof(packet.wszFromName));

	if(lpwszToCharacterName) 
		_wcscpy(packet.wszToName, _countof(packet.wszToName), lpwszToCharacterName, _countof(packet.wszToName));

	m_pSession->AddSendData(SC_BESTFRIEND, eBestFriend::SC_REGISTACK, (char*)&packet, sizeof(packet));
}	

void CDNUserSendManager::SendCompleteBestFriend(int nRetCode, LPCWSTR lpwszCharacterName)
{
	BestFriend::SCComplete packet;
	memset(&packet, 0, sizeof(packet));

	packet.iRet = nRetCode;
	if (lpwszCharacterName)
		_wcscpy(packet.wszName, _countof(packet.wszName), lpwszCharacterName, _countof(packet.wszName));

	m_pSession->AddSendData(SC_BESTFRIEND, eBestFriend::SC_COMPLETE, (char*)&packet, sizeof(packet));
}	

void CDNUserSendManager::SendEditBestFriendMemo(int nRetCode, bool bFromMe, LPCWSTR lpwszMemo)
{
	BestFriend::SCEditMemo packet;
	memset(&packet, 0, sizeof(packet));

	packet.iRet = nRetCode;
	packet.bFromMe = bFromMe;
	
	if(lpwszMemo) 
		_wcscpy(packet.wszMemo, _countof(packet.wszMemo), lpwszMemo, _countof(packet.wszMemo));

	m_pSession->AddSendData(SC_BESTFRIEND, eBestFriend::SC_EDITMEMO, (char*)&packet, sizeof(packet));
}	

void CDNUserSendManager::SendCancelBestFriend(int nRetCode, LPCWSTR lpwszName, bool bCancel)
{
	BestFriend::SCCancel packet;
	memset(&packet, 0, sizeof(packet));

	packet.iRet = nRetCode;
	packet.bCancel = bCancel;
	if (lpwszName)
		_wcscpy(packet.wszName, _countof(packet.wszName), lpwszName, _countof(packet.wszName));

	m_pSession->AddSendData(SC_BESTFRIEND, eBestFriend::SC_CANCELBF, (char*)&packet, sizeof(packet));
}

void CDNUserSendManager::SendCloseBestFriend(int nRetCode, LPCWSTR lpwszName)
{
	BestFriend::SCClose packet;
	memset(&packet, 0, sizeof(packet));

	packet.iRet = nRetCode;
	if (lpwszName)
		_wcscpy(packet.wszName, _countof(packet.wszName), lpwszName, _countof(packet.wszName));

	m_pSession->AddSendData(SC_BESTFRIEND, eBestFriend::SC_CLOSEBF, (char*)&packet, sizeof(packet));
}
#endif

void CDNUserSendManager::SendItemModItemExpireDate(int nRet)
{
	SCModItemExpireDate result;
	memset(&result, 0, sizeof(result));

	result.nRet = nRet;
	m_pSession->AddSendData(SC_ITEM, eItem::SC_MODITEMEXPIREDATE, (char*)&result, sizeof(SCModItemExpireDate));
}

#if defined( PRE_PRIVATECHAT_CHANNEL )
void CDNUserSendManager::SendPrivateChatChannelResult(int nRet)
{
	PrivateChatChannel::SCPrivateChatChannleResult result;
	memset(&result, 0, sizeof(result));

	result.nRet = nRet;
	m_pSession->AddSendData(SC_PRIVATECHAT_CHANNEL, ePrivateChatChannel::SC_PRIVATECHAT_CHANNEL_RESULT, (char*)&result, sizeof(PrivateChatChannel::SCPrivateChatChannleResult));
}

void CDNUserSendManager::SendPrivateChatChannelInfo(TPrivateChatChannelInfo ChannelInfo, std::list<TPrivateChatChannelMember> &MemberList)
{
	if(MemberList.empty())
		return;

	PrivateChatChannel::SCPrivateChannleInfo TxPacket;
	memset(&TxPacket, 0, sizeof(TxPacket));

	_wcscpy(TxPacket.wszChannelName, _countof(TxPacket.wszChannelName), ChannelInfo.wszName, (int)wcslen(ChannelInfo.wszName));

	for(std::list<TPrivateChatChannelMember>::iterator itor = MemberList.begin();itor!=MemberList.end();itor++)
	{
		TxPacket.Members[TxPacket.nCount].biCharacterDBID = itor->biCharacterDBID;
		TxPacket.Members[TxPacket.nCount].bMaster = itor->bMaster;
		_wcscpy(TxPacket.Members[TxPacket.nCount].wszCharacterName, _countof(TxPacket.Members[TxPacket.nCount].wszCharacterName), itor->wszCharacterName, (int)wcslen(itor->wszCharacterName));
		TxPacket.nCount++;
	}
	
	int iSize = sizeof(TxPacket)-sizeof(TxPacket.Members)+(TxPacket.nCount*sizeof(TxPacket.Members[0]));
	m_pSession->AddSendData( SC_PRIVATECHAT_CHANNEL, ePrivateChatChannel::SC_PRIVATECHAT_CHANNEL_INFO, reinterpret_cast<char*>(&TxPacket), iSize );
}

void CDNUserSendManager::SendPrivateChatChannelJoinResult( int nRet, WCHAR* wszChannelName)
{
	PrivateChatChannel::SCPrivateChatChannleJoinResult TxPacket;
	memset(&TxPacket, 0, sizeof(TxPacket));

	TxPacket.nRet = nRet;	
	if(wszChannelName)
		_wcscpy(TxPacket.wszChannelName, _countof(TxPacket.wszChannelName), wszChannelName, (int)wcslen(wszChannelName));

	m_pSession->AddSendData( SC_PRIVATECHAT_CHANNEL, ePrivateChatChannel::SC_PRIVATECHAT_CHANNEL_JOINRESULT, reinterpret_cast<char*>(&TxPacket), sizeof(PrivateChatChannel::SCPrivateChatChannleJoinResult) );
}

void CDNUserSendManager::SendPrivateChatChannelInviteResult( int nRet )
{
	PrivateChatChannel::SCPrivateChatChannleInviteResult TxPacket;
	memset(&TxPacket, 0, sizeof(TxPacket));

	TxPacket.nRet = nRet;	

	m_pSession->AddSendData( SC_PRIVATECHAT_CHANNEL, ePrivateChatChannel::SC_PRIVATECHAT_CHANNEL_INVITERESULT, reinterpret_cast<char*>(&TxPacket), sizeof(PrivateChatChannel::SCPrivateChatChannleOutResult) );
}

void CDNUserSendManager::SendPrivateChatChannelOutResult( int nRet )
{
	PrivateChatChannel::SCPrivateChatChannleOutResult TxPacket;
	memset(&TxPacket, 0, sizeof(TxPacket));

	TxPacket.nRet = nRet;	

	m_pSession->AddSendData( SC_PRIVATECHAT_CHANNEL, ePrivateChatChannel::SC_PRIVATECHAT_CHANNEL_OUTRESULT, reinterpret_cast<char*>(&TxPacket), sizeof(PrivateChatChannel::SCPrivateChatChannleOutResult) );
}

void CDNUserSendManager::SendPrivateChatChannelMemberAdd( int nRet, TPrivateChatChannelMember Member)
{
	PrivateChatChannel::SCPrivateChatChannleAdd TxPacket;
	memset(&TxPacket, 0, sizeof(TxPacket));

	TxPacket.nRet = nRet;
	TxPacket.Member.biCharacterDBID = Member.biCharacterDBID;
	TxPacket.Member.bMaster = Member.bMaster;	
	_wcscpy(TxPacket.Member.wszCharacterName, _countof(TxPacket.Member.wszCharacterName), Member.wszCharacterName, (int)wcslen(Member.wszCharacterName));
		
	m_pSession->AddSendData( SC_PRIVATECHAT_CHANNEL, ePrivateChatChannel::SC_PRIVATECHAT_CHANNEL_ADD, reinterpret_cast<char*>(&TxPacket), sizeof(PrivateChatChannel::SCPrivateChatChannleAdd) );
}

void CDNUserSendManager::SendPrivateChatChannelKickResult(int nRetCode, bool bMe)
{
	PrivateChatChannel::CSPrivateChatChannleKickResult TxPacket;
	memset(&TxPacket, 0, sizeof(TxPacket));

	TxPacket.nRet = nRetCode;
	TxPacket.bMe = bMe;	

	m_pSession->AddSendData( SC_PRIVATECHAT_CHANNEL, ePrivateChatChannel::SC_PRIVATECHAT_CHANNEL_KICKRESULT, reinterpret_cast<char*>(&TxPacket), sizeof(PrivateChatChannel::CSPrivateChatChannleKickResult) );
}

void CDNUserSendManager::SendPrivateChatChannelMemberDel( int nRet, INT64 biCharacterDBID, WCHAR* wszName)
{
	PrivateChatChannel::SCPrivateChatChannleDel TxPacket;
	memset(&TxPacket, 0, sizeof(TxPacket));

	TxPacket.nRet = nRet;
	TxPacket.biCharacterDBID = biCharacterDBID;	
	if(wszName)
		_wcscpy(TxPacket.wszCharacterName, _countof(TxPacket.wszCharacterName), wszName, (int)wcslen(wszName));

	m_pSession->AddSendData( SC_PRIVATECHAT_CHANNEL, ePrivateChatChannel::SC_PRIVATECHAT_CHANNEL_DEL, reinterpret_cast<char*>(&TxPacket), sizeof(PrivateChatChannel::SCPrivateChatChannleDel) );
}

void CDNUserSendManager::SendPrivateChatChannelMemberKick( int nRet, INT64 biCharacterDBID, WCHAR* wszName)
{
	PrivateChatChannel::SCPrivateChatChannleDel TxPacket;
	memset(&TxPacket, 0, sizeof(TxPacket));

	TxPacket.nRet = nRet;
	TxPacket.biCharacterDBID = biCharacterDBID;
	if(wszName)
		_wcscpy(TxPacket.wszCharacterName, _countof(TxPacket.wszCharacterName), wszName, (int)wcslen(wszName));

	m_pSession->AddSendData( SC_PRIVATECHAT_CHANNEL, ePrivateChatChannel::SC_PRIVATECHAT_CHANNEL_KICK, reinterpret_cast<char*>(&TxPacket), sizeof(PrivateChatChannel::SCPrivateChatChannleDel) );
}

void CDNUserSendManager::SendPrivateChatChannelInfoMod( int nRet, int nModType, WCHAR* wszMasterName, INT64 biCharacterDBID )
{
	PrivateChatChannel::SCPrivateChatChannleMod TxPacket;
	memset(&TxPacket, 0, sizeof(TxPacket));

	TxPacket.nRet = nRet;
	TxPacket.biCharacterDBID = biCharacterDBID;
	TxPacket.nModType = nModType;
	
	if(wszMasterName)
		_wcscpy(TxPacket.wszCharacterName, _countof(TxPacket.wszCharacterName), wszMasterName, (int)wcslen(wszMasterName));

	m_pSession->AddSendData( SC_PRIVATECHAT_CHANNEL, ePrivateChatChannel::SC_PRIVATECHAT_CHANNEL_MOD, reinterpret_cast<char*>(&TxPacket), sizeof(PrivateChatChannel::SCPrivateChatChannleMod) );
}
#endif

#if defined( PRE_ADD_TOTAL_LEVEL_SKILL )
void CDNUserSendManager::SendTotalLevel(UINT nSessionID, int nTotalLevelSkill)
{
	SCActorMessage ActorMsg;
	memset(&ActorMsg, 0, sizeof(SCActorMessage));

	CPacketCompressStream Stream( ActorMsg.cBuf, 128 );

	Stream.Write( &nTotalLevelSkill, sizeof(int) );

	ActorMsg.nSessionID = nSessionID;
	int nSize = (USHORT)Stream.Tell();

	m_pSession->AddSendData(SC_ACTOR, eActor::SC_TOTAL_LEVEL, (char*)&ActorMsg, sizeof(SCActorMessage) -(USHORT)(128 - nSize));
}
void CDNUserSendManager::SendTotalLevelSkillList(UINT nSessionID, int* nTotalLevelSkill)
{
	SCActorMessage ActorMsg;
	memset(&ActorMsg, 0, sizeof(SCActorMessage));

	CPacketCompressStream Stream( ActorMsg.cBuf, 128 );

	int nCount = 4;
	Stream.Write( &nCount, sizeof(int) );
	for(int i=0;i<TotalLevelSkill::Common::MAXSLOTCOUNT;i++)
	{
		Stream.Write( &i, sizeof(int) );
		Stream.Write( &nTotalLevelSkill[i], sizeof(int) );		
	}

	ActorMsg.nSessionID = nSessionID;
	int nSize = (USHORT)Stream.Tell();

	m_pSession->AddSendData(SC_ACTOR, eActor::SC_TOTAL_LEVEL_SKILL_ACTIVE_LIST, (char*)&ActorMsg, sizeof(SCActorMessage) -(USHORT)(128 - nSize));
}
void CDNUserSendManager::SendTotalLevelSkillCashSlot(UINT nSessionID, int nCashSlotIndex, bool bFlag, __time64_t tExpireDate)
{
	SCActorMessage ActorMsg;
	memset(&ActorMsg, 0, sizeof(SCActorMessage));

	CPacketCompressStream Stream( ActorMsg.cBuf, 128 );	
	Stream.Write( &nCashSlotIndex, sizeof(int) );
	Stream.Write(&bFlag, sizeof(bool));
	Stream.Write(&tExpireDate, sizeof(__time64_t));	

	ActorMsg.nSessionID = nSessionID;
	int nSize = (USHORT)Stream.Tell();

	m_pSession->AddSendData(SC_ACTOR, eActor::SC_TOTAL_LEVEL_SKILL_CASHSLOT_ACTIVATE, (char*)&ActorMsg, sizeof(SCActorMessage) -(USHORT)(128 - nSize));
}
void CDNUserSendManager::SendAddTotalLevelSkill(UINT nSessionID, int nSlotIndex, int nSkillID, bool isInitialize/* = false*/)
{
	SCActorMessage ActorMsg;
	memset(&ActorMsg, 0, sizeof(SCActorMessage));

	CPacketCompressStream Stream( ActorMsg.cBuf, 128 );

	Stream.Write( &nSlotIndex, sizeof(int) );
	Stream.Write( &nSkillID, sizeof(int) );	
	Stream.Write( &isInitialize, sizeof(bool) );

	ActorMsg.nSessionID = nSessionID;
	int nSize = (USHORT)Stream.Tell();

	m_pSession->AddSendData(SC_ACTOR, eActor::SC_ADD_TOTAL_LEVEL_SKILL, (char*)&ActorMsg, sizeof(SCActorMessage) -(USHORT)(128 - nSize));
}
void CDNUserSendManager::SendDelTotalLevelSkill(UINT nSessionID, int nSlotIndex)
{
	SCActorMessage ActorMsg;
	memset(&ActorMsg, 0, sizeof(SCActorMessage));

	CPacketCompressStream Stream( ActorMsg.cBuf, 128 );
	
	Stream.Write( &nSlotIndex, sizeof(int) );

	ActorMsg.nSessionID = nSessionID;
	int nSize = (USHORT)Stream.Tell();

	m_pSession->AddSendData(SC_ACTOR, eActor::SC_REMOVE_TOTAL_LEVEL_SKILL, (char*)&ActorMsg, sizeof(SCActorMessage) -(USHORT)(128 - nSize));
}
#endif
#ifdef PRE_ADD_PRESET_SKILLTREE
void CDNUserSendManager::SendSkillPresetList(TSkillSetIndexData * pSetData, TSkillSetPartialData * pSkillData, int nRetCode)
{
	SCSKillSetList packet;
	memset(&packet, 0, sizeof(SCSKillSetList));

	int nCount = 0;
	packet.nRetCode = nRetCode;
	if (pSetData != NULL && pSkillData != NULL)
	{
		memcpy_s(&packet.SKillIndex, sizeof(packet.SKillIndex), pSetData, sizeof(TSkillSetIndexData) * SKILLPRESETMAX);
		for (int i = 0; i < SKILLPRESETMAX; i++)
		{
			nCount += packet.SKillIndex[i].cCount;
		}
		memcpy_s(&packet.SKills, sizeof(packet.SKills), pSkillData, sizeof(TSkillSetPartialData) * nCount);
	}

	m_pSession->AddSendData(SC_SKILL, eSkill::SC_LIST_SKILLSET, (char*)&packet, sizeof(SCSKillSetList) - sizeof(packet.SKills) + (sizeof(TSkillSetPartialData) * nCount));
}

void CDNUserSendManager::SendSkillPresetAddResult(int nRetCode)
{
	SCSKillSaveResult packet;
	memset(&packet, 0, sizeof(SCSKillSaveResult));

	packet.nRetCode = nRetCode;

	m_pSession->AddSendData(SC_SKILL, eSkill::SC_SAVE_SKILLSET, (char*)&packet, sizeof(SCSKillSaveResult));
}

void CDNUserSendManager::SendSkillPresetDelResult(int nRetCode)
{
	SCSKillDeleteResult packet;
	memset(&packet, 0, sizeof(SCSKillDeleteResult));

	packet.nRetCode = nRetCode;

	m_pSession->AddSendData(SC_SKILL, eSkill::SC_DELETE_SKILLSET, (char*)&packet, sizeof(SCSKillDeleteResult));
}
#endif		//#ifdef PRE_ADD_PRESET_SKILLTREE

#if defined( PRE_WORLDCOMBINE_PVP )
void CDNUserSendManager::SendWorldPvPRoomStartMsg( bool bShow, UINT	uiPvPIndex )
{
	WorldPvPMissionRoom::SCWorldPvPRoomStartMsg packet;
	memset(&packet, 0, sizeof(WorldPvPMissionRoom::SCWorldPvPRoomStartMsg));

	packet.bShow = bShow;
	packet.uiPvPIndex = uiPvPIndex;

	m_pSession->AddSendData(SC_PVP, ePvP::SC_PVP_WORLDPVPROOM_STARTMSG, (char*)&packet, sizeof(WorldPvPMissionRoom::SCWorldPvPRoomStartMsg));
}

void CDNUserSendManager::SendWorldPvPRoomStartResult( int nRet )
{
	WorldPvPMissionRoom::SCWorldPvPRoomStartReturn packet;
	memset(&packet, 0, sizeof(WorldPvPMissionRoom::SCWorldPvPRoomStartReturn));

	packet.nRet = nRet;

	m_pSession->AddSendData(SC_PVP, ePvP::SC_PVP_WORLDPVPROOM_START_RESULT, (char*)&packet, sizeof(WorldPvPMissionRoom::SCWorldPvPRoomStartReturn));
}

void CDNUserSendManager::SendWorldPvPRoomJoinResult( int nRet )
{
	WorldPvPMissionRoom::SCWorldPvPRoomJoinResult packet;
	memset(&packet, 0, sizeof(WorldPvPMissionRoom::SCWorldPvPRoomJoinResult));

	packet.nRet = nRet;

	m_pSession->AddSendData(SC_PVP, ePvP::SC_PVP_WORLDPVPROOM_JOIN_RESULT, (char*)&packet, sizeof(WorldPvPMissionRoom::SCWorldPvPRoomJoinResult));
}

void CDNUserSendManager::SendWorldPvPRoomAllKillTeamInfo( UINT* nSessionID, int* nTeam )
{
	WorldPvPMissionRoom::SCWorldPvPRoomAllKillTeamInfo packet;
	memset(&packet, 0, sizeof(WorldPvPMissionRoom::SCWorldPvPRoomAllKillTeamInfo));
	
	for( int i=0;i<PARTYMAX;i++	)
	{
		if( nSessionID[i] > 0 )
		{
			packet.nSessionID[i] = nSessionID[i];
			packet.nTeam[i] = nTeam[i];
		}
		else
			break;
	}

	m_pSession->AddSendData(SC_PVP, ePvP::SC_PVP_WORLDPVPROOM_ALLKILLTEAMINFO, (char*)&packet, sizeof(WorldPvPMissionRoom::SCWorldPvPRoomAllKillTeamInfo));
}

void CDNUserSendManager::SendWorldPvPRoomTournamentUserInfo( BYTE cUserCount, PvPCommon::UserInfoList *TournamentUserInfo, int nSize )
{
	WorldPvPMissionRoom::SCWorldPvPRoomTournamentUserInfo packet;
	memset(&packet, 0, sizeof(WorldPvPMissionRoom::SCWorldPvPRoomTournamentUserInfo));

	packet.cUserCount = cUserCount;
	memcpy(&packet.UserInfoList, TournamentUserInfo, sizeof(PvPCommon::UserInfoList) );

	nSize += sizeof(packet.cUserCount);

	m_pSession->AddSendData(SC_PVP, ePvP::SC_PVP_WORLDPVPROOM_TOURNAMENTUSERINFO, (char*)&packet, nSize);	
}

#endif

#if defined( PRE_ADD_PVP_COMBOEXERCISE )
void CDNUserSendManager::SendPvPComboExerciseRoomMasterInfo( UINT uiRoomMasterSessionID )
{
	SCPvPComboExerciseRoomMasterInfo packet;
	memset(&packet, 0, sizeof(SCPvPComboExerciseRoomMasterInfo));

	packet.uiRoomMasterSessionID = uiRoomMasterSessionID;

	m_pSession->AddSendData(SC_PVP, ePvP::SC_PVP_COMBOEXERCISE_ROOMMASTERINFO, (char*)&packet, sizeof(SCPvPComboExerciseRoomMasterInfo));
}

void CDNUserSendManager::SendPvPComboExerciseChangeRoomMaster( UINT uiRoomMasterSessionID )
{
	SCPvPComboExerciseChangeRoomMaster packet;
	memset(&packet, 0, sizeof(SCPvPComboExerciseChangeRoomMaster));

	packet.uiRoomMasterSessionID = uiRoomMasterSessionID;

	m_pSession->AddSendData(SC_PVP, ePvP::SC_PVP_COMBOEXERCISE_CHANGEROOMMASTER, (char*)&packet, sizeof(SCPvPComboExerciseChangeRoomMaster));
}

void CDNUserSendManager::SendPvPComboExerciseRecallMonsterResult( int nRetCode )
{
	SCPvPComboExerciseRecallMonsterResult packet;
	memset(&packet, 0, sizeof(SCPvPComboExerciseRecallMonsterResult));

	packet.nRet = nRetCode;

	m_pSession->AddSendData(SC_PVP, ePvP::SC_PVP_COMBOEXERCISE_RECALLMONSTERRESULT, (char*)&packet, sizeof(SCPvPComboExerciseRecallMonsterResult));
}
#endif // #if defined( PRE_ADD_PVP_COMBOEXERCISE )

#if defined( PRE_ALTEIAWORLD_EXPLORE )

void CDNUserSendManager::SendAlteiaWorldJoinInfo( BYTE cWeeklyClearCount, BYTE cDailyClearCount, BYTE cTicketCount )
{
	AlteiaWorld::SCAlteiaWorldJoinInfo packet;
	memset(&packet, 0, sizeof(AlteiaWorld::SCAlteiaWorldJoinInfo));

	packet.cWeeklyClearCount = cWeeklyClearCount;
	packet.cDailyClearCount = cDailyClearCount;
	packet.cTicketCount = cTicketCount;

	m_pSession->AddSendData(SC_ALTEIAWORLD, eAlteiaWorld::SC_ALTEIAWORLD_JOIN_INFO, (char*)&packet, sizeof(AlteiaWorld::SCAlteiaWorldJoinInfo));
}

void CDNUserSendManager::SendAlteiaWorldJoinResult( int nRetCode )
{
	AlteiaWorld::SCAlteiaWorldJoinResult packet;
	memset(&packet, 0, sizeof(AlteiaWorld::SCAlteiaWorldJoinResult));

	packet.nRet = nRetCode;

	m_pSession->AddSendData(SC_ALTEIAWORLD, eAlteiaWorld::SC_ALTEIAWORLD_JOIN_RESULT, (char*)&packet, sizeof(AlteiaWorld::SCAlteiaWorldJoinResult));
}

void CDNUserSendManager::SendAlteiaWorldSendTicket( const WCHAR* wszSendName, int nTicketCount/*=0*/ )
{
	AlteiaWorld::SCAlteiaWorldSendTicket packet;
	memset(&packet, 0, sizeof(AlteiaWorld::SCAlteiaWorldSendTicket));
	
	_wcscpy( packet.wszCharacterName, _countof(packet.wszCharacterName), wszSendName, (int)wcslen(wszSendName));
	packet.nTicketCount = nTicketCount;
	
	m_pSession->AddSendData(SC_ALTEIAWORLD, eAlteiaWorld::SC_ALTEIAWORLD_SENDTICKET, (char*)&packet, sizeof(AlteiaWorld::SCAlteiaWorldSendTicket));
}

void CDNUserSendManager::SendAlteiaWorldSendTicketResult( int nRetCode, int nSendTicketCount )
{
	AlteiaWorld::SCAlteiaWorldSendTicketResult packet;
	memset(&packet, 0, sizeof(AlteiaWorld::SCAlteiaWorldSendTicketResult));

	packet.nRet = nRetCode;
	packet.nSendTicketCount = nSendTicketCount;

	m_pSession->AddSendData(SC_ALTEIAWORLD, eAlteiaWorld::SC_ALTEIAWORLD_SENDTICKET_RESULT, (char*)&packet, sizeof(AlteiaWorld::SCAlteiaWorldSendTicketResult));
}

void CDNUserSendManager::SendAlteiaWorldDiceResult( int nRetCode, int nCount/*=0*/ )
{
	AlteiaWorld::SCAlteiaWorldDiceResult packet;
	memset(&packet, 0, sizeof(AlteiaWorld::SCAlteiaWorldDiceResult));

	packet.nRet = nRetCode;
	packet.nNumber = nCount;

	m_pSession->AddSendData(SC_ALTEIAWORLD, eAlteiaWorld::SC_ALTEIAWORLD_DICE_RESULT, (char*)&packet, sizeof(AlteiaWorld::SCAlteiaWorldDiceResult));
}

#if defined( _VILLAGESERVER )
void CDNUserSendManager::SendAlteiaWorldPrivateGoldKeyRank( const TAGetAlteiaWorldPrivateGoldKeyRank *pPacket )
{
	AlteiaWorld::SCAlteiaWorldPrivateGoldKeyRankingInfo packet;
	memset(&packet, 0, sizeof(AlteiaWorld::SCAlteiaWorldPrivateGoldKeyRankingInfo));

	packet.nMyBestGoldKeyCount = m_pSession->GetAlteiaBestGoldKeyCount();
	packet.nCount = pPacket->nCount;
	memcpy(&packet.MemberInfo, pPacket->RankMemberInfo, sizeof(packet.MemberInfo) );

	m_pSession->AddSendData(SC_ALTEIAWORLD, eAlteiaWorld::SC_ALTEIAWORLD_PRIVATEGOLDKEYRANKING_INFO, (char*)&packet, sizeof(AlteiaWorld::SCAlteiaWorldPrivateGoldKeyRankingInfo));
}

void CDNUserSendManager::SendAlteiaWorldPrivatePlayTimeRank( const TAGetAlteiaWorldPrivatePlayTimeRank *pPacket )
{
	AlteiaWorld::SCAlteiaWorldPrivatePlayTimeRankingInfo packet;
	memset(&packet, 0, sizeof(AlteiaWorld::SCAlteiaWorldPrivatePlayTimeRankingInfo));

	packet.nPlaySec = m_pSession->GetAlteiaBestPlayTime();
	packet.nCount = pPacket->nCount;
	memcpy(&packet.MemberInfo, pPacket->RankMemberInfo, sizeof(packet.MemberInfo) );

	m_pSession->AddSendData(SC_ALTEIAWORLD, eAlteiaWorld::SC_ALTEIAWORLD_PRIVATEPLAYTIMERANKING_INFO, (char*)&packet, sizeof(AlteiaWorld::SCAlteiaWorldPrivatePlayTimeRankingInfo));
}

void CDNUserSendManager::SendAlteiaWorldGuildGoldKeyRank( const TAGetAlteiaWorldGuildGoldKeyRank *pPacket )
{
	AlteiaWorld::SCAlteiaWorldGuildGoldKeyRankingInfo packet;
	memset(&packet, 0, sizeof(AlteiaWorld::SCAlteiaWorldGuildGoldKeyRankingInfo));

	packet.nMyBestGoldKeyCount = m_pSession->GetAlteiaBestGoldKeyCount();
	packet.nCount = pPacket->nCount;
	memcpy(&packet.GuildRankInfo, pPacket->RankMemberInfo, sizeof(packet.GuildRankInfo) );

	m_pSession->AddSendData(SC_ALTEIAWORLD, eAlteiaWorld::SC_ALTEIAWORLD_GUILDGOLDKEYRANKING_INFO, (char*)&packet, sizeof(AlteiaWorld::SCAlteiaWorldGuildGoldKeyRankingInfo));
}

void CDNUserSendManager::SendAlteiaWorldSendTicketInfo( INT64* biCharacterDBID, BYTE cTicketCount )
{
	AlteiaWorld::SCAlteiaWorldSendTicketInfo packet;
	memset(&packet, 0, sizeof(AlteiaWorld::SCAlteiaWorldSendTicketInfo));

	memcpy(&packet.biCharacterDBID, biCharacterDBID, sizeof(packet.biCharacterDBID) );
	packet.cSendTicketCount = cTicketCount;

	m_pSession->AddSendData(SC_ALTEIAWORLD, eAlteiaWorld::SC_ALTEIAWORLD_SENDTICKET_INFO, (char*)&packet, sizeof(AlteiaWorld::SCAlteiaWorldSendTicketInfo));
}
#endif

#endif // #if defined( PRE_ALTEIAWORLD_EXPLORE )

#if defined(PRE_ADD_GAMEQUIT_REWARD)
void CDNUserSendManager::SendGameQuitRewardType(GameQuitReward::RewardType::eType eRewardType)
{
	SCGameQuitRewardCheckRes packet;
	memset(&packet, 0, sizeof(SCGameQuitRewardCheckRes));

	packet.eRewardType = eRewardType;

	m_pSession->AddSendData(SC_ETC, eEtc::SC_GAMEQUIT_REWARDCHECK_RES, (char*)&packet, sizeof(SCGameQuitRewardCheckRes));
}

void CDNUserSendManager::SendGameQuitRewardResult(int nRet)
{
	SCGameQuitRewardRes packet;
	memset(&packet, 0, sizeof(SCGameQuitRewardRes));

	packet.nRet = nRet;

	m_pSession->AddSendData(SC_ETC, eEtc::SC_GAMEQUIT_REWARD_RES, (char*)&packet, sizeof(SCGameQuitRewardRes));
}
#endif	// #if defined(PRE_ADD_GAMEQUIT_REWARD)

#if defined( PRE_ADD_STAMPSYSTEM )
void CDNUserSendManager::SendStampSystemInit(int nWeekDay, INT64 biTableStartTime, bool * pCompleteFlagData)
{
	using namespace StampSystem;

	SCStampSystemInit packet;
	memset(&packet, 0, sizeof(SCStampSystemInit));

	packet.nWeekDay = nWeekDay;
	packet.biTableStartTime = biTableStartTime;
	memcpy(packet.bCompleteFlagData, pCompleteFlagData, sizeof(bool) * Common::MaxStampSlotCount);

	m_pSession->AddSendData(SC_STAMPSYSTEM, eStampSystem::SC_STAMPSYSTEM_INIT, (char*)&packet, sizeof(SCStampSystemInit));
}

void CDNUserSendManager::SendStampSystemAddComplete(int nChallengeIndex, int nWeekDay)
{
	using namespace StampSystem;

	SCStampSystemAddComplete packet;
	memset(&packet, 0, sizeof(SCStampSystemAddComplete));

	packet.nChallengeIndex = nChallengeIndex;
	packet.nWeekDay = nWeekDay;

	m_pSession->AddSendData(SC_STAMPSYSTEM, eStampSystem::SC_STAMPSYSTEM_ADD_COMPLETE, (char*)&packet, sizeof(SCStampSystemAddComplete));
}

void CDNUserSendManager::SendStampSystemClear()
{
	using namespace StampSystem;

	m_pSession->AddSendData(SC_STAMPSYSTEM, eStampSystem::SC_STAMPSYSTEM_CLEAR_COMPLETE, NULL, 0);
}

void CDNUserSendManager::SendStampSystemChangeWeekDay(int nWeekDay)
{
	using namespace StampSystem;

	SCStampSystemChangeWeekDay packet;
	memset(&packet, 0, sizeof(SCStampSystemChangeWeekDay));

	packet.nWeekDay = nWeekDay;

	m_pSession->AddSendData(SC_STAMPSYSTEM, eStampSystem::SC_STAMPSYSTEM_CHANGE_WEEKDAY, (char*)&packet, sizeof(SCStampSystemChangeWeekDay));
}

void CDNUserSendManager::SendStampSystemChangeTable(INT64 biTableStartTime)
{
	using namespace StampSystem;

	SCStampSystemChangeTable packet;
	memset(&packet, 0, sizeof(SCStampSystemChangeTable));

	packet.biTableStartTime = biTableStartTime;

	m_pSession->AddSendData(SC_STAMPSYSTEM, eStampSystem::SC_STAMPSYSTEM_CHANGE_TABLE, (char*)&packet, sizeof(SCStampSystemChangeTable));
}
#endif // #if defined( PRE_ADD_STAMPSYSTEM )

#if defined( PRE_ADD_PVP_EXPUP_ITEM )
void CDNUserSendManager::SendUsePvPExpupItem(int nRet, BYTE cLevel, UINT uiXP, int nIncrement) 
{
	SCUsePvPExpup packet;
	memset(&packet, 0, sizeof(SCUsePvPExpup));

	packet.nRet = nRet;
	packet.cLevel = cLevel;
	packet.uiXP = uiXP;
	packet.nIncrement = nIncrement;

	m_pSession->AddSendData(SC_ITEM, eItem::SC_USE_PVPEXPUP, (char*)&packet, sizeof(SCUsePvPExpup));
}
#endif // #if defined( PRE_ADD_PVP_EXPUP_ITEM )


#if defined( PRE_DRAGONBUFF )
void CDNUserSendManager::SendWorldBuffMsg( WCHAR* wszCharacterName, int nItemID )
{
	SCWorldBuffMsg packet;
	memset(&packet, 0, sizeof(SCWorldBuffMsg));

	if( wszCharacterName )
	_wcscpy( packet.wszCharacterName, _countof(packet.wszCharacterName), wszCharacterName, (int)wcslen(wszCharacterName));	
	
	packet.nItemID = nItemID;
	
	m_pSession->AddSendData( SC_ETC, eEtc::SC_WORLDBUFFMSG, (char*)&packet, sizeof(SCWorldBuffMsg));
}
#endif
#if defined( PRE_ADD_GUILD_CONTRIBUTION )
void CDNUserSendManager::SendGuildContributionPoint( int nWeeklyContributionPoint, int nTotalContributionPoint )
{
	GuildContribution::SCGuildContributionPoint packet;
	memset(&packet, 0, sizeof(GuildContribution::SCGuildContributionPoint));

	packet.nWeeklyContributionPoint = nWeeklyContributionPoint;
	packet.nTotalContributionPoint = nTotalContributionPoint;

	m_pSession->AddSendData(SC_GUILD, eGuild::SC_GUILD_CONTRIBUTION_POINT, (char*)&packet, sizeof(GuildContribution::SCGuildContributionPoint));
}

void CDNUserSendManager::SendGuildWeeklyContributionRankList( const TAGetGuildContributionRanking* pData )
{
	GuildContribution::SCGuildContributionRank packet;
	memset(&packet, 0, sizeof(GuildContribution::SCGuildContributionRank));

	packet.nCount = pData->nCount;
	memcpy(packet.Data, pData->ContributionRankingList, sizeof(GuildContribution::TGuildContributionRankingData) * pData->nCount);
	int nLen = sizeof(GuildContribution::SCGuildContributionRank) - sizeof(GuildContribution::TGuildContributionRankingData) * (GUILDSIZE_MAX - pData->nCount);
	m_pSession->AddSendData(SC_GUILD, eGuild::SC_GUILD_CONTRIBUTION_RANK, (char*)&packet, nLen);
}
#endif	// #if defined( PRE_ADD_GUILD_CONTRIBUTION )


#if defined(PRE_ADD_EQUIPLOCK)
void CDNUserSendManager::SendLockItems(CDNUserItem* pItem)
{
	SCEquipItemLockList packet;
	memset(&packet, 0, sizeof(SCEquipItemLockList));
	pItem->GetLockItemList(packet.nEquipCount, packet.nCashEquipCount, packet.LockItems);

	int nSize = sizeof(SCEquipItemLockList) - sizeof(EquipItemLock::TLockItemInfo) * (EQUIPMAX + CASHEQUIPMAX - packet.nEquipCount - packet.nCashEquipCount);
	m_pSession->AddSendData(SC_ITEM, eItem::SC_EQUIPITEM_LOCKLIST, (char*)&packet, nSize);
}

void CDNUserSendManager::SendAddLockItem(int nRet, DBDNWorldDef::ItemLocation::eCode Code, char cSlotIndex, __time64_t tLockDate)
{
	SCItemLockRes packet;
	memset(&packet, 0, sizeof(SCItemLockRes));

	packet.nRet = nRet;
	if(nRet == ERROR_NONE)
	{
		packet.LockItems.ItemData.eItemLockStatus = EquipItemLock::Lock;
		packet.LockItems.cItemSlotIndex = cSlotIndex;
		packet.LockItems.ItemData.tUnLockDate = tLockDate;
		packet.IsCashEquip = (Code == DBDNWorldDef::ItemLocation::CashEquip);
	}

	m_pSession->AddSendData(SC_ITEM, eItem::SC_ITEM_LOCK_RES, (char*)&packet, sizeof(packet));
}

void CDNUserSendManager::SendRequestUnlockItem(int nRet, DBDNWorldDef::ItemLocation::eCode Code, char cSlotIndex, __time64_t tUnLockDate, __time64_t tUnLockRequestDate)
{
	SCItemLockRes packet;
	memset(&packet, 0, sizeof(SCItemLockRes));

	packet.nRet = nRet;
	if(nRet == ERROR_NONE)
	{
		packet.LockItems.cItemSlotIndex = cSlotIndex;
		packet.LockItems.ItemData.eItemLockStatus = EquipItemLock::RequestUnLock;
		packet.LockItems.ItemData.tUnLockDate = tUnLockDate;
		packet.LockItems.ItemData.tUnLockRequestDate = tUnLockRequestDate;
		packet.IsCashEquip = (Code == DBDNWorldDef::ItemLocation::CashEquip);	
	}
	m_pSession->AddSendData(SC_ITEM, eItem::SC_ITEM_UNLOCK_RES, (char*)&packet, sizeof(packet));
}
#endif	// #if defined(PRE_ADD_EQUIPLOCK)

#if defined( PRE_ADD_DWC )
void CDNUserSendManager::SendDWCCreateTeam(int nRetCode, UINT nTeamID, const WCHAR *wszTeamName)
{
	SCDWCCreateTeam packet = { 0, };
	packet.nRetCode = nRetCode;
	packet.nTeamID = nTeamID;
	if (wszTeamName)
		_wcscpy(packet.wszTeamName, _countof(packet.wszTeamName), wszTeamName, (int)wcslen(wszTeamName));
	m_pSession->AddSendData(SC_DWC, eDWC::SC_DWC_CREATETEAM, (char*)&packet, sizeof(packet));
}

void CDNUserSendManager::SendDWCLeaveTeam(int nRetCode, UINT nTeamID, INT64 biLeaveCharDBID, const WCHAR *wszLeaveCharName)
{
	SCDWCLeaveTeam packet = { 0, };
	packet.nRetCode = nRetCode;
	packet.nTeamID = nTeamID;
	packet.biLeaveCharDBID = biLeaveCharDBID;
	if (wszLeaveCharName)
		_wcscpy(packet.wszLeaveCharName, _countof(packet.wszLeaveCharName), wszLeaveCharName, (int)wcslen(wszLeaveCharName));
	m_pSession->AddSendData(SC_DWC, eDWC::SC_DWC_LEAVETEAM, (char*)&packet, sizeof(packet));
}

void CDNUserSendManager::SendDWCDismissTeam(int nRetCode, UINT nTeamID)
{
	SCDWCDismissTeam packet = { 0, };
	packet.nRetCode = nRetCode;
	packet.nTeamID = nTeamID;
	m_pSession->AddSendData(SC_DWC, eDWC::SC_DWC_DISMISS_TEAM, (char*)&packet, sizeof(packet));
}

void CDNUserSendManager::SendDWCInviteMemberReq(UINT nTeamID, UINT nFromSessionID, const WCHAR *wszTeamName, const WCHAR *wszFromCharName)
{
	SCDWCInviteMemberReq packet = { 0, };
	packet.nTeamID = nTeamID;
	packet.nFromSessionID = nFromSessionID;
	if (wszTeamName)
		_wcscpy(packet.wszTeamName, _countof(packet.wszTeamName), wszTeamName, (int)wcslen(wszTeamName));
	if (wszFromCharName)
		_wcscpy(packet.wszFromCharName, _countof(packet.wszFromCharName), wszFromCharName, (int)wcslen(wszFromCharName));
	m_pSession->AddSendData(SC_DWC, eDWC::SC_DWC_INVITE_MEMBER_REQ, (char*)&packet, sizeof(packet));
}

void CDNUserSendManager::SendDWCInviteMemberAck(int nRetCode, const WCHAR *wszTargetCharName)
{
	SCDWCInviteMemberAck packet = { 0, };
	packet.nRetCode = nRetCode;
	if (wszTargetCharName)
		_wcscpy(packet.wszTargetCharName, _countof(packet.wszTargetCharName), wszTargetCharName, (int)wcslen(wszTargetCharName));
	m_pSession->AddSendData(SC_DWC, eDWC::SC_DWC_INVITE_MEMBER_ACK, (char*)&packet, sizeof(packet));
}

void CDNUserSendManager::SendDWCGetTeamInfo(int nRetCode, const TDWCTeamInfo *pTeamInfo)
{
	SCDWCGetTeamInfo packet = { 0, };
	packet.nRetCode = nRetCode;
	if (pTeamInfo)
		packet.TeamInfo = *pTeamInfo;
	m_pSession->AddSendData(SC_DWC, eDWC::SC_DWC_GET_TEAMINFO, (char*)&packet, sizeof(packet));
}

void CDNUserSendManager::SendDWCGetTeamMember(int nRetCode, UINT nTeamID, const std::vector<TDWCTeamMemberInfo> &vecMembers)
{
	SCDWCGetTeamMember packet = { 0, };
	packet.nRetCode = nRetCode;
	packet.nTeamID = nTeamID;
	int nCount = (int)vecMembers.size();
	if (nCount > DWC::MAX_TEAM_MEMBER)
		nCount = DWC::MAX_TEAM_MEMBER;
	packet.nCount = nCount;
	for (int i = 0; i < nCount; ++i)
		packet.MemberList[i] = vecMembers[i];
	int nSize = sizeof(SCDWCGetTeamMember) - sizeof(packet.MemberList) + (sizeof(TDWCTeamMemberInfo) * nCount);
	m_pSession->AddSendData(SC_DWC, eDWC::SC_DWC_GET_TEAMMEMBER, (char*)&packet, nSize);
}

void CDNUserSendManager::SendDWCGetRankPage(int nRetCode, int nTotalCount, int nPageNum, const std::vector<TDWCRankInfo> &vecRanks)
{
	SCDWCGetRankPage packet = { 0, };
	packet.nRetCode = nRetCode;
	packet.nTotalCount = nTotalCount;
	packet.nPageNum = nPageNum;
	int nCount = (int)vecRanks.size();
	if (nCount > DWC::MAX_RANK_PER_PAGE)
		nCount = DWC::MAX_RANK_PER_PAGE;
	packet.nCount = nCount;
	for (int i = 0; i < nCount; ++i)
		packet.RankList[i] = vecRanks[i];
	int nSize = sizeof(SCDWCGetRankPage) - sizeof(packet.RankList) + (sizeof(TDWCRankInfo) * nCount);
	m_pSession->AddSendData(SC_DWC, eDWC::SC_DWC_GET_RANKPAGE, (char*)&packet, nSize);
}

void CDNUserSendManager::SendDWCFindRank(int nRetCode, const TDWCRankInfo *pRankInfo)
{
	SCDWCFindRank packet = { 0, };
	packet.nRetCode = nRetCode;
	if (pRankInfo)
		packet.RankInfo = *pRankInfo;
	m_pSession->AddSendData(SC_DWC, eDWC::SC_DWC_FIND_RANK, (char*)&packet, sizeof(packet));
}

void CDNUserSendManager::SendDWCChannelInfo(const TDWCChannelInfo &info)
{
	SCDWCChannelInfo packet = { 0, };
	packet.ChannelInfo = info;
	m_pSession->AddSendData(SC_DWC, eDWC::SC_DWC_CHANNELINFO, (char*)&packet, sizeof(packet));
}

void CDNUserSendManager::SendDWCScoreInfo(int nDWCPoint, int nTotalWin, int nTotalLose, int nTotalDraw)
{
	SCDWCScoreInfo packet = { 0, };
	packet.nDWCPoint = nDWCPoint;
	packet.nTotalWin = nTotalWin;
	packet.nTotalLose = nTotalLose;
	packet.nTotalDraw = nTotalDraw;
	m_pSession->AddSendData(SC_DWC, eDWC::SC_DWC_SCORE_INFO, (char*)&packet, sizeof(packet));
}

void CDNUserSendManager::SendDWCGradeInfo(int nRank, BYTE cGrade)
{
	SCDWCGradeInfo packet = { 0, };
	packet.nRank = nRank;
	packet.cGrade = cGrade;
	m_pSession->AddSendData(SC_DWC, eDWC::SC_DWC_GRADE_INFO, (char*)&packet, sizeof(packet));
}
#endif // #if defined( PRE_ADD_DWC )