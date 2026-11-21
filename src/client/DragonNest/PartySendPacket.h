#pragma once

#include "DNPacket.h"
#include "ClientSessionManager.h"
#include "ClientSocket.h"
#include "DnCommonUtil.h"

#ifdef PRE_PARTY_DB
#include "DnPartyData.h"

inline void SendCreateParty(const SPartyCreateParam& param)
#else
inline void SendCreateParty(BYTE cPartyType, BYTE cPartyMemberMax, int nUserLvLimitMin, int nUserLvLimitMax, TPARTYITEMLOOTRULE ItemLootRule, TITEMRANK ItemLootRank, const WCHAR * pPartyName, const WCHAR * pPass, int nMapIdx, TDUNGEONDIFFICULTY Difficulty, BYTE cIsPartyDice, BYTE cUseVoice = 0)
#endif
{
	CSCreateParty CreateParty;
	memset(&CreateParty, 0, sizeof(CSCreateParty));
	
#ifdef PRE_PARTY_DB
	CreateParty.PartyType = param.PartyType;
	CreateParty.cPartyMemberMax = param.cPartyMemberMax;
	CreateParty.ItemLootRule = param.ItemLootRule;
	CreateParty.ItemRank = param.ItemLootRank;
	CreateParty.cUserLvLimitMin = BYTE(param.nUserLvLimitMin);
	CreateParty.nTargetMapIdx = param.nMapIdx;
	CreateParty.Difficulty = param.Difficulty;

	if (param.cIsPartyDice != 0)
		CreateParty.iBitFlag |= Party::BitFlag::JobDice;
	_wcscpy(CreateParty.wszPartyName, _countof(CreateParty.wszPartyName), param.wszPartyName.c_str(), (int)wcslen(param.wszPartyName.c_str()));

	if (param.iPassword != Party::Constants::INVALID_PASSWORD)
	{
		CreateParty.iBitFlag |= Party::BitFlag::Password;
		CreateParty.iPassword = param.iPassword;
	}

	if (param.bAllowWorldZonePartyList)
		CreateParty.iBitFlag |= Party::BitFlag::WorldmapAllow;

	if (param.cUseVoice != 0)
		CreateParty.iBitFlag |= Party::BitFlag::VoiceChat;

	CClientSessionManager::GetInstance().SendPacket(CS_PARTY, eParty::CS_CREATEPARTY, (char*)&CreateParty, sizeof(CSCreateParty));
#else
	if (Difficulty < Dungeon::Difficulty::Easy || Difficulty > Dungeon::Difficulty::Max)
		return;

	CreateParty.cRaidParty = cPartyType;
	CreateParty.cUserLvLimitMax = (BYTE)nUserLvLimitMax;
	CreateParty.cPartyMemberMax = cPartyMemberMax;
	CreateParty.ItemLootRule = ItemLootRule;	//Refer to ePartyItemLootRule
	CreateParty.ItemRank = ItemLootRank;
	CreateParty.cUserLvLimitMin = (BYTE)nUserLvLimitMin;
	CreateParty.nTargetMapIdx = nMapIdx;
	CreateParty.Difficulty = Difficulty;

	CreateParty.cIsJobDice = cIsPartyDice;
	CreateParty.cUseVoice = cUseVoice;
	CreateParty.cNameLen = (BYTE)wcslen(pPartyName);
	if (CommonUtil::IsValidPartyNameLen(CreateParty.cNameLen) == false || CreateParty.cNameLen <= 0)
		return;
	_wcscpy(CreateParty.wszBuf, _countof(CreateParty.wszBuf), pPartyName, (int)wcslen(pPartyName));

	if (pPass != NULL)
	{
		CreateParty.cPassWordLen = (BYTE)wcslen(pPass);
		if (CommonUtil::IsValidPartyPwdLen(CreateParty.cPassWordLen) == false || CreateParty.cPassWordLen <= 0)
			return;
		_wcscpy(CreateParty.wszBuf + CreateParty.cNameLen, _countof(CreateParty.wszBuf) - CreateParty.cNameLen, pPass, (int)wcslen(pPass));
	}

	CClientSessionManager::GetInstance().SendPacket(CS_PARTY, eParty::CS_CREATEPARTY, (char*)&CreateParty, sizeof(CSCreateParty) - \
		sizeof(CreateParty.wszBuf) + ((CreateParty.cNameLen + CreateParty.cPassWordLen) * sizeof(WCHAR)));
#endif

	FUNC_LOG();
}

