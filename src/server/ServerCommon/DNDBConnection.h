#pragma once

#include "Connection.h"


/*---------------------------------------------------------------------------------------
									CDNDBConnection

Modified commonly for Village and Game.
Must pass cWorldSetID as a parameter to the function because of the Game server
---------------------------------------------------------------------------------------*/

#ifdef _GAMESERVER
class CDNRUDPGameServer;
#endif

class CDNUserItem;
class CGrowingArea;
#if defined( PRE_PARTY_DB ) && defined( _VILLAGESERVER )
#include "DNParty.h"
#endif
class CDNDBConnection: public CConnection
{
private:
#if defined( _GAMESERVER )
	CDNRUDPGameServer*	_GetGameServer( int iMainCmd, int iSubCmd, char* pData );
#endif //#if defined( _GAMESERVER )

public:
	CDNDBConnection(void);
	virtual ~CDNDBConnection(void);

	virtual int AddSendData(int iMainCmd, int iSubCmd, char *pData, int iLen, BYTE cSeq = 0);

	void Reconnect();

	void GetDBSID();
	int m_nServerID;

	int MessageProcess(int nMainCmd, int nSubCmd, char *pData, int nLen);
	bool PreMessageProcess(int nMainCmd, int nSubCmd, char * pData, int nLen);
	bool OnRecvAuth(int nSubCmd, char *pData);
	bool OnRecvEtc(int nSubCmd, char *pData);
	bool OnRecvQuest(int nSubCmd, char *pData);
	bool OnRecvGuild(int nSubCmd, char *pData);
	bool OnRecvMasterSystem( int nSubCmd, char* pData );
#if defined( PRE_ADD_NPC_REPUTATION_SYSTEM )
	bool OnRecvMail(int nSubCmd, char *pData);
#endif // #if defined( PRE_ADD_NPC_REPUTATION_SYSTEM )
	bool OnDBRecvFarm(int nSubCmd, char * pData, int nLen);
	bool OnDBRecvDBJobSystem(int nSubCmd, char* pData);
	bool OnRecvGuildRecruit(int nSubCmd, char* pData);
#if defined (PRE_ADD_DONATION)
	bool OnRecvDonation(int nSubCmd, char* pData, int nLen);
#endif // #if defined (PRE_ADD_DONATION)
#if defined( PRE_PARTY_DB )
	bool OnRecvParty( int nSubCmd, char* pData, int nLen );
#endif // #if defined( PRE_PARTY_DB ) && defined( _VILLAGESERVER )
#ifdef PRE_MOD_PVPRANK
	bool OnRecvPvP( int nSubCmd, char* pData, int nLen );
#endif		//#ifdef PRE_MOD_PVPRANK
#if defined( PRE_WORLDCOMBINE_PVP ) && !defined(PRE_MOD_PVPRANK)
	bool OnRecvPvP( int nSubCmd, char* pData, int nLen );
#endif
#if defined( PRE_PRIVATECHAT_CHANNEL )
	bool OnRecvPrivateChannel( int nSubCmd, char* pData, int nLen );
#endif

	int QueryMWTest( TQMWTest* pQ );

	// MAINCMD_AUTH
	int QueryStoreAuth(char cLastServerType, CDNUserSession *pUserSession);
	int QueryCheckAuth(INT64 biCertifyingKey, CDNUserSession *pUserSesion);
	int QueryCheckAuth(INT64 biCertifyingKey, BYTE cThreadID, int nAccountDBID, int nSessionID);
	int QueryResetAuth(BYTE cThreadID,int uAccountDBID, UINT nSessionID);
	int QueryResetAuthServer(BYTE cThreadID,int nServerID);
	int QueryResetErrorCheck( UINT uAccountDBID );
	
	// MAINCMD_STATUS
	// Character related
#if defined( PRE_PARTY_DB )
	void QuerySelectCharacter(BYTE cThreadID, CDNUserSession *pSession);
#else
	void QuerySelectCharacter(BYTE cThreadID, CDNUserSession *pSession, TPARTYID PartyID);
#endif
	void QueryUpdateUserData(int nMainCmd, CDNUserSession *pSession, TUserData *pUserData, bool bExecuteScheduleedTask );
	void QueryChangeStageUserData(CDNUserSession *pSession, BYTE cPCBangRebirthCoin, std::vector<INT64> &VecSerialList, std::vector<USHORT> &VecDurList, 
		int nDeathCount, BYTE cDifficult, INT64 biPickUpCoin, INT64 biPetItemSerial, int nPetExp);
	void QueryGetCharacterPartialyByName(CDNUserSession *pSession, const WCHAR *pCharName, BYTE cReqType);
	void QueryGetCharacterPartialyByDBID(CDNUserSession *pSession, INT64 biCharacterDBID, BYTE cReqType);

	void QueryCheckFirstVillage(CDNUserSession *pSession);
	void QueryLevel(CDNUserSession *pSession, char cChangeCode);
	void QueryExp(CDNUserSession *pSession, char cChangeCode, INT64 biFKey);
	void QueryCoin(CDNUserSession *pSession, char cCoinChangeCode, INT64 biChangeKey, INT64 nChangeCoin, INT64 nTotalCoin);
	void QueryWarehouseCoin(CDNUserSession *pSession, char cCoinChangeCode, INT64 nChangeCoin, INT64 nTotalCoin, INT64 nTotalWarehouseCoin);
	void QueryMapInfo(CDNUserSession *pSession);
	void QueryCashRebirthCoin(CDNUserSession *pSession);
	void QueryAddCashRebirthCoin(CDNUserSession *pSession, int nRebirthCount, int nRebirthPrice, int nRebirthCode, int nRebirthKey);
	void QueryRebirthCoin(CDNUserSession *pSession, int nRebirthCoin, int nPCBangRebirthCoin, int nVIPRebirthCoin);
	void QueryFatigue(CDNUserSession *pSession, char cFatigueTypeCode, int nChangeFatigue);
	void QueryEventFatigue(CDNUserSession *pSession, int nChangeFatigue, bool bReset, char cInitTime);

	void QueryJob(CDNUserSession *pSession, int nJob);
	void QueryGlyphDelayTime(CDNUserSession *pSession, int nTime);
	void QueryGlyphRemainTime(CDNUserSession *pSession, int nTime);
	void QueryNotifier(CDNUserSession *pSession, int nSlot, int nNotifierType, int nID);
#if !defined(PRE_DELETE_DUNGEONCLEAR)
	void QueryDungeonClear(CDNUserSession *pSession, int nType, int nMapIndex);
#endif	// #if !defined(PRE_DELETE_DUNGEONCLEAR)
	void QueryEternityItem(CDNUserSession *pSession, int nEternityCode, int nValue);
	void QueryColor(CDNUserSession *pSession, int nItemType, DWORD dwColor);
	void QueryNestClear(CDNUserSession *pSession, int nMapIndex, char cClearType = NestClear::Type::Normal);
	void QueryInitNestClearCount(CDNUserSession *pSession);
	void QueryChangeCharacterName(CDNUserSession *pSession, LPCWSTR pChangeCharacterName, INT64 biItemSerial);
	void QueryGetListEtcPoint(BYTE cThreadID, CDNUserSession *pSession);
	void QueryAddEtcPoint( CDNUserSession *pSession, char cType, INT64 biPoint );
	void QueryUseEtcPoint( CDNUserSession *pSession, char cType, INT64 biPoint );
#ifdef PRE_ADD_CHANGEJOB_CASHITEM
	void QueryChangeJobCode( CDNUserSession *pSession, int nFirstJobIDBefore, int nFirstJobIDAfter, int nSecondJobIDBefore, int nSecondJobIDAfter );
#endif // #ifdef PRE_ADD_CHANGEJOB_CASHITEM
	void QueryGetListVariableReset( BYTE cThreadID, CDNUserSession *pSession );
	void QueryModVariableReset( CDNUserSession *pSession, CommonVariable::Type::eCode Type, INT64 biValue, __time64_t ResetTime );
	void QueryChagneSkillPage(CDNUserSession *pSession, char cSkillPage);
#if defined( PRE_ADD_TOTAL_LEVEL_SKILL )
	void QueryGetTotalSkillLevel(BYTE cThreadID, CDNUserSession *pSession);
	void QueryAddTotalSkillLevel(CDNUserSession *pSession, int nSlotIndex, int nSkillID);
#endif
#ifdef PRE_ADD_PRESET_SKILLTREE
	void QueryGetSkillPresetList(CDNUserSession * pSession);
	void QueryAddSkillPreset(CDNUserSession * pSession, CSSaveSkillSet * pSkill);
	void QueryDelSkillPreset(CDNUserSession * pSession, BYTE cIndex);
#endif		//#ifdef PRE_ADD_PRESET_SKILLTREE

	// MAINCMD_ETC
	// Query-Queue
	void QueryUpdateQueryQueue(const TDBQueryQueue *pDBQueryQueue);
#ifdef PRE_MOD_RESTRICT_IDENTITY_IP
	void QuerySimpleConfig(BYTE cThreadID);
#endif		//#ifdef PRE_MOD_RESTRICT_IDENTITY_IP
#ifdef PRE_ADD_SEETMAINTENANCEFLAG
	void QuerySetMaintenanceFlag(BYTE cThreadID, int nFlag);
#endif		//#ifdef PRE_ADD_SEETMAINTENANCEFLAG

#if defined (PRE_ADD_DONATION)
	void QueryDonate(CDNUserSession *pSession, time_t tDonationTime, INT64 dwCoin);
	void QueryDonationRanking(CDNUserSession *pSession);
	void QueryDonationTopRanker(BYTE cThreadID, char cWorldID);
#endif // #if defined (PRE_ADD_DONATION)

