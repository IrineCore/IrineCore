#include "stdafx.h"
#include "DNUserBase.h"
#include "DNUserSession.h"
#include "DNUserSendManager.h"
#include "DNFriend.h"
#include "DNIsolate.h"
#include "DNMissionSystem.h"
#include "DNAppellation.h"
#include "DNGameDataManager.h"
#include "DNDBConnectionManager.h"
#include "DNRestraint.h"
#include "DNGuildSystem.h"
#include "DNDBConnection.h"
#include "DNDBConnectionManager.h"
#include "DNLogConnection.h"
#include "DNWorldUserState.h"
#include "DNMasterConnection.h"
#include "DNAuthManager.h"
#include "DNQuestManager.h"
#include "DNCashConnection.h"
#include "EtUIXML.h"
#include "TimeSet.h"
#include "DNEvent.h"
#include "DNCashRepository.h"
#include "DNTimeEventSystem.h"
#include "DNMailSender.h"
#include "SundriesFunc.h"
#include "DNCountryUnicodeSet.h"
#ifdef _USE_VOICECHAT
#include "DNVoiceChat.h"
#endif
#include "DNCommonVariable.h"

#if defined(_VILLAGESERVER)
#include "DNPartyManager.h"
#include "DNUserSessionManager.h"
#include "DNNpcObject.h"
#include "DNFieldManager.h"
#include "DNVillageUserEventHandler.hpp"
extern TVillageConfig g_Config;

#elif defined(_GAMESERVER)
#include "DNGameUserEventHandler.hpp"
#include "DNMasterConnectionManager.h"
#include "DnPlayerActor.h"
#include "DNUserTcpConnection.h"
extern TGameConfig g_Config;
#endif
#if defined( PRE_ADD_NPC_REPUTATION_SYSTEM )
#include "ReputationSystemRepository.h"
#include "NpcReputationProcessor.h"
#endif // #if defined( PRE_ADD_NPC_REPUTATION_SYSTEM )
#include "MasterSystemCacheRepository.h"
#if defined( PRE_ADD_SECONDARY_SKILL )
#include "SecondarySkillRepositoryServer.h"
#include "ManufactureSkill.h"
#endif // #if defined( PRE_ADD_SECONDARY_SKILL )
#include "DNPeriodQuestSystem.h"
#include "DNGuildRecruitCacheRepository.h"
#ifdef PRE_ADD_LIMITED_CASHITEM
#include "DNLimitedCashItemRepository.h"
#endif		//#ifdef PRE_ADD_LIMITED_CASHITEM
#if defined (PRE_ADD_BESTFRIEND)
#include "DNBestFriend.h"
#endif
#if defined( PRE_PRIVATECHAT_CHANNEL )
#include "DNPrivateChatChannel.h"
#include "DnPrivateChatManager.h"
#endif
#if defined( PRE_ADD_STAMPSYSTEM )
#include "DNStampSystem.h"
#endif // #if defined( PRE_ADD_STAMPSYSTEM )

#include "DNGesture.h"
#include "Version.h"

CDNUserBase::CDNUserBase(void): CDNUserSendManager((CDNUserSession*)this)
{
	m_nAccountDBID = m_nSessionID = 0;
	m_biCharacterDBID = 0;
#if defined( PRE_ADD_DWC )
	m_nDWCTeamID = 0;
	memset(m_wszDWCTeamName, 0, sizeof(m_wszDWCTeamName));
#endif // #if defined( PRE_ADD_DWC )
	memset(&m_wszAccountName, 0, sizeof(m_wszAccountName));
	memset(&m_szAccountName, 0, sizeof(m_szAccountName));
	memset(&m_szCharacterName, 0, sizeof(m_szCharacterName));

	if( g_pDBConnectionManager )
		m_pDBCon = g_pDBConnectionManager->GetDBConnection( m_cDBThreadID );
	else
		m_pDBCon = NULL;

	m_bLoadUserData = false;

	memset(&m_UserData, 0, sizeof(TUserData));
	memset(&m_Profile, 0, sizeof(TProfile));
	memset( &m_PvPLadderScoreInfo, 0, sizeof(m_PvPLadderScoreInfo) );
	memset( &m_PvPTotalGhoulScores, 0, sizeof(m_PvPTotalGhoulScores) );
	memset( &m_PvPAddGhoulScores, 0, sizeof(m_PvPAddGhoulScores) );
	memset(&m_UnionReputePointInfo, 0, sizeof(m_UnionReputePointInfo));

	// m_pSendManager = new CDNUserSendManager(this);
	m_pItem = new CDNUserItem(m_pSession);
	m_pQuest = new CDNUserQuest(m_pSession);
	m_pCheatCommand = new CDNCheatCommand(m_pSession);
	m_pGMCommand = new CDNGMCommand(m_pSession);
	m_pFriend = new CDNFriend(this);
	m_pIsolate = new CDNIsolate(this);
	m_pMissionSystem = new CDNMissionSystem(m_pSession);
	m_pEventSystem = new CDNEventSystem(m_pSession);
	m_pAppellation = new CDNAppellation(m_pSession);
	m_pRestraint = new CDNRestraint(this);
#if defined( PRE_ADD_NPC_REPUTATION_SYSTEM )
	m_pReputationSystem = new CReputationSystemRepository( static_cast<CDNUserSession*>(this) );
#endif // #if defined( PRE_ADD_NPC_REPUTATION_SYSTEM )
#if defined( PRE_ADD_SECONDARY_SKILL )
	m_pSecondarySkillRepository = new CSecondarySkillRepositoryServer( static_cast<CDNUserSession*>(this) );
#endif // #if defined( PRE_ADD_SECONDARY_SKILL )
	m_pGesture = new CDNGesture(m_pSession);
	m_pTimeEventSystem = new CDNTimeEventSystem(m_pSession);
	m_pCommonVariable = new CDNCommonVariable(m_pSession);
#if defined (PRE_ADD_BESTFRIEND)
	m_pBestFriend = new CDNBestFriend(m_pSession);
#endif

	m_TalkParamList.clear();
	m_bSkipParagraphCheck = m_bCalledNpcResponse = 	m_bIsNpcTalk = false;
	m_nExchangeTargetSessionID = m_nExchangeSenderSID = m_nExchangeReceiverSID = 0;

	m_dwLastMessageTick = timeGetTime();
	m_cLastMainCmd = m_cLastSubCmd = 0;

	m_bHide = false;

	m_bNeedUpdateOption = false;
	memset(&m_GameOption, 0, sizeof(TGameOptions));
	m_GameOption.SetDefault();

#ifdef PRE_ADD_GACHA_JAPAN
	m_nGachaponShopID = 0;
#endif // PRE_ADD_GACHA_JAPAN

	m_nWorldSetID = m_nRoomID = m_nChannelID = m_nClickedNpcID = m_nCutSceneID = 0;
	m_nClickedNpcObjectID = 0;
	m_wGameID = 0;
	m_cVillageID = 0;
	m_sUserWindowState = WINDOW_NONE;

	m_bPCBang = false;
	m_cPCBangGrade = PCBang::Grade::None;
	m_nPcBangBonusExp = 0;
	m_bAdult = true;
	m_cAge = 0;
	m_cDailyCreateCount = 0;
	m_nPrmInt1 = 0;
	m_cLastServerType = SERVERTYPE_MAX;
	m_bCharOutLog = false;

	m_nCashBalance = 0;

	// Auth
	m_bCertified = false;
	m_dwCertifyingTick = 0;
	m_biCertifyingKey = 0;

	memset(&m_wszVirtualIp, 0, sizeof(m_wszVirtualIp));
	memset(&m_szVirtualIp, 0, sizeof(m_szVirtualIp));

	::memset(m_dwTick, 0, sizeof(m_dwTick));

	m_cLastStageClearRank = -1;
	m_GuildSelfView.Reset();

	m_bVoiceAvailable = false;
#ifdef _USE_VOICECHAT	
	m_nVoiceChannelID = 0;
	m_nVoiceJoinType = _VOICEJOINTYPE_NONE;
	m_cIsTalking = false;
	memset(m_nVoiceMutedList, 0, sizeof(m_nVoiceMutedList));
#endif

	m_dwNpcTalkLastIndexHashCode = 0;
	m_dwNpcTalkLastTargetHashCode = 0;

	m_bIsSetSecondAuthPW	= false;	// Whether 2nd auth password is set
	m_bIsSetSecondAuthLock	= false;	// 2nd auth account Lock status


	m_cVIPRebirthCoin = 0;
	m_bVIP = false;
	m_nVIPTotalPoint = 0;
	m_tVIPEndDate = 0;
	m_bVIPAutoPay = false;
	m_nVIPBonusExp = 0;
	m_bIntroducer = false;

	m_bFriendBonus = false;
	for (int i = 0; i < eCheckPaperingAmount; i++){
		m_dwCheckPaperingRemainTime[i] = 0;
		m_listRecentCheckTime[i].clear();
	}

	m_pEffectRepository = new CDNEffectRepository((CDNUserSession*)this );
	memset( &m_MasterSystemData, 0, sizeof(m_MasterSystemData) );
	m_bSecurityUpdate = false;
	m_nVehicleObjectID = 0;
#if defined(PRE_ADD_MISSION_COUPON)
	m_nExpiredPetID = 0;
#endif
	m_nExpandNestClearCount = 0;
#if defined(PRE_ADD_TSCLEARCOUNTEX)
	m_nExpandTreasureStageClearCount = 0;
#endif	// #if defined(PRE_ADD_TSCLEARCOUNTEX)

	GuildWarReset();

	m_iRemoteEnchantItemID = 0;
	m_iRemoteItemCompoundItemID = 0;

	m_iHackAbuseDBValue			= 0;
	m_iHackAbuseDBCallCount		= 0;
	m_iHackPlayRestraintValue	= 0;
	m_iHackResetRestraintValue	= 0;
	m_nHackAbuseCharacterCntWithoutMe = 0;

#if defined(PRE_ADD_ANTI_CHAT_SPAM)
	m_bSpammer = false;
#endif

	m_dwCheckTcpPing	= timeGetTime();
	m_pairTcpPing		= std::make_pair(0,0);

	m_bSaleAbortListSended = false;

	m_ShopType = Shop::Type::Normal;

	memset(&m_OwnCharacterLevelList, 0, sizeof(m_OwnCharacterLevelList));

#if defined( _VILLAGESERVER )
	m_pUserEventHandler = new CDNVillageUserEventHandler( m_pSession );
#else
	m_pUserEventHandler = new CDNGameUserEventHandler( m_pSession );
#endif // #if defined( _VILLAGESERVER )

#ifdef PRE_ADD_COMEBACK
	m_bComebackUser = false;
#endif		//#ifdef PRE_ADD_COMEBACK
#if defined(PRE_ADD_GAMEQUIT_REWARD)
	m_bReConnectUserReward = false;
	m_eUserGameQuitRewardType = GameQuitReward::RewardType::None;
#endif	// #if defined(PRE_ADD_GAMEQUIT_REWARD)
	m_nShopID = 0;
#if defined(PRE_ADD_REMOTE_OPENSHOP)
	m_bRemoteShopOpen = false;
#endif	// #if defined(PRE_ADD_REMOTE_OPENSHOP)
#if defined( PRE_ADD_LIMITED_SHOP )	
#if defined( PRE_FIX_74404 )
	m_LimitedShopBuyedItemList.clear();
#else // #if defined( PRE_FIX_74404 )
	m_LimitedShopBuyedItem.clear();
#endif // #if defined( PRE_FIX_74404 )
#endif // #if defined( PRE_ADD_LIMITED_SHOP )	
	memset(m_bPeriodQuestSchedule, 0, sizeof(m_bPeriodQuestSchedule));
	memset(m_dwPeriodQuestTick, 0, sizeof(m_dwPeriodQuestTick));

#if defined( PRE_ADD_TOTAL_LEVEL_SKILL )
	m_nTotalLevelSKillLevel = 0;
	memset(m_nTotalLevelSKillData, 0, sizeof(m_nTotalLevelSKillData));
	memset(m_bTotalLevelSkillCashSlot, 0, sizeof(m_bTotalLevelSkillCashSlot));
	memset(m_nTotalLevelSkillCashSlot, 0, sizeof(m_nTotalLevelSkillCashSlot));	
#endif
#if defined( PRE_ADD_NEWCOMEBACK )
	m_nComebackAppellation = 0;
#endif
#ifdef PRE_ADD_JOINGUILD_SUPPORT
	m_cGuildJoinLevel = 0;
	m_bWasRewardedGuildMaxLevel = true;
	m_bPartyBegginerGuild = false;
#endif		//#ifdef PRE_ADD_JOINGUILD_SUPPORT


	m_bFinalize = false;

#if defined(PRE_RECEIVEGIFTALL)
	m_nReceiveGiftPageCount = 0;
	m_VecReceiveGiftResultList.clear();
#endif	// #if defined(PRE_RECEIVEGIFTALL)

#if defined( PRE_ADD_STAMPSYSTEM )
	m_pStampSystem = new CDNStampSystem( static_cast<CDNUserSession*>(this) );
#endif // #if defined( PRE_ADD_STAMPSYSTEM )
#if defined(PRE_FIX_74387)
	m_bNowResellItem = false;
#endif
#if defined(PRE_ADD_CP_RANK)
	m_cStageClearBestRank = 0;
	m_nStageClearBestCP = 0;
#endif //#if defined(PRE_ADD_CP_RANK)
}

CDNUserBase::~CDNUserBase(void)
{
	SAFE_DELETE(m_pItem);
	SAFE_DELETE(m_pQuest);
	SAFE_DELETE(m_pCheatCommand);
	SAFE_DELETE(m_pGMCommand);
	SAFE_DELETE(m_pFriend);
	SAFE_DELETE(m_pIsolate);
	SAFE_DELETE(m_pMissionSystem);
	SAFE_DELETE(m_pEventSystem);
	SAFE_DELETE(m_pAppellation);
	SAFE_DELETE(m_pRestraint);
	SAFE_DELETE(m_pEffectRepository)
#if defined( PRE_ADD_NPC_REPUTATION_SYSTEM )
	SAFE_DELETE(m_pReputationSystem);
#endif // #if defined( PRE_ADD_NPC_REPUTATION_SYSTEM )
#if defined( PRE_ADD_SECONDARY_SKILL )
	SAFE_DELETE(m_pSecondarySkillRepository);
#endif // #if defined( PRE_ADD_SECONDARY_SKILL )
	SAFE_DELETE( m_pGesture );
	SAFE_DELETE(m_pTimeEventSystem);
	SAFE_DELETE(m_pCommonVariable);
#if defined (PRE_ADD_BESTFRIEND)
	SAFE_DELETE(m_pBestFriend);
#endif

#if defined( PRE_ADD_STAMPSYSTEM )
	SAFE_DELETE( m_pStampSystem );
#endif // #if defined( PRE_ADD_STAMPSYSTEM )

	SAFE_DELETE( m_pUserEventHandler );
}

void CDNUserBase::FinalizeEvent()
{
	if (!m_bFinalize)
	{
		m_bFinalize = true;
		m_pUserEventHandler->OnFinalize();
	}
}

void CDNUserBase::GuildWarReset()
{
	m_wGuildWarScheduleID = 0;
	m_nGuildWarRewardFestivalPoint = 0;		// Festival points to receive as reward
	m_nGuildWarRewardGuildPoint = 0;		// Guild points to receive as reward
	m_biGuildWarFestivalPoint = 0;		// Guild War festival points
}

void CDNUserBase::DelGuildWarFestivalPoint(INT64 biPoint)
{
	if (m_biGuildWarFestivalPoint < biPoint)
		return;

	m_biGuildWarFestivalPoint -= (int)biPoint;
}

#if defined( PRE_ADD_NPC_REPUTATION_SYSTEM )
bool CDNUserBase::LoadReputation( TAGetListNpcFavor* pA )
{
	for( UINT i=0 ; i<pA->cCount ; ++i )
	{
		m_pReputationSystem->SetNpcReputation( pA->ReputationArr[i].iNpcID, IReputationSystem::NpcFavor, pA->ReputationArr[i].iFavorPoint );
		m_pReputationSystem->SetNpcReputation( pA->ReputationArr[i].iNpcID, IReputationSystem::NpcMalice, pA->ReputationArr[i].iMalicePoint);
	}

	return true;
}
#endif // #if defined( PRE_ADD_NPC_REPUTATION_SYSTEM )

bool CDNUserBase::LoadUserData(TASelectCharacter *pSelect)
{
	m_nVehicleObjectID = m_nSessionID + 1;

	m_bIsSetSecondAuthPW	= pSelect->bIsSetSecondAuthPW;
	m_bIsSetSecondAuthLock	= pSelect->bIsSetSecondAuthLock;

	if (m_pRestraint->LoadRestraint(&pSelect->RestraintData) == false){
		//Can occur if blocked while playing the game
		m_pSession->DetachConnection(L"Connect|UserAccount Blocked");
		g_Log.Log(LogType::_NORMAL, m_pSession, L"Connect|[ADBID:%u, CDBID:%I64d, SID:%u] QUERY_SELECTCHARACTER UserAccount Blocked\r\n", pSelect->nAccountDBID, m_biCharacterDBID, m_nSessionID);
		return false;
	}

	m_UserData = pSelect->UserData;
#ifdef PRE_MOD_PVPRANK
	//Calculate rank!
	if (m_UserData.PvP.uiXP >= g_pDataManager->GetPvPExpThreshold())
	{
		int nRetLevel = g_pDataManager->GetRelativePvPRank(m_UserData.PvP.nExpAbsoluteRank, m_UserData.PvP.fExpRateRank);
		if (nRetLevel > 0)
#if defined(PRE_ADD_PVPLEVEL_MISSION)
			SetPvPLevel(static_cast<BYTE>(nRetLevel), false);
#else	
			m_UserData.PvP.cLevel = static_cast<BYTE>(nRetLevel);
#endif
		else
			g_Log.Log(LogType::_ERROR, m_pSession, L"Calclation Relative PvPRank Error! [%d][%d][%f][%d]\n", m_UserData.PvP.uiXP, m_UserData.PvP.nExpAbsoluteRank, m_UserData.PvP.fExpRateRank, nRetLevel);
	}
#endif		//#ifdef PRE_MOD_PVPRANK

	m_KeySetting = pSelect->KeySetting;
	m_PadSetting = pSelect->PadSetting;

	if (m_UserData.Status.cJobArray[0] <= 0) m_UserData.Status.cJobArray[0] = m_UserData.Status.cClass;
	if ((GetAccountLevel() >= AccountLevel_New) && (GetAccountLevel() <= AccountLevel_QA)) m_bHide = true;	// GMs are hidden by default.

	SetCharacterName(GetCharacterName());

	ChangeExp( 0, DBDNWorldDef::CharacterExpChangeCode::Admin, 0 );

	// GUILD
	m_GuildSelfView = pSelect->GuildSelfView;

	m_pItem->LoadUserData(pSelect);
	m_pMissionSystem->LoadUserData();
	m_pAppellation->LoadUserData();
	m_pTimeEventSystem->LoadUserData();

#if defined( PRE_ADD_STAMPSYSTEM )
	m_pStampSystem->LoadUserData();
#endif // #if defined( PRE_ADD_STAMPSYSTEM )

#ifdef PRE_ADD_JOINGUILD_SUPPORT
	m_cGuildJoinLevel = pSelect->cGuildJoinLevel;					//Guild join level (first time only)
	m_bWasRewardedGuildMaxLevel = pSelect->bWasRewardedGuildMaxLevel;		//Flag for whether max-level reward was received in guild
#endif		//#ifdef PRE_ADD_JOINGUILD_SUPPORT


	for( int i=0 ; i<DNNotifier::RegisterCount::Total ; ++i )
	{
		if( m_UserData.Status.NotifierData[i].eType == DNNotifier::Type::MainQuest && m_UserData.Status.NotifierData[i].iIndex == 0 )
		{
			m_UserData.Status.NotifierData[i].Clear();
		}
	}

	for( UINT i=0 ; i<DNNotifier::RegisterCount::Total ; ++i )
	{
		m_NotifierRepository.Register( i, m_UserData.Status.NotifierData[i] );
	}

	if (GetGuildUID().IsSet()) 
	{
		CDNGuildBase* pGuild = g_pGuildManager->At(GetGuildUID());
		if (!pGuild) 
		{
			if (m_pDBCon && m_pDBCon->GetActive())
			{
				m_pDBCon->QueryGetGuildInfo(m_pSession, GetGuildUID().nDBID, true);
				// Get active guild effects	
				m_pDBCon->QueryGetGuildRewardItem( m_cDBThreadID, m_nWorldSetID, GetAccountDBID(), GetGuildUID().nDBID );
			}
		}
#if defined(_VILLAGESERVER)
		else
		{
			bool bGuildCheck = true;
#if !defined( PRE_ADD_NODELETEGUILD )
			CDetachAutoEx<CDNGuildBase>	AutoDetach (pGuild);
			if (FALSE == pGuild->IsEnable())
				bGuildCheck = false;
#endif
#if defined(PRE_ADD_BEGINNERGUILD)
			if (bGuildCheck)
			{
				if (GetLevel() >= (int)CGlobalWeightTable::GetInstance().GetValue(CGlobalWeightTable::BeginnerGuild_GraduateLevel))
				{
					if (pGuild->GetInfo()->cGuildType == BeginnerGuild::Type::Beginner)
					{
						if (GetDBConnection())
							GetDBConnection()->QueryDelGuildMember(GetDBThreadID(), GetAccountDBID(), GetCharacterDBID(), GetAccountDBID(), GetCharacterDBID(), GetGuildUID().nDBID, GetLevel(), GetWorldSetID(), false, true);
					}
				}
			}
			else
				_DANGER_POINT();
#endif		//#if defined(PRE_ADD_BEGINNERGUILD)
#if defined(PRE_ADD_JOINGUILD_SUPPORT)
			if (bGuildCheck && m_bWasRewardedGuildMaxLevel == false && m_cGuildJoinLevel > 0)
			{
				if (GetLevel() >= (int)CGlobalWeightTable::GetInstance().GetValue(CGlobalWeightTable::PlayerLevelLimit))
				{
					int nMailID = g_pDataManager->GetRewardGuildSupportMailID(m_cGuildJoinLevel);
					if (nMailID > 0)
					{
						TGuildMember * pGuildMaster = pGuild->GetGuildMaster();
						if (pGuildMaster)
						{
							CDNMailSender::Process(pGuildMaster->nAccountDBID, pGuildMaster->nCharacterDBID, GetAccountDBID(), GetCharacterDBID(), GetCharacterName(), GetWorldSetID(), nMailID);
							m_bWasRewardedGuildMaxLevel = true;
						}
					}
				}
			}
#endif		//#if defined(PRE_ADD_JOINGUILD_SUPPORT)
		}
#endif		//#if defined(_VILLAGESERVER)
	}

	if (m_pDBCon && m_pDBCon->GetActive())
		m_pDBCon->QueryGetPeriodQuestDate(m_pSession);

#if defined(PRE_ADD_VIP)
	m_nVIPTotalPoint = pSelect->nVIPTotalPoint;
	m_tVIPEndDate = pSelect->tVIPEndDate;
	m_bVIPAutoPay = pSelect->bAutoPay;

	__time64_t Time;
	time(&Time);

	if (m_tVIPEndDate != -1){
		if (m_tVIPEndDate >= Time){
			m_bVIP = true;
			m_nVIPBonusExp = g_pDataManager->GetVIPExp(GetClassID(), GetLevel());

			int nRebirthMax = g_pDataManager->GetCoinCount(GetLevel(), GetWorldSetID());
			if (GetRebirthCoin() > nRebirthMax){
				m_cVIPRebirthCoin = GetRebirthCoin() - nRebirthMax;
				m_UserData.Status.cRebirthCoin = nRebirthMax;
			}
		}
		else{	// VIP completed/expired
			if (m_bVIPAutoPay){		// If auto-payment is selected
				if (g_pCashConnection->GetActive()){
					g_pCashConnection->SendBalanceInquiry(m_pSession, false, true);
				}
			}
		}
	}
#endif	// #if defined(PRE_ADD_VIP)

#if defined( PRE_ADD_TOTAL_LEVEL_SKILL )
	for( int i=0;i<TotalLevelSkill::Common::MAXSLOTCOUNT;i++ )	
	{
		if( m_nTotalLevelSKillData[i] )
		{
			if(g_pDataManager && !g_pDataManager->bIsTotalLevelSkillLimitLevel(i, m_nTotalLevelSKillData[i], GetTotalLevelSkillLevel(), GetLevel()))
				m_nTotalLevelSKillData[i] = 0;
		}		
	}
#endif

	// Schedule moved here
	if (m_UserData.Status.tLastFatigueDate < 0) m_UserData.Status.tLastFatigueDate = 0;
	if (m_UserData.Status.tLastRebirthCoinDate < 0) m_UserData.Status.tLastRebirthCoinDate = 0;
	if (m_UserData.Mission.tDailyMissionDate < 0) m_UserData.Mission.tDailyMissionDate = 0;
	if (m_UserData.Mission.tWeeklyMissionDate < 0) m_UserData.Mission.tWeeklyMissionDate = 0;
#if defined( PRE_ADD_MONTHLY_MISSION)
	if (m_UserData.Mission.tMonthlyMissionDate < 0) m_UserData.Mission.tMonthlyMissionDate = 0;
#endif	// #if defined( PRE_ADD_MONTHLY_MISSION)
	if (m_UserData.Status.tLastEventFatigueDate < 0) m_UserData.Status.tLastEventFatigueDate = 0;
	if (m_UserData.Status.tLastTimeEventDate < 0) m_UserData.Status.tLastTimeEventDate = 0;
#if defined( PRE_ADD_STAMPSYSTEM )
	if (m_UserData.Status.tLastStampDate < 0) m_UserData.Status.tLastStampDate = 0;
#endif // #if defined( PRE_ADD_STAMPSYSTEM )

	RegisterSchedule( CDNSchedule::ResetFatigue, m_UserData.Status.tLastFatigueDate );
	RegisterSchedule( CDNSchedule::ResetWeeklyFatigue, m_UserData.Status.tLastFatigueDate );
	RegisterSchedule( CDNSchedule::ResetRebirthCoin, m_UserData.Status.tLastRebirthCoinDate );
	RegisterSchedule( CDNSchedule::ResetDailyMission, m_UserData.Mission.tDailyMissionDate );
	RegisterSchedule( CDNSchedule::ResetWeeklyMission, m_UserData.Mission.tWeeklyMissionDate );
#if defined(PRE_ADD_MONTHLY_MISSION)
	RegisterSchedule( CDNSchedule::ResetMonthlyMission, m_UserData.Mission.tMonthlyMissionDate );
#endif	// #if defined(PRE_ADD_MONTHLY_MISSION)
	RegisterSchedule( CDNSchedule::ResetEventFatigue, m_UserData.Status.tLastEventFatigueDate );
	RegisterSchedule( CDNSchedule::ResetDailyTimeEvent, m_UserData.Status.tLastTimeEventDate );
#if defined( PRE_ADD_STAMPSYSTEM )
	RegisterSchedule( CDNSchedule::ResetWeeklyStamp, m_UserData.Status.tLastStampDate );
#endif // #if defined( PRE_ADD_STAMPSYSTEM )

	ResetRepeatQuest();

	g_pPeriodQuestSystem->BuildCompletedPeriodQuests(m_pQuest);

	return true;
}

#if defined(PRE_ADD_MUTE_USERCHATTING)
bool CDNUserBase::IsMuteChatting()
{
	INT64 biBlindTime = 0;
	if(!m_pCommonVariable->GetDataValue(CommonVariable::Type::MuteChatting, biBlindTime))
		return false;
	if(biBlindTime <= 0)
		return false;

	__time64_t _blindStartTime;
	m_pCommonVariable->GetDataModDate(CommonVariable::Type::MuteChatting, _blindStartTime);

	CTimeSet tCurTime;
	tCurTime.Reset();

	CTimeSet tMuteTime(_blindStartTime, true);
	tMuteTime.AddSecond(biBlindTime * 60);

	if((tMuteTime - tCurTime) > 0)
		return true;

	//If less than 0, chat ban time has expired. Reset.
	m_pCommonVariable->ModData(CommonVariable::Type::MuteChatting, 0);
	return false;
}
#endif	// #if defined(PRE_ADD_MUTE_USERCHATTING)

bool CDNUserBase::ModCommonVariableData( CommonVariable::Type::eCode Type, INT64 biValue, __time64_t tDate/* = 0*/ )
{
	return m_pCommonVariable->ModData(Type, biValue, tDate);
}

bool CDNUserBase::GetCommonVariableDataValue( CommonVariable::Type::eCode Type, INT64& biValue )
{
	return m_pCommonVariable->GetDataValue(Type, biValue);
}

bool CDNUserBase::IncCommonVariableData( CommonVariable::Type::eCode Type, __time64_t tDate/* = 0*/ )
{
	return m_pCommonVariable->IncData(Type, tDate);
}

void CDNUserBase::UpdateAttendanceEventData(bool bSendResult)
{
 	INT64 biValue  = 0;
	__time64_t tLastModifyDate = 0;
	m_pCommonVariable->GetDataValue(CommonVariable::Type::AttendanceEvent, biValue);
	m_pCommonVariable->GetDataModDate(CommonVariable::Type::AttendanceEvent, tLastModifyDate);
	
	DWORD iDoneDayIndex = LODWORD(biValue);	
	DWORD iOngoingDay = HIDWORD(biValue);

	WORD iDoneDay = LOWORD(iDoneDayIndex);	//Completion date
	WORD iIndex = HIWORD(iDoneDayIndex);		//Current event round

	//Before attendance event round check, everyone is at round 0, so force change to round 1 
	if (iIndex == 0)
		iIndex = 1;

	if( !g_pDataManager->CheckEveryDayEventTime(&iIndex) )
	{
		iDoneDay = 0;
		iOngoingDay = 0;
		tLastModifyDate = 0;
	}

	bool bCheckAttendanceFirst = false;
	int nGapDay = GetDateGap(tLastModifyDate);
 	
 	if( nGapDay == 1 ) //One day has passed since the last check
 	{
		iOngoingDay++;
		if( iOngoingDay > iDoneDay )
		{			
			iDoneDay = (WORD)iOngoingDay;
			iDoneDayIndex = MAKELONG(iDoneDay, iIndex);
 			if( CheckAttendanceEventDone(iOngoingDay) )
			{				
	 			ModCommonVariableData(CommonVariable::Type::AttendanceEvent, MAKELONG64(iDoneDayIndex, iOngoingDay));
				bCheckAttendanceFirst = true;
			}
			else
				ModCommonVariableData(CommonVariable::Type::AttendanceEvent, MAKELONG64(iDoneDayIndex, iOngoingDay));
		}
		else
		{
			ModCommonVariableData(CommonVariable::Type::AttendanceEvent, MAKELONG64(iDoneDayIndex, iOngoingDay));
		}
 	}
	else if( nGapDay == 0)
	{
		//For test users who logged in past midnight, attendance event got messed up, so force-reset it from the beginning
		if( iOngoingDay == 0 )
		{
			iOngoingDay = 1;
			iDoneDay = 1;
			iDoneDayIndex = MAKELONG(iDoneDay, iIndex);
		}
		ModCommonVariableData(CommonVariable::Type::AttendanceEvent, MAKELONG64(iDoneDayIndex, iOngoingDay), tLastModifyDate);
		bCheckAttendanceFirst = false;

	}
 	else if( nGapDay > 1 )//First time or more than a day since the last check
 	{
 		if(  CheckAttendanceEventDone(1) ) // Modified to always start receiving gifts from the beginning again
		{
			iDoneDay = 1;
			iDoneDayIndex = MAKELONG(iDoneDay, iIndex);
			ModCommonVariableData(CommonVariable::Type::AttendanceEvent, MAKELONG64(iDoneDayIndex, 1));
			bCheckAttendanceFirst = true;
		}		
 	}
	else
	{
		if( iDoneDay == 0 && CheckAttendanceEventDone(1) ) // If iDoneDay is 0, it is the initial setup, so CheckAttendanceEventDone() must be called 			
		{
			iDoneDay = 1;
			iDoneDayIndex = MAKELONG(iDoneDay, iIndex);
			ModCommonVariableData(CommonVariable::Type::AttendanceEvent, MAKELONG64(iDoneDayIndex, 1));
			bCheckAttendanceFirst = true;
		}
		else
		{			
			g_Log.Log(LogType::_ERROR, m_pSession, L"[UpdateAttendanceEventData] GapDay:%d", nGapDay);
		}		
	}

	if (bSendResult)
		SendAttendanceEventResult( bCheckAttendanceFirst );
}

int CDNUserBase::GetDateGap(__time64_t tLastModifyTime)
{
	if( 0 >= tLastModifyTime ) return -1;

	// Compare dates
	tm pCurTime, pLastTime;
	time_t Time;
	time(&Time);

	pCurTime = *localtime(&Time);
	pLastTime = *localtime(&tLastModifyTime);

	int nYear = 0;
	if ((pCurTime.tm_year - pLastTime.tm_year)  > 0 ) 
		nYear = 365;
	return(pCurTime.tm_yday + nYear - pLastTime.tm_yday);
}

bool CDNUserBase::CheckAttendanceEventDone(int iOngoingDay)
{
	TEveryDayEventData* pData = g_pDataManager->GetEveryDayEventRewardMailID(iOngoingDay);
	if( pData )
	{
		if (pData->nMailID > 0)
			CDNMailSender::Process( m_pSession, pData->nMailID, DBDNWorldDef::PayMethodCode::Event, 0 );
		if (pData->nCashMailID > 0)
			CDNMailSender::Process( m_pSession, pData->nCashMailID, DBDNWorldDef::PayMethodCode::Event, 0 );

		return true;
	}

	return false;
}

bool CDNUserBase::SaveUserData()
{
	if (GetLastVillageGateNo() < 0) SetLastVillageGateNo(0);
	m_pItem->SaveUserData();
	return true;
}

void CDNUserBase::DoUpdate(DWORD dwCurTick)
{


	if( !m_pDBCon || m_pDBCon->GetDelete() )
	{
		if( m_pSession )
			m_pSession->DetachConnection( L"DB M/W Disconnect!" );			
	}

#if defined( _GAMESERVER )
	// #Session dump fix
	// On game server, DB IP info is sent before Tcp Connection is established, so do not run scheduler before Tcp connects.
	if( m_pSession == NULL || m_pSession->GetIp() == NULL )
		return;
#endif // #if defined( _GAMESERVER )

	CDNSchedule::Process( dwCurTick );
	m_pTimeEventSystem->Process( dwCurTick );

	if( bIsCheckPing() == true )
	{
		if( dwCurTick-m_dwCheckTcpPing >= CHECK_TCP_PING_TICK )
		{
			m_dwCheckTcpPing = dwCurTick;
			if( m_pSession )
				m_pSession->SendTcpPing( dwCurTick );
		}
	}

	ProcessPeriodQuest(dwCurTick);
	m_pItem->DoUpdate(dwCurTick);
}

bool CDNUserBase::CheckDBConnection()
{
	if( m_pDBCon && m_pDBCon->GetActive() )
		return true;

	return false;
}

void CDNUserBase::CashMessageProcess(int nMainCmd, int nSubCmd, char *pData, int nLen)
{
	switch(nSubCmd)
	{
	case CASH_BALANCEINQUIRY:
		{
			const TACashBalanceInquiry *pCash = (TACashBalanceInquiry*)pData;

			if (pCash->nResult == ERROR_NONE){
				SetCashBalance(pCash->nCashBalance);
				SetPetal(pCash->nPetal);

#if defined(PRE_ADD_VIP)
				if (pCash->bServer){
					int nAutoPayItemSN = g_pDataManager->GetVIPAutoPayItemSN();
					if (CheckEnoughCashBalance(g_pDataManager->GetCashCommodityVIPAutomaticPaySalePrice(nAutoPayItemSN))){
						if (g_pCashConnection->GetActive()){
							g_pCashConnection->SendVIPBuy(m_pSession, nAutoPayItemSN, true);
						}
					}
					else{	// Send mail when cash is insufficient
						int nMailID = g_pDataManager->GetVIPShortCashMailID();
						if (nMailID > 0)
							CDNMailSender::Process((CDNUserSession*)this, nMailID, 0, 0);
					}
					return;
				}
#endif	// #if defined(PRE_ADD_VIP)
			}
			else{
				g_Log.Log(LogType::_NORMAL, m_pSession, L"[CASH_BALANCEINQUIRY] Result:%d", pCash->nResult);
				if (pCash->bOpen) 
					IsWindowStateNoneSet(WINDOW_CASHSHOP);
			}

			if (pCash->bOpen)
			{
				//Send when opening cash shop if present, send only once
				if (m_bSaleAbortListSended == false && g_pCashConnection->m_cSaleAbortCount > 0)
				{
					SendSaleAbortList(g_pCashConnection->m_cSaleAbortCount, g_pCashConnection->m_nSaleAbortList);
					m_bSaleAbortListSended = true;
				}

				SendCashShopOpen(pCash->nCashBalance, pCash->nPetal, pCash->nResult, m_wszAccountName);
			}
			else{
				SendCashShopBalanceInquiry(pCash->nResult, pCash->nPetal, pCash->nCashBalance);
			}
		}
		break;

	case CASH_BUY:
		{
			const TACashBuy *pCash = (TACashBuy*)pData;

			int nResult = pCash->nResult;
			TCashShopInfo Info[PREVIEWCARTLISTMAX] = { 0, };
#if defined(_VILLAGESERVER) && defined(PRE_ADD_CASH_REFUND)
			TPaymentItemInfo PaymentList[PREVIEWCARTLISTMAX] = { 0, };				
#endif
			int nCount = 0;
			if (nResult == ERROR_NONE){
				int nPrice = 0;
				char cPaymentRules = Cash::PaymentRules::None;
				cPaymentRules = pCash->cPaymentRules;
#if defined(PRE_ADD_SALE_COUPON)
				if( pCash->biSaleCouponSerial > 0)				
					m_pItem->DeleteCashInventoryBySerial(pCash->biSaleCouponSerial, 1, true );				
#endif
				for (int i = 0; i < pCash->cProductCount; i++){
					if (pCash->BuyList[i].bFail) continue;	// If failed, skip for now
					nPrice = g_pDataManager->GetCashCommodityPrice(pCash->BuyList[i].nItemSN);

					if (pCash->cPaymentRules != Cash::PaymentRules::Petal)
						DelCashByPaymentRules(nPrice, cPaymentRules); 

#if defined(_VILLAGESERVER) && defined(PRE_ADD_CASH_REFUND) //Items that succeeded and entered the cash inventory..
					if( !m_pItem->GetCashMoveInven() && !g_pDataManager->GetCashCommodityNoRefund(pCash->BuyList[i].CashItem.nItemID, pCash->BuyList[i].nItemSN) )
					{
						m_pItem->AddPaymentItem(pCash->ItemList[nCount]);						
						memcpy(&PaymentList[nCount], &pCash->ItemList[nCount].PaymentItemInfo, sizeof(TPaymentItemInfo) );
						++nCount;
					}
					else
#endif
					{
						m_pItem->ApplyCashShopItem(pCash->BuyList[i]);
					}
				}
				SetPetal(pCash->nPetalBalance);

				// Send only the failed list to the client
				nCount = 0;
				for (int i = 0; i < pCash->cProductCount; i++){
					if (!pCash->BuyList[i].bFail) continue;
					Info[nCount].cSlotIndex = pCash->BuyList[i].cCartIndex;
					Info[nCount].nItemSN = pCash->BuyList[i].nItemSN;
					Info[nCount].nItemID = pCash->BuyList[i].CashItem.nItemID;
					nCount++;
				}

				if (nCount > 0) 
					nResult = ERROR_CASHSHOP_CALLGM;	// Return error value if even 1 failure occurs
			}

			switch(nResult)
			{
			case ERROR_GENERIC_INVALIDREQUEST:
			case 101147:	// 101147 = Limited item. Exceeded maximum purchasable quantity.
			case 101190:	// 101190 = Exceeds the maximum number of character slots you can own.
			case ERROR_NONE:
			case ERROR_CASHSHOP_PROHIBITSALE:
			case ERROR_CASHSHOP_COUNTOVER:
				{
					INT64 nSeedPoint = 0;

#if defined(_VILLAGESERVER) && defined(PRE_ADD_CASH_REFUND)
					SendCashShopBuy((UINT)GetCashBalance(), GetPetal(), nResult, pCash->cCartType, nCount, Info, PaymentList, nSeedPoint);
#else
					SendCashShopBuy((UINT)GetCashBalance(), GetPetal(), nResult, pCash->cCartType, nCount, Info, NULL, nSeedPoint);
#endif
				}
				break;

			default:
				SendCompleteDetachMsg( nResult, L"ERROR_CASHSHOP_CALLGM" );
				g_Log.Log(LogType::_ERROR, m_pSession, L"[CASH_BUY] Fail!!!!(Ret:%d)\n", nResult);
				break;
			}
		}
		break;

	case CASH_PACKAGEBUY:
		{
			const TACashPackageBuy *pCash = (TACashPackageBuy*)pData;

			if (pCash->nResult == ERROR_NONE)
			{	// Can only buy one bundle of a package
				int nPrice = g_pDataManager->GetCashCommodityPrice(pCash->nPackageSN);
#if defined(PRE_ADD_SALE_COUPON)
				if( pCash->biSaleCouponSerial > 0)	
					m_pItem->DeleteCashInventoryBySerial(pCash->biSaleCouponSerial, 1, true );				
#endif
#if defined(_VILLAGESERVER) && defined(PRE_ADD_CASH_REFUND) // If not direct-to-cash-inventory and refundable, add to payment inventory.
				if( !m_pItem->GetCashMoveInven() && !g_pDataManager->GetCashCommodityNoRefund(0, pCash->nPackageSN))
				{
					m_pItem->AddPaymentPackageItem(pCash->PaymentItem);
				}
				else
#endif
				{
					for (int i = 0; i < pCash->cPackageCount; i++)
					{
						m_pItem->ApplyCashShopItem(pCash->BuyList[i]);
					}
				}

				char cPaymentRules = Cash::PaymentRules::None;
				cPaymentRules = pCash->cPaymentRules;

				if (pCash->cPaymentRules != Cash::PaymentRules::Petal)
					DelCashByPaymentRules(nPrice, cPaymentRules); 

				SetPetal(pCash->nPetalBalance);
			}


			INT64 nSeedPoint = 0;

#if defined(_VILLAGESERVER) && defined(PRE_ADD_CASH_REFUND)
			SendCashShopPackageBuy(pCash->nPackageSN, (UINT)GetCashBalance(), GetPetal(), pCash->nResult, &pCash->PaymentItem.PaymentPackageItemInfo, nSeedPoint);
#else
			SendCashShopPackageBuy(pCash->nPackageSN, (UINT)GetCashBalance(), GetPetal(), pCash->nResult, NULL, nSeedPoint);
#endif
		}
		break;

	case CASH_GIFT:
		{
			const TACashGift *pCash = (TACashGift*)pData;

			int nResult = pCash->nResult;
			TCashShopInfo Info[PREVIEWCARTLISTMAX] = { 0, };
			int nCount = 0;
			if (nResult == ERROR_NONE){
				int nPrice = 0;
				for (int i = 0; i < pCash->cGiftCount; i++){
					if (pCash->GiftList[i].bFail){
						Info[nCount].cSlotIndex = pCash->GiftList[i].cCartIndex;
						Info[nCount].nItemSN = pCash->GiftList[i].nItemSN;
						Info[nCount].nItemID = pCash->GiftList[i].nItemID;
						nCount++;

						continue;	// If failed, skip for now
					}
					nPrice = g_pDataManager->GetCashCommodityPrice(pCash->GiftList[i].nItemSN);
					DelCashBalance(nPrice);
				}
				SetPetal(pCash->nPetalBalance);

				if (nCount > 0)
					nResult = ERROR_NEXONBILLING_99;	// Return error value if even 1 failure occurs

				if (pCash->nReceiverAccountDBID != m_nAccountDBID){
#if defined(_VILLAGESERVER)
					CDNUserSession *pUserSession = g_pUserSessionManager->FindUserSessionByAccountDBID(pCash->nReceiverAccountDBID);	// Find the recipient
					if (pUserSession && (pUserSession->GetCharacterDBID() == pCash->biReceiverCharacterDBID)){	// If on the same server, just process directly
						pUserSession->SendCashshopNotifyGift(true, pCash->nReceiverGiftCount);	// Notify that a gift has arrived
					}
					else {	// If none, set to master 
						g_pMasterConnection->SendNotifyGift(pCash->nReceiverAccountDBID, pCash->biReceiverCharacterDBID, true, pCash->nReceiverGiftCount);
					}
#elif defined(_GAMESERVER)
					// g_pMasterConnection->SendNotifyGift(pCash->nReceiverAccountDBID, pCash->biReceiverCharacterDBID, true, pCash->nReceiverGiftCount);
#endif
				}
			}


			INT64 nSeedPoint = 0;

#if defined(PRE_ADD_CADGE_CASH)
			if( nResult == ERROR_NONE && pCash->nMailDBID > 0 )
				GetEventSystem()->OnEvent( EventSystem::OnGiftToCadger );
#endif
			SendCashShopGift((UINT)GetCashBalance(), GetPetal(), nResult, pCash->cCartType, nCount, Info, nSeedPoint);
		}
		break;

	case CASH_PACKAGEGIFT:
		{
			const TACashPackageGift *pCash = (TACashPackageGift*)pData;

			if (pCash->nResult == ERROR_NONE){	// Can only buy one bundle of a package
				int nPrice = g_pDataManager->GetCashCommodityPrice(pCash->nPackageSN);
				DelCashBalance(nPrice);
				SetPetal(pCash->nPetalBalance);
			}

			if (pCash->nReceiverAccountDBID != m_nAccountDBID){
#if defined(_VILLAGESERVER)
				CDNUserSession *pUserSession = g_pUserSessionManager->FindUserSessionByAccountDBID(pCash->nReceiverAccountDBID);	// Find the recipient
				if (pUserSession && (pUserSession->GetCharacterDBID() == pCash->biReceiverCharacterDBID)){	// If on the same server, just process directly
					pUserSession->SendCashshopNotifyGift(true, pCash->nReceiverGiftCount);	// Notify that a gift has arrived
				}
				else {	// If none, set to master 
					g_pMasterConnection->SendNotifyGift(pCash->nReceiverAccountDBID, pCash->biReceiverCharacterDBID, true, pCash->nReceiverGiftCount);
				}
#elif defined(_GAMESERVER)
				// g_pMasterConnection->SendNotifyGift(pCash->nReceiverAccountDBID, pCash->biReceiverCharacterDBID, true, pCash->nReceiverGiftCount);
#endif
			}


			INT64 nSeedPoint = 0;

#if defined(PRE_ADD_CADGE_CASH)
			if( pCash->nResult == ERROR_NONE && pCash->nMailDBID > 0 )
				GetEventSystem()->OnEvent( EventSystem::OnGiftToCadger );
#endif
			SendCashShopPackageGift(pCash->nPackageSN, (UINT)GetCashBalance(), GetPetal(), pCash->nResult, nSeedPoint);
		}
		break;

	case CASH_COUPON:
		{
			const TACashCoupon *pCash = (TACashCoupon*)pData;
			SendCoupon(pCash->nResult);
		}
		break;

#if defined(PRE_ADD_VIP)
	case CASH_VIPBUY:
		{
			const TACashVIPBuy *pCash = (TACashVIPBuy*)pData;
			
			if (pCash->nResult == ERROR_NONE){
				SetPetal(pCash->nPetal);
				// SetCashBalance(pCash->nCash);
				int nPrice = g_pDataManager->GetCashCommodityPrice(pCash->nItemSN);
				DelCashBalance(nPrice);

				if (!m_bVIP){	// If already VIP, no need to give fatigue again
					m_bVIP = true;	// Set for value update
					SetDefaultVIPFatigue();	// Set VIP fatigue
				}

				m_bVIP = true;
				m_nVIPTotalPoint = pCash->nVIPTotalPoint;
				m_tVIPEndDate = pCash->tVIPEndDate;
				m_bVIPAutoPay = pCash->bAutoPay;

				_MakeGiftVIPMonthItemByItemSN(pCash->nItemSN);
			}

			INT64 nSeedPoint = 0;

			if (pCash->bServer)
				SendVIPInfo(m_nVIPTotalPoint, m_tVIPEndDate, m_bVIPAutoPay, m_bVIP);
			else
				SendVIPBuy((UINT)GetCashBalance(), pCash->nPetal, pCash->nResult, pCash->nVIPTotalPoint, pCash->tVIPEndDate, m_bVIPAutoPay, nSeedPoint);

		}
		break;

	case CASH_VIPGIFT:
		{
			const TACashVIPGift *pCash = (TACashVIPGift*)pData;

			if (pCash->nResult == ERROR_NONE){
				SetPetal(pCash->nPetal);
				//SetCashBalance(pCash->nCash);
				int nPrice = g_pDataManager->GetCashCommodityPrice(pCash->nItemSN);
				DelCashBalance(nPrice);
			}

			if (pCash->nReceiverAccountDBID != m_nAccountDBID){
#if defined(_VILLAGESERVER)
				CDNUserSession *pUserSession = g_pUserSessionManager->FindUserSessionByAccountDBID(pCash->nReceiverAccountDBID);	// Find the recipient
				if (pUserSession && (pUserSession->GetCharacterDBID() == pCash->biReceiverCharacterDBID)){	// If on the same server, just process directly
					pUserSession->SendCashshopNotifyGift(true, pCash->nReceiverGiftCount);	// Notify that a gift has arrived
				}
				else {	// If none, set to master 
					g_pMasterConnection->SendNotifyGift(pCash->nReceiverAccountDBID, pCash->biReceiverCharacterDBID, true, pCash->nReceiverGiftCount);
				}
#elif defined(_GAMESERVER)
				// g_pMasterConnection->SendNotifyGift(pCash->nReceiverAccountDBID, pCash->biReceiverCharacterDBID, true, pCash->nReceiverGiftCount);
#endif
			}

			INT64 nSeedPoint = 0;

			SendVIPGift((UINT)GetCashBalance(), pCash->nPetal, pCash->nResult, nSeedPoint);
		}
		break;
#endif	// #if defined(PRE_ADD_VIP)

#if defined(_VILLAGESERVER) && defined(PRE_ADD_CASH_REFUND)
	case CASH_MOVE_CASHINVEN:
		{
			const TACashMoveCashInven* pCash = (TACashMoveCashInven*)pData;
			if( pCash->nResult == ERROR_NONE )
			{
				TPaymentItemInfoEx* PaymentItemInfo = m_pItem->GetPaymentItem(pCash->biPurchaseOrderDetailID);
				if( PaymentItemInfo ) //In big trouble without this.
				{
					m_pItem->ApplyCashShopItem(pCash->CashItem);

					//Delete it..
					m_pItem->DelPaymentItem(pCash->biPurchaseOrderDetailID);
					SetPetal(pCash->nTotalPetal);
					INT64 nSeedPoint = 0;
					SendCashShopMoveCashinven(pCash->biPurchaseOrderDetailID, ERROR_NONE, pCash->nTotalPetal, nSeedPoint);
					break;
				}
			}
			SendCashShopMoveCashinven(0, pCash->nResult, 0, 0);
		}		
		break;

	case CASH_PACKAGE_MOVE_CASHINVEN:
		{
			const TACashPackageMoveCashInven* pCash = (TACashPackageMoveCashInven*)pData;
			if( pCash->nResult == ERROR_NONE )
			{
				TPaymentPackageItemInfoEx* PaymentPackageItemInfo = m_pItem->GetPaymentPackageItem(pCash->biPurchaseOrderDetailID);
				if( PaymentPackageItemInfo ) //In big trouble without this.
				{
					for (int i = 0; i < PACKAGEITEMMAX; i++)
					{
						if( pCash->CashItemList[i].CashItem.nItemID == 0)
							break;

						m_pItem->ApplyCashShopItem(pCash->CashItemList[i]);
					}
					//Delete it..
					m_pItem->DelPaymentPackageItem(pCash->biPurchaseOrderDetailID);
					SetPetal(pCash->nTotalPetal);
					INT64 nSeedPoint = 0;
					SendCashShopMoveCashinven(pCash->biPurchaseOrderDetailID, ERROR_NONE, pCash->nTotalPetal, nSeedPoint);
					break;
				}
			}	
			SendCashShopMoveCashinven(0, pCash->nResult, 0, 0);
		}
		break;

	case CASH_REFUNDCASH:
		{
			const TACashRefund* pCash = (TACashRefund*)pData;
			if (pCash->nResult == ERROR_NONE)
			{
				if( pCash->cItemType == 1) // Normal
				{
					TPaymentItemInfoEx* PaymentItemInfo = m_pItem->GetPaymentItem(pCash->biPurchaseOrderDetailID);
					if( PaymentItemInfo ) //In big trouble without this.
					{	
						// Apply refund cash.
						AddCashBalance(PaymentItemInfo->nPrice);
						//Delete it..
						m_pItem->DelPaymentItem(pCash->biPurchaseOrderDetailID);

						SendCashShopCashRefund(pCash->biPurchaseOrderDetailID, ERROR_NONE, (int)GetCashBalance());
						break;
					}
				}
				else if( pCash->cItemType == 2) // Package
				{
					TPaymentPackageItemInfoEx* PaymentPackageItemInfo = m_pItem->GetPaymentPackageItem(pCash->biPurchaseOrderDetailID);
					if( PaymentPackageItemInfo ) //In big trouble without this.
					{
						// Apply refund cash.
						AddCashBalance(PaymentPackageItemInfo->nPrice);
						//Delete it..
						m_pItem->DelPaymentPackageItem(pCash->biPurchaseOrderDetailID);
						SendCashShopCashRefund(pCash->biPurchaseOrderDetailID, ERROR_NONE, (int)GetCashBalance());
						break;
					}
				}
			}
			SendCashShopCashRefund(pCash->biPurchaseOrderDetailID, pCash->nResult, (int)GetCashBalance() );
		}
		break;
#endif
	}
}

void CDNUserBase::DBMessageProcess(int nMainCmd, int nSubCmd, char *pData, int nLen)
{
	DelDataBaseMessageSequence(nMainCmd, nSubCmd);
	
	switch(nMainCmd)
	{
	case MAINCMD_AUTH: OnDBRecvAuth(nSubCmd, pData); break;
	case MAINCMD_STATUS: OnDBRecvCharInfo(nSubCmd, pData); break;
	case MAINCMD_ETC: OnDBRecvEtc(nSubCmd, pData); break;
	case MAINCMD_QUEST: OnDBRecvQuest(nSubCmd, pData); break;
	case MAINCMD_MISSION: OnDBRecvMission(nSubCmd, pData); break;
	case MAINCMD_SKILL: OnDBRecvSkill(nSubCmd, pData); break;
	case MAINCMD_FRIEND: OnDBRecvFriend(nSubCmd, pData); break;
	case MAINCMD_ISOLATE: OnDBRecvIsolate(nSubCmd, pData); break;
	case MAINCMD_PVP: OnDBRecvPvP(nSubCmd, pData); break;
	case MAINCMD_DARKLAIR: OnDBRecvDarkLair(nSubCmd, pData); break;
	case MAINCMD_GUILD: OnDBRecvGuild(nSubCmd, pData); break;
	case MAINCMD_MAIL: OnDBRecvMail(nSubCmd, pData); break;
	case MAINCMD_MARKET: OnDBRecvMarket(nSubCmd, pData); break;
	case MAINCMD_ITEM: OnDBRecvItem(nSubCmd, pData); break;
	case MAINCMD_CASH: OnDBRecvCash(nSubCmd, pData); break;
	case MAINCMD_MSGADJUST: OnDBRecvMsgadjustment(nSubCmd, pData); break;
#if defined( PRE_ADD_NPC_REPUTATION_SYSTEM )
	case MAINCMD_REPUTATION: OnDBRecvReputation(nSubCmd, pData); break;
#endif // #if defined( PRE_ADD_NPC_REPUTATION_SYSTEM )
	case MAINCMD_MASTERSYSTEM: OnDBRecvMasterSystem( nSubCmd, pData ); break;
#if defined( PRE_ADD_SECONDARY_SKILL )
	case MAINCMD_SECONDARYSKILL: OnDBRecvSecondarySkill( nSubCmd, pData ); break;
#endif // #if defined( PRE_ADD_SECONDARY_SKILL )
	case MAINCMD_FARM: OnDBRecvFarm(nSubCmd, pData); break;
	case MAINCMD_GUILDRECRUIT: OnDBRecvGuildRecruit( nSubCmd, pData ); break;
#if defined (PRE_ADD_DONATION)
	case MAINCMD_DONATION: OnDBRecvDonation(nSubCmd, pData); break;
#endif // #if defined (PRE_ADD_DONATION)
#if defined( PRE_PARTY_DB )
	case MAINCMD_PARTY: OnDBRecvParty( nSubCmd, pData ); break;
#endif // #if defined( PRE_PARTY_DB )
#if defined (PRE_ADD_BESTFRIEND)
	case MAINCMD_BESTFRIEND: OnDBRecvBestFriend(nSubCmd, pData); break;
#endif // #if defined (PRE_ADD_BESTFRIEND)
#if defined (PRE_PRIVATECHAT_CHANNEL)
	case MAINCMD_PRIVATECHATCHANNEL: OnDBRecvPrivateChatChannel(nSubCmd, pData); break;
#endif // #if defined (PRE_PRIVATECHATCHANNEL)
#if defined( PRE_ADD_STAMPSYSTEM )
	case MAINCMD_STAMPSYSTEM: OnDBRecvStampSystem(nSubCmd, pData); break;
#endif // #if defined( PRE_ADD_STAMPSYSTEM )
#if defined( PRE_ALTEIAWORLD_EXPLORE )
	case MAINCMD_ALTEIAWORLD: OnDBRecvAlteiaWorld(nSubCmd, pData); break;
#endif
#if defined( PRE_ADD_DWC )
	case MAINCMD_DWC: OnDBRecvDWC(nSubCmd, pData); break;
#endif
	}
}

void CDNUserBase::OnDBRecvAuth(int nSubCmd, char *pData)
{
	switch(nSubCmd)
	{
	case QUERY_BEGINAUTH:
		{
			// Should never enter here.
		}
		break;

	case QUERY_STOREAUTH:	// VI -> CL (Enter dungeon) / VI -> CL (Reselect character)
		{
			const TAStoreAuth *pA = (TAStoreAuth*)pData;

			// Write log
			g_Log.Log((ERROR_NONE != pA->nRetCode)?(LogType::_ERROR):(LogType::_NORMAL), m_pSession, L"[Auth] StoreAuth(ACK) - Result:%d, nAccountDBID:%d(%s/%s), CertifyingKey:%I64d\r\n", pA->nRetCode, GetAccountDBID(), GetAccountName(), GetCharacterName(), GetCertifyingKey());

			if (ERROR_NONE != pA->nRetCode) {
				if( m_bCertified ) // If already authenticated, release it..
					m_bCertified = false;
				m_pSession->DetachConnection(L"QUERY_STOREAUTH Fail");
				break;
			}

#if defined(_VILLAGESERVER)
			switch(m_pSession->m_eUserState) {
				case STATE_READYTOVILLAGE: m_pSession->UpdateMoveToVillageUserData(); break;
				case STATE_READYTOGAME: m_pSession->UpdateMoveToGameUserData(); break;
				case STATE_READYTOLOGIN: m_pSession->ReconnectLogin(ERROR_NONE); break;
				default: 
					break;
			}
#elif defined(_GAMESERVER)

			// GA->VI, GA->LO reduce user fatigue
			m_pSession->DecreaseFatigue();

			switch(m_pSession->GetState()) 
			{
				case SESSION_STATE_READY_TO_VILLAGE:
					m_pSession->ChangeServerUserData();
					break;

				case SESSION_STATE_READY_TO_LOGIN:
					{
						m_pSession->SetSessionState(SESSION_STATE_RECONNECTLOGIN);
						SendReconnectLogin(ERROR_NONE, GetAccountDBID(), GetCertifyingKey());

					}
					break;
				default:	// Does a newly added condition exist?
					break;
			}
#endif
			m_bCertified = false;
		}
		break;

	case QUERY_CHECKAUTH:	// VI <-> DB (Initial login) / VI <-> DB (Reselect character) / VI <-> DB (Return to village)
		{
			const TACheckAuth *pA = (TACheckAuth*)pData;

			// Write log
			g_Log.Log((ERROR_NONE != pA->nRetCode)?(LogType::_ERROR):(LogType::_NORMAL), m_pSession, L"[Auth] CheckAuth(ACK) - Result:%d, nAccountDBID:%d, CertifyingKey:%I64d, CerServerID:%d\r\n", pA->nRetCode, GetAccountDBID(), GetCertifyingKey(), g_pAuthManager->GetServerID());

			if (ERROR_NONE != pA->nRetCode) {
#if defined(_GAMESERVER)
				m_pSession->SetSessionState(SESSION_STATE_GAME_PLAY);
#endif
				if( m_bCertified ) // If already authenticated, release it..
					m_bCertified = false;
				m_pSession->DetachConnection(L"CheckAuth Fail");
				break;
			}

			m_bCertified = true;
			m_cAge = pA->nAge;
			m_cDailyCreateCount = pA->nDailyCreateCount;
			m_nPrmInt1 = pA->nPrmInt1;
			m_cLastServerType = pA->cLastServerType;
#if defined(_VILLAGESERVER)
			if (g_pMasterConnection && g_pMasterConnection->GetActive())
				g_pMasterConnection->SendCheckUser(m_nSessionID);
			else
				m_pSession->DetachConnection(L"MasterCon Not Found");
#endif

		}
		break;

	case QUERY_RESETAUTH:
		{
			// Should never enter here.
		}
		break;
	}
}

void CDNUserBase::OnDBRecvCharInfo(int nSubCmd, char *pData)
{
	switch( nSubCmd )
	{
		case QUERY_GETLISTEFFECTITEM:
		{
			const TAEffectItem* pPacket = reinterpret_cast<TAEffectItem*>(pData);

			if( !GetEffectRepository() || !GetEffectRepository()->Add( pPacket->ItemInfo.biItemSerial, pPacket->ItemInfo.iItemID, pPacket->ItemInfo.ExpireDate ) )
			{
				g_Log.Log(LogType::_ERROR, m_pSession, L"QUERY_GETLISTEFFECTITEM ItemID:%d", pPacket->ItemInfo.iItemID );
				m_pSession->DetachConnection(L"QUERY_GETLISTEFFECTITEM");
			}
			break;
		}

		case QUERY_GETLIST_ETCPOINT:
			{
				const TAGetListEtcPoint *pPacket = reinterpret_cast<TAGetListEtcPoint*>(pData);
				if (ERROR_NONE != pPacket->nRetCode) return;
				SetEtcPointList( pPacket );
			}
			break;

		case QUERY_ADD_ETCPOINT:
			{
				const TAAddEtcPoint* pPacket = reinterpret_cast<TAAddEtcPoint*>(pData);
				if( pPacket->nRetCode != ERROR_NONE ) return;
				SetEtcPointEach(pPacket->cType, pPacket->biLeftPoint);
			}
			break;

		case QUERY_USE_ETCPOINT:
			{
				const TAUseEtcPoint* pPacket = reinterpret_cast<TAUseEtcPoint*>(pData);
				if( pPacket->nRetCode != ERROR_NONE ) return;
				SetEtcPointEach(pPacket->cType, pPacket->biLeftPoint);
			}
			break;

#ifdef _VILLAGESERVER
#ifdef PRE_ADD_CHANGEJOB_CASHITEM
		case QUERY_CHANGEJOBCODE:
			{
				const TAChangeJobCode* pPacket = reinterpret_cast<TAChangeJobCode*>(pData);
				int nChangeFirstJob = m_pSession->GetItem()->GetChangeFirstJobID();
				int nChangeSecondJob = m_pSession->GetItem()->GetChangeSecondJobID();
				if( pPacket->nRetCode == ERROR_NONE ) 
				{
					m_pSession->GetSkill()->OnResponseChangeJobCode( pPacket->wTotalSkillPoint );
					m_pSession->GetItem()->OnResponseChangeJobCode( true );

					CDNGuildBase* pGuild = g_pGuildManager->At(m_pSession->GetGuildUID());
					if (pGuild)
					{
#if !defined( PRE_ADD_NODELETEGUILD )
						CDetachAutoEx<CDNGuildBase>	AutoDetach(pGuild);
						if (FALSE == pGuild->IsEnable()) break;
#endif
						g_pGuildManager->MaModifyMembJob(pGuild, m_pSession->GetCharacterDBID(), m_pSession->GetStatusData()->cJob);					
					}

					if (g_pMasterConnection)
						g_pMasterConnection->SendChangeGuildMemberInfo(m_pSession->GetGuildUID(), m_pSession->GetAccountDBID(), m_pSession->GetCharacterDBID(), 0, 0, GUILDMEMBUPDATE_TYPE_JOBCODE, nChangeFirstJob, nChangeSecondJob, 0, 0);
				}
				else
				{
					m_pSession->GetItem()->OnResponseChangeJobCode( false );
				}
				
				// Send response to client.
				SendChangeJobCashItemRes( pPacket->nRetCode, pPacket->wTotalSkillPoint, nChangeFirstJob, nChangeSecondJob );
			}
			break;
#endif // #ifdef PRE_ADD_CHANGEJOB_CASHITEM
#endif // #ifdef _VILLAGESERVER

		case QUERY_GETLIST_VARIABLERESET:
		{
			const TAGetListVariableReset* pA = reinterpret_cast<TAGetListVariableReset*>(pData);
			bool bInitialize = false;
			if( pA->nRetCode == ERROR_NONE )
			{
				bInitialize = m_pCommonVariable->InitializeData( pA );
			}

			if( bInitialize == false )
			{
				g_Log.Log(LogType::_ERROR, m_pSession, L"QUERY_GETLIST_VARIABLERESET AccountID:%d Ret:%d\r\n", pA->nAccountDBID, pA->nRetCode );
				m_pSession->DetachConnection(L"QUERY_GETLIST_VARIABLERESET");
			}
			break;
		}

	case QUERY_OWNCHRACTERLEVEL:
		{
			const TAOwnChracterLevel * pPacket = (TAOwnChracterLevel*)pData;

			for (int i = 0; i < pPacket->cCount; i++)
			{
				m_OwnCharacterLevelList[i] = pPacket->Level[i];
			}
		}
		break;
#if defined( PRE_ADD_TOTAL_LEVEL_SKILL )
	case QUERY_GET_TOTALSKILLLEVEL:
		{
			const TAGetTotalLevelSkill * pPacket = (TAGetTotalLevelSkill*)pData;
			m_nTotalLevelSKillLevel = pPacket->nTotalSkillLevel;
			for(int i=0;i<TotalLevelSkill::Common::MAXSLOTCOUNT;i++)
			{
				if(pPacket->TotalSkill[i].nSkillID > 0)
				{					
					m_nTotalLevelSKillData[i] = pPacket->TotalSkill[i].nSkillID;
				}				
			}
		}
		break;	
#endif

#ifdef PRE_ADD_PRESET_SKILLTREE
	case QUERY_GET_SKILLPRESET_LIST:
		{
			TAGetSKillPreSetList * pPacket = (TAGetSKillPreSetList*)pData;
			if (m_pSession)
			{
				if (pPacket->nRetCode == ERROR_NONE)
					m_pSession->SendSkillPresetList(pPacket->SKillIndex, pPacket->SKills, pPacket->nRetCode);
				else
					m_pSession->SendSkillPresetList(NULL, NULL, pPacket->nRetCode);
			}
		}
		break;

	case QUERY_ADD_SKILLPRESET:
		{
			TAAddSkillPreSet * pPacket = (TAAddSkillPreSet*)pData;
			if (m_pSession)
				m_pSession->SendSkillPresetAddResult(pPacket->nRetCode);
		}
		break;

	case QUERY_DEL_SKILLPRESET:
		{
			TADelSkillPreSet * pPacket = (TADelSkillPreSet*)pData;
			if (m_pSession)
				m_pSession->SendSkillPresetDelResult(pPacket->nRetCode);
		}
		break;
#endif		//#ifdef PRE_ADD_PRESET_SKILLTREE
	}
}

void CDNUserBase::OnDBRecvEtc(int nSubCmd, char *pData)
{
	switch(nSubCmd)
	{
		case QUERY_DBRESULTERROR:
		{
			const TADBResultError *pResult = (TADBResultError*)pData;

#if defined( PRE_ADD_SECONDARY_SKILL )
			if( pResult->cMainCmd == MAINCMD_SECONDARYSKILL )
				_ASSERT(0);
#endif // #if defined( PRE_ADD_SECONDARY_SKILL )

			g_Log.Log(LogType::_ERROR, m_nWorldSetID, m_nAccountDBID, m_biCharacterDBID, m_nSessionID, L"QUERY_DBRESULTERROR [M:%d S:%d] Ret:%d", pResult->cMainCmd, pResult->cSubCmd, pResult->nRetCode);
			SendCompleteDetachMsg(ERROR_GENERIC_INVALIDREQUEST, L"QUERY_DBRESULTERROR" );	// Invalid request(?)
		}
		break;

		case QUERY_GETGAMEOPTION:
		{
			const TAGetGameOption * pPacket = (TAGetGameOption*)pData;
			
			if (pPacket->nRetCode == ERROR_NONE)
				memcpy_s(&m_GameOption, sizeof(TGameOptions), &pPacket->Option, sizeof(TGameOptions));
		}
		break;

		case QUERY_RESTRAINT:
		{
			const TARestraint * pPacket = (TARestraint*)pData;

			if (pPacket->nRetCode == ERROR_NONE)
			{
				TRestraintData Restraint;
				memset(&Restraint, 0, sizeof(TRestraintData));
				for (int i = 0;( i < RESTRAINTMAX ) && ( i < pPacket->cCount ); i++)
					memcpy(&Restraint.Restraint[i], &pPacket->restraint[i], sizeof(TRestraint));

				if (m_pRestraint->LoadRestraint(&Restraint) == false)
				{
					//If blocked user, disconnect immediately
					m_pSession->DetachConnection(L"QUERY_RESTRAINT UserBlocked");
					break;
				}
			}
			else
			{
				//How should sanction-related errors be handled... probably disconnect?
				m_pSession->DetachConnection(L"QUERY_RESTRAINT failed");
			}
		}
		break;

		case QUERY_VALIDATE_SECONDAUTH:
		{
			const TAValidateSecondAuth* pPacket = reinterpret_cast<TAValidateSecondAuth*>(pData);
			SendSecondAuthValidate(pPacket->nRetCode, pPacket->nAuthCheckType, pPacket->cFailCount);
			break;
		}

		case QUERY_GET_PROFILE:
			{
				const TAGetProfile* pPacket = (TAGetProfile*)pData;
				if( pPacket->nRetCode == ERROR_NONE )
				{
					memcpy_s( &m_Profile, sizeof(TProfile), &pPacket->sProfile, sizeof(TProfile) );
				}
			}
			break;

		case QUERY_SET_PROFILE:
			{
				const TASetProfile* pPacket = (TASetProfile*)pData;
				if( pPacket->nRetCode == ERROR_NONE )
				{
					memcpy_s( &m_Profile, sizeof(TProfile), &pPacket->sProfile, sizeof(TProfile) );
				}
			}
			break;

		case QUERY_GET_ABUSEMONITOR:
		{
			const TAGetAbuseMonitor* pPacket = reinterpret_cast<TAGetAbuseMonitor*>(pData);
			if( pPacket->nRetCode == ERROR_NONE )
			{
				m_iHackAbuseDBValue			= pPacket->iAbuseCount;
				m_iHackAbuseDBCallCount		= pPacket->iCallCount;
				m_iHackPlayRestraintValue	= pPacket->iPlayRestraintValue;
				m_iHackResetRestraintValue	= pPacket->iDBResetRestraintValue;
			}
			break;
		}

		case QUERY_GET_WHOLE_ABUSEMONITOR:
		{
#if defined(_GAMESERVER) && defined(PRE_ADD_ABUSE_ACCOUNT_RESTRAINT)
			const TAGetWholeAbuseMonitor * pPacket = (TAGetWholeAbuseMonitor*)pData;

			int nHackAbuseCharacterCnt = 0;
			if (pPacket->nRetCode == ERROR_NONE)
			{
				for (int i = 0; i < pPacket->cCount; i++)
				{
					if (pPacket->Abuse[i].iAbuseCount > pPacket->Abuse[i].iPlayRestraintValue)
					{
					 	if (pPacket->Abuse[i].biCharacterDBID != GetCharacterDBID())
							m_nHackAbuseCharacterCntWithoutMe++;
						nHackAbuseCharacterCnt++;
					}
				}
			}

			if (nHackAbuseCharacterCnt >= 2)
			{
				std::wstring wszRestraintReason = GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 100070);
				std::wstring wszRestraintReasonForDolis = GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 100071);
				GetDBConnection()->QueryAddRestraint(m_pSession, DBDNWorldDef::RestraintTargetCode::Account, DBDNWorldDef::RestraintTypeCode::ConnectRestraint, 
					wszRestraintReason.c_str(), wszRestraintReasonForDolis.c_str(), 9999, DBDNWorldDef::RestraintDolisReasonCode::AbuseRestraintCode);

				m_pSession->DetachConnection(L"AddAccountRestraint");
			}
#endif
			break;
		}
#if defined( PRE_ADD_GAMEQUIT_REWARD )
		case QUERY_MOD_NEWBIE_REWARDFLAG:
			{
				const TAHeader* pPacket = (TAHeader*)pData;
				if(pPacket->nRetCode== ERROR_NONE)
				{
					if(SetGameQuitRewardItem(TLevelupEvent::NewbieReconnectReward, DBDNWorldDef::AddMaterializedItem::NewbiewReConnectReward))
						g_Log.Log(LogType::_GAMEQUITREWARD, m_pSession, L"AccountDBID[%d] ReConnectNewbieReward Success\n", GetAccountDBID());
				}
				else
					g_Log.Log(LogType::_GAMEQUITREWARD, m_pSession, L"AccountDBID[%d] ReConnectNewbieReward Failed(Ret : %d)\n", GetAccountDBID(), pPacket->nRetCode);
			}
			break;
#endif	// #if defined( PRE_ADD_GAMEQUIT_REWARD )
#if defined(PRE_ADD_CP_RANK) && defined(_GAMESERVER)
		case QUERY_GET_STAGE_CLEAR_BEST :
			{
				const TAGetStageClearBest* pPacket = (TAGetStageClearBest*)pData;
				if( pPacket->nRetCode == ERROR_NONE )
				{
					CDNGameRoom * pGameRoom = m_pSession->GetGameRoom();
					if (pGameRoom)
					{
						pGameRoom->SetAbyssStageClearBest(pPacket);
					}						
				}
				else
					g_Log.Log(LogType::_ERROR, m_pSession, L"Get Stage Clear Best DBError(Ret : %d)\n", pPacket->nRetCode);
			}
			break;
		case QUERY_GET_STAGE_PERSONAL_BEST :
			{
				const TAGetStageClearPersonalBest* pPacket = (TAGetStageClearPersonalBest*)pData;
				if( pPacket->nRetCode == ERROR_NONE )
				{
					m_nStageClearBestCP = pPacket->nClearPoint;
					if( pPacket->Code > 0)					
						m_cStageClearBestRank = (char)pPacket->Code-DBDNWorldDef::ClearGradeCode::eCode::SSS;										
					else
						m_cStageClearBestRank = 0;
				}
				else
					g_Log.Log(LogType::_ERROR, m_pSession, L"Get Stage Personal Best DBError(Ret : %d)\n", pPacket->nRetCode);
			}
			break;
#endif //#if defined(PRE_ADD_CP_RANK) && defined(_GAMESERVER)
	}
}

void CDNUserBase::OnDBRecvQuest(int nSubCmd, char *pData)
{
	switch(nSubCmd)
	{
	case QUERY_DELETE_CHARACTER_PERIODQUEST:
		{
			const TADeletePeriodQuest * pPacket = (TADeletePeriodQuest*)pData;

			if (pPacket->nRetCode != ERROR_NONE)
			{
				g_Log.Log(LogType::_ERROR, L"QUERY_DELETE_CHARACTER_PERIODQUEST FAIL\r\n");
				break;
			}

			// Update user quest info.
			TQuestGroup* pQuest = GetQuestData();

			// Initialize info previously received via AddQuest.
			int nSlot = 0;
			int nQuestID = 0;
			for (int i=0; i<pPacket->nQuestCount; i++)
			{	
				nQuestID = pPacket->nQuestIDs[i];

				nSlot = GetQuest()->FindPlayingQuest(nQuestID);
				if( nSlot < 0  ) continue;

				pQuest->Quest[nSlot].nQuestID = 0;
				pQuest->Quest[nSlot].cQuestState = QuestState_None;
				ZeroMemory( &(pQuest->Quest[nSlot]), sizeof(TQuest));
			}

			// Newly apply timed quest info.
			g_pPeriodQuestSystem->RefreshPeriodQuest(pPacket->nPeriodQuestType, m_pQuest);

			// Update pending remote quest info awaiting acceptance.
			m_pQuest->CheckAcceptWaitRemoteQuestList();
			
			m_pQuest->SendAllQuestRefresh();
		}
		break;	

	// Get the time when timed quests were reset.
	case QUERY_GET_CHARACTER_PERIODQUESTDATE:
		{
			const TAGetPeriodQuestDate * pPacket = (TAGetPeriodQuestDate*)pData;

			if (pPacket->nRetCode != ERROR_NONE)
			{
				g_Log.Log(LogType::_ERROR, L"QUERY_GET_CHARACTER_PERIODQUESTDATE FAIL\r\n");
				break;
			}

			CheckResetPeriodQuest(pPacket);	// Register scheduling
		}
		break;

	// Process completed event quests
	case QUERY_GET_LISTCOMPLETE_EVENTQUEST:
		{
			const TAGetListCompleteEventQuest * pPacket = (TAGetListCompleteEventQuest*)pData;

			if (pPacket->nRetCode != ERROR_NONE)
			{
				g_Log.Log(LogType::_ERROR, L"QUERY_GET_LISTCOMPLETE_EVENTQUEST FAIL\r\n");
				break;
			}
			
			g_pPeriodQuestSystem->RefreshEventQuest(m_pQuest, pPacket->nCount, pPacket->EventQuestList);

			m_pQuest->SendAllQuestRefresh();
		}
		break;

	case QUERY_COMPLETE_EVENTQUEST:
		{
			const TACompleteEventQuest * pPacket = (TACompleteEventQuest*)pData;

			if (pPacket->nRetCode != ERROR_NONE)
			{
				g_Log.Log(LogType::_ERROR, L"QUERY_COMPLETE_EVENTQUEST FAIL\r\n");
				break;
			}
		}
		break;
	}
}

void CDNUserBase::OnDBRecvMission(int nSubCmd, char *pData)
{
	switch(nSubCmd)
	{
	case QUERY_CLEARMISSION :
		{
			const TAHeader* pPacket = (TAHeader*)pData;
			if(pPacket->nRetCode == ERROR_NONE )
			{
				wstring wszString = FormatW(L"Reset succeeded. Please return to character select and log back in.\r\n");
				SendChat( CHATTYPE_NORMAL, (int)wszString.size()*sizeof(WCHAR), L"", (WCHAR*)wszString.c_str() );					
			}
		}
		break;
	}
}

void CDNUserBase::OnDBRecvSkill(int nSubCmd, char *pData)
{
	switch(nSubCmd)
	{
		case QUERY_RESETSKILL:
		{
			const TAResetSkill *pUpdate = (TAResetSkill*)pData;
			switch(pUpdate->nRetCode)
			{
			case 103105:
			case ERROR_NONE:
				// Restore the SP that needs to be recovered.
				SetSkillPoint( pUpdate->wTotalSkillPoint, pUpdate->cSkillPage );
				SendSkillReset( pUpdate->wTotalSkillPoint, pUpdate->cSkillPage );
				break;

			default:
				g_Log.Log(LogType::_FILELOG, m_nWorldSetID, m_nAccountDBID, m_biCharacterDBID, m_nSessionID, L"[M:%d S:%d] Ret:%d", MAINCMD_SKILL, nSubCmd, pUpdate->nRetCode);
				m_pSession->DetachConnection(L"QUERY_DBRESULTERROR");
			}
		}
		break;

		case QUERY_RESETSKILLBYSKILLIDS:
		{
			const TAResetSkillBySkillIDS* pUpdate = reinterpret_cast<TAResetSkillBySkillIDS*>(pData);
			if( pUpdate->nRetCode == ERROR_NONE )
			{
				SetSkillPoint( pUpdate->nTotalSkillPoint, pUpdate->cSkillPage);
#ifdef _VILLAGESERVER
				m_pSession->GetSkill()->OnResponseSkillResetCashItemFromDBServer( true );
#endif
				SendUseSkillResetCashItemRes( pUpdate->nRetCode, pUpdate->nTotalSkillPoint, pUpdate->cSkillPage );
			}
			else
			{
#ifdef _VILLAGESERVER
				m_pSession->GetSkill()->OnResponseSkillResetCashItemFromDBServer( false );
#endif
				m_pSession->DetachConnection(L"QUERY_RESETSKILLBYSKILLIDS:ERROR");
			}
			break;
		}
#if defined( _VILLAGESERVER )
		case QUERY_USEEXPANDSKILLPAGE:
		{
			const TAUseExpandSkillPage* pUpdate = reinterpret_cast<TAUseExpandSkillPage*>(pData);
			if( pUpdate->nRetCode == ERROR_NONE	)
			{
				m_pSession->GetItem()->SetSkillPageCount( DualSkill::Type::MAX );	
				m_pSession->GetSkill()->ApplyExpendedSkillPage( DualSkill::Type::Secondary );
				SendExpandSkillPageResult(ERROR_NONE);
				SendSkillReset( m_pSession->GetSkillPoint( DualSkill::Type::Secondary ), DualSkill::Type::Secondary );
				SendSkill(m_UserData.Skill[DualSkill::Type::Secondary].SkillList, DualSkill::Type::Secondary);	
				SendSkillPageCount(m_pSession->GetItem()->GetSkillPageCount());
			}
			else
			{
				m_pSession->DetachConnection(L"QUERY_USEEXPANDSKILLPAGE:ERROR");
			}
		}
		break;
#endif
	}
}

void CDNUserBase::OnDBRecvFriend(int nSubCmd, char *pData)
{
	switch(nSubCmd)
	{
	case QUERY_FRIENDLIST:
		{
			const TAFriendList * pPacket = (TAFriendList*)pData;
			if (pPacket->nRetCode != ERROR_NONE)
			{
				SendFriendResult(ERROR_FRIEND_GETLISTFAIL);
				g_Log.Log(LogType::_ERROR, m_pSession, L"[ADBID:%u, CDBID:%I64d, SID:%u] QUERY_FRIENDLIST Ret:%d\r\n", pPacket->nAccountDBID, m_biCharacterDBID, m_nSessionID, pPacket->nRetCode);
				return;
			}

			for (int i = 0; i < pPacket->cCount; i++)
			{
				if (pPacket->FriendData[i].nGroupDBID > 0)
					m_pFriend->AddGroup(pPacket->FriendData[i].nGroupDBID, pPacket->FriendData[i].wszGroupName);

				if (pPacket->FriendData[i].nFriendAccountDBID > 0)
				{
					if (pPacket->FriendData[i].biFriendCharacterDBID == GetCharacterDBID())
						continue;

#ifdef PRE_ADD_DOORS
					m_pFriend->AddFriend(pPacket->FriendData[i].nFriendAccountDBID, pPacket->FriendData[i].biFriendCharacterDBID, pPacket->FriendData[i].wszFriendCharacterName, pPacket->FriendData[i].nGroupDBID, pPacket->FriendData[i].bMobileAuthentication);
#else		//#ifdef PRE_ADD_DOORS
					m_pFriend->AddFriend(pPacket->FriendData[i].nFriendAccountDBID, pPacket->FriendData[i].biFriendCharacterDBID, pPacket->FriendData[i].wszFriendCharacterName, pPacket->FriendData[i].nGroupDBID, false);
#endif		//#ifdef PRE_ADD_DOORS
				}
			}
		}
		break;

	case QUERY_ADDGROUP:
		{
			const TAAddGroup * pPacket = (TAAddGroup*)pData;
			if (pPacket->nRetCode == ERROR_NONE)
			{
				if (m_pFriend->AddGroup(pPacket->nGroupDBID, pPacket->wszGroupName) == true)
					SendFriendGroupAdded(pPacket->nGroupDBID, pPacket->wszGroupName, pPacket->nRetCode);
				else
					SendFriendGroupAdded(pPacket->nGroupDBID, pPacket->wszGroupName, ERROR_FRIEND_ADDGROUPFAIL);
			}
			else
				SendFriendGroupAdded(pPacket->nGroupDBID, pPacket->wszGroupName, pPacket->nRetCode);
		}
		break;

	case QUERY_MODGROUPNAME:
		{
			const TAModGroupName * pPacket = (TAModGroupName*)pData;
			if (pPacket->nRetCode == ERROR_NONE)
			{
				if (m_pFriend->UpdateGroup(pPacket->nGroupDBID, pPacket->wszGroupName))
					SendFriendGroupUpdated(pPacket->nGroupDBID, pPacket->wszGroupName, pPacket->nRetCode);
				else
					SendFriendGroupUpdated(pPacket->nGroupDBID, pPacket->wszGroupName, ERROR_FRIEND_GROUP_NOTFOUND);
			}
			else
				SendFriendGroupUpdated(pPacket->nGroupDBID, pPacket->wszGroupName, pPacket->nRetCode);
		}
		break;

	case QUERY_DELGROUP:
		{
			const TADelGroup * pPacket = (TADelGroup*)pData;
			if (pPacket->nRetCode == ERROR_NONE)
			{
				if (m_pFriend->DelGroup(pPacket->nGroupDBID))
					SendFriendGroupDeleted(pPacket->nGroupDBID, pPacket->nRetCode);
				else
					SendFriendGroupDeleted(pPacket->nGroupDBID, ERROR_FRIEND_GROUP_NOTFOUND);
			}
			else
				SendFriendGroupDeleted(pPacket->nGroupDBID, pPacket->nRetCode);
		}
		break;

	case QUERY_MODFRIENDANDGROUPMAPPING:
		{
			TAModFriendAndGroupMapping * pPacket = (TAModFriendAndGroupMapping*)pData;
			if (pPacket->nRetCode == ERROR_NONE)
			{
				for (int i = 0; i < pPacket->cCount; i++)
				{
					if (m_pFriend->UpdateFriend(pPacket->biFriendCharacterDBIDs[i], pPacket->nGroupDBID, NULL) == false)
					{
						_DANGER_POINT();
						pPacket->biFriendCharacterDBIDs[i] = 0;
					}
				}
			}
			SendFriendUpdated(pPacket->cCount, pPacket->biFriendCharacterDBIDs, pPacket->nGroupDBID, pPacket->nRetCode);
		}
		break;
		
	case QUERY_ADDFRIEND:
		{
			const TAAddFriend * pPacket = (TAAddFriend*)pData;
			if (pPacket->nRetCode == ERROR_NONE)
			{
				if (m_pFriend->AddFriend(pPacket->nFriendAccountDBID, pPacket->biFriendCharacterDBID, pPacket->wszFriendName, pPacket->nGroupDBID, false))
				{
					TFriendGroup * pGroup = NULL;
					if (pPacket->nGroupDBID > 0)
					{
						pGroup = m_pFriend->GetGroup(pPacket->nGroupDBID);
						if (pGroup == NULL)
							_DANGER_POINT();
					}

					if (pPacket->cNeedDelIsolate > 0)
					{
						if (m_pIsolate->DelIsolateItem(pPacket->biFriendCharacterDBID) == false)
						{
							_DANGER_POINT();
						}
						else
						{
							SendIsolateDel(pPacket->wszFriendName);
						}
					}									

					//Check the location of the target being added
					sWorldUserState State;
					if (g_pWorldUserState->GetUserState(pPacket->wszFriendName, pPacket->biFriendCharacterDBID, &State) == false)
						State.nLocationState = _LOCATION_NONE;

					SendFriendAdded(pPacket->biFriendCharacterDBID, (pGroup == NULL ? 0 : pGroup->nGroupDBID), pPacket->wszFriendName, &State, pPacket->nRetCode);

					if (State.nLocationState != _LOCATION_NONE){
#if defined(_VILLAGESERVER)
						g_pMasterConnection->SendFriendAddNotify(pPacket->nFriendAccountDBID, GetCharacterName());
#elif defined(_GAMESERVER)
						g_pMasterConnectionManager->SendFriendAddNotify(GetWorldSetID(), pPacket->nFriendAccountDBID, GetCharacterName());
#endif
					}

					if( m_pFriend->GetFriendCount() == FRIEND_MAXCOUNT )
						GetEventSystem()->OnEvent( EventSystem::OnFriendFull );

#if defined(_GAMESERVER)
					if (m_pSession)
						m_pSession->GetGameRoom()->UpdateAppliedEventValue();

#elif defined(_VILLAGESERVER)
					if (m_pSession)
					{
						if (m_pSession->GetPartyID() > 0)
						{
							CDNParty * pParty = g_pPartyManager->GetParty(m_pSession->GetPartyID());
							if (pParty)
								pParty->SendAllRefreshParty(PARTYREFRESH_NONE);
							else
								_DANGER_POINT();
						}
					}
#endif	//#elif defined(_VILLAGESERVER)

					break;
				}
			}
			else
				SendFriendResult(pPacket->nRetCode);
		}
		break;

	case QUERY_DELFRIEND:
		{
			const TADelFriend * pPacket = (TADelFriend*)pData;

			if (pPacket->nRetCode == ERROR_NONE)
			{
				for (int i = 0; i < pPacket->cCount; i++)
				{
					if (m_pFriend->DelFriend(pPacket->biFriendCharacterDBIDs[i]) == false)
						_DANGER_POINT();
				}
				SendFriendDeleted(pPacket->cCount, pPacket->biFriendCharacterDBIDs, pPacket->nRetCode);

#if defined(_GAMESERVER)
				if (m_pSession)
					m_pSession->GetGameRoom()->UpdateAppliedEventValue();

#elif defined(_VILLAGESERVER)
				if (m_pSession)
				{
					if (m_pSession->GetPartyID() > 0)
					{
						CDNParty * pParty = g_pPartyManager->GetParty(m_pSession->GetPartyID());
						if (pParty)
							pParty->SendAllRefreshParty(PARTYREFRESH_NONE);
						else
							_DANGER_POINT();
					}
				}
#endif	//#elif defined(_VILLAGESERVER)
			}
			else
				SendFriendDeleted(0, NULL, pPacket->nRetCode);
		}
		break;
	}
}

void CDNUserBase::OnDBRecvIsolate(int nSubCmd, char *pData)
{
	switch(nSubCmd)
	{
		// Isolate
	case QUERY_GETISOLATELIST:
		{
			const TAGetIsolateList * pPacket = (TAGetIsolateList*)pData;
			if (pPacket->nRetCode == ERROR_NONE)
			{
				for (int i = 0; i < pPacket->cCount; i++)
				{
					if (m_pIsolate->AddIsolateItem(&pPacket->Isolate[i]) == false)
						_DANGER_POINT();
				}
			}
		}
		break;

	case QUERY_ADDISOLATE:
		{
			const TAAddIsolate * pPacket = (TAAddIsolate*)pData;
			if (pPacket->nRetCode == ERROR_NONE)
			{
				if (m_pIsolate->AddIsolateItem(&pPacket->IsolateItem))
				{
					if (pPacket->cNeedDelFriend > 0)
					{
						if (m_pFriend->DelFriend(pPacket->IsolateItem.biIsolateCharacterDBID) == false)
						{
							_DANGER_POINT();
						}
						else
							SendFriendDeleted(1, &pPacket->IsolateItem.biIsolateCharacterDBID, ERROR_NONE);
					}
					else
					{
						//Exception handling: if target friend deletes their character while user is online after adding friend, it disappears from DB
						//pPacket->cNeedDelFriend arrives as 0. Assume it is already deleted and remove from memory (sync will match again upon moving).
						INT64 biDelFriendDBID = m_pFriend->DelFriend(pPacket->IsolateItem.wszIsolateName);
						if (biDelFriendDBID > 0)
							SendFriendDeleted(1, &biDelFriendDBID, ERROR_NONE);
					}

					SendIsolateAdd(pPacket->IsolateItem.wszIsolateName);
					break;
				}
			}
			else
			{
				SendIsolateResult(pPacket->nRetCode);
			}
		}
		break;

	case QUERY_DELISOLATE:
		{
			const TADelIsolate * pPacket = (TADelIsolate*)pData;
			if (pPacket->nRetCode == ERROR_NONE)
			{
				WCHAR szName[NAMELENMAX];				
				std::vector <std::wstring> vList;
				for (int i = 0; i < pPacket->cCount; i++)
				{
					memset(szName, 0, sizeof(szName));
					if (m_pIsolate->GetIsolateChrName(pPacket->biIsolateCharacterDBIDs[i], szName))
					{
						if (m_pIsolate->DelIsolateItem(pPacket->biIsolateCharacterDBIDs[i]) == false)
						{
							_DANGER_POINT();
						}
						else
							vList.push_back(szName);
					}
				}
				SendIsolateDel(&vList);
			}
			else
				SendIsolateResult(pPacket->nRetCode);
		}
		break;
	}
}

void CDNUserBase::OnDBRecvPvP(int nSubCmd, char *pData)
{
	switch( nSubCmd )
	{
		case QUERY_GETLIST_PVP_LADDERSCORE:
		{
			TAGetListPvPLadderScore* pA = reinterpret_cast<TAGetListPvPLadderScore*>(pData);
			if( pA->nRetCode == ERROR_NONE )
			{
				memset( &m_PvPLadderScoreInfo, 0, sizeof(m_PvPLadderScoreInfo) );
				int iSize = sizeof(pA->Data)-sizeof(pA->Data.LadderScore)+pA->Data.cLadderTypeCount*sizeof(pA->Data.LadderScore[0]);
				memcpy( &m_PvPLadderScoreInfo, &pA->Data, iSize );
			}
			else
			{
				m_pSession->DetachConnection(L"QUERY_GETLIST_PVP_LADDERSCORE Failed!");
			}
			break;
		}
		case QUERY_GETLIST_PVP_LADDERSCORE_BYJOB:
		{
			TAGetListPvPLadderScoreByJob* pA = reinterpret_cast<TAGetListPvPLadderScoreByJob*>(pData);

			if( pA->nRetCode == ERROR_NONE )
			{
				m_PvPLadderScoreInfoByJob.bInit = true;
				int iSize = sizeof(pA->Data)-sizeof(pA->Data.LadderScoreByJob)+pA->Data.cJobCount*sizeof(pA->Data.LadderScoreByJob[0]);
				memcpy( &m_PvPLadderScoreInfoByJob.Data, &pA->Data, iSize );
			}

			SendPvPLadderScoreInfoByJob( pA );
			break;
		}
		case QUERY_MOD_PVP_LADDERSCORE_FORCHEAT:
		{
			TAModPvPLadderScoresForCheat* pA = reinterpret_cast<TAModPvPLadderScoresForCheat*>(pData);

			if( pA->nRetCode != ERROR_NONE )
				break;

			const_cast<TPvPLadderScoreInfo*>(GetPvPLadderScoreInfoPtr())->SetLadderScoreForCheat( static_cast<LadderSystem::MatchType::eCode>(pA->cPvPLadderCode), pA->iPvPLadderPoint, pA->iPvPLadderGradePoint, pA->iHiddenPvPLadderGradePoint );
			SendPvPLadderScoreInfo( GetPvPLadderScoreInfoPtr() );
			break;
		}
		case QUERY_GET_PVP_GHOULSCORES :
			{
				TAGetPVPGhoulScores* pA = reinterpret_cast<TAGetPVPGhoulScores*>(pData);
				if( pA->nRetCode == ERROR_NONE )
				{
					m_PvPTotalGhoulScores.nPoint[GhoulMode::PointType::PlayCount] = pA->nPlayCount;
					m_PvPTotalGhoulScores.nPoint[GhoulMode::PointType::GhoulWin] = pA->nGhoulWin;
					m_PvPTotalGhoulScores.nPoint[GhoulMode::PointType::HumanWin] = pA->nHumanWin;
					m_PvPTotalGhoulScores.nPoint[GhoulMode::PointType::TimeOver] = pA->nTimeOver;
					m_PvPTotalGhoulScores.nPoint[GhoulMode::PointType::GhoulKill] = pA->nGhoulKill;
					m_PvPTotalGhoulScores.nPoint[GhoulMode::PointType::HumanKill] = pA->nHumanKill;
					m_PvPTotalGhoulScores.nPoint[GhoulMode::PointType::HolyWaterUse] = pA->nHolyWaterUse;					
				}
				else
				{
					m_pSession->DetachConnection(L"QUERY_GET_PVP_GHOULSCORES Failed!");
				}
			}
			break;
	}
}

void CDNUserBase::OnDBRecvDarkLair(int nSubCmd, char *pData)
{
}

void CDNUserBase::OnDBRecvGuild(int nSubCmd, char *pData)
{
}

void CDNUserBase::OnDBRecvMail(int nSubCmd, char *pData)
{
	switch(nSubCmd)
	{
	case QUERY_GETCOUNTRECEIVEMAIL:
		{
			TAGetCountReceiveMail *pMail = (TAGetCountReceiveMail*)pData;
			if (pMail->nRetCode == ERROR_NONE){ 
				SendNotifyMail(pMail->iTotalMailCount, pMail->iNotReadMailCount, pMail->i7DaysLeftMailCount, false);
			}
		}
		break;

	case QUERY_SENDSYSTEMMAIL:
		{
			TASendSystemMail *pMail = (TASendSystemMail*)pData;

			if (pMail->nRetCode == ERROR_NONE){
				SendNotifyMail(pMail->iTotalMailCount, pMail->iNotReadMailCount, pMail->i7DaysLeftMailCount, true);
			}
		}
		break;

#if defined(PRE_SPECIALBOX)
	case QUERY_ADDEVENTREWARD:
		{
			TAAddEventReward *pSpecialBox = (TAAddEventReward*)pData;

			if (pSpecialBox->nRetCode == ERROR_NONE){
				if (pSpecialBox->nEventTotalCount > 0)
					SendNotifySpecialBoxCount(pSpecialBox->nEventTotalCount, true);
			}
		}
		break;

	case QUERY_GETCOUNTEVENTREWARD:
		{
			TAGetCountEventReward *pSpecialBox = (TAGetCountEventReward*)pData;

			if (pSpecialBox->nRetCode == ERROR_NONE){
				if (pSpecialBox->nEventTotalCount > 0)
					SendNotifySpecialBoxCount(pSpecialBox->nEventTotalCount, false);
			}
		}
		break;

	case QUERY_GETLISTEVENTREWARD:
		{
			TAGetListEventReward *pSpecialBox = (TAGetListEventReward*)pData;

			if (pSpecialBox->nRetCode == ERROR_NONE)
				m_pItem->SetSpecialBoxInfoList(pSpecialBox);

			SendSpecialBoxList(pSpecialBox->nRetCode, pSpecialBox->cCount, pSpecialBox->SpecialBoxInfo);
		}
		break;

	case QUERY_GETLISTEVENTREWARDITEM:
		{
			TAGetListEventRewardItem *pSpecialBox = (TAGetListEventRewardItem*)pData;

			if (pSpecialBox->nRetCode == ERROR_NONE){
				m_pItem->SetSelectSpecialBoxItems(pSpecialBox);
			}

			SendSpecialBoxItemList(pSpecialBox->nRetCode, pSpecialBox->cCount, pSpecialBox->SpecialBoxItem, m_pItem->GetSelectSpecialBoxRewardCoin(pSpecialBox->nEventRewardID));
		}
		break;

	case QUERY_ADDEVENTREWARDRECEIVER:
		{
			TAAddEventRewardReceiver *pSpecialBox = (TAAddEventRewardReceiver*)pData;

			int nRet = pSpecialBox->nRetCode;
			if (pSpecialBox->nRetCode == ERROR_NONE)
				nRet = m_pItem->ReceiveSpecialBox(pSpecialBox->nEventRewardID, pSpecialBox->nItemID);

			SendReceiveSpecialBoxItem(nRet);
		}
		break;
#endif	// #if defined(PRE_SPECIALBOX)

#if defined( PRE_MOD_71820 )
#ifdef PRE_ADD_JOINGUILD_SUPPORT
	case QUERY_GUILDSUPPORT_REWARDINFO:
		{
			TAGuildSupportRewardInfo * pPacket = (TAGuildSupportRewardInfo*)pData;

			if (pPacket->nRetCode == ERROR_NONE)
			{
				m_bWasRewardedGuildMaxLevel = pPacket->bWasGuildSupportRewardFlag;
				m_cGuildJoinLevel = pPacket->cJoinGuildLevel;

				if (m_bWasRewardedGuildMaxLevel == false && GetLevel() >= (int)CGlobalWeightTable::GetInstance().GetValue(CGlobalWeightTable::PlayerLevelLimit))
				{
					int nMailID = g_pDataManager->GetRewardGuildSupportMailID(m_cGuildJoinLevel);
					if (nMailID > 0)
					{
						const TGuildUID GuildUID = GetGuildUID();
						if (GuildUID.IsSet())
						{
							CDNGuildBase* pGuild = g_pGuildManager->At(GuildUID);
							if (pGuild) 
							{
#if !defined( PRE_ADD_NODELETEGUILD )
								CDetachAutoEx<CDNGuildBase>	AutoDetach(pGuild);
								if (TRUE == pGuild->IsEnable())
								{
									TGuildMember * pGuildMaster = pGuild->GetGuildMaster();
									if (pGuildMaster)
									{
										CDNMailSender::Process(pGuildMaster->nAccountDBID, pGuildMaster->nCharacterDBID, GetAccountDBID(), GetCharacterDBID(), GetCharacterName(), GetWorldSetID(), nMailID);
										m_bWasRewardedGuildMaxLevel = true;
									}
								}
#else		//#if !defined( PRE_ADD_NODELETEGUILD )
								TGuildMember * pGuildMaster = pGuild->GetGuildMaster();
								if (pGuildMaster)
								{
									CDNMailSender::Process(pGuildMaster->nAccountDBID, pGuildMaster->nCharacterDBID, GetAccountDBID(), GetCharacterDBID(), GetCharacterName(), GetWorldSetID(), nMailID);
									m_bWasRewardedGuildMaxLevel = true;
								}
#endif		//#if !defined( PRE_ADD_NODELETEGUILD )
							}
						}
						else
							_DANGER_POINT();
					}
					else
						g_Log.Log(LogType::_ERROR, m_pSession, L"QUERY_GUILDSUPPORT_REWARDINFO Error MailID Invalid [%d]\n", nMailID);
				}
			}			
		}
		break;
	case QUERY_SENDGUILDMAIL:
		{
			TASendGuildMail * pPacket = (TASendGuildMail*)pData;
			if (pPacket->nRetCode == ERROR_NONE)
			{
				if (pPacket->nReceiverAccountDBID != GetAccountDBID())
				{
#if defined(_VILLAGESERVER)
					CDNUserSession *pUserObj = g_pUserSessionManager->FindUserSessionByAccountDBID(pPacket->nReceiverAccountDBID);	// Find the recipient
					if (pUserObj && (pUserObj->GetCharacterDBID() == pPacket->biReceiverCharacterDBID)){	// If on the same server, just process directly
						pUserObj->SendNotifyMail(pPacket->iTotalMailCount, pPacket->iNotReadMailCount, pPacket->i7DaysLeftMailCount, true);	// Notify that mail has arrived
					}
					else {	// If none, set to master 
						g_pMasterConnection->SendNotifyMail(pPacket->nReceiverAccountDBID, pPacket->biReceiverCharacterDBID, pPacket->iTotalMailCount, pPacket->iNotReadMailCount, pPacket->i7DaysLeftMailCount, true);
					}
#elif defined(_GAMESERVER)
					CDNRUDPGameServer * pGameServer = g_pGameServerManager->GetGameServerByAID(pPacket->nReceiverAccountDBID); // Find the recipient
					if(pGameServer){
						CDNUserSession *pUserObj = pGameServer->GetUserSession(pPacket->nReceiverAccountDBID);
						if (pUserObj && (pUserObj->GetCharacterDBID() == pPacket->biReceiverCharacterDBID)){	// If on the same server, just process directly
							pUserObj->SendNotifyMail(pPacket->iTotalMailCount, pPacket->iNotReadMailCount, pPacket->i7DaysLeftMailCount, true);	// Notify that mail has arrived
						}
					}			
					else {// If none, set to master
						g_pMasterConnectionManager->SendNotifyMail(pPacket->cWorldSetID, pPacket->nReceiverAccountDBID, pPacket->biReceiverCharacterDBID, pPacket->iTotalMailCount, pPacket->iNotReadMailCount, pPacket->i7DaysLeftMailCount, true);
					}
#endif // #if defined(_VILLAGESERVER)					
				}
			}
		}
		break;
#endif		//#ifdef PRE_ADD_JOINGUILD_SUPPORT
#endif // #if defined( PRE_MOD_71820 )
	}
}

void CDNUserBase::OnDBRecvMarket(int nSubCmd, char *pData)
{
}

void CDNUserBase::OnDBRecvItem(int nSubCmd, char *pData)
{
	switch(nSubCmd)
	{
	case QUERY_GETLIST_REPURCHASEITEM:
		{
			TAGetListRepurchaseItem* pA = reinterpret_cast<TAGetListRepurchaseItem*>(pData);

			GetItem()->OnRecvGetListRepurchaseItem( pA );
			break;
		}

	case QUERY_GETPAGEMATERIALIZEDITEM:
		{
			TAGetPageMaterializedItem *pItem = (TAGetPageMaterializedItem*)pData;

			if (pItem->nRetCode != ERROR_NONE){
				m_pSession->DetachConnection(L"QUERY_GETPAGEMATERIALIZEDITEM LoadCashInvenList failed!");
				return;
			}

			m_pItem->LoadCashInventory(pItem->wPageNumber, pItem->nTotalListCount, pItem->cCount, pItem->CashItem);

			int nRemain = m_pItem->GetCashInventoryTotalCount() - m_pItem->GetCashInventoryCount();
			if (nRemain > 0){
				m_pSession->GetDBConnection()->QueryGetPageMaterializedItem(m_pSession->GetDBThreadID(), m_pSession, m_pItem->GetCashInventoryPage(), CASHINVENTORYPAGEMAX);
			}
		}
		break;

	case QUERY_GETPAGEVEHICLE:
		{
			TAGetPageVehicle *pItem = (TAGetPageVehicle*)pData;

			if (pItem->nRetCode != ERROR_NONE){
				m_pSession->DetachConnection(L"QUERY_GETPAGEVEHICLE LoadVehicleInventory failed!");
				return;
			}

			m_pItem->LoadVehicleInventory(pItem);
#if defined( _VILLAGESERVER )
			GetEventSystem()->OnEvent( EventSystem::OnPetLevelUp );
#endif
		}
		break;

	case QUERY_CHANGEPETNAME:
		{
			const TAChangePetName *pPacket = reinterpret_cast<TAChangePetName*>(pData);

			if (pPacket->nRetCode != ERROR_NONE)
			{
				SendChangePetNameResult(pPacket->nRetCode, m_pSession->GetSessionID(), pPacket->petSerial, pPacket->name);
				break;
			}

			int nRet = m_pItem->ChangePetName(pPacket);
			SendChangePetNameResult(pPacket->nRetCode, m_pSession->GetSessionID(), pPacket->petSerial, pPacket->name);
		}
		break;
	case QUERY_ITEMEXPIREBYCHEAT :
		{
			const TAItemExpire* pPacket = reinterpret_cast<TAItemExpire*>(pData);
			const TItem* pItem = m_pItem->GetCashInventory(pPacket->biSerial);
			if( pPacket->nRetCode == ERROR_NONE && pItem )
			{
				// Delete from cash inventory..
				m_pItem->DeleteCashInventoryBySerial(pPacket->biSerial, 1, false);
			}
		}
		break;
	case QUERY_MODITEMEXPIREDATE:
		{
			const TAModItemExpireDate *pPacket = reinterpret_cast<TAModItemExpireDate*>(pData);
			if( pPacket->nRetCode != ERROR_NONE ) 
			{
				if (m_pSession)
				{
					m_pSession->DetachConnection(L"QUERY_MODITEMEXPIREDATE failed!");
					return;
				}
			}
			SendItemModItemExpireDate(pPacket->nRetCode);
			m_pItem->ModPetExpireDate(pPacket);
		}
		break;

	case QUERY_MODPETEXP:
		{
			const TAModPetExp* pPacket = reinterpret_cast<TAModPetExp*>(pData);
			if( pPacket->nRetCode != ERROR_NONE )
			{
				if (m_pSession)
				{
					m_pSession->DetachConnection(L"QUERY_MODITEMEXPIREDATE failed!");
					return;
				}
			}
		}
		break;

	case QUERY_MISSINGITEMLIST:
		{
			TAMissingItemList *pItem = (TAMissingItemList*)pData;

			if (pItem->nRetCode == ERROR_NONE){
				m_pItem->CalcMissingItem(pItem);				
			}
		}
		break;

	case QUERY_RECOVERMISSINGITEM:
		{
			TARecoverMissingItem *pItem = (TARecoverMissingItem*)pData;

			if (pItem->nRetCode == ERROR_NONE){
				m_pItem->RecoverMissingItem(pItem);
			}
		}
		break;

	case QUERY_MODADDITIVEITEM:
		{
			TAModAdditiveItem *pItem = (TAModAdditiveItem*)pData;
			m_pItem->CompleteCostumeDesign(pItem);
		}
		break;

#if defined (PRE_ADD_COSRANDMIX) && defined (_VILLAGESERVER)
	case QUERY_MODRANDOMITEM:
		{
			TAModRandomItem *pItem = (TAModRandomItem*)pData;
			m_pItem->CompleteRandomDesign(pItem);
		}	
		break;
#endif

	case QUERY_DELEXPIREITEM:
		{
			TADelExpiritem *pItem = (TADelExpiritem*)pData;
			m_pItem->RefreshExpireitem(pItem);
		}
		break;

	case QUERY_DELCASHITEM:
		{
			TADelCashItem *pItem = (TADelCashItem*)pData;
			m_pItem->OnRemoveCashItem(pItem);
		}
		break;

	case QUERY_RECOVERCASHITEM:
		{
			TARecoverCashItem *pItem = (TARecoverCashItem*)pData;
			m_pItem->OnRecoverCashItem(pItem);
		}
		break;

#if defined (PRE_ADD_EXCHANGE_POTENTIAL)
	case QUERY_MOVEPOTENTIAL:
		{
			TAMovePotential * pPacket = (TAMovePotential*)pData;
			if (pPacket->nRetCode != ERROR_NONE)
				_DANGER_POINT_MSG(L"QUERY_MOVEPOTENTIAL Recv Error");
		}
		break;
#endif		//#if defined (PRE_ADD_EXCHANGE_POTENTIAL)
#if defined( PRE_ADD_NAMEDITEM_SYSTEM )
	case QUERY_CHEATCHECK_NAMEDITEMCOUNT:
		{
			TACheckNamedItemCount* pA = reinterpret_cast<TACheckNamedItemCount*>(pData);
			WCHAR wszBuf[MAX_PATH];
			wsprintf( wszBuf, L"[NamedItem] Nameditem Count:%d", pA->nCount );
			SendDebugChat( wszBuf );
		}
		break;
#endif
#if defined( PRE_ADD_LIMITED_SHOP )
	case QUERY_GETLIMITEDSHOPITEM:
		{
			TAGetLimitedShopItem* pPacket = reinterpret_cast<TAGetLimitedShopItem*>(pData);
			for (int i = 0; i < pPacket->nCount; ++i)
			{
#if defined( PRE_FIX_74404 )
				AddLimitedShopBuyedItem(pPacket->ItemData[i].nShopID, pPacket->ItemData[i].nItemID, pPacket->ItemData[i].nBuyCount, pPacket->ItemData[i].nResetCycle, false);				
#else	
				AddLimitedShopBuyedItem(pPacket->ItemData[i].nItemID, pPacket->ItemData[i].nBuyCount, pPacket->ItemData[i].nResetCycle, false);				
#endif
			}
		}
		break;
#endif
#if defined(PRE_FIX_74387)
	case QUERY_RESELLITEM :
		{
			TAHeader* pA = reinterpret_cast<TAHeader*>(pData);			
			m_pSession->SendShopSellResult(pA->nRetCode);
			m_pSession->SetNowResellItem(false);
		}
		break;
#endif //#if defined(PRE_FIX_74387)
#if defined(PRE_ADD_EQUIPLOCK)
	case QUERY_ADDLOCK_ITEM:
		{
			const TALockItemInfo* pA = reinterpret_cast<const TALockItemInfo*>(pData);
			if( pA->nRetCode == ERROR_NONE )
				GetItem()->LockEquipItem(pA->Code, pA->cItemSlotIndex, pA->tLockDate);

			SendAddLockItem(pA->nRetCode, pA->Code, pA->cItemSlotIndex, pA->tLockDate);
		}
		break;
	case QUERY_REQUEST_ITEMUNLOCK:
		{
			const TAUnLockRequestItemInfo* pA = reinterpret_cast<const TAUnLockRequestItemInfo*>(pData);
			if(pA->nRetCode == ERROR_NONE)
				GetItem()->RequestUnLockEquipItem(pA->Code, pA->cItemSlotIndex, pA->tUnLockDate, pA->tUnLockRequestDate);

			SendRequestUnlockItem(pA->nRetCode, pA->Code, pA->cItemSlotIndex, pA->tUnLockDate, pA->tUnLockRequestDate);
		}
		break;
	case QUERY_GET_LIST_LOCKEDITEMS:
		{
			const TAGetListLockedItems* pA = reinterpret_cast<TAGetListLockedItems*>(pData);
			//Save locked equipment and send it to the client (do not process if there are no locked items)
			if( pA->nRetCode == ERROR_NONE && pA->nCount > 0 )
				m_pItem->LoadLockItem(pA);
		}
		break;
#endif	// #if defined(PRE_ADD_EQUIPLOCK)
#if defined(PRE_ADD_CHAOSCUBE)
	case QUERY_GETLIST_CHAOSGAUGEITEMS:
		{
			const TAGetListChaosGaugeItems* pA = reinterpret_cast<const TAGetListChaosGaugeItems*>(pData);
			if( pA->nRetCode == ERROR_NONE )
			{
				int nCount = (pA->nCount > 20) ? 20 : pA->nCount;
				for( int i = 0; i < nCount; ++i )
					m_pItem->SetChaosGauge(pA->GaugeList[i].nChaosGaugeType, pA->GaugeList[i].nItemGauge);
				if( nCount > 0 )
					SendChaosGaugeCubeList(m_pItem->GetChaosGaugeMap());
			}
		}
		break;
#endif	// #if defined(PRE_ADD_CHAOSCUBE)
	}
}

void CDNUserBase::OnDBRecvCash(int nSubCmd, char *pData)
{
	switch(nSubCmd)
	{
	case QUERY_NOTIFYGIFT:
		{
			TANotifyGift *pCashShop = (TANotifyGift*)pData;
			if (pCashShop->nRetCode == ERROR_NONE){ 
				if (pCashShop->nGiftCount > 0) SendCashshopNotifyGift(pCashShop->bNew, pCashShop->nGiftCount);
			}
		}
		break;

	case QUERY_GETLISTGIVEFAILITEM:
		{
			TAGetListGiveFailItem *pCashShop = (TAGetListGiveFailItem*)pData;
			m_pItem->DBQueryCashFailItemApply(pCashShop);
		}
		break;

	case QUERY_MODGIVEFAILFLAG:	// Restore cash on purchase failure
		{
			TAModGiveFailFlag *pCash = (TAModGiveFailFlag*)pData;

			for (int i = 0; i < pCash->nCount; i++){
				m_pItem->ApplyCashShopItem(pCash->CashItem[i]);
			}
		}
		break;

	case QUERY_CHECKGIFTRECEIVER:
		{
			TACheckGiftReceiver *pCashShop = (TACheckGiftReceiver*)pData;
			SendCashShopCheckReceiver(pCashShop->cLevel, pCashShop->cJob, pCashShop->nRetCode);
		}
		break;

	case QUERY_GETLISTGIFTBOX:
		{
			TAGetListGiftBox *pCashShop = (TAGetListGiftBox*)pData;

			CDNMailSender::ModifyGiftData(pCashShop->GiftBox, pCashShop->cCount);
			SendCashShopGiftList(pCashShop->nRetCode, pCashShop->cCount, pCashShop->GiftBox);
		}
		break;

	case QUERY_MODGIFTRECEIVEFLAG:
		{
			TAModGiftReceiveFlag *pCashShop = (TAModGiftReceiveFlag*)pData;

			int nRet = ERROR_NONE;
#if defined(PRE_RECEIVEGIFTALL)
			if (pCashShop->cCount > 1){
				nRet = m_pItem->CheckModGiftReceive(pCashShop);
				if (nRet != ERROR_NONE){
					if (pCashShop->bReceiveAll)
						SendCashShopReceiveGiftAll(m_VecReceiveGiftResultList, nRet);
					else
						SendCashShopReceiveGift(pCashShop->ReceiveGift[0].GiftData.nGiftDBID, nRet);
					break;
				}

				if (m_VecReceiveGiftResultList.empty()){
					SendCashShopReceiveGiftAll(m_VecReceiveGiftResultList, ERROR_ITEM_FAIL);
					break;
				}
			}
#endif	// #if defined(PRE_RECEIVEGIFTALL)

			nRet = m_pItem->ModGiftReceiveFlag(pCashShop);
			if (nRet != ERROR_NONE){
#if defined(PRE_RECEIVEGIFTALL)
				if (pCashShop->bReceiveAll)
					SendCashShopReceiveGiftAll(m_VecReceiveGiftResultList, nRet);
				else
#endif	// #if defined(PRE_RECEIVEGIFTALL)
					SendCashShopReceiveGift(pCashShop->ReceiveGift[0].GiftData.nGiftDBID, nRet);
				break;
			}
		}
		break;

	case QUERY_RECEIVEGIFT:
		{
			TAReceiveGift *pCashShop = (TAReceiveGift*)pData;

			if (pCashShop->nRetCode == ERROR_NONE){
				if (pCashShop->cCount > 1){	// package
					for (int i = 0; i < pCashShop->cCount; i++){
						m_pItem->ApplyCashShopItem(pCashShop->GiftItem[i].nItemSN, pCashShop->GiftItem[i].AddItem, pCashShop->GiftItem[i].dwPartsColor1, pCashShop->GiftItem[i].dwPartsColor2, 
							&(pCashShop->GiftItem[i].VehiclePart1), &(pCashShop->GiftItem[i].VehiclePart2));
					}
				}
				else{
#if defined(PRE_ADD_VIP)
					if (g_pDataManager->GetItemMainType(pCashShop->GiftItem[0].AddItem.nItemID) == ITEMTYPE_VIP){
						TCashCommodityData CashData;
						if (!g_pDataManager->GetCashCommodityData(pCashShop->nItemSN, CashData)){
							SendCashShopReceiveGift(pCashShop->biPurchaseOrderID, ERROR_ITEM_FAIL);
							return;
						}
						TCashCommodityData *pCashData = &CashData;

						if (!m_bVIPAutoPay)
							m_bVIPAutoPay = pCashData->bAutomaticPay;

						m_pDBCon->QueryIncreaseVIPPoint(m_pSession, pCashData->nVIPPoint, pCashShop->biPurchaseOrderID, pCashData->wPeriod, m_bVIPAutoPay);

						if (pCashShop->cPayMethodCode != DBDNWorldDef::PayMethodCode::LevelupEvent)
							_MakeGiftVIPMonthItemByItemSN(pCashShop->nItemSN);
					}
					else
#endif	// #if defined(PRE_ADD_VIP)
						m_pItem->ApplyCashShopItem(pCashShop->nItemSN, pCashShop->GiftItem[0].AddItem, pCashShop->GiftItem[0].dwPartsColor1, pCashShop->GiftItem[0].dwPartsColor2, &(pCashShop->GiftItem[0].VehiclePart1), &(pCashShop->GiftItem[0].VehiclePart2));
				}

				if (pCashShop->nReceiverAccountDBID != m_nAccountDBID){
#if defined(_VILLAGESERVER)
					CDNUserSession *pUserSession = g_pUserSessionManager->FindUserSessionByAccountDBID(pCashShop->nReceiverAccountDBID);	// Find the recipient
					if (pUserSession && (pUserSession->GetCharacterDBID() == pCashShop->biReceiverCharacterDBID)){	// If on the same server, just process directly
						pUserSession->SendNotifyMail(pCashShop->nReceiverTotalMailCount, pCashShop->nReceiverNotReadMailCount, pCashShop->nReceiver7DaysLeftMailCount, true);	// Notify that mail has arrived
					}
					else {	// If none, set to master 
						g_pMasterConnection->SendNotifyMail(pCashShop->nReceiverAccountDBID, pCashShop->biReceiverCharacterDBID, pCashShop->nReceiverTotalMailCount, pCashShop->nReceiverNotReadMailCount, pCashShop->nReceiver7DaysLeftMailCount, true);
					}
#endif
				}
			}

#if defined(PRE_RECEIVEGIFTALL)
			if (pCashShop->bReceiveAll)
			{
#if defined(PRE_RECEIVEGIFTALL)
				m_nReceiveGiftPageCount++;
#endif	// #if defined(PRE_RECEIVEGIFTALL)

				if (m_nReceiveGiftPageCount == pCashShop->cTotalCount){
					SendCashShopReceiveGiftAll(m_VecReceiveGiftResultList, pCashShop->nRetCode);
				}
			}
			else
#endif	// #if defined(PRE_RECEIVEGIFTALL)
				SendCashShopReceiveGift(pCashShop->biPurchaseOrderID, pCashShop->nRetCode);
		}
		break;

	case QUERY_MAKEGIFTBYQUEST:
		{
			TAMakeGiftByQuest *pCash = (TAMakeGiftByQuest*)pData;
			if (pCash->nRetCode == ERROR_NONE){
				if (pCash->nGiftCount > 0) SendCashshopNotifyGift(true, pCash->nGiftCount);
			}
		}
		break;

	case QUERY_MAKEGIFTBYMISSION:
		{
			TAMakeGiftByMission *pCash = (TAMakeGiftByMission*)pData;
			if (pCash->nRetCode == ERROR_NONE){
				if (pCash->nGiftCount > 0) SendCashshopNotifyGift(true, pCash->nGiftCount);
			}
		}
		break;

	case QUERY_MAKEGIFT:
		{
			TAMakeGift *pCash = (TAMakeGift*)pData;
			if (pCash->nRetCode == ERROR_NONE){
				if (pCash->nGiftCount > 0) SendCashshopNotifyGift(true, pCash->nGiftCount);
			}
		}
		break;

#if defined(PRE_ADD_VIP)
	case QUERY_INCREASEVIPPOINT:
		{
			TAIncreaseVIPPoint *pCash = (TAIncreaseVIPPoint*)pData;
			if (pCash->nRetCode == ERROR_NONE){
				if (pCash->tVIPEndDate > 0){
					if (!m_bVIP){	// If already VIP, no need to give fatigue again
						m_bVIP = true;	// Set for value update
						SetDefaultVIPFatigue();	// Set VIP fatigue
					}

					m_bVIP = true;
					m_nVIPTotalPoint = pCash->nVIPTotalPoint;
					m_tVIPEndDate = pCash->tVIPEndDate;
				}

				SendVIPInfo(m_nVIPTotalPoint, m_tVIPEndDate, m_bVIPAutoPay, m_bVIP);
			}
		}
		break;

	case QUERY_GETVIPPOINT:
		{
			TAGetVIPPoint *pCash = (TAGetVIPPoint*)pData;
			if (pCash->nRetCode == ERROR_NONE){
				m_nVIPTotalPoint = pCash->nVIPTotalPoint;
				m_tVIPEndDate = pCash->tVIPEndDate;
				m_bVIPAutoPay = pCash->bAutoPay;

				__time64_t tCur;
				time(&tCur);
				if (m_tVIPEndDate >= tCur)
					m_bVIP = true;

				SendVIPInfo(m_nVIPTotalPoint, m_tVIPEndDate, m_bVIPAutoPay, m_bVIP);
			}
		}
		break;

	case QUERY_MODVIPAUTOPAYFLAG:
		{
			TAModVIPAutoPayFlag *pCash = (TAModVIPAutoPayFlag*)pData;
			if (pCash->nRetCode == ERROR_NONE){
				m_bVIPAutoPay = pCash->bAutoPay;
			}
		}
		break;

#endif	// #if defined(PRE_ADD_VIP)

	case QUERY_PETAL:
		{
			TAPetal *pCash = (TAPetal*)pData;
			if (pCash->nRetCode == ERROR_NONE){
				m_pItem->DeleteItemByUse(pCash->cInvenType, pCash->cInvenIndex, pCash->biInvenSerial, false );
				SetPetal(pCash->nTotalPetal);
				SendPetalTokenResult(pCash->nUseItemID, pCash->nTotalPetal);
			}
		}
		break;

#if defined(_VILLAGESERVER)
#if defined(PRE_ADD_GIFT_RETURN)
	case QUERY_GIFTRETURN :
		{
			TAGiftReturn *pCash = (TAGiftReturn*)pData;
			if (pCash->nRetCode == ERROR_NONE)
			{
				CDNUserSession *pUserSession = g_pUserSessionManager->FindUserSessionByAccountDBID(pCash->nReceiverAccountDBID);	// Find the recipient
				if (pUserSession && (pUserSession->GetCharacterDBID() == pCash->biReceiverCharacterDBID)){	// If on the same server, just process directly
					pUserSession->SendCashshopNotifyGift(true, pCash->nReceiverTotalMailCount);	// Notify that it was returned
				}
				else {	// If none, set to master 
					g_pMasterConnection->SendNotifyGift(pCash->nReceiverAccountDBID, pCash->biReceiverCharacterDBID, true, pCash->nReceiverTotalMailCount);
				}				
			}
			SendCashShopGiftReturn(pCash->biPurchaseOrderID, pCash->nRetCode);
		}
		break;
#endif // #if defined(PRE_ADD_GIFT_RETURN)
#if defined(PRE_ADD_CASH_REFUND)
	case QUERY_PAYMENTINVEN_LIST :
		{
			TAPaymentItemList* pPaymentItemList = (TAPaymentItemList*)pData;
			if ( pPaymentItemList->nRetCode == ERROR_NONE)			
				m_pItem->LoadPaymentItem(pPaymentItemList);			
		}
		break;

	case QUERY_PAYMENTINVEN_PAKAGELIST :
		{
			TAPaymentPackageItemList* pPaymentPackageItemList = (TAPaymentPackageItemList*)pData;
			if ( pPaymentPackageItemList->nRetCode == ERROR_NONE)
				m_pItem->LoadPaymentPackageItem(pPaymentPackageItemList);
		}
		break;
#endif // #if defined(PRE_ADD_CASH_REFUND)
#endif //#if defined(_VILLAGESERVER) 
	}
}

int CDNUserBase::OnRecvCharMessage(int nSubCmd, char *pData, int nLen)
{
	switch(nSubCmd)
	{
	case eChar::CS_ADDQUICKSLOT:
		{
			if (sizeof(CSAddQuickSlot) != nLen)
				return ERROR_INVALIDPACKET;

			CSAddQuickSlot *pAddSlot = (CSAddQuickSlot*)pData;
			AddQuickSlot(pAddSlot->cSlotIndex, pAddSlot->cSlotType, pAddSlot->nID);
			// SendAddQuickSlot(pAddSlot->cSlotIndex, pAddSlot->cSlotType, pAddSlot->nID, nRet);
			return ERROR_NONE;
		}
		break;

	case eChar::CS_DELQUICKSLOT:
		{
			if (sizeof(CSDelQuickSlot) != nLen)
				return ERROR_INVALIDPACKET;

			CSDelQuickSlot *pDelSlot = (CSDelQuickSlot*)pData;
			DelQuickSlot(pDelSlot->cSlotIndex);
			// SendDelQuickSlot(pDelSlot->cSlotIndex, nRet);
			return ERROR_NONE;
		}
		break;
#if defined( PRE_ADD_NOTIFY_ITEM_COMPOUND )
	case eChar::CS_COMPOUND_NOTIFY:
	{
		if (sizeof(CSCompoundNotify) != nLen)
			return ERROR_INVALIDPACKET;

		SetCompoundNotify((CSCompoundNotify*)pData);

		return ERROR_NONE;
	}
		break;
#endif

	}

	return ERROR_UNKNOWN_HEADER;
}

int CDNUserBase::OnRecvItemMessage(int nSubCmd, char *pData, int nLen)
{
	switch(nSubCmd)
	{
	case eItem::CS_REMOVECASH:
		{
			if (sizeof(CSRemoveCash) != nLen)
				return ERROR_INVALIDPACKET;

			CSRemoveCash *pItem = (CSRemoveCash*)pData;
			if (!IsNoneWindowState() && !IsWindowState(WINDOW_BLIND)){
				SendRemoveCash(NULL, ERROR_ITEM_FAIL);
				return ERROR_NONE;
			}

			if (m_pRestraint->CheckRestraint(_RESTRAINTTYPE_TRADE) == false)
				return ERROR_NONE;

			int nRet = m_pItem->OnRecvRemoveCash(pItem->biItemSerial, pItem->bRecovery);
			if (nRet != ERROR_NONE)
				SendRemoveCash(NULL, nRet);

			return ERROR_NONE;
		}
		break;
#if defined(PRE_ADD_EQUIPLOCK)
	case eItem::CS_ITEM_LOCK_REQ:
		{
			if(sizeof(CSItemLockReq) != nLen)
				return ERROR_INVALIDPACKET;

			CSItemLockReq *pItem = (CSItemLockReq*)pData;
			DBDNWorldDef::ItemLocation::eCode Code;
			if(pItem->IsCashEquip)
				Code = DBDNWorldDef::ItemLocation::CashEquip;
			else
				Code = DBDNWorldDef::ItemLocation::Equip;
		
			//Check if the slot is a cash (or normal equipment) slot and verify the item serial number matches
			if( !m_pItem->IsValidEquipLockSlot(Code, pItem->nItemSlotIndex, pItem->biItemSerial) )
				return ERROR_ITEM_NOTFOUND;

#if defined(_GAMESERVER)
			//Reject lock requests if game mode is Ghoul mode
			if (!m_pSession->GetGameRoom() || m_pSession->GetGameRoom()->bIsZombieMode())
				return ERROR_NONE;
#endif	// #if defined(_GAMESERVER)

			GetDBConnection()->QueryAddLockItem(static_cast<CDNUserSession*>(this), Code, pItem->nItemSlotIndex);

			return ERROR_NONE;
		}
		break;
	case eItem::CS_ITEM_UNLOCK_REQ:
		{
			if(sizeof(CSItemLockReq) != nLen)
				return ERROR_INVALIDPACKET;

			CSItemLockReq *pItem = (CSItemLockReq*)pData;
			DBDNWorldDef::ItemLocation::eCode Code;
			if(pItem->IsCashEquip)
				Code = DBDNWorldDef::ItemLocation::CashEquip;
			else
				Code = DBDNWorldDef::ItemLocation::Equip;

			//Check if the slot is a cash (or normal equipment) slot and verify the item serial number matches
			if( !m_pItem->IsValidEquipLockSlot(Code, pItem->nItemSlotIndex, pItem->biItemSerial) )
				return ERROR_ITEM_NOTFOUND;

#if defined(_GAMESERVER)
			//Reject unlock requests if game mode is Ghoul mode
			if (!m_pSession->GetGameRoom() || m_pSession->GetGameRoom()->bIsZombieMode())
				return ERROR_NONE;
#endif	// #if defined(_GAMESERVER)

			GetDBConnection()->QueryRequestItemLock(static_cast<CDNUserSession*>(this), Code, pItem->nItemSlotIndex, (int)CGlobalWeightTable::GetInstance().GetValue(CGlobalWeightTable::ItemUnLockWaitTime));

			return ERROR_NONE;
		}
		break;
#endif	// #if defined(PRE_ADD_EQUIPLOCK)
#if defined(PRE_ADD_TALISMAN_SYSTEM)
	case eItem::CS_AUTOUNPACK_CLOSE:
		{
			if (nLen != 0)
				return ERROR_INVALIDPACKET;

			return ERROR_NONE;
		}
		break;

	case eItem::CS_TALISMAN_OPENSLOT:
		{
			if (sizeof(CSTalismanOpenSlot) != nLen)
				return ERROR_INVALIDPACKET;

			CSTalismanOpenSlot *pPacket = (CSTalismanOpenSlot*)pData;
			if (!IsNoneWindowState() && !IsWindowState(WINDOW_BLIND)) {
				SendTalismanOpenSlot(ERROR_ITEM_FAIL, GetTalismanOpenFlag());
				return ERROR_NONE;
			}

			m_pItem->RequestTalismanOpenSlot(pPacket->nSlotIndex);
			return ERROR_NONE;
		}
		break;
#endif	// #if defined(PRE_ADD_TALISMAN_SYSTEM)
	}
	return ERROR_UNKNOWN_HEADER;
}

int CDNUserBase::OnRecvItemGoodsMessage(int nSubCmd, char *pData, int nLen)
{
	switch(nSubCmd)
	{
	case eItemGoods::CS_UNSEAL:
		{
			if (sizeof(CSUnSealItem) != nLen)
				return ERROR_INVALIDPACKET;

			CSUnSealItem* pItem = (CSUnSealItem*)pData;

			int nRet = m_pItem->UnsealItem(pItem->cInvenIndex, pItem->biInvenSerial);
			SendUnsealItemResult(nRet, pItem->cInvenIndex);

			return ERROR_NONE;
		}
		break;

	case eItemGoods::CS_SEAL:
		{
			if (sizeof(CSSealItem) != nLen)
				return ERROR_INVALIDPACKET;

			CSSealItem *pItem = (CSSealItem*)pData;
			int nRet = m_pItem->SealItem(pItem->cInvenIndex, pItem->biInvenSerial, pItem->biItemSerial);
			SendSealItemResult(nRet, pItem->cInvenIndex);

			return ERROR_NONE;
		}
		break;

	case eItemGoods::CS_POTENTIALJEWEL:
		{
			if (sizeof(CSPotentialItem) != nLen)
				return ERROR_INVALIDPACKET;
#if defined(PRE_MOD_POTENTIAL_JEWEL_RENEWAL)
			m_pItem->ResetPrevPotentialData();
#endif	// #if defined(PRE_MOD_POTENTIAL_JEWEL_RENEWAL)
			CSPotentialItem *pItem = (CSPotentialItem *)pData;
			int nRet = m_pItem->PotentialItem( pItem->cInvenIndex, pItem->biInvenSerial, pItem->biItemSerial );
			SendPotentialItemResult( nRet, pItem->cInvenIndex );

			return ERROR_NONE;
		}
		break;

#if defined(PRE_MOD_POTENTIAL_JEWEL_RENEWAL)
	case eItemGoods::CS_POTENTIALJEWEL_ROLLBACK:
		{
			if (sizeof(CSPotentialItem) != nLen)
				return ERROR_INVALIDPACKET;			

			CSPotentialItem *pItem = (CSPotentialItem *)pData;
			int nRet = m_pItem->RollbackPotentialItem(pItem->cInvenIndex, pItem->biInvenSerial, pItem->biItemSerial);
			m_pItem->ResetPrevPotentialData();
			SendPotentialItemRollbackResult(nRet);
			return ERROR_NONE;
		}
		break;
#endif	//#if defined(PRE_MOD_POTENTIAL_JEWEL_RENEWAL)
	case eItemGoods::CS_GUILDRENAME :
		{
			if (sizeof(CSGuildRename) != nLen)
				return ERROR_INVALIDPACKET;			

			CSGuildRename *pGuildRename = (CSGuildRename*)pData;
			m_pItem->RequestChangeGuildNameItem(pGuildRename);
			return ERROR_NONE;
		}
		break;

	case eItemGoods::CS_CHARACTERRENAME:
		{
			if (sizeof(CSCharacterRename) != nLen)
				return ERROR_INVALIDPACKET;

			CSCharacterRename *pPacket = (CSCharacterRename*)pData;
			m_pItem->RequestChangeCharacterNameItem(pPacket);
			return ERROR_NONE;
		}
		break;

	case eItemGoods::CS_CHANGEPETNAME:
		{
			if (sizeof(CSChangePetName) != nLen)
				return ERROR_INVALIDPACKET;

			CSChangePetName *pPacket = (CSChangePetName*)pData;
			m_pItem->RequestChangePetName(pPacket);
			return ERROR_NONE;
		}
		break;

	case eItemGoods::CS_GUILDMARK :
		{
			if (sizeof(CSGuildMark) != nLen)
				return ERROR_INVALIDPACKET;			

			CSGuildMark *pGuildMark = (CSGuildMark*)pData;
			int nRet = m_pItem->ChangeGuildMark(pGuildMark);
			if (nRet != ERROR_NONE)
				SendGuildMarkResult(nRet, 0, 0, 0);
			
			return ERROR_NONE;
		}
		break;

	case eItemGoods::CS_CHARMITEMREQUEST:
		{
			if (sizeof(CSCharmItemRequest) != nLen)
				return ERROR_INVALIDPACKET;
			if (m_pRestraint->CheckRestraint(_RESTRAINTTYPE_TRADE) == false)
				return ERROR_NONE;

			CSCharmItemRequest *pItem = (CSCharmItemRequest*)pData;
			int nRet = m_pItem->RequestCharmItem(pItem);
			if (nRet != ERROR_NONE){
				IsWindowStateNoneSet(WINDOW_PROGRESS);
				SendCharmItemRequest(pItem->cInvenType, -1, 0, nRet);
			}

			return ERROR_NONE;
		}
		break;

	case eItemGoods::CS_CHARMITEMCOMPLETE:
		{
			if (!m_pItem->IsValidRequestTimer(CDNUserItem::RequestType_UseRandomItem)) break;

			if (sizeof(CSCharmItemComplete) != nLen)
				return ERROR_INVALIDPACKET;

			CSCharmItemComplete *pItem = (CSCharmItemComplete *)pData;
			int nRet = m_pItem->CompleteCharmItem(pItem);
			if (nRet != ERROR_NONE){
				SendCharmItemComplete(pItem->cInvenType, -1, 0, 0, nRet);
			}

			IsWindowStateNoneSet(WINDOW_PROGRESS);
			return ERROR_NONE;
		}
		break;

	case eItemGoods::CS_CHARMITEMCANCEL:
		{
			if (nLen != 0)
				return ERROR_INVALIDPACKET;

			m_pItem->SetRequestTimer(CDNUserItem::RequestType_None, 0);
			m_pSession->BroadcastingEffect(EffectType_Random, EffectState_Cancel);
			IsWindowStateNoneSet(WINDOW_PROGRESS);
			return ERROR_NONE;
		}
		break;

#if defined (PRE_ADD_CHAOSCUBE)
	case eItemGoods::CS_CHAOSCUBEREQUEST:
		{
			if (sizeof(CSChaosCubeRequest) != nLen)
				return ERROR_INVALIDPACKET;
			if (m_pRestraint->CheckRestraint(_RESTRAINTTYPE_TRADE) == false)
				return ERROR_NONE;

			CSChaosCubeRequest *pItem = (CSChaosCubeRequest*)pData;
			if (!m_pItem->RequestChaosCube(pItem))
				IsWindowStateNoneSet(WINDOW_PROGRESS);
			return ERROR_NONE;
		}
		break;

	case eItemGoods::CS_CHAOSCUBECOMPLETE:
		{
			if (!m_pItem->IsValidRequestTimer(CDNUserItem::RequestType_UseRandomItem)) 
				break;

			if (sizeof(CSChaosCubeComplete) != nLen)
				return ERROR_INVALIDPACKET;

			CSChaosCubeComplete *pItem = (CSChaosCubeComplete *)pData;
			if (m_pItem->CompleteChaosCube(pItem) == false)
				_DANGER_POINT();
			IsWindowStateNoneSet(WINDOW_PROGRESS);
			return ERROR_NONE;
		}
		break;
	case eItemGoods::CS_CHAOSCUBECANCEL:
		{
			if (nLen != 0)
				return ERROR_INVALIDPACKET;

			m_pItem->SetRequestTimer(CDNUserItem::RequestType_None, 0);
			m_pSession->BroadcastingEffect(EffectType_Random, EffectState_Cancel);
			IsWindowStateNoneSet(WINDOW_PROGRESS);
			return ERROR_NONE;
		}
		break;

	case eItemGoods::CS_CHAOSGAUGECUBE_ADDSTUFF:
		{
			if (sizeof(CSChaosGaugeCubeAddStuff) != nLen)
				return ERROR_INVALIDPACKET;
			if (m_pRestraint->CheckRestraint(_RESTRAINTTYPE_TRADE) == false)
				return ERROR_NONE;

			CSChaosGaugeCubeAddStuff *pItem = (CSChaosGaugeCubeAddStuff*)pData;
			m_pItem->AddChaosGaugeStuff(pItem);
			return ERROR_NONE;
		}
		break;

	case eItemGoods::CS_CHAOSGAUGECUBE_COMPLETE:
		{
			if (sizeof(CSChaosGaugeCubeComplete) != nLen)
				return ERROR_INVALIDPACKET;
			if (m_pRestraint->CheckRestraint(_RESTRAINTTYPE_TRADE) == false)
				return ERROR_NONE;

			CSChaosGaugeCubeComplete *pItem = (CSChaosGaugeCubeComplete*)pData;
			m_pItem->CompleteChaosGaugeCube(pItem);
			return ERROR_NONE;
		}
		break;
#endif // #if defined (PRE_ADD_CHAOSCUBE)

#if defined (PRE_ADD_BESTFRIEND)
	case eItemGoods::CS_BESTFRIENDREQUEST:
		{
			if (sizeof(CSBestFriendItemRequest) != nLen)
				return ERROR_INVALIDPACKET;
			if (m_pRestraint->CheckRestraint(_RESTRAINTTYPE_TRADE) == false)
				return ERROR_NONE;

			CSBestFriendItemRequest *pItem = (CSBestFriendItemRequest*)pData;
			if (!m_pItem->RequestBestFriendItem(pItem))
				IsWindowStateNoneSet(WINDOW_PROGRESS);
			return ERROR_NONE;
		}
		break;
	case eItemGoods::CS_BESTFRIENDCOMPLETE:
		{
			if (!m_pItem->IsValidRequestTimer(CDNUserItem::RequestType_UseRandomItem)) break;

			if (sizeof(CSBestFriendItemComplete) != nLen)
				return ERROR_INVALIDPACKET;

			CSBestFriendItemComplete *pItem = (CSBestFriendItemComplete *)pData;
			if (m_pItem->CompleteBestFriendItem(pItem) == false)
				_DANGER_POINT();
			IsWindowStateNoneSet(WINDOW_PROGRESS);
			return ERROR_NONE;
		}
		break;
	case eItemGoods::CS_BESTFRIENDCANCEL:
		{
			if (nLen != 0)
				return ERROR_INVALIDPACKET;

			m_pItem->SetRequestTimer(CDNUserItem::RequestType_None, 0);
			m_pSession->BroadcastingEffect(EffectType_Random, EffectState_Cancel);
			IsWindowStateNoneSet(WINDOW_PROGRESS);
			return ERROR_NONE;
		}
		break;
#endif // #if defined (PRE_ADD_BESTFRIEND)

	case eItemGoods::CS_ENCHANTJEWEL:
		{
			if (sizeof(CSEnchantJewelItem) != nLen)
				return ERROR_INVALIDPACKET;

			CSEnchantJewelItem *pItem = (CSEnchantJewelItem *)pData;
			int nRet = m_pItem->EnchantJewel( pItem->cInvenIndex, pItem->biInvenSerial, pItem->biItemSerial );
			SendEnchantJewelItemResult( nRet, pItem->cInvenIndex );

			return ERROR_NONE;
		}
		break;

#if defined(PRE_ADD_REMOVE_PREFIX)
	case eItemGoods::CS_REMOVE_PREFIX:
		{
			if (sizeof(CSEnchantJewelItem) != nLen)
				return ERROR_INVALIDPACKET;

			CSEnchantJewelItem *pItem = (CSEnchantJewelItem *)pData;
			int nRet = m_pItem->RemovePrefix( pItem->cInvenIndex, pItem->biInvenSerial, pItem->biItemSerial );
			SendRemovePrefixItemResult( nRet, pItem->cInvenIndex );
			return ERROR_NONE;
		}
		break;
#endif // PRE_ADD_REMOVE_PREFIX

	case eItemGoods::CS_PETSKILLITEM :
		{
			if( sizeof(CSPetSkillItem) != nLen)
				return ERROR_INVALIDPACKET;

			CSPetSkillItem *pItem = (CSPetSkillItem*)pData;
			char cSlotNum = 0;
			int nSkillID = 0;
			int nRet = m_pItem->AddPetSkill( pItem->biItemSerial, cSlotNum, nSkillID );
			SendPetSkillItemResult(nRet, cSlotNum, nSkillID);
			return ERROR_NONE;
		}
		break;
	case eItemGoods::CS_PETSKILLEXPAND :
		{
			if( sizeof(CSPetSkillExpand) != nLen)
				return ERROR_INVALIDPACKET;

			CSPetSkillExpand *pItem = (CSPetSkillExpand*)pData;
			int nRet = m_pItem->PetSkillExpand(pItem->biItemSerial);
			SendPetSkillExpandResult(nRet);
			return ERROR_NONE;
		}
		break;

	case eItemGoods::CS_WARP_VILLAGE_LIST:
		{
			if (nLen != sizeof(CSWarpVillageList))
				return ERROR_INVALIDPACKET;

			return OnRecvWarpVillageList((CSWarpVillageList*)pData);
		}
		break;

	case eItemGoods::CS_WARP_VILLAGE:
		{
			if (nLen != sizeof(CSWarpVillage))
				return ERROR_INVALIDPACKET;

			return OnRecvWarpVillage((CSWarpVillage*)pData);
		}
		break;

#ifdef PRE_ADD_EXCHANGE_POTENTIAL
	case eItemGoods::CS_EXCHANGE_POTENTIAL:
		{
			CSExchangePotential * pPacket = (CSExchangePotential*)pData;

			// State - processing another task.
			if( !IsNoneWindowState() || IsWindowState(WINDOW_BLIND) )
			{
				_DANGER_POINT_MSG(L"EXCHANGE_POTENTIAL window");
				SendExchangePotoential(ERROR_GENERIC_INVALIDREQUEST);
			}
			else
			{
				if (GetItem() == NULL)
				{
					_DANGER_POINT_MSG(L"EXCHANGE_POTENTIAL GetItem");
					SendExchangePotoential(ERROR_GENERIC_UNKNOWNERROR);
					return ERROR_NONE;
				}

				//For testing for now
				int nRetCode = GetItem()->ExchangePotential(pPacket);
				SendExchangePotoential(nRetCode);
			}
			return ERROR_NONE;
		}
		break;
#endif		//#ifdef PRE_ADD_EXCHANGE_POTENTIAL
#if defined(PRE_ADD_EXCHANGE_ENCHANT)
	case eItemGoods::CS_EXCHANGE_ENCHANT:
		{
			CSExchangeEnchant * pPacket = (CSExchangeEnchant*)pData;

			// Enhancement transfer arrives in blind state..
			if( !IsWindowState(WINDOW_BLIND))							
				SendExchangeEnchant(ERROR_GENERIC_INVALIDREQUEST);			
			else
			{
				int nRetCode = m_pItem->ExchangeEnchant(pPacket);
				SendExchangeEnchant(nRetCode);
			}
			return ERROR_NONE;
		}
		break;
#endif //#if defined(PRE_ADD_EXCHANGE_ENCHANT)
	}

	return ERROR_UNKNOWN_HEADER;
}

int CDNUserBase::OnRecvTradeMessage(int nSubCmd, char *pData, int nLen)
{
	switch(nSubCmd)
	{
#if defined (PRE_MOD_GAMESERVERSHOP)
		// Shop
#if defined(PRE_ADD_REMOTE_OPENSHOP)
	case eTrade::CS_SHOP_REMOTEOPEN:
		{
			if (sizeof(CSShopBuy) != nLen)
				return ERROR_INVALIDPACKET;

			CSShopRemoteOpen *pShop = (CSShopRemoteOpen*)pData;

			m_pItem->OnRecvShopRemoteOpen(pShop->Type);
			SendShopOpen(m_nShopID, pShop->Type);
			return ERROR_NONE;
		}
		break;

#endif	// #if defined(PRE_ADD_REMOTE_OPENSHOP)

	case eTrade::CS_SHOP_BUY:	// Purchase
		{
#if defined(PRE_ADD_REMOTE_OPENSHOP)
			if (!m_bRemoteShopOpen && !IsValidPacketByNpcClick())	// Check NPC distance
#else	// #if defined(PRE_ADD_REMOTE_OPENSHOP)
			if (!IsValidPacketByNpcClick())	// Check NPC distance
#endif	// #if defined(PRE_ADD_REMOTE_OPENSHOP)
				return ERROR_GENERIC_INVALIDREQUEST;

			if( !IsWindowState(WINDOW_BLIND) ){
				SendShopBuyResult(ERROR_GENERIC_INVALIDREQUEST);
				return ERROR_NONE;	// Another window must not be open
			}

			if (m_pRestraint->CheckRestraint(_RESTRAINTTYPE_TRADE) == false)
				return ERROR_NONE;

			if (sizeof(CSShopBuy) != nLen)
				return ERROR_INVALIDPACKET;

			CSShopBuy *pBuy = (CSShopBuy*)pData;
			int nRet = m_pItem->OnRecvBuyNpcShopItem(m_nShopID, pBuy->cTapIndex, pBuy->cShopIndex, pBuy->wCount);
			SendShopBuyResult(nRet);
			return ERROR_NONE;
		}
		break;

	case eTrade::CS_SHOP_SELL:	// Sell
		{
#if defined(PRE_ADD_REMOTE_OPENSHOP)
			if (!m_bRemoteShopOpen && !IsValidPacketByNpcClick())	// Check NPC distance
#else	// #if defined(PRE_ADD_REMOTE_OPENSHOP)
			if (!IsValidPacketByNpcClick())	// Check NPC distance
#endif	// #if defined(PRE_ADD_REMOTE_OPENSHOP)
				return ERROR_GENERIC_INVALIDREQUEST;

			if( !IsWindowState(WINDOW_BLIND) ){ // If not in blind state..
				SendShopSellResult(ERROR_GENERIC_INVALIDREQUEST);
				return ERROR_NONE;	// Another window must not be open
			}

			if (m_pRestraint->CheckRestraint(_RESTRAINTTYPE_TRADE) == false && AccountLevel_DWC != GetAccountLevel())
				return ERROR_NONE;

			if (sizeof(CSShopSell) != nLen)
				return ERROR_INVALIDPACKET;

			// Applying 0.5s delay for selling to shop.
			DWORD dwCurTick = timeGetTime();
			if( m_dwResellItemTick && dwCurTick-m_dwResellItemTick < 500 )
			{
				//Take a break and stop messing around..
				SendShopSellResult(ERROR_GENERIC_TIME_WAIT);
				return ERROR_NONE;
			}			
#if defined(PRE_FIX_74387)
			if( m_bNowResellItem )
			{
				//This one is definitely a bot...				
				SendShopSellResult(ERROR_GENERIC_TIME_WAIT);
				return ERROR_NONE;
			}
#endif
			CSShopSell *pSell = (CSShopSell*)pData;
			int nRet = m_pItem->OnRecvSellNpcShopItem(pSell);
#if defined(PRE_FIX_74387)
			if( nRet != ERROR_NONE ) // Send response after DB round-trip.			
				SendShopSellResult(nRet);
			else
				m_bNowResellItem = true;
#else
			SendShopSellResult(nRet);
#endif //#if defined(PRE_FIX_74387)
			m_dwResellItemTick = dwCurTick;
			return ERROR_NONE;
		}
		break;

	case eTrade::CS_SHOP_REPURCHASE:
		{
#if defined(PRE_ADD_REMOTE_OPENSHOP)
			if (!m_bRemoteShopOpen && !IsValidPacketByNpcClick())	// Check NPC distance
#else	// #if defined(PRE_ADD_REMOTE_OPENSHOP)
			if (!IsValidPacketByNpcClick())	// Check NPC distance
#endif	// #if defined(PRE_ADD_REMOTE_OPENSHOP)
				return ERROR_GENERIC_INVALIDREQUEST;


			if ( !IsWindowState(WINDOW_BLIND)){
				SendShopRepurchase(ERROR_GENERIC_INVALIDREQUEST);
				return ERROR_NONE;	// Another window must not be open
			}

			if (m_pRestraint->CheckRestraint(_RESTRAINTTYPE_TRADE) == false)
				return ERROR_NONE;

			if ( sizeof(CSShopRepurchase) != nLen)
				return ERROR_INVALIDPACKET;

			CSShopRepurchase* pPacket = reinterpret_cast<CSShopRepurchase*>(pData);
			int iRet = m_pItem->OnRecvShopRepurchase( pPacket );
			SendShopRepurchase( iRet, pPacket->iRepurchaseID );
			return ERROR_NONE;
		}

	case eTrade::CS_SHOP_GETLIST_REPURCHASE:
		{
#if defined(PRE_ADD_REMOTE_OPENSHOP)
			if (!m_bRemoteShopOpen && !IsValidPacketByNpcClick())	// Check NPC distance
#else	// #if defined(PRE_ADD_REMOTE_OPENSHOP)
			if (!IsValidPacketByNpcClick())	// Check NPC distance
#endif	// #if defined(PRE_ADD_REMOTE_OPENSHOP)
				return ERROR_GENERIC_INVALIDREQUEST;

			if ( !IsWindowState(WINDOW_BLIND)){
				SendShopRepurchaseList(ERROR_GENERIC_INVALIDREQUEST);
				return ERROR_NONE;	// Another window must not be open
			}

			if (m_pRestraint->CheckRestraint(_RESTRAINTTYPE_TRADE) == false)
				return ERROR_NONE;

			if (0 != nLen)
				return ERROR_INVALIDPACKET;

			GetDBConnection()->QueryGetListRepurchaseItem( m_pSession );
			return ERROR_NONE;
		}

	case eTrade::CS_REPAIR_EQUIP:	// Repair currently equipped
		{
			if (!IsValidPacketByNpcClick())	// Check NPC distance
				return ERROR_GENERIC_INVALIDREQUEST;


			if (nLen != 0)
				return ERROR_INVALIDPACKET;

			m_pItem->OnRecvRepairEquip();
			return ERROR_NONE;
		}
		break;

	case eTrade::CS_REPAIR_ALL:		// Repair all
		{
			if (!IsValidPacketByNpcClick())	// Check NPC distance
				return ERROR_GENERIC_INVALIDREQUEST;


			if (nLen != 0)
				return ERROR_INVALIDPACKET;

			m_pItem->OnRecvRepairAll();
			return ERROR_NONE;
		}
		break;
#endif // #if defined (PRE_MOD_GAMESERVERSHOP)

		// exchange personal trade
	case eTrade::CS_EXCHANGE_REQUEST:		// Request trade
		{
			if (sizeof(CSExchangeRequest) != nLen)
				return ERROR_INVALIDPACKET;

			CSExchangeRequest *pRequest = (CSExchangeRequest*)pData;

			//When I am banned/restricted
			if (m_pRestraint->CheckRestraint(_RESTRAINTTYPE_TRADE) == false)
			{
				SendExchangeReject(pRequest->nReceiverSessionID, ERROR_EXCHANGE_CANTEXCHANGE);
				return ERROR_NONE;
			}

			if( GetLevel() < static_cast<int>(CGlobalWeightTable::GetInstance().GetValue( CGlobalWeightTable::LimitLevel_Exchange )) )
			{
				SendExchangeReject(pRequest->nReceiverSessionID, ERROR_GENERIC_LEVELLIMIT);
				return ERROR_NONE;
			}
#if defined(_VILLAGESERVER)
			CDNUserSession *pUserSession = g_pUserSessionManager->FindUserSession(pRequest->nReceiverSessionID);
#elif defined(_GAMESERVER)
			CDNUserSession *pUserSession = m_pSession->FindUserSession(pRequest->nReceiverSessionID);	// Find user
#endif
			if (!pUserSession){
				SendExchangeRequest(pRequest->nReceiverSessionID, ERROR_EXCHANGE_USERNOTFOUND);
				return ERROR_NONE;
			}

			// Check GM trade
			if ((GetAccountLevel() >= AccountLevel_New) && (GetAccountLevel() < AccountLevel_Master)){	// Tradeable only by master or above
				SendExchangeRequest(pRequest->nReceiverSessionID, ERROR_EXCHANGE_FAIL);
				return ERROR_NONE;
			}

#if defined(PRE_ADD_36935)
			// Check trade eligibility based on partner's level
			if ((pUserSession->GetAccountLevel() >= AccountLevel_New) && (pUserSession->GetAccountLevel() < AccountLevel_Master)){	// Tradeable only by master or above
				SendExchangeRequest(pRequest->nReceiverSessionID, ERROR_EXCHANGE_FAIL);
				return ERROR_NONE;
			}
#endif // #if defined(PRE_ADD_36935)

#if defined(_VILLAGESERVER)
			if (GetChannelID() != pUserSession->GetChannelID()){	// Cannot trade unless in the same channel
				SendExchangeRequest(pRequest->nReceiverSessionID, ERROR_EXCHANGE_FAIL);
				return ERROR_NONE;
			}
#elif defined(_GAMESERVER)

#if defined( PRE_WORLDCOMBINE_PARTY )
			if( m_pSession->GetGameRoom()->bIsWorldCombineParty() )
			{
				SendExchangeRequest(pRequest->nReceiverSessionID, ERROR_EXCHANGE_FAIL);
				return ERROR_NONE;
			}
#endif // #if defined( PRE_WORLDCOMBINE_PARTY )

			if (GetRoomID() != pUserSession->GetRoomID()){	// Cannot trade unless in the same room
				SendExchangeRequest(pRequest->nReceiverSessionID, ERROR_EXCHANGE_FAIL);
				return ERROR_NONE;
			}

			if (m_pSession->GetPlayerActor()->IsDie() || m_pSession->GetPlayerActor()->IsGhost()){	// Must not be dead or a ghost
				SendExchangeRequest(pRequest->nReceiverSessionID, ERROR_EXCHANGE_FAIL);
				return ERROR_NONE;
			}

			if (pUserSession->GetPlayerActor()->IsDie() || pUserSession->GetPlayerActor()->IsGhost()){	// Must not be dead or a ghost
				SendExchangeRequest(pRequest->nReceiverSessionID, ERROR_EXCHANGE_FAIL);
				return ERROR_NONE;
			}
#if defined(PRE_ADD_EQUIPLOCK)
			//Block trading if game mode is Ghoul mode
			if (!pUserSession->GetGameRoom() || pUserSession->GetGameRoom()->bIsZombieMode())
			{
				SendExchangeRequest(pRequest->nReceiverSessionID, ERROR_EXCHANGE_FAIL);
				return ERROR_NONE;
			}
#endif	// #if defined(PRE_ADD_EQUIPLOCK)
#endif

			//Cannot request trade with a blocked user
			if( m_pIsolate && m_pIsolate->IsIsolateItem(pUserSession->GetCharacterName()))
			{
				SendIsolateResult(ERROR_ISOLATE_REQUESTFAIL);
				return ERROR_NONE;
			}

			//If the trade recipient has blocked me
			if( pUserSession->GetIsolate() && pUserSession->GetIsolate()->IsIsolateItem(GetCharacterName()))
			{
				SendExchangeReject(pRequest->nReceiverSessionID);
				return ERROR_NONE;
			}

			//The trade recipient is currently under a trade suspension penalty
			if (pUserSession->GetRestraint()->CheckRestraint(_RESTRAINTTYPE_TRADE, false) == false)
			{
				SendExchangeReject(pRequest->nReceiverSessionID, ERROR_EXCHANGE_CANTEXCHANGE);
				return ERROR_NONE;
			}

			//Check target's game options
			if (pUserSession->IsAcceptAbleOption(GetCharacterDBID(), GetAccountDBID(), _ACCEPTABLE_CHECKTYPE_TRADEREQUEST) == false)
			{
				SendExchangeReject(pRequest->nReceiverSessionID, ERROR_EXCHANGE_CANTEXCHANGE);
				return ERROR_NONE;
			}		

			if( pUserSession->GetLevel() < static_cast<int>(CGlobalWeightTable::GetInstance().GetValue( CGlobalWeightTable::LimitLevel_Exchange )) )
			{
				SendExchangeReject(pRequest->nReceiverSessionID, ERROR_GENERIC_LEVELLIMIT);
				return ERROR_NONE;
			}

			if( !IsNoneWindowState() || !pUserSession->IsNoneWindowState() ) 
			{
				// If my state is not None, it may be a cancellation.
				if( IsWindowState(WINDOW_EXCHANGE) && pRequest->bCancel && pUserSession->IsWindowState(WINDOW_EXCHANGE) && m_nExchangeReceiverSID == pRequest->nReceiverSessionID)
				{
					pUserSession->SendExchangeRequest(m_nSessionID, ERROR_EXCHANGE_SENDERCANCEL);	// Hide accept/decline button window
					ClearExchangeInfo();
					pUserSession->ClearExchangeInfo();
					return ERROR_NONE;
				}
				// Otherwise cannot trade.. (the other player may be doing something else)
				SendExchangeRequest(pRequest->nReceiverSessionID, ERROR_EXCHANGE_FAIL);
				return ERROR_NONE;
			}
			SetWindowState(WINDOW_EXCHANGE);
			pUserSession->SetWindowState(WINDOW_EXCHANGE);
			pUserSession->SendExchangeRequest(m_nSessionID, ERROR_NONE);	// Request accept/decline button window
			m_nExchangeReceiverSID = pRequest->nReceiverSessionID;	// Save requesting session
			pUserSession->m_nExchangeSenderSID = m_nSessionID;

			return ERROR_NONE;
		}
		break;

	case eTrade::CS_EXCHANGE_ACCEPT:		// Accept trade
		{
			if (sizeof(CSExchangeAccept) != nLen)
				return ERROR_INVALIDPACKET;

			CSExchangeAccept *pAccept = (CSExchangeAccept*)pData;

#if defined(_VILLAGESERVER)
			CDNUserSession *pUserSession = g_pUserSessionManager->FindUserSession(pAccept->nSenderSessionID);
#elif defined(_GAMESERVER)
			CDNUserSession *pUserSession = m_pSession->FindUserSession(pAccept->nSenderSessionID);	// Find user
#endif
			if (!pUserSession){
				SendExchangeStart(pAccept->nSenderSessionID, ERROR_EXCHANGE_USERNOTFOUND);
				ClearExchangeInfo();
				return ERROR_NONE;
			}
			// At the moment the request is sent, both must already be switched to WINDOW_EXCHANGE
			if( !IsWindowState(WINDOW_EXCHANGE) || !pUserSession->IsWindowState(WINDOW_EXCHANGE) )
			{
				SendExchangeStart(pAccept->nSenderSessionID, ERROR_EXCHANGE_FAIL);
				pUserSession->SendExchangeStart(m_nSessionID, ERROR_EXCHANGE_FAIL);
#ifdef PRE_FIX_CANCELTRADE
				if (pUserSession->IsWindowState(WINDOW_EXCHANGE))
					pUserSession->ClearExchangeInfo();
#endif
				return ERROR_NONE;	// Another window must not be open
			}
			else
			{				
				if( pUserSession->m_nExchangeReceiverSID != m_nSessionID || m_nExchangeSenderSID != pAccept->nSenderSessionID )
				{
					//Mismatched pair.. what is this..?
					SendExchangeStart(pAccept->nSenderSessionID, ERROR_EXCHANGE_FAIL);
					ClearExchangeInfo();
					return ERROR_NONE;
				}
			}

#if defined(_VILLAGESERVER)
			if (GetChannelID() != pUserSession->GetChannelID()){	// Cannot trade unless in the same channel
				SendExchangeStart(pAccept->nSenderSessionID, ERROR_EXCHANGE_FAIL);
				pUserSession->SendExchangeStart(m_nSessionID, ERROR_EXCHANGE_FAIL);

				ClearExchangeInfo();
				pUserSession->ClearExchangeInfo();
				return ERROR_NONE;
			}

#elif defined(_GAMESERVER)
			if (GetRoomID() != pUserSession->GetRoomID()){
				SendExchangeStart(pAccept->nSenderSessionID, ERROR_EXCHANGE_FAIL);
				pUserSession->SendExchangeStart(m_nSessionID, ERROR_EXCHANGE_FAIL);

				ClearExchangeInfo();
				pUserSession->ClearExchangeInfo();
				return ERROR_NONE;
			}

			if (m_pSession->GetPlayerActor()->IsDie() || m_pSession->GetPlayerActor()->IsGhost()){	// Must not be dead or a ghost
				SendExchangeStart(pAccept->nSenderSessionID, ERROR_EXCHANGE_FAIL);
				pUserSession->SendExchangeStart(m_nSessionID, ERROR_EXCHANGE_FAIL);

				ClearExchangeInfo();
				pUserSession->ClearExchangeInfo();
				return ERROR_NONE;
			}

			if (pUserSession->GetPlayerActor()->IsDie() || pUserSession->GetPlayerActor()->IsGhost()){	// Must not be dead or a ghost
				SendExchangeStart(pAccept->nSenderSessionID, ERROR_EXCHANGE_FAIL);
				pUserSession->SendExchangeStart(m_nSessionID, ERROR_EXCHANGE_FAIL);

				ClearExchangeInfo();
				pUserSession->ClearExchangeInfo();
				return ERROR_NONE;
			}
#if defined(PRE_ADD_EQUIPLOCK)
			//Block trading if game mode is Ghoul mode
			if (!pUserSession->GetGameRoom() || pUserSession->GetGameRoom()->bIsZombieMode())
			{
				SendExchangeStart(pAccept->nSenderSessionID, ERROR_EXCHANGE_FAIL);
				pUserSession->SendExchangeStart(m_nSessionID, ERROR_EXCHANGE_FAIL);

				ClearExchangeInfo();
				pUserSession->ClearExchangeInfo();
				return ERROR_NONE;
			}
#endif	// #if defined(PRE_ADD_EQUIPLOCK)
#endif
			if (pAccept->bAccept){	// Accept
				// Set accepted user
				SetExchangeTargetSessionID(pAccept->nSenderSessionID);
				SendExchangeStart(pAccept->nSenderSessionID, ERROR_NONE);

				// Set requesting user
				pUserSession->SetExchangeTargetSessionID(m_nSessionID);
				pUserSession->SendExchangeStart(m_nSessionID, ERROR_NONE);
			}
			else {	// Reject
				pUserSession->SendExchangeReject(m_nSessionID);
				ClearExchangeInfo();
				pUserSession->ClearExchangeInfo();
			}

			return ERROR_NONE;
		}
		break;

	case eTrade::CS_EXCHANGE_ADDITEM:		// Register item
		{
			if (sizeof(CSExchangeAddItem) != nLen)
				return ERROR_INVALIDPACKET;

			CSExchangeAddItem *pAddItem = (CSExchangeAddItem*)pData;

			// #23747 Related to character data anomaly and equipment duplication risk
			if( pAddItem->wCount <= 0 )
				return ERROR_INVALIDPACKET;

			TItem Item = { 0, };
#if defined(_VILLAGESERVER)
			CDNUserSession *pUserSession = g_pUserSessionManager->FindUserSession(m_nExchangeTargetSessionID);
#elif defined(_GAMESERVER)
			CDNUserSession *pUserSession = m_pSession->FindUserSession(m_nExchangeTargetSessionID);	// Find user
#endif
			if (!pUserSession){
				SendExchangeAddItem(m_nSessionID, pAddItem->cExchangeIndex, -1, Item, ERROR_EXCHANGE_USERNOTFOUND);
				return ERROR_NONE;
			}

#if defined(_VILLAGESERVER)
			if (GetChannelID() != pUserSession->GetChannelID()){	// Cannot trade unless in the same channel
				SendExchangeCancel();
				ClearExchangeInfo();

				pUserSession->SendExchangeCancel();
				pUserSession->ClearExchangeInfo();

				return ERROR_NONE;
			}
#elif defined(_GAMESERVER)
			if (GetRoomID() != pUserSession->GetRoomID()){
				SendExchangeCancel();
				ClearExchangeInfo();
				pUserSession->SendExchangeCancel();
				pUserSession->ClearExchangeInfo();
				return ERROR_NONE;
			}
			if (m_pSession->GetPlayerActor()->IsDie() || m_pSession->GetPlayerActor()->IsGhost()){	// Must not be dead or a ghost
				SendExchangeCancel();
				ClearExchangeInfo();

				pUserSession->SendExchangeCancel();
				pUserSession->ClearExchangeInfo();
				return ERROR_NONE;
			}
			if (pUserSession->GetPlayerActor()->IsDie() || pUserSession->GetPlayerActor()->IsGhost()){	// Must not be dead or a ghost
				SendExchangeCancel();
				ClearExchangeInfo();

				pUserSession->SendExchangeCancel();
				pUserSession->ClearExchangeInfo();
				return ERROR_NONE;
			}
#endif
			if ( !IsWindowState(WINDOW_EXCHANGE) ){
				SendExchangeCancel();
				ClearExchangeInfo();

				pUserSession->SendExchangeCancel();
				pUserSession->ClearExchangeInfo();

				return ERROR_NONE;
			}


			// If more items are added while in registered state
			if (m_pItem->GetExchangeRegist()){
				SendExchangeCancel();
				ClearExchangeInfo();

				pUserSession->SendExchangeCancel();
				pUserSession->ClearExchangeInfo();
				return ERROR_NONE;
			}

			// If more items are added while in completed state
			if (m_pItem->GetExchangeConfirm()){
				SendExchangeCancel();
				ClearExchangeInfo();

				pUserSession->SendExchangeCancel();
				pUserSession->ClearExchangeInfo();
				return ERROR_NONE;
			}

			char cExchangeIndex = -1;
			int nRet = m_pItem->AddExchangeItem(pAddItem);
			if (nRet != ERROR_NONE){
				SendExchangeAddItem(m_nSessionID, pAddItem->cExchangeIndex, -1, Item, nRet);
				return ERROR_NONE;
			}

			// Send to both sides only when successful
			if (m_pItem->GetInventory(pAddItem->cInvenIndex)){
				Item = *(m_pItem->GetInventory(pAddItem->cInvenIndex));
				Item.wCount = pAddItem->wCount;
			}

			SendExchangeAddItem(m_nSessionID, cExchangeIndex, pAddItem->cInvenIndex, Item, ERROR_NONE);
			pUserSession->SendExchangeAddItem(m_nSessionID, pAddItem->cExchangeIndex, pAddItem->cInvenIndex, Item, ERROR_NONE);

			return ERROR_NONE;
		}
		break;

	case eTrade::CS_EXCHANGE_DELETEITEM:	// Unregister item
		{
			if (sizeof(CSExchangeDeleteItem) != nLen)
				return ERROR_INVALIDPACKET;

			CSExchangeDeleteItem *pDeleteItem = (CSExchangeDeleteItem*)pData;

#if defined(_VILLAGESERVER)
			CDNUserSession *pUserSession = g_pUserSessionManager->FindUserSession(m_nExchangeTargetSessionID);
#elif defined(_GAMESERVER)
			CDNUserSession *pUserSession = m_pSession->FindUserSession(m_nExchangeTargetSessionID);	// Find user
#endif
			if (!pUserSession){
				SendExchangeDeleteItem(m_nSessionID, pDeleteItem->cExchangeIndex, ERROR_EXCHANGE_USERNOTFOUND);
				return ERROR_NONE;
			}

#if defined(_VILLAGESERVER)
			if (GetChannelID() != pUserSession->GetChannelID()){	// Cannot trade unless in the same channel
				SendExchangeCancel();
				ClearExchangeInfo();

				pUserSession->SendExchangeCancel();
				pUserSession->ClearExchangeInfo();

				return ERROR_NONE;
			}

#elif defined(_GAMESERVER)
			if (GetRoomID() != pUserSession->GetRoomID()){
				SendExchangeCancel();
				ClearExchangeInfo();

				pUserSession->SendExchangeCancel();
				pUserSession->ClearExchangeInfo();
				return ERROR_NONE;
			}

			if (m_pSession->GetPlayerActor()->IsDie() || m_pSession->GetPlayerActor()->IsGhost()){	// Must not be dead or a ghost
				SendExchangeCancel();
				ClearExchangeInfo();

				pUserSession->SendExchangeCancel();
				pUserSession->ClearExchangeInfo();
				return ERROR_NONE;
			}

			if (pUserSession->GetPlayerActor()->IsDie() || pUserSession->GetPlayerActor()->IsGhost()){	// Must not be dead or a ghost
				SendExchangeCancel();
				ClearExchangeInfo();

				pUserSession->SendExchangeCancel();
				pUserSession->ClearExchangeInfo();
				return ERROR_NONE;
			}
#endif
			if ( !IsWindowState(WINDOW_EXCHANGE) ){
				SendExchangeCancel();
				ClearExchangeInfo();

				pUserSession->SendExchangeCancel();
				pUserSession->ClearExchangeInfo();
				return ERROR_NONE;
			}

			// If more items are deleted while in registered state
			if (m_pItem->GetExchangeRegist()){
				SendExchangeCancel();
				ClearExchangeInfo();

				pUserSession->SendExchangeCancel();
				pUserSession->ClearExchangeInfo();
				return ERROR_NONE;
			}

			// If more items are deleted while in completed state
			if (m_pItem->GetExchangeConfirm()){
				SendExchangeCancel();
				ClearExchangeInfo();

				pUserSession->SendExchangeCancel();
				pUserSession->ClearExchangeInfo();
				return ERROR_NONE;
			}

			int nRet = m_pItem->DeleteExchangeItem(pDeleteItem->cExchangeIndex);
			if (nRet != ERROR_NONE){
				SendExchangeDeleteItem(m_nSessionID, pDeleteItem->cExchangeIndex, ERROR_EXCHANGE_FAIL);
				return ERROR_NONE;
			}

			// Send to both sides only when successful
			SendExchangeDeleteItem(m_nSessionID, pDeleteItem->cExchangeIndex, ERROR_NONE);
			pUserSession->SendExchangeDeleteItem(m_nSessionID, pDeleteItem->cExchangeIndex, ERROR_NONE);

			return ERROR_NONE;
		}
		break;

	case eTrade::CS_EXCHANGE_ADDCOIN:		// Register coin
		{
			if (sizeof(CSExchangeAddCoin) != nLen)
				return ERROR_INVALIDPACKET;

			CSExchangeAddCoin *pAddCoin = (CSExchangeAddCoin*)pData;

#if defined(_VILLAGESERVER)
			CDNUserSession *pUserSession = g_pUserSessionManager->FindUserSession(m_nExchangeTargetSessionID);
#elif defined(_GAMESERVER)
			CDNUserSession *pUserSession = m_pSession->FindUserSession(m_nExchangeTargetSessionID);	// Find user
#endif
			if (!pUserSession){
				SendExchangeAddCoin(m_nSessionID, pAddCoin->nCoin, ERROR_EXCHANGE_USERNOTFOUND);
				return ERROR_NONE;
			}

#if defined(_VILLAGESERVER)
			if (GetChannelID() != pUserSession->GetChannelID()){	// Cannot trade unless in the same channel
				SendExchangeCancel();
				ClearExchangeInfo();

				pUserSession->SendExchangeCancel();
				pUserSession->ClearExchangeInfo();

				return ERROR_NONE;
			}

#elif defined(_GAMESERVER)
			if (GetRoomID() != pUserSession->GetRoomID()){
				SendExchangeCancel();
				ClearExchangeInfo();

				pUserSession->SendExchangeCancel();
				pUserSession->ClearExchangeInfo();
				return ERROR_NONE;
			}

			if (m_pSession->GetPlayerActor()->IsDie() || m_pSession->GetPlayerActor()->IsGhost()){	// Must not be dead or a ghost
				SendExchangeCancel();
				ClearExchangeInfo();

				pUserSession->SendExchangeCancel();
				pUserSession->ClearExchangeInfo();
				return ERROR_NONE;
			}

			if (pUserSession->GetPlayerActor()->IsDie() || pUserSession->GetPlayerActor()->IsGhost()){	// Must not be dead or a ghost
				SendExchangeCancel();
				ClearExchangeInfo();

				pUserSession->SendExchangeCancel();
				pUserSession->ClearExchangeInfo();
				return ERROR_NONE;
			}
#endif
			if ( !IsWindowState(WINDOW_EXCHANGE) ){
				SendExchangeCancel();
				ClearExchangeInfo();

				pUserSession->SendExchangeCancel();
				pUserSession->ClearExchangeInfo();
				return ERROR_NONE;
			}

			// If coin is added while in registered state
			if (m_pItem->GetExchangeRegist()){
				SendExchangeCancel();
				ClearExchangeInfo();

				pUserSession->SendExchangeCancel();
				pUserSession->ClearExchangeInfo();
				return ERROR_NONE;
			}

			// If coin is added while in completed state
			if (m_pItem->GetExchangeConfirm()){
				SendExchangeCancel();
				ClearExchangeInfo();

				pUserSession->SendExchangeCancel();
				pUserSession->ClearExchangeInfo();
				return ERROR_NONE;
			}

			if (!CheckEnoughCoin(pAddCoin->nCoin)){
				SendExchangeAddCoin(m_nSessionID, pAddCoin->nCoin, ERROR_ITEM_INSUFFICIENCY_MONEY);
				return ERROR_NONE;
			}

			int nRet = m_pItem->AddExchangeCoin(pAddCoin->nCoin);
			if (nRet != ERROR_NONE){
				SendExchangeAddCoin(m_nSessionID, pAddCoin->nCoin, ERROR_EXCHANGE_FAIL);
				return ERROR_NONE;
			}

			// Send to both sides only when successful
			SendExchangeAddCoin(m_nSessionID, pAddCoin->nCoin, ERROR_NONE);
			pUserSession->SendExchangeAddCoin(m_nSessionID, pAddCoin->nCoin, ERROR_NONE);

			return ERROR_NONE;
		}
		break;

	case eTrade::CS_EXCHANGE_CONFIRM:		// Confirm
		{
			if (sizeof(CSExchangeConfirm) != nLen)
				return ERROR_INVALIDPACKET;

			CSExchangeConfirm *pConfirm = (CSExchangeConfirm*)pData;			

#if defined(_VILLAGESERVER)
			CDNUserSession *pUserSession = g_pUserSessionManager->FindUserSession(m_nExchangeTargetSessionID);
#elif defined(_GAMESERVER)
			CDNUserSession *pUserSession = m_pSession->FindUserSession(m_nExchangeTargetSessionID);	// Find user
#endif
			if (!pUserSession){
				SendExchangeCancel();
				ClearExchangeInfo();
				return ERROR_NONE;
			}

			// Bad actors in China
			if ( GetAccountDBID() == pUserSession->GetAccountDBID() )
			{
				SendExchangeCancel();
				ClearExchangeInfo();

				pUserSession->SendExchangeCancel();
				pUserSession->ClearExchangeInfo();

				m_pSession->DetachConnection(L"TradeHack");
				g_Log.Log(LogType::_HACK, m_pSession, L"[ADBID:%u CDBID:%I64d SID:%u] Same Account Trade Hack\r\n", m_nAccountDBID, m_biCharacterDBID, m_nSessionID);
				pUserSession->DetachConnection(L"TradeHack");
				g_Log.Log(LogType::_HACK, m_pSession, L"[ADBID:%u CDBID:%I64d SID:%u] Same Account Trade Hack\r\n", pUserSession->GetAccountDBID(), pUserSession->GetCharacterDBID(), pUserSession->GetSessionID() );

				return ERROR_NONE;
			}

			//Cannot continue trade if a trade restriction is applied mid-trade
			if (m_pRestraint->CheckRestraint(_RESTRAINTTYPE_TRADE) == false || \
				pUserSession->GetRestraint()->CheckRestraint(_RESTRAINTTYPE_TRADE) == false)
			{
				SendExchangeCancel();
				ClearExchangeInfo();

				pUserSession->SendExchangeCancel();
				pUserSession->ClearExchangeInfo();
				return ERROR_NONE;
			}

#if defined(_VILLAGESERVER)
			if (GetChannelID() != pUserSession->GetChannelID()){	// Cannot trade unless in the same channel
				SendExchangeCancel();
				ClearExchangeInfo();

				pUserSession->SendExchangeCancel();
				pUserSession->ClearExchangeInfo();
				return ERROR_NONE;
			}

#elif defined(_GAMESERVER)
			if (GetRoomID() != pUserSession->GetRoomID()){
				SendExchangeCancel();
				ClearExchangeInfo();

				pUserSession->SendExchangeCancel();
				pUserSession->ClearExchangeInfo();
				return ERROR_NONE;
			}

			if (m_pSession->GetPlayerActor()->IsDie() || m_pSession->GetPlayerActor()->IsGhost()){	// Must not be dead or a ghost
				SendExchangeCancel();
				ClearExchangeInfo();

				pUserSession->SendExchangeCancel();
				pUserSession->ClearExchangeInfo();
				return ERROR_NONE;
			}

			if (pUserSession->GetPlayerActor()->IsDie() || pUserSession->GetPlayerActor()->IsGhost()){	// Must not be dead or a ghost
				SendExchangeCancel();
				ClearExchangeInfo();

				pUserSession->SendExchangeCancel();
				pUserSession->ClearExchangeInfo();
				return ERROR_NONE;
			}
#if defined(PRE_ADD_EQUIPLOCK)
			//Block trading if game mode is Ghoul mode
			if (!pUserSession->GetGameRoom() || pUserSession->GetGameRoom()->bIsZombieMode())
			{
				SendExchangeCancel();
				ClearExchangeInfo();

				pUserSession->SendExchangeCancel();
				pUserSession->ClearExchangeInfo();
				return ERROR_NONE;
			}
#endif	// #if defined(PRE_ADD_EQUIPLOCK)
#endif
			if ( !IsWindowState(WINDOW_EXCHANGE) ){
				SendExchangeCancel();
				ClearExchangeInfo();

				pUserSession->SendExchangeCancel();
				pUserSession->ClearExchangeInfo();
				return ERROR_NONE;
			}

			switch(pConfirm->cType)
			{
			case EXCHANGE_REGIST:
				{
					if (m_pItem->GetExchangeRegist() || m_pItem->GetExchangeConfirm()){	// Cancel if already registered or final-confirmed and received again
						SendExchangeCancel();
						ClearExchangeInfo();

						pUserSession->SendExchangeCancel();
						pUserSession->ClearExchangeInfo();
						return ERROR_NONE;
					}

					m_pItem->SetExchangeRegist();	// Button settings
					SendExchangeConfirm(m_nSessionID, pConfirm->cType);
					pUserSession->SendExchangeConfirm(m_nSessionID, pConfirm->cType);
				break;
				}

			case EXCHANGE_CONFIRM:
				{
					if (!m_pItem->GetExchangeRegist() || m_pItem->GetExchangeConfirm()){	// Cancel if not in registered state or already in final confirmed state
						SendExchangeCancel();
						ClearExchangeInfo();

						pUserSession->SendExchangeCancel();
						pUserSession->ClearExchangeInfo();
						return ERROR_NONE;
					}

					m_pItem->SetExchangeConfirm();	// Button settings
					SendExchangeConfirm(m_nSessionID, pConfirm->cType);
					pUserSession->SendExchangeConfirm(m_nSessionID, pConfirm->cType);
				}
				break;
			}

			// If both are in confirmed state...
			if (m_pItem->GetExchangeConfirm() && pUserSession->GetItem()->GetExchangeConfirm()){
				// Check coin
				if (!CheckMaxCoin(pUserSession->GetItem()->GetExchangeCoin()) || !pUserSession->CheckMaxCoin(m_pItem->GetExchangeCoin())){
					SendExchangeComplete(ERROR_EXCHANGE_OVERFLOWMONEY);
					pUserSession->SendExchangeComplete(ERROR_EXCHANGE_OVERFLOWMONEY);

					ClearExchangeInfo();
					pUserSession->ClearExchangeInfo();

					return ERROR_NONE;
				}

				// Insufficient inven space
				if ((!m_pItem->IsValidExchange(pUserSession->GetItem()->FindExchangeCount())) ||(!pUserSession->GetItem()->IsValidExchange(m_pItem->FindExchangeCount()))){
					SendExchangeComplete(ERROR_ITEM_INVENTORY_NOTENOUGH);
					pUserSession->SendExchangeComplete(ERROR_ITEM_INVENTORY_NOTENOUGH);

					ClearExchangeInfo();
					pUserSession->ClearExchangeInfo();

					return ERROR_NONE;
				}

				DBPacket::TExchangeItem ExchangeInfo[EXCHANGEMAX] = { 0, }, TargetExchangeInfo[EXCHANGEMAX] = { 0, };
				int nExchangeTax = 0, nTargetExchangeTax = 0;
				if ((!m_pItem->CompleteExchange(pUserSession, ExchangeInfo, nExchangeTax)) ||(!pUserSession->GetItem()->CompleteExchange(m_pSession, TargetExchangeInfo, nTargetExchangeTax))){
					SendExchangeComplete(ERROR_EXCHANGE_FAIL);
					pUserSession->SendExchangeComplete(ERROR_EXCHANGE_FAIL);

					ClearExchangeInfo();
					pUserSession->ClearExchangeInfo();

					return ERROR_NONE;
				}

				m_pDBCon->QueryExchangeProperty(m_pSession, ExchangeInfo, m_pItem->GetExchangeCoin(), nExchangeTax, pUserSession->GetCharacterDBID(), 
					TargetExchangeInfo, pUserSession->GetItem()->GetExchangeCoin(), nTargetExchangeTax, pUserSession->GetAccountDBID());

				SendExchangeComplete(ERROR_NONE);
				pUserSession->SendExchangeComplete(ERROR_NONE);

				ClearExchangeInfo();
				pUserSession->ClearExchangeInfo();
			}

			return ERROR_NONE;
		}
		break;

	case eTrade::CS_EXCHANGE_CANCEL:		// Cancel
		{
			if (nLen != 0)
				return ERROR_INVALIDPACKET;

#if defined(_VILLAGESERVER)
			CDNUserSession *pUserSession = g_pUserSessionManager->FindUserSession(m_nExchangeTargetSessionID);
#elif defined(_GAMESERVER)
			CDNUserSession *pUserSession = m_pSession->FindUserSession(m_nExchangeTargetSessionID);	// Find user
#endif
			if (pUserSession){
				pUserSession->SendExchangeCancel();
				pUserSession->ClearExchangeInfo();
			}

			SendExchangeCancel();
			ClearExchangeInfo();

			return ERROR_NONE;
		}
		break;

#if defined(PRE_SPECIALBOX)
	case eTrade::CS_SPECIALBOX_LIST:
		{
			m_pDBCon->QueryGetListEventReward(m_pSession);
		}
		break;

	case eTrade::CS_SPECIALBOX_ITEMLIST:
		{
			if (!IsWindowState(WINDOW_BLIND)){
				SendSpecialBoxItemList(ERROR_GENERIC_INVALIDREQUEST, 0, NULL, 0);
				return ERROR_NONE;	// Another window must not be open			
			}
			
			if (sizeof(CSSpecialBoxItemList) != nLen)
				return ERROR_INVALIDPACKET;

			CSSpecialBoxItemList *pSpecialBox = (CSSpecialBoxItemList*)pData;

			if (pSpecialBox->nEventRewardID <= 0){
				SendSpecialBoxItemList(ERROR_GENERIC_INVALIDREQUEST, 0, NULL, 0);
				return ERROR_NONE;
			}

			m_pDBCon->QueryGetListEventRewardItem(m_pSession, pSpecialBox->nEventRewardID);
		}
		break;

	case eTrade::CS_SPECIALBOX_RECEIVEITEM:
		{
			if (!IsWindowState(WINDOW_BLIND)){
				SendReceiveSpecialBoxItem(ERROR_GENERIC_INVALIDREQUEST);
				return ERROR_NONE;	// Another window must not be open			
			}

			if (sizeof(CSReceiveSpecialBoxItem) != nLen)
				return ERROR_INVALIDPACKET;

			CSReceiveSpecialBoxItem *pSpecialBox = (CSReceiveSpecialBoxItem*)pData;

			if (pSpecialBox->nEventRewardID <= 0){
				SendReceiveSpecialBoxItem(ERROR_GENERIC_INVALIDREQUEST);
				return ERROR_NONE;
			}

			int nRet = m_pItem->CheckReceiveSpecialBox(pSpecialBox->nEventRewardID, pSpecialBox->nItemID);
			if (nRet != ERROR_NONE){
				SendReceiveSpecialBoxItem(nRet);
				return ERROR_NONE;
			}

			m_pDBCon->QueryAddEventRewardReceiver(m_pSession, pSpecialBox->nEventRewardID, pSpecialBox->nItemID);
		}
		break;
#endif	// #if defined(PRE_SPECIALBOX)
	}

	return ERROR_NONE;
}

int CDNUserBase::OnRecvCashShopMessage(int nSubCmd, char *pData, int nLen)
{
	switch(nSubCmd)
	{
	case eCashShop::CS_BALANCEINQUIRY:
		{
			int nRet = _OnCashBalanceInquiry();
			if (nRet != ERROR_NONE)
				SendCashShopBalanceInquiry(nRet, 0, 0);
		}
		break;

	case eCashShop::CS_SHOPOPEN:
		{
			int nRet = _OnCashShopOpen();
			if (nRet != ERROR_NONE)
				SendCashShopOpen((UINT)GetCashBalance(), GetPetal(), nRet, NULL);
		}
		break;

	case eCashShop::CS_SHOPCLOSE:
		{
			IsWindowStateNoneSet(WINDOW_CASHSHOP);
			SendCashShopClose(ERROR_NONE);
		}
		break;

	case eCashShop::CS_BUY:
		{
			CSCashShopBuy *pCashShop = (CSCashShopBuy*)pData;

			int nRet = _OnCashBuy(pCashShop);
			if (nRet != ERROR_NONE)
				SendCashShopBuy(0, 0, nRet, pCashShop->cType, 0, NULL);
		}
		break;

	case eCashShop::CS_PACKAGEBUY:
		{
			CSCashShopPackageBuy *pCashShop = (CSCashShopPackageBuy*)pData;

			int nRet = _OnCashPackageBuy(pCashShop);
			if (nRet != ERROR_NONE)
				SendCashShopPackageBuy(pCashShop->nPackageSN, 0, 0, nRet);
		}
		break;

	case eCashShop::CS_CHECKRECEIVER:
		{
			CSCashShopCheckReceiver *pCashShop = (CSCashShopCheckReceiver*)pData;

			int nRet = _OnCashCheckReceiver(pCashShop);
			if (nRet != ERROR_NONE)
				SendCashShopCheckReceiver(0, 0, nRet);
		}
		break;

#if defined(PRE_ADD_CADGE_CASH)
	case eCashShop::CS_CADGE:
		{
			CSCashShopCadge *pCashShop = (CSCashShopCadge*)pData;

			int nRet = _OnCashCadge(pCashShop);
			if (nRet != ERROR_NONE)
				SendCashShopCadge(nRet);
		}
		break;
#endif	// #if defined(PRE_ADD_CADGE_CASH)

	case eCashShop::CS_GIFT:
		{
			CSCashShopGift *pCashShop = (CSCashShopGift*)pData;

			int nRet = _OnCashGift(pCashShop);
			if (nRet != ERROR_NONE)
				SendCashShopGift(0, 0, nRet, 0, 0, NULL);
		}
		break;

	case eCashShop::CS_PACKAGEGIFT:
		{
			CSCashShopPackageGift *pCashShop = (CSCashShopPackageGift*)pData;

			int nRet = _OnCashPackageGift(pCashShop);
			if (nRet != ERROR_NONE)
				SendCashShopPackageGift(pCashShop->nPackageSN, 0, 0, nRet);
		}
		break;

	case eCashShop::CS_GIFTLIST:
		{
			if( !IsWindowState(WINDOW_CASHSHOP) ){
				SendCashShopGiftList(ERROR_GENERIC_INVALIDREQUEST, 0, NULL);
				break;
			}

			m_pDBCon->QueryGetListGiftBox(m_pSession);
		}
		break;

#if defined(PRE_ADD_68286)
	case eCashShop::CS_GIFTLIST_BY_SHORTCUT:
		{
			SetWindowState(WINDOW_CASHSHOP);
			m_pDBCon->QueryGetListGiftBox(m_pSession);
		}
		break;
#endif // PRE_ADD_68286

	case eCashShop::CS_RECEIVEGIFT:
		{
			if( !IsWindowState(WINDOW_CASHSHOP) ){
				SendCashShopReceiveGift(0, ERROR_GENERIC_INVALIDREQUEST);
				break;
			}

			CSCashShopReceiveGift *pCashShop = (CSCashShopReceiveGift*)pData;
#if defined(PRE_RECEIVEGIFTALL)
			m_VecReceiveGiftResultList.clear();
			m_nReceiveGiftPageCount = 0;
#endif	// #if defined(PRE_RECEIVEGIFTALL)

			TReceiveGiftData GiftData[GIFTPAGEMAX] = {0,};
			GiftData[0] = pCashShop->GiftData;

			m_pDBCon->QueryModGiftReceive(m_pSession, false, GiftData);
		}
		break;

#if defined(PRE_RECEIVEGIFTALL)
	case eCashShop::CS_RECEIVEGIFTALL:
		{
			if( !IsWindowState(WINDOW_CASHSHOP) ){
				SendCashShopReceiveGift(0, ERROR_GENERIC_INVALIDREQUEST);
				break;
			}

			if (sizeof(CSCashShopReceiveGiftAll) != nLen)
				return ERROR_GENERIC_INVALIDREQUEST;

			CSCashShopReceiveGiftAll *pCashShop = (CSCashShopReceiveGiftAll*)pData;
			m_VecReceiveGiftResultList.clear();
			m_nReceiveGiftPageCount = 0;

			if(pCashShop->cCount < 0 || pCashShop->cCount > GIFTPAGEMAX)
				return ERROR_GENERIC_INVALIDREQUEST;

			TReceiveGiftData GiftData[GIFTPAGEMAX] = {0,};
			memcpy(GiftData, pCashShop->GiftData, sizeof(TReceiveGiftData) * pCashShop->cCount);

			for (int i = 0; i < pCashShop->cCount; i++){
				if (pCashShop->GiftData[i].nGiftDBID <= 0) continue;

				if (find(m_VecReceiveGiftResultList.begin(), m_VecReceiveGiftResultList.end(), GiftData[i].nGiftDBID) == m_VecReceiveGiftResultList.end()){
					m_VecReceiveGiftResultList.push_back(GiftData[i].nGiftDBID);
				}
				else{
					memset(&(GiftData[i]), 0, sizeof(TReceiveGiftData));
				}
			}

			m_pDBCon->QueryModGiftReceive(m_pSession, true, GiftData);
		}
		break;
#endif	// #if defined(PRE_RECEIVEGIFTALL)

#if defined(PRE_ADD_GIFT_RETURN)
	case eCashShop::CS_GIFT_RETURN:
		{
			if( !IsWindowState(WINDOW_CASHSHOP) ){
				SendCashShopGiftReturn(0, ERROR_GENERIC_INVALIDREQUEST);
				break;
			}

			CSCashShopGiftReturn *pCashShop = (CSCashShopGiftReturn*)pData;
			m_pDBCon->QueryModGiftRejectFlag(m_pSession, pCashShop->nGiftDBID );			
		}
		break;
#endif

	case eCashShop::CS_COUPON:
		{
			CSCashShopCoupon *pCashShop = (CSCashShopCoupon*)pData;

			int nRet = _OnCashCoupon(pCashShop);
			if (nRet != ERROR_NONE)
				SendCoupon(nRet);
		}
		break;

#if defined(PRE_ADD_VIP)
	case eCashShop::CS_VIPBUY:
		{
			CSVIPBuy *pCashShop = (CSVIPBuy*)pData;

			int nRet = _OnCashVIPBuy(pCashShop);
			if (nRet != ERROR_NONE)
				SendVIPBuy((UINT)GetCashBalance(), GetPetal(), ERROR_GENERIC_INVALIDREQUEST, 0, 0, false, 0);
		}
		break;

	case eCashShop::CS_VIPGIFT:
		{
			CSVIPGift *pCashShop = (CSVIPGift*)pData;

			int nRet = _OnCashVIPGift(pCashShop);
			if (nRet != ERROR_NONE)
				SendVIPGift((UINT)GetCashBalance(), GetPetal(), ERROR_GENERIC_INVALIDREQUEST, 0);
		}
		break;
#endif	// #if defined(PRE_ADD_VIP)

#if defined(_VILLAGESERVER) && defined(PRE_ADD_CASH_REFUND)
	case eCashShop::CS_MOVE_CASHINVEN:
		{
			CSMoveCashInven* pCashShop = (CSMoveCashInven*)pData;

			int nRet = _OnMoveCashInven(pCashShop);
			if (nRet != ERROR_NONE)
				SendCashShopMoveCashinven(0, nRet, 0, 0);
		}
		break;

	case eCashShop::CS_CASH_REFUND:
		{
			CSCashRefund* pCashShop = (CSCashRefund*)pData;

			int nRet = _OnCashRefund(pCashShop);
			if (nRet != ERROR_NONE)
				SendCashShopCashRefund(0, nRet);
		}
		break;		
#endif  // #if defined(PRE_ADD_CASH_REFUND)

	case eCashShop::CS_CHARGE_TIME:
		{
			__time64_t tTime;
			time(&tTime);

			SendCashShopServerTimeForCharge(tTime);
		}
		break;

#ifdef PRE_ADD_LIMITED_CASHITEM
	case eCashShop::CS_LIMITEDCASHITEM:
		{
			std::vector<LimitedCashItem::TLimitedQuantityCashItem> vList;
			g_pLimitedCashItemRepository->GetLimitedItemList(vList);
			SendQuantityLimitedItemList(vList);

			std::vector<LimitedCashItem::TChangedLimitedQuantity> vChanged;
			g_pLimitedCashItemRepository->GetChangedLimitedItemList(vChanged);
			SendChangedQuantityItemList(vChanged);
		}
		break;
#endif		//#ifdef PRE_ADD_LIMITED_CASHITEM
	}

	return ERROR_NONE;
}

int CDNUserBase::OnRecvQuestMessage(int nSubCmd, char *pData, int nLen)
{
	switch(nSubCmd)
	{
	case eQuest::CS_CANCEL_QUEST:
		{
			if (sizeof(CSCancelQuest) != nLen) {
				return ERROR_INVALIDPACKET;
			}

			CSCancelQuest* pPacket = (CSCancelQuest*)pData;
			m_pQuest->OnCancelQuest(pPacket->nQuestIndex);
		}
		return ERROR_NONE;

	case eQuest::CS_SELECT_QUEST_REWARD:
		{
			if (!IsValidPacketByNpcClick()){	// Check NPC distance
				return ERROR_GENERIC_INVALIDREQUEST;
			}

			if (sizeof(CSSelectQuestReward) != nLen) {
				return ERROR_INVALIDPACKET;
			}

			CSSelectQuestReward* pPacket = (CSSelectQuestReward*)pData;
			m_pQuest->SavePacketReward(pPacket);
		}
		return ERROR_NONE;

	case eQuest::CS_SHORTCUT_QUEST:
		{
#if defined(_VILLAGESERVER)
			if (sizeof(CSShortCutQuest) != nLen) {
				return ERROR_INVALIDPACKET;
			}

			CSShortCutQuest* pPacket = (CSShortCutQuest*)pData;
			//Added temporary verification
			if( pPacket->nQuestID < 3000 || pPacket->nQuestID > 3999) //Guild board quest range
			{
				_DANGER_POINT();
				return ERROR_NONE;
			}

#if !defined(PRE_FIX_QUEST_GUILD_BOARD)	//Step verification is performed in RewardAfterCompletingQuest
			if( GetQuest()->GetQuestStep(pPacket->nQuestID) != 2 ) //Guild board completion step 2 (temp check)
			{
				_DANGER_POINT();
				return ERROR_NONE;
			}
#endif
			
			if (m_pQuest->PreparePacketReward(pPacket->nQuestID))
				m_pQuest->RewardAfterCompletingQuest(pPacket->nQuestID, true);

			// Initialize reward-related variables
			m_pQuest->ResetRewardFlag();
#endif
		}
		return ERROR_NONE;

#if defined(PRE_ADD_REMOTE_QUEST)
	case eQuest::CS_ACCEPT_REMOTE_QEUST:
		{
			if (sizeof(CSAcceptRemoteQuest) != nLen) {
				return ERROR_INVALIDPACKET;
			}

			CSAcceptRemoteQuest* pPacket = (CSAcceptRemoteQuest*)pData;
			GetQuest()->AcceptRemoteQuest(pPacket->nQuestID);
		}
		return ERROR_NONE;
	
	case eQuest::CS_COMPLETE_REMOTE_QUEST:
		{
			if (sizeof(CSCompleteRemoteQuest) != nLen) {
				return ERROR_INVALIDPACKET;
			}

			CSCompleteRemoteQuest* pPacket = (CSCompleteRemoteQuest*)pData;
			GetQuest()->CompleteRemoteQuest(pPacket->nQuestID, pPacket);
		}
		return ERROR_NONE;
	case eQuest::CS_CANCEL_REMOTE_QUEST:
		{
			if (sizeof(CSCancelRemoteQuest) != nLen) {
				return ERROR_INVALIDPACKET;
			}

			CSCancelRemoteQuest* pPacket = (CSCancelRemoteQuest*)pData;
			GetQuest()->CancelRemoteQuest(pPacket->nQuestID);
		}
		return ERROR_NONE;
#endif
		
	}

	return ERROR_UNKNOWN_HEADER;
}

int CDNUserBase::OnRecvSystemMessage(int nSubCmd, char *pData, int nLen)
{
#pragma warning(disable:4065)
	switch(nSubCmd)
	{
		case eSystem::CS_TCP_PING:
		{
			if (sizeof(CSTCPPing) != nLen)
				return ERROR_INVALIDPACKET;

			CSTCPPing* pPacket = reinterpret_cast<CSTCPPing*>(pData);

			DWORD dwCur = timeGetTime();
			if( dwCur < pPacket->dwTick )
			{
				_DANGER_POINT();
				return ERROR_NONE;
			}

			DWORD dwGap = (dwCur-pPacket->dwTick);

			m_pairTcpPing.first += dwGap;
			++m_pairTcpPing.second;
			break;
		}
		case eSystem::CS_CLIENT_PING:
		{
			if (sizeof(CSClientPing) != nLen)
				return ERROR_INVALIDPACKET;

			CSClientPing* pPacket = reinterpret_cast<CSClientPing*>(pData);
			SendClientPing(pPacket->dwTick);
			return ERROR_NONE;
		}
#if defined(PRE_ADD_VIP)
	case eSystem::CS_VIPAUTOMATICPAY:
		{
			if (sizeof(CSVIPAutomaticPay) != nLen)
				return ERROR_INVALIDPACKET;

			CSVIPAutomaticPay *pSystem = (CSVIPAutomaticPay*)pData;
			m_bVIPAutoPay = pSystem->bAutomaticPay;
			SendVIPAutomaticPay(m_bVIPAutoPay);
		}
		break;
#endif	// #if defined(PRE_ADD_VIP)

	default:
		break;
	}
#pragma warning(default:4065)

	return ERROR_UNKNOWN_HEADER;
}

int CDNUserBase::OnRecvFriendMessage(int nSubCmd, char *pData, int nLen)
{
	switch(nSubCmd)
	{
	case eFriend::CS_FRIEND_GROUP_ADD:
		{
			CSFriendAddGroup * pPacket = (CSFriendAddGroup*)pData;

			if (sizeof(CSFriendAddGroup) - sizeof(pPacket->wszBuf) +(pPacket->cNameLen * sizeof(WCHAR)) != nLen)
				return ERROR_INVALIDPACKET;

			if (pPacket->cNameLen <= 0 || pPacket->cNameLen >= FRIEND_GROUP_NAMELENMAX)
			{
				SendFriendResult(ERROR_GENERIC_INVALIDREQUEST);
				return ERROR_NONE;
			}

			WCHAR wszName[FRIEND_GROUP_NAMELENMAX];
			_wcscpy(wszName, FRIEND_GROUP_NAMELENMAX, pPacket->wszBuf, pPacket->cNameLen);

			m_pDBCon->QueryAddGroup(m_pSession, wszName);
			return ERROR_NONE;
		}
		break;

	case eFriend::CS_FRIEND_GROUP_DELETE:
		{
			CSFriendDelGourp * pPacket = (CSFriendDelGourp*)pData;

			if (sizeof(CSFriendDelGourp) != nLen)
				return ERROR_INVALIDPACKET;

			TFriendGroup * pGroup = m_pFriend->GetGroup(pPacket->nGroupDBID);
			if (pGroup == NULL)
				return ERROR_GENERIC_UNKNOWNERROR;

			m_pDBCon->QueryDelGroup(m_pSession, pPacket->nGroupDBID);
			return ERROR_NONE;
		}
		break;

	case eFriend::CS_FRIEND_GROUP_STATEUPDATE:
		{
			CSFriendGroupUpdate * pPacket = (CSFriendGroupUpdate*)pData;

			if (sizeof(CSFriendGroupUpdate) - sizeof(pPacket->wszBuf) +(pPacket->cNameLen * sizeof(WCHAR)) != nLen)
				return ERROR_INVALIDPACKET;

			if (pPacket->cNameLen <= 0 || pPacket->cNameLen >= FRIEND_GROUP_NAMELENMAX)
			{
				_DANGER_POINT();
				SendFriendGroupUpdated(pPacket->nGroupDBID, NULL, ERROR_GENERIC_INVALIDREQUEST);
				return ERROR_NONE;
			}

			if (m_pFriend->GetGroup(pPacket->nGroupDBID) == NULL)
			{
				SendFriendGroupUpdated(pPacket->nGroupDBID, NULL, ERROR_FRIEND_GROUP_NOTFOUND);
				return ERROR_NONE;
			}

			WCHAR wszName[FRIEND_GROUP_NAMELENMAX];
			_wcscpy(wszName, FRIEND_GROUP_NAMELENMAX, pPacket->wszBuf, pPacket->cNameLen);

			m_pDBCon->QueryModGroupName(m_pSession, pPacket->nGroupDBID, wszName);
			return ERROR_NONE;
		}
		break;

	case eFriend::CS_FRIEND_ADD:
		{
			CSFriendAdd * pPacket = (CSFriendAdd*)pData;

			if (sizeof(CSFriendAdd) - sizeof(pPacket->wszBuf) +(pPacket->cNameLen * sizeof(WCHAR)) != nLen)
				return ERROR_INVALIDPACKET;

			if (m_pFriend->GetFriendCount() >= FRIEND_MAXCOUNT)
			{
				SendFriendAdded(0, 0, 0, NULL, ERROR_FRIEND_ALREADYMAX);
				return ERROR_GENERIC_INVALIDREQUEST;
			}

			if (pPacket->cNameLen <= 0 || pPacket->cNameLen >= NAMELENMAX)
			{
				SendFriendAdded(0, 0, 0, NULL, ERROR_GENERIC_INVALIDREQUEST);
				return ERROR_GENERIC_INVALIDREQUEST;
			}

			WCHAR wszName[NAMELENMAX];
			_wcscpy(wszName, NAMELENMAX, pPacket->wszBuf, pPacket->cNameLen);

			if (CheckLastSpace(wszName))
			{
				SendFriendAdded(0, 0, 0, NULL, ERROR_GENERIC_INVALIDREQUEST);
				return ERROR_GENERIC_INVALIDREQUEST;
			}

			if (!__wcsicmp_l(GetCharacterName(), wszName))
			{
				SendFriendAdded(0, 0, 0, NULL, ERROR_GENERIC_INVALIDREQUEST);
				return ERROR_NONE;
			}

			if (m_pFriend->HasFriend(wszName) == true)
			{
				SendFriendAdded(0, 0, 0, NULL, ERROR_FRIEND_DESTUSER_ALREADYFRIEND);
				return ERROR_NONE;
			}

			TFriendGroup * pGroup = NULL;
			if (pPacket->nGroupDBID > 0)
			{
				pGroup = m_pFriend->GetGroup(pPacket->nGroupDBID);
				if (pGroup == NULL)
				{
					SendFriendAdded(0, 0, 0, NULL, ERROR_FRIEND_GROUP_NOTFOUND);
					return ERROR_NONE;
				}
			}

			m_pDBCon->QueryAddFriend(m_pSession, wszName, pPacket->nGroupDBID);
			return ERROR_NONE;
		}
		break;

	case eFriend::CS_FRIEND_DELETE:
		{
			CSFriendDelete * pPacket = (CSFriendDelete*)pData;

			if (sizeof(CSFriendDelete) != nLen)
				return ERROR_INVALIDPACKET;

			if (m_pFriend->HasFriend(pPacket->biFriendCharacterDBID) == false)
			{
				SendFriendDeleted(0, NULL, ERROR_FRIEND_FRIEND_NORFOUND);
				return ERROR_NONE;
			}

			m_pDBCon->QueryDelFriend(m_pSession, 1, &pPacket->biFriendCharacterDBID);
			return ERROR_NONE;
		}
		break;

	case eFriend::CS_FRIEND_STATEUPDATE:
		{
			CSFriendUpdate * pPacket = (CSFriendUpdate*)pData;

			if (sizeof(CSFriendUpdate) != nLen)
				return ERROR_INVALIDPACKET;

			if (m_pFriend->HasFriend(pPacket->biFriendCharacterDBID) == false)
			{
				SendFriendUpdated(0, &pPacket->biFriendCharacterDBID, 0, ERROR_FRIEND_FRIEND_NORFOUND);
				return ERROR_NONE;
			}

			TFriendGroup * pGroup = NULL;
			if (pPacket->nGroupDBID > 0)
			{
				pGroup = m_pFriend->GetGroup(pPacket->nGroupDBID);
				if (pGroup == NULL)
				{
					SendFriendGroupUpdated(pPacket->nGroupDBID, NULL, ERROR_FRIEND_GROUP_NOTFOUND);
					return ERROR_NONE;
				}
			}

			m_pDBCon->QueryModFriendAndGroupMapping(m_pSession, pPacket->nGroupDBID, 1, &pPacket->biFriendCharacterDBID);
			return ERROR_NONE;
		}
		break;

	case eFriend::CS_FRIEND_INFO:
		{
			CSFriendDetailInfo * pPacket = (CSFriendDetailInfo*)pData;

			if (sizeof(CSFriendDetailInfo) != nLen)
				return ERROR_INVALIDPACKET;

			TFriend * pFriend = m_pFriend->GetFriend(pPacket->biFriendCharacterDBID);
			if (pFriend == NULL)
			{
				SendFriendResult(ERROR_FRIEND_FRIEND_NORFOUND);
				return ERROR_NONE;
			}

			if (m_pDBCon && m_pDBCon->GetActive()) m_pDBCon->QueryGetCharacterPartialyByName(m_pSession, pFriend->wszCharacterName, _REQCHARACTERPARTIALY_FRIEND);
			return ERROR_NONE;
		}
		break;

	case eFriend::CS_FRIEND_REQ_LIST:
		{
			if (nLen != 0)
				return ERROR_INVALIDPACKET;

			m_pFriend->SendFriendList();
			return ERROR_NONE;
		}
		break;

	case eFriend::CS_FRIEND_REQ_LOCATION:
		{
			if (nLen != 0)
				return ERROR_INVALIDPACKET;

			m_pFriend->SendFriendLocation();
			return ERROR_NONE;
		}
		break;
	}
	return ERROR_UNKNOWN_HEADER;
}

int CDNUserBase::OnRecvIsolateMessage(int nSubCmd, char *pData, int nLen)
{
	switch(nSubCmd)
	{
	case eIsolate::CS_ISOLATE_GET:
		{
			if (nLen != 0)
				return ERROR_INVALIDPACKET;

			SCIsolateList packet;
			memset(&packet, 0, sizeof(packet));

			int nSize = 0;
			m_pIsolate->GetIsolateList(&packet, nSize);

			if (packet.cCount > 0)
				SendIsolateList(&packet, nSize);
			return ERROR_NONE;
		}
		break;

	case eIsolate::CS_ISOLATE_ADD:
		{
			CSIsolateAdd * pPacket = (CSIsolateAdd*)pData;

			if (sizeof(CSIsolateAdd) != nLen)
				return ERROR_INVALIDPACKET;

			if (CheckLastSpace(pPacket->wszIsolateName))
			{
				SendIsolateResult(ERROR_ISOLATE_ADDFAIL);
				return ERROR_NONE;
			}

			if (!__wcsicmp_l(pPacket->wszIsolateName, GetCharacterName()))
			{
				SendIsolateResult(ERROR_ISOLATE_ADDFAIL);
				return ERROR_NONE;
			}

			if ( m_pIsolate && m_pIsolate->IsIsolateItem(pPacket->wszIsolateName) == true)
			{
				SendIsolateResult(ERROR_ISOLATE_ALREADYADDED);
				return ERROR_NONE;
			}

			m_pDBCon->QueryAddIsolate(m_pSession, pPacket->wszIsolateName);
			return ERROR_NONE;
		}
		break;

	case eIsolate::CS_ISOLATE_DELETE:
		{
			CSIsoLateDelete * pPacket = (CSIsoLateDelete*)pData;

			if (sizeof(CSIsoLateDelete) != nLen)
				return ERROR_INVALIDPACKET;

			INT64 biIsolateCharDBID = m_pIsolate->GetIsolateCharDBID(pPacket->wszIsolateName);
			if (biIsolateCharDBID <= 0)
			{
				//Need to define error code
				SendIsolateResult(ERROR_GENERIC_INVALIDREQUEST);
				return ERROR_NONE;
			}

			m_pDBCon->QueryDelIsolate( m_pSession, biIsolateCharDBID );
			return ERROR_NONE;
		}
		break; 
	}
	return ERROR_UNKNOWN_HEADER;
}

int CDNUserBase::OnRecvGameOptionMessage(int nSubCmd, char *pData, int nLen)
{
	switch(nSubCmd)
	{
	case eGameOption::CS_GAMEOPTION_UPDATEOPTION:
		{
			CSGameOptionUpdate * pPacket = (CSGameOptionUpdate*)pData;

			if (sizeof(CSGameOptionUpdate) != nLen)
				return ERROR_INVALIDPACKET;

			m_bNeedUpdateOption = true;
			memcpy_s(&m_GameOption, sizeof(TGameOptions), &pPacket->Option, sizeof(TGameOptions));
			return ERROR_NONE;
		}

	case eGameOption::CS_GAMEOPTION_REQCOMMOPTION:
		{
			CSGameOptionReqComm * pPacket = (CSGameOptionReqComm*)pData;

			if (sizeof(CSGameOptionReqComm) != nLen)
				return ERROR_INVALIDPACKET;

#if defined(_VILLAGESERVER)
			CDNUserSession *pSession = g_pUserSessionManager->FindUserSession(pPacket->nSessionID);
#elif defined(_GAMESERVER)
			CDNUserSession * pSession = m_pSession->FindUserSession(pPacket->nSessionID);
#endif
			if (pSession)
			{
				const TGameOptions * pOption = pSession->GetGameOption();
				if (pOption)
				{
#if defined(_VILLAGESERVER)
					SendGameOption(pPacket->nSessionID, pOption, pSession->GetPartyID());
#elif defined(_GAMESERVER)
					if (pSession->GetGameRoom())
						SendGameOption(pPacket->nSessionID, pOption, pSession->GetGameRoom()->GetPartyIndex());
					else
						_DANGER_POINT();
#endif
				}
				else
					_DANGER_POINT();
			}
			else
				_DANGER_POINT();
			return ERROR_NONE;
		}

		case eGameOption::CS_GAMEOPTION_QUEST_NOTIFIER:
		{
			if (sizeof(CSGameOptionQuestNotifier) != nLen)
				return ERROR_INVALIDPACKET;

			int iRet = _CmdSetQuestNotifier( reinterpret_cast<CSGameOptionQuestNotifier*>(pData) );
			if ( iRet != ERROR_NONE )
			{
				SendGameOptionRefreshNotifier();
			}
			return iRet;
		}

		case eGameOption::CS_GAMEOPTION_MISSION_NOTIFIER:
		{
			if (sizeof(CSGameOptionMissionNotifier) != nLen)
				return ERROR_INVALIDPACKET;

			int iRet = _CmdSetMissionNotifier( reinterpret_cast<CSGameOptionMissionNotifier*>(pData) );
			if ( iRet != ERROR_NONE )
			{
				SendGameOptionRefreshNotifier();
			}
			return iRet;
		}
		case eGameOption::CS_GAMEOPTION_UPDATE_KEYSETTING:
		{
			CSGameOptionUpdateKeySetting* pPacket = reinterpret_cast<CSGameOptionUpdateKeySetting*>(pData);

			if (m_pDBCon && m_pDBCon->GetActive()) 
				m_pDBCon->QueryUpdateKeySetting( m_cDBThreadID, GetWorldSetID(), GetAccountDBID(), pPacket );
			
			return ERROR_NONE;
		}
		case eGameOption::CS_GAMEOPTION_UPDATE_PADSETTING:
		{
			CSGameOptionUpdatePadSetting* pPacket = reinterpret_cast<CSGameOptionUpdatePadSetting*>(pData);

			if (m_pDBCon && m_pDBCon->GetActive()) 
				m_pDBCon->QueryUpdatePadSetting( m_cDBThreadID, GetWorldSetID(), GetAccountDBID(), pPacket );

			return ERROR_NONE;
		}

		case eGameOption::CS_GAMEOPTION_VALIDATE_SECONDAUTH:
			{
				CSGameOptionValidateSecondAuth* pPacket = reinterpret_cast<CSGameOptionValidateSecondAuth*>(pData);

				if (bIsSetSecondAuthPW() == false)
				{
					SendSecondAuthValidate(ERROR_SECONDAUTH_NOTEXIST, pPacket->nAuthCheckType, 0);
					return ERROR_NONE;
				}

				if (m_pDBCon && m_pDBCon->GetActive())
					m_pDBCon->QueryValidateSecondAuth( m_cDBThreadID, GetAccountDBID(), MakeSecondPasswordString( szVersion, pPacket->nSeed, pPacket->nValue ).c_str(), pPacket->nAuthCheckType);

				return ERROR_NONE;
			}

		case eGameOption::CS_GAMEOPTION_GET_PROFILE:
			{
				if( 0 != nLen )	return ERROR_INVALIDPACKET;

				SendProfile( m_Profile );

				return ERROR_NONE;
			}
			break;
		case eGameOption::CS_GAMEOPTION_SET_PROFILE:
			{
				CSGameOptionSetProfile * pPacket = reinterpret_cast<CSGameOptionSetProfile*>(pData);
				if( sizeof(CSGameOptionSetProfile) != nLen )	return ERROR_INVALIDPACKET;

				if( m_pDBCon && m_pDBCon->GetActive() )
					m_pDBCon->QuerySetProfile( m_pSession, pPacket->sProfile.cGender, pPacket->sProfile.wszGreeting, pPacket->sProfile.bOpenPublic );

				return ERROR_NONE;
			}
			break;
	}
	return ERROR_UNKNOWN_HEADER;
}

int CDNUserBase::OnRecvRadioMessage(int nSubCmd, char *pData, int nLen)
{	
	return ERROR_NONE;
}

int CDNUserBase::OnRecvGuildMessage(int nSubCmd, char *pData, int nLen)
{
	return ERROR_NONE;
}

int CDNUserBase::OnRecvEtcMessage(int nSubCmd, char *pData, int nLen)
{
	switch( nSubCmd )
	{
		case eEtc::CS_DARKLAIR_RANK_BOARD:
		{
			if( CheckDBConnection() == false )	
				return ERROR_GENERIC_DBCON_NOT_FOUND;

			if( sizeof(CSDarkLairRankBoard) != nLen )
				return ERROR_INVALIDPACKET;

			CSDarkLairRankBoard* pPacket = reinterpret_cast<CSDarkLairRankBoard*>(pData);
#if defined(PRE_ADD_CHALLENGE_DARKLAIR)
			if(pPacket->iMapIndex > DarkLair::TopFloorAbstractMapIndex)
			{
				int iMapIndex = pPacket->iMapIndex%DarkLair::TopFloorAbstractMapIndex;
				if( g_pDataManager->IsCloseGateByTime(iMapIndex) )
				{	//Usage time limit. Please respect it.
					SCDarkLairRankBoard TxPacket;
					memset( &TxPacket, 0, sizeof(TxPacket) );
					TxPacket.iRet = ERROR_NOT_AVAILABLE_CHECK_RANKING_TIME;					
					int iSize = sizeof(TxPacket.iRet);
					m_pSession->AddSendData( SC_ETC, eEtc::SC_DARKLAIR_RANK_BOARD, reinterpret_cast<char*>(&TxPacket), iSize );
					return ERROR_NONE;
				}
			}
#endif
			m_pDBCon->QueryGetDarkLairRankBoard( m_pSession, pPacket->iMapIndex, pPacket->cPartyUserCount );
			return ERROR_NONE;
		}
		case eEtc::CS_PVPLADDER_RANK_BOARD:
		{
			if( CheckDBConnection() == false )	
				return ERROR_GENERIC_DBCON_NOT_FOUND;

			if( sizeof(CSPvPLadderRankBoard) != nLen )
				return ERROR_INVALIDPACKET;

			CSPvPLadderRankBoard* pPacket = reinterpret_cast<CSPvPLadderRankBoard*>(pData);
			m_pDBCon->QueryGetPvPLadderRankBoard( m_pSession, pPacket->MatchType );
			return ERROR_NONE;
		}
		case eEtc::CS_WINDOW_STATE :
			{
				if( sizeof(CSWindowState) != nLen )
					return ERROR_INVALIDPACKET;

				CSWindowState* pPacket = reinterpret_cast<CSWindowState*>(pData);
				if( pPacket->sWinState == WINDOW_NONE )
				{
					SetNoneWindowState();
					// Originally CS_CLOSE_UIWINDOW handled here, but with WINDOW_BLIND added, handle here..
					if( GetRemoteEnchantItemID() )
						SetRemoteEnchantItemID(0);
					if( GetRemoteItemCompoundItemID())
						SetRemoteItemCompoundItemID(0);
				}
				else
					SetWindowState((eWindowState)pPacket->sWinState);
				return ERROR_NONE;
			}
			break;
#if defined(PRE_ADD_PVP_RANKING)
		case eEtc::CS_PVP_RANK_BOARD:
			{
#if defined( _VILLAGESERVER )
				if( CheckDBConnection() == false )	
					return ERROR_GENERIC_DBCON_NOT_FOUND;

				m_pDBCon->QueryGetPvPRankBoard( m_pSession );
#endif
				return ERROR_NONE;
			}
			break;
		case eEtc::CS_PVP_RANK_LIST:
			{
#if defined( _VILLAGESERVER )
				if( CheckDBConnection() == false )	
					return ERROR_GENERIC_DBCON_NOT_FOUND;

				if( sizeof(CSPvPRankList) != nLen )
					return ERROR_INVALIDPACKET;

				CSPvPRankList* pPacket = reinterpret_cast<CSPvPRankList*>(pData);

				m_pDBCon->QueryGetPvPRankList( m_pSession, pPacket->iPage, pPacket->cClassCode, pPacket->cSubClassCode, pPacket->wszGuildName );							
#endif
				return ERROR_NONE;
			}
			break;
		case eEtc::CS_PVP_RANK_INFO:
			{
#if defined( _VILLAGESERVER )
				if( CheckDBConnection() == false )	
					return ERROR_GENERIC_DBCON_NOT_FOUND;

				if( sizeof(CSPvPRankInfo) != nLen )
					return ERROR_INVALIDPACKET;

				CSPvPRankInfo* pPacket = reinterpret_cast<CSPvPRankInfo*>(pData);

				m_pDBCon->QueryGetPvPRankInfo( m_pSession, pPacket->wszCharName );
#endif
				return ERROR_NONE;
			}
			break;

		case eEtc::CS_PVP_LADDER_RANK_BOARD:
			{
#if defined( _VILLAGESERVER )
				if( CheckDBConnection() == false )	
					return ERROR_GENERIC_DBCON_NOT_FOUND;

				if( sizeof(CSPvPLadderRankBoard2) != nLen )
					return ERROR_INVALIDPACKET;

				CSPvPLadderRankBoard2* pPacket = reinterpret_cast<CSPvPLadderRankBoard2*>(pData);

				m_pDBCon->QueryGetPvPLadderRankBoard2( m_pSession, pPacket->MatchType );
#endif
				return ERROR_NONE;
			}
			break;
		case eEtc::CS_PVP_LADDER_RANK_LIST:
			{
#if defined( _VILLAGESERVER )
				if( CheckDBConnection() == false )	
					return ERROR_GENERIC_DBCON_NOT_FOUND;

				if( sizeof(CSPvPLadderRankList) != nLen )
					return ERROR_INVALIDPACKET;

				CSPvPLadderRankList* pPacket = reinterpret_cast<CSPvPLadderRankList*>(pData);

				m_pDBCon->QueryGetPvPLadderRankList( m_pSession, pPacket->MatchType, pPacket->iPage, pPacket->cClassCode, pPacket->cSubClassCode, pPacket->wszGuildName );
#endif
				return ERROR_NONE;
			}
			break;
		case eEtc::CS_PVP_LADDER_RANK_INFO:
			{
#if defined( _VILLAGESERVER )
				if( CheckDBConnection() == false )	
					return ERROR_GENERIC_DBCON_NOT_FOUND;

				if( sizeof(CSPvPLadderRankInfo) != nLen )
					return ERROR_INVALIDPACKET;

				CSPvPLadderRankInfo* pPacket = reinterpret_cast<CSPvPLadderRankInfo*>(pData);

				m_pDBCon->QueryGetPvPLadderRankInfo( m_pSession, pPacket->MatchType, pPacket->wszCharName );
#endif
				return ERROR_NONE;
			}
			break;
#endif
#if defined(PRE_ADD_GAMEQUIT_REWARD)
		case eEtc::CS_GAMEQUIT_REWARDCHECK_REQ:
			{
				SendGameQuitRewardType(m_eUserGameQuitRewardType);
				return ERROR_NONE;
			}
			break;
		case eEtc::CS_GAMEQUIT_REWARD_REQ:
			{
				if(sizeof(CSGameQuitRewardReq) != nLen)
					return ERROR_INVALIDPACKET;

				CSGameQuitRewardReq *pPacket = (CSGameQuitRewardReq*)pData;
				int nRet = ERROR_NONE;
				bool IsSuccess = false;
				
				if(pPacket->eRewardType == m_eUserGameQuitRewardType && pPacket->eRewardType != GameQuitReward::RewardType::None)
				{
					switch(pPacket->eRewardType)
					{
					case GameQuitReward::RewardType::ComeBackReward:
						IsSuccess = SetGameQuitRewardItem(TLevelupEvent::ComeBackUserQuitReward, DBDNWorldDef::AddMaterializedItem::ComebackGameQuitReward);
						break;
					case GameQuitReward::RewardType::NewbieReward:
						IsSuccess = SetGameQuitRewardItem(TLevelupEvent::NewbieQuitReward, DBDNWorldDef::AddMaterializedItem::NewbieGameQuitReward);
						break;
					}
				}
				
				if(!IsSuccess) 
				{
					nRet = ERROR_ITEM_NOTFOUND;
					//Log when reward was not received
					g_Log.Log(LogType::_GAMEQUITREWARD, m_pSession, L"[SetGameQuitRewardItem][Type:%d(Recv:%d)] LevelupEventTable Data Error(Lev[%d] Class[%d] Job[%d]\n", m_eUserGameQuitRewardType, pPacket->eRewardType, GetLevel(), GetClassID(), GetUserJob());
				}
				m_eUserGameQuitRewardType = GameQuitReward::RewardType::None;

				SendGameQuitRewardResult(nRet);
				return ERROR_NONE;
			}
			break;	
#endif	// #if defined(PRE_ADD_GAMEQUIT_REWARD)
	}

	return ERROR_UNKNOWN_HEADER;
}

int CDNUserBase::OnRecvChatRoomMessage(int nSubCmd, char *pData, int nLen)
{
	return ERROR_NONE;
}

int CDNUserBase::OnRecvMasterSystemMessage( int nSubCmd, char* pData, int nLen )
{
	switch( nSubCmd )
	{
		case eMasterSystem::CS_MASTERCHARACTERINFO:
		{
			if( nLen != 0 )
				return ERROR_INVALIDPACKET;

			// Check
			if( GetLevel() < static_cast<int>(CGlobalWeightTable::GetInstance().GetValue( CGlobalWeightTable::MasterSystem_MasterMinLevel )) )
			{
				SendMasterSystemMasterCharacterInfo( ERROR_GENERIC_INVALIDREQUEST );
				return ERROR_NONE;
			}

			MasterSystem::CCacheRepository::GetInstance().GetMasterCharacter( static_cast<CDNUserSession*>(this), GetCharacterDBID() );
			return ERROR_NONE;
		}
		case eMasterSystem::CS_MASTERANDCLASSMATE:
		{
			if( nLen != 0 )
				return ERROR_INVALIDPACKET;

				// Check
			if( GetLevel() > static_cast<int>(CGlobalWeightTable::GetInstance().GetValue( CGlobalWeightTable::MasterSystem_PupilMaxLevel )) )
			{
				SendMasterSystemMasterAndClassmateInfo( ERROR_GENERIC_INVALIDREQUEST, std::vector<TMasterAndClassmateInfo>() );
				return ERROR_NONE;
			}

			MasterSystem::CCacheRepository::GetInstance().GetMasterClassmateList( static_cast<CDNUserSession*>(this), GetCharacterDBID(), true );
			return ERROR_NONE;
		}
	}

	return ERROR_UNKNOWN_HEADER;
}

#if defined( PRE_ADD_SECONDARY_SKILL )

int CDNUserBase::OnRecvSecondarySkillMessage(int nSubCmd, char *pData, int nLen)
{
	_ASSERT( dynamic_cast<CSecondarySkillRepositoryServer*>(m_pSecondarySkillRepository) );

	switch( nSubCmd )
	{
		case eSecondarySkill::CS_DELETE:
		{
			if( sizeof(SecondarySkill::CSDelete) != nLen )
				return ERROR_INVALIDPACKET;

			SecondarySkill::CSDelete* pPacket = reinterpret_cast<SecondarySkill::CSDelete*>(pData);
			if( m_pSecondarySkillRepository->Delete( pPacket->iSkillID ) == false )
			{
				// Send error to client
				SendDelSecondarySkill( ERROR_SECONDARYSKILL_DELETE_FAILED, pPacket->iSkillID );
			}
			return ERROR_NONE;
		}
		case eSecondarySkill::CS_ADD_RECIPE:
		{
			if( sizeof(SecondarySkill::CSAddRecipe) != nLen )
				return ERROR_INVALIDPACKET;

			SecondarySkill::CSAddRecipe* pPacket = reinterpret_cast<SecondarySkill::CSAddRecipe*>(pData);
			int iRet = static_cast<CSecondarySkillRepositoryServer*>(m_pSecondarySkillRepository)->AddRecipe( pPacket );
			if( iRet != ERROR_NONE )
			{
				// Send error to client
				SendAddSecondarySkillRecipe( iRet, NULL );
			}
			return ERROR_NONE;
		}
		case eSecondarySkill::CS_DELETE_RECIPE:
		{
			if( sizeof(SecondarySkill::CSDeleteRecipe) != nLen )
				return ERROR_INVALIDPACKET;

			SecondarySkill::CSDeleteRecipe* pPacket = reinterpret_cast<SecondarySkill::CSDeleteRecipe*>(pData);
			if( static_cast<CSecondarySkillRepositoryServer*>(m_pSecondarySkillRepository)->DeleteRecipe( pPacket ) == false )
			{
				// Send error to client
				SendDeleteSecondarySkillRecipe( ERROR_SECONDARYSKILL_DELETERECIPE_FAILED, pPacket->iSkillID, pPacket->iItemID );
			}
			return ERROR_NONE;
		}
		case eSecondarySkill::CS_EXTRACT_RECIPE:
		{
			if( sizeof(SecondarySkill::CSExtractRecipe) != nLen )
				return ERROR_INVALIDPACKET;

			SecondarySkill::CSExtractRecipe* pPacket = reinterpret_cast<SecondarySkill::CSExtractRecipe*>(pData);
			if( static_cast<CSecondarySkillRepositoryServer*>(m_pSecondarySkillRepository)->ExtractRecipe( pPacket ) == false )
			{
				// Send error to client
				SendExtractSecondarySkillRecipe( ERROR_SECONDARYSKILL_EXTRACTRECIPE_FAILED, pPacket->iSkillID, pPacket->iItemID );
			}
			
			return ERROR_NONE;
		}
		case eSecondarySkill::CS_MANUFACTURE:
		{
			if( sizeof(SecondarySkill::CSManufacture) != nLen )
				return ERROR_INVALIDPACKET;

			SecondarySkill::CSManufacture* pPacket = reinterpret_cast<SecondarySkill::CSManufacture*>(pData);
			int iRet = static_cast<CSecondarySkillRepositoryServer*>(m_pSecondarySkillRepository)->DoManufacture( pPacket );
			if( iRet != ERROR_NONE )
			{
				if( pPacket->bIsStart == false )
				{
					CManufactureSkill* pManufactureSkill = m_pSecondarySkillRepository->GetManufactureSkill( pPacket->iSkillID );
					if( pManufactureSkill )
					{
						// Initialize Tick
						pManufactureSkill->ClearManufactureTick();
						// Set Flag
						static_cast<CSecondarySkillRepositoryServer*>(m_pSession->GetSecondarySkillRepository())->SetManufacturingFlag( false );
					}
				}

				// Send error to client
				SendManufactureSecondarySkill( iRet, pPacket->bIsStart, pPacket->iSkillID, pPacket->iItemID );
			}

			return ERROR_NONE;
		}
		case eSecondarySkill::CS_CANCEL_MANUFACTURE:
		{
			if( nLen != 0 )
				return ERROR_INVALIDPACKET;

			static_cast<CSecondarySkillRepositoryServer*>(m_pSecondarySkillRepository)->CancelManufacture( false );
			return ERROR_NONE;
		}
	}

	return ERROR_UNKNOWN_HEADER;
}

#endif // #if defined( PRE_ADD_SECONDARY_SKILL )

#if defined( PRE_ADD_NPC_REPUTATION_SYSTEM )

int CDNUserBase::OnRecvReputationMessage( int nSubCmd, char* pData, int nLen )
{
	switch( nSubCmd )
	{
		case eReputation::CS_GIVENPCPRESENT:
		{
			if( sizeof(CSGiveNpcPresent) != nLen )
				return ERROR_INVALIDPACKET;

			CSGiveNpcPresent* pPacket = (CSGiveNpcPresent*)pData;
			CNpcReputationProcessor::PresentProcess( static_cast<CDNUserSession*>(this), pPacket->nNpcID, pPacket->nPresentID, pPacket->nPresentCount );
		}

		return ERROR_NONE;
	}

	return ERROR_UNKNOWN_HEADER;
}

#endif // #if defined( PRE_ADD_NPC_REPUTATION_SYSTEM )

int CDNUserBase::OnRecvGestureMessage(int nSubCmd, char *pData, int nLen)
{
	switch(nSubCmd)
	{
	case eGesture::CS_USEGESTURE:
		{
			if (sizeof(CSUseGesture) != nLen)
				return ERROR_INVALIDPACKET;

			CSUseGesture *pUseGesture = (CSUseGesture*)pData;
			int nGestureID = pUseGesture->nGestureID;

			//ValidCheck
			if (HasGesture(nGestureID)) {
				if (CheckPapering(eGesturePapering))
					break;
				SendUseGesture(nGestureID);
				GetEventSystem()->OnEvent( EventSystem::OnSocialAction, 1,
					EventSystem::SocialActionID, nGestureID );
			}
			else
				_DANGER_POINT();
			return ERROR_NONE;
		}
		break;
	}

	return ERROR_UNKNOWN_HEADER;
}

int CDNUserBase::OnRecvPlayerCustomEventUI(int nSubCmd, char * pData, int nLen)
{
	switch (nSubCmd)
	{
	case ePlayerCustomEventUI::CS_GETPLAYERUI:
		{
			CSGetPlayerCustomEventUI * pPacket = (CSGetPlayerCustomEventUI*)pData;

			CDNUserSession * pSession = NULL;
#ifdef _VILLAGESERVER
			pSession = g_pUserSessionManager->FindUserSession(pPacket->nSessionID);
#elif _GAMESERVER
			if (m_pSession)
			{
				CDNGameRoom * pGameRoom = m_pSession->GetGameRoom();
				if (pGameRoom)
					pSession = pGameRoom->GetUserSession(pPacket->nSessionID);
			}
#endif		//#ifdef _VILLAGESERVER

			if (pSession == NULL)
			{
				_DANGER_POINT_MSG(L"CS_GETPLAYERUI pSession NULL");
				return ERROR_NONE;
			}

			std::vector <int> vList;
			g_pDataManager->GetPlayerCustomEventUIList(vList);

			int nSize = (int)vList.size();
			if (nSize == 0 || nSize >= PlayerCustomEventUI::PlayerCustomEventUIMax)
			{
				_DANGER_POINT_MSG(L"CS_GETPLAYERUI Size MisMatch");
				return ERROR_NONE;
			}

			TMissionGroup *pMission = pSession->GetMissionData();
			if (pMission == NULL)
			{
				_DANGER_POINT_MSG(L"CS_GETPLAYERUI pMission NULL");
				return ERROR_NONE;
			}

			SCGetPlayerCustomEventUI packet;
			memset(&packet, 0, sizeof(SCGetPlayerCustomEventUI));

			packet.cCount = nSize;
			int i = 0;
			for (std::vector <int>::iterator ii = vList.begin(); ii != vList.end(); ii++)
			{
				packet.UIs[i].nMissionID = (*ii);
				packet.UIs[i].cFlag = GetBitFlag(pMission->MissionAchieve, (*ii) - 1) == true ? 1 : 0;
				i++;
			}

			SendPlayerCustomEventUI(&packet);
			return ERROR_NONE;
		}
		break;
	}
	return ERROR_UNKNOWN_HEADER;
}

int CDNUserBase::OnRecvGuildRecruitMessage( int nSubCmd, char* pData, int nLen )
{
	switch( nSubCmd )
	{
	case eGuildRecruit::CS_GUILDRECRUIT_LIST:
		{
			if( sizeof(GuildRecruitSystem::CSGuildRecruitList) != nLen )
				return ERROR_INVALIDPACKET;

			// Check
			if( GetGuildUID().IsSet() )
			{			
				SendGuildRecruitList( ERROR_GENERIC_INVALIDREQUEST, 0, std::vector<TGuildRecruitInfo>() );
				return ERROR_NONE;
			}

			GuildRecruitSystem::CSGuildRecruitList* pPacket = reinterpret_cast<GuildRecruitSystem::CSGuildRecruitList*>(pData);
#if defined( PRE_ADD_GUILD_EASYSYSTEM )		
			GuildRecruitSystem::CCacheRepository::GetInstance().GetGuildRecruitList( static_cast<CDNUserSession*>(this), pPacket->uiPage, GetClassID(), GetLevel(), pPacket->cPurposeCode, pPacket->wszGuildName, pPacket->cSortType );
#else
			GuildRecruitSystem::CCacheRepository::GetInstance().GetGuildRecruitList( static_cast<CDNUserSession*>(this), pPacket->uiPage, GetClassID(), GetLevel() );
#endif
			
			return ERROR_NONE;
		}
		break;		
	case eGuildRecruit::CS_GUILDRECRUIT_MYLIST:
		{
			if( nLen != 0 )
				return ERROR_INVALIDPACKET;

			// Check
			if( GetGuildUID().IsSet() )
			{			
				SendMyGuildRecruit( ERROR_GENERIC_INVALIDREQUEST, std::vector<TGuildRecruitInfo>() );
				return ERROR_NONE;
			}
			
			GuildRecruitSystem::CCacheRepository::GetInstance().GetMyGuildRecruit( static_cast<CDNUserSession*>(this) );
			return ERROR_NONE;
		}
		break;
	case eGuildRecruit::CS_GUILDRECRUIT_REQUESTCOUNT:
		{
			if( nLen != 0 )
				return ERROR_INVALIDPACKET;

			// Check
			if( GetGuildUID().IsSet() )
			{			
				SendGuildRecruitRequestCount( ERROR_GENERIC_INVALIDREQUEST, 0, GuildRecruitSystem::Max::MaxRequestCount );
				return ERROR_NONE;
			}
			GetDBConnection()->QueryGetGuildRecruitRequestCount( m_pSession );
			return ERROR_NONE;
		}
		break;
	case eGuildRecruit::CS_GUILDRECRUIT_CHARACTER:
		{
			if( nLen != 0 )
				return ERROR_INVALIDPACKET;

			// Check
			if( !GetGuildUID().IsSet() )
			{			
				SendGuildRecruitCharacter( ERROR_GENERIC_INVALIDREQUEST, std::vector<TGuildRecruitCharacter>() );
				return ERROR_NONE;
			}

			GuildRecruitSystem::CCacheRepository::GetInstance().GetGuildRecruitCharacter( static_cast<CDNUserSession*>(this), GetGuildUID() );
			return ERROR_NONE;
		}
		break;
	case eGuildRecruit::CS_GUILDRECRUIT_REGISTERINFO:
		{			
			if( nLen != 0 )
				return ERROR_INVALIDPACKET;

			// Check
			if( !GetGuildUID().IsSet() )
			{			
				SendGuildRecruitCharacter( ERROR_GENERIC_INVALIDREQUEST, std::vector<TGuildRecruitCharacter>() );
				return ERROR_NONE;
			}

			GetDBConnection()->QueryRegisterInfoGuildRecruit( m_pSession );
			return ERROR_NONE;
		}
		break;
	}
	return ERROR_UNKNOWN_HEADER;
}

#if defined (PRE_ADD_BESTFRIEND)
int CDNUserBase::OnRecvBestFriendMessage(int nSubCmd, char *pData, int nLen)
{
	switch( nSubCmd )
	{	
	case eBestFriend::CS_GETINFO:
		{
			if (false == m_pBestFriend->IsRegistered())
			{
				TBestFriendInfo Info = {0,};
				SendGetBestFriend(ERROR_BESTFRIEND_NOT_REGISTERED, Info);
				return ERROR_BESTFRIEND_NOT_REGISTERED;
			}

			m_pBestFriend->CheckAndSendData();			
			return ERROR_NONE;
		}
		break;
	}
	return ERROR_UNKNOWN_HEADER;
}
#endif

#if defined( PRE_PRIVATECHAT_CHANNEL )
int CDNUserBase::OnRecvPrivateChatChannelMessage(int nSubCmd, char *pData, int nLen)
{	
	switch( nSubCmd )
	{	
	case ePrivateChatChannel::CS_PRIVATECHAT_CHANNEL_ADD:
		{
			PrivateChatChannel::CSPrivateChatChannleAdd *pPacket = reinterpret_cast<PrivateChatChannel::CSPrivateChatChannleAdd*>(pData);
			if (sizeof(PrivateChatChannel::CSPrivateChatChannleAdd) != nLen)
				return ERROR_INVALIDPACKET;

			int nRet = CheckInvalidChannelName(pPacket->wszChannelName);
			if( nRet != ERROR_NONE )
			{				
				return nRet;
			}
			if(g_pPrivateChatChannelManager->GetPrivateChatChannelId(GetWorldSetID(), pPacket->wszChannelName) > 0)
			{
				SendPrivateChatChannelResult(ERROR_ALREADY_CHANNELNAME);
				return ERROR_ALREADY_CHANNELNAME;
			}

			SetPrivateChannelID(PrivateChatChannel::Common::WaitDBResult);
			// DB processing
#if defined( PRE_ADD_PRIVATECHAT_CHANNEL)
			m_pDBCon->QueryAddPrivateChatChannel( m_pSession, pPacket->wszChannelName, pPacket->nPassWord );
#else
			m_pDBCon->QueryAddPrivateChatChannel( m_pSession, pPacket->wszChannelName, -1 );
#endif
			return ERROR_NONE;
		}
		break;	
	case ePrivateChatChannel::CS_PRIVATECHAT_CHANNEL_JOIN:
		{
			PrivateChatChannel::CSPrivateChatChannleJoin *pPacket = reinterpret_cast<PrivateChatChannel::CSPrivateChatChannleJoin*>(pData);
			if (sizeof(PrivateChatChannel::CSPrivateChatChannleJoin) != nLen)
				return ERROR_INVALIDPACKET;
			int nRet = CheckInvalidChannelName(pPacket->wszChannelName);
			if( nRet != ERROR_NONE )
			{				
				return nRet;
			}
			INT64 nChannelID = 0;
			if(g_pPrivateChatChannelManager)
			{
				nChannelID = g_pPrivateChatChannelManager->GetPrivateChatChannelId(GetWorldSetID(), pPacket->wszChannelName);				
			}
			else
				break;

			CDNPrivateChaChannel* pPrivateChatChannel = g_pPrivateChatChannelManager->GetPrivateChannelInfo(GetWorldSetID(), nChannelID );
			if( pPrivateChatChannel )
			{
				if( pPacket->nPassWord == -1 )
				{
					if(pPrivateChatChannel->GetChannelPassword() >= 0)
					{
						SendPrivateChatChannelJoinResult( PrivateChatChannel::Common::NeedPassWord, pPrivateChatChannel->GetChannelName());
						return ERROR_NONE;
					}
				}
				else if(pPacket->nPassWord !=pPrivateChatChannel->GetChannelPassword())
				{
					SendPrivateChatChannelResult( ERROR_INVALID_PRIVATECHATCHANNELPASSWORD );
					return ERROR_INVALID_PRIVATECHATCHANNELPASSWORD;
				}
				SetPrivateChannelID(pPrivateChatChannel->GetChannelID());
			}
			else
			{
				SendPrivateChatChannelResult(ERROR_NOTEXIST_CHANNELNAME);
				return ERROR_NOTEXIST_CHANNELNAME;
			}

			// DB processing			
			m_pDBCon->QueryAddPrivateChatChannelMember( m_pSession, nChannelID, PrivateChatChannel::Common::JoinMember );
			return ERROR_NONE;
		}
		break;	
	case ePrivateChatChannel::CS_PRIVATECHAT_CHANNEL_INVITE:
		{
			PrivateChatChannel::CSPrivateChatChannleInvite *pPacket = reinterpret_cast<PrivateChatChannel::CSPrivateChatChannleInvite*>(pData);
			if (sizeof(PrivateChatChannel::CSPrivateChatChannleInvite) != nLen)
				return ERROR_INVALIDPACKET;			

			CDNPrivateChaChannel* pPrivateChatChannel = g_pPrivateChatChannelManager->GetPrivateChannelInfo( GetWorldSetID(), GetPrivateChannelID() );
			if( pPrivateChatChannel )
			{
				if( GetCharacterDBID() != pPrivateChatChannel->GetMasterCharacterDBID() )
					return ERROR_INVALIDPACKET;
#if defined( _VILLAGESERVER )
				CDNUserSession* pSession = g_pUserSessionManager->FindUserSessionByName(pPacket->wszCharacterName);
				if(pSession)
				{
					if(pSession->GetPrivateChannelID())
					{
						SendPrivateChatChannelInviteResult(ERROR_ALREADY_JOINCHANNEL);
						return ERROR_ALREADY_JOINCHANNEL;
					}
					else
					{
						pSession->SetPrivateChannelID(pPrivateChatChannel->GetChannelID());
						pSession->GetDBConnection()->QueryInvitePrivateChatChannelMember(pSession, pPrivateChatChannel->GetChannelID(), GetAccountDBID());
					}
					return ERROR_NONE;
				}
#endif				
#if defined( _VILLAGESERVER )
				if(g_pMasterConnection && g_pMasterConnection->GetActive() )
				{
					g_pMasterConnection->SendInvitePrivateChatChannel(pPrivateChatChannel->GetChannelID(), GetAccountDBID(), pPacket->wszCharacterName);
				}
#elif defined( _GAMESERVER )
				if(g_pMasterConnectionManager )
				{
					g_pMasterConnectionManager->SendInvitePrivateChatChannel(GetWorldSetID(), pPrivateChatChannel->GetChannelID(), GetAccountDBID(), pPacket->wszCharacterName);
				}
#endif

			}			
			return ERROR_NONE;
		}
		break;
	case ePrivateChatChannel::CS_PRIVATECHAT_CHANNEL_OUT:
		{
			PrivateChatChannel::CSPrivateChatChannleOut *pPacket = reinterpret_cast<PrivateChatChannel::CSPrivateChatChannleOut*>(pData);
			if (sizeof(PrivateChatChannel::CSPrivateChatChannleOut) != nLen)
				return ERROR_INVALIDPACKET;	

			if(GetPrivateChannelID() <= 0)
			{
				return ERROR_NONE;
			}

			CDNPrivateChaChannel* pPrivateChatChannel = g_pPrivateChatChannelManager->GetPrivateChannelInfo( GetWorldSetID(), GetPrivateChannelID() );
			if(pPrivateChatChannel)
			{
				pPrivateChatChannel->DelPrivateChannelMember( PrivateChatChannel::Common::OutMember, GetCharacterDBID() );
				SendPrivateChatChannelOutResult( ERROR_NONE );

				if( GetCharacterDBID() == pPrivateChatChannel->GetMasterCharacterDBID() )
				{					
					m_pDBCon->QueryModPrivateChatChannelInfo( m_pSession, pPrivateChatChannel->GetChannelName(), PrivateChatChannel::Common::ChangeMaster, pPrivateChatChannel->GetChannelPassword(), pPrivateChatChannel->GetNextMasterCharacterDBID());
					pPrivateChatChannel->ModPrivateChannelInfo(  PrivateChatChannel::Common::ChangeMaster, pPrivateChatChannel->GetChannelPassword(), pPrivateChatChannel->GetNextMasterCharacterDBID() );
#if defined( _VILLAGESERVER )
					if(g_pMasterConnection && g_pMasterConnection->GetActive() )
					{
						g_pMasterConnection->SendModPrivateChatChannelInfo( pPrivateChatChannel->GetChannelID(), PrivateChatChannel::Common::ChangeMaster, pPrivateChatChannel->GetChannelPassword(), pPrivateChatChannel->GetMasterCharacterDBID() );
					}
#elif defined( _GAMESERVER )
					if(g_pMasterConnectionManager )
					{
						g_pMasterConnectionManager->SendModPrivateChatChannelInfo( GetWorldSetID(), pPrivateChatChannel->GetChannelID(), PrivateChatChannel::Common::ChangeMaster, pPrivateChatChannel->GetChannelPassword(), pPrivateChatChannel->GetMasterCharacterDBID() );
					}
#endif
				}				

#if defined( _VILLAGESERVER )
				if(g_pMasterConnection && g_pMasterConnection->GetActive() )
				{					
					g_pMasterConnection->SendDelPrivateChatChannelMember( PrivateChatChannel::Common::OutMember, GetPrivateChannelID(), GetCharacterDBID(), GetCharacterName() );
				}
#elif defined( _GAMESERVER )
				if(g_pMasterConnectionManager )
				{					
					g_pMasterConnectionManager->SendDelPrivateChatChannelMember( GetWorldSetID(), PrivateChatChannel::Common::OutMember, GetPrivateChannelID(), GetCharacterDBID(), GetCharacterName() );
				}
#endif
			}

			m_pDBCon->QueryOutPrivateChatChannelMember( m_pSession, PrivateChatChannel::Common::OutMember );
			SetPrivateChannelID(0);

			return ERROR_NONE;
		}
		break;	
	case ePrivateChatChannel::CS_PRIVATECHAT_CHANNEL_KICK:
		{
			PrivateChatChannel::CSPrivateChatChannleKick *pPacket = reinterpret_cast<PrivateChatChannel::CSPrivateChatChannleKick*>(pData);
			if (sizeof(PrivateChatChannel::CSPrivateChatChannleKick) != nLen)
				return ERROR_INVALIDPACKET;		


			if(GetPrivateChannelID() <= 0)
			{
				return ERROR_NONE;
			}

			CDNPrivateChaChannel* pPrivateChatChannel = g_pPrivateChatChannelManager->GetPrivateChannelInfo( GetWorldSetID(), GetPrivateChannelID() );

			if(pPrivateChatChannel)
			{
				if( GetCharacterDBID() == pPrivateChatChannel->GetMasterCharacterDBID() )
				{						
					TPrivateChatChannelMember Member = pPrivateChatChannel->GetPrivateChannelMember(pPacket->wszCharacterName);
					if(Member.biCharacterDBID > 0)
						m_pDBCon->QueryKickPrivateChatChannelMember( m_pSession, pPacket->wszCharacterName, Member.biCharacterDBID );
					else
						SendPrivateChatChannelKickResult(ERROR_INVALID_PIRVATECHATMEMBERNAME, false);
				}
				else
					return ERROR_GENERIC_INVALIDREQUEST;
			}
			return ERROR_NONE;
		}
		break;	
	case ePrivateChatChannel::CS_PRIVATECHAT_CHANNEL_MOD:
		{
			PrivateChatChannel::CSPrivateChatChannleMod *pPacket = reinterpret_cast<PrivateChatChannel::CSPrivateChatChannleMod*>(pData);
			if (sizeof(PrivateChatChannel::CSPrivateChatChannleMod) != nLen)
				return ERROR_INVALIDPACKET;		


			if(GetPrivateChannelID() <= 0)
			{
				return ERROR_NONE;
			}

			CDNPrivateChaChannel* pPrivateChatChannel = g_pPrivateChatChannelManager->GetPrivateChannelInfo( GetWorldSetID(), GetPrivateChannelID() );

			if(pPrivateChatChannel)
			{
				if( GetCharacterDBID() == pPrivateChatChannel->GetMasterCharacterDBID() )
				{					
					if(pPacket->nModType == PrivateChatChannel::Common::ChangeMaster )
					{
						if(pPrivateChatChannel->CheckPrivateChannelMember(pPacket->biCharacterDBID))
						{
							pPrivateChatChannel->ModPrivateChannelInfo(  PrivateChatChannel::Common::ChangeMaster, pPrivateChatChannel->GetChannelPassword(), pPacket->biCharacterDBID );
							m_pDBCon->QueryModPrivateChatChannelInfo( m_pSession, pPrivateChatChannel->GetChannelName(), pPacket->nModType, pPrivateChatChannel->GetChannelPassword(), pPacket->biCharacterDBID );
						}
						else
						{
							SendPrivateChatChannelResult(ERROR_NOTEXIST_CHANNELMEMBER);
							return ERROR_NOTEXIST_CHANNELMEMBER;
						}
					}
					else if(pPacket->nModType == PrivateChatChannel::Common::ChangePassWord && pPacket->nPassWord >= -1 )
					{
						pPrivateChatChannel->ModPrivateChannelInfo( PrivateChatChannel::Common::ChangePassWord, pPacket->nPassWord, GetCharacterDBID() );
						m_pDBCon->QueryModPrivateChatChannelInfo( m_pSession, pPrivateChatChannel->GetChannelName(), pPacket->nModType, pPacket->nPassWord, GetCharacterDBID() );
					}
					else
						return ERROR_GENERIC_INVALIDREQUEST;

#if defined( _VILLAGESERVER )
					if(g_pMasterConnection && g_pMasterConnection->GetActive() )
					{
						g_pMasterConnection->SendModPrivateChatChannelInfo( pPrivateChatChannel->GetChannelID(), (PrivateChatChannel::Common::eModType)pPacket->nModType, pPrivateChatChannel->GetChannelPassword(), pPrivateChatChannel->GetMasterCharacterDBID() );
					}
#elif defined( _GAMESERVER )
					if(g_pMasterConnectionManager )
					{
						g_pMasterConnectionManager->SendModPrivateChatChannelInfo( GetWorldSetID(), pPrivateChatChannel->GetChannelID(), (PrivateChatChannel::Common::eModType)pPacket->nModType, pPrivateChatChannel->GetChannelPassword(), pPrivateChatChannel->GetMasterCharacterDBID() );
					}
#endif
				}
				else
					return ERROR_GENERIC_INVALIDREQUEST;
			}
		}
		break;		
	}
	return ERROR_UNKNOWN_HEADER;
}
#endif

#if defined( PRE_ALTEIAWORLD_EXPLORE )
int CDNUserBase::OnRecvWorldAlteiaMessage(int nSubCmd, char *pData, int nLen)
{	
//	switch( nSubCmd )
//	{	
//	}
	return ERROR_UNKNOWN_HEADER;
}
#endif // #if defined( PRE_ALTEIAWORLD_EXPLORE )


#if defined(PRE_ADD_CHAT_MISSION)
int CDNUserBase::OnRecvMissionMessage( int nSubCmd, char* pData, int nLen )
{
	switch( nSubCmd )
	{	
	case eMission::CS_CHAT_MISSION:
		{
			if (sizeof(CSChatMission) != nLen)
				return ERROR_INVALIDPACKET;

			CSChatMission *pPacket = (CSChatMission*)pData;			
			
			//Only check if MID is actually in use -> forging packets takes more effort than typing chat directly, so skip further validation
			if(!g_pDataManager->CheckTypingData(pPacket->nUIStringMid))
				return ERROR_INVALIDPACKET;	//Disconnect if packet Mid is tampered with

			GetEventSystem()->OnEvent( EventSystem::OnChatMission, 1, EventSystem::UIStringMID, pPacket->nUIStringMid );			
			return ERROR_NONE;
		}
		break;
	}

	return ERROR_UNKNOWN_HEADER;
}
#endif

TSkillGroup *CDNUserBase::GetSkillData( bool bEntireData/* = false*/ )
{ 
	if(!bEntireData && m_UserData.Status.cSkillPage >= DualSkill::Type::Primary && m_UserData.Status.cSkillPage < DualSkill::Type::MAX)
		return &m_UserData.Skill[m_UserData.Status.cSkillPage]; 

	return m_UserData.Skill;
}

void CDNUserBase::SetAccountName(WCHAR *pAccountName)
{
	if ( !pAccountName ) return;

	_wcscpy(m_wszAccountName, _countof(m_wszAccountName), pAccountName, (int)wcslen(pAccountName));
	WideCharToMultiByte(CP_ACP, 0, m_wszAccountName, -1, m_szAccountName, IDLENMAX, NULL, NULL);
}

void CDNUserBase::SetCharacterName(const WCHAR* pwszName)
{
	if (!pwszName) return;

	_wcscpy(m_UserData.Status.wszCharacterName, _countof(m_UserData.Status.wszCharacterName), pwszName, (int)wcslen(pwszName));
	WideCharToMultiByte(CP_ACP, 0, GetCharacterName(), -1, m_szCharacterName, NAMELENMAX, NULL, NULL);
}

void CDNUserBase::SetMapIndex(int nMapIndex)
{
	if (nMapIndex <= 0) return;
	m_UserData.Status.nMapIndex = nMapIndex;
}

void CDNUserBase::SetLastMapIndex( int nMapIndex )
{
	if (nMapIndex <= 0) return;
#if defined( _VILLAGESERVER )
	const sChannelInfo* pChannelInfo = g_pMasterConnection->GetChannelInfoFromMapIndex( nMapIndex );
	if( pChannelInfo && !(pChannelInfo->nChannelAttribute&GlobalEnum::CHANNEL_ATT_USELASTMAPINDEX) )
	{
		m_UserData.Status.nLastVillageMapIndex = nMapIndex;
	}
#elif defined(_GAMESERVER)
	int nAttribute = g_pDataManager->GetChannelMapAtt(GetWorldSetID(), nMapIndex);
	if (nAttribute == 0)	return;
	if( !(nAttribute&GlobalEnum::CHANNEL_ATT_USELASTMAPINDEX) )
	{
		m_UserData.Status.nLastVillageMapIndex = nMapIndex;
	}
#endif
}

#if defined(PRE_ADD_EXPUP_ITEM)
void CDNUserBase::ChangeExp(int nChangeExp, int nLogCode, INT64 biFKey, bool bAbsolute)
#else
void CDNUserBase::ChangeExp(int nChangeExp, int nLogCode, INT64 biFKey)
#endif
{
	if( nLogCode != DBDNWorldDef::CharacterExpChangeCode::Admin && nChangeExp == 0) 
		return;

	if( m_UserData.Status.cAccountLevel < AccountLevel_Master ) {
		int nLevelLimit = (int)CGlobalWeightTable::GetInstance().GetValue( CGlobalWeightTable::PlayerLevelLimit );
		if( m_UserData.Status.cLevel >= nLevelLimit ) return;
	}

#if defined( _VILLAGESERVER )
#if defined(PRE_ADD_EXPUP_ITEM)
	if( nChangeExp > 0 && !bAbsolute ) // Exclude bonus when using absolute-value items
#else
	if( nChangeExp > 0 )
#endif
	{	
		TPlayerCommonLevelTableInfo* pPlayerCommonLevelTableInfo = g_pDataManager->GetPlayerCommonLevelTable(GetLevel());
		if( pPlayerCommonLevelTableInfo )
		{
			//Increase by 1.5x here (doing it in one place should be easier to manage... dyss)
			nChangeExp = (int)(nChangeExp * pPlayerCommonLevelTableInfo->fAddGainExp);		
		}

#if defined(PRE_ADD_WEEKLYEVENT)
		int nThreadID = 0;
#ifdef _GAMESERVER
		nThreadID = m_pSession->GetGameRoom()->GetServerID();
#endif		//#ifdef _GAMESERVER
		float fEventValue = g_pDataManager->GetWeeklyEventValuef(WeeklyEvent::Player, GetClassID(), WeeklyEvent::Event_5, nThreadID);

		if (fEventValue != 0.f)
			nChangeExp += (int)(nChangeExp * fEventValue);
#endif	// #if defined(PRE_ADD_WEEKLYEVENT)
	}
#endif // #if defined( _VILLAGESERVER )

	int nExp = nChangeExp;

	m_UserData.Status.nExp += nExp;
	if( m_UserData.Status.cAccountLevel < AccountLevel_Master ) {
		int nLevelLimit = (int)CGlobalWeightTable::GetInstance().GetValue( CGlobalWeightTable::PlayerLevelLimit );
		int nLimitExp = g_pDataManager->GetExp( GetUserJob(), nLevelLimit );
		// if( m_UserData.Status.nExp == nLimitExp ) return;
		// else 
		if( m_UserData.Status.nExp > nLimitExp ) m_UserData.Status.nExp = nLimitExp;
	}

	BYTE cLevel = g_pDataManager->GetLevel(GetUserJob(), GetExp());
	if( m_UserData.Status.cAccountLevel < AccountLevel_Master ) {
		int nLevelLimit = (int)CGlobalWeightTable::GetInstance().GetValue( CGlobalWeightTable::PlayerLevelLimit );
		if (cLevel > nLevelLimit) cLevel = nLevelLimit;
	}

	if (cLevel > m_UserData.Status.cLevel){		// If higher than current level
		int nLevelLog = DBDNWorldDef::CharacterLevelChangeCode::Normal;
		switch(nLogCode)
		{
		case DBDNWorldDef::CharacterExpChangeCode::Cheat: nLevelLog = DBDNWorldDef::CharacterLevelChangeCode::Cheat; break;
		case DBDNWorldDef::CharacterExpChangeCode::Admin: nLevelLog = DBDNWorldDef::CharacterLevelChangeCode::Admin; break;
		}

		SetLevel(cLevel, nLevelLog, true);	// Save to DB here.
		if( IsPCBang() )
		{
			//If in a PC bang, check if there are newly available titles upon leveling up and send
			m_pAppellation->SetPCbangAppellation(true);
		}

		m_pDBCon->QueryExp(m_pSession, nLogCode, biFKey);
	}
	else{
		if (nLogCode != DBDNWorldDef::CharacterExpChangeCode::Dungeon)	// Do not leave behind if it is a stage
		{
			m_pDBCon->QueryExp(m_pSession, nLogCode, biFKey);
		}
#if defined(_VILLAGESERVER)
		if( nExp > 0 )
			SendAddExp(m_nSessionID, m_UserData.Status.nExp, nExp);
#endif
	}
}

void CDNUserBase::SetExp(UINT nExp, int nLogCode, INT64 biFKey, bool bDBSave)
{
	if (m_UserData.Status.nExp == nExp) return;

	int nGapExp = nExp - m_UserData.Status.nExp;

	m_UserData.Status.nExp = nExp;
	// Level restriction
	if( m_UserData.Status.cAccountLevel < AccountLevel_Master ) {
		int nLevelLimit = (int)CGlobalWeightTable::GetInstance().GetValue( CGlobalWeightTable::PlayerLevelLimit );
		int nLimitExp = g_pDataManager->GetExp( GetUserJob(), nLevelLimit );
		if( m_UserData.Status.nExp > nLimitExp ) {
			m_UserData.Status.nExp = nLimitExp;
		}
	}

	if (bDBSave)
	{
		m_pDBCon->QueryExp(m_pSession, nLogCode, biFKey);
	}
}
void CDNUserBase::SetLevel(BYTE cLevel, int nLogCode, bool bDBSave)
{
	if (GetLevel() == cLevel) return;	// If equal, exit
	if (cLevel > CHARLEVELMAX) cLevel = CHARLEVELMAX;	// Must not exceed max value
#if defined(PRE_ADD_EXPUP_ITEM)	
	BYTE cPreLevel = GetLevel();		
#endif
	m_UserData.Status.cLevel = cLevel;

	if (bDBSave){
		m_pDBCon->QueryLevel(m_pSession, nLogCode);	// Save level to DB 20100128
	}
#if defined(PRE_ADD_EXPUP_ITEM)
	if (cPreLevel < cLevel)
	{
		for( BYTE bCurLevel=cPreLevel+1; bCurLevel<=cLevel; ++bCurLevel)
		{
			SendLevelupEventMail(bCurLevel, GetClassID(), GetUserJob());
		}
	}	
#else
	SendLevelupEventMail(GetLevel(), GetClassID(), GetUserJob());
#endif // #if defined(PRE_ADD_EXPUP_ITEM)
#if defined( PRE_ADD_TOTAL_LEVEL_SKILL )
	if (cPreLevel < cLevel)
	{
		AddTotalLevelSkillLevel(cLevel - cPreLevel);
		SendTotalLevel( m_pSession->GetSessionID(), GetTotalLevelSkillLevel() );
	}
#endif

#if defined( PRE_MOD_71820 )
	const TGuildUID GuildUID = GetGuildUID();
	if (GuildUID.IsSet())
	{
		CDNGuildBase* pGuild = g_pGuildManager->At(GuildUID);
		if (pGuild) 
		{
#if defined(PRE_ADD_JOINGUILD_SUPPORT)
			if (GetDBConnection() && bDBSave)
				GetDBConnection()->QueryGuildSupportRewardInfo(m_pSession);
#endif		//#if defined(PRE_ADD_JOINGUILD_SUPPORT)
		}
		else
			_DANGER_POINT();
	}
#endif // #if defined( PRE_MOD_71820 )

}

unsigned char CDNUserBase::GetLevel()
{
	if (m_UserData.Status.cLevel <= 0) m_UserData.Status.cLevel = 1;
	if (m_UserData.Status.cLevel > CHARLEVELMAX) m_UserData.Status.cLevel = CHARLEVELMAX;

	return m_UserData.Status.cLevel;
}

#if defined(PRE_ADD_PVPLEVEL_MISSION)
void CDNUserBase::SetPvPLevel(BYTE cLevel, bool bCallEvent)
{
	m_UserData.PvP.cLevel = cLevel;
	if(bCallEvent)
		GetEventSystem()->OnEvent( EventSystem::OnPvPLevelChange );
}
#endif


void CDNUserBase::SetUserJob(BYTE cJob)
{
	m_UserData.Status.cJob = cJob;
	m_pDBCon->QueryJob(m_pSession, cJob);
	GetEventSystem()->OnEvent( EventSystem::OnJobChange );

	for (int i = 0; i < JOBMAX; i++)
	{
		if (m_UserData.Status.cJobArray[i] > 0) 
			continue;
		m_UserData.Status.cJobArray[i] = cJob;
#if defined(_VILLAGESERVER)
		if( GetPartyID() > 0)
		{
			CDNParty *Party = g_pPartyManager->GetParty(GetPartyID());
			if(Party)
			{
				Party->SendAllRefreshParty( PARTYREFRESH_NONE );
			}
		}
#endif
		return;
	}	
}

// Coin
bool CDNUserBase::AddCoin(INT64 nCoin, int nLogCode, INT64 biFKey, bool bSend)
{
	if ((nCoin <= 0) ||(nCoin > COINMAX)) return false;
	if (m_UserData.Status.nCoin + nCoin > COINMAX) return false;

	INT64 nTotalCoin = m_UserData.Status.nCoin;
	m_UserData.Status.nCoin += nCoin;

	if (m_UserData.Status.nCoin <= 0) m_UserData.Status.nCoin = 0;
	if (m_UserData.Status.nCoin >= COINMAX) m_UserData.Status.nCoin = COINMAX;		// Cap at 4.2 billion for now.

	if (nLogCode > 0)
		m_pDBCon->QueryCoin(m_pSession, nLogCode, biFKey, nCoin, m_UserData.Status.nCoin);	// Save coin to DB

	if (bSend) SendAddCoin(m_nSessionID, nTotalCoin, nCoin);

	// ItemID is 0 when acquiring money
	GetEventSystem()->OnEvent( EventSystem::OnItemGain, 2, EventSystem::ItemID, 0, EventSystem::UseCoinCount, (int)nCoin );

	return true;
}

bool CDNUserBase::DelCoin(INT64 nCoin, int nLogCode, INT64 biFKey, bool bSend)
{
	if ((nCoin <= 0) ||(nCoin > COINMAX)) return false;
	if (m_UserData.Status.nCoin < nCoin) return false;

	INT64 nTotalCoin = m_UserData.Status.nCoin;
	m_UserData.Status.nCoin -= nCoin;

	if (m_UserData.Status.nCoin <= 0) m_UserData.Status.nCoin = 0;
	if (m_UserData.Status.nCoin >= COINMAX) m_UserData.Status.nCoin = COINMAX;		// Cap at 4.2 billion for now.

	if (nLogCode > 0)
		m_pDBCon->QueryCoin(m_pSession, nLogCode, biFKey, -nCoin, m_UserData.Status.nCoin);	// Save coin to DB

	if (bSend) SendAddCoin(m_nSessionID, nTotalCoin, -nCoin);
	return true;
}

INT64 CDNUserBase::GetCoin()
{
	if (m_UserData.Status.nCoin <= 0) m_UserData.Status.nCoin = 0;
	if (m_UserData.Status.nCoin >= COINMAX) m_UserData.Status.nCoin = COINMAX;		// Cap at 4.2 billion for now.

	return m_UserData.Status.nCoin;
}

bool CDNUserBase::CheckEnoughCoin(INT64 nCheckCoin)
{
	if (nCheckCoin < 0) return false;	// Check for overflow just in case
	if (nCheckCoin > GetCoin()) return false;	// No money in the pouch~
	return true;
}

bool CDNUserBase::CheckMaxCoin(INT64 nCheckCoin)
{
	if (GetCoin() + nCheckCoin > COINMAX) return false;	// Exceeded max
	return true;
}

void CDNUserBase::SetWarehouseCoin(INT64 nCoin)
{
	if ((nCoin <= 0) ||(nCoin > COINMAX)) return;
	if (m_UserData.Status.nWarehouseCoin == nCoin) return;

	m_UserData.Status.nWarehouseCoin = nCoin;
	if (m_UserData.Status.nWarehouseCoin > COINMAX) m_UserData.Status.nWarehouseCoin = COINMAX;

	m_pDBCon->QueryWarehouseCoin(m_pSession, DBDNWorldDef::CoinChangeCode::Use, nCoin, GetCoin(), GetWarehouseCoin());	// Save warehousecoin to DB
}

void CDNUserBase::AddWarehouseCoin(INT64 nCoin, int nLogCode, INT64 biFKey)
{
	if ((nCoin <= 0) ||(nCoin > COINMAX)) return;
	if (m_UserData.Status.nWarehouseCoin + nCoin > COINMAX) return;

	m_UserData.Status.nWarehouseCoin += nCoin;
	if (m_UserData.Status.nWarehouseCoin > COINMAX) m_UserData.Status.nWarehouseCoin = COINMAX;
	if (m_UserData.Status.nWarehouseCoin < 0) m_UserData.Status.nWarehouseCoin = 0;

	if (nLogCode > 0)
		m_pDBCon->QueryWarehouseCoin(m_pSession, nLogCode, nCoin, GetCoin(), GetWarehouseCoin());	// Save warehousecoin to DB
}

void CDNUserBase::DelWarehouseCoin(INT64 nCoin, int nLogCode, INT64 biFKey)
{
	if ((nCoin <= 0) ||(nCoin > COINMAX)) return;
	if (nCoin > m_UserData.Status.nWarehouseCoin) return;

	m_UserData.Status.nWarehouseCoin -= nCoin;
	if (m_UserData.Status.nWarehouseCoin > COINMAX) m_UserData.Status.nWarehouseCoin = COINMAX;
	if (m_UserData.Status.nWarehouseCoin < 0) m_UserData.Status.nWarehouseCoin = 0;

	if (nLogCode > 0)
		m_pDBCon->QueryWarehouseCoin(m_pSession, nLogCode, nCoin, GetCoin()+nCoin, GetWarehouseCoin());	// Save warehousecoin to DB

}

INT64 CDNUserBase::GetWarehouseCoin()
{
	if (m_UserData.Status.nWarehouseCoin <= 0) m_UserData.Status.nWarehouseCoin = 0;
	if (m_UserData.Status.nWarehouseCoin >= COINMAX) m_UserData.Status.nWarehouseCoin = COINMAX;		// Cap at 4.2 billion for now.

	return m_UserData.Status.nWarehouseCoin;
}

bool CDNUserBase::CheckEnoughWarehouseCoin(INT64 nCheckCoin)
{
	if (nCheckCoin > GetWarehouseCoin()) return false;	// No money in the pouch~
	return true;
}

bool CDNUserBase::CheckMaxWarehouseCoin(INT64 nCheckCoin)
{
	if (GetWarehouseCoin() + nCheckCoin > COINMAX) return false;	// Exceeded max
	return true;
}

// SkillPoint
void CDNUserBase::ChangeSkillPoint(unsigned short nPoint, int nSkillID, bool bDBSave, int nLogCode, char cSkillPage)
{
	if (nPoint == 0) return;

	if (cSkillPage == DualSkill::Type::MAX)
	{
		cSkillPage = GetSkillPage();
	}

	if(cSkillPage < DualSkill::Type::Primary || cSkillPage >= DualSkill::Type::MAX)	
		return;
	
	m_UserData.Skill[cSkillPage].wSkillPoint += nPoint;
	if (m_UserData.Skill[cSkillPage].wSkillPoint <= 0) m_UserData.Skill[cSkillPage].wSkillPoint = 0;

	// Save to DB
	if (bDBSave){
		m_pDBCon->QueryIncreaseSkillPoint(m_pSession, nPoint, nLogCode, cSkillPage);	// Save skill points to DB
	}
}

void CDNUserBase::SetSkillPoint(USHORT nPoint, char cSkillPage)
{
	if (nPoint == 0) return;

	//// If less than or equal to currently owned skills, just return
	//if ( nPoint <= GetSkillPoint() ) return;

	if( cSkillPage < DualSkill::Type::Primary || cSkillPage >= DualSkill::Type::MAX)
		return;
	m_UserData.Skill[cSkillPage].wSkillPoint = nPoint;
	if (m_UserData.Skill[cSkillPage].wSkillPoint <= 0) m_UserData.Skill[cSkillPage].wSkillPoint = 0;
}

// Set SkillPoint. Used for cheat keys and skill reset.
void CDNUserBase::SetCheatSkillPoint(unsigned short nPoint, int nSkillID)
{
	if (nPoint == 0) return;

	// If less than or equal to currently owned skills, just return
	if ( nPoint <= GetSkillPoint() )
		return;

	USHORT unIncreasePoint = nPoint-GetSkillPoint();

	m_UserData.Skill[GetSkillPage()].wSkillPoint = nPoint;
	if (m_UserData.Skill[GetSkillPage()].wSkillPoint <= 0) m_UserData.Skill[GetSkillPage()].wSkillPoint = 0;

	m_pDBCon->QueryIncreaseSkillPoint(m_pSession, unIncreasePoint, DBDNWorldDef::SkillPointCode::LevelUp, GetSkillPage());

	// Not started by client alone; also used by server-side cheatset, so send a separate packet.
	SendPushSkillPoint( nPoint );
}

USHORT CDNUserBase::GetSkillPoint()		//Currently selected skill tree points 
{
	return m_UserData.Skill[m_UserData.Status.cSkillPage].wSkillPoint; 
}

//Specific skill tree point
USHORT CDNUserBase::GetSkillPoint(int nSkillPageIndex)
{ 													
	if(nSkillPageIndex >= DualSkill::Type::Primary && nSkillPageIndex <= DualSkill::Type::Secondary)
		return m_UserData.Skill[nSkillPageIndex].wSkillPoint; 

	return 0;
}

float CDNUserBase::GetAvailSkillPointRatioByJob( int nSkillID )
{
	// Job ID
	const TSkillData* pSkillData = g_pDataManager->GetSkillData( nSkillID );
	if (!pSkillData) return 0.f;
	//const TJobTableData* pJobData = g_pDataManager->GetJobTableData( pSkillData->nNeedJobID );
	const TJobTableData* pJobData = g_pDataManager->GetJobTableData( GetUserJob() );
	if (!pJobData) return 0.f;

	// Job advancement tier
	float fLimitRatio = pJobData->afMaxUsingSP[ g_pDataManager->GetJobNumber( pSkillData->nNeedJobID ) ];

	//int iSkillPoint = GetSkillPoint();
	// Apply the per-job max usage ratio to the value based on the current character's total SP.

	//// Used skill points + remaining skill points - skill points gained from elixirs = (character level - 1) * 20
	//int nTotalSP = m_pUserSession->GetSkillPoint() + nUseSkillPoint - m_pUserSession->GetStatusData()->wEternityItem[ETERNITY_SP - 1];

	return fLimitRatio;
}

void CDNUserBase::SetCheatMaxFatigue()
{
	int nMax = g_pDataManager->GetFatigue(GetUserJob(), GetLevel());
	if (GetFatigue() != nMax){
		m_UserData.Status.wFatigue = nMax;
		m_pDBCon->QueryFatigue(m_pSession, DBDNWorldDef::FatigueTypeCode::Daily, nMax);
	}

	nMax = g_pDataManager->GetWeeklyFatigue(GetUserJob(), GetLevel());
	if (GetWeeklyFatigue() != nMax){
		m_UserData.Status.wWeeklyFatigue = nMax;
		m_pDBCon->QueryFatigue(m_pSession, DBDNWorldDef::FatigueTypeCode::Weekly, nMax);
	}

	nMax = g_pDataManager->GetPCBangParam1(PCBang::Type::Fatigue, GetPCBangGrade());
	if (GetPCBangFatigue() != nMax){
		m_UserData.Status.wPCBangFatigue = nMax;
		m_pDBCon->QueryFatigue(m_pSession, DBDNWorldDef::FatigueTypeCode::PCBang, nMax);
	}

#if defined(PRE_ADD_VIP)
	nMax = g_pDataManager->GetVIPFatigue(GetClassID(), GetLevel());
	if (GetVIPFatigue() != nMax){
		m_UserData.Status.wVIPFatigue = nMax;
		m_pDBCon->QueryFatigue(m_pSession, DBDNWorldDef::FatigueTypeCode::VIP, nMax);
	}
#endif	// #if defined(PRE_ADD_VIP)

	SendFatigue(m_nSessionID, GetFatigue(), GetWeeklyFatigue(), GetPCBangFatigue(), GetEventFatigue(), GetVIPFatigue());
}

void CDNUserBase::SetCheatFatigue(int nFatigue, int nWeeklyFatigue, int nPCBangFatigue, int nEventFatigue, int nVIPFatigue)
{
	if ((nFatigue > 0) && (m_UserData.Status.wFatigue != nFatigue)){
		int nMax = g_pDataManager->GetFatigue(GetUserJob(), GetLevel());
		if (nFatigue != nMax){
			if (nFatigue > nMax) nFatigue = nMax;
			m_UserData.Status.wFatigue = nFatigue;
			m_pDBCon->QueryFatigue(m_pSession, DBDNWorldDef::FatigueTypeCode::Daily, nFatigue);
		}
	}
	if ((nWeeklyFatigue > 0) && (m_UserData.Status.wWeeklyFatigue != nWeeklyFatigue)){
		int nMax = g_pDataManager->GetWeeklyFatigue(GetUserJob(), GetLevel());
		if (nWeeklyFatigue != nMax){
			if (nWeeklyFatigue > nMax) nWeeklyFatigue = nMax;
			m_UserData.Status.wWeeklyFatigue = nWeeklyFatigue;
			m_pDBCon->QueryFatigue(m_pSession, DBDNWorldDef::FatigueTypeCode::Weekly, nWeeklyFatigue);
		}
	}
	if ((nPCBangFatigue > 0) && (m_UserData.Status.wPCBangFatigue != nPCBangFatigue)){
		int nMax = g_pDataManager->GetPCBangParam1(PCBang::Type::Fatigue, GetPCBangGrade());
		if (nPCBangFatigue != nMax){
			if (nPCBangFatigue > nMax) nPCBangFatigue = nMax;
			m_UserData.Status.wPCBangFatigue = nPCBangFatigue;
			m_pDBCon->QueryFatigue(m_pSession, DBDNWorldDef::FatigueTypeCode::PCBang, nPCBangFatigue);
		}
	}
	if ((nEventFatigue > 0) && (m_UserData.Status.wEventFatigue != nEventFatigue)){
		int nMax = GetMaxEventFatigue();
		if (nEventFatigue != nMax){
			if (nEventFatigue > nMax) nEventFatigue = nMax;
			m_UserData.Status.wEventFatigue = nEventFatigue;
			m_pDBCon->QueryEventFatigue(m_pSession, nEventFatigue, false, 0);
		}
	}
#if defined(PRE_ADD_VIP)
	if ((nVIPFatigue > 0) && (m_UserData.Status.wVIPFatigue != nVIPFatigue)){
		int nMax = g_pDataManager->GetVIPFatigue(GetClassID(), GetLevel());
		if (nVIPFatigue != nMax){
			if (nVIPFatigue > nMax) nVIPFatigue = nMax;
			m_UserData.Status.wVIPFatigue = nVIPFatigue;
			m_pDBCon->QueryFatigue(m_pSession, DBDNWorldDef::FatigueTypeCode::VIP, nVIPFatigue);
		}
	}
#endif	// #if defined(PRE_ADD_VIP)

	SendFatigue(m_nSessionID, GetFatigue(), GetWeeklyFatigue(), GetPCBangFatigue(), GetEventFatigue(), GetVIPFatigue());
}

void CDNUserBase::SetDefaultMaxFatigue(bool bSend)
{
	int nMax = g_pDataManager->GetFatigue(GetUserJob(), GetLevel());
	m_UserData.Status.wFatigue = nMax;
	m_pDBCon->QueryFatigue(m_pSession, DBDNWorldDef::FatigueTypeCode::Daily, nMax);

	// Default value must be set whether in PC bang or not (need to get value when moving from normal to PC bang)
	nMax = g_pDataManager->GetPCBangParam1(PCBang::Type::Fatigue, PCBang::Grade::Normal);
	m_UserData.Status.wPCBangFatigue = nMax;
	m_pDBCon->QueryFatigue(m_pSession, DBDNWorldDef::FatigueTypeCode::PCBang, nMax);

#if defined(PRE_ADD_VIP)
	nMax = g_pDataManager->GetVIPFatigue(GetClassID(), GetLevel());
	m_UserData.Status.wVIPFatigue = nMax;
	m_pDBCon->QueryFatigue(m_pSession, DBDNWorldDef::FatigueTypeCode::VIP, nMax);
#endif	// #if defined(PRE_ADD_VIP)

	if (bSend) SendFatigue(m_nSessionID, GetFatigue(), GetWeeklyFatigue(), GetPCBangFatigue(), GetEventFatigue(), GetVIPFatigue());
}

void CDNUserBase::SetDefaultMaxWeeklyFatigue(bool bSend)
{
	int nMax = g_pDataManager->GetWeeklyFatigue(GetUserJob(), GetLevel());
	m_UserData.Status.wWeeklyFatigue = nMax;
	m_pDBCon->QueryFatigue(m_pSession, DBDNWorldDef::FatigueTypeCode::Weekly, nMax);

	if (bSend) SendFatigue(m_nSessionID, GetFatigue(), GetWeeklyFatigue(), GetPCBangFatigue(), GetEventFatigue(), GetVIPFatigue());
}

#if defined(PRE_ADD_VIP)
void CDNUserBase::SetDefaultVIPFatigue()
{
	int nMax = g_pDataManager->GetVIPFatigue(GetClassID(), GetLevel());
	m_UserData.Status.wVIPFatigue = nMax;
	m_pDBCon->QueryFatigue(m_pSession, DBDNWorldDef::FatigueTypeCode::VIP, nMax);

	SendFatigue(m_nSessionID, GetFatigue(), GetWeeklyFatigue(), GetPCBangFatigue(), GetEventFatigue(), GetVIPFatigue());

	// Let's also just send VIPRebirthCoin here
	m_cVIPRebirthCoin = g_pDataManager->GetVIPRebirthCoinCount(GetLevel());
	m_pDBCon->QueryRebirthCoin(m_pSession, GetRebirthCoin(), GetPCBangRebirthCoin(), m_cVIPRebirthCoin);
	SendRebirthCoin(ERROR_NONE, 0, _REBIRTH_SELF, m_nSessionID);
}
#endif	// #if defined(PRE_ADD_VIP)

void CDNUserBase::SetDefaultPCBangFatigue()
{
	int nMax = g_pDataManager->GetPCBangParam1(PCBang::Type::Fatigue, GetPCBangGrade());
	m_UserData.Status.wPCBangFatigue += nMax;	// Since users can move across PC bang grades, add them all up for now
	m_pDBCon->QueryFatigue(m_pSession, DBDNWorldDef::FatigueTypeCode::PCBang, m_UserData.Status.wPCBangFatigue);

	SendFatigue(m_nSessionID, GetFatigue(), GetWeeklyFatigue(), GetPCBangFatigue(), GetEventFatigue(), GetVIPFatigue());
}

void CDNUserBase::SetFatigue(int nFatigue, int nWeeklyFatigue, int nPCBangFatigue, int nEventFatigue, int nVIPFatigue, bool bDBSave/* = true*/)
{
	if ((nFatigue > 0) && (m_UserData.Status.wFatigue != nFatigue)){
		int nMax = g_pDataManager->GetFatigue(GetUserJob(), GetLevel());
		if (nFatigue != nMax){
			if (nFatigue > nMax) nFatigue = nMax;
			m_UserData.Status.wFatigue = nFatigue;
			if (bDBSave) m_pDBCon->QueryFatigue(m_pSession, DBDNWorldDef::FatigueTypeCode::Daily, nFatigue);
		}
	}
	if ((nWeeklyFatigue > 0) && (m_UserData.Status.wWeeklyFatigue != nWeeklyFatigue)){
		int nMax = g_pDataManager->GetWeeklyFatigue(GetUserJob(), GetLevel());
		if (nWeeklyFatigue != nMax){
			if (nWeeklyFatigue > nMax) nWeeklyFatigue = nMax;
			m_UserData.Status.wWeeklyFatigue = nWeeklyFatigue;
			if (bDBSave) m_pDBCon->QueryFatigue(m_pSession, DBDNWorldDef::FatigueTypeCode::Weekly, nWeeklyFatigue);
		}
	}
	if ((nPCBangFatigue > 0) && (m_UserData.Status.wPCBangFatigue != nPCBangFatigue)){
		int nMax = g_pDataManager->GetPCBangParam1(PCBang::Type::Fatigue, GetPCBangGrade());
		if (nPCBangFatigue != nMax){
			if (nPCBangFatigue > nMax) nPCBangFatigue = nMax;
			m_UserData.Status.wPCBangFatigue = nPCBangFatigue;
			if (bDBSave) m_pDBCon->QueryFatigue(m_pSession, DBDNWorldDef::FatigueTypeCode::PCBang, nPCBangFatigue);
		}
	}

	if ((nEventFatigue > 0) && (m_UserData.Status.wEventFatigue != nEventFatigue)){
		int nMax = GetMaxEventFatigue();
		if (nEventFatigue != nMax){
			if (nEventFatigue > nMax) nEventFatigue = nMax;
			m_UserData.Status.wEventFatigue = nEventFatigue;
			if (bDBSave)
				m_pDBCon->QueryEventFatigue(m_pSession, nEventFatigue, false, 0);
		}
	}

#if defined(PRE_ADD_VIP)
	if ((nVIPFatigue > 0) && (m_UserData.Status.wVIPFatigue != nVIPFatigue)){
		int nMax = g_pDataManager->GetVIPFatigue(GetClassID(), GetLevel());
		if (nVIPFatigue != nMax){
			if (nVIPFatigue > nMax) nVIPFatigue = nMax;
			m_UserData.Status.wVIPFatigue = nVIPFatigue;
			if (bDBSave) m_pDBCon->QueryFatigue(m_pSession, DBDNWorldDef::FatigueTypeCode::VIP, nVIPFatigue);
		}
	}
#endif	// #if defined(PRE_ADD_VIP)

	SendFatigue(m_nSessionID, GetFatigue(), GetWeeklyFatigue(), GetPCBangFatigue(), GetEventFatigue(), GetVIPFatigue());
}

void CDNUserBase::DecreaseFatigue(int nGap)
{
	// Event > PCBang(PCBangFatigue) > VIP > Daily(Fatigue) > Weekly(WeeklyFatigue)

	int nRemainEvent = nGap;
	if (GetEventFatigue() > 0){
		if (GetEventFatigue() < nRemainEvent)
			nRemainEvent -= GetEventFatigue();
		else 
			nRemainEvent -= nGap;

		DelEventFatigue(nGap - nRemainEvent);
	}
	if (nRemainEvent == 0) return;

	int nRemainPCBang = nRemainEvent;
	if (GetPCBangFatigue() > 0){
		if (GetPCBangFatigue() < nRemainPCBang)
			nRemainPCBang -= GetPCBangFatigue();
		else 
			nRemainPCBang -= nRemainEvent;

		DelPCBangFatigue(nRemainEvent - nRemainPCBang);
	}
	if (nRemainPCBang == 0) return;

#if defined(PRE_ADD_VIP)
	int nRemainVIP = nRemainPCBang;
	if (GetVIPFatigue() > 0){
		if (GetVIPFatigue() < nRemainVIP)
			nRemainVIP -= GetVIPFatigue();
		else
			nRemainVIP -= nRemainPCBang;

		DelVIPFatigue(nRemainPCBang - nRemainVIP);
	}
	if (nRemainVIP == 0) return;

	int nRemain = nRemainVIP;
	if (GetFatigue() > 0){
		if (GetFatigue() < nRemain)
			nRemain -= GetFatigue();
		else
			nRemain -= nRemainVIP;

		DelFatigue(nRemainVIP - nRemain);
	}
	if (nRemain == 0) return;

#else	// #if defined(PRE_ADD_VIP)
	int nRemain = nRemainPCBang;
	if (GetFatigue() > 0){
		if (GetFatigue() < nRemain)
			nRemain -= GetFatigue();
		else
			nRemain -= nRemainPCBang;

		DelFatigue(nRemainPCBang - nRemain);
	}
	if (nRemain == 0) return;
#endif	// #if defined(PRE_ADD_VIP)

	int nRemainWeekly = nRemain;
	if (GetWeeklyFatigue() > 0){
		if (GetWeeklyFatigue() < nRemainWeekly)
			nRemainWeekly -= GetWeeklyFatigue();
		else
			nRemainWeekly -= nRemain;

		DelWeeklyFatigue(nRemain - nRemainWeekly);
	}
	if (nRemainWeekly == 0) return;
}

void CDNUserBase::IncreaseFatigue(int nGap)
{
	if (nGap > 0){
		int nMax = g_pDataManager->GetFatigue(GetUserJob(), GetLevel());
		m_UserData.Status.wFatigue += nGap;
		if (m_UserData.Status.wFatigue > nMax) m_UserData.Status.wFatigue = nMax;

		m_pDBCon->QueryFatigue(m_pSession, DBDNWorldDef::FatigueTypeCode::Daily, m_UserData.Status.wFatigue);
	}
}

int CDNUserBase::GetPCBangFatigue()
{
	if (m_bPCBang) return m_UserData.Status.wPCBangFatigue;
	return 0;
}

int CDNUserBase::GetVIPFatigue()
{
	if (m_bVIP) return m_UserData.Status.wVIPFatigue;
	return 0;
}

int CDNUserBase::GetMaxEventFatigue()
{
#if defined(PRE_ADD_WORLD_EVENT)
	int nMax = g_pEvent->GetExtendFatigue(GetClassID());
#else
	int nMax = g_pEvent->GetExtendFatigue();
#endif //#if defined(PRE_ADD_WORLD_EVENT)
	if (nMax <= 0)
		nMax = g_pDataManager->GetFatigue(GetUserJob(), GetLevel());

	return nMax;
}

void CDNUserBase::ResetEventFatigue(int nResetTime)
{
	m_UserData.Status.wEventFatigue = 0;
	m_pDBCon->QueryEventFatigue(m_pSession, 0, true, nResetTime);

	SendFatigue(m_nSessionID, GetFatigue(), GetWeeklyFatigue(), GetPCBangFatigue(), GetEventFatigue(), GetVIPFatigue());
}

int CDNUserBase::GetAllFatigue()
{
	return GetFatigue()+GetWeeklyFatigue()+GetPCBangFatigue()+GetVIPFatigue()+GetEventFatigue();
}

void CDNUserBase::AddCashRebirthCoin(int nRebirthCount)
{
	if (nRebirthCount < 0) return;

	if( nRebirthCount == 0 )
	{
		// Write log and return
		g_Log.Log( LogType::_ERROR, m_pSession, L"AddCashRebirthCoin|[ADBID:%u, CDBID:%I64d, SID:%u] AddCashRebirthCoin Failed!", GetAccountDBID(), GetCharacterDBID(), m_pSession->GetSessionID() );
		return;
	}

	m_UserData.Status.wCashRebirthCoin += nRebirthCount;
	int nMax = g_pDataManager->GetCashCoinLimit(GetLevel());
	if (m_UserData.Status.wCashRebirthCoin > nMax)
		m_UserData.Status.wCashRebirthCoin = nMax;

#if defined(_VILLAGESERVER)
	SendRebirthCoin(ERROR_NONE, 0, _REBIRTH_SELF, m_nSessionID);
#elif defined(_GAMESERVER)
	if(m_pSession && m_pSession->GetPlayerActor() && m_pSession->GetPlayerActor()->GetPartyData())
		SendRebirthCoin(ERROR_NONE, m_pSession->GetPlayerActor()->GetPartyData()->nUsableRebirthCoin, _REBIRTH_SELF, m_nSessionID);
#endif	// #if defined(_VILLAGESERVER)
}

void CDNUserBase::SetDefaultMaxRebirthCoin(bool bSend)
{
	int nMax = g_pDataManager->GetCoinCount(GetLevel(), GetWorldSetID());
	m_UserData.Status.cRebirthCoin = nMax;

#if defined(PRE_ADD_VIP)
	if (IsVIP()){
		m_cVIPRebirthCoin = g_pDataManager->GetVIPRebirthCoinCount(GetLevel());
	}
#endif	// #if defined(PRE_ADD_VIP)

	int nPCMax = 0;
	nPCMax = g_pDataManager->GetPCBangParam1(PCBang::Type::RebirthCoin, PCBang::Grade::Normal);
	m_UserData.Status.cPCBangRebirthCoin = nPCMax;

	m_pDBCon->QueryRebirthCoin(m_pSession, nMax, nPCMax, GetVIPRebirthCoin());
}

void CDNUserBase::SetDefaultPCBangRebirthCoin()
{
	int nPCMax = g_pDataManager->GetPCBangParam1(PCBang::Type::RebirthCoin, GetPCBangGrade());
	m_UserData.Status.cPCBangRebirthCoin += nPCMax;	// Since users can move across PC bang grades, add them all up for now
	m_pDBCon->QueryRebirthCoin(m_pSession, GetRebirthCoin(), m_UserData.Status.cPCBangRebirthCoin, GetVIPRebirthCoin());

	SendRebirthCoin(ERROR_NONE, 0, _REBIRTH_SELF, m_nSessionID);
}

void CDNUserBase::SetRebirthCoin(int nRebirthCoin, int nPCBangRebirthCoin, int nRebirthCashCoin)
{
	if ((nRebirthCoin > 0) && (m_UserData.Status.cRebirthCoin != nRebirthCoin)){
		int nMax = g_pDataManager->GetCoinLimit(GetLevel());
		if (nRebirthCoin > nMax) nRebirthCoin = nMax;
		m_UserData.Status.cRebirthCoin = nRebirthCoin;
	}
	if ((nPCBangRebirthCoin > 0) && (m_UserData.Status.cPCBangRebirthCoin != nPCBangRebirthCoin)){
		int nMax = g_pDataManager->GetPCBangParam1(PCBang::Type::RebirthCoin, GetPCBangGrade());
		if (nPCBangRebirthCoin > nMax) nPCBangRebirthCoin = nMax;
		m_UserData.Status.cPCBangRebirthCoin = nPCBangRebirthCoin;
	}

	if ((nRebirthCashCoin > 0) && (m_UserData.Status.wCashRebirthCoin != nRebirthCashCoin)){
		m_UserData.Status.wCashRebirthCoin += nRebirthCashCoin;
		int nMax = g_pDataManager->GetCashCoinLimit(GetLevel());
		if (m_UserData.Status.wCashRebirthCoin > nMax) m_UserData.Status.wCashRebirthCoin = nMax;
		m_pDBCon->QueryAddCashRebirthCoin(m_pSession, nRebirthCashCoin, 0, DBDNWorldDef::CashRebirthCode::Admin, 0);
	}
}
void CDNUserBase::AddRebirthCoin(int nRebirthCoin, int nRebirthCashCoin)
{
	//Not checked here because CheckRebirthCoin checks the max value
	if (nRebirthCoin > 0)
	{		
		m_UserData.Status.cRebirthCoin += nRebirthCoin;
		m_pDBCon->QueryRebirthCoin(m_pSession, m_UserData.Status.cRebirthCoin, m_UserData.Status.cPCBangRebirthCoin, 0);
	}
	else if (nRebirthCashCoin > 0)
	{
		m_UserData.Status.wCashRebirthCoin += nRebirthCashCoin;		
		m_pDBCon->QueryAddCashRebirthCoin(m_pSession, nRebirthCashCoin, 0, DBDNWorldDef::CashRebirthCode::Item, 0);
	}

#if defined( _GAMESERVER )
	if(m_pSession && m_pSession->GetPlayerActor() && m_pSession->GetPlayerActor()->GetPartyData())
		SendRebirthCoin(ERROR_NONE, m_pSession->GetPlayerActor()->GetPartyData()->nUsableRebirthCoin, _REBIRTH_SELF, m_nSessionID);
#else
	SendRebirthCoin(ERROR_NONE, 0, _REBIRTH_SELF, m_nSessionID);
#endif
}

bool CDNUserBase::CheckRebirthCoin(int nRebirthCoin, int nRebirthCashCoin)
{	
	//For now, code assuming no item increases both at the same time; modify later if one is added
	if (nRebirthCoin > 0)
	{
		int nMax = g_pDataManager->GetCoinLimit(GetLevel());
		if ( nMax >= m_UserData.Status.cRebirthCoin + nRebirthCoin )
		{
			return true;
		}
	}
	else if (nRebirthCashCoin > 0)
	{
		int nMax = g_pDataManager->GetCashCoinLimit(GetLevel());
		if (nMax >= m_UserData.Status.wCashRebirthCoin + nRebirthCashCoin)
		{
			return true;
		}
	}
	return false;
}

bool CDNUserBase::DecreaseRebirthCoin(int nGap)
{
	// PC bang(PCBangRebirthCoin) > vip > Daily(RebirthCoin) > Cash(RebirthCashCoin)

	int nPCBangRemain = nGap;
	if (GetPCBangRebirthCoin() > 0){
		if (GetPCBangRebirthCoin() < nPCBangRemain)
			nPCBangRemain -= GetPCBangRebirthCoin();
		else 
			nPCBangRemain -= nGap;

		DelPCBangRebirthCoin(nGap - nPCBangRemain);
	}
	if (nPCBangRemain == 0) return true;

	int nVIPRemain = nPCBangRemain;
#if defined(PRE_ADD_VIP)
	if (GetVIPRebirthCoin() > 0){
		if (GetVIPRebirthCoin() < nVIPRemain)
			nVIPRemain -= GetVIPRebirthCoin();
		else
			nVIPRemain -= nPCBangRemain;

		DelVIPRebirthCoin(nPCBangRemain - nVIPRemain);
	}
	if (nVIPRemain == 0) return true;
#endif	// #if defined(PRE_ADD_VIP)

	int nRemain = nVIPRemain;
	if (GetRebirthCoin() > 0){
		if (GetRebirthCoin() < nRemain)
			nRemain -= GetRebirthCoin();
		else
			nRemain -= nPCBangRemain;

		DelRebirthCoin(nPCBangRemain - nRemain);
	}
	if (nRemain == 0) return true;

	int nCashRemain = nRemain;
	if (GetRebirthCashCoin() > 0){
		if (GetRebirthCashCoin() < nCashRemain)
			nCashRemain -= GetRebirthCashCoin();
		else
			nCashRemain -= nRemain;

		DelCashRebirthCoin(nRemain - nCashRemain);
	}
	if (nCashRemain == 0) return true;

	return false;
}

int CDNUserBase::GetPCBangRebirthCoin()
{
	if (m_bPCBang) return m_UserData.Status.cPCBangRebirthCoin; 
	return 0;
}

BYTE CDNUserBase::GetVIPRebirthCoin()
{
	if (m_bVIP) return m_cVIPRebirthCoin;
	return 0;
}

int CDNUserBase::GetTotalRebirthCoin()
{
	int iTotal = GetPCBangRebirthCoin()+GetRebirthCoin()+GetRebirthCashCoin();

#if defined(PRE_ADD_VIP)
	iTotal += GetVIPRebirthCoin();
#endif // #if defined(PRE_ADD_VIP)

	return iTotal;
}

// Implemented per issue #4036...
bool CDNUserBase::ChangeDyeColor( int nItemType, int *pTypeParam )
{
	DWORD dwColor = pTypeParam[0];
	/*
	if( pTypeParam[0] >= 0 ) {
		dwColor = pTypeParam[0];
	}
else { // If negative, reference the drop table and dye with a random value.

		TItemDropData *pDropData = g_pDataManager->GetItemDropData( pTypeParam[1] );

		int nTotalProb = 0;
		for( int i = 0; i < 20; i++) {		
			if( pDropData->nProb[i] <= 0 ) continue;
			if( pDropData->nInfo[i] < 1 ) continue;
			nTotalProb += pDropData->nProb[i];		
		}
#if defined(_GAMESERVER)
		int nMagicNumber = _rand(m_pSession->GetGameRoom()) % nTotalProb;
#elif defined(_VILLAGESERVER)
		int nMagicNumber = _rand() % nTotalProb;
#endif
		int nOffset = 0;
		for( int i = 0; i < 20; i++) {		
			if( pDropData->nProb[i] <= 0 ) continue;
			if( pDropData->nInfo[i] < 1 ) continue;
			if( nOffset <= nMagicNumber && nMagicNumber < nOffset + pDropData->nProb[i] ) {
				dwColor = (DWORD)(pDropData->nIndex[ i ]);
				break;
			}
			nOffset += pDropData->nProb[i];
		}
	}
	*/

	switch(nItemType){
	case ITEMTYPE_HAIRDYE: 
		if( GetStatusData()->dwHairColor == dwColor ) return false;
		GetStatusData()->dwHairColor = dwColor; 
		break;
	case ITEMTYPE_SKINDYE: 
		if( GetStatusData()->dwSkinColor == dwColor ) return false;
		GetStatusData()->dwSkinColor = dwColor; 
		break;
	case ITEMTYPE_EYEDYE: 
		if( GetStatusData()->dwEyeColor == dwColor ) return false;
		GetStatusData()->dwEyeColor = dwColor; 
		break;
	}

	BroadcastChangeColor(nItemType, dwColor);

	m_pDBCon->QueryColor(m_pSession, nItemType, dwColor);
	return true;
}

void CDNUserBase::BroadcastChangeColor(int nItemType, DWORD dwColor)
{
#if defined(_VILLAGESERVER)
	if (!m_pSession) return;

	m_pSession->GetParamData()->cType = nItemType;
	m_pSession->GetParamData()->dwColor = dwColor;

	m_pSession->SendUserLocalMessage(0, FM_CHANGECOLOR);

	if (m_pSession->GetPartyID() > 0){
		CDNParty * pParty = g_pPartyManager->GetParty(m_pSession->GetPartyID());
		if (pParty) pParty->SendAllRefreshParty(PARTYREFRESH_NONE);
	}

#elif defined(_GAMESERVER)
	if (!m_pSession->GetGameRoom()) return;

	for(DWORD i = 0; i < m_pSession->GetGameRoom()->GetUserCount(); i++) {
		CDNGameRoom::PartyStruct *pStruct = m_pSession->GetGameRoom()->GetPartyData(i);
		if( pStruct == NULL ) continue;
		pStruct->pSession->SendChangeColor(m_pSession->GetSessionID(), nItemType, dwColor);
	}
#endif
}

// QuickSlot
int CDNUserBase::AddQuickSlot(BYTE cSlotIndex, BYTE cSlotType, INT64 nID)
{
	if (cSlotIndex >= QUICKSLOTMAX) return ERROR_CHARACTER_QUICKSLOT_NOTFOUND;
	if (nID <= 0) return ERROR_CHARACTER_QUICKSLOT_NOTFOUND;

	bool bClear = false;
	switch(cSlotType)
	{
	case QUICKSLOTTYPE_NONE:	// none
		bClear = true;
		break;

	case QUICKSLOTTYPE_ITEM:	// item
		{
			TItemData *pItemData = g_pDataManager->GetItemData((int)nID);
			if (!pItemData) bClear = true;
		}
		break;

	case QUICKSLOTTYPE_SKILL:	// skill
		{
			TSkillData *pSkillData = g_pDataManager->GetSkillData((int)nID);
			if (!pSkillData) bClear = true;
		}
		break;

	case QUICKSLOTTYPE_GESTURE:	// Gesture
		{
			// Slot restrictions
			if (cSlotIndex < QUICKSLOTSEPARATE) bClear = true;
		}
		break;
	}

	if (bClear){
		m_UserData.Status.QuickSlot[cSlotIndex].cType = 0;
		m_UserData.Status.QuickSlot[cSlotIndex].nID = 0;
	}
	else{
		m_UserData.Status.QuickSlot[cSlotIndex].cType = cSlotType;
		m_UserData.Status.QuickSlot[cSlotIndex].nID = nID;
	}

	return ERROR_NONE;
}

int CDNUserBase::DelQuickSlot(BYTE cSlotIndex)
{
	if (cSlotIndex >= QUICKSLOTMAX) return ERROR_CHARACTER_QUICKSLOT_NOTFOUND;

	memset(&m_UserData.Status.QuickSlot[cSlotIndex], 0, sizeof(TQuickSlot));

	return ERROR_NONE;
}

#if !defined(PRE_DELETE_DUNGEONCLEAR)
//Dungeon
int CDNUserBase::GetDungeonLevelIndex( int nMapTableID, bool *bExist )
{
	int nEmptySlot = -1;
	for( int i=0; i<DUNGEONCLEARMAX; i++ ) {
		if( GetDungeonClearMapIndex(i) == 0 ) {
			if( nEmptySlot == -1 ) nEmptySlot = i;
			continue;
		}
		if( GetDungeonClearMapIndex(i) == nMapTableID ) {
			if( bExist ) *bExist = true;
			return i;
		}
	}
	if( bExist ) *bExist = false;
	return nEmptySlot;
}

bool CDNUserBase::CheckDungeonEnterLevel( int nMapTableID )
{
	bool bExist;
	int nSlotIndex = GetDungeonLevelIndex( nMapTableID, &bExist );
	if( nSlotIndex > -1 ) {
		if( !bExist ) {
			SetDungeonClear(nSlotIndex, nMapTableID, 0);	// Includes DB save 090731
		}
	}
	return true;
}

void CDNUserBase::CalcDungeonEnterLevel(int nMapTableID, char &cOpenHard, char &cOpenVeryHard)
{
	if( CheckDungeonEnterLevel( nMapTableID ) == false ) {
		cOpenHard = true;
		cOpenVeryHard = true;
		return;
	}

	int nSlotIndex = GetDungeonLevelIndex( nMapTableID, NULL );

	cOpenHard = false;
	cOpenVeryHard = false;
	//	bOpenNightmare = false;
	if( GetDungeonClearType(nSlotIndex) >= 1 ) cOpenHard = true;
	if( GetDungeonClearType(nSlotIndex) >= 2 ) cOpenVeryHard = true;
	//	if( GetDungeonClearType(nSlotIndex) >= 3 ) bOpenNightmare = true;
}

// Dungeon Clear
void CDNUserBase::SetDungeonClear(int nSlotIndex, int nMapIndex, int nType)
{
	if ((nSlotIndex < 0) ||(nSlotIndex >= DUNGEONCLEARMAX)) return;

	m_UserData.Status.DungeonClear[nSlotIndex].nMapIndex = nMapIndex;
	m_UserData.Status.DungeonClear[nSlotIndex].cType = nType;
	m_pDBCon->QueryDungeonClear(m_pSession, nType, nMapIndex);
}

void CDNUserBase::SetDungeonClearType(int nSlotIndex, int nType)
{
	if ((nSlotIndex < 0) ||(nSlotIndex >= DUNGEONCLEARMAX)) return;

	m_UserData.Status.DungeonClear[nSlotIndex].cType = nType;
	m_pDBCon->QueryDungeonClear(m_pSession, nType, m_UserData.Status.DungeonClear[nSlotIndex].nMapIndex);
}

int CDNUserBase::GetDungeonClearMapIndex(int nSlotIndex)
{
	if ((nSlotIndex < 0) ||(nSlotIndex >= DUNGEONCLEARMAX)) return -1;
	return m_UserData.Status.DungeonClear[nSlotIndex].nMapIndex;
}

char CDNUserBase::GetDungeonClearType(int nSlotIndex)
{
	if ((nSlotIndex < 0) ||(nSlotIndex >= DUNGEONCLEARMAX)) return -1;
	return m_UserData.Status.DungeonClear[nSlotIndex].cType;
}
#endif	// #if !defined(PRE_DELETE_DUNGEONCLEAR)

// Nest Clear
void CDNUserBase::AddNestClear(int nMapIndex)
{
	int nSlotIndex = -1;
	for( int i=0; i<NESTCLEARMAX; i++ ) {
		if( m_UserData.Status.NestClear[i].nMapIndex == nMapIndex ) {
			nSlotIndex = i;
			break;
		}
	}
	if( nSlotIndex == -1 ) {
		for( int i=0; i<NESTCLEARMAX; i++ ) {
			if( m_UserData.Status.NestClear[i].nMapIndex == 0 ) {
				nSlotIndex = i;
				break;
			}
		}
	}
	if( nSlotIndex == -1 ) return;

	m_UserData.Status.NestClear[nSlotIndex].nMapIndex = nMapIndex;

	char cClearType = NestClear::Type::Normal;
	if (IsPCBang()){
		int nNestClearCount = g_pDataManager->GetPCBangNestClearCount(GetPCBangGrade(), nMapIndex);
		if (m_UserData.Status.NestClear[nSlotIndex].cPCBangClearCount >= nNestClearCount){
			m_UserData.Status.NestClear[nSlotIndex].cClearCount += 1;
		}
		else{
			m_UserData.Status.NestClear[nSlotIndex].cPCBangClearCount += 1;
			cClearType = NestClear::Type::PCBang;
		}
	}
	else
		m_UserData.Status.NestClear[nSlotIndex].cClearCount += 1;

	m_pDBCon->QueryNestClear(m_pSession, nMapIndex, cClearType);
}

char CDNUserBase::GetNestClearTotalCount(int nMapIndex)
{
	int nNestClearCount = g_pDataManager->GetPCBangNestClearCount(GetPCBangGrade(), nMapIndex);

	for (int i = 0; i < NESTCLEARMAX; i++){
		if (m_UserData.Status.NestClear[i].nMapIndex <= 0) continue;

		if (m_UserData.Status.NestClear[i].nMapIndex == nMapIndex){
			int nPCBangCount = m_UserData.Status.NestClear[i].cPCBangClearCount;
			if (m_UserData.Status.NestClear[i].cPCBangClearCount > nNestClearCount)
				nPCBangCount = nNestClearCount;

			return m_UserData.Status.NestClear[i].cClearCount + nPCBangCount;
		}
	}

	return 0;
}

TNestClearData *CDNUserBase::GetNestClearCount(int nMapIndex)
{
	for (int i = 0; i < NESTCLEARMAX; i++){
		if (m_UserData.Status.NestClear[i].nMapIndex <= 0) continue;

		if (m_UserData.Status.NestClear[i].nMapIndex == nMapIndex){
			return &(m_UserData.Status.NestClear[i]);
		}
	}

	return NULL;
}

void CDNUserBase::InitNestClear(bool bSend)
{
	m_pDBCon->QueryInitNestClearCount(m_pSession);
	memset(m_UserData.Status.NestClear, 0, sizeof(m_UserData.Status.NestClear));

	if (bSend) SendNestClear(m_UserData.Status.NestClear);
}

void CDNUserBase::InitLadderGradePoint( bool bSend )
{
	g_Log.Log( LogType::_LADDER, m_nWorldSetID, m_nAccountDBID, m_biCharacterDBID, m_nSessionID, L"InitLadderGradePoint() Count=%d\r\n", static_cast<int>(m_PvPLadderScoreInfo.cLadderTypeCount) );

	if( m_PvPLadderScoreInfo.cLadderTypeCount == 0 )
		return;


	int iAddedLadderPoint = m_PvPLadderScoreInfo.GetTopExchangeLadderPoint();

	if( m_PvPLadderScoreInfo.iPvPLadderPoint + iAddedLadderPoint > static_cast<int>(CGlobalWeightTable::GetInstance().GetValue( CGlobalWeightTable::PvPLadder_LimitPoint )) )
		iAddedLadderPoint = static_cast<int>(CGlobalWeightTable::GetInstance().GetValue( CGlobalWeightTable::PvPLadder_LimitPoint )) - m_PvPLadderScoreInfo.iPvPLadderPoint;

	for( int i=0 ; i<m_PvPLadderScoreInfo.cLadderTypeCount ; ++i )
	{
		int iLP = (i==0) ? iAddedLadderPoint : 0;
		GetDBConnection()->QueryInitPvPLadderGradePoint( m_pSession, static_cast<LadderSystem::MatchType::eCode>(m_PvPLadderScoreInfo.LadderScore[i].cPvPLadderCode), iLP, m_PvPLadderScoreInfo.LadderScore[i].iPvPLadderGradePoint );

		m_PvPLadderScoreInfo.LadderScore[i].InitWeeklyGradePoint();
	}

	// Increase ladder points
	m_PvPLadderScoreInfo.iPvPLadderPoint += iAddedLadderPoint;

	if( bSend )
	{
		SendPvPLadderScoreInfo( &m_PvPLadderScoreInfo );
	}
}

void CDNUserBase::SetViewCashEquipBitmap(char cEquipIndex, bool bFlag)
{
	bool bValid = false;
	if( cEquipIndex >= 0 && cEquipIndex < CASHEQUIPMAX ) bValid = true;
	if( cEquipIndex == HIDEHELMET_BITINDEX ) bValid = true;
	if( !bValid ) return;
	SetBitFlag(m_UserData.Status.cViewCashEquipBitmap, cEquipIndex, bFlag);
}

char* CDNUserBase::GetViewCashEquipBitmap()
{
	return m_UserData.Status.cViewCashEquipBitmap;
}

bool CDNUserBase::IsExistViewCashEquipBitmap(char cEquipIndex)
{
	return GetBitFlag(m_UserData.Status.cViewCashEquipBitmap, cEquipIndex);
}

// Cash
void CDNUserBase::AddPetal(int nReserve)
{
	if (nReserve <= 0) return;
	m_UserData.Status.nPetal += nReserve;
	if (m_UserData.Status.nPetal >= 2000000000) m_UserData.Status.nPetal = 2000000000;

	// m_pDBCon->QueryReserve(m_cDBThreadID, m_nWorldSetID, m_nAccountDBID, m_biCharacterDBID, m_CashData.nPetal);
}

void CDNUserBase::DelPetal(int nReserve)
{
	if (nReserve <= 0) return;
	m_UserData.Status.nPetal -= nReserve;
	if (m_UserData.Status.nPetal <= 0) m_UserData.Status.nPetal = 0;
}

void CDNUserBase::SetPetal(int nReserve)
{
	if (nReserve < 0) return;
	m_UserData.Status.nPetal = nReserve;
	if (m_UserData.Status.nPetal >= 2000000000) m_UserData.Status.nPetal = 2000000000;
}

DWORD CDNUserBase::GetTick(eTick pTick) const
{
	DN_ASSERT(CHECK_LIMIT(pTick, eTickCnt),	"Invalid!");

	return(m_dwTick[pTick]);
}

VOID CDNUserBase::SetTick(eTick pTick) const
{
	DN_ASSERT(CHECK_LIMIT(pTick, eTickCnt),	"Invalid!");

	m_dwTick[pTick] = ::GetTickCount();
	m_dwTick[pTick] = (m_dwTick[pTick])?(m_dwTick[pTick]):(1);
}

VOID CDNUserBase::SetTick(eTick pTick, DWORD pTick32) const
{
	DN_ASSERT(CHECK_LIMIT(pTick, eTickCnt),	"Invalid!");
	DN_ASSERT(0 != pTick32,						"Invalid!");

	m_dwTick[pTick] = (pTick32)?(pTick32):(1);
}

VOID CDNUserBase::ResetTick(eTick pTick) const
{
	DN_ASSERT(CHECK_LIMIT(pTick, eTickCnt),	"Invalid!");

	m_dwTick[pTick] = 0;
}

BOOL CDNUserBase::IsTick(eTick pTick) const
{
	DN_ASSERT(CHECK_LIMIT(pTick, eTickCnt),	"Invalid!");

	return(0 != m_dwTick[pTick]);
}

void CDNUserBase::OnScheduleEvent( ScheduleEventEnum ScheduleEvent, ScheduleStruct *pStruct, bool bInitialize )
{
	switch( ScheduleEvent ) {
		case CDNSchedule::ResetFatigue:
			{
				SetDefaultMaxFatigue(!bInitialize);
				ModCommonVariableData(CommonVariable::Type::AddFatigue, 0);
#if defined(PRE_ADD_EXPUP_ITEM)
				ModCommonVariableData(CommonVariable::Type::EXPUPITEM_COUNT, 0);
#endif
#if defined( PRE_ADD_PVP_EXPUP_ITEM )
				ModCommonVariableData(CommonVariable::Type::PvPExpupItem_UseCount, 0);
#endif // #if defined( PRE_ADD_PVP_EXPUP_ITEM )

#if defined(PRE_ADD_VIP)
				// Send VIP mail (let's squeeze handling it in here)
				if ((m_tVIPEndDate != -1) && (!m_bVIPAutoPay)){		// If paid for VIP and not an auto-billing user
					if (m_bVIP){
						__time64_t Time;
						time(&Time);
						__time64_t nGap = m_tVIPEndDate - Time;

						int nMailID = g_pDataManager->GetVIP7DaysLeftMailID();
						if ((nGap > 518400) && (nGap < 691200)){	// If 7 days before service expiration
							if (nMailID > 0)
								CDNMailSender::Process((CDNUserSession*)this, nMailID, 0, 0);
						}

						nMailID = g_pDataManager->GetVIP1DayLeftMailID();
						if ((nGap > 0) && (nGap < 172800)){	// If 1 day before service expiration
							if (nMailID > 0)
								CDNMailSender::Process((CDNUserSession*)this, nMailID, 0, 0);
						}
					}
				}
#endif	// #if defined(PRE_ADD_VIP)
				if( GetHackResetRestraintValue() <= 0 || GetHackAbuseDBValue() < GetHackResetRestraintValue() ) {
					// Reset only once for now.. (auto abusing)
					m_pDBCon->QueryDelAbuseMonitor( m_pSession, 1 );
				}
			}
			break;

		case CDNSchedule::ResetWeeklyFatigue:
		{
			SetDefaultMaxWeeklyFatigue();
			InitNestClear(!bInitialize);
			InitLadderGradePoint( !bInitialize );
			break;
		}
		case CDNSchedule::ResetEventFatigue:
			ResetEventFatigue(pStruct->cHour);
			break;

		case CDNSchedule::ResetRebirthCoin:
			SetDefaultMaxRebirthCoin(!bInitialize);
			break;

		case CDNSchedule::ResetDailyMission:
			{
				time_t Time;
				tm *pTime;

				time(&Time);
				pTime = localtime(&Time);

//				int nRandomSeed = ( pTime->tm_year * 1000 ) +( pTime->tm_mon * 100 );
				int nRandomSeed = GetDateValue( 1900 + pTime->tm_year, pTime->tm_mon + 1, 0 );
				int nValue = (( pTime->tm_mday * 24 ) +( pTime->tm_hour - pStruct->cHour ) ) / 24;
				nRandomSeed += nValue;

				GetMissionSystem()->RequestDailyMissionList( CDNMissionSystem::Daily, nRandomSeed, !bInitialize );
#if defined( PRE_ALTEIAWORLD_EXPLORE )
#if defined( _VILLAGESERVER )
				m_pSession->AlteiaWorldDailyResetInfo();
#endif
#if defined( _GAMESERVER )
				m_pSession->SetAlteiaDailyPlayCount(0);
#endif
				m_pDBCon->QueryResetAlteiaWorldPlayAlteia( m_pSession, AlteiaWorld::ResetType::DailyPlayCount );
				m_pDBCon->QueryResetAlteiaWorldPlayAlteia( m_pSession, AlteiaWorld::ResetType::SendTicketList );
#endif
			}
			break;
		case CDNSchedule::ResetWeeklyMission:
			{
				time_t Time;
				tm *pTime;

				time(&Time);
				pTime = localtime(&Time);

//				int nRandomSeed = ( pTime->tm_year * 1000 ) +( pTime->tm_mon * 100 );
				int nWeek = pStruct->Cycle - CDNSchedule::Sunday;
				int nRandomSeed = ( GetDateValue( 1900 + pTime->tm_year, pTime->tm_mon + 1, 0 ) - 4 - nWeek ) / 7;

				int nValue = GetDateValue( 1900 + pTime->tm_year, pTime->tm_mon + 1, pTime->tm_mday ) - 4 - nWeek;
				int nOffset = nValue % 7;
				int nWeekCount = ( 7 +( pTime->tm_mday - nOffset ) +( nOffset / 7 ) ) / 7;
				if( nOffset == 0 ) {
					if( pTime->tm_hour < pStruct->cHour ) nWeekCount--;
				}
				nRandomSeed += nWeekCount;

				GetMissionSystem()->RequestDailyMissionList( CDNMissionSystem::Weekly, nRandomSeed, !bInitialize );
				GetMissionSystem()->RequestDailyMissionList( CDNMissionSystem::WeekendEvent, nRandomSeed, !bInitialize);
				GetMissionSystem()->RequestDailyMissionList( CDNMissionSystem::WeekendRepeat, nRandomSeed, !bInitialize);
#if defined( PRE_ALTEIAWORLD_EXPLORE )
#if defined( _VILLAGESERVER )
				m_pSession->AlteiaWorldWeeklyResetInfo();
#endif
				m_pDBCon->QueryResetAlteiaWorldPlayAlteia( m_pSession, AlteiaWorld::ResetType::WeeklyPlayCount);				
#endif
			}
			break;

#if defined( PRE_ADD_MONTHLY_MISSION )
		case CDNSchedule::ResetMonthlyMission:
			{
				time_t Time;
				tm *pTime;

				time(&Time);
				pTime = localtime(&Time);
				int nWeek = pStruct->Cycle - CDNSchedule::Sunday;
				int nRandomSeed = ( GetDateValue( 1900 + pTime->tm_year, pTime->tm_mon + 1, pTime->tm_mday) ) / 12;

				GetMissionSystem()->RequestDailyMissionList( CDNMissionSystem::Monthly, nRandomSeed, !bInitialize );
			}
			break;
#endif	// #if defined( PRE_ADD_MONTHLY_MISSION )

		case CDNSchedule::AlarmResetDailyMission:
			{
				SetDefaultMaxDailyPeriodQuest(pStruct);
			}
			break;

		case CDNSchedule::AlarmResetWeeklyMission:
			{
				SetDefaultMaxWeeklyPeriodQuest(pStruct);
			}
			break;

		case CDNSchedule::ResetDailyTimeEvent:
			{
				if( m_pTimeEventSystem )
					m_pTimeEventSystem->ResetDailyTimeEvent();

				if( g_pDataManager->GetEveryDayEventRewardMailID(1) )	//Call function only during attendance event period
					UpdateAttendanceEventData(true);
#if defined( PRE_ADD_LIMITED_SHOP )				
				ResetLimitedShopDailyCount();				
#endif
#if defined( PRE_ADD_TOTAL_LEVEL_SKILL )
				ResetTotalLevelSkillMedalCount();
#endif
			}
			break;
#if defined( PRE_ADD_STAMPSYSTEM )
		case CDNSchedule::ResetWeeklyStamp:
			{
				GetStampSystem()->ResetWeeklyData( pStruct->tDate );
			}
			break;
		case CDNSchedule::AlarmDailyStamp:
			{
				GetStampSystem()->ResetDailyData( pStruct->tDate );
			}
			break;
#endif // #if defined( PRE_ADD_STAMPSYSTEM )
	}
}

bool CDNUserBase::IsAcceptAbleOption(INT64 nCharacterDBID, UINT nAccountDBID, int nCheckType)
{
	BYTE cCheckAcceptableType = _CA_OPTIONTYPE_ACCEPT;

	switch(nCheckType)
	{
	case _ACCEPTABLE_CHECKTYPE_PARTYINVITE: cCheckAcceptableType = GetGameOption()->cPartyInviteAcceptable; break;
	case _ACCEPTABLE_CHECKTYPE_GUILDINVITE: cCheckAcceptableType = GetGameOption()->cGuildInviteAcceptable; break;
	case _ACCEPTABLE_CHECKTYPE_TRADEREQUEST: cCheckAcceptableType = GetGameOption()->cTradeRequestAcceptable; break;
	case _ACCEPTABLE_CHECKTYPE_DUELREQUEST: cCheckAcceptableType = GetGameOption()->cDuelRequestAcceptable; break;
	case _ACCEPTABLE_CHECKTYPE_LADDERINVITE: cCheckAcceptableType = GetGameOption()->cLadderInviteAcceptable; break;
	case _ACCEPTABEL_CHECKTYPE_QUICKPVPINVITE: cCheckAcceptableType = GetGameOption()->cQuickPvPInviteAcceptable; break;
	default: return false;
	}

	if (cCheckAcceptableType == _CA_OPTIONTYPE_DENYALL) return false;
	if (cCheckAcceptableType == _CA_OPTIONTYPE_ACCEPT) return true;
	if (cCheckAcceptableType == _CA_OPTIONTYPE_FRIEND_GUILD_ACCEPT)
	{
		if (m_pFriend->HasFriend(nCharacterDBID)) return true;
		if (nCheckType == _ACCEPTABLE_CHECKTYPE_GUILDINVITE) return false;

		const TGuildUID GuildUID = GetGuildUID();
		if (!GuildUID.IsSet()) return false;

		CDNGuildBase* pGuild = g_pGuildManager->At(GuildUID);
		if (pGuild)
		{
#if !defined( PRE_ADD_NODELETEGUILD )
			CDetachAutoEx<CDNGuildBase>	AutoDetach(pGuild);
			if (FALSE == pGuild->IsEnable()) return false;
#endif

			DN_ASSERT(!pGuild->IsLock(),	"Already Locked!");
			if (pGuild->IsMemberExist(nCharacterDBID, NULL)) return true;
		}
	}

	return false;
}

void CDNUserBase::SetExchangeTargetSessionID(UINT nSessionID)
{
	m_nExchangeTargetSessionID = nSessionID;
	m_nExchangeSenderSID = m_nExchangeReceiverSID = 0;
}

void CDNUserBase::ClearExchangeInfo()
{
	SetExchangeTargetSessionID(0);
	m_pItem->ClearExchangeData();
	IsWindowStateNoneSet(WINDOW_EXCHANGE);
}

bool CDNUserBase::HasGesture(int nGestureID)
{
	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TGESTURE );

	int iUnlockLv = pSox->GetFieldFromLablePtr( nGestureID, "_UnlockLv" )->GetInteger();
	int iGettureType = pSox->GetFieldFromLablePtr( nGestureID, "_GestureType" )->GetInteger();		

	if( iGettureType == Gesture::Type::GuildRewardItem )
	{
		return m_pGesture->HasEffectItemGesture( nGestureID );
	}
	else if( iUnlockLv == GESTURE_UNLOCKLV_CASH )
	{
		return m_pGesture->HasCashGesture( nGestureID );
	}
	else
	{
		if( GetLevel() >= iUnlockLv )
			return true;
	}	

	return false;
}
void CDNUserBase::GetPartyMemberInfo(SPartyMemberInfo &Info)
{
	Info.nSessionID = m_nSessionID;
	Info.nExp = GetExp();
	Info.cLevel = GetLevel();
	memcpy( Info.cJobArray, m_UserData.Status.cJobArray, sizeof(Info.cJobArray) );
	Info.cClassID = GetClassID();
	Info.nMapIndex = GetMapIndex();
	Info.nHairID = (m_pItem->GetEquip(EQUIP_HAIR) == NULL) ? 0 : m_pItem->GetEquip(EQUIP_HAIR)->nItemID;
	Info.nFaceID = (m_pItem->GetEquip(EQUIP_FACE) == NULL) ? 0 : m_pItem->GetEquip(EQUIP_FACE)->nItemID;
	Info.dwHairColor = GetHairColor();
	Info.dwEyeColor = GetEyeColor();
	Info.dwSkinColor = GetSkinColor();
	Info.wFatigue = GetFatigue() + GetWeeklyFatigue() + GetPCBangFatigue() + GetEventFatigue();
	Info.cPvPLevel = m_UserData.PvP.cLevel;
	Info.wStdRebirthCoin = GetRebirthCoin() + GetPCBangRebirthCoin() + GetVIPRebirthCoin();
	Info.wCashRebirthCoin = GetRebirthCashCoin();
	_wcscpy(Info.wszCharacterName, _countof(Info.wszCharacterName), GetCharacterName(), (int)wcslen(GetCharacterName()));

	int nEquipID = 0;
	const TItem* pHelmetEquipItemData = m_pItem->GetEquip(EQUIP_HELMET);
	if (pHelmetEquipItemData != NULL)
		nEquipID = (pHelmetEquipItemData->nLookItemID != 0) ? pHelmetEquipItemData->nLookItemID : pHelmetEquipItemData->nItemID;

	int nCashEquipID = 0;
	const TItem* pCashHelmetEquipItemData = m_pItem->GetCashEquip(CASHEQUIP_HELMET);
	if (pCashHelmetEquipItemData != NULL)
		nCashEquipID = (pCashHelmetEquipItemData->nLookItemID != 0) ? pCashHelmetEquipItemData->nLookItemID : pCashHelmetEquipItemData->nItemID;
	Info.nHelmetID = ( IsExistViewCashEquipBitmap(CASHEQUIP_HELMET) ) ? nCashEquipID : nEquipID;
	if( IsExistViewCashEquipBitmap(HIDEHELMET_BITINDEX) ) Info.nHelmetID = 0;

	nEquipID = (m_pItem->GetEquip(EQUIP_EARRING) == NULL) ? 0 : m_pItem->GetEquip(EQUIP_EARRING)->nItemID;
	nCashEquipID = (m_pItem->GetCashEquip(CASHEQUIP_EARRING) == NULL) ? 0 : m_pItem->GetCashEquip(CASHEQUIP_EARRING)->nItemID;
	Info.nEarringID = ( IsExistViewCashEquipBitmap(CASHEQUIP_EARRING) ) ? nCashEquipID : nEquipID;
	Info.cMemberIndex = m_pSession->GetPartyMemberIndex();
	Info.uiPvPUserState = m_pSession->GetPvPUserState();

#if defined( PRE_WORLDCOMBINE_PARTY )
	Info.nWorldSetID = m_pSession->GetWorldSetID();
#endif
#if defined( PRE_ADD_NEWCOMEBACK )
	if( m_pSession->GetComebackAppellation() > 0 )
		Info.bCheckComebackAppellation = true;	
#endif
}

void CDNUserBase::SetGuildSelfView(const TGuildView& pGuildView)
{
	if ((*(static_cast<TGuildView*>(&m_GuildSelfView))) == pGuildView) {
		return;
	}
	(*(static_cast<TGuildView*>(&m_GuildSelfView))) = pGuildView;

	RefreshGuildSelfView();
}

void CDNUserBase::SetGuildSelfView(const TGuildSelfView& pGuildSelfView)
{
	if (m_GuildSelfView == pGuildSelfView) {
		return;
	}
	m_GuildSelfView = pGuildSelfView;

	RefreshGuildSelfView();
}

void CDNUserBase::ResetGuildSelfView()
{
	if (!m_GuildSelfView.IsSet()) {
		return;
	}
	m_GuildSelfView.Reset();

	RefreshGuildSelfView();
}

void CDNUserBase::RefreshGuildSelfView()
{
	DN_ASSERT(0, "Can't be Called!");
	
}

void CDNUserBase::AddGuildPoint(char cPointType, int nPointValue, int nMissionID)
{
	const TGuildUID GuildUID = GetGuildUID();
	if (!GuildUID.IsSet())
		return;

	CDNGuildBase* pGuild = g_pGuildManager->At(GuildUID);

	if (pGuild)
	{
#if !defined( PRE_ADD_NODELETEGUILD )
		CDetachAutoEx<CDNGuildBase>	AutoDetach(pGuild);
		if (FALSE == pGuild->IsEnable()) return;
#endif

		if (false == pGuild->AddPoint(cPointType, nPointValue, GetCharacterDBID(), nMissionID))
			SendUpdateGuildExp(ERROR_GUILD_DAILYLIMIT, cPointType, 0, 0, 0, 0);
#if defined(PRE_ADD_GUILD_CONTRIBUTION) && defined(_VILLAGESERVER)
		else
			m_pDBCon->QueryGetGuildContributionPoint(m_cDBThreadID, g_Config.nWorldSetID, GetAccountDBID(), GetCharacterDBID());
#endif	// #if defined(PRE_ADD_GUILD_CONTRIBUTION) && defined(_VILLAGESERVER)
	}
}

#ifdef _USE_VOICECHAT
bool CDNUserBase::JoinVoiceChannel(UINT nVoiceChannelID, const char * pIP, TPosition * pPos, int nRotate, bool bForceSet, bool bMutePass)
{
	if (bForceSet) m_bVoiceAvailable = true;
	if (m_bVoiceAvailable == false || pIP == NULL || m_nVoiceChannelID > 0 || g_pVoiceChat == NULL) return false;

	char szName[NAMELENMAX];
	WideCharToMultiByte(CP_ACP, 0, GetCharacterName(), -1, szName, NAMELENMAX, NULL, NULL);
	bool bRet = g_pVoiceChat->JoinChannel(nVoiceChannelID, GetAccountDBID(), szName, pIP, true, m_nVoiceJoinType);
	if (bRet)
	{
		m_nVoiceChannelID = nVoiceChannelID;
		if (m_nVoiceJoinType != _VOICEJOINTYPE_NONE)
		{
			if (pPos) 
			{
				int nX, nY, nZ;
				nX = pPos->nX / 1000;
				nY = pPos->nY / 1000;
				nZ = pPos->nZ / 1000;
				SetVoicePos(nX, nY, nZ, nRotate);
			}
			else
			{
				SetVoicePos(0, 0, 0, 0);
			}

			if (bMutePass == false)
			{
				for (int i = 0; i < PARTYCOUNTMAX; i++)
				{
					if (m_nVoiceMutedList[i] <= 0) continue;
					VoiceMuteOnetoOne(m_nVoiceMutedList[i], true);
				}
			}
		}
		return true;
	}
	return false;
}

void CDNUserBase::LeaveVoiceChannel()
{
	if (m_nVoiceChannelID <= 0 || m_nVoiceJoinType == _VOICEJOINTYPE_NONE || g_pVoiceChat == NULL) return;
	g_pVoiceChat->LeaveChannel(m_nVoiceChannelID, GetAccountDBID());
	m_nVoiceChannelID = 0;
	m_nVoiceJoinType = _VOICEJOINTYPE_NONE;
	memset(m_nVoiceMutedList, 0, sizeof(m_nVoiceMutedList));
}

bool CDNUserBase::IsMutedUser(UINT nMuteID)
{
	for (int i = 0; i < PARTYCOUNTMAX; i++)
		if (m_nVoiceMutedList[i] == nMuteID)
			return true;
	return false;
}

void CDNUserBase::SetVoiceMute(UINT nMuteID, bool bMute)
{
	int i;
	if (bMute)
	{
		if (IsMutedUser(nMuteID))
			return;
		
		for(i = 0; i < PARTYCOUNTMAX; i++)
		{
			if (m_nVoiceMutedList[i] > 0) continue;
			m_nVoiceMutedList[i] = nMuteID;
			return;
		}
	}
	else
	{
		for(i = 0; i < PARTYCOUNTMAX; i++)
		{
			if (m_nVoiceMutedList[i] == nMuteID)
			{
				m_nVoiceMutedList[i] = 0;
				return;
			}
		}
	}
}

void CDNUserBase::MakeMute()
{
	for (int i = 0; i < PARTYCOUNTMAX; i++)
	{
		if (m_nVoiceMutedList[i] <= 0) continue;
		VoiceMuteOnetoOne(m_nVoiceMutedList[i], true);
	}
}

bool CDNUserBase::VoiceMuteOnetoOne(UINT nMuteAccountDBID, bool bMute)
{
	if (g_pVoiceChat == NULL) return false;
	if (g_pVoiceChat->MuteOneToOne(GetAccountDBID(), nMuteAccountDBID, bMute))
	{
		SetVoiceMute(nMuteAccountDBID, bMute);
		return true;
	}
	return false;
}

bool CDNUserBase::VoiceComplaintRequest(UINT nComplaineeAccountDBID, const char * pCategory, const char * pSubject, const char * pMsg)
{
	if (g_pVoiceChat == NULL) return false;
	return g_pVoiceChat->ComplaintRequest(GetAccountDBID(), nComplaineeAccountDBID, nComplaineeAccountDBID, pCategory, pSubject, pMsg);
}

bool CDNUserBase::SetVoicePos(int nX, int nY, int nZ, int nRotate)
{
	if (m_nVoiceChannelID <= 0) return false;
	if (g_pVoiceChat->IsChannelSpartial(m_nVoiceChannelID))
		return g_pVoiceChat->SetUserPos(GetAccountDBID(), nX, nY, nZ, nRotate);
	return false;
}

bool CDNUserBase::IsTalking(BYTE * pTalking)
{
	//Keep the previous talking value and return true if it changed. Compares and returns only 0 vs non-zero values.
	if (g_pVoiceChat == NULL) return false;
	if (m_nVoiceJoinType == _VOICEJOINTYPE_NONE || m_nVoiceJoinType == _VOICEJOINTYPE_LISTENONLY) return false;
	if (g_pVoiceChat->IsUserTalking(GetAccountDBID(), pTalking))
	{
		BYTE cOlder = m_cIsTalking;
		m_cIsTalking = *pTalking;
		if (cOlder > 0 && *pTalking <= 0)
			return true;
		if (cOlder <= 0 && *pTalking > 0)
			return true;
	}
	return false;
}
#endif

void CDNUserBase::PCBangResult(MAPCBangResult *pResult)
{
	SetPCBangGrade(pResult->cPCBangGrade);
	SendPCBang(m_cPCBangGrade, pResult);

}

void CDNUserBase::SetPCBang(bool bPCBang)
{
	m_bPCBang = bPCBang;

	if (m_bPCBang){
		m_cPCBangGrade = PCBang::Grade::Normal;
		m_nPcBangBonusExp = g_pDataManager->GetPCBangParam1(PCBang::Type::Exp, PCBang::Grade::Normal);

	}
}

void CDNUserBase::SetPCBangGrade(char cGrade)
{
	m_cPCBangGrade = cGrade;

	if (m_cPCBangGrade == PCBang::Grade::None)
		m_bPCBang = false;
	else{
		m_bPCBang = true;
		m_nPcBangBonusExp = g_pDataManager->GetPCBangParam1(PCBang::Type::Exp, GetPCBangGrade());

		GetEventSystem()->OnEvent(EventSystem::OnPCBang_Item);
		GetEventSystem()->OnEvent(EventSystem::OnPCBang_Fatigue);
		GetEventSystem()->OnEvent(EventSystem::OnPCBang_RebirthCoin);
	}
}

void CDNUserBase::RefreshPCBangMissionList()
{
	int nRebirthCoin = g_pDataManager->GetPCBangParam1(PCBang::Type::RebirthCoin, PCBang::Grade::Normal);
	if (nRebirthCoin == 0)
		m_UserData.Status.cPCBangRebirthCoin = 0;

	int nFatigue = g_pDataManager->GetPCBangParam1(PCBang::Type::Fatigue, PCBang::Grade::Normal);
	if (nFatigue == 0)
		m_UserData.Status.wPCBangFatigue = 0;

	memset(m_UserData.Mission.PCBangMission, 0, sizeof(m_UserData.Mission.PCBangMission));

	// Fatigue
	m_UserData.Mission.PCBangMission[0].nMissionID = g_pDataManager->GetPCBangParam1(PCBang::Type::MissionFatigue, PCBang::Grade::Gold);		// Gold Premium PC Bang fatigue reward - fixed 600002
	m_UserData.Mission.PCBangMission[1].nMissionID = g_pDataManager->GetPCBangParam1(PCBang::Type::MissionFatigue, PCBang::Grade::Silver);		// Silver Premium PC bang fatigue reward - fixed 600003
	m_UserData.Mission.PCBangMission[2].nMissionID = g_pDataManager->GetPCBangParam1(PCBang::Type::MissionFatigue, PCBang::Grade::Red);			// Red Premium PC Bang fatigue reward - fixed 600004
	m_UserData.Mission.PCBangMission[3].nMissionID = g_pDataManager->GetPCBangParam1(PCBang::Type::MissionFatigue, PCBang::Grade::Premium);		// Premium PC bang fatigue reward - fixed 600010
	// Coin
	m_UserData.Mission.PCBangMission[4].nMissionID = g_pDataManager->GetPCBangParam1(PCBang::Type::MissionRebirthCoin, PCBang::Grade::Gold);	// Gold Premium PC Bang coin reward - fixed 600006
	m_UserData.Mission.PCBangMission[5].nMissionID = g_pDataManager->GetPCBangParam1(PCBang::Type::MissionRebirthCoin, PCBang::Grade::Silver);	// Silver Premium PC bang coin reward - fixed 600007
	m_UserData.Mission.PCBangMission[6].nMissionID = g_pDataManager->GetPCBangParam1(PCBang::Type::MissionRebirthCoin, PCBang::Grade::Red);		// Red Premium PC Bang coin reward - fixed 600008
	m_UserData.Mission.PCBangMission[7].nMissionID = g_pDataManager->GetPCBangParam1(PCBang::Type::MissionRebirthCoin, PCBang::Grade::Premium);	// Premium PC bang coin reward - fixed 600011

	m_UserData.Mission.PCBangMission[8].nMissionID = g_pDataManager->GetPCBangParam1(PCBang::Type::MissionReward, PCBang::Grade::Gold);
	m_UserData.Mission.PCBangMission[9].nMissionID = g_pDataManager->GetPCBangParam1(PCBang::Type::MissionReward, PCBang::Grade::Silver);
	m_UserData.Mission.PCBangMission[10].nMissionID = g_pDataManager->GetPCBangParam1(PCBang::Type::MissionReward, PCBang::Grade::Red);
	m_UserData.Mission.PCBangMission[11].nMissionID = g_pDataManager->GetPCBangParam1(PCBang::Type::MissionReward, PCBang::Grade::Premium);
	m_UserData.Mission.PCBangMission[12].nMissionID = g_pDataManager->GetPCBangParam1(PCBang::Type::MissionRewardCash, PCBang::Grade::Gold);
	m_UserData.Mission.PCBangMission[13].nMissionID = g_pDataManager->GetPCBangParam1(PCBang::Type::MissionRewardCash, PCBang::Grade::Silver);
	m_UserData.Mission.PCBangMission[14].nMissionID = g_pDataManager->GetPCBangParam1(PCBang::Type::MissionRewardCash, PCBang::Grade::Red);
	m_UserData.Mission.PCBangMission[15].nMissionID = g_pDataManager->GetPCBangParam1(PCBang::Type::MissionRewardCash, PCBang::Grade::Premium);
}

void CDNUserBase::SetVIPTotalPoint(int nPoint)
{
	if (nPoint < 0) return;

	m_nVIPTotalPoint = nPoint;
	if (m_nVIPTotalPoint > 10000) m_nVIPTotalPoint = 10000;
}

void CDNUserBase::SetCashBalance(int nBalance)
{
	if (nBalance < 0) return;
	m_nCashBalance = nBalance;
}

void CDNUserBase::DelCashBalance(int nCash)
{
	if (nCash <= 0) return;
	m_nCashBalance -= nCash;
	if (m_nCashBalance < 0) m_nCashBalance = 0;
}

void CDNUserBase::DelCashByPaymentRules(int nBalance, char cPaymentRules)
{
	switch(cPaymentRules)
	{
	case Cash::PaymentRules::None:
		DelCashBalance(nBalance);
		break;

	}
}

#if defined(PRE_ADD_CASH_REFUND)
void CDNUserBase::AddCashBalance(int nCash)
{
	if ( nCash < 0) return;
	m_nCashBalance += nCash;
}
#endif

bool CDNUserBase::CheckEnoughCashByPaymentRules(int nBalance, char cPaymentRules)
{
	switch(cPaymentRules)
	{
	case Cash::PaymentRules::None:
		if (!CheckEnoughCashBalance(nBalance)) return false;
		break;

	}

	return true;
}

bool CDNUserBase::CheckEnoughCashBalance(int nBalance)
{
	if (nBalance > GetCashBalance()) return false;
	return true;
}

bool CDNUserBase::CheckEnoughPetal(int nPetal)
{
	if (GetPetal() < nPetal) return false;
	return true;
}


bool CDNUserBase::IsValidPacketByNpcClick()
{
	EtVector2 vVec( 0.f , 0.f );

#if defined(_VILLAGESERVER)
	if (m_nClickedNpcObjectID <= 0) return false;
	CDNNpcObject* pNpc = g_pFieldManager->GetNpcObjectByNpcObjID(GetChannelID(), m_nClickedNpcObjectID);
	if (!pNpc) return false;

	if (pNpc->GetChannelID() != GetChannelID()) return false;

	vVec.x = (pNpc->GetCurrentPos().nX - m_pSession->GetCurrentPos().nX) / 1000.0f;
	vVec.y = (pNpc->GetCurrentPos().nZ - m_pSession->GetCurrentPos().nZ) / 1000.0f;

#elif defined(_GAMESERVER)
	DnActorHandle hNPC = CDnActor::FindActorFromUniqueID(m_pSession->GetGameRoom(), m_nClickedNpcObjectID);
	if (!hNPC) return false;

	vVec.x = hNPC->GetPosition()->x - m_pSession->GetPlayerActor()->GetPosition()->x;
	vVec.y = hNPC->GetPosition()->y - m_pSession->GetPlayerActor()->GetPosition()->y;

#endif

	float fLength = EtVec2Length(&vVec);

	if ((fLength > 1.f) && (fLength < 2000.f)){
		return true;
	}

	//g_Log.Log(LogType::_FILELOG, L"Length:%f\r\n", fLength);

	return false;
}

void CDNUserBase::MakeGiftByQuest(int nQuestID, int *nRewardItemList)
{
	wstring wStr = FormatW(L"%d", nQuestID);
	m_pDBCon->QueryMakeGiftByQuest(m_pSession, nQuestID, (WCHAR*)wStr.c_str(), nRewardItemList, true);
}

int CDNUserBase::_CmdSetQuestNotifier( const CSGameOptionQuestNotifier* pPacket )
{
	for( UINT i=0 ; i<_countof(pPacket->data) ; ++i )
	{
		bool	bRet		= false;
		int		iQuestIndex = pPacket->data[i].iIndex;

		if( pPacket->data[i].eType == DNNotifier::Type::MainQuest )
		{
			if( iQuestIndex >= 0 && !g_pQuestManager->bIsMainQuest( iQuestIndex ) )	// Check MainQuest
			{
				_DANGER_POINT();
				return ERROR_GENERIC_INVALIDREQUEST;
			}
			if( i >(DNNotifier::RegisterCount::MainQuest-1) )				// Check Index
			{
				_DANGER_POINT();
				return ERROR_GENERIC_INVALIDREQUEST;
			}

			bRet = m_NotifierRepository.RegisterMainQuest( iQuestIndex );
		}
		else if( pPacket->data[i].eType == DNNotifier::Type::SubQuest )
		{
			if( iQuestIndex >= 0 && !g_pQuestManager->bIsSubQuest( iQuestIndex ) )	// Check SubQuest
			{
				_DANGER_POINT();
				return ERROR_GENERIC_INVALIDREQUEST;
			}
			if( i < DNNotifier::RegisterCount::MainQuest )					// Check Index
			{
				_DANGER_POINT();
				return ERROR_GENERIC_INVALIDREQUEST;
			}

			bRet = m_NotifierRepository.RegisterSubQuest( i, iQuestIndex );
		}
		// Deselected item
		else if( pPacket->data[i].eType == DNNotifier::Type::Max )
		{
			bRet = true;
		}

		if( bRet )
		{
			m_UserData.Status.NotifierData[i].eType	= pPacket->data[i].eType;
			m_UserData.Status.NotifierData[i].iIndex	= iQuestIndex;

			m_pDBCon->QueryNotifier( m_pSession, i, pPacket->data[i].eType, iQuestIndex );
		}
	}

	return ERROR_NONE;
}

int CDNUserBase::_CmdSetMissionNotifier( const CSGameOptionMissionNotifier* pPacket )
{
	for( UINT i=0 ; i<_countof(pPacket->data) ; ++i )
	{
		bool	bRet			= false;
		int		iMissionIndex	= pPacket->data[i].iIndex;

#if defined(PRE_MOD_MISSION_HELPER)
		// Deselected item
		if( pPacket->data[i].eType == DNNotifier::Type::Max )
		{
			bRet = true;
		}
		else if(iMissionIndex >= 0)
		{	
			if(!m_pMissionSystem->bCheckDailyMission(iMissionIndex, pPacket->data[i].eType))
			{
				_DANGER_POINT();
				return ERROR_GENERIC_INVALIDREQUEST;
			}

			bRet = m_NotifierRepository.RegisterMission( DNNotifier::RegisterCount::TotalQuest+i ,iMissionIndex, pPacket->data[i].eType );
		}
#else
		if( pPacket->data[i].eType == DNNotifier::Type::DailyMission )
		{
			if( iMissionIndex >= 0 && !m_pMissionSystem->bIsDailyMission( iMissionIndex ) )		// Check DailyMission
			{
				_DANGER_POINT();
				return ERROR_GENERIC_INVALIDREQUEST;
			}
			if( i > DNNotifier::RegisterCount::DailyMission )				// Check Index
			{
				_DANGER_POINT();
				return ERROR_GENERIC_INVALIDREQUEST;
			}

			bRet = m_NotifierRepository.RegisterDailyMission( iMissionIndex );
		}
		else if( pPacket->data[i].eType == DNNotifier::Type::WeeklyMission )
		{			
			if( iMissionIndex >= 0 && !m_pMissionSystem->bIsWeeklyMission( iMissionIndex ) )		// Check WeeklyMission
			{
				_DANGER_POINT();
				return ERROR_GENERIC_INVALIDREQUEST;
			}
			if( i < DNNotifier::RegisterCount::DailyMission )				// Check Index
			{
				_DANGER_POINT();
				return ERROR_GENERIC_INVALIDREQUEST;
			}

			bRet = m_NotifierRepository.RegisterWeeklyMission( iMissionIndex );
		}
		// Deselected item
		else if( pPacket->data[i].eType == DNNotifier::Type::Max )
		{
			bRet = true;
		}
#endif

		if( bRet )
		{
			int iIndex = i+DNNotifier::RegisterCount::MainQuest+DNNotifier::RegisterCount::SubQuest;
			m_UserData.Status.NotifierData[iIndex].eType		= pPacket->data[i].eType;
			m_UserData.Status.NotifierData[iIndex].iIndex	= iMissionIndex;

			m_pDBCon->QueryNotifier( m_pSession, iIndex, pPacket->data[i].eType, iMissionIndex );
		}
	}

	return ERROR_NONE;
}

int CDNUserBase::DelFatigue(int nGap)
{
	if (m_UserData.Status.wFatigue == 0) return m_UserData.Status.wFatigue;

	m_UserData.Status.wFatigue -= nGap;
	if (m_UserData.Status.wFatigue < 0) m_UserData.Status.wFatigue = 0;

	GetEventSystem()->OnEvent( EventSystem::OnDailyFatigue);

	m_pDBCon->QueryFatigue(m_pSession, DBDNWorldDef::FatigueTypeCode::Daily, m_UserData.Status.wFatigue);
	return m_UserData.Status.wFatigue;
}

int CDNUserBase::DelWeeklyFatigue(int nGap)
{
	if (m_UserData.Status.wWeeklyFatigue == 0) return m_UserData.Status.wWeeklyFatigue;

	m_UserData.Status.wWeeklyFatigue -= nGap;
	if (m_UserData.Status.wWeeklyFatigue < 0) m_UserData.Status.wWeeklyFatigue = 0;

	GetEventSystem()->OnEvent( EventSystem::OnWeeklyFatigue);

	m_pDBCon->QueryFatigue(m_pSession, DBDNWorldDef::FatigueTypeCode::Weekly, m_UserData.Status.wWeeklyFatigue);
	return m_UserData.Status.wWeeklyFatigue;
}

int CDNUserBase::DelPCBangFatigue(int nGap)
{
	if (m_UserData.Status.wPCBangFatigue == 0) return m_UserData.Status.wPCBangFatigue;

	m_UserData.Status.wPCBangFatigue -= nGap;
	if (m_UserData.Status.wPCBangFatigue < 0) m_UserData.Status.wPCBangFatigue = 0;

	GetEventSystem()->OnEvent( EventSystem::OnPCBangFatigue);

	m_pDBCon->QueryFatigue(m_pSession, DBDNWorldDef::FatigueTypeCode::PCBang, m_UserData.Status.wPCBangFatigue);
	return m_UserData.Status.wPCBangFatigue;
}

int CDNUserBase::DelEventFatigue(int nGap)
{
	if (m_UserData.Status.wEventFatigue == 0) return m_UserData.Status.wEventFatigue;

	m_UserData.Status.wEventFatigue -= nGap;
	if (m_UserData.Status.wEventFatigue < 0) m_UserData.Status.wEventFatigue = 0;

	m_pDBCon->QueryEventFatigue(m_pSession, m_UserData.Status.wEventFatigue, false, 0);
	return m_UserData.Status.wEventFatigue;
}

int CDNUserBase::DelVIPFatigue(int nGap)
{
	if (m_UserData.Status.wVIPFatigue == 0) return m_UserData.Status.wVIPFatigue;

	m_UserData.Status.wVIPFatigue -= nGap;
	if (m_UserData.Status.wVIPFatigue < 0) m_UserData.Status.wVIPFatigue = 0;

#if defined(PRE_ADD_VIP)
	GetEventSystem()->OnEvent( EventSystem::OnVIPFatigue);
#endif

	m_pDBCon->QueryFatigue(m_pSession, DBDNWorldDef::FatigueTypeCode::VIP, m_UserData.Status.wVIPFatigue);
	return m_UserData.Status.wVIPFatigue;
}

int CDNUserBase::DelRebirthCoin(int nGap)
{
	if (m_UserData.Status.cRebirthCoin == 0) return m_UserData.Status.cRebirthCoin;

	if( m_UserData.Status.cRebirthCoin > nGap )
		m_UserData.Status.cRebirthCoin -= nGap;
	else
		m_UserData.Status.cRebirthCoin = 0;

	return GetRebirthCoin();
}

int CDNUserBase::DelPCBangRebirthCoin(int nGap)
{
	if (m_UserData.Status.cPCBangRebirthCoin == 0) return m_UserData.Status.cPCBangRebirthCoin;

	if (m_UserData.Status.cPCBangRebirthCoin > nGap)
		m_UserData.Status.cPCBangRebirthCoin -= nGap;
	else
		m_UserData.Status.cPCBangRebirthCoin = 0;

	return GetPCBangRebirthCoin();
}

int CDNUserBase::DelVIPRebirthCoin(int nGap)
{
	if (m_cVIPRebirthCoin == 0) return m_cVIPRebirthCoin;

	if (m_cVIPRebirthCoin > nGap)
		m_cVIPRebirthCoin -= nGap;
	else
		m_cVIPRebirthCoin = 0;

	return GetVIPRebirthCoin();
}

int CDNUserBase::DelCashRebirthCoin(int nGap)
{
	if (m_UserData.Status.wCashRebirthCoin == 0) return m_UserData.Status.wCashRebirthCoin;

	if (m_UserData.Status.wCashRebirthCoin > nGap)
		m_UserData.Status.wCashRebirthCoin -= nGap;
	else
		m_UserData.Status.wCashRebirthCoin = 0;

	m_pDBCon->QueryCashRebirthCoin(m_pSession);

	return GetRebirthCashCoin();
}

bool CDNUserBase::CheckPapering( eCheckPapering eType )
{
	DWORD dwCurTime = timeGetTime();
	DWORD dwCheckTime = 0;
	DWORD dwRestrictionTime = 0;
	int nCheckCount = 0;

	switch( eType )
	{
	case eChatPapering:
		{
			dwCheckTime = CHAT_PAPERING_CHECKTIME*1000;
			dwRestrictionTime = CHAT_PAPERING_RESTRICTIONTIME*1000;
			nCheckCount = CHAT_PAPERING_CHECKCOUNT;
		}
		break;
	case eGesturePapering:
		{
			dwCheckTime = GESTURE_PAPERING_CHECKTIME*1000;
			dwRestrictionTime = GESTURE_PAPERING_RESTRICTIONTIME*1000;
			nCheckCount = GESTURE_PAPERING_CHECKCOUNT;
		}
		break;
	}

	// First clean up expired ChatTime whenever called
	std::list<DWORD>::iterator iter = m_listRecentCheckTime[eType].begin();
	while( iter != m_listRecentCheckTime[eType].end() )
	{
		DWORD dwTime = *iter;
		if( dwCurTime > dwTime + dwCheckTime )
		{
			iter = m_listRecentCheckTime[eType].erase( iter );
			continue;
		}
		++iter;
	}

	// Check if currently under sanction
	if( m_dwCheckPaperingRemainTime[eType] )
	{
		if( dwCurTime < m_dwCheckPaperingRemainTime[eType] + dwRestrictionTime )
			return true;
		else
			m_dwCheckPaperingRemainTime[eType] = 0;
	}

	// Check count limit
	if((int)m_listRecentCheckTime[eType].size()+1 >= nCheckCount )
		m_dwCheckPaperingRemainTime[eType] = dwCurTime;

	m_listRecentCheckTime[eType].push_back( dwCurTime );

	return false;
}

#if defined( PRE_ADD_NPC_REPUTATION_SYSTEM )

void CDNUserBase::OnDBRecvReputation(int nSubCmd, char * pData)
{
	switch(nSubCmd)
	{
		case QUERT_GET_LISTNPCFAVOR:
		{
			TAGetListNpcFavor* pPacket = reinterpret_cast<TAGetListNpcFavor*>(pData);
			LoadReputation( pPacket );
			break;
		}
	}
}

#endif // #if defined( PRE_ADD_NPC_REPUTATION_SYSTEM )

void CDNUserBase::OnDBRecvMasterSystem(int nSubCmd, char* pData)
{
	switch( nSubCmd )
	{
		case QUERY_GET_SIMPLEINFO:
		{
			TAGetMasterPupilInfo* pA = reinterpret_cast<TAGetMasterPupilInfo*>(pData);

			if( pA->nRetCode == ERROR_NONE )
			{
				memset( &m_MasterSystemData, 0, sizeof(m_MasterSystemData) );
				m_MasterSystemData.SimpleInfo.iMasterCount			= pA->SimpleInfo.iMasterCount;
				m_MasterSystemData.SimpleInfo.iPupilCount			= pA->SimpleInfo.iPupilCount;
				m_MasterSystemData.SimpleInfo.iGraduateCount		= pA->SimpleInfo.iGraduateCount;
				m_MasterSystemData.SimpleInfo.BlockDate				= pA->SimpleInfo.BlockDate;
				m_MasterSystemData.SimpleInfo.cCharacterDBIDCount	= pA->SimpleInfo.cCharacterDBIDCount;
				for( UINT i=0 ; i<pA->SimpleInfo.cCharacterDBIDCount ; ++i )
				{
					m_MasterSystemData.SimpleInfo.OppositeInfo[i].CharacterDBID = pA->SimpleInfo.OppositeInfo[i].CharacterDBID;
					m_MasterSystemData.SimpleInfo.OppositeInfo[i].nFavorPoint = pA->SimpleInfo.OppositeInfo[i].nFavorPoint;
					_wcscpy( m_MasterSystemData.SimpleInfo.OppositeInfo[i].wszCharName, _countof(m_MasterSystemData.SimpleInfo.OppositeInfo[i].wszCharName), 
						pA->SimpleInfo.OppositeInfo[i].wszCharName, (int)wcslen(pA->SimpleInfo.OppositeInfo[i].wszCharName) );
				}

				if( pA->bClientSend == true )
				{
					SendMasterSystemSimpleInfo( m_MasterSystemData.SimpleInfo );
				}

				OnMasterSystemEvent( pA->EventCode );
			}
			else
			{
				_DANGER_POINT();
			}
			break;
		}
		case QUERY_GET_COUNTINFO:
		{
			TAGetMasterSystemCountInfo* pA = reinterpret_cast<TAGetMasterSystemCountInfo*>(pData);
			if( pA->bClientSend )
				SendMasterSystemCountInfo( pA->iMasterCount, pA->iPupilCount, pA->iClassmateCount );
			return;
		}
		case QUERY_GRADUATE:
		{
			TAGraduate* pA = reinterpret_cast<TAGraduate*>(pData);
			SendMasterSystemGraduate( pA->wszPupilCharName );
			break;
		}
	}
}

void CDNUserBase::OnMasterSystemEvent( MasterSystem::EventType::eCode EventCode )
{
	switch( EventCode )
	{
		// Check Event when Offline
		case MasterSystem::EventType::VillageFirstConnect:
		{
			GetEventSystem()->OnEvent( EventSystem::OnMasterSystemJoin );
			GetEventSystem()->OnEvent( EventSystem::OnMasterSystemGraduated );
			break;
		}
		case MasterSystem::EventType::Join:
		{
			GetEventSystem()->OnEvent( EventSystem::OnMasterSystemJoin );
			break;
		}
		case MasterSystem::EventType::Graduate:
		{
			GetEventSystem()->OnEvent( EventSystem::OnMasterSystemGraduate );
			break;		
		}
		case MasterSystem::EventType::Graduated:
		{
			GetEventSystem()->OnEvent( EventSystem::OnMasterSystemGraduated );
			break;		
		}
	}
}

bool CDNUserBase::bIsMasterApplicationPenalty()
{
#if !defined( _FINAL_BUILD )
#if defined( _VILLAGESERVER )
	CDNUserSession* pSession = static_cast<CDNUserSession*>(this);
	if( pSession->m_bIsMasterSystemSkipDate )
		return false;
#endif // #if defined( _VILLAGESERVER )
#endif // #if !defined( _FINAL_BUILD )

	CTimeSet CurTime;

	/*
	if( CurTime.GetTimeT64_LC()-m_MasterSystemData.SimpleInfo.BlockDate < MasterSystem::Penalty::PupilLeavePenaltySec )
		return true;
	*/
	
	// If BlockDate has passed, not in penalty period
	if( CurTime.GetTimeT64_LC() >= m_MasterSystemData.SimpleInfo.BlockDate )
		return false;
	return true;
}

#if defined( PRE_ADD_SECONDARY_SKILL )

void CDNUserBase::OnDBRecvSecondarySkill(int nSubCmd, char* pData)
{
	switch( nSubCmd )
	{
		case QUERY_GETLIST_SECONDARYSKILL:
		{
			TAGetListSecondarySkill* pA = reinterpret_cast<TAGetListSecondarySkill*>(pData);

			if( pA->nRetCode == ERROR_NONE )
			{
				for( UINT i=0 ; i<pA->cCount ; ++i )
				{
					if( m_pSecondarySkillRepository->Add( pA->SkillList[i].iSkillID, pA->SkillList[i].iExp ) == false )
					{
						_ASSERT(0);
						g_Log.Log( LogType::_ERROR, m_pSession, L"SecondarySkill|[ADBID:%u, CDBID:%I64d, SID:%u] AddRecipe Failed! SkillID:%d Exp:%d", GetAccountDBID(), GetCharacterDBID(), m_pSession->GetSessionID(), pA->SkillList[i].iSkillID, pA->SkillList[i].iExp );
						m_pSession->DetachConnection(L"AddSecondarySkill Failed!");				
						return;
					}
				}
			}
			else
			{
				_ASSERT(0);
				g_Log.Log( LogType::_ERROR, m_pSession, L"SecondarySkill|[ADBID:%u, CDBID:%I64d, SID:%u] QUERY_GETLIST_SECONDARYSKILL Ret:%d", GetAccountDBID(), GetCharacterDBID(), m_pSession->GetSessionID(), pA->nRetCode );
				m_pSession->DetachConnection(L"QUERY_GETLIST_SECONDARYSKILL Failed!");				
			}

			break;
		}
		case QUERY_GETLIST_SECONDARYSKILL_RECIPE:
		{
			TAGetListManufactureSkillRecipe* pA = reinterpret_cast<TAGetListManufactureSkillRecipe*>(pData);

			if( pA->nRetCode == ERROR_NONE )
			{
				_ASSERT( dynamic_cast<CSecondarySkillRepositoryServer*>(m_pSecondarySkillRepository) );

				for( UINT i=0 ; i<pA->cCount ; ++i )
				{
					if( static_cast<CSecondarySkillRepositoryServer*>(m_pSecondarySkillRepository)->AddRecipe( &pA->RecipeList[i] ) == false )
					{
						_ASSERT(0);
						g_Log.Log( LogType::_ERROR, m_pSession, L"SecondarySkillRecipe|[ADBID:%u, CDBID:%I64d, SID:%u] AddRecipe Failed! SkillID:%d ItemID:%d", GetAccountDBID(), GetCharacterDBID(), m_pSession->GetSessionID(), pA->RecipeList[i].iSkillID, pA->RecipeList[i].iItemID );
						m_pSession->DetachConnection(L"AddRecipe Failed!");				
						return;
					}
				}
			}
			else
			{
				_ASSERT(0);
				g_Log.Log( LogType::_ERROR, m_pSession, L"SecondarySkillRecipe|[ADBID:%u, CDBID:%I64d, SID:%u] QUERY_GETLIST_SECONDARYSKILL_RECIPE Ret:%d", GetAccountDBID(), GetCharacterDBID(), m_pSession->GetSessionID(), pA->nRetCode );
				m_pSession->DetachConnection(L"QUERY_GETLIST_SECONDARYSKILL_RECIPE Failed!");				
			}

			break;
		}
	}
}

#endif // #if defined( PRE_ADD_SECONDARY_SKILL )


void CDNUserBase::OnDBRecvFarm(int nSubCmd, char * pData)
{
	switch(nSubCmd)
	{
		case QUERY_GETCOUNT_HARVESTDEPOTITEM:
		{
			TAGetCountHarvestDepotItem* pPacket = reinterpret_cast<TAGetCountHarvestDepotItem*>(pData);
			if (pPacket->nRetCode == ERROR_NONE )
			{ 
				SendFarmWareHouseItemCount( pPacket->iCount );
			}
		}
		break;
	}
}

void CDNUserBase::OnDBRecvGuildRecruit(int nSubCmd, char * pData)
{
	return;
}

#if defined( PRE_PARTY_DB )
void CDNUserBase::OnDBRecvParty( int nSubCmd, char* pData )
{
}
#endif // #if defined( PRE_PARTY_DB )

#if defined (PRE_ADD_BESTFRIEND)
void CDNUserBase::OnDBRecvBestFriend(int nSubCmd, char* pData)
{
	switch( nSubCmd )
	{
	case QUERY_GET_BESTFRIEND:
		{
			const TAGetBestFriend *pPacket = reinterpret_cast<TAGetBestFriend*>(pData);
			if (ERROR_NONE != pPacket->nRetCode) 
				break;

			m_pBestFriend->LoadData(pPacket);
		}
		break;
	default:
		break;
	}
}
#endif

#if defined( PRE_PRIVATECHAT_CHANNEL )
void CDNUserBase::OnDBRecvPrivateChatChannel(int nSubCmd, char* pData)
{
	switch( nSubCmd )
	{
	case QUERY_GET_PRIVATECHATCHANNELMEMBER:
		{
			const TAPrivateChatChannelMember* pPacket = reinterpret_cast<const TAPrivateChatChannelMember*>(pData);			

			if (ERROR_NONE != pPacket->nRetCode)
			{
				SetPrivateChannelID(0);
				SendPrivateChatChannelResult(pPacket->nRetCode);
				return;
			}
			if(g_pPrivateChatChannelManager)
			{
				CDNPrivateChaChannel* pPrivateChatChannel = g_pPrivateChatChannelManager->GetPrivateChannelInfo( pPacket->cWorldSetID, pPacket->nPrivateChatChannelID);
				if(pPrivateChatChannel)
				{					
					SetPrivateChannelID(pPacket->nPrivateChatChannelID);					
					if( pPrivateChatChannel->CheckPrivateChannelMember(GetCharacterDBID()) )
					{						
						SendPrivateChatChannel(pPrivateChatChannel);
						SetPrivateChannelID(pPacket->nPrivateChatChannelID);
						pPrivateChatChannel->SetPrivateChannelMemberAccountDBID(GetAccountDBID(), GetCharacterDBID());
						m_pDBCon->QueryModPrivateMemberServerID( m_pSession );
					}
					else
						m_pDBCon->QueryAddPrivateChatChannelMember( m_pSession, pPacket->nPrivateChatChannelID, PrivateChatChannel::Common::EnterMember );
				}				
			}			
		}
		break;
	case QUERY_CREATE_PRIVATECHATCHANNEL:
		{
			const TAAddPrivateChatChannel *pPacket = reinterpret_cast<TAAddPrivateChatChannel*>(pData);
			if (ERROR_NONE != pPacket->nRetCode)
			{
				SetPrivateChannelID(0);
				SendPrivateChatChannelResult(ERROR_ALREADY_CHANNELNAME);
				return;
			}			

			CDNPrivateChaChannel* pPrivateChatChannel = g_pPrivateChatChannelManager->GetPrivateChannelInfo( pPacket->cWorldSetID, pPacket->tPrivateChatChannel.nPrivateChatChannelID);
			if(pPrivateChatChannel)
			{				
				SendPrivateChatChannelJoinResult(pPacket->nRetCode, pPrivateChatChannel->GetChannelName());				
				SendPrivateChatChannel(pPrivateChatChannel);
				SetPrivateChannelID(pPacket->tPrivateChatChannel.nPrivateChatChannelID);				
				GetDBConnection()->QueryModPrivateMemberServerID( m_pSession );
			}
		}
		break;
	case QUERY_ADD_PRIVATECHATMEMBER:
		{
			const TAAddPrivateChatMember *pPacket = reinterpret_cast<TAAddPrivateChatMember*>(pData);
			if (ERROR_NONE != pPacket->nRetCode && pPacket->nRetCode != 103345)	// 103345 occurs when the DB server is down, so resend the member list
			{
				switch(pPacket->nRetCode)
				{
				case 103329:
					{
						SendPrivateChatChannelResult(ERROR_NOTEXIST_CHANNELNAME);
					}
					break;
				case 103347:
					{
						SendPrivateChatChannelResult(ERROR_MAX_CHANNELMEMBER);
					}
					break;
				}
				SetPrivateChannelID(0);				
				return;
			}			

			CDNPrivateChaChannel* pPrivateChatChannel = g_pPrivateChatChannelManager->GetPrivateChannelInfo( pPacket->cWorldSetID, pPacket->nPrivateChatChannelID);

			if(pPrivateChatChannel)
			{
				SendPrivateChatChannelJoinResult(pPacket->nRetCode, pPrivateChatChannel->GetChannelName());				
				SendPrivateChatChannel(pPrivateChatChannel);
				SetPrivateChannelID(pPacket->nPrivateChatChannelID);
				if( GetCharacterDBID() == pPrivateChatChannel->GetMasterCharacterDBID() )
					m_pDBCon->QueryModPrivateChatChannelInfo( m_pSession, pPrivateChatChannel->GetChannelName(), PrivateChatChannel::Common::ChangeMaster, pPrivateChatChannel->GetChannelPassword(), GetCharacterDBID() );
				GetDBConnection()->QueryModPrivateMemberServerID( m_pSession );
			}
		}
		break;	
	case QUERY_DEL_PRIVATECHATMEMBER:
		{
			const TADelPrivateChatMember *pPacket = reinterpret_cast<TADelPrivateChatMember*>(pData);
			if (ERROR_NONE != pPacket->nRetCode)
			{
				SetPrivateChannelID(0);
				SendPrivateChatChannelResult(pPacket->nRetCode);
				return;
			}			

			CDNPrivateChaChannel* pPrivateChatChannel = g_pPrivateChatChannelManager->GetPrivateChannelInfo(pPacket->cWorldSetID, pPacket->nPrivateChatChannelID);

			if(pPrivateChatChannel)
			{					
				pPrivateChatChannel->DelPrivateChannelMember(PrivateChatChannel::Common::OutMember, pPacket->biCharacterDBID );				
				SendPrivateChatChannelOutResult(pPacket->nRetCode);
			}
		}
		break;
	case QUERY_KICK_PRIVATECHATMEMBER:
		{
			const TADelPrivateChatMember *pPacket = reinterpret_cast<TADelPrivateChatMember*>(pData);
			if (ERROR_NONE != pPacket->nRetCode)
			{
				SetPrivateChannelID(0);
				SendPrivateChatChannelResult(pPacket->nRetCode);
				return;
			}
		}
		break;
	case QUERY_MOD_PRIVATECHATCHANNELINFO:
		{
			const TAModPrivateChatChannelInfo *pPacket = reinterpret_cast<TAModPrivateChatChannelInfo*>(pData);
			if (ERROR_NONE != pPacket->nRetCode)
			{
				SetPrivateChannelID(0);
				SendPrivateChatChannelResult(pPacket->nRetCode);
				return;
			}			
		}
		break;
	case QUERY_INVITE_PRIVATECHATMEMBER:
		{
			const TAInvitePrivateChatMember *pPacket = reinterpret_cast<TAInvitePrivateChatMember*>(pData);
			if (ERROR_NONE != pPacket->nRetCode)
			{
				SetPrivateChannelID(0);				
				return;
			}			

			CDNPrivateChaChannel* pPrivateChatChannel = g_pPrivateChatChannelManager->GetPrivateChannelInfo( pPacket->cWorldSetID, pPacket->nPrivateChatChannelID);

			if(pPrivateChatChannel)
			{
				SendPrivateChatChannelJoinResult(pPacket->nRetCode, pPrivateChatChannel->GetChannelName());				
				SendPrivateChatChannel(pPrivateChatChannel);
				SetPrivateChannelID(pPacket->nPrivateChatChannelID);
			}
		}
		break;	
	default:
		break;
	}
}
#endif

#if defined( PRE_ALTEIAWORLD_EXPLORE )
void CDNUserBase::OnDBRecvAlteiaWorld(int nSubCmd, char* pData)
{
//	switch( nSubCmd )
//	{	
//	default:
//		break;
//	}
}
#endif

#if defined( PRE_ADD_STAMPSYSTEM )
void CDNUserBase::OnDBRecvStampSystem( int nSubCmd, char* pData )
{
	switch( nSubCmd )
	{
	case QUERY_GETLIST_COMPLETECHALLENGES:
		{
			const TAGetListCompleteChallenges* pPacket = reinterpret_cast<TAGetListCompleteChallenges*>(pData);
			if( ERROR_NONE != pPacket->nRetCode )
			{
				return;
			
			}
			GetStampSystem()->InitCompleteSlot( pPacket->CompleteInfo );
		}
		break;
	case QUERY_INIT_COMPLETECHALLENGE:
		{
		}
		break;
	case QUERY_ADD_COMPLETECHALLENGE:
		{
		}
		break;
	default:
		break;
	}
}
#endif // #if defined( PRE_ADD_STAMPSYSTEM )



void CDNUserBase::NotifyGuildMemberLevelUp(char cLevel)
{
	if(GetGuildUID().IsSet()) 
	{

		const TGuildUID GuildUID = GetGuildUID();

		CDNGuildBase* pGuild = g_pGuildManager->At(GuildUID);

		if(pGuild) 
		{
#if !defined( PRE_ADD_NODELETEGUILD )
			CDetachAutoEx<CDNGuildBase>	AutoDetach(pGuild);
			if(FALSE == pGuild->IsEnable()) return;
#endif

			pGuild->UpdateMemberLevel(GetCharacterDBID(), cLevel);

			// Synchronize level via master server.
#if defined(_VILLAGESERVER)
			g_pMasterConnection->SendGuildMemberLevelUp(pGuild->GetUID(), GetCharacterDBID(), cLevel);
#elif defined(_GAMESERVER)
			g_pMasterConnectionManager->SendGuildMemberLevelUp(GetWorldSetID(), pGuild->GetUID(), GetCharacterDBID(), cLevel);
#endif

		}
	}
}

void CDNUserBase::ModifyCompleteQuest()
{
	if(!m_pQuest)
		return;

	g_pPeriodQuestSystem->MarkingSelectQuest(m_pQuest);
	m_pDBCon->QueryGetListCompleteEventQuest(m_pSession);
#if defined(PRE_ADD_REMOTE_QUEST)
	m_pQuest->CheckAcceptWaitRemoteQuestList();
#endif
}

void CDNUserBase::ResetRepeatQuest()
{
	if(!m_pQuest)
	{
		ASSERT(0);
		return;
	}

	g_pQuestManager->ResetRepeatQuest(m_pQuest);
}

int CDNUserBase::GetGuildWareAllowCount()
{
	const TGuildUID GuildUID = GetGuildSelfView().GuildUID;
	if(!GuildUID.IsSet())
		return -1;

	CDNGuildBase* pGuild = g_pGuildManager->At(GuildUID);

	if(!pGuild)
		return -1;

#if !defined( PRE_ADD_NODELETEGUILD )
	CDetachAutoEx<CDNGuildBase>	AutoDetach(pGuild);
	if(FALSE == pGuild->IsEnable()) return -1;
#endif

	return pGuild->GetWareSize();

}

int CDNUserBase::GetGuildRoleType()
{
	const TGuildUID GuildUID = GetGuildSelfView().GuildUID;
	if(!GuildUID.IsSet())
		return -1;

	CDNGuildBase* pGuild = g_pGuildManager->At(GuildUID);

	if(!pGuild)
		return -1;

#if !defined( PRE_ADD_NODELETEGUILD )
	CDetachAutoEx<CDNGuildBase>	AutoDetach(pGuild);
	if(FALSE == pGuild->IsEnable()) return -1;
#endif

	TGuildMember* pGuildMember = pGuild->GetMemberInfo(GetCharacterDBID());
	if(!pGuildMember)
		return -1;

	return pGuildMember->btGuildRole;
}

void CDNUserBase::ExtendGuildWareAllowCount(int nTotalSize)
{
	const TGuildUID GuildUID = GetGuildSelfView().GuildUID;
	if(!GuildUID.IsSet()) 
	{
		_ASSERT( 0 );
		return;
	}

	CDNGuildBase* pGuild = g_pGuildManager->At(GuildUID);

	if(!pGuild)
	{
		_ASSERT( 0 );
		return;
	}

#if !defined( PRE_ADD_NODELETEGUILD )
	CDetachAutoEx<CDNGuildBase>	AutoDetach(pGuild);
	if(FALSE == pGuild->IsEnable()) return;
#endif

	pGuild->UpdateWareSize(this, (short)nTotalSize);
}

void CDNUserBase::UpdatePeriodQuest(int nWorldID, int nPeriodQuestType, std::vector<int>& vTotalQuestIDs)
{
	// Handle resetting periodic quest info
	m_pDBCon->QueryDeletePeriodQuest( m_pSession, nPeriodQuestType, vTotalQuestIDs);
};

void CDNUserBase::CheckResetPeriodQuest(const TAGetPeriodQuestDate* pPacket)
{
	if(pPacket->nCount > 0)
	{
		for (int i=0; i<pPacket->nCount; i++)
		{
			int nType = pPacket->PeriodQuestDate[i].nPeriodType;

			if(m_UserData.Status.tLastPeriodQuestDate[nType] < 0)
				m_UserData.Status.tLastPeriodQuestDate[nType] = 0;

			m_UserData.Status.tLastPeriodQuestDate[nType] = pPacket->PeriodQuestDate[i].tAssignDate;

			switch(nType)
			{
			case PERIODQUEST_RESET_DAILY:
				{
					RegisterSchedule(CDNSchedule::AlarmResetDailyMission, m_UserData.Status.tLastPeriodQuestDate[nType]);
				}
				break;
			case PERIODQUEST_RESET_WEEKLY:
				{
					RegisterSchedule(CDNSchedule::AlarmResetWeeklyMission, m_UserData.Status.tLastPeriodQuestDate[nType]);
				}
				break;
			default:
				continue;
			}
		}
	}
	else
	{ 
		// If there is no quest time saved in DB, this target should receive the quest for the first time, so reset the schedule to receive it again.
		m_UserData.Status.tLastPeriodQuestDate[PERIODQUEST_RESET_DAILY] = 0;
		m_UserData.Status.tLastPeriodQuestDate[PERIODQUEST_RESET_WEEKLY] = 0;

		RegisterSchedule(CDNSchedule::AlarmResetDailyMission, m_UserData.Status.tLastPeriodQuestDate[PERIODQUEST_RESET_DAILY]);
		RegisterSchedule(CDNSchedule::AlarmResetWeeklyMission, m_UserData.Status.tLastPeriodQuestDate[PERIODQUEST_RESET_WEEKLY]);
	}
}

void CDNUserBase::ResetPeriodQuest(int nType)
{
	std::vector<int> vTotalQuestIDs;
	g_pPeriodQuestSystem->GetResetQuestList(m_pQuest, nType, vTotalQuestIDs);
	UpdatePeriodQuest(m_nWorldSetID, nType, vTotalQuestIDs);
}

void CDNUserBase::ProcessPeriodQuest(DWORD dwCurTick)
{
	for (int i=PERIODQUEST_RESET_EVENT; i<PERIODQUEST_RESET_MAX; i++)
	{
		if (!m_bPeriodQuestSchedule[i])
			continue;

		DWORD dwWaitingTick = (DWORD)((GetCharacterDBID() % MAX_PERIODQUEST_DISTRIBUTION) * PERIODQUEST_WAITTIME);
		if (dwWaitingTick > GetTickTerm(m_dwPeriodQuestTick[i], dwCurTick))
			continue;

		m_bPeriodQuestSchedule[i] = false;
		m_dwPeriodQuestTick[i] = dwCurTick;

		ResetPeriodQuest(i);
	}
}

void CDNUserBase::SetDefaultMaxDailyPeriodQuest(ScheduleStruct *pStruct)
{
	CTimeSet tCurrentSet;
	if (pStruct->cHour == tCurrentSet.GetHour())
	{
		if (pStruct->cMinute <= tCurrentSet.GetMinute() && 
			tCurrentSet.GetMinute() <= pStruct->cMinute + MAX_PERIODQUEST_PROCESSMIN)
		{
			m_bPeriodQuestSchedule[PERIODQUEST_RESET_DAILY] = true;
			m_dwPeriodQuestTick[PERIODQUEST_RESET_DAILY]  = timeGetTime();
			return;
		}
	}

	ResetPeriodQuest(PERIODQUEST_RESET_DAILY);
}

void CDNUserBase::SetDefaultMaxWeeklyPeriodQuest(ScheduleStruct *pStruct)
{
	CTimeSet tCurrentSet;
	if (pStruct->cHour == tCurrentSet.GetHour())
	{
		if (pStruct->cMinute <= tCurrentSet.GetMinute() && 
			tCurrentSet.GetMinute() <= pStruct->cMinute + MAX_PERIODQUEST_PROCESSMIN)
		{
			m_bPeriodQuestSchedule[PERIODQUEST_RESET_WEEKLY] = true;
			m_dwPeriodQuestTick[PERIODQUEST_RESET_WEEKLY]  = timeGetTime();
			return;
		}
	}

	ResetPeriodQuest(PERIODQUEST_RESET_WEEKLY);
}

void CDNUserBase::SetEtcPointList( const TAGetListEtcPoint *pPacket )
{
	m_UnionReputePointInfo.SetUnionReputePoint(NpcReputation::UnionType::Commercial, pPacket->biEtcPoint[DBDNWorldDef::EtcPointCode::Union_Commercial]);
	m_UnionReputePointInfo.SetUnionReputePoint(NpcReputation::UnionType::Royal, pPacket->biEtcPoint[DBDNWorldDef::EtcPointCode::Union_Royal]);
	m_UnionReputePointInfo.SetUnionReputePoint(NpcReputation::UnionType::Liberty, pPacket->biEtcPoint[DBDNWorldDef::EtcPointCode::Union_Liberty]);	
	m_biGuildWarFestivalPoint = pPacket->biEtcPoint[DBDNWorldDef::EtcPointCode::GuildWar_Festival];		
}

void CDNUserBase::SetEtcPointEach(int nType, INT64 biLeftPoint)
{
	switch(nType)
	{
	case DBDNWorldDef::EtcPointCode::Union_Commercial:
		{
			m_UnionReputePointInfo.SetUnionReputePoint(NpcReputation::UnionType::Commercial, biLeftPoint);
			SendUnionPoint();
		}
		break;
	case DBDNWorldDef::EtcPointCode::Union_Royal:
		{
			m_UnionReputePointInfo.SetUnionReputePoint(NpcReputation::UnionType::Royal, biLeftPoint);
			SendUnionPoint();
		}
		break;
	case DBDNWorldDef::EtcPointCode::Union_Liberty:
		{
			m_UnionReputePointInfo.SetUnionReputePoint(NpcReputation::UnionType::Liberty, biLeftPoint);
			SendUnionPoint();
		}
		break;
	case DBDNWorldDef::EtcPointCode::GuildWar_Festival:
		{
			m_biGuildWarFestivalPoint = biLeftPoint;
			SendGuildWarFestivalPoint();
		}		
		break;
	}
}

void CDNUserBase::AddEtcPoint( BYTE cType, int nPlusPoint)
{
	if(cType > DBDNWorldDef::EtcPointCode::None && nPlusPoint > 0)
		m_pDBCon->QueryAddEtcPoint(m_pSession, cType, nPlusPoint);
}

void CDNUserBase::UseEtcPoint( BYTE cType, int nUsePoint )
{
	if(cType > DBDNWorldDef::EtcPointCode::None && nUsePoint > 0)
		m_pDBCon->QueryUseEtcPoint(m_pSession, cType, nUsePoint);
}

int CDNUserBase::ParseEnchant( CSEnchantItem* pPacket, int nLen )
{
	if(bIsRemoteEnchant()==false && !IsValidPacketByNpcClick()){	// Check NPC distance
		return ERROR_GENERIC_INVALIDREQUEST;
	}

	if(sizeof(CSEnchantItem) != nLen)
		return ERROR_INVALIDPACKET;

	if( !IsWindowState(WINDOW_BLIND) ){
#if defined(PRE_ADD_EQUIPED_ITEM_ENCHANT)
		SendEnchant(pPacket->cItemIndex, pPacket->bEnchantEquipedItem, ERROR_ITEM_ENCHANTFAIL);
#else
		SendEnchant(pPacket->cInvenIndex, ERROR_ITEM_ENCHANTFAIL);
#endif
		return ERROR_NONE;
	}

	if(m_pRestraint->CheckRestraint(_RESTRAINTTYPE_TRADE) == false)
		return ERROR_NONE;

	if( bIsRemoteEnchant() == true )
	{
		if( m_pItem->GetCashItemCountByType( ITEMTYPE_REMOTE_ENCHANT ) <= 0 && m_pItem->GetInventoryItemCountByType( ITEMTYPE_REMOTE_ENCHANT ) <= 0 )
		{
#if defined(PRE_ADD_EQUIPED_ITEM_ENCHANT)
			SendEnchant(pPacket->cItemIndex, pPacket->bEnchantEquipedItem, ERROR_ITEM_INSUFFICIENCY_ITEMTYPE_REMOTEENCHANT);
#else
			SendEnchant(pPacket->cInvenIndex, ERROR_ITEM_INSUFFICIENCY_ITEMTYPE_REMOTEENCHANT);
#endif
			return ERROR_NONE;
		}
	}

	int nRet = m_pItem->CheckUpgradeEnchant(pPacket);

#if defined(PRE_ADD_EQUIPED_ITEM_ENCHANT)
	SendEnchant(pPacket->cItemIndex, pPacket->bEnchantEquipedItem, nRet);
#else
	SendEnchant(pPacket->cInvenIndex, nRet);
#endif
	if( nRet == ERROR_NONE ) m_pSession->BroadcastingEffect(EffectType_Enchant, EffectState_Start);

	return ERROR_NONE;
}

int CDNUserBase::ParseEnchantComplete( CSEnchantItem* pPacket, int nLen )
{
	if(!m_pItem->IsValidRequestTimer(CDNUserItem::RequestType_Enchant)) {
		SendEnchantComplete(0, 0, 0, ERROR_ITEM_ENCHANTFAIL);
		return ERROR_NONE;
	}

	if(bIsRemoteEnchant()==false && !IsValidPacketByNpcClick()){	// Check NPC distance
		return ERROR_GENERIC_INVALIDREQUEST;
	}

	if(sizeof(CSEnchantItem) != nLen)
		return ERROR_INVALIDPACKET;

	if( !IsWindowState(WINDOW_BLIND) ){
		SendEnchantComplete(0, 0, 0, ERROR_ITEM_ENCHANTFAIL);
		return ERROR_NONE;
	}

	if( bIsRemoteEnchant() == true )
	{
		if( m_pItem->UseItemByType( ITEMTYPE_REMOTE_ENCHANT, 1, DBDNWorldDef::UseItem::Use ) == false )
		{
#if defined(PRE_ADD_EQUIPED_ITEM_ENCHANT)
			SendEnchant(pPacket->cItemIndex, pPacket->bEnchantEquipedItem, ERROR_ITEM_INSUFFICIENCY_ITEMTYPE_REMOTEENCHANT);
#else
			SendEnchant(pPacket->cInvenIndex, ERROR_ITEM_INSUFFICIENCY_ITEMTYPE_REMOTEENCHANT);
#endif
			return ERROR_ITEM_FAIL;
		}
	}

	int nRet = m_pItem->UpgradeEnchant(pPacket);
	if(nRet != ERROR_NONE) SendEnchantComplete(0, 0, 0, nRet);
	return ERROR_NONE;
}

int CDNUserBase::ParseEnchantCancel( int nLen )
{
	if(!bIsRemoteEnchant() && !IsValidPacketByNpcClick()){	// Check NPC distance
		return ERROR_GENERIC_INVALIDREQUEST;
	}

	if(nLen != 0)
		return ERROR_INVALIDPACKET;

	SendEnchantCancel(ERROR_NONE);
	m_pSession->BroadcastingEffect(EffectType_Enchant, EffectState_Cancel);
	return ERROR_NONE;
}

int CDNUserBase::ParseItemCompound( CSItemCompoundOpenReq* pPacket, int nLen )
{
	if( !bIsRemoteItemCompound() && !IsValidPacketByNpcClick()){	// Check NPC distance
		return ERROR_GENERIC_INVALIDREQUEST;
	}

	if(sizeof(CSItemCompoundOpenReq) != nLen)
		return ERROR_INVALIDPACKET;

	if( !IsWindowState(WINDOW_BLIND) ) {
		SendEmblemCompoundRes( ERROR_ITEM_ITEM_COMPOUND_FAIL, 0 );
		return ERROR_NONE;
	}

	if(m_pRestraint->CheckRestraint(_RESTRAINTTYPE_TRADE) == false)
		return ERROR_NONE;

	CDnItemCompounder::S_ITEM_SETTING_INFO ItemSettingInfo;
	CDnItemCompounder::S_OUTPUT Output;
	CDnItemCompounder::S_COMPOUND_INFO_EXTERN CompoundInfo;

	ItemSettingInfo.iCompoundTableID = pPacket->nCompoundTableID;
	ItemSettingInfo.iHasMoney = GetCoin();		// Note: GetCoin return value is INT64.

	g_pDataManager->GetItemCompounder()->GetCompoundInfo( pPacket->nCompoundTableID, &CompoundInfo );

	bool bValidHaveItem = true;
	int iItemSettingArrayCount = 0;
	for( int i = 0; i < COMPOUNDITEMMAX; ++i )
	{
		if( 0 == CompoundInfo.aiItemID[ i ] )
			break;

		int iHaveCount = 0;
		if( CompoundInfo.abResultItemIsNeedItem[i] ) {
			std::vector<TItem *> pVecResultList;
			//When searching for items, only search items matching the option settings..
			iHaveCount = m_pItem->GetInventoryItemListFromItemID( CompoundInfo.aiItemID[ i ], CompoundInfo.iCompoundPreliminaryID, pVecResultList );
			if( iHaveCount ) {
				bool bValidItem = false;
				for( DWORD j=0; j<pVecResultList.size(); j++ ) {
					if( pVecResultList[j]->nSerial == pPacket->biNeedItemSerialID ) {
						bValidItem = true;
						break;
					}
				}
				if( !bValidItem ) {
					bValidHaveItem = false;
					break;
				}
			}
		}
		else iHaveCount = m_pItem->GetInventoryItemCount( CompoundInfo.aiItemID[ i ], -1 );
		if( iHaveCount < CompoundInfo.aiItemCount[ i ] )
		{
			bValidHaveItem = false;
			break;
		}

		ItemSettingInfo.aiItemID[ i ] = CompoundInfo.aiItemID[ i ];
		ItemSettingInfo.aiItemCount[ i ] = CompoundInfo.aiItemCount[ i ];
		++iItemSettingArrayCount;
	}

	int iResult = ERROR_ITEM_ITEM_COMPOUND_FAIL;

	if( bValidHaveItem )
	{
		if( bIsRemoteItemCompound() == true )
		{
			const TItemData* pRemoteItemData = g_pDataManager->GetItemData( m_pSession->GetRemoteItemCompoundItemID() );
			if( pRemoteItemData )
			{
				INT64 iDiscount = CompoundInfo.iCost * pRemoteItemData->nTypeParam[0] / 100;
				ItemSettingInfo.iDiscountedCost = CompoundInfo.iCost - iDiscount;
			}
		}

		if( GetCheatCommand()->m_bLucky ) g_pDataManager->GetItemCompounder()->CheckPossibility( false );
#if defined( _GAMESERVER )
		g_pDataManager->ItemCompound( m_pSession->GetGameRoom(), ItemSettingInfo, &Output );
#else
		g_pDataManager->ItemCompound( ItemSettingInfo, &Output );
#endif // #if defined( _GAMESERVER )
		if( GetCheatCommand()->m_bLucky ) g_pDataManager->GetItemCompounder()->CheckPossibility( true );

		if( CDnItemCompounder::R_SUCCESS == Output.eResultCode ||
			CDnItemCompounder::R_POSSIBILITY_FAIL == Output.eResultCode )
		{
			iResult = ERROR_NONE;
			m_pSession->BroadcastingEffect(EffectType_Compound, EffectState_Start);
		}
	}

	SendItemCompoundOpen( iResult );

	return ERROR_NONE;
}

int CDNUserBase::ParseItemCompoundComplete( CSCompoundItemReq* pCompoundItem, int nLen )
{
	if( !bIsRemoteItemCompound() && !IsValidPacketByNpcClick()){	// Check NPC distance
		return ERROR_GENERIC_INVALIDREQUEST;
	}

	if(sizeof(CSCompoundItemReq) != nLen)
		return ERROR_INVALIDPACKET;

	// Note: Later if different lists are shown per NPC, check whether received data exists in that NPC's list.

	CDnItemCompounder::S_ITEM_SETTING_INFO ItemSettingInfo;
	CDnItemCompounder::S_OUTPUT Output;
	CDnItemCompounder::S_COMPOUND_INFO_EXTERN CompoundInfo;

	ItemSettingInfo.iCompoundTableID = pCompoundItem->nCompoundTableID;
	ItemSettingInfo.iHasMoney = GetCoin();		// Note: GetCoin return value is INT64.

	g_pDataManager->GetItemCompounder()->GetCompoundInfo( pCompoundItem->nCompoundTableID, &CompoundInfo );

	bool bValidHaveItem = true;
	int iItemSettingArrayCount = 0;
	for( int i = 0; i < NUM_MAX_NEED_ITEM; ++i )
	{
		if( 0 == CompoundInfo.aiItemID[ i ] )
			break;

		int iHaveCount = 0;
		if( CompoundInfo.abResultItemIsNeedItem[i] ) {
			std::vector<TItem *> pVecResultList;
			//When searching for items, only search items matching the option settings..
			iHaveCount = m_pItem->GetInventoryItemListFromItemID( CompoundInfo.aiItemID[ i ], CompoundInfo.iCompoundPreliminaryID, pVecResultList );
			if( iHaveCount ) {
				bool bValidItem = false;
				for( DWORD j=0; j<pVecResultList.size(); j++ ) {
					if( pVecResultList[j]->nSerial == pCompoundItem->biNeedItemSerialID ) {
						bValidItem = true;
						break;
					}
				}
				if( !bValidItem ) {
					bValidHaveItem = false;
					break;
				}
			}
		}
		else iHaveCount = m_pItem->GetInventoryItemCount( CompoundInfo.aiItemID[ i ], -1 );

		if( iHaveCount < CompoundInfo.aiItemCount[ i ] )
		{
			bValidHaveItem = false;
			break;
		}

		ItemSettingInfo.aiItemID[ i ] = CompoundInfo.aiItemID[ i ];
		ItemSettingInfo.aiItemCount[ i ] = CompoundInfo.aiItemCount[ i ];
		++iItemSettingArrayCount;
	}

	int iResult = ERROR_ITEM_EMBLEM_COMPOUND_FAIL;
	int nResultItemID = 0;
	std::vector<CDNUserItem::TSaveItemInfo> VecResultItemList;
	if( bValidHaveItem )
	{
		iResult = ERROR_ITEM_ITEM_COMPOUND_FAIL;
		if( IsWindowState(WINDOW_BLIND) )
		{
			if( bIsRemoteItemCompound() == true )
			{
				if( m_pItem->UseItemByItemID( GetRemoteItemCompoundItemID(), 1, DBDNWorldDef::UseItem::Use ) == false )
				{
					SendItemCompoundRes( iResult, 0, 0, 0 );
					return ERROR_NONE;
				}

				const TItemData* pRemoteItemData = g_pDataManager->GetItemData( m_pSession->GetRemoteItemCompoundItemID() );
				if( pRemoteItemData )
				{
					INT64 iDiscount = CompoundInfo.iCost * pRemoteItemData->nTypeParam[0] / 100;
					ItemSettingInfo.iDiscountedCost = CompoundInfo.iCost - iDiscount;
				}
			}

			if( GetCheatCommand()->m_bLucky ) g_pDataManager->GetItemCompounder()->CheckPossibility( false );
#if defined( _GAMESERVER )
			g_pDataManager->ItemCompound( m_pSession->GetGameRoom(), ItemSettingInfo, &Output );
#else
			g_pDataManager->ItemCompound( ItemSettingInfo, &Output );
#endif // #if defined( _GAMESERVER )
			if( GetCheatCommand()->m_bLucky ) g_pDataManager->GetItemCompounder()->CheckPossibility( true );

			if( CDnItemCompounder::R_SUCCESS == Output.eResultCode ||
				CDnItemCompounder::R_POSSIBILITY_FAIL == Output.eResultCode )
			{
				if(!DelCoin( Output.iCost, DBDNWorldDef::CoinChangeCode::CompoundTax, m_nClickedNpcID ))
					SendItemCompoundRes( ERROR_ITEM_EMBLEM_COMPOUND_FAIL, 0, 0, 0 );

				// Design originally allowed multiple result items, but changed again.
				// Since it may change back to allowing multiple result items, set the first item for now.
				// Return drop item table when random case occurs
				if( !Output.vlItemCount.empty() && Output.vlItemCount.front() == 0 )
				{							
					int nResultItemCount = 0;
					int nLastValue = 1;
					int nDepth = ITEMDROP_DEPTH;

#if defined( PRE_ADD_FARM_DOWNSCALE )
					CDNUserItem::CalcDropItems( m_pSession, Output.vlItemID.front(), nResultItemID, nResultItemCount, nLastValue, nDepth );
#else
					m_pItem->CalcDropItems( Output.vlItemID.front(), nResultItemID, nResultItemCount, nLastValue, nDepth );
#endif // #if defined( PRE_ADD_FARM_DOWNSCALE )
				}
				else {
					nResultItemID = Output.vlItemID.empty() ? 0 : Output.vlItemID.front();
				}
				// Check if result item is a heraldry... abusing
				if( nResultItemID > 0)
				{
					if( g_pDataManager->GetItemMainType(nResultItemID) == ITEMTYPE_GLYPH)
					{
						m_pSession->BroadcastingEffect(EffectType_Compound, EffectState_Fail);
						SendItemCompoundRes( ERROR_ITEM_EMBLEM_COMPOUND_FAIL, 0, 0, 0 );
						g_Log.Log(LogType::_ERROR, 0, GetAccountDBID(), GetCharacterDBID(), 0, L"ERROR Try Emblem Compound EmlbemID:%d!!\r\n", nResultItemID);
						return ERROR_NONE;
					}
				}
				if(!m_pItem->ItemCompoundComplete( nResultItemID, Output.cItemOptionIndex, CompoundInfo.aiItemID,  CompoundInfo.aiItemCount, iItemSettingArrayCount, ( CompoundInfo.bResultItemIsNeedItem ) ? pCompoundItem->biNeedItemSerialID : 0, &VecResultItemList ))
					SendItemCompoundRes( ERROR_ITEM_EMBLEM_COMPOUND_FAIL, 0, 0, 0 );

				if( CDnItemCompounder::R_POSSIBILITY_FAIL == Output.eResultCode ){
					iResult = ERROR_ITEM_ITEM_COMPOUND_POSSIBILITY_FAIL;
					m_pSession->BroadcastingEffect(EffectType_Compound, EffectState_Fail);
				}
				else{
					iResult = ERROR_NONE;
					m_pSession->BroadcastingEffect(EffectType_Compound, EffectState_Success);
				}
			}
			else
			{
				// Subdivide failure reasons.. 
				iResult = _MakeItemCompoundErrorCode( Output );
			}
		}
	}

	// Send result response to client.
	char cLevel = ( VecResultItemList.empty() ) ? 0 : VecResultItemList[0].Item.cLevel;
	char cOption = ( VecResultItemList.empty() ) ? 0 : VecResultItemList[0].Item.cOption;
	SendItemCompoundRes( iResult, nResultItemID, cLevel, cOption );
	return ERROR_NONE;
}

int CDNUserBase::ParseItemCompoundCancel( CSItemCompoundCancelReq* pPacket, int nLen )
{
	if( !bIsRemoteItemCompound() && !IsValidPacketByNpcClick()){	// Check NPC distance
		return ERROR_GENERIC_INVALIDREQUEST;
	}

	if(sizeof(CSItemCompoundCancelReq) != nLen)
		return ERROR_INVALIDPACKET;

	SendItemCompoundCancel( ERROR_NONE );
	m_pSession->BroadcastingEffect(EffectType_Compound, EffectState_Cancel);

	return ERROR_NONE;
}

int CDNUserBase::_MakeItemCompoundErrorCode( CDnItemCompounder::S_OUTPUT &Output )
{
	int iResult = ERROR_ITEM_ITEM_COMPOUND_FAIL;

	if( Output.eResultCode == CDnItemCompounder::R_FAIL )
	{
		// Combination failed due to an unknown error.
		iResult = ERROR_ITEM_ITEM_COMPOUND_FAIL;
	}
	else
		//if( Output.eResultCode == CDnItemCompounder::R_POSSIBILITY_FAIL )
		//{
		//	// Failed probability check. Not an error.
		//	iResult = ERROR_ITEM_ITEM_COMPOUND_POSSIBILITY_FAIL;
		//}
		//else
	{
		// This indicates an error. Abnormal situation where the packet should never have reached the server in the first place.
		switch( Output.eErrorCode )
		{
		case CDnItemCompounder::E_NOT_ENOUGH_MONEY:			// Not enough money
			iResult = ERROR_ITEM_ITEM_COMPOUND_NOT_ENOUGH_MONEY;
			break;

		case CDnItemCompounder::E_NOT_ENOUGH_ITEM:			// Insufficient material items
			iResult = ERROR_ITEM_ITEM_COMPOUND_NOT_ENOUGH_ITEM;
			break;

		case CDnItemCompounder::E_NOT_MATCH_SLOT:				// Item does not fit the slot
			iResult = ERROR_ITEM_ITEM_COMPOUND_NOT_MATCH_SLOT;
			break;

		case CDnItemCompounder::E_NOT_ENOUGH_ITEM_COUNT:		// Fits the slot, but insufficient quantity
			iResult = ERROR_ITEM_ITEM_COMPOUND_NOT_ENOUGH_ITEM_COUNT;
			break;
		}
	}	

	return iResult;
}

#if defined( PRE_ADD_VIP_FARM )

bool CDNUserBase::bIsFarmVip()
{
	// Countries other than China and Taiwan check VIP farm entry via EffectItem
	if( m_pEffectRepository->bIsExpiredItem( ITEMTYPE_FARM_VIP ) == true )
		return false;
	
	return true;
}

#endif // #if defined( PRE_ADD_VIP_FARM )

void CDNUserBase::AddGhoulScore(GhoulMode::PointType::eCode Type)
{
	++m_PvPTotalGhoulScores.nPoint[Type];
	++m_PvPAddGhoulScores.nPoint[Type];
}

void CDNUserBase::ClearAddGhoulScore()
{
	memset(&m_PvPAddGhoulScores, 0, sizeof(m_PvPAddGhoulScores));
}

int CDNUserBase::GetMaxLevelCharacterCount()
{
	int nCount = 0;
	int nLevelLimit = (int)CGlobalWeightTable::GetInstance().GetValue( CGlobalWeightTable::PlayerLevelLimit );
	for( int i=0; i<CHARCOUNTMAX; i++ ) {
		if( m_OwnCharacterLevelList[i].cLevel >= nLevelLimit ) nCount++;
	}

	return nCount;
}

int CDNUserBase::GetLevelCharacterCount(int nLevel)
{
	int nCount = 0;
	for( int i=0; i<CHARCOUNTMAX; i++ ) {
		if( m_OwnCharacterLevelList[i].cLevel >= nLevel ) nCount++;
	}

	return nCount;
}

void CDNUserBase::NextTalk(UINT nNpcObjectID, WCHAR* wszTalkIndex, WCHAR* wszTarget, std::vector<TalkParam>& talkParam )
{
	SendNextTalk(nNpcObjectID, wszTalkIndex, wszTarget, talkParam);
}

void CDNUserBase::_MakeGiftVIPMonthItemByItemSN(int nItemSN)
{
#if defined(PRE_ADD_VIP)
	TVIPData *pVIPData = g_pDataManager->GetVIPData();
	if (pVIPData){
		std::wstring wstrMemo = FormatW(GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 2020011), g_pDataManager->GetCashCommodityCount(pVIPData->nMonthItemSN));
		int nGiftCount = g_pDataManager->GetCashCommodityPeriod(nItemSN) / 30;

		int nItemList[5] = {0,};
		nItemList[0] = pVIPData->nMonthItemSN;
		for (int i = 0; i < nGiftCount; i++){
			m_pDBCon->QueryMakeGift(m_cDBThreadID, GetWorldSetID(), GetAccountDBID(), GetCharacterDBID(), IsPCBang(), (WCHAR*)wstrMemo.c_str(), DBDNWorldDef::PayMethodCode::VIP, GetLevel(), (char*)m_pSession->GetIp(), nItemList, false);
		}
	}
#endif	// #if defined(PRE_ADD_VIP)
}

int CDNUserBase::_OnCashBalanceInquiry()
{
	if (!g_pCashConnection->GetActive()) return ERROR_GENERIC_INVALIDREQUEST;

	g_pCashConnection->SendBalanceInquiry(m_pSession, false, false);

	return ERROR_NONE;
}

int CDNUserBase::_OnCashShopOpen()
{
	if( !IsNoneWindowState() )
		return ERROR_GENERIC_INVALIDREQUEST;

#ifdef _VILLAGESERVER
	if (m_pSession->m_bIsStartGame || m_pSession->m_bIsMove)
		return ERROR_GENERIC_INVALIDREQUEST;
#endif		//#ifdef _VILLAGESERVER

	if (!g_pCashConnection->GetActive())
		return ERROR_GENERIC_INVALIDREQUEST;

#if defined(_VILLAGESERVER) && defined(PRE_ADD_CASH_REFUND) // Send billing inventory first..
	SendPaymentItemList(m_pItem);
	SendPaymentPackageItemList(m_pItem);
#endif

	g_pCashConnection->SendBalanceInquiry(m_pSession, true, false);

	SetWindowState(WINDOW_CASHSHOP);
	return ERROR_NONE;
}

int CDNUserBase::_OnCashBuy(CSCashShopBuy *pCashShop)
{
#if defined(PRE_ADD_INSTANT_CASH_BUY)
	if (pCashShop->cType >= InstantCashShopTypeMin){
		if (pCashShop->cCount != 1) return ERROR_ITEM_FAIL;
#if defined(PRE_ADD_CASH_REFUND)
		if (!pCashShop->bMoveCashInven) return ERROR_ITEM_FAIL;
#endif	// #if defined(PRE_ADD_CASH_REFUND)

		if (!g_pDataManager->IsCashBuyShortcut(pCashShop->cType - InstantCashShopTypeMin, m_pSession->GetMapIndex(), pCashShop->BuyList[0].nItemSN))
			return ERROR_ITEM_FAIL;
	}
	else{
		if (!IsWindowState(WINDOW_CASHSHOP))  return ERROR_GENERIC_INVALIDREQUEST;

		if (pCashShop->cCount <= 0 || pCashShop->cCount > PREVIEWCARTLISTMAX){	// No products available
			g_Log.Log(LogType::_NORMAL, m_nWorldSetID, m_nAccountDBID, m_biCharacterDBID, m_nSessionID, L"Invalid cash buy count(%d)\r\n", pCashShop->cCount);
			return ERROR_ITEM_FAIL;
		}
	}
#else	// #if defined(PRE_ADD_INSTANT_CASH_BUY)
	if (!IsWindowState(WINDOW_CASHSHOP))  return ERROR_GENERIC_INVALIDREQUEST;

	if (pCashShop->cCount <= 0 || pCashShop->cCount > PREVIEWCARTLISTMAX){	// No products available
		g_Log.Log(LogType::_NORMAL, m_nWorldSetID, m_nAccountDBID, m_biCharacterDBID, m_nSessionID, L"Invalid cash buy count(%d)\r\n", pCashShop->cCount);
		return ERROR_ITEM_FAIL;
	}
#endif	// #if defined(PRE_ADD_INSTANT_CASH_BUY)

#if defined(PRE_ADD_SALE_COUPON)
	// Perform various checks when using a Sale coupon item..
	if( pCashShop->biSaleCouponSerial > 0)
	{
		// A coupon can strictly purchase only one item..
		// Check if a coupon exists and whether the item can be bought with a coupon.
		if( pCashShop->cCount > 1 || !g_pDataManager->IsSaleCouponByItem(pCashShop->BuyList[0].nItemSN) ) 
			return ERROR_ITEM_FAIL;		

		int nRet = _CheckSaleCoupon(pCashShop->nSaleCouponSN, pCashShop->biSaleCouponSerial, pCashShop->BuyList[0].nItemSN);
		if (nRet != ERROR_NONE)
			return nRet;
	}
#endif // #if defined(PRE_ADD_SALE_COUPON)

	std::vector<TEffectItemData> VecEffectItemList, VecCashList;
	VecEffectItemList.clear();
	VecCashList.clear();

	for (int i = 0; i < pCashShop->cCount; i++){
		if ((pCashShop->BuyList[i].nItemSN <= 0) ||(pCashShop->BuyList[i].nItemID <= 0)) return ERROR_ITEM_NOTFOUND;
		if( !g_pDataManager->CheckCashSNItemID(pCashShop->BuyList[i].nItemSN, pCashShop->BuyList[i].nItemID) )
			return ERROR_ITEM_NOTFOUND;
		bool bPackage = g_pDataManager->IsCashPackageData(pCashShop->BuyList[i].nItemSN);
		if (bPackage) return ERROR_ITEM_NOTFOUND;
#if defined(PRE_ADD_SALE_COUPON)
		// Just in case an item requiring a Sale coupon was sent as-is..
		if( g_pDataManager->IsSaleCouponByItem(pCashShop->BuyList[i].nItemSN) && pCashShop->biSaleCouponSerial <= 0)
			return ERROR_ITEM_FAIL;
#endif
#if defined(PRE_ADD_VIP)
		if (g_pDataManager->IsVIPSell(pCashShop->BuyList[i].nItemSN)){
			if (m_nVIPTotalPoint <= 0) return ERROR_VIP_FAIL;

			const CVIPGradeTable::SVIPGradeUnit* pUnit = CVIPGradeTable::GetInstance().GetValue(m_nVIPTotalPoint);
			if (!pUnit)	return ERROR_VIP_FAIL;

			if (g_pDataManager->GetCashCommodityVIPLevel(pCashShop->BuyList[i].nItemSN) > pUnit->level) return ERROR_VIP_FAIL;	// Level is too low
		}
#endif	// #if defined(PRE_ADD_VIP)


		TEffectItemData CashInfo = {0,};
		CashInfo.nItemSN = pCashShop->BuyList[i].nItemSN;
		CashInfo.nItemID = pCashShop->BuyList[i].nItemID;
		CashInfo.nCount = g_pDataManager->GetCashCommodityCount(pCashShop->BuyList[i].nItemSN);
		VecCashList.push_back(CashInfo);

		if (m_pItem->IsEffectCashItem(pCashShop->BuyList[i].nItemID)){	// If it is an intangible item
			VecEffectItemList.push_back(CashInfo);
		}
	}

	int nRet = m_pItem->CheckEffectItemListCountLimit(VecEffectItemList, false, false, false);	// Check max count of intangible cash items that can be purchased
	if (nRet != ERROR_NONE)
		return nRet;

	nRet = m_pItem->CheckCashDuplicationBuy(VecCashList, false);
	if (nRet != ERROR_NONE)
		return nRet;

	TCashBuyItem CashBuyList[PREVIEWCARTLISTMAX];
	memset(&CashBuyList, 0, sizeof(CashBuyList));
	for (int i = 0; i < pCashShop->cCount; i++){
		CashBuyList[i].cCartIndex = pCashShop->BuyList[i].cSlotIndex;
		nRet = m_pItem->MakeBuyCashItem(pCashShop->BuyList[i].nItemSN, pCashShop->BuyList[i].nItemID, pCashShop->BuyList[i].nOptionIndex, CashBuyList[i]);
		if (nRet != ERROR_NONE) return nRet;
	}

	int nTotalPrice = 0, nPrice = 0;
	for (int i = 0; i < pCashShop->cCount; i++){
		if (pCashShop->BuyList[i].nItemSN <= 0) continue;
		if (g_pDataManager->GetItemOverlapCount(pCashShop->BuyList[i].nItemID) < g_pDataManager->GetCashCommodityCount(pCashShop->BuyList[i].nItemSN)) break;

		nPrice = g_pDataManager->GetCashCommodityPrice(pCashShop->BuyList[i].nItemSN);
		if (nPrice <= 0) continue;
		nTotalPrice += nPrice;
	}

	char cPaymentRules = Cash::PaymentRules::None;
	cPaymentRules = pCashShop->cPaymentRules;

	if (pCashShop->cPaymentRules == Cash::PaymentRules::Petal){	// When purchasing with reward points...
		if (!CheckEnoughPetal(nTotalPrice))
			return ERROR_CASHSHOP_RESERVE_NOTENOUGH;	// Not enough money to buy

		for (int i = 0; i < pCashShop->cCount; i++){
			if (g_pDataManager->IsReserveCommodity(pCashShop->BuyList[i].nItemSN)) continue;
			if (pCashShop->BuyList[i].nItemID != 0) continue;	// Prevent purchase if 0

			return ERROR_CASHSHOP_RESERVE_DISABLE;
		}
	}
	else{
		if (!CheckEnoughCashByPaymentRules(nTotalPrice, cPaymentRules))
			return ERROR_CASHSHOP_CASH_NOTENOUGH;
	}

	if (!g_pCashConnection->GetActive()) return ERROR_GENERIC_INVALIDREQUEST;

#if defined(_VILLAGESERVER) && defined(PRE_ADD_CASH_REFUND)
	m_pItem->SetCashMoveInven(pCashShop->bMoveCashInven);
#endif
	g_pCashConnection->SendBuy(m_pSession, pCashShop, CashBuyList);

	return ERROR_NONE;
}

int CDNUserBase::_OnCashPackageBuy(CSCashShopPackageBuy *pCashShop)
{
	if( !IsWindowState(WINDOW_CASHSHOP) )
		return ERROR_GENERIC_INVALIDREQUEST;

	if (pCashShop->cCount <= 0 || pCashShop->cCount > PACKAGEITEMMAX){	// No products available
		g_Log.Log(LogType::_NORMAL, m_nWorldSetID, m_nAccountDBID, m_biCharacterDBID, m_nSessionID, L"Invalid cash package buy count(%d)\r\n", pCashShop->cCount);
		return ERROR_ITEM_FAIL;
	}

	std::vector<TEffectItemData> VecEffectItemList, VecCashList;
	VecEffectItemList.clear();
	VecCashList.clear();

	TCashPackageData PackageData;
	bool bPackage = g_pDataManager->GetCashPackageData(pCashShop->nPackageSN, PackageData);
	if (!bPackage)
		return ERROR_ITEM_NOTFOUND;

#if defined(PRE_ADD_SALE_COUPON)
	// Perform various checks when using a Sale coupon item..
	if( pCashShop->biSaleCouponSerial > 0)
	{
		 // Whether it can be purchased with a coupon.
		if( !g_pDataManager->IsSaleCouponByItem(pCashShop->nPackageSN))
			return ERROR_ITEM_FAIL;

		int nRet = _CheckSaleCoupon(pCashShop->nSaleCouponSN, pCashShop->biSaleCouponSerial, pCashShop->nPackageSN);
		if (nRet != ERROR_NONE)
			return nRet;
	}
	else
	{
		//Check if an item purchasable only with a coupon was sent without one...
		if( g_pDataManager->IsSaleCouponByItem(pCashShop->nPackageSN) )
			return ERROR_ITEM_FAIL;
	}
#endif // #if defined(PRE_ADD_SALE_COUPON)

	for (int i = 0; i < pCashShop->cCount; i++){
		if ((pCashShop->BuyList[i].nItemSN <= 0) ||(pCashShop->BuyList[i].nItemID <= 0))
			return ERROR_ITEM_NOTFOUND;

		if( (int)(PackageData.nVecCommoditySN.size()) <= i || PackageData.nVecCommoditySN[i] != pCashShop->BuyList[i].nItemSN )
			return ERROR_ITEM_NOTFOUND;

		if( !g_pDataManager->CheckCashSNItemID(pCashShop->BuyList[i].nItemSN, pCashShop->BuyList[i].nItemID ) )
			return ERROR_ITEM_NOTFOUND;
#if defined(PRE_ADD_VIP)
		if (g_pDataManager->IsVIPSell(pCashShop->nPackageSN)){
			if (m_nVIPTotalPoint <= 0)
				return ERROR_VIP_FAIL;

			const CVIPGradeTable::SVIPGradeUnit* pUnit = CVIPGradeTable::GetInstance().GetValue(m_nVIPTotalPoint);
			if (!pUnit)
				return ERROR_VIP_FAIL;

			if (g_pDataManager->GetCashCommodityVIPLevel(pCashShop->nPackageSN) > pUnit->level)
				return ERROR_VIP_FAIL;	// Level is too low
		}
#endif	// #if defined(PRE_ADD_VIP)


		TEffectItemData CashInfo = {0,};
		CashInfo.nItemSN = pCashShop->BuyList[i].nItemSN;
		CashInfo.nItemID = pCashShop->BuyList[i].nItemID;
		CashInfo.nCount = g_pDataManager->GetCashCommodityCount(pCashShop->BuyList[i].nItemSN);
		VecCashList.push_back(CashInfo);

		if (m_pItem->IsEffectCashItem(pCashShop->BuyList[i].nItemID)){	// If it is an intangible item
			VecEffectItemList.push_back(CashInfo);
		}
	}

	int nRet = m_pItem->CheckEffectItemListCountLimit(VecEffectItemList, false, true, false);	// Check max limit on how many cash items can be purchased
	if (nRet != ERROR_NONE) return nRet;

	nRet = m_pItem->CheckCashDuplicationBuy(VecCashList, false);
	if (nRet != ERROR_NONE) return nRet;

	TCashItemBase PackageList[PACKAGEITEMMAX];
	for (int i = 0; i < pCashShop->cCount; i++){
		nRet = m_pItem->MakeBuyCashItem(pCashShop->BuyList[i].nItemSN, pCashShop->BuyList[i].nItemID, pCashShop->BuyList[i].nOptionIndex, PackageList[i]);
		if (nRet != ERROR_NONE) return nRet;
	}

	if (pCashShop->nPackageSN <= 0)
		return ERROR_ITEM_FAIL;

	int nTotalPrice = g_pDataManager->GetCashCommodityPrice(pCashShop->nPackageSN);

	char cPaymentRules = Cash::PaymentRules::None;
	cPaymentRules = pCashShop->cPaymentRules;

	if (pCashShop->cPaymentRules == Cash::PaymentRules::Petal){	// When purchasing with reward points...
		for (int i = 0; i < pCashShop->cCount; i++){
			if (g_pDataManager->IsReserveCommodity(pCashShop->BuyList[i].nItemSN)) continue;
			if (pCashShop->BuyList[i].nItemID != 0) continue;	// Prevent purchase if 0

			return ERROR_CASHSHOP_RESERVE_DISABLE;
		}

		if (!CheckEnoughPetal(nTotalPrice))
			return ERROR_CASHSHOP_RESERVE_NOTENOUGH;	// Not enough money to buy
	}
	else{
		if (!CheckEnoughCashByPaymentRules(nTotalPrice, cPaymentRules))
			return ERROR_CASHSHOP_CASH_NOTENOUGH;
	}

	if (!g_pCashConnection->GetActive())
		return ERROR_GENERIC_INVALIDREQUEST;

#if defined(_VILLAGESERVER) && defined(PRE_ADD_CASH_REFUND)
	m_pItem->SetCashMoveInven(pCashShop->bMoveCashInven);
#endif

	g_pCashConnection->SendPackageBuy(m_pSession, pCashShop, PackageList);

	return ERROR_NONE;
}

int CDNUserBase::_OnCashCheckReceiver(CSCashShopCheckReceiver *pCashShop)
{
	if( !IsWindowState(WINDOW_CASHSHOP) )
		return ERROR_GENERIC_INVALIDREQUEST;

	if (CheckLastSpace(pCashShop->wszToCharacterName))
		return ERROR_GENERIC_INVALIDREQUEST;

	if (__wcsicmp_l(pCashShop->wszToCharacterName, GetCharacterName()) == 0)		// Compare names and return if sent to self
		return ERROR_CASHSHOP_DONTSENDYOURSELF;

	m_pDBCon->QueryCheckGiftReceiver(m_pSession, pCashShop->wszToCharacterName);

	return ERROR_NONE;
}

#if defined(PRE_ADD_CADGE_CASH)
int CDNUserBase::_OnCashCadge(CSCashShopCadge *pCashShop)
{
	if (!IsWindowState(WINDOW_CASHSHOP))
		return ERROR_GENERIC_INVALIDREQUEST;

	if (pCashShop->cCount <= 0 || pCashShop->cCount > PREVIEWCARTLISTMAX){
		g_Log.Log(LogType::_NORMAL, m_pSession, L"Invalid cash cadge count(%d)\r\n", pCashShop->cCount);
		return ERROR_ITEM_FAIL;
	}

	if (pCashShop->nPackageSN > 0){
		if (!g_pDataManager->IsPresentCommodity(pCashShop->nPackageSN))	// Non-giftable item
			return ERROR_GIFT_NOTFORSALE;
	}
	else{
		for (int i = 0; i < pCashShop->cCount; i++){
			if (!g_pDataManager->IsPresentCommodity(pCashShop->PackageGiftList[i].nItemSN))	// Non-giftable item
				return ERROR_GIFT_NOTFORSALE;
		}
	}

	m_pDBCon->QuerySendWishMail(m_pSession, pCashShop);

	return ERROR_NONE;
}
#endif	// #if defined(PRE_ADD_CADGE_CASH)

int CDNUserBase::_OnCashGift(CSCashShopGift *pCashShop)
{
	if (!g_pCashConnection->GetActive())
		return ERROR_GENERIC_INVALIDREQUEST;

	if( !IsWindowState(WINDOW_CASHSHOP) )
		return ERROR_GENERIC_INVALIDREQUEST;


	if (pCashShop->cCount <= 0 || pCashShop->cCount > PREVIEWCARTLISTMAX){
		g_Log.Log(LogType::_NORMAL, m_nWorldSetID, m_nAccountDBID, m_biCharacterDBID, m_nSessionID, L"Invalid cash gift count(%d)\r\n", pCashShop->cCount);
		return ERROR_ITEM_FAIL;
	}

	int nRet = 0;
	TCashGiftItem CashGiftList[PREVIEWCARTLISTMAX] = {0, };
	for (int i = 0; i < pCashShop->cCount; i++){
		if (!g_pDataManager->IsPresentCommodity(pCashShop->GiftList[i].nItemSN))	// Non-giftable item
			return ERROR_GIFT_NOTFORSALE;
		if( !g_pDataManager->CheckCashSNItemID(pCashShop->GiftList[i].nItemSN, pCashShop->GiftList[i].nItemID) )
			return ERROR_ITEM_NOTFOUND;
#if defined(PRE_ADD_SALE_COUPON)
		// Just in case an item requiring a Sale coupon was sent as a gift..
		if( g_pDataManager->IsSaleCouponByItem(pCashShop->GiftList[i].nItemSN))
			return ERROR_ITEM_FAIL;
#endif//#if defined(PRE_ADD_SALE_COUPON)

		CashGiftList[i].cCartIndex = pCashShop->GiftList[i].cSlotIndex;
		CashGiftList[i].nItemSN = pCashShop->GiftList[i].nItemSN;
		CashGiftList[i].nItemID = pCashShop->GiftList[i].nItemID;
		CashGiftList[i].cItemOption = pCashShop->GiftList[i].nOptionIndex;
	}

	int nTotalPrice = 0, nPrice = 0;
	for (int i = 0; i < pCashShop->cCount; i++){
		if (pCashShop->GiftList[i].nItemSN <= 0) continue;
		nPrice = g_pDataManager->GetCashCommodityPrice(pCashShop->GiftList[i].nItemSN);
		if (nPrice <= 0) continue;
		nTotalPrice += nPrice;
	}

	if (!CheckEnoughCashBalance(nTotalPrice))	// Not enough money to buy
		return ERROR_CASHSHOP_CASH_NOTENOUGH;

	g_pCashConnection->SendGift(m_pSession, pCashShop, CashGiftList);

	return ERROR_NONE;
}

int CDNUserBase::_OnCashPackageGift(CSCashShopPackageGift *pCashShop)
{
	if (!IsWindowState(WINDOW_CASHSHOP))
		return ERROR_GENERIC_INVALIDREQUEST;


	if (!g_pCashConnection->GetActive())
		return ERROR_GENERIC_INVALIDREQUEST;

	if (pCashShop->cCount <= 0 || pCashShop->cCount > PACKAGEITEMMAX){	// No products available
		g_Log.Log(LogType::_NORMAL, m_nWorldSetID, m_nAccountDBID, m_biCharacterDBID, m_nSessionID, L"Invalid cash package gift count(%d)\r\n", pCashShop->cCount);
		return ERROR_ITEM_FAIL;
	}

	if (!g_pDataManager->IsPresentCommodity(pCashShop->nPackageSN))	// Non-giftable item
		return ERROR_GIFT_NOTFORSALE;

	TCashPackageData PackageData;
	bool bPackage = g_pDataManager->GetCashPackageData(pCashShop->nPackageSN, PackageData);
	if (!bPackage)
		return ERROR_ITEM_NOTFOUND;

#if defined(PRE_ADD_SALE_COUPON)
	// Just in case an item requiring a Sale coupon was sent as a gift..
	if( g_pDataManager->IsSaleCouponByItem(pCashShop->nPackageSN))
		return ERROR_ITEM_FAIL;
#endif//#if defined(PRE_ADD_SALE_COUPON)

	int nRet = 0;
	TCashPackageGiftItem PackageList[PACKAGEITEMMAX];
	for (int i = 0; i < pCashShop->cCount; i++){
		if( (int)(PackageData.nVecCommoditySN.size()) <= i || PackageData.nVecCommoditySN[i] != pCashShop->PackageGiftList[i].nItemSN )
			return ERROR_ITEM_NOTFOUND;
		if( !g_pDataManager->CheckCashSNItemID(pCashShop->PackageGiftList[i].nItemSN, pCashShop->PackageGiftList[i].nItemID ))
			return ERROR_ITEM_NOTFOUND;
		PackageList[i].nItemSN = pCashShop->PackageGiftList[i].nItemSN;
		PackageList[i].nItemID = pCashShop->PackageGiftList[i].nItemID;
		PackageList[i].cItemOption = pCashShop->PackageGiftList[i].nOptionIndex;
	}

	if (pCashShop->nPackageSN <= 0)
		return ERROR_ITEM_FAIL;

	int nTotalPrice = g_pDataManager->GetCashCommodityPrice(pCashShop->nPackageSN);

	if (!CheckEnoughCashBalance(nTotalPrice))	// Not enough money to buy
		return ERROR_CASHSHOP_CASH_NOTENOUGH;

	g_pCashConnection->SendPackageGift(m_pSession, pCashShop, PackageList);

	return ERROR_NONE;
}

int CDNUserBase::_OnCashCoupon(CSCashShopCoupon *pCashShop)
{
	if (!g_pCashConnection->GetActive())
		return ERROR_GENERIC_INVALIDREQUEST;
	if (!IsWindowState(WINDOW_CASHSHOP))
		return ERROR_GENERIC_INVALIDREQUEST;

	g_pCashConnection->SendCoupon(m_pSession, pCashShop->wszCoupon);

	return ERROR_NONE;
}

#if defined(PRE_ADD_VIP)
int CDNUserBase::_OnCashVIPBuy(CSVIPBuy *pCashShop)
{
	if (!g_pCashConnection->GetActive())
		return ERROR_GENERIC_INVALIDREQUEST;

	if (!IsWindowState(WINDOW_CASHSHOP))
		return ERROR_GENERIC_INVALIDREQUEST;

	g_pCashConnection->SendVIPBuy(m_pSession, pCashShop->nItemSN, false);

	return ERROR_NONE;
}

int CDNUserBase::_OnCashVIPGift(CSVIPGift *pCashShop)
{
	if (!g_pCashConnection->GetActive())
		return ERROR_GENERIC_INVALIDREQUEST;

	if (!IsWindowState(WINDOW_CASHSHOP))
		return ERROR_GENERIC_INVALIDREQUEST;

	g_pCashConnection->SendVIPGift(m_pSession, pCashShop);

	return ERROR_NONE;
}
#endif	// #if defined(PRE_ADD_VIP)

#if defined(_VILLAGESERVER) && defined(PRE_ADD_CASH_REFUND)
int CDNUserBase::_OnMoveCashInven(CSMoveCashInven* pCashShop)
{
	if (!g_pCashConnection->GetActive())
		return ERROR_GENERIC_INVALIDREQUEST;

	if (!IsWindowState(WINDOW_CASHSHOP))
		return ERROR_GENERIC_INVALIDREQUEST;

	if ( pCashShop->cItemType == 1) // Normal item
	{
		TPaymentItemInfoEx* pPaymentItem = m_pItem->GetPaymentItem(pCashShop->biDBID);
		if ( pPaymentItem == NULL )
			return ERROR_GENERIC_INVALIDREQUEST;

		TCashItemBase CashItem;
		memset(&CashItem, 0, sizeof(TItem));

		int nRet = m_pItem->MakeBuyCashItem(pPaymentItem->PaymentItemInfo.ItemInfo.nItemSN, pPaymentItem->PaymentItemInfo.ItemInfo.nItemID, pPaymentItem->PaymentItemInfo.ItemInfo.cItemOption, CashItem);
		if (nRet != ERROR_NONE)
			return nRet;

		g_pCashConnection->SendMoveCashInven(m_nAccountDBID, m_biCharacterDBID, GetWorldSetID(), GetMapIndex(), m_nChannelID, &CashItem, pPaymentItem);
	}
	else if( pCashShop->cItemType == 2) // Package
	{
		TPaymentPackageItemInfoEx* pPaymentPackageItem = m_pItem->GetPaymentPackageItem(pCashShop->biDBID);
		if( pPaymentPackageItem == NULL )
			return ERROR_GENERIC_INVALIDREQUEST;

		TCashItemBase PackageList[PACKAGEITEMMAX];
		for (int i = 0; i < PACKAGEITEMMAX; i++)
		{
			if( pPaymentPackageItem->PaymentPackageItemInfo.ItemInfoList[i].nItemID == 0)
				break;

			int nRet = m_pItem->MakeBuyCashItem(pPaymentPackageItem->PaymentPackageItemInfo.ItemInfoList[i].nItemSN, pPaymentPackageItem->PaymentPackageItemInfo.ItemInfoList[i].nItemID, 
				pPaymentPackageItem->PaymentPackageItemInfo.ItemInfoList[i].cItemOption, PackageList[i]);
			if (nRet != ERROR_NONE)
				return nRet;
		}

		g_pCashConnection->SendPackageMoveCashInven(m_nAccountDBID, m_biCharacterDBID, GetWorldSetID(), GetMapIndex(), m_nChannelID, PackageList, pPaymentPackageItem);
	}

	return ERROR_NONE;
}

int CDNUserBase::_OnCashRefund(CSCashRefund* pCashShop)
{
	if (!g_pCashConnection->GetActive())
		return ERROR_GENERIC_INVALIDREQUEST;
	if( !IsWindowState(WINDOW_CASHSHOP) )
		return ERROR_GENERIC_INVALIDREQUEST;

	if( pCashShop->cItemType == 1) // Normal item
	{
		TPaymentItemInfoEx* pPaymentItem = m_pItem->GetPaymentItem(pCashShop->biDBID);
		if( pPaymentItem == NULL )
			return ERROR_GENERIC_INVALIDREQUEST;

		// Period check is done on the cash server..
		g_pCashConnection->SendCashRefund(m_nAccountDBID, pCashShop->cItemType, GetWorldSetID(), pPaymentItem->PaymentItemInfo.ItemInfo.nItemSN,
			pPaymentItem->uiOrderNo, pPaymentItem->PaymentItemInfo.biDBID);
	}
	else if( pCashShop->cItemType == 2) // Package
	{
		TPaymentPackageItemInfoEx* pPaymentPackageItem = m_pItem->GetPaymentPackageItem(pCashShop->biDBID);
		if( pPaymentPackageItem == NULL )
			return ERROR_GENERIC_INVALIDREQUEST;

		// Period check is done on the cash server..
		g_pCashConnection->SendCashRefund(m_nAccountDBID, pCashShop->cItemType, GetWorldSetID(), pPaymentPackageItem->PaymentPackageItemInfo.nPackageSN,
			pPaymentPackageItem->uiOrderNo, pPaymentPackageItem->PaymentPackageItemInfo.biDBID);
	}
	return ERROR_NONE;
}
#endif	// #if defined(PRE_ADD_CASH_REFUND)

// Add
void CDNUserBase::AddWindowState( eWindowState AddState )
{
	m_sUserWindowState = m_sUserWindowState | AddState;
}

void CDNUserBase::SetWindowState( eWindowState SetState )
{
	m_sUserWindowState = SetState;
}

// Whether the state is included.
bool CDNUserBase::IsWindowState( eWindowState InclusionState) const
{
	if( m_sUserWindowState & InclusionState )
		return true;
	return false;
}
// Add State only when None
bool CDNUserBase::IsNoneWindowStateAdd( eWindowState AddState )
{
	if( m_sUserWindowState == WINDOW_NONE )
	{
		m_sUserWindowState = m_sUserWindowState | AddState;
		return true;
	}
	return false;
}
bool CDNUserBase::IsNoneWindowStateSet( eWindowState SetState )
{
	if( m_sUserWindowState == WINDOW_NONE )
	{
		m_sUserWindowState = SetState;
		return true;
	}
	return false;
}
// Set None only when the state is included
void CDNUserBase::IsWindowStateNoneSet( eWindowState InclusionState)
{
	if( m_sUserWindowState & InclusionState )	
		m_sUserWindowState = WINDOW_NONE;	
}
void CDNUserBase::SetNoneWindowState()
{
	m_sUserWindowState = WINDOW_NONE;
}
bool CDNUserBase::IsNoneWindowState() const
{
	return m_sUserWindowState == WINDOW_NONE;
}

int CDNUserBase::OnRecvWarpVillageList(const CSWarpVillageList* pPacket)
{
	if (!m_pItem)
	{
		SendWarpVillageResult(ERROR_ITEM_FAIL);
		return ERROR_NONE;
	}

	if (!m_pSession->IsNoneWindowState())
		return ERROR_ITEM_FAIL;

	int nResult = m_pItem->CheckWarpVillage(pPacket->nItemSerial);
	if (nResult != ERROR_NONE)
	{
		SendWarpVillageResult(nResult);
		return ERROR_NONE;
	}

	vector<WarpVillage::WarpVillageInfo> vList;
	vector<int> vVillageMaps = g_pDataManager->GetVillageMaps();

	for each (int nMapID in vVillageMaps)
	{
		int nLevel = g_pDataManager->GetMapPermitLevel(nMapID);
		if (nLevel < 0)
		{
			const TChannelInfo* pChannelInfo = g_pDataManager->GetChannelInfo(m_pSession->GetWorldSetID(), nMapID);
			if (!pChannelInfo)
				continue;
			if (pChannelInfo->nAttribute & (GlobalEnum::CHANNEL_ATT_PVP | GlobalEnum::CHANNEL_ATT_FARMTOWN))
				nLevel = pChannelInfo->nLimitLevel;
			else
				continue;
		}

		vList.push_back(WarpVillage::WarpVillageInfo(nMapID, static_cast<BYTE>(nLevel)));
	}

	SetWindowState(WINDOW_BLIND);
	m_pSession->SendWarpVillageList(vList);

	return ERROR_NONE;
}

int CDNUserBase::OnRecvWarpVillage(const CSWarpVillage* pPacket)
{
	if (!m_pItem)
	{
		SendWarpVillageResult(ERROR_ITEM_FAIL);
		return ERROR_NONE;
	}

	int nResult = m_pItem->TryWarpVillage(pPacket->nMapIndex, pPacket->nItemSerial);
	if (nResult != ERROR_NONE)
		SendWarpVillageResult(nResult);

	return ERROR_NONE;
}

bool CDNUserBase::DeleteWarpVillageItemByUse(INT64 nItemSerial)
{
	if (nItemSerial > 0)
	{
		const TItem* pItem = m_pItem->GetCashInventory(nItemSerial);
		if (!pItem)
			return false;

		const TItemData* pItemData = g_pDataManager->GetItemData(pItem->nItemID);
		if (!pItemData)
			return false;

		switch (pItemData->nType)
		{
		case ITEMTYPE_FREE_PASS:
			if (!m_pItem->DeleteItemByUse(ITEMPOSITION_CASHINVEN, -1, nItemSerial))
				return false;
			break;

		case  ITEMTYPE_UNLIMITED_FREE_PASS:
			break;

		default:
			return false;
		}
	}

	return true;
}

bool CDNUserBase::AddDataBaseMessageSequence(char cMainCmd, char cSubCmd)
{
	if (CheckDataBaseMessageSequence(cMainCmd, cSubCmd))
		return false;

	TDatabaseMessageSequence seq;
	memset(&seq, 0, sizeof(TDatabaseMessageSequence));

	seq.cMainCmd = cMainCmd;
	seq.cSubCmd = cSubCmd;
	seq.dwAddTime = timeGetTime();

	m_vDBMessageSequencer.push_back(seq);
	return true;
}

void CDNUserBase::DelDataBaseMessageSequence(char cMainCmd, char cSubCmd)
{
	if (m_vDBMessageSequencer.empty()) return;
	std::vector <TDatabaseMessageSequence>::iterator ii;
	for (ii = m_vDBMessageSequencer.begin(); ii != m_vDBMessageSequencer.end(); ii++)
	{
		if ((*ii).cMainCmd == cMainCmd && (*ii).cSubCmd == cSubCmd)
		{
			m_vDBMessageSequencer.erase(ii);
			return;
		}
	}
}

bool CDNUserBase::CheckDataBaseMessageSequence(char cMainCmd, char cSubCmd)
{
	std::vector <TDatabaseMessageSequence>::iterator ii;
	for (ii = m_vDBMessageSequencer.begin(); ii != m_vDBMessageSequencer.end(); ii++)
	{
		if ((*ii).cMainCmd == cMainCmd && (*ii).cSubCmd == cSubCmd)
		{
			if ((*ii).dwAddTime + (1000*60) <= timeGetTime())
			{
				m_vDBMessageSequencer.erase(ii);
				return false;
			}
			return true;
		}
	}
	return false;
}

#if defined (PRE_ADD_DONATION)
int CDNUserBase::OnRecvDonation(int nSubCmd, char* pData, int nLen)
{
	if (!CheckDBConnection())	
		return ERROR_GENERIC_DBCON_NOT_FOUND;

	switch (nSubCmd)
	{
	case eDonation::CS_DONATE:
		{
			if (sizeof(CSDonate) != nLen)
				return ERROR_INVALIDPACKET;

			CSDonate* pPacket = reinterpret_cast<CSDonate*>(pData);

			CTimeSet CurTime;
			if (CurTime.GetDay() == 1 && CurTime.GetHour() == 0)
			{
				SendDonationResult(ERROR_NOT_AVAILABLE_DONATION_TIME);
				return ERROR_NONE;
			}

			if (0 < pPacket->nCoin % 10000)
				return ERROR_GENERIC_INVALIDREQUEST;

			if (!CheckEnoughCoin(pPacket->nCoin))
				return ERROR_MAIL_INSUFFICIENCY_MONEY;

			DelCoin(pPacket->nCoin, DBDNWorldDef::CoinChangeCode::Donation, 0, true);
			m_pDBCon->QueryDonate(m_pSession, CurTime.GetTimeT64_LC(), pPacket->nCoin);
		}
		break;

	case eDonation::CS_DONATION_RANKING:
		{
			if (sizeof(CSDonationRanking) != nLen)
				return ERROR_INVALIDPACKET;

			m_pDBCon->QueryDonationRanking(m_pSession);
		}
		break;
	}

	return ERROR_UNKNOWN_HEADER;
}
#endif // #if defined (PRE_ADD_DONATION)

void CDNUserBase::SendLevelupEventMail(int nLevel, int nClass, int nJob)
{
	// Reward if an event is active
	std::vector<TLevelupEvent*> VecEventList;
	g_pDataManager->GetLevelupEvent(nLevel, nClass, nJob, VecEventList);
	if (VecEventList.empty()) return;

	for (int j = 0; j <(int)VecEventList.size(); j++){
		if ((VecEventList[j]->nEventType == TLevelupEvent::ShandaPromotion) && (!m_bIntroducer)) continue;		// If there is no recommender
		if ((VecEventList[j]->nEventType == TLevelupEvent::CombackUser)) continue;
		if ((VecEventList[j]->nEventType == TLevelupEvent::ComeBackUserInven)) continue;
#if defined(PRE_ADD_GAMEQUIT_REWARD)
		if ((VecEventList[j]->nEventType == TLevelupEvent::NewbieQuitReward)) continue;
		if ((VecEventList[j]->nEventType == TLevelupEvent::NewbieReconnectReward)) continue;
		if ((VecEventList[j]->nEventType == TLevelupEvent::ComeBackUserQuitReward)) continue;
#endif	// #if defined(PRE_ADD_GAMEQUIT_REWARD)
		if (VecEventList[j]->nEventType == TLevelupEvent::LevelupNew){
			INT64 biValue = 0;
			GetCommonVariableDataValue(CommonVariable::Type::LevelupEventNew, biValue);
			if (biValue > 0) return;

			ModCommonVariableData(CommonVariable::Type::LevelupEventNew, 1);
		}

		if (VecEventList[j]->nMailID > 0){
			CDNMailSender::Process(m_pSession, VecEventList[j]->nMailID, DBDNWorldDef::PayMethodCode::LevelupEvent, nLevel);
		}
		if (VecEventList[j]->nCashMailID > 0){
#if defined(PRE_LEVELUPREWARD_DIRECT)
			bool bSendResult = false;
			switch (VecEventList[j]->nEventType)
			{
			case TLevelupEvent::LevelupNew:
				{
					bSendResult = m_pItem->CreateCashInvenItemByMailID(VecEventList[j]->nCashMailID);
				}
				break;

			default:
				{
					bSendResult = CDNMailSender::Process(m_pSession, VecEventList[j]->nCashMailID, DBDNWorldDef::PayMethodCode::LevelupEvent, nLevel);
				}
				break;
			}
#else	// #if defined(PRE_LEVELUPREWARD_DIRECT)
			bool bSendResult = CDNMailSender::Process(m_pSession, VecEventList[j]->nCashMailID, DBDNWorldDef::PayMethodCode::LevelupEvent, nLevel);
#endif	// #if defined(PRE_LEVELUPREWARD_DIRECT)

#if defined(PRE_ADD_LEVELUP_CASHGIFT_NOTIFY)
			SendServerMsg( CHATTYPE_SYSTEM, CEtUIXML::idCategory1, SERVERMSG_INDEX626, 3, 3); // Caption3, 3 sec
#endif	// #if defined(PRE_ADD_LEVELUP_CASHGIFT_NOTIFY)

			g_Log.Log(LogType::_NORMAL, m_pSession, L"Send LevelupEvent CashMail(ADBID:%d, CashMainID:%d, Level:%d, SendResult:%d)\r\n", GetAccountDBID(), VecEventList[j]->nCashMailID, nLevel, bSendResult);
		}
	}
}

#if defined( PRE_ADD_BESTFRIEND )
void CDNUserBase::CloseBestFirend()
{
	if(m_pBestFriend)
		m_pBestFriend->Close();
}
#endif
#if defined( PRE_ADD_LIMITED_SHOP )
void CDNUserBase::ResetLimitedShopDailyCount()
{	
	DelLimitedShopBuyedItem(LimitedShop::LimitedItemType::Day);	
	GetDBConnection()->QueryResetLimitedShopItem(m_pSession, LimitedShop::LimitedItemType::Day);

	ResetLimitedShopWeeklyCount();
}
void CDNUserBase::ResetLimitedShopWeeklyCount()
{
	__time64_t tLastModifyDate = 0;
	m_pCommonVariable->GetDataModDate(CommonVariable::Type::LimitedShopWeek, tLastModifyDate);	

	tm pDate = *localtime(&tLastModifyDate);	

	int nWeek = CDNSchedule::Saturday - CDNSchedule::Sunday;
	int nValue;

	nValue = GetDateValue( 1900 + pDate.tm_year, pDate.tm_mon + 1, pDate.tm_mday ) - 4 - nWeek;
	int nDBValue = nValue / 7;
	if( nValue % 7 == 0 ) 
	{
		if( pDate.tm_hour < 0 ) nDBValue--;
	}

	time_t LocalTime;
	tm pLocalTime;

	time(&LocalTime);
	pLocalTime = *localtime(&LocalTime);

	nValue = GetDateValue( 1900 + pLocalTime.tm_year, pLocalTime.tm_mon + 1, pLocalTime.tm_mday ) - 4 - nWeek;
	int nCurValue = nValue / 7;
	if( nValue % 7 == 0 ) 
	{
		if( pLocalTime.tm_hour < 0 ) nCurValue--;
	}

	if( nDBValue != nCurValue )
	{
		DelLimitedShopBuyedItem(LimitedShop::LimitedItemType::Week);
		ModCommonVariableData(CommonVariable::Type::LimitedShopWeek, 0);
		GetDBConnection()->QueryResetLimitedShopItem(m_pSession, LimitedShop::LimitedItemType::Week);
	}
}

#if defined( PRE_FIX_74404 )
int CDNUserBase::GetLimitedShopBuyedItem(int nShopID, int nItemID)
{
	for(std::list<LimitedShop::LimitedItemData>::iterator itor=m_LimitedShopBuyedItemList.begin();itor!=m_LimitedShopBuyedItemList.end();itor++)
	{
		if( itor->nShopID == nShopID && itor->nItemID == nItemID )
			return itor->nBuyCount;
	}
	return 0;
}
void CDNUserBase::AddLimitedShopBuyedItem(int nShopID, int nItemID, int nBuyCount, int nResetCycle, bool bDBSend/*=true*/)
{
	LimitedShop::LimitedItemData ItemData;
	bool bFlag = false;
	for(std::list<LimitedShop::LimitedItemData>::iterator itor=m_LimitedShopBuyedItemList.begin();itor!=m_LimitedShopBuyedItemList.end();itor++)
	{
		if( itor->nShopID == nShopID && itor->nItemID == nItemID )
		{			
			itor->nBuyCount += nBuyCount;
			ItemData = *itor;
			bFlag = true;
		}
	}

	if( !bFlag )
	{
		ItemData.nShopID = nShopID;
		ItemData.nItemID = nItemID;
		ItemData.nBuyCount = nBuyCount;
		ItemData.nResetCycle = nResetCycle;
		m_LimitedShopBuyedItemList.push_back(ItemData);
	}
	
	if(bDBSend)
	{
		SendLimitedShopItemData(ItemData.nShopID, ItemData.nItemID, ItemData.nBuyCount, ItemData.nResetCycle);
		GetDBConnection()->QueryAddLimitedShopItem(m_pSession, ItemData, nBuyCount);
	}
}
void CDNUserBase::DelLimitedShopBuyedItem(int nResetCycle)
{
	bool bSend = false;
	for(std::list<LimitedShop::LimitedItemData>::iterator itor=m_LimitedShopBuyedItemList.begin();itor!=m_LimitedShopBuyedItemList.end();)
	{
		if(itor->nResetCycle == nResetCycle)
		{
			itor = m_LimitedShopBuyedItemList.erase(itor);
			bSend = true;
		}
		else
			itor++;		
	}

	if(bSend)
	{
		SendLimitedShopItemData(m_LimitedShopBuyedItemList, true);
	}
}
#else // #if defined( PRE_FIX_74404 )
int CDNUserBase::GetLimitedShopBuyedItem(int nItemID)
{
	std::map<int, LimitedShop::LimitedItemData>::iterator itor = m_LimitedShopBuyedItem.find(nItemID);
	if(itor != m_LimitedShopBuyedItem.end())
		return itor->second.nBuyCount;
	return 0;
}
void CDNUserBase::AddLimitedShopBuyedItem(int nItemID, int nBuyCount, int nResetCycle, bool bDBSend/*=true*/)
{
	LimitedShop::LimitedItemData ItemData;
	std::map<int, LimitedShop::LimitedItemData>::iterator itor = m_LimitedShopBuyedItem.find(nItemID);
	if(itor != m_LimitedShopBuyedItem.end())
	{
		itor->second.nBuyCount += nBuyCount;
		ItemData = itor->second;
	}
	else
	{		
		ItemData.nItemID = nItemID;
		ItemData.nBuyCount = nBuyCount;
		ItemData.nResetCycle = nResetCycle;
		m_LimitedShopBuyedItem.insert(std::make_pair(nItemID,ItemData));
	}
	if(bDBSend)
	{
		SendLimitedShopItemData(ItemData.nItemID, ItemData.nBuyCount, ItemData.nResetCycle);
		GetDBConnection()->QueryAddLimitedShopItem(m_pSession, ItemData, nBuyCount);
	}
}
void CDNUserBase::DelLimitedShopBuyedItem(int nResetCycle)
{
	bool bSend = false;
	for(std::map<int, LimitedShop::LimitedItemData>::iterator itor = m_LimitedShopBuyedItem.begin();itor != m_LimitedShopBuyedItem.end();)
	{
		if(itor->second.nResetCycle == nResetCycle)
		{
			itor = m_LimitedShopBuyedItem.erase(itor);
			bSend = true;
		}
		else
			itor++;
	}
	if(bSend)
	{
		SendLimitedShopItemData(m_LimitedShopBuyedItem, true);
	}
}
#endif // #if defined( PRE_FIX_74404 )
#endif // #if defined( PRE_ADD_LIMITED_SHOP )

#if defined( PRE_ADD_NOTIFY_ITEM_COMPOUND )
void CDNUserBase::SetCompoundNotify(const CSCompoundNotify* pPacket)
{
	ModCommonVariableData(CommonVariable::Type::NotifyCompound, MAKELONG64(pPacket->nItemID, pPacket->nItemOption));
}
#endif

#if defined( PRE_PRIVATECHAT_CHANNEL )
int CDNUserBase::CheckInvalidChannelName(WCHAR* wszName)
{
	if( !wszName )
		return ERROR_INVALID_CHANNELNAMELENGTH;

	if(GetPrivateChannelID() != 0)
	{
		SendPrivateChatChannelResult(ERROR_DUPLICATE_CHANNELJOIN);
		return ERROR_DUPLICATE_CHANNELJOIN;
	}

	if( wcslen(wszName) < PrivateChatChannel::Common::MinNameLen ||  wcslen(wszName) >= PrivateChatChannel::Common::MaxNameLen )
	{
		SendPrivateChatChannelResult(ERROR_INVALID_CHANNELNAMELENGTH);
		return ERROR_INVALID_CHANNELNAMELENGTH;
	}

	DWORD dwCheckType = ALLOW_STRING_DEFAULT;
	if (!g_CountryUnicodeSet.Check(wszName, dwCheckType) || CheckLastSpace(wszName)) 
	{
		SendPrivateChatChannelResult(ERROR_INVALID_CHANNELNAME);
		return ERROR_INVALID_CHANNELNAME;
	}			

	if (g_pDataManager->CheckProhibitWord(wszName))
	{
		SendPrivateChatChannelResult(ERROR_INVALID_CHANNELNAME);
		return ERROR_INVALID_CHANNELNAME;
	}
	return ERROR_NONE;
}

void CDNUserBase::SendPrivateChatChannel( CDNPrivateChaChannel* pPrivateChatChannel )
{
	if( pPrivateChatChannel )
	{
		std::list<TPrivateChatChannelMember> MemberList;
		MemberList.clear();
		pPrivateChatChannel->GetPrivateChannelMember(MemberList);
		SendPrivateChatChannelInfo(pPrivateChatChannel->GetChannelInfo(), MemberList);
	}	
}
#endif

#if defined( PRE_ADD_TOTAL_LEVEL_SKILL )
bool CDNUserBase::AddTotalLevelSkillData(BYTE cSlotIndex, int nSkillID, bool isInitialize/* = false*/)
{
	if( cSlotIndex < 0 || cSlotIndex >= TotalLevelSkill::Common::MAXSLOTCOUNT )
	{
		return false;
	}

	if( g_pDataManager->bIsTotalLevelSkillCashSlot(cSlotIndex) )
	{
		if(!m_bTotalLevelSkillCashSlot[cSlotIndex])
			return false;
	}

	if(nSkillID > 0)
	{
		int nSkillType = g_pDataManager->GetTotalLevelSkillType(nSkillID);

		for(int i=0;i<TotalLevelSkill::Common::MAXSLOTCOUNT;i++)
		{
			if( i == cSlotIndex )
				continue;

			if(nSkillID == m_nTotalLevelSKillData[i])
				return false;

			if( g_pDataManager->bIsTotalLevelSkillCashSlot(i) )
				continue;

			if(nSkillType == g_pDataManager->GetTotalLevelSkillType(m_nTotalLevelSKillData[i]) && !m_bTotalLevelSkillCashSlot[cSlotIndex])
				return false;			
		}
	}

	if( nSkillID == 0 || g_pDataManager->bIsTotalLevelSkillLimitLevel( cSlotIndex, nSkillID, m_nTotalLevelSKillLevel, GetLevel() ) )
	{
#if defined( _GAMESERVER )
		if( m_pSession->GetPlayerActor() )
			m_pSession->GetPlayerActor()->RemoveTotalLevelSkill(cSlotIndex);
#endif		
		m_nTotalLevelSKillData[cSlotIndex] = nSkillID;	

		GetDBConnection()->QueryAddTotalSkillLevel(m_pSession, cSlotIndex, nSkillID);
		return true;
	}	
	return false;
}

void CDNUserBase::ResetTotalLevelSkillMedalCount()
{
	INT64 biValue  = 0;
	__time64_t tLastModifyDate = 0;
	m_pCommonVariable->GetDataValue(CommonVariable::Type::TotalLevelSkillMedal, biValue);
	m_pCommonVariable->GetDataModDate(CommonVariable::Type::TotalLevelSkillMedal, tLastModifyDate);
	
	ModCommonVariableData(CommonVariable::Type::TotalLevelSkillMedal, 0);	
}

int CDNUserBase::GetTotalLevelSkillMedalCount()
{
	INT64 biValue  = 0;	
	GetCommonVariableDataValue(CommonVariable::Type::TotalLevelSkillMedal, biValue);
	return (int)biValue;
}

void CDNUserBase::AddTotalLevelSkillMedalCount( int nMedalCount )
{
	INT64 biValue  = 0;	
	GetCommonVariableDataValue(CommonVariable::Type::TotalLevelSkillMedal, biValue);
	biValue += nMedalCount;	
	ModCommonVariableData(CommonVariable::Type::TotalLevelSkillMedal, biValue);	
}

float CDNUserBase::GetTotalLevelSkillEffect( TotalLevelSkill::Common::eVillageEffectType eType )
{
	float fValue = 0.0f;
	for(int i=0;i<TotalLevelSkill::Common::MAXSLOTCOUNT;i++)
	{
		if( m_nTotalLevelSKillData[i] > 0 )
		{
			fValue += g_pDataManager->GetTotalLevelSkillValue( m_nTotalLevelSKillData[i], eType );
		}
	}	
	return fValue;
}

int CDNUserBase::GetintTotalLevelSkillEffect( TotalLevelSkill::Common::eVillageEffectType eType )
{
	int nValue = 0;
	for(int i=0;i<TotalLevelSkill::Common::MAXSLOTCOUNT;i++)
	{
		if( m_nTotalLevelSKillData[i] > 0 )
		{
			nValue = g_pDataManager->GetintTotalLevelSkillValue( m_nTotalLevelSKillData[i], eType );
			if( nValue > 0 )
				break;
		}
	}	
	return nValue;
}
#endif

#if defined( PRE_ADD_GAMEQUIT_REWARD )
bool CDNUserBase::SetGameQuitRewardItem(TLevelupEvent::eEventType eType, DBDNWorldDef::AddMaterializedItem::eCode eLogType)
{	
	std::vector<TLevelupEvent*> VecEventList;
	g_pDataManager->GetLevelupEvent(GetLevel(), GetClassID(), GetUserJob(), VecEventList);
	if (VecEventList.empty()) return false;

	for(int i = 0; i < VecEventList.size(); i++)
	{
		if(VecEventList[i]->nEventType != eType) continue;

		TMailTableData* pMailData = g_pDataManager->GetMailTableData(VecEventList[i]->nCashMailID);
		if(!pMailData) return false;

		TCashCommodityData CashData;
		if(!g_pDataManager->GetCashCommodityData(pMailData->ItemSNArr[0], CashData)) return false;
			
		TItemData* pItem = g_pDataManager->GetItemData(CashData.nItemID[0]);
		if(!pItem) return false;
		if(m_pItem->CreateCashInvenItem(pItem->nItemID, 1, eLogType) != ERROR_NONE) return false;
		
		return true;
	}
	
	return false;
}
#endif	// #if defined( PRE_ADD_GAMEQUIT_REWARD )


int CDNUserBase::_CheckSaleCoupon(int nSaleCouponSN, INT64 biSaleCouponSerial, int nBuyItemSN)
{
	// Check if the item actually exists..
	const TItem* pCouponItem = m_pItem->GetCashInventory(biSaleCouponSerial);
	if( !pCouponItem )
		return ERROR_ITEM_FAIL;

	int nCouponItemID = g_pDataManager->GetCashCommodityItem0(nSaleCouponSN);
	if( pCouponItem->nItemID != nCouponItemID )
		return ERROR_ITEM_FAIL;

	int nCouponID = g_pDataManager->GetItemTypeParam1(nCouponItemID);
	if( !g_pDataManager->IsSaleCouponItemBuyItem(nCouponID, nBuyItemSN) )
		return ERROR_ITEM_FAIL;

	if (m_pItem->IsExpired(*pCouponItem)) // Check expiration period..
		return ERROR_ITEM_FAIL;

#if defined(PRE_ADD_SALE_COUPON_LEVEL_LIMIT)	//Check level restriction
	const TItemData* pItemData = g_pDataManager->GetItemData(pCouponItem->nItemID);
	if (!pItemData)
		return ERROR_ITEM_FAIL;

	if( GetLevel() < pItemData->cLevelLimit )
		return ERROR_ITEM_FAIL;
#endif

	return ERROR_NONE;
}

#if defined( PRE_ADD_DWC )
void CDNUserBase::OnDBRecvDWC( int nSubCmd, char* pData )
{
	switch( nSubCmd )
	{
	case QUERY_ADD_DWCTEAM:
		{
			const TAAddDWCTeam* pA = reinterpret_cast<const TAAddDWCTeam*>(pData);
			if( pA->nRetCode == ERROR_NONE )
			{
				m_nDWCTeamID = pA->nTeamID;
				_wcscpy(m_wszDWCTeamName, _countof(m_wszDWCTeamName), pA->wszTeamName, (int)wcsnlen(pA->wszTeamName, DWC::MAX_TEAMNAME_LEN - 1));
			}
			SendDWCCreateTeam( pA->nRetCode, pA->nTeamID, pA->wszTeamName );
		}
		break;

	case QUERY_ADD_DWCTEAMMEMBER:
		{
			const TAAddDWCTeamMember* pA = reinterpret_cast<const TAAddDWCTeamMember*>(pData);
			if( pA->nRetCode == ERROR_NONE )
			{
				m_nDWCTeamID = pA->nTeamID;
				if( m_pDBCon )
					m_pDBCon->QueryGetDWCTeamByChar( m_cDBThreadID, m_pSession, true );
			}
			SendDWCInviteMemberAck( pA->nRetCode, pA->wszCharacterName );
		}
		break;

	case QUERY_DEL_DWCTEAMMEMBER:
		{
			const TADelDWCTeamMember* pA = reinterpret_cast<const TADelDWCTeamMember*>(pData);
			if( pA->nRetCode == ERROR_NONE )
			{
				m_nDWCTeamID = 0;
				memset(m_wszDWCTeamName, 0, sizeof(m_wszDWCTeamName));
			}
			SendDWCLeaveTeam( pA->nRetCode, pA->nTeamID, pA->biCharacterDBID, pA->wszCharacterName );
		}
		break;

	case QUERY_GET_DWCTEAM_BY_CHAR:
		{
			const TAGetDWCTeam* pA = reinterpret_cast<const TAGetDWCTeam*>(pData);
			if( pA->nRetCode == ERROR_NONE && pA->TeamInfo.nTeamID > 0 )
			{
				m_nDWCTeamID = pA->TeamInfo.nTeamID;
				_wcscpy(m_wszDWCTeamName, _countof(m_wszDWCTeamName), pA->TeamInfo.wszTeamName, (int)wcsnlen(pA->TeamInfo.wszTeamName, DWC::MAX_TEAMNAME_LEN - 1));
			}
			else
			{
				m_nDWCTeamID = 0;
				memset(m_wszDWCTeamName, 0, sizeof(m_wszDWCTeamName));
			}
			SendDWCGetTeamInfo( pA->nRetCode, &pA->TeamInfo );
			if( pA->nRetCode == ERROR_NONE && pA->bNeedMemberList && m_nDWCTeamID > 0 && m_pDBCon )
				m_pDBCon->QueryGetListDWCTeamMember( m_pSession, m_nDWCTeamID );
		}
		break;

	case QUERY_GETLIST_DWCTEAMMEMBER:
		{
			const TAGetListDWCTeamMember* pA = reinterpret_cast<const TAGetListDWCTeamMember*>(pData);
			std::vector<TDWCTeamMemberInfo> vecMembers;
			int nCount = (pA->nCount > DWC::MAX_TEAM_MEMBER) ? DWC::MAX_TEAM_MEMBER : pA->nCount;
			for( int i = 0; i < nCount; ++i )
			{
				TDWCTeamMemberInfo m = pA->MemberList[i];
#if defined( _VILLAGESERVER )
				CDNUserSession* pTarget = g_pUserSessionManager->FindUserSessionByName(m.wszCharacterName);
				if( pTarget )
				{
					m.cLevel = (BYTE)pTarget->GetLevel();
					m.nMapIndex = pTarget->GetMapIndex();
					m.nChannelID = pTarget->GetChannelID();
					m.cConnectState = 1;
				}
#endif
				vecMembers.push_back( m );
			}
			SendDWCGetTeamMember( pA->nRetCode, pA->nTeamID, vecMembers );
		}
		break;

	case QUERY_GETPAGE_DWCRANKING:
		{
			const TAGetPageDWCRanking* pA = reinterpret_cast<const TAGetPageDWCRanking*>(pData);
			std::vector<TDWCRankInfo> vecRanks;
			int nCount = (pA->nCount > DWC::MAX_RANK_PER_PAGE) ? DWC::MAX_RANK_PER_PAGE : pA->nCount;
			for( int i = 0; i < nCount; ++i )
				vecRanks.push_back( pA->RankList[i] );
			SendDWCGetRankPage( pA->nRetCode, pA->nTotalCount, pA->nPageNum, vecRanks );
		}
		break;

	case QUERY_GET_DWCRANKING:
		{
			const TAGetDWCRanking* pA = reinterpret_cast<const TAGetDWCRanking*>(pData);
			SendDWCFindRank( pA->nRetCode, &pA->RankInfo );
		}
		break;

	case QUERY_GET_DWCCHANNEL:
		{
			const TAGetDWCChannel* pA = reinterpret_cast<const TAGetDWCChannel*>(pData);
			if( pA->nRetCode == ERROR_NONE )
				SendDWCChannelInfo( pA->ChannelInfo );
		}
		break;
	}
}

int CDNUserBase::OnRecvDWCMessage( int nSubCmd, char* pData, int nLen )
{
	switch( nSubCmd )
	{
	case eDWC::CS_DWC_CREATETEAM:
		{
			if( sizeof(CSDWCCreateTeam) != nLen )
				return ERROR_INVALIDPACKET;
			CSDWCCreateTeam *pPacket = (CSDWCCreateTeam*)pData;
			WCHAR wszName[DWC::MAX_TEAMNAME_LEN] = { 0 };
			_wcscpy( wszName, _countof(wszName), pPacket->wszTeamName, (int)wcsnlen(pPacket->wszTeamName, DWC::MAX_TEAMNAME_LEN - 1) );
			if( wcslen(wszName) == 0 )
			{
				SendDWCCreateTeam( ERROR_GENERIC_INVALIDREQUEST, 0, L"" );
				return ERROR_NONE;
			}
			if( m_pDBCon )
				m_pDBCon->QueryAddDWCTeam( m_pSession, wszName );
			else
				SendDWCCreateTeam( ERROR_DB, 0, wszName );
			return ERROR_NONE;
		}
		break;

	case eDWC::CS_DWC_LEAVETEAM:
		{
			if( m_nDWCTeamID > 0 && m_pDBCon )
				m_pDBCon->QueryDelDWCTeamMember( m_pSession, m_nDWCTeamID );
			else
				SendDWCLeaveTeam( ERROR_GENERIC_INVALIDREQUEST, 0, GetCharacterDBID(), GetCharacterName() );
			return ERROR_NONE;
		}
		break;

	case eDWC::CS_DWC_INVITE_MEMBER:
		{
			if( sizeof(CSDWCInviteMember) != nLen )
				return ERROR_INVALIDPACKET;
			CSDWCInviteMember *pPacket = (CSDWCInviteMember*)pData;
			WCHAR wszTarget[NAMELENMAX] = { 0 };
			_wcscpy( wszTarget, _countof(wszTarget), pPacket->wszCharacterName, (int)wcsnlen(pPacket->wszCharacterName, NAMELENMAX - 1) );
			if( m_nDWCTeamID == 0 )
			{
				SendDWCInviteMemberAck( ERROR_GENERIC_INVALIDREQUEST, wszTarget );
				return ERROR_NONE;
			}
#if defined( _VILLAGESERVER )
			CDNUserSession* pTarget = g_pUserSessionManager->FindUserSessionByName( wszTarget );
			if( pTarget )
			{
				pTarget->SendDWCInviteMemberReq( m_nDWCTeamID, m_nSessionID, m_wszDWCTeamName, GetCharacterName() );
				return ERROR_NONE;
			}
#endif
			SendDWCInviteMemberAck( ERROR_GENERIC_INVALIDREQUEST, wszTarget );
			return ERROR_NONE;
		}
		break;

	case eDWC::CS_DWC_INVITE_MEMBER_ACK:
		{
			if( sizeof(CSDWCInviteMemberAck) != nLen )
				return ERROR_INVALIDPACKET;
			CSDWCInviteMemberAck *pPacket = (CSDWCInviteMemberAck*)pData;
			if( pPacket->bAccept && pPacket->nTeamID > 0 && m_pDBCon )
			{
				m_pDBCon->QueryAddDWCTeamMember( m_pSession, pPacket->nTeamID, GetCharacterDBID(), GetCharacterName() );
			}
			else
			{
#if defined( _VILLAGESERVER )
				CDNUserSession* pFrom = g_pUserSessionManager->FindUserSession( pPacket->nFromSessionID );
				if( pFrom )
					pFrom->SendDWCInviteMemberAck( ERROR_GENERIC_INVALIDREQUEST, GetCharacterName() );
#endif
			}
			return ERROR_NONE;
		}
		break;

	case eDWC::CS_DWC_GET_TEAMINFO:
		{
			if( sizeof(CSDWCGetTeamInfo) != nLen )
				return ERROR_INVALIDPACKET;
			CSDWCGetTeamInfo *pPacket = (CSDWCGetTeamInfo*)pData;
			if( m_pDBCon )
				m_pDBCon->QueryGetDWCTeamByChar( m_cDBThreadID, m_pSession, pPacket->bNeedMemberList );
			else
			{
				TDWCTeamInfo info = { 0, };
				SendDWCGetTeamInfo( ERROR_NONE, &info );
			}
			return ERROR_NONE;
		}
		break;

	case eDWC::CS_DWC_GET_RANKPAGE:
		{
			if( sizeof(CSDWCGetRankPage) != nLen )
				return ERROR_INVALIDPACKET;
			CSDWCGetRankPage *pPacket = (CSDWCGetRankPage*)pData;
			if( m_pDBCon )
				m_pDBCon->QueryGetPageDWCRanking( m_pSession, pPacket->nPageNum, DWC::MAX_RANK_PER_PAGE );
			else
			{
				std::vector<TDWCRankInfo> vecRanks;
				SendDWCGetRankPage( ERROR_NONE, 0, pPacket->nPageNum, vecRanks );
			}
			return ERROR_NONE;
		}
		break;

	case eDWC::CS_DWC_FIND_RANK:
		{
			if( sizeof(CSDWCFindRank) != nLen )
				return ERROR_INVALIDPACKET;
			CSDWCFindRank *pPacket = (CSDWCFindRank*)pData;
			WCHAR wszFind[DWC::MAX_FIND_NAME_LEN] = { 0 };
			_wcscpy( wszFind, _countof(wszFind), pPacket->wszFindName, (int)wcsnlen(pPacket->wszFindName, DWC::MAX_FIND_NAME_LEN - 1) );
			if( m_pDBCon )
				m_pDBCon->QueryGetDWCRanking( m_pSession, pPacket->cFindType, wszFind );
			else
			{
				TDWCRankInfo info = { 0, };
				SendDWCFindRank( ERROR_NONE, &info );
			}
			return ERROR_NONE;
		}
		break;
	}

	return ERROR_UNKNOWN_HEADER;
}
#endif // #if defined( PRE_ADD_DWC )