inline void SendPartyOut()
{
	CClientSessionManager::GetInstance().SendPacket(CS_PARTY, eParty::CS_PARTYOUT, NULL, 0);
	FUNC_LOG();
}

#ifdef PRE_PARTY_DB
inline void SendJoinParty(TPARTYID PartyID, int iPassword)
{ 
	CSJoinParty JoinParty;
	memset(&JoinParty, 0, sizeof(CSJoinParty));

	JoinParty.PartyID = PartyID;
	JoinParty.iPassword = iPassword;

	CClientSessionManager::GetInstance().SendPacket(CS_PARTY, eParty::CS_JOINPARTY, (char*)&JoinParty, sizeof(CSJoinParty));
	FUNC_LOG();
}
#else
inline void SendJoinParty(TPARTYID PartyID, const WCHAR * pPartyPassword = NULL)
{ 
	CSJoinParty JoinParty;
	memset(&JoinParty, 0, sizeof(CSJoinParty));

	JoinParty.PartyID = PartyID;

	if (pPartyPassword != NULL)
	{
		JoinParty.cPartyPasswordSize = (BYTE)wcslen(pPartyPassword);
		if (CommonUtil::IsValidPartyPwdLen(JoinParty.cPartyPasswordSize) == false)
			return;
		_wcscpy(JoinParty.wszBuf, _countof(JoinParty.wszBuf), pPartyPassword, (int)wcslen(pPartyPassword));
	}

	CClientSessionManager::GetInstance().SendPacket(CS_PARTY, eParty::CS_JOINPARTY, (char*)&JoinParty, sizeof(CSJoinParty) - \
		sizeof(JoinParty.wszBuf) + (JoinParty.cPartyPasswordSize * sizeof(WCHAR)));
	FUNC_LOG();
}
#endif

#ifdef PRE_PARTY_DB
	#ifdef PRE_FIX_61545
inline void SendReqPartyListInfo(int nPage, const WCHAR* pSearchWord, TDUNGEONDIFFICULTY Difficulty, Party::OrderType::eCode orderType, Party::StageOrderType::eCode stageOrderType, bool bOnlySameVillage, WCHAR searchWorldSeperator)
	#else
inline void SendReqPartyListInfo(int nPage, const WCHAR* pSearchWord, TDUNGEONDIFFICULTY Difficulty, Party::OrderType::eCode orderType, Party::StageOrderType::eCode stageOrderType, bool bOnlySameVillage)
	#endif
{
	CSPartyListInfo PartyInfo;
	memset(&PartyInfo, 0, sizeof(CSPartyListInfo));

	if (CommonUtil::IsValidPartySearchWord(pSearchWord) != CommonUtil::ePARTYWORD_NONE)
		return;

	_wcscpy(PartyInfo.wszSearchWord, _countof(PartyInfo.wszSearchWord), pSearchWord, (int)wcslen(pSearchWord));
	PartyInfo.unReqPage = nPage;
	PartyInfo.OrderType = orderType;
	PartyInfo.StageOrderType = stageOrderType;
	PartyInfo.bOnlySameVillage = bOnlySameVillage;
#ifdef PRE_FIX_61545
	PartyInfo.cSeperator = searchWorldSeperator;
#endif

	OutputDebug("Req PartyListPage:%d\n", nPage);

	CClientSessionManager::GetInstance().SendPacket(CS_PARTY, eParty::CS_PARTYLISTINFO, (char*)&PartyInfo, sizeof(CSPartyListInfo));
	FUNC_LOG();
}
#else
inline void SendReqPartyListInfo( int nPage, int * nSortMapIdx, TDUNGEONDIFFICULTY Difficulty, bool bSort )
{
	if (nSortMapIdx == NULL) return;

	CSPartyListInfo PartyInfo;
	memset(&PartyInfo, 0, sizeof(CSPartyListInfo));

	PartyInfo.cOffSetCnt = PARTYLISTOFFSET;
	PartyInfo.cGetPage = nPage;
#if defined( PRE_PARTY_DB )
#else
	memcpy(PartyInfo.nSortMapIdx, nSortMapIdx, sizeof(PartyInfo.nSortMapIdx));
#endif // #if defined( PRE_PARTY_DB )
	PartyInfo.Difficulty = Difficulty;
	PartyInfo.cIsRefresh = bSort ? 0 : 1;

	CClientSessionManager::GetInstance().SendPacket(CS_PARTY, eParty::CS_PARTYLISTINFO, (char*)&PartyInfo, sizeof(CSPartyListInfo));
	FUNC_LOG();
}
#endif