	// Event
	void QueryEventList(BYTE cThreadID, int nWorldSetID);
	// GameOption
	void QueryGetGameOption(BYTE cThreadID, CDNUserSession *pSession);
	void QuerySetGameOption(CDNUserSession *pSession, TGameOptions * pOption);
	void QueryUpdateSecondAuthPassword( BYTE cThreadID, UINT uiAccountDBID, CSGameOptionUpdateSecondAuthPassword* pPacket );
	void QueryUpdateSecondAuthLock( BYTE cThreadID, UINT uiAccountDBID, CSGameOptionUpdateSecondAuthLock* pPacket );
	void QueryInitSecondAuth( BYTE cThreadID, UINT uiAccountDBID );
	void QueryValidateSecondAuth( BYTE cThreadID, UINT uiAccountDBID, const WCHAR* wszPW, int nAuthCheckType );
	// KeySetting
	void QueryGetKeySetting( BYTE cThreadID, int nWorldSetID, UINT nAccountDBID );
	void QueryModKeySetting( BYTE cThreadID, int nWorldSetID, UINT nAccountDBID, TKeySetting* pKeySetting );
	void QueryUpdateKeySetting( BYTE cThreadID, int nWorldSetID, UINT nAccountDBID, CSGameOptionUpdateKeySetting* pPacket );
	// PadSetting
	void QueryGetPadSetting( BYTE cThreadID, int nWorldSetID, UINT nAccountDBID );
	void QueryModPadSetting( BYTE cThreadID, int nWorldSetID, UINT nAccountDBID, TPadSetting* pPadSetting );
	void QueryUpdatePadSetting( BYTE cThreadID, int nWorldSetID, UINT nAccountDBID, CSGameOptionUpdatePadSetting* pPacket );

	void QueryGetProfile( BYTE cThreadID, CDNUserSession *pSession );
	void QuerySetProfile( CDNUserSession *pSession, BYTE cGender, const WCHAR * pwzGreeting, bool bOpenPublic );
	//Restraint
	void QueryGetRestraint(CDNUserSession *pSession);
	void QueryAddRestraint(CDNUserSession *pSession, int nTartgetCode, int nRestraintCode, const WCHAR * pwszRestraintReason, const WCHAR * pwszRestraintReasonForDolis, int nDurationDay, int nDolisReasonCode = DBDNWorldDef::RestraintDolisReasonCode::DefaultCode);

	void QueryLogout(CDNUserSession *pSession);
	void QueryModLastConnectDate(CDNUserSession *pSession);
	// Character Login/Logout
	void QueryLoginCharacter( CDNUserSession *pSession );
	void QueryLogoutCharacter( CDNUserSession *pSession );

	void QuerySaveConnectDurationTime(CDNUserSession *pSession);
	void QueryModTimeEventDate( CDNUserSession *pSession );
	void QueryAddAbuseLog( CDNUserSession *pSession, eAbuseCode eType, wchar_t* buf );
	void QueryAddAbuseMonitor( CDNUserSession *pSession, int iCount, int iCount2 );
	void QueryDelAbuseMonitor( CDNUserSession *pSession, char cType );	
	void QueryGetAbuseMonitor( BYTE cThreadID, CDNUserSession *pSession );
	void QueryGetWholeAbuseMonitor(BYTE cThreadID, CDNUserSession *pSession);
	void QueryModCharacterSlotCount(CDNUserSession *pSession);

	// MAINCMD_MISSION
	void QueryMissionGain(CDNUserSession *pSession);
	void QueryMissionAchieve(CDNUserSession *pSession, int nMissionID);
	void QuerySetDailyMission(CDNUserSession *pSession);
	void QuerySetWeeklyMission(CDNUserSession *pSession);
#if defined(PRE_ADD_MONTHLY_MISSION)
	void QuerySetMonthlyMission(CDNUserSession *pSession);
#endif	// #if defined(PRE_ADD_MONTHLY_MISSION)
	void QuerySetWeekendEventMission(CDNUserSession *pSession);
	void QueryModDailyWeeklyMission(CDNUserSession *pSession, char cDailyMissionType, char cIndex, TDailyMission *pMission, bool bIsRepeatMission=false);
	void QuerySetGuildWarMission(CDNUserSession *pSession);
	void QueryModGuildWarMission(CDNUserSession *pSession, char cDailyMissionType, char cIndex, TDailyMission *pMission, bool bAchieve);
	void QuerySetPCBangMission(CDNUserSession *pSession);
	void QueryDelAllMissionsForCheat(CDNUserSession *pSession);
	void QuerySetGuildCommonMission(CDNUserSession *pSession);
	void QuerySetWeekendRepeatMission(CDNUserSession *pSession);

	// MAINCMD_APPELLATION
	void QueryAddAppellation(CDNUserSession *pSession, int nAppellationID);
	void QuerySelectAppellation(CDNUserSession *pSession, int nSelect, char cKind);
	void QueryDelAppellation(CDNUserSession *pSession, int ArrayIndex);

	// MAINCMD_SKILL
	void QueryAddSkill(CDNUserSession *pSession, int nSkillID, char cSkillLevel, int nCoolTime, char cSkillChangeCode, INT64 biBuyPrice, INT64 biCurrentCoin=0, INT64 biPickUpCoin=0);
	void QueryIncreaseSkillPoint(CDNUserSession *pSession, USHORT wSkillPoint, int nChangeCode, int nSkillPage);
	void QueryDelSkill(CDNUserSession *pSession, int nSkillID, char cSkillChangeCode, bool bSkillPointBack, int nSkillPage);
	void QueryResetSkill(CDNUserSession *pSession, int nSkillPage);
	void QueryResetSkillBySkillIDs( CDNUserSession *pSession, std::vector<int>& vSkillIDs );
	void QueryModSkillLevel(CDNUserSession *pSession, int nSkillID, BYTE cSkillLevel, int nCoolTime, USHORT wUsedSkillPoint, char cSkillChangeCode);
	void QueryDecreaseSkillPoint(CDNUserSession *pSession, USHORT wSkillPoint, int nSkillPage );
	void QueryUseExpandSkillPage(CDNUserSession *pSession, int *SkillID, short SkillPoint);
	void QuerySetSkillPoint(CDNUserSession *pSession, USHORT wSkillPoint, int nSkillPage=0 );

	// MAINCMD_ITEM
	void QueryAddMaterializedItem(CDNUserSession *pSession, BYTE cItemSlotIndex, char cAddMaterializedItemCode, INT64 biFKey, const TItem &AddItem, INT64 biItemPrice,
		INT64 biSenderCharacterDBID, BYTE cItemLocationCode, BYTE cPayMethodCode, bool bMerge, INT64 biMergeTargetItemSerial, TVehicle *pVehicleEquip = NULL);
	void QueryUseItem(CDNUserSession *pSession, char cUseItemCode, INT64 biItemSerial, short wUsedItemCount, bool bAllFlag, int iTargetLogItemID=0, char cTargetLogItemLevel=0);
	void QueryUseItemEx(BYTE cThreadID, int nWorldSetID, UINT nAccountDBID, char cUseItemCode, INT64 biItemSerial, short wUsedItemCount, int nChannelID, int nMapID, const WCHAR* pwszIP, bool bAllFlag, int iTargetLogItemID=0, char cTargetLogItemLevel=0 );
	void QueryResellItem(CDNUserSession *pSession, INT64 biItemSerial, short wItemCount, INT64 biResellPrice, bool bAllFlag, bool bRebuyFlag, bool bBelongFlag);
	void QueryModMaterializedItem(CDNUserSession *pSession, TItem &ModItem, int nModFieldBitmap = ModItemAll, int nEnchantJewelID=0);

	void QueryEnchantItem(CDNUserSession *pSession, INT64 biItemSerial, INT64 biCoin, bool bEnchantSuccessFlag, bool bItemDestroyFlag, BYTE cCurrentItemLevel, int nCurrentItemPotential, 
		TItem *pNewItem, int nSupportItemID, std::vector<TItemInfo> &VecItemList, INT64 biExtractItemSN=0);
	void QueryChangeItemLocation(CDNUserSession *pSession, INT64 biBeforeItemSerial, INT64 biBeforeNewItemSerial, int nBeforeItemID, char cAfterItemLocationCode, BYTE cAfterItemSlotIndex,
		short wBeforeItemCount, bool bMerge, INT64 biMergeTargetItemSerial, bool bMergeAll, bool bVehicleParts = false, INT64 biVehicleItemSerial = 0, char cVehicleSlotIndex = 0, int nGoldFee=0);
	void QuerySwitchItemLocation(CDNUserSession *pSession, char cSrcCode, BYTE cSrcSlotIndex, INT64 biDestSerial, char cDestCode, BYTE cDestSlotIndex, INT64 biSrcSerial);
	void QueryModItemDurability(CDNUserSession *pSession, INT64 biTax, std::vector<INT64> &VecSerialList, std::vector<USHORT> &VecDurList, INT64 biCurrentCoin=0, INT64 biPickUpCoin=0);
	void QueryExchangeProperty(CDNUserSession *pSession, DBPacket::TExchangeItem *ExchangeItemArray, INT64 biExchangeCoin, int nTax, 
		INT64 biTargetCharacterDBID, DBPacket::TExchangeItem *TargetExchangeItemArray, INT64 biTargetExchangeCoin, int nTargetTax, UINT uiTargetAccountDBID );
	void QueryUpsEquip(CDNUserSession *pSession, int nItemLocation, int nSlotIndex, INT64 biSerial);
	void QueryTakeCashEquipmentOff(CDNUserSession *pSession, int nTotalCount, int *nCashEquipIndexList);
	void QueryPutOnepieceOn(CDNUserSession *pSession, INT64 biOnepieceItemSerial, char cOnepieceEquipIndex, int nTotalCount, int *nSubCashEquipIndexList);
	void QueryGetPageMaterializedItem(BYTE cThreadID, CDNUserSession *pSession, int nPageNumber, int nPageSize);
	void QueryGetPageVehicle(CDNUserSession *pSession, int nPageNumber, int nPageSize);
	void QueryModPetSkinColor(CDNUserSession *pSession, INT64 biBodySerial, DWORD dwColor, DWORD dwColor2);
	void QueryDelPetEquipment(CDNUserSession *pSession, INT64 biBodySerial, INT64 biPartsSerial);
	void QueryRecoverMissingItem(CDNUserSession *pSession, char cItemLocationCode, BYTE cSlotIndex, TItem &RecoverItem);
	void QueryAddEffectItems(CDNUserSession *pSession, int iItemID, char cLifeSpanRenewal, DBDNWorldDef::EffectItemGetCode::eCode Code, INT64 biFkey, int iPrice, bool bEnternityFlag, 
		int iLifeSpan, INT64 biItemSerial, int iRemoveItemID);
	void QueryModAdditiveItem(CDNUserSession *pSession, INT64 biItemSerial, int iAdditiveItemID, INT64 biAdditiveSerial, INT64 biMixItemSerial);
#if defined (PRE_ADD_COSRANDMIX)
	void QueryModRandomItem(CDNUserSession *pSession, INT64 biStuffSerialA, INT64 biStuffSerialB, INT64 biMixItemSerial, short wMixItemCount, TItem &RandomItem, INT64 biFee);
#endif
	void QuerySaveItemLocationIndex(CDNUserSession *pSession);
	void QueryGetListRepurchaseItem( CDNUserSession *pSession );
	void QueryDelExpireitem( BYTE cThreadID, int nWorldSetID, UINT uiAccountDBID, INT64 biCharacterDBID );
	void QueryDelCashItem(CDNUserSession *pSession, const TItem *pDelItem);
	void QueryRecoverCashItem(CDNUserSession *pSession, const TItem *pRecoverItem);
#if defined( PRE_ADD_NAMEDITEM_SYSTEM )
	void QueryCheckNamedItemCount(CDNUserSession *pSession, int nItemID, BYTE cIndex);
	void QueryCheatCheckNamedItemCount(CDNUserSession *pSession, int nItemID, BYTE cIndex);
#endif
#if defined (PRE_ADD_EXCHANGE_POTENTIAL)
	void QueryMovePotential(CDNUserSession * pSession, INT64 biExtractSerial, char cExtractPotential, INT64 biInjectSerial, char cInjectPotential, char cMoveableLimitCount, int nMapID);
#endif		//#if defined (PRE_ADD_EXCHANGE_POTENTIAL)

#if defined( PRE_ADD_LIMITED_SHOP )
	void QueryGetLimitedShopItem(BYTE cThreadID, CDNUserSession *pSession);
	void QueryAddLimitedShopItem(CDNUserSession * pSession, LimitedShop::LimitedItemData ItemData, int nBuyCount);
	void QueryResetLimitedShopItem(CDNUserSession * pSession, int nResetCycle);
#endif

	void QueryDelEffectItem( CDNUserSession * pSession, INT64 nItemSerial );	// Delete effect item

	// MAINCMD_QUEST
	void QueryAddQuest(CDNUserSession *pSession, char cIdx, TQuest &Quest);
	void QueryDelQuest(CDNUserSession *pSession, char cIdx, int nQuestID, bool bComplete, bool bRepeat, char *CompleteQuestArray);
	void QueryModQuest(CDNUserSession *pSession, char cIdx, TQuest &Quest, char cQuestStatusCode);
	void QueryClearQuest(CDNUserSession *pSession);
	void QueryForceCompleteQuest(CDNUserSession *pSession, int nQuestID, int nQuestCode, char *pCompleteQuestArray);
	void QueryGetListCompleteEventQuest(CDNUserSession *pSession);
	void QueryCompleteEventQuest(CDNUserSession *pSession, int nScheduleID, __time64_t tExpireDate);
	void QueryGetWorldEventQuestCounter(BYTE cThreadID, int nWorldSetID, int nScheduleID);
	void QueryModWorldEventQuestCounter(CDNUserSession *pSession, int nScheduleID, int nCounterDelta);

	// MAINCMD_FRIEND
	void QueryFriendList(BYTE cThreadID, CDNUserSession *pSession);
	void QueryAddGroup(CDNUserSession *pSession, WCHAR *pGroupName);
	void QueryModGroupName(CDNUserSession *pSession, int nGroupDBID, WCHAR *pGroupName);
	void QueryDelGroup(CDNUserSession *pSession, int nGroupDBID);
	void QueryModFriendAndGroupMapping(CDNUserSession *pSession, int nGroupDBID, char cCount, INT64 *biFriendCharacterDBIDs);
	void QueryAddFriend(CDNUserSession *pSession, WCHAR *pFriendName, int nGroupDBID);
	void QueryModFriendMemo(CDNUserSession *pSession, INT64 biFriendCharacterDBID, WCHAR *pMemo);
	void QueryDelFriend(CDNUserSession *pSession, char cCount, INT64 *biFriendCharacterDBIDs);

	// MAINCMD_ISOLATE
	void QueryGetIsolateList(BYTE cThreadID, CDNUserSession *pSession);
	void QueryAddIsolate(CDNUserSession *pSession, const WCHAR * pIsolateName);
	void QueryAddIsolateAfterDelFriend(CDNUserSession *pSession, const WCHAR * pIsolateName, INT64 biDelFriendCharacterDBID);
	void QueryDelIsolate(CDNUserSession *pSession, INT64 biIsolateCharacterDBID);