inline void SendRefreshGateInfo(bool boEnter, EtVector3 &Pos)
{
	CSRefreshGateInfo Refresh;
	memset(&Refresh, 0, sizeof(CSRefreshGateInfo));

	Refresh.boEnter = boEnter;
	Refresh.Position.x = Pos.x;
	Refresh.Position.y = Pos.y;
	Refresh.Position.z = Pos.z;

	CClientSessionManager::GetInstance().SendPacket(CS_PARTY, eParty::CS_REFRESHGATEINFO, (char*)&Refresh, sizeof(CSRefreshGateInfo));
	FUNC_LOG();
}

inline void SendStartStage( TDUNGEONDIFFICULTY Difficult, char cDungeonSelectIndex = -1, bool bReturnVillage = false, UINT nFarmDBID = 0, BYTE cSelectMapIndex = 0 )
{
	CSStartStage Packet;
	memset(&Packet, 0, sizeof(CSStartStage));

	Packet.Difficulty = Difficult;
	Packet.bReturnVillage = bReturnVillage;
	Packet.cSelectDungeonIndex = cDungeonSelectIndex;
	Packet.nFarmDBID = nFarmDBID;
	Packet.cSelectMapIndex = cSelectMapIndex;

	CClientSessionManager::GetInstance().SendPacket(CS_PARTY, eParty::CS_STARTSTAGE, (char*)&Packet, sizeof(CSStartStage));
	FUNC_LOG();
}

inline void SendCancelStage()
{
	CClientSessionManager::GetInstance().SendPacket(CS_PARTY, eParty::CS_CANCELSTAGE, NULL, 0);
	FUNC_LOG();
}

inline void SendInvitePartyMember(const WCHAR * pMemberName)
{
	CSInvitePartyMember packet;
	memset(&packet, 0, sizeof(packet));

	packet.cInviteMemberSize = (BYTE)wcslen(pMemberName);
	if (CommonUtil::IsValidCharacterNameLen(packet.cInviteMemberSize) == false)
		return;
	_wcscpy(packet.wszBuf, _countof(packet.wszBuf), pMemberName, (int)wcslen(pMemberName));

	CClientSessionManager::GetInstance().SendPacket(CS_PARTY, eParty::CS_PARTYINVITE, (char*)&packet, sizeof(packet) - sizeof(packet.wszBuf) + \
		(packet.cInviteMemberSize * sizeof(WCHAR)));
	FUNC_LOG();
}

inline void SendInviteDenied(TPARTYID PartyIdx, const WCHAR * pInviter, bool bIsOpenBlind = false)
{
	CSInviteDenied packet;
	memset(&packet, 0, sizeof(CSInviteDenied));

	packet.PartyIdx = PartyIdx;
	packet.cIsOpenBlind = bIsOpenBlind == true ? 1 : 0;
	packet.cInviterNameLen = (BYTE)wcslen(pInviter);
	if (CommonUtil::IsValidCharacterNameLen(packet.cInviterNameLen) == false)
		return;
	_wcscpy(packet.wszBuf, _countof(packet.wszBuf), pInviter, (int)wcslen(pInviter));

	CClientSessionManager::GetInstance().SendPacket(CS_PARTY, eParty::CS_PARTYINVITE_DENIED, (char*)&packet, sizeof(packet) - \
		sizeof(packet.wszBuf) + (packet.cInviterNameLen * sizeof(WCHAR)));
	FUNC_LOG();
}