	// MAINCMD_GUILD
	void QueryDismissGuild(CDNUserSession *pSession);
	void QueryGetGuildHistoryList(CDNUserSession *pSession, INT64 biIndex, bool bDirection);
	void QueryGetGuildInfo(CDNUserSession *pSession, UINT nGuildDBID, bool bNeedMembList);
	void QueryAddGuildHistory(BYTE cThreadID, UINT nGuildDBID, char cWorldSetID, BYTE btHistoryType, int iInt1 = 0, int iInt2 = 0, INT64 biInt64 = 0, LPCWSTR pText = NULL);
	void QueryCreateGuild(BYTE cThreadID, UINT nReqAccountDBID, INT64 nReqCharacterDBID, LPCWSTR wszGuildName, int nMapIndex, char cLevel, int nEstablishExpense, char cWorldSetID, int wGuildRoleAuth[GUILDROLE_TYPE_CNT]);
	void QueryDelGuildMember(BYTE cThreadID, UINT nReqAccountDBID, INT64 nReqCharacterDBID, UINT nDelAccountDBID, INT64 nDelCharacterDBID, UINT nGuildDBID, char cLevel, char cWorldSetID, bool bIsExiled, bool bGraduateBeginnerGuild);
	void QueryGuildInviteMember(BYTE cThreadID, UINT nAccountDBID, UINT nGuildDBID, char cWorldSetID, UINT nToSessionID, bool bNeedMembList);
	void QueryGetGuildWareInfo(BYTE cThreadID, UINT nGuildDBID, char cWorldSetID, short wMaxCount);
	void QuerySetGuildWareInfo(BYTE cTheadID, UINT nGuildDBID, char cWorldSetID, BYTE cSlotCount, TGuildWareSlot* pWareSlot);
	void QueryGetGuildWareHistory(BYTE cThreadID, UINT nAccountDBID, INT64 nCharacterDBID, UINT nGuildDBID, char cWorldSetID, int nIndex);
	void QueryAddGuildMember(BYTE cThreadID, UINT nAccountDBID, INT64 nCharacterDBID, UINT nGuildDBID, char cLevel, char cWorldSetID, short wGuildSize);
	void QueryChangeGuildInfo(BYTE cThreadID, UINT nAccountDBID, INT64 nCharacterDBID, UINT nGuildDBID, char cWorldSetID, BYTE btGuildUpdate, int iInt1 = 0, int iInt2 = 0, int iInt3 = 0, int iInt4 = 0, INT64 biInt64 = 0, LPCWSTR pText = NULL, long nAddCommonPoint = 0);
	void QueryChangeGuildMemberInfo(BYTE cThreadID, UINT nReqAccountDBID, INT64 nReqCharacterDBID, UINT nChgAccountDBID, INT64 nChgCharacterDBID, UINT nGuildDBID, char cWorldSetID, BYTE btGuildMemberUpdate, int iInt1 = 0, int iInt2 = 0, INT64 biInt64 = 0, LPCWSTR pText = NULL, char cLevel = 0);
	void QueryMoveItemInGuildWare(BYTE cThreadID, UINT nAccountDBID, char cWorldSetID, TQMoveItemInGuildWare* pMove);
	void QueryMoveInvenToGuildWare(BYTE cThreadID, UINT nAccountDBID, char cWorldSetID, TQMoveInvenToGuildWare* pMove);
	void QueryMoveGuildWareToInven(BYTE cThreadID, UINT nAccountDBID, char cWorldSetID, TQMoveGuildWareToInven* pMove);
	void QueryGuildWarehouseCoin(BYTE cThreadID, UINT nAccountDBID, char cWorldSetID, TQGuildWareHouseCoin* pPacket);
	void QueryChangeGuildWareSize(BYTE cThreadID, UINT nAccountDBID, INT64 nCharacterDBID, UINT nGuildDBID, char cWorldSetID, short wSize);
	void QueryChangeGuildMark(BYTE cThreadID, UINT nAccountDBID, INT64 nCharacterDBID, UINT nGuildDBID, char cWorldSetID, short wGuildMark, short wGuildMarkBG, short wGuildMarkBorder);
	void QueryChangeGuildName(BYTE cThreadID, char cWorldSetID, UINT nAccountDBID, UINT nGuildDBID, LPCWSTR pGuildName, INT64 biItemSerial);
	void QueryGetGuildRewardItem(BYTE cThreadID, char cWorldSetID, UINT nAccountDBID, UINT nGuildDBID);
	void QueryAddGuildRewardItem(BYTE cThreadID, char cWorldSetID, UINT nAccountDBID, INT64 biCharacterDBID, UINT nGuildDBID, int nItemID, bool bEnternityFlag, int nLifeSpan, BYTE cItemBuyType, int nNeedGold, int nMapID, int nRemoveItemID);
	void QueryChangeGuildSize( BYTE cThreadID, UINT nAccountDBID, UINT nGuildDBID, char cWorldSetID, int nGuildSize );
#if defined(PRE_ADD_GUILD_CONTRIBUTION)
	void QueryGetGuildContributionPoint( BYTE cThreadID, char cWorldSetID, UINT nAccountDBID, INT64 biCharacterDBID );
	void QueryGetWeeklyGuildContributionPointRanking( BYTE cThreadID, char cWorldSetID, UINT nAccountDBID, UINT nGuildDBID );
#endif	// #if defined(PRE_ADD_GUILD_CONTRIBUTION)
	// Query for cheat
	void QueryCheatDelGuildEffectItem( BYTE cThreadID, int iWorldID, UINT uiAccountDBID, INT64 biCharacterDBID, UINT nGuildDBID, int nItemID );
	void QueryChangePetName(BYTE threadID, char worldSetID, UINT accountDBID, INT64 itemSerial, INT64 petSerial, LPCWSTR pName);
	void QueryModItemExpireDate(BYTE cThreadID, int nWorldSetID, UINT nAccountDBID, INT64 biCharacterDBID, INT64 biExpireDateItemSerial, INT64 biItemSerial, int nMin, int nMapID, const WCHAR* pwszIP);
	void QueryModPetExp(BYTE cTheadID, int nWorldSetID, UINT nAccountDBID, INT64 biPetSerialID, int nExp);
	void QueryItemExpireByCheat(BYTE cTheadID, int nWorldSetID, UINT nAccountDBID, INT64 biCharacterDBID, INT64 biSerial); // General-purpose
	void QueryModPetSkill(BYTE cThreadID, int nWorldSetID, UINT nAccountDBID, INT64 biPetItemSerial, char cSlotNum, int nSkillID );
	void QueryModPetSkillExpand(BYTE cThreadID, int nWorldSetID, UINT nAccountDBID, INT64 biPetItemSerial);
	void QueryModDegreeOfHunger(BYTE cThreadID, int nWorldSetID, UINT nAccoutDBID, INT64 biPetItemSerial, DWORD nDegreeOfHunger);
	void QueryEnrollGuildWar(BYTE cThreadID, UINT nGuildDBID, char cWorldSetID, UINT nAccountDBID);
	void QueryGetGuildWarSchedule(BYTE cThreadID, char cWorldSetID);
	void QueryGetGuildWarFinalSchedule(BYTE cThreadID, char cWorldSetID, short wScheduleID);
	void QueryAddGuildWarPoint(CDNUserSession *pSession, int nMissionID, int nOpeningPoint);
	void QueryGetGuildWarPoint(BYTE cThreadID, char cWorldSetID, UINT nAccountDBID, char cQueryType, INT64 biDBID);
	void QueryGetGuildWarPointPartTotal(BYTE cThreadID, char cWorldSetID, UINT nAccountDBID, char cQueryType, INT64 biCharacterDBID);
	void QueryGetGuildWarPointGuildPartTotal(BYTE cThreadID, char cWorldSetID, UINT nAccountDBID, char cQueryType,	UINT nGuildDBID);
	void QueryGetGuildWarPointDaily(BYTE cThreadID, char cWorldSetID );
	void QueryGetGuildWarPointGuildTotal(BYTE cThreadID, char cWorldSetID, UINT nAccountDBID, bool bMasterServer, char cQueryType, short wScheduleID, UINT nGuildDBID);
	void QueryAddGuildWarFinalMatchList(BYTE cThreadID, char cWorldSetID, short wScheduleID, UINT* pGuildDBID);
	void QueryAddGuildWarPopularityVote(BYTE cThreadID, char cWorldSetID, UINT nAccountDBID, INT64 biCharacterDBID, UINT nGuildDBID);
	void QueryGetGuildWarPopularityVote(BYTE cThreadID, char cWorldSetID, UINT nAccountDBID, char cQueryType, INT64 biCharacterDBID);	
	void QueryAddGuildWarFinalResults(BYTE cThreadID, char cWorldSetID, UINT nGuildDBID, short wScheduleID,	char cMatchSequence, char cMatchTypeCode, char cMatchResultCode);
	void QueryGetGuildWarFinalResults(BYTE cThreadID, char cWorldSetID, short wScheduleID);	
	void QueryAddGuildWarRewarForCharacter(BYTE cThreadID, CDNUserSession *pSession);
	void QueryGetGuildWarRewarForCharacter(BYTE cThreadID, CDNUserSession *pSession);
	void QueryAddGuildWarRewarForGuild(BYTE cThreadID, char cWorldSetID, short wScheduleID, UINT nGuildDBID, char cRewardType);	
	void QueryAddGuildWarPointQueue(BYTE cThreadID, char cWorldSetID, UINT nGUildDBID, INT64 biAddPoint, const WCHAR* pwszIP);
	void QueryAddGuildWarSystemMailQueue(BYTE cThreadID, int nWorldSetID, WCHAR *pSenderName, UINT nGuildDBID, DBDNWorldDef::MailTypeCode::eCode MailCode, INT64 biFKey, WCHAR *pSubject, WCHAR *pContent, INT64 biAttachCoin,
		TItem *AttachItemArray, int nChannelID, int nMapID, bool bNewFlag);
	void QueryAddGuildWarMakeGiftQueue(BYTE cThreadID, int nWorldSetID, char cRewardType, char* cClassID, INT64* biCharacterDBID);
	void QueryAddGuildWarMakeGiftNotGuildInfo(BYTE cThreadID, int nWorldSetID, char cRewardType, UINT nGuildDBID);	
	void QueryGetGuildWarPointRunningTotal(BYTE cThreadID, char cWorldSetID );
	void QueryGetGuildWarPreWinGuild(BYTE cThreadID, char cWorldSetID);
	void QueryGetGuildWarRewarForGuild(BYTE cThreadID, char cWorldSetID, short wScheduleID, UINT nGuildDBID, char cQueryType = 'V');
	void QueryGetGuildWarPointFinalRewards(BYTE cThreadID, char cWorldSetID );
	void QueryGetListGuildWarItemTradeRecord(BYTE threadID, CDNUserSession *pSession);
	void QueryAddGuildWarItemTradeRecord(CDNUserSession *pSession, int itemID, int count);

	// MAINCMD_JOBSYSTEM
	void QueryAddDBJobSystemReserve(BYTE cThreadID, int nWorldSetID, char cJobType);
	void QueryGetDBJobSystemReserve(BYTE cThreadID, int nWorldSetID, int nJobSeq);
#if defined(PRE_ADD_GUILD_CONTRIBUTION)
	void QueryModGuildExp(BYTE cThreadID, char cWorldSetID, UINT nGuildDBID, char cPointType, int nPointValue, int nMaxPoint, int nLevel, INT64 biCharacterDBID, int nMissionID, bool IsGuildLevelMax = false);
#else	// #if defined(PRE_ADD_GUILD_CONTRIBUTION)
	void QueryModGuildExp(BYTE cThreadID, char cWorldSetID, UINT nGuildDBID, char cPointType, int nPointValue, int nMaxPoint, int nLevel, INT64 biCharacterDBID, int nMissionID);
#endif	// #if defined(PRE_ADD_GUILD_CONTRIBUTION)
	void QueryModGuildCheat(BYTE cThreadID, char cWorldSetID, UINT nGuildDBID, char cCheatType, int nPoint, char cLevel);

	// MAINCMD_MAIL
	void GetCountReceiveMail(CDNUserSession *pSession);
	void QueryGetPageReceiveMail(CDNUserSession *pSession, int nPageNo);
	void QuerySendMail(CDNUserSession *pSession, const CSSendMail *pPacket, DBPacket::TSendMailItem *SendItemArray, int nTax);
	void QuerySendSystemMail(BYTE cThreadID, int nWorldSetID, UINT nAccountDBID, WCHAR *pSenderName, INT64 biReceiverCharDBID, DBDNWorldDef::MailTypeCode::eCode MailCode, INT64 biFKey, WCHAR *pSubject, WCHAR *pContent, INT64 biAttachCoin,
		TItem *AttachItemArray, int nChannelID, int nMapID, bool bNewFlag = true);
	void QueryReadMail(CDNUserSession *pSession, int nMailDBID);
	void QueryDelMail(CDNUserSession *pSession, int *nMailDBIDArray);
	void QueryGetListMailAttachment(CDNUserSession *pSession, int *nMailDBIDArray);
	void QueryTakeMailAttachList(CDNUserSession *pSession, char cCount, DBPacket::TRequestTakeAttachInfo *InfoList);
	void QueryTakeMailAttach(CDNUserSession *pSession, DBPacket::TRequestTakeAttachInfo &Info);
#ifdef PRE_ADD_BEGINNERGUILD
	void QueryGetWillSendMails(CDNUserSession * pSession);
	void QueryDelWillSendMails(CDNUserSession * pSession, int nWillSendMailID);
#endif		//#ifdef PRE_ADD_BEGINNERGUILD
#if defined(PRE_ADD_CADGE_CASH)
	void QuerySendWishMail(CDNUserSession *pSession, const CSCashShopCadge *pPacket);
	void QueryReadWishMail(CDNUserSession *pSession, int nMailDBID);
#endif	// #if defined(PRE_ADD_CADGE_CASH)
#if defined(PRE_SPECIALBOX)
	void QueryAddEventReward(CDNUserSession *pSession, BYTE cReceiveTypeCode, BYTE cTargetTypeCode, BYTE cTargetClassCode, BYTE cTargetMinLevel, BYTE cTargetMaxLevel, int nEventNameUIString, 
		int nSenderNameUIString, int nContentUIString, int nRewardCoin, char cEventRewardTypeCode, std::vector<TSpecialBoxItemInfo> &VecItemList);
	void QueryGetCountEventReward(CDNUserSession *pSession);
	void QueryGetListEventReward(CDNUserSession *pSession);
	void QueryGetListEventRewardItem(CDNUserSession *pSession, int nEventRewardID);
	void QueryAddEventRewardReceiver(CDNUserSession *pSession, int nEventRewardID, int nItemID);
#endif	// #if defined(PRE_SPECIALBOX)
#ifdef PRE_ADD_JOINGUILD_SUPPORT
	void QueryGuildSupportRewardInfo(CDNUserSession * pSession);
	void QuerySendGuildMail(BYTE cThreadID, int nWorldSetID, WCHAR *pSenderName, UINT nReceiverAccountDBID, INT64 biReceiverCharDBID, UINT nAchivementAccountDBID, INT64 biAchivementCharacterDBID, DBDNWorldDef::MailTypeCode::eCode MailCode, INT64 biFKey, WCHAR *pSubject, WCHAR *pContent, INT64 biAttachCoin,
		TItem *AttachItemArray, int nChannelID, int nMapID, bool bNewFlag = true);					//Received by the user, but goes into the guild storage upon receipt
#endif		//#ifdef PRE_ADD_JOINGUILD_SUPPORT

	// MAINCMD_MARKET
	void QueryGetListMyTrade(CDNUserSession *pSession);
	void QueryAddTrade(CDNUserSession *pSession, const TItem &RegisterItem, INT64 biItemNewSerial, BYTE cInvenIndex, int nPrice, int nRegisterTax, int nSellTax, int nSellPeriod, bool bDisplayFlag1, bool bDisplayFlag2, 
		WCHAR *pItemName, char cItemGrade, char cLimitLevel, char cJob, int nInvenType, bool bPremiumTrade, int nMaxRegisterItemCount, bool bCash, char cMethodCode, int nExchangeCode);
	void QueryGetPageTrade(CDNUserSession *pSession, const CSMarketList *pMarketList, char *cJobCodeArray, char *cItemExchangeArray, WCHAR *pSearchItemName);
	void QueryCancelTrade(CDNUserSession *pSession, int nMarketDBID, int nItemSlotIndex, bool bCashItem);
#if defined(PRE_ADD_DIRECT_BUY_UPGRADEITEM)
	void QueryBuyTrade(CDNUserSession *pSession, int nMarketDBID, int nItemSlotIndex, bool bMini);
#else	// #if defined(PRE_ADD_DIRECT_BUY_UPGRADEITEM)
	void QueryBuyTrade(CDNUserSession *pSession, int nMarketDBID, int nItemSlotIndex, bool bCashItem, int nPetalPrice);
#endif	// #if defined(PRE_ADD_DIRECT_BUY_UPGRADEITEM)
	void QueryGetListTradeforCalculation(CDNUserSession *pSession);
	void QueryTradeCalculation(CDNUserSession *pSession, int nMarketDBID);
	void QueryTradeCalculationAll(CDNUserSession *pSession);
#if !defined(PRE_ADD_DIRECT_BUY_UPGRADEITEM)
	void QueryGetTradeItemID(CDNUserSession *pSession, int nMarketDBID);
#endif	// #if defined(PRE_ADD_DIRECT_BUY_UPGRADEITEM)
	void QueryGetPetalBalance(CDNUserSession *pSession);
	void QueryGetTradePrice(CDNUserSession *pSession, const CSMarketPrice *pMarket);
	void QueryGetListMiniTrade(CDNUserSession *pSession, int nItemID, char cPayMethodCode);

	// MAINCMD_PVP
	bool QueryUpdatePvPData(BYTE cThreadID, CDNUserSession* pGameSession, PvPCommon::QueryUpdatePvPDataType::eCode Type );
#ifdef PRE_MOD_PVPRANK
	void QuerySetPvPRankCriteria(BYTE cThreadID, UINT nThreshold);
	void QueryCalcPvPRank(BYTE cThreadID);
#endif		//#ifdef PRE_MOD_PVPRANK
#if defined(PRE_ADD_PVP_RANKING)
	void QueryAggregatePvPRank(CDNUserSession *pSession, bool bIsPvPLadder);
#endif

#if defined( PRE_PVP_GAMBLEROOM )
	void QueryAddGambleRoom(BYTE cThreadID, char cWorldSetID, int nRoomID, BYTE cGambleType, int nGamblePrice);
	void QueryAddGambleRoomMember(BYTE cThreadID, char cWorldSetID, INT64 nGambleDBID, INT64 biCharacterDBID);
	void QueryEndGambleRoom(BYTE cThreadID, char cWorldSetID, INT64 nGambleDBID, TPvPGambleRoomWinData* Member);
#endif

	// MAINCMD_DARKLAIR
	bool QueryUpdateDarkLairResult(BYTE cThreadID, TQUpdateDarkLairResult* pPacket, const int iPacketSize );
	void QueryGetDarkLairRankBoard( CDNUserSession* pSession, int iMapIndex, BYTE cPartyUserCount );

	// MAINCMD_CASH
	void SendModGiveFail(CDNUserSession* pSession, int nCount, DBPacket::TModCashFailItemEx *CashItemArray);
	void QueryCheckGiftReceiver(CDNUserSession* pSession, WCHAR *pCharName);
	void QueryGetListGiftBox(CDNUserSession* pSession);
	void QueryModGiftReceive(CDNUserSession* pSession, bool bReceiveAll, TReceiveGiftData *GiftData);
	void QueryReceiveGift(CDNUserSession* pSession, bool bReceiveAll, char cTotalCount, const TReceiveGiftData &GiftData, char cItemCount, TGiftItem *AddItemList, int nItemSN);
	void QueryNotifyGift(CDNUserSession* pSession, bool bNew);
	void QueryMakeGiftByQuest(CDNUserSession* pSession, int nQuestID, WCHAR *pMemo, int *nRewardItemList, bool bNewFlag);
	void QueryMakeGift(BYTE cThreadID, int nWorldSetID, UINT nAccountDBID, INT64 biCharacterDBID, bool bPCBang, WCHAR *pMemo, char cPayMethod, int iOrderKey, char *pIp, int *nRewardItemList, bool bNewFlag);
	void QueryIncreaseVIPPoint(CDNUserSession* pSession, int nBasicPoint, INT64 biPurchaseOrderID, short wVIPPeriod, bool bAutoPay);
	void QueryGetVIPPoint(CDNUserSession* pSession);
	void QueryModVIPAutoPayFlag(CDNUserSession* pSession, bool bAutoPay);
	void QueryGiftByCheat(CDNUserSession* pSession, int nItemSN);
	void QueryPetal(CDNUserSession* pSession, int nPetal, int nUseItemID, char cInvenType, BYTE cInvenIndex, INT64 biInvenSerial);
#if defined( PRE_PVP_GAMBLEROOM )
	void QueryPetalLog(CDNUserSession* pSession, int nPetal, int nUseItemID, BYTE cLogCode, INT64 nGambleID);
	void QueryUsePetal(BYTE cThreadID, char cWorldSetID, UINT nAccountDBID, INT64 biCharacterDBID, int nPetal, BYTE cLogCode, INT64 nGambleID);
#endif
#if defined(PRE_ADD_GIFT_RETURN)
	void QueryModGiftRejectFlag(CDNUserSession* pSession, INT64 biPurchaseOrderID);
#endif
#if defined(PRE_ADD_CASH_REFUND)
	void QueryGetListRefundableProducts(CDNUserSession* pSession);
#endif
	void QueryCheatGiftClear(CDNUserSession* pSession);

	// DB Message Adjustment
	void QueryMsgAdj(CDNUserSession* pSession, int nChannelID, int nVerifyMapIndex, char cGateNo);
#ifdef PRE_FIX_63822
	void QueryMsgAdjSeq(CDNUserSession * pSession, int nSubCmd);
#endif		//#ifdef PRE_FIX_63822