inline void SendOverServerInviteResult(int nGameServerID, const WCHAR * pInviterName, bool bAgree, bool bIsOpenBlind = false)
{
	CSPartyInviteOverServerResult packet;
	memset(&packet, 0, sizeof(CSPartyInviteOverServerResult));

	packet.nGameServerID = nGameServerID;
	_wcscpy(packet.wszInviterName, _countof(packet.wszInviterName), pInviterName, (int)wcslen(pInviterName));
	if (bIsOpenBlind)
		packet.nRetCode = ERROR_PARTY_INVITEFAIL_DESTUSER_SITUATION_NOTALLOWED;
	else
		packet.nRetCode = bAgree == true ? ERROR_NONE : ERROR_PARTY_INVITEFAIL_DESTINVITE_DENIED;

	CClientSessionManager::GetInstance().SendPacket(CS_PARTY, eParty::CS_PARTYINVITE_OVERSERVER_RESULT, (char*)&packet, sizeof(packet));
	FUNC_LOG();
}

inline void SendReqPartyMemberInfo(TPARTYID PartyID)
{
	CSPartyMemberInfo packet;

	packet.PartyID = PartyID;

	CClientSessionManager::GetInstance().SendPacket(CS_PARTY, eParty::CS_PARTYMEMBER_INFO, (char*)&packet, sizeof(packet));
	FUNC_LOG();
}

inline void SendReqPartyMemberKick(UINT nKickMemberSessionID, char cKickKind)
{
	CSPartyMemberKick packet;
	memset(&packet, 0, sizeof(packet));

	packet.nSessionID = nKickMemberSessionID;
	packet.cKickKind = cKickKind;

	CClientSessionManager::GetInstance().SendPacket(CS_PARTY, eParty::CS_PARTYMEMBER_KICK, (char*)&packet, sizeof(packet));
	FUNC_LOG();
}

inline void SendReqPartyLeaderSwap(int nNewLeaderSessionID)
{
	CSPartyLeaderSwap packet;
	packet.nSessionID = nNewLeaderSessionID;

	CClientSessionManager::GetInstance().SendPacket(CS_PARTY, eParty::CS_PARTYLEADER_SWAP, (char*)&packet, sizeof(packet));
	FUNC_LOG();
}

inline void SendReqMoveChannelList()
{
	CClientSessionManager::GetInstance().SendPacket(CS_PARTY, eParty::CS_CHANNELLIST, NULL, 0);
	FUNC_LOG();
}

inline void SendReqMoveSelectChannel(int nChannelID )
{
	CSVillageSelectChannel packet;
	packet.nChannelID = nChannelID;

	CClientSessionManager::GetInstance().SendPacket(CS_PARTY, eParty::CS_SELECTCHANNEL, (char*)&packet, sizeof(packet));
	//blondymarry start	
	//FUNC_LOG();
	InfoLog("Send Move Channel : %d ",nChannelID );
	//blondymarry end
}