	// MAINCMD_LOG
	void QueryAddPartyStartLog( CDNUserSession* pSession, INT64 biPartyLogID );
	void QueryAddPartyEndLog( BYTE cThreadID, int iWorldID, UINT uiAccountDBID, INT64 biPartyLogID );
	void QueryAddStageStartLog( BYTE cThreadID, int iWorldID, UINT uiAccountDBID, INT64 biRoomLogID, INT64 biPartyLogID, BYTE cPartySize, int iMapID, DBDNWorldDef::DifficultyCode::eCode Code );
#if defined( PRE_ADD_36870 )
	void QueryAddStageClearLog( BYTE cThreadID, int iWorldID, UINT uiAccountDBID, INT64 biRoomLogID, INT64 biCharacterDBID, int iJobID, BYTE cLevel, bool bClearFlag, DBDNWorldDef::ClearGradeCode::eCode ClearCode, 
		int iMaxComboCount, BYTE cBossKillCount, int iRewardCharacterExp, int iBonusCharacterExp, int iClearPoint, int iBonusClearPoint, int iClearTimeSec, int iClearRound );
#else
	void QueryAddStageClearLog( BYTE cThreadID, int iWorldID, UINT uiAccountDBID, INT64 biRoomLogID, INT64 biCharacterDBID, int iJobID, BYTE cLevel, bool bClearFlag, DBDNWorldDef::ClearGradeCode::eCode ClearCode, 
		int iMaxComboCount, BYTE cBossKillCount, int iRewardCharacterExp, int iBonusCharacterExp, int iClearPoint, int iBonusClearPoint, int iClearTimeSec );
#endif // #if defined( PRE_ADD_36870 )
	void QueryAddStageRewardLog( BYTE cThreadID, int iWorldID, UINT uiAccountDBID, INT64 biRoomLogID, INT64 biCharacterDBID, DBDNWorldDef::RewardBoxCode::eCode Code, INT64 biRewardItemSerial, int iRewardItemID, int iRewardRet );
	void QueryAddStageEndLog( BYTE cThreadID, int iWorldID, UINT uiAccountDBID, INT64 biRoomLogID, DBDNWorldDef::WhereToGoCode::eCode Code );
	void QueryAddPvPStartLog( BYTE cThreadID, int iWorldID, UINT uiAccountDBID, INT64 biRoomID1, int iRoomID2, BYTE cMaxPlayerCount, DBDNWorldDef::PvPModeCode::eCode Code, int iPvPModeDetailValue,
		bool bBreakIntoFlag, bool bItemUseFlag, bool bSecretFlag, int iMapID, INT64 biRoomMasterCharacterDBID=0, INT64 biCreateRoomCharacterDBID=0 );
	void QueryAddPvPResultLog( BYTE cThreadID, int iWorldID, UINT uiAccountDBID, INT64 biRoomID1, int iRoomID2, INT64 biCharacterDBID, DBDNWorldDef::PvPTeamCode::eCode TeamCode,
		bool bBreakIntoFlag, DBDNWorldDef::PvPResultCode::eCode ResultCode, int iPlayTimeSec, int iPvPKillPoint, int iPvPAssistPoint, int iPvPSupportPoint, UINT* uiKillScore, UINT* uiKilledScore, int nOccupationCount, BYTE cOccupationWinType);
	void QueryAddPvPEndLog( BYTE cThreadID, int iWorldID, UINT uiAccountDBID, INT64 biRoomID1, int iRoomID2 );

#if defined( PRE_ADD_58761 )
	// Stage gate clear log
	void QueryAddNestGateStartLog(BYTE cThreadID, int iWorldID, UINT uiAccountDBID, INT64 biRoomID, BYTE cPartySize, int nGateID);
	void QueryAddNestGateEndLog(BYTE cThreadID, int iWorldID, UINT uiAccountDBID, INT64 biRoomID, BYTE cPartySize);
	void QueryAddNestGateClearLog(BYTE cThreadID, int iWorldID, UINT uiAccountDBID, INT64 biRoomID, bool bClearFlag, INT64 biChracterDBID, BYTE cCharacterJob, BYTE cCharacterLevel);
	void QueryNestDeathLog(CDNUserSession *pSession, INT64 biRoomID, int nMonsterID, int nSkillID, BYTE cCharacterJob, BYTE cCharacterLevel);
#endif

	// MAINCMD_REPUTATION
#if defined( PRE_ADD_NPC_REPUTATION_SYSTEM )
	void QueryGetListNpcReputation( BYTE cThreadID, CDNUserSession *pSession, int iMapID=0 );
	void QueryModNpcReputation( CDNUserSession *pSession, std::vector<TNpcReputation>& Data );
#endif // #if defined( PRE_ADD_NPC_REPUTATION_SYSTEM )
	void ParseGetMasterAndFavorPoint( TAGetMasterAndFavorPoint* pA );
	void QueryGetMasterSystemSimpleInfo( BYTE cThreadID, CDNUserSession *pSession, bool bClientSend, MasterSystem::EventType::eCode EventCode );
	void QueryGetMasterSystemCountInfo( BYTE cThreadID, CDNUserSession *pSession, bool bClientSend, std::vector<INT64>& biPartyUserList );
	void QueryGetPageMasterCharacter( CDNUserSession *pSession, UINT uiPage, BYTE cJob, BYTE cGender );
	void QueryGetMasterCharacterType1( CDNUserSession *pSession, INT64 biMasterCharacterDBID );
	void QueryGetMasterCharacterType2( CDNUserSession *pSession, INT64 biMasterCharacterDBID, INT64 biPupilCharacterDBID );
	void QueryGetPupilList( CDNUserSession *pSession, INT64 biMasterCharacterDBID );
	void QueryRegisterMasterCharacter( CDNUserSession *pSession, WCHAR* pwszText );
	void QueryRegisterCancelMasterCharacter( CDNUserSession *pSession );
	void QueryJoinMasterSystem( CDNUserSession *pSession, INT64 biMasterCharacterDBID, INT64 biPupilCharacterDBID, bool bIsDirectMenu, DBDNWorldDef::TransactorCode::eCode Code, bool bIsSkip=false );
	void QueryGetMasterAndClassmate( CDNUserSession *pSession, INT64 biCharacterDBID );
	void QueryGetClassmateInfo( CDNUserSession *pSession, INT64 biClassmateCharacterDBID );
	void QueryLeaveMasterSystem( CDNUserSession *pSession, INT64 biMasterCharacterDBID, INT64 biPupilCharacterDBID, DBDNWorldDef::TransactorCode::eCode Code, int iPenaltyDay, int iPenaltyRespectPoint, bool bIsSkip=false );
	void QueryMasterSystemGraduate( CDNUserSession *pSession );
	void QueryModMasterSystemRespectPoint( CDNUserSession *pSession, int iRespectPoint );
	void QueryModMasterSystemFavorPoint( CDNUserSession *pSession, INT64 biMasterCharacterDBID, INT64 biPupilCharacterDBID, int iFavorPoint );
	void QueryModMasterSystemGraduateCount( CDNUserSession *pSession, int iGraduateCount );

#if defined( PRE_ADD_SECONDARY_SKILL )
	void QueryAddSecondarySkill( CDNUserSession *pSession, int iSkillID, SecondarySkill::Type::eType Type );
	void QueryDeleteSecondarySkill( CDNUserSession *pSession, int iSkillID );
	void QueryGetListSecondarySkill( BYTE cThreadID, CDNUserSession *pSession );
	void QueryModSecondarySkillExp( CDNUserSession *pSession, int iSkillID, int iExp, int iExpAfter );
	void QueryAddSecondarySkillRecipe( CDNUserSession *pSession, bool bIsCash, INT64 biItemSerial, int iSkillID );
	void QueryDeleteSecondarySkillRecipe( CDNUserSession *pSession, INT64 biItemSerial );
	void QueryExtractSecondarySkillRecipe( CDNUserSession *pSession, INT64 biItemSerial, DBDNWorldDef::ItemLocation::eCode Code );
	void QueryModSecondarySkillRecipeExp( CDNUserSession *pSession, INT64 biItemSerial, short nAddExp, short nExpAfter );
#endif // #if defined( PRE_ADD_SECONDARY_SKILL )

	// MAINCMD_FARM
	void QueryFarmList(BYTE cThreadID, int iWorldID);
	void QueryGetListField( BYTE cThreadID, int iWorldID, int iRoomID, int iFarmDBID );
	void QueryGetListFieldByCharacter(CDNUserSession *pSession, bool bRefreshGate = true);
	void QueryGetCountHarvestDepotItem( CDNUserSession *pSession );
#if defined( _GAMESERVER )
	void QueryAddField( BYTE cThreadID, CDNUserSession *pSession, CGrowingArea* pArea, int iMaxFieldCount, BYTE cInvenIndex, INT64 biItemSerial, const CSFarmPlant* pPacket );
	void QueryDelField( BYTE cThreadID, int iWorldID, int iFarmDBID, int iRoomID, int iAreaIndex );
	void QueryAddFieldAttachment( BYTE cThreadID, CDNUserSession *pSession, int iFarmDBID, int iRoomID, int iAreaIndex, int iAttachItemID, BYTE cInvenType, BYTE cInvenIndex, INT64 biItemSerial, bool bVirtualAttach=false );
	void QueryModFieldElapsedTime( BYTE cThreadID, int iWorldID, UINT uiAccountDBID, int iFarmDBID, std::vector<int>& vAreaIndex, std::vector<int>& vElapsedTimeSec );
	void QueryHarvest( BYTE cThreadID, int iWorldID, UINT uiAccountDBID, int iFarmDBID, int iRoomID, int iAreaIndex, INT64 biOwnerCharacterDBID, INT64 biCharacterDBID, std::vector<TItem>& vItem, int iMapID, const char* pszIP );
	void QueryGetListHarvestDepotItem( BYTE cThreadID, int iWorldID, UINT uiAccountDBID, INT64 biCharacterDBID, INT64 biLastUniqueID );
	void QueryGetFieldCountByCharacter( BYTE cThreadID, int iWorldID, UINT uiAccountDBID, INT64 biCharacterDBID, bool bIsSend );
	void QueryGetFieldItemCount( BYTE cThreadID, int iWorldID, UINT uiAccountDBID, INT64 biCharacterDBID, int iItemID, int iRoomID, int iAreaIndex, UINT nSessionID );
	void QueryGetListFieldForCharacter( BYTE cThreadID, int iWorldID, UINT uiAccountDBID, INT64 biCharacterDBID, int iRoomID );