#ifdef PRE_PARTY_DB
inline void SendPartyInfoModify(BYTE cPartyMemberMax, TPARTYITEMLOOTRULE LootRule, TITEMRANK LootRank, BYTE cUserLvLimitMin, const WCHAR * pName, int iPassword, int mapIdx, TDUNGEONDIFFICULTY mapDifficulty, BYTE cIsJobDice, bool bAllowWorldZonePartyList)
#else
inline void SendPartyInfoModify(BYTE cPartyMemberMax, TPARTYITEMLOOTRULE LootRule, TITEMRANK LootRank, BYTE cUserLvLimitMin, BYTE cUserLvLimitMax, const WCHAR * pName, const WCHAR * pPass, int mapIdx, TDUNGEONDIFFICULTY mapDifficulty, BYTE cIsJobDice)
#endif
{
	CSPartyInfoModify packet;
	memset(&packet, 0, sizeof(CSPartyInfoModify));

	packet.cPartyMemberMax = cPartyMemberMax;
	packet.LootRule = LootRule;
	packet.LootRank = LootRank;
	packet.cUserLvLimitMin = cUserLvLimitMin;
#if defined( PRE_PARTY_DB )
#else
	packet.cUserLvLimitMax = cUserLvLimitMax;
#endif // #if defined( PRE_PARTY_DB )
	packet.nTargetMapIdx = mapIdx;
	packet.Difficulty = mapDifficulty;
#ifdef PRE_PARTY_DB
	packet.iBitFlag = 0;
	if (cIsJobDice != 0)
		packet.iBitFlag |= Party::BitFlag::JobDice;

	if (CommonUtil::IsValidPartyNameLen(int(wcslen(pName))) == false)
		return;

	if (iPassword != Party::Constants::INVALID_PASSWORD)
		packet.iBitFlag |= Party::BitFlag::Password;
	packet.iPassword = iPassword;

	if (bAllowWorldZonePartyList)
		packet.iBitFlag |= Party::BitFlag::WorldmapAllow;
#else
	packet.cIsJobDice = cIsJobDice;

	if (CommonUtil::IsValidPartyNameLen(int(wcslen(pName))) == false || 
		CommonUtil::IsValidPartyPwdLen(int(wcslen(pPass))) == false)
		return;
#endif


	_wcscpy(packet.wszName, _countof(packet.wszName), pName, (int)wcslen(pName));
#ifdef PRE_PARTY_DB
	packet.iPassword = iPassword;
#else
	_wcscpy(packet.wszPass, _countof(packet.wszPass), pPass, (int)wcslen(pPass));
#endif

	CClientSessionManager::GetInstance().SendPacket(CS_PARTY, eParty::CS_PARTYMODIFY, (char*)&packet, sizeof(packet));
	FUNC_LOG();
}

inline void SendPartyJoinGetReversionItem(bool bJoin)
{
	CSJoinGetReversionItem packet;
	packet.bJoin = bJoin;

	CClientSessionManager::GetInstance().SendPacket(CS_PARTY, eParty::CS_JOINGETREVERSEITEM, (char*)&packet, sizeof(packet));
	FUNC_LOG();
}

inline void SendSelectDungeonInfo(int nMapIndex, BYTE cDiffyculty)
{
	CSSelectDunGeon packet;
	memset(&packet, 0, sizeof(packet));

	packet.cDiffyculty = cDiffyculty;
//	packet.nRecommendLv = nRecommandLv;
//	packet.nRecommendUserCnt = nRecommandUserCnt;
	packet.nMapIndex = nMapIndex;

	CClientSessionManager::GetInstance().SendPacket(CS_PARTY, eParty::CS_SELECTDUNGEON, (char*)&packet, sizeof(packet));
	FUNC_LOG();
}

inline void SendReadyRequest()
{
	CClientSessionManager::GetInstance().SendPacket(CS_PARTY, eParty::CS_READYREQUEST, NULL, 0);
}

inline void SendPartyAskJoin(const WCHAR * pwszChracterName)
{
	CSPartyAskJoin packet;
	memset(&packet, 0, sizeof(CSPartyAskJoin));

	_wcscpy(packet.wszPartyLeaderName, _countof(packet.wszPartyLeaderName), pwszChracterName, (int)wcslen(pwszChracterName));

	CClientSessionManager::GetInstance().SendPacket(CS_PARTY, eParty::CS_PARTYASKJOIN, (char*)&packet, sizeof(CSPartyAskJoin));
}

inline void SendPartyAskJoinDecision(bool bDecision, const WCHAR * pwszAsker)	//true join agree, false deny
{
	CSPartyAskJoinDecision packet;
	memset(&packet, 0, sizeof(CSPartyAskJoinDecision));

	packet.cRet = bDecision == true ? 1 : 0;
	_wcscpy(packet.wszPartyAsker, _countof(packet.wszPartyAsker), pwszAsker, (int)wcslen(pwszAsker));

	CClientSessionManager::GetInstance().SendPacket(CS_PARTY, eParty::CS_PARTYASKJOINDECISION, (char*)&packet, sizeof(CSPartyAskJoinDecision));
}

inline void SendUserAbsent(bool bAbsent)
{
	CSMemberAbsent packet;
	memset(&packet, 0, sizeof(CSMemberAbsent));

	packet.cAbsentFlag = bAbsent == true ? 0 : 1;

	CClientSessionManager::GetInstance().SendPacket(CS_PARTY, eParty::CS_ABSENT, (char*)&packet, sizeof(packet));
}

inline void SendVerifyDungeon(int nMapIdx)
{
	CSVerifyDungeon packet;
	memset(&packet, 0, sizeof(CSVerifyDungeon));

	packet.nMapIdx = nMapIdx;
	packet.cDifficulty = 0;

	CClientSessionManager::GetInstance().SendPacket(CS_PARTY, eParty::CS_VERIFYTARGETDUNGEON, (char*)&packet, sizeof(packet));
}

inline void SendPartySwapMemberIdx(const std::vector<TSwapMemberIndex>& memberIndexes)
{
	CSPartySwapMemberIndex packet;
	memset(&packet, 0, sizeof(CSPartySwapMemberIndex));

	packet.cCount = (BYTE)memberIndexes.size();
	int i = 0;
	for (; i < packet.cCount; ++i)
	{
		packet.Index[i] = memberIndexes[i];
	}
	
	CClientSessionManager::GetInstance().SendPacket(CS_PARTY, eParty::CS_SWAPMEMBERINDEX, (char*)&packet, sizeof(packet));
}

inline void SendPartyCheckLastDungeonInfo(bool bIsOK)
{
	CSPartyConfirmLastDungeonInfo packet;
	packet.bIsOK = bIsOK;
	CClientSessionManager::GetInstance().SendPacket(CS_PARTY, eParty::CS_CONFIRM_LASTDUNGEONINFO, (char*)&packet, sizeof(packet));
}

inline void SendRequestNestInfo( int nSessionID )
{
	CSGetPlayerCustomEventUI packet;
	packet.nSessionID = nSessionID;

	CClientSessionManager::GetInstance().SendPacket(CS_CUSTOMEVENTUI, ePlayerCustomEventUI::CS_GETPLAYERUI, (char*)&packet, sizeof(packet));
}

#if defined ( PRE_MOD_REQ_JOIN_PARTY_ANSWER_MSG_APP )
inline void SendReqPartyInfo( TPARTYID PartyID )
{
	CSPartyInfo packet;
	packet.PartyID = PartyID;
	ZeroMemory(packet.wszCharName, _countof(packet.wszCharName)); 
	
	CClientSessionManager::GetInstance().SendPacket(CS_PARTY, eParty::CS_PARTYINFO, (char*)&packet, sizeof(packet) );
}

inline void SendReqPartyInfo( const WCHAR* pwszPlayerName )
{
	CSPartyInfo packet;
	packet.PartyID = 0;
	_wcscpy(packet.wszCharName, _countof(packet.wszCharName), pwszPlayerName, (int)wcslen(pwszPlayerName));

	CClientSessionManager::GetInstance().SendPacket(CS_PARTY, eParty::CS_PARTYINFO, (char*)&packet, sizeof(packet) );
}
#endif 

inline void SendSelectStage( const BYTE cSelectStage )
{
	CSSelectStage packet;
	packet.cSelectMapIndex = cSelectStage;

	CClientSessionManager::GetInstance().SendPacket(CS_PARTY, eParty::CS_SELECTSTAGE, (char*)&packet, sizeof(packet));
}