	void QueryAddFieldForCharacter( BYTE cThreadID, int iWorldID, UINT uiAccountDBID, INT64 biCharacterDBID, int iRoomID, CGrowingArea* pArea, int iMaxFieldCount, BYTE cInvenIndex, INT64 biItemSerial, const CSFarmPlant* pPacket, int iMapID, const WCHAR* pwszIP );
	void QueryDelFieldForCharacter( BYTE cThreadID, int iWorldID, INT64 biCharacterDBID, int iRoomID, int iAreaIndex );
#if defined( PRE_ADD_VIP_FARM )
	void QueryAddFieldForCharacterAttachment( BYTE cThreadID, int iWorldID, UINT uiAccountDBID, INT64 biCharacterDBID, int iRoomID, int iAreaIndex, int iAttachItemID, BYTE cInvenType, BYTE cInvenIndex, INT64 biItemSerial, int iMapID, const WCHAR* pwszIP, bool bVirtualAttach=false );
#else
	void QueryAddFieldForCharacterAttachment( BYTE cThreadID, int iWorldID, UINT uiAccountDBID, INT64 biCharacterDBID, int iRoomID, int iAreaIndex, int iAttachItemID, BYTE cInvenType, BYTE cInvenIndex, INT64 biItemSerial, int iMapID, const WCHAR* pwszIP );
#endif // #if defined( PRE_ADD_VIP_FARM )
	void QueryModFieldForCharacterElapsedTime( BYTE cThreadID, int iWorldID, UINT uiAccountDBID, INT64 biCharacterDBID, std::vector<int>& vAreaIndex, std::vector<int>& vElapsedTimeSec );
	void QueryHarvestForCharacter( BYTE cThreadID, int iWorldID, UINT uiAccountDBID, int iRoomID, int iAreaIndex, INT64 biOwnerCharacterDBID, INT64 biCharacterDBID, std::vector<TItem>& vItem, int iMapID, const char* pszIP );
#endif // #if defined( _GAMESERVER )
	void QueryDeletePeriodQuest( CDNUserSession *pSession, int nPeriodQuestType, std::vector<int>& vRemoveQuestIDs);
	void QueryGetPeriodQuestDate( CDNUserSession *pSession );

	bool QueryAddPvPLadderResult( CDNUserSession *pSession, PvPCommon::QueryUpdatePvPDataType::eCode Type, LadderSystem::MatchType::eCode MatchType, int iGradePoint, int iHiddenGradePoint, LadderSystem::MatchResult::eCode Result, BYTE cVSJobCode, const std::vector<LadderKillResult>& vData );
	void QueryGetListPvPLadderScore( BYTE cThreadID, CDNUserSession *pSession );
	void QueryGetListPvPLadderScoreByJob( CDNUserSession *pSession );
	void QueryInitPvPLadderGradePoint( CDNUserSession *pSession, LadderSystem::MatchType::eCode MatchType, int iAddLadderPoint, int iLadderGradePoint );
	void QueryUsePvPLadderPoint( CDNUserSession *pSession, int iUseLadderPoint );
	void QueryAddPvPLadderCUCount( BYTE cThreadID, int iWorldID, LadderSystem::MatchType::eCode MatchType, int iCUCount );
	void QueryModPvPLadderScoresForCheat( CDNUserSession *pSession, LadderSystem::MatchType::eCode MatchType, int iPvPLadderPoint, int iPvPLadderGradePoint, int iHiddenPvPLadderGradePoint );
	void QueryGetPvPLadderRankBoard( CDNUserSession *pSession, LadderSystem::MatchType::eCode MatchType );

#if defined(PRE_ADD_PVP_RANKING)
	void QueryGetPvPRankBoard( CDNUserSession *pSession );
	void QueryGetPvPRankList( CDNUserSession *pSession, int nPage, BYTE cClassCode, BYTE cSubClassCode, const WCHAR* wszGuildName );
	void QueryGetPvPRankInfo( CDNUserSession *pSession, const WCHAR* wszCharName );
	void QueryGetPvPLadderRankBoard2( CDNUserSession *pSession, LadderSystem::MatchType::eCode MatchType );
	void QueryGetPvPLadderRankList( CDNUserSession *pSession, LadderSystem::MatchType::eCode MatchType, int nPage, BYTE cClassCode, BYTE cSubClassCode, const WCHAR* wszGuildName );
	void QueryGetPvPLadderRankInfo( CDNUserSession *pSession, LadderSystem::MatchType::eCode MatchType, const WCHAR* wszCharName );
#endif // #if defined(PRE_ADD_PVP_RANKING)
	void QueryGetPVPGhoulScores( BYTE cThreadID, CDNUserSession *pSession );
	void QueryAddPVPGhoulScores( CDNUserSession *pSession );
#if defined(PRE_ADD_PVP_TOURNAMENT)
#if defined( PRE_PVP_GAMBLEROOM )
	void QueryAddTournamentResult(BYTE cThreadID, int nWorldSetID, STournamentMatchInfo* pTournamentMatchInfo, INT64 nGambleDBID);
#else // #if defined(PRE_PVP_GAMBLEROOM )
	void QueryAddTournamentResult(BYTE cThreadID, int nWorldSetID, STournamentMatchInfo* pTournamentMatchInfo);
#endif // #if defined(PRE_PVP_GAMBLEROOM )
#endif //#if defined(PRE_ADD_PVP_TOURNAMENT)
	void QueryGetGuildRecruitCharacter( CDNUserSession *pSession, UINT nGuildDBID );
	void QueryGetMyGuildRecruit( CDNUserSession *pSession );
	void QueryGetGuildRecruitRequestCount( CDNUserSession *pSession );
	void QueryGetPageGuildRecruit( CDNUserSession *pSession, UINT uiPage, BYTE cJob, BYTE cLevel, BYTE cPurposeCode, WCHAR* pwszGuildName, BYTE cSortType );
	void QueryRegisterOnGuildRecruit( CDNUserSession *pSession, WCHAR* pwszText, BYTE* cClassCode, int nMinLevel, int nMaxLevel, BYTE cPurposeCode, bool bCheckHomePage );
	void QueryRegisterModGuildRecruit( CDNUserSession *pSession, WCHAR* pwszText, BYTE* cClassCode, int nMinLevel, int nMaxLevel, BYTE cPurposeCode, bool bCheckHomePage );
	void QueryRegisterInfoGuildRecruit( CDNUserSession *pSession );
	
	void QueryRegisterOffGuildRecruit( CDNUserSession *pSession );
	void QueryRequestOnGuildRecruit( CDNUserSession *pSession, UINT nGuildDBID );
	void QueryRequestOffGuildRecruit( CDNUserSession *pSession, UINT nGuildDBID );
	// Query for cheat	
	void QueryCheatDelGuildRecruit( CDNUserSession *pSession, UINT nGuildDBID, BYTE DelType );
	void QueryAcceptOnGuildRecruit( CDNUserSession *pSession, UINT nGuildDBID, INT64 biAcceptCharacterDBID, short wGuildSize, WCHAR* pwszCharacterName );
	void QueryAcceptOffGuildRecruit( CDNUserSession *pSession, UINT nGuildDBID, INT64 biDenyCharacterDBID, WCHAR* pwszCharacterName );

#if defined( PRE_PARTY_DB )
	void QueryDelPartyForServer( int iWorldID, int iServerID );
#if defined( _GAMESERVER )
	void QueryDelPartyForGameServer( int iWorldID, int iServerID );
#endif
	void QueryGetListParty( int iWorldID );
#if defined(PRE_FIX_62281)
	void QueryAddParty( CDNUserSession *pSession, int iManagedID, Party::LocationType::eCode LocationType, int iLocationID, const CSCreateParty* pPacket, const WCHAR * wszAfterInviteCharacterName=NULL, bool bCheat=false );		
#else
	void QueryAddParty( CDNUserSession *pSession, int iManagedID, Party::LocationType::eCode LocationType, int iLocationID, const CSCreateParty* pPacket, UINT uiAfterInvenAccountDBID=0, bool bCheat=false );		
#endif
	// Function to save party and party members simultaneously (used for Village -> GameServer, GameServer -> Village)
#if defined( _GAMESERVER )
	void QueryAddPartyAndMemberGame( CDNUserSession *pSession, int iManagedID, Party::LocationType::eCode LocationType, int iLocationID, const Party::AddPartyAndMemberGame *PartyData, int iRoomID = 0 );
	void QueryModParty( CDNUserSession* pSession, const Party::Data& PartyData, int iBitFlag );
#endif
#if defined( _VILLAGESERVER )
	void QueryAddPartyAndMemberVillage( int iWorldID, UINT uiAccountDBID, int iManagedID, Party::LocationType::eCode LocationType, const Party::AddPartyAndMemberVillage *PartyData );
	void QueryModParty( CDNUserSession *pSession, CDNParty* pParty, int iManagedID, Party::LocationType::eCode LocationType, int iLocationID, const CSPartyInfoModify* pPacket );
	//Update party channel info when moving via cheat
	void QueryModParty( CDNUserSession *pSession, CDNParty* pParty, int iManagedID, Party::LocationType::eCode LocationType, int iLocationID );		
#endif	
	void QueryDelParty( int iWorldID, TPARTYID PartyID );
	void QueryJoinParty( CDNUserSession *pSession, int iWorldID, TPARTYID PartyID, int iMaxUserCount );
	void QueryOutParty( int iWorldID, UINT uiAccountDBID, INT64 biCharacterDBID, UINT nSessionID, TPARTYID PartyID, Party::QueryOutPartyType::eCode Type );
	void QueryModPartyLeader( CDNUserSession* pSession, TPARTYID PartyID );
	void QueryGetPartyJoinMembers( CDNUserSession* pSession, TPARTYID PartyID );
#if defined( PRE_ADD_NEWCOMEBACK )
	void QueryModComebackFlag( CDNUserSession* pSession, bool bRewardComeback );
	void QueryModMemberComebackInfo( CDNUserSession* pSession, TPARTYID PartyID );
#endif
#endif // #if defined( PRE_PARTY_DB )

#if defined( PRE_WORLDCOMBINE_PARTY )
	void QueryGetListWorldParty( int iWorldID );
	void QueryAddWorldParty( int iManagedID, Party::LocationType::eCode LocationType, int iLocationID, Party::Data *PartyData );
#endif

#if defined(PRE_ADD_DOORS_PROJECT)
#if defined(_GAMESERVER)
	void QuerySaveCharacterAbility(CDNUserSession* pSession);
#endif //#if defined(_GAMESERVER)
#endif //#if defined(PRE_ADD_DOORS_PROJECT)
#if defined (PRE_ADD_BESTFRIEND)
	void QueryGetBestFriend(BYTE cThreadID, CDNUserSession *pSession, bool bSend);
	void QueryRegistBestFriend(CDNUserSession *pSession, UINT nAccountDBID, INT64 biCharacterDBID, LPCWSTR lpwszCharacterName, INT64 biRegistSerial, int nItemID);
	void QueryCancelBestFriend(CDNUserSession *pSession, bool bCancel);
	void QueryCloseBestFriend(CDNUserSession *pSession);
	void QueryEditBestFriendMemo(CDNUserSession *pSession, LPCWSTR lpwszMemo);
#endif // #if defined (PRE_ADD_BESTFRIEND)

#if defined( PRE_PRIVATECHAT_CHANNEL )
	void QueryGetPrivateChatChannelInfo(BYTE cThreadID, int nWorldID);
	void QueryGetPrivateChatChannelMember(CDNUserSession *pSession, INT64 nChannelID);
	void QueryAddPrivateChatChannel(CDNUserSession *pSession, WCHAR* wszChannelName, int nPassWord);
	void QueryAddPrivateChatChannelMember(CDNUserSession *pSession, INT64 nChannelID, PrivateChatChannel::Common::eModType eType );
	void QueryInvitePrivateChatChannelMember(CDNUserSession *pSession, INT64 nChannelID, UINT nInviteAccountID );
	void QueryOutPrivateChatChannelMember(CDNUserSession *pSession, PrivateChatChannel::Common::eModType eType);
	void QueryKickPrivateChatChannelMember(CDNUserSession *pSession, WCHAR* wszKickName, INT64 biKickCharacterDBID);
	void QueryModPrivateChatChannelInfo( CDNUserSession *pSession, WCHAR* wszChannelName, int nModType, int nPassWord, INT64 biCharacterDBID );
	void QueryModPrivateMemberServerID( CDNUserSession *pSession );
	void QueryDelPrivateMemberServerID( BYTE cThreadID, int nWorldID, int nServerID );
#endif

#if defined( PRE_FIX_67546 )
	void QueryAddThreadCount( BYTE cThreadID );
#endif	

#if defined( PRE_WORLDCOMBINE_PVP )
#if defined( _GAMESERVER )
	void QueryAddWorldPvPRoom( BYTE cThreadID, int nWorldID, UINT nGMAccountDBID, int nServerID, int nRoomID, TWorldPvPMissionRoom* pMissonRoom );
	void QueryAddWorldPvPRoomMember( BYTE cThreadID, int nWorldID, CDNUserSession* pSession, BYTE cMaxMemberCount, int nWorldPvPRoomDBIndex, bool bObserverFlag );	
	void QueryDelWorldPvPRoomForServer( int nServerID );
	void QueryDelWorldPvPRoom( int nWorldID, int nPvPRoomDBIndex );
	void QueryDelWorldPvPRoomMember( BYTE cThreadID, int nWorldID, int nPvPRoomDBIndex,  CDNUserSession* pSession );	
#endif
#if defined( _VILLAGESERVER )
	void QueryGetListWorldPvPRoom();
	void QueryUpdateWorldPvPRoom(BYTE cThreadID, int nWorldID);	
#endif
#endif

#if defined( PRE_ALTEIAWORLD_EXPLORE )
	void QueryResetAlteiaWorldEvent( BYTE cThreadID, int nWorldID );
	void QueryAddAlteiaWorldEventTime( BYTE cThreadID, int nWorldID, time_t tAlteiaEventStartTime, time_t tAlteiaEventEndTime );
	void QueryAddAlteiaWorldPlayResult( CDNUserSession *pSession, int nGoldKeyCount, DWORD dwPlayTime, int nGuildID );
	void QueryGetAlteiaWorldInfo( BYTE cThreadID, CDNUserSession *pSession );
	void QueryGetAlteiaWorldPrivateGoldKeyRank( CDNUserSession *pSession );
	void QueryGetAlteiaWorldPrivatePlayTimeRank( CDNUserSession *pSession );
	void QueryGetAlteiaWorldGuildGoldKeyRank( CDNUserSession *pSession );
	void QueryGetAlteiaWorldSendTicketList( BYTE cThreadID, CDNUserSession *pSession );
	void QueryAddAlteiaWorldSendTicketList( CDNUserSession *pSession, INT64 biRecvCharacterDBID, WCHAR* wszSendCharacterName, int nSendMaxCount );
	void QueryResetAlteiaWorldPlayAlteia( CDNUserSession *pSession, AlteiaWorld::ResetType::eResetType eType );
#endif

#if defined( PRE_ADD_STAMPSYSTEM )
	void QueryGetListCompleteChallenges( BYTE cThreadID, CDNUserSession* pSession );
	void QueryInitCompleteChallenge( CDNUserSession* pSession );
	void QueryAddCompleteChallenge( CDNUserSession* pSession, BYTE cChallengeIndex, int nWeekDay );
#endif // #if defined( PRE_ADD_STAMPSYSTEM )
#if defined(PRE_ADD_GAMEQUIT_REWARD)
	void QueryModNewbieRewardFlag(CDNUserSession *pSession, bool bRewardFlag);
#endif	// #if defined(PRE_ADD_GAMEQUIT_REWARD)
#if defined(PRE_ADD_CP_RANK)
	void QueryAddStageClearBest( CDNUserSession* pSession, int nMapID, DBDNWorldDef::ClearGradeCode::eCode ClearCode, int nClearPoint, int nLimitLevel);
	void QueryGetStageClearBest( CDNUserSession* pSession, int nMapID );
	void QueryGetStageClearPersonalBest( CDNUserSession* pSession, int nMapID );
	void QueryInitStageCP(CDNUserSession * pSession, char cInitType);
#endif
#if defined( PRE_FIX_76282 )
	void SendSyncGoPvPLobby( CDNUserSession* pSession );
#endif // #if defined( PRE_FIX_76282 )
#if defined(PRE_ADD_EQUIPLOCK)
	void QueryAddLockItem( CDNUserSession* pSession, DBDNWorldDef::ItemLocation::eCode Code, BYTE cItemSlotIndex );
	void QueryRequestItemLock( CDNUserSession* pSession, DBDNWorldDef::ItemLocation::eCode Code, BYTE cItemSlotIndex, int nUnLockWaitTime );
	void QueryUnLockItem( CDNUserSession* pSession, DBDNWorldDef::ItemLocation::eCode Code, BYTE cItemSlotIndex, int nItemID, INT64 biItemSerial );
#endif	// #if defined(PRE_ADD_EQUIPLOCK)
#if defined(PRE_ADD_TALISMAN_SYSTEM)
	void QueryModTalismanSlotBitmap( CDNUserSession* pSession, int nTalismanOpenFlag );
#endif	// #if defined(PRE_ADD_TALISMAN_SYSTEM)
#if defined(PRE_ADD_CHAOSCUBE)
	void QueryGetListChaosGaugeItems( BYTE cThreadID, CDNUserSession* pSession );
	void QueryUpsChaosGaugeItem( CDNUserSession* pSession, int nChaosGaugeType, int nItemGauge );
#endif	// #if defined(PRE_ADD_CHAOSCUBE)
#if defined(PRE_ADD_DWC)
	void QueryAddDWCTeam( CDNUserSession* pSession, const WCHAR* wszTeamName );
	void QueryAddDWCTeamMember( CDNUserSession* pSession, UINT nTeamID, INT64 biTargetCharDBID, const WCHAR* wszCharName, int nMaxMemberCount = DWC::MAX_TEAM_MEMBER );
	void QueryDelDWCTeamMember( CDNUserSession* pSession, UINT nTeamID );
	void QueryGetDWCTeamByChar( BYTE cThreadID, CDNUserSession* pSession, bool bNeedMemberList = true );
	void QueryGetListDWCTeamMember( CDNUserSession* pSession, UINT nTeamID );
	void QueryGetPageDWCRanking( CDNUserSession* pSession, int nPageNum, int nPageSize = DWC::MAX_RANK_PER_PAGE );
	void QueryGetDWCRanking( CDNUserSession* pSession, BYTE cFindType, const WCHAR* wszFindName );
	void QueryGetDWCChannel( BYTE cThreadID, CDNUserSession* pSession );
#endif	// #if defined(PRE_ADD_DWC)
};
