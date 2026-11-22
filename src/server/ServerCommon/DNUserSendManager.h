#pragma once

/*---------------------------------------------------------------------------------------
									CDNUserSendManager

- Collection of Send functions sent to Client
- Here too, both Village and Game must use it, so it's plastered with #ifdefs...
---------------------------------------------------------------------------------------*/

class CDNUserItem;
class CDNUserSession;
#if defined(_VILLAGESERVER)
class CDNPvPRoom;
#endif
class CDNGesture;

class CDNGuildBase;
class CDNParty;

#if defined( PRE_ADD_SECONDARY_SKILL )
class CSecondarySkillRecipe;
#endif // #if defined( PRE_ADD_SECONDARY_SKILL )
#if defined( _VILLAGESERVER )
namespace LadderSystem
{
	class CRoom;
}
#endif // #if defined( _VILLAGESERVER )

class CDNUserSendManager
{
protected:
	CDNUserSession* m_pSession;

public:
	CDNUserSendManager(CDNUserSession *pSession);
	virtual ~CDNUserSendManager(void);

	// System


	void Send( int iMainCmd, int iSubCmd, char* pData, int iLen );
#if defined(PRE_MOD_DUPLICATION_LOGIN_MES)
	void SendCompleteDetachMsg(int nRetCode, const WCHAR * pwszDetachReason, bool IsDisConnectByClient = false);
#else	// #if defined(PRE_MOD_DUPLICATION_LOGIN_MES)
	void SendCompleteDetachMsg(int nRetCode, const WCHAR * pwszDetachReason);
#endif	// #if defined(PRE_MOD_DUPLICATION_LOGIN_MES)
	void SendConnectVillage(int nRet, int nAttr, bool bWithOutLoading = false);
	void SendGameInfo( GameTaskType::eType GameTaskType, BYTE cReqGameIDType, ULONG nIP, USHORT nPort, USHORT nTcpPort, UINT nAccountDBID, INT64 biCertifyingKey);
	void SendReConnectReq();
	void SendReconnectLogin(int nRet, UINT nAccountDBID, INT64 biCertifyingKey);
	void SendCountDownMsg(bool bIsStart, int nCountTime, UINT nCode, BYTE msgboxType);
#if defined(PRE_ADD_VIP)
	void SendVIPAutomaticPay(bool bAuto);
#endif	// #if defined(PRE_ADD_VIP)

	// Char
	void SendMapInfo(UINT nMapIndex, BYTE cMapArrayIndex, BYTE cEnviIndex, BYTE cEnviArrayIndex, int nChannelID, int nChannelIdx, int nMeritID );
	void SendEnter(CDNUserSession *pSession);
	void SendEnterUser(CDNUserSession *pSession);
	void SendLeaveUser(UINT nSessionID, char cType, int nAccountLevel);
	void SendEnterNpc(UINT nNpcObjectID, int nNpcID, float x, float y, float z, float fRotate);
	void SendLeaveNpc(UINT nObjectID);
	void SendEnterPropNpc(UINT nNpcObjectID, int nPropUniqueID);
	void SendLeavePropNpc(int nPropUniqueID);

	void SendQuickSlotList(TQuickSlot *QuickSlotArray);
	void SendFatigue(UINT nSessionID, short wFatigue, short wWeekFatigue, short wPCBangFatigue, short wEventFatigue, short wVIPFatigue);
	void SendFatigueResult(UINT nSessionID, int iResult);
	void SendHide(UINT nSessionID, bool bHide);
	void SendPCBang(char cPCBangGrade, MAPCBangResult *pPacket);
	void SendNestClear(TNestClearData *NestClearList);
	void SendUpdateNestClear(TNestClearData *pNestData);

	void SendKeySetting( TKeySetting* pKeySetting );
	void SendPadSetting( TPadSetting* pPadSetting );
	void SendPvPFatigueReward( int nGainExp, int nGainMedal );

	void SendPvPLadderScoreInfo( const TPvPLadderScoreInfo* pLadderScoreInfo );
	void SendPvPLadderScoreInfoByJob( TAGetListPvPLadderScoreByJob* pA );
	void SendPvPLadderPointRefresh( int iPoint );
	void SendPvPLadderInviteUser( int iRet, const WCHAR* pwszCharName );
	void SendPvPLadderInviteConfirmReq( const WCHAR* pwszCharName, LadderSystem::MatchType::eCode MatchType, int iCurUserCount, int iAvgGradePoint );
	void SendPvPLadderInviteConfirm( int iRet, const WCHAR* pwszCharName );
	void SendPvPLadderKickOut( int iRet );

	void SendPvPGhoulScores( const TPvPGhoulScores* pGhoulSocres);
	void SendPvPHolyWaterKillCount(UINT nSessionID , UINT nKillerSessionID);
#ifdef PRE_ADD_COLOSSEUM_BEGINNER
	void SendPvPChangeChannelResult(BYTE cType, int nRetCode);
#endif		//#ifdef PRE_ADD_COLOSSEUM_BEGINNER
#if defined(PRE_ADD_REBIRTH_EVENT)
	void SendRebirthMaxCoin(int nRebirthMaxCoin);
#endif

	// Actor
#if defined(_VILLAGESERVER)
	void SendActorMsg(UINT nSessionID, USHORT wActorProtocol, int nSize, char *pData, TParamData* pParamData=NULL );
	void SendAddExp(UINT nSessionID, int nExp, int nAddExp, int nEventBonusExp = 0, int nPcBangExp = 0);	// nExp = current exp, nAddExp = final added exp, nEventBonusExp = event bonus exp, nPcBangExp = PC bang bonus exp
	void SendLevelUp(UINT nSessionID, int nLevel, int nExp);	// nLevel = current level, nExp = current exp
#endif
	void SendChangeJob(UINT nSessionID, int nJobID);
	void SendAddCoin(UINT nSessionID, INT64 nTotalCoin, INT64 nAddCoin);	// nTotalCoin = existing coins, nAddCoin = coins to add (negative values might also work)
	void SendGhostType( UINT nSessionID, Ghost::Type::eCode Type );
	void SendUdpPing( UINT nSessionID, DWORD dwTick );

	// Party
	void SendCreateParty(int nRetCode, TPARTYID PartyID, bool bIsAutoCreate = false, ePartyType PartyType = _NORMAL_PARTY);
	void SendJoinParty(int nRetCode, TPARTYID nPartyID, const WCHAR *pwszPartyName);
#if defined( PRE_PARTY_DB )
#if defined( PRE_WORLDCOMBINE_PARTY )	
	void SendRefreshParty(UINT nLeaderSessionID, int nCount, bool bHasVoiceChat, SPartyMemberInfo *pMemberInfo, short nRetCode, Party::Data *PartyData, int nWorldCombinePartyTableIndex);
#else
	void SendRefreshParty(UINT nLeaderSessionID, const WCHAR * pName, BYTE cPartyMemberMax, BYTE cUserLvLimitMin, int nCount, TPARTYITEMLOOTRULE ItemLootRule, TITEMRANK ItemLootRank, bool bHasVoiceChat, int nTargetMapIdx, TDUNGEONDIFFICULTY Difficulty, SPartyMemberInfo *pMemberInfo, short nRetCode, ePartyType PartyType, int iBitFlag, int iPassword );
#endif
#else
	void SendRefreshParty(UINT nLeaderSessionID, const WCHAR * pName, const WCHAR * pPass, BYTE cPartyMemberMax, BYTE cUserLvLimitMin, BYTE cUserLvLimitMax, int nCount, TPARTYITEMLOOTRULE ItemLootRule, TITEMRANK ItemLootRank, bool bHasVoiceChat, int nTargetMapIdx, TDUNGEONDIFFICULTY Difficulty, SPartyMemberInfo *pMemberInfo, short nRetCode, BYTE cIsJobDice, BYTE cPartyType);
#endif // #if defined( PRE_PARTY_DB )
	void SendPartyOut(int nRetCode, char cKickKind = -1, UINT nSessionID = 0);
	void SendKickPartymember(UINT nSessionID, int nRet);
	void SendSwapPartyLeader(UINT nSessionID, int nRetCode);
#if defined( PRE_PARTY_DB )
	void SendPartyList( int iRet, const CSPartyListInfo* pPacket, std::list<CDNParty*>& PartyList );
#else
	void SendPartyList(std::list <SPartyListInfo> * pList, int nWholeCnt, int nRet);
#endif // #if defined( PRE_PARTY_DB )
	void SendRefreshGateInfo(char *pGateNo, int nCount, int nRet);
	void SendGateInfo(UINT nLeaderUID, char cGateNo, char cMapCount, char *cCanEnter, char *cPermitFlag );

	void SendSyncEquipData( TPartyMemberEquip *EquipArray, int iCount );

	void SendDefaultPartsData( const std::vector<TPartyMemberDefaultParts>& vDefaultParts );
	void SendWeaponOrderData( const std::vector<TPartyMemberWeaponOrder>& vWeaponOrder );
	void SendEquipData( const std::vector<TPartyMemberEquip>& vEquip );
	void SendCashEquipData( const std::vector<TPartyMemberCashEquip>& vEquip );
	void SendGlyphEquipData( const std::vector<TPartyMemberGlyphEquip>& vGlyphEquip );
#if defined(PRE_ADD_TALISMAN_SYSTEM)
	void SendTalismanEquipData( const std::vector<TPartyMemberTalismanEquip>& vTalismanEquip );
#endif	// #if defined(PRE_ADD_TALISMAN_SYSTEM)
#ifdef PRE_MOD_SYNCPACKET
	void SendVehicleEquipData( const std::vector<TPartyVehicle>& vVehicle );
	void SendPetEquipData( const std::vector<TPartyVehicle>& vPet );
#else		//#ifdef PRE_MOD_SYNCPACKET
	void SendVehicleEquipData( const std::vector<TVehicle>& vVehicle );
	void SendPetEquipData( const std::vector<TVehicle>& vPet );
#endif		//#ifdef PRE_MOD_SYNCPACKET
	void SendSkillData( const std::vector<TPartyMemberSkill>& vSkill );
	void SendPartyEtcData( const std::vector<TPartyEtcData>& vEtcData );
	void SendPartyGuildData( const std::vector<TPartyMemberGuild::TPartyMemberGuildView>& vMemberGuildView );
	void SendSyncSkillData(TPartyMemberSkill *pSkillArray, int iCount);
#if defined (PRE_ADD_BESTFRIEND)
	void SendPartyBestFriend( const std::vector<TPartyBestFriend>& vMemberBestFriend );
#endif
#if defined( PRE_PARTY_DB )
	void SendStartStage( bool bDirectConnect, int nMapIndex = -1, char cGateNo = -1, int nRandomSeed = -1, TDUNGEONDIFFICULTY Difficulty = Dungeon::Difficulty::Easy, int nExtendDropRate = 0 );
#else
	void SendStartStage( bool bDirectConnect, int nMapIndex = -1, char cGateNo = -1, int nRandomSeed = -1, TDUNGEONDIFFICULTY Difficulty = 0, int nExtendDropRate = 0 );
#endif // #if defined( PRE_PARTY_DB )
	void SendSelectStage( char cSelectIndex );
	void SendVillageInfo(const char * pIP, USHORT nPort, short nRet, UINT nAccountDBID, INT64 biCertifyingKey);
	void SendStartVillage();
	void SendCancelStage( int nRetCode );
	void SendChangeLeader(UINT nSessionID);
#if defined( PRE_PARTY_DB )
	void SendPartyInviteMsg(TPARTYID PartyID, const WCHAR * pPartyName, int iPassword, const WCHAR * pSrcName, BYTE cMaxUserCount, BYTE cCurUserCount, USHORT nAvrLevel, int nGameServerID = -1, int nGameMapIdx = -1);
#else
	void SendPartyInviteMsg(TPARTYID PartyID, const WCHAR * pPartyName, const WCHAR * pPass, const WCHAR * pSrcName, BYTE cMaxUserCount, BYTE cCurUserCount, USHORT nAvrLevel, int nGameServerID = -1, int nGameMapIdx = -1);
#endif // #if defined( PRE_PARTY_DB )
	void SendPartyIniviteNotice(const WCHAR * pwszInvitedName, int nRetCode);
	void SendPartyInviteFail(int nRet);
	void SendPartyInviteDenied(const WCHAR * pDest, int nRet);
	void SendPartyMemberInfo(SCPartyMemberInfo * pPacket);
	void SendChannelInfo(SCChannelList * pPacket);
	void SendMoveChannelFail(int nRet);
	void SendPartyInfoErr(short nRetCode);
	void SendSelectDungeonInfo(int nMapIndex, BYTE cDiffyculty);
	void SendQuickJoinFail(short nRetCode);
	void SendPartyModify(short nRetCode);
	void SendVerifyJoinParty(int nPartyIdx, int nChannelID, short nRetCode);
	void SendPartyMemberMove(UINT nSessionID, EtVector3 vPos);
	void SendPartyMemberPart(CDNUserSession *pSession);
	void SendRequestJoinGetReversionItem(const TItem& reversionItem, const DWORD& dropUniqueID);
	void SendRollDiceForGetReversionItem(UINT nSessionID, bool bRoll, int diceValue);
	void SendResultGetReversionItem(UINT nSessionID, const TItem& itemInfo, DWORD dropItemUniqueId);
	void SendAskJoinResult(int nRet);
	void SendAskJoinToLeader(const WCHAR * pwszCharacterName, char cJob, BYTE cLevel);
#if defined( PRE_PARTY_DB )
	void SendAskJoinAgreeInfo(TPARTYID PartyID, int iPassword );
#else
	void SendAskJoinAgreeInfo(TPARTYID PartyID, const WCHAR * pwszPassword);
#endif // #if defined( PRE_PARTY_DB )
	void SendUpdatePartyUI(UINT nSessionID, bool bLeader);
	void SendPartySwapMemberIndex(CSPartySwapMemberIndex * pPacket, int nRetCode);
	void SendReadyRequest();
	void SendMemberAbsent(UINT nSessionID, BYTE cAbsentFlag);
	void SendVerifyDungeonAlloed(int nMapIdx, BYTE cDifficulty, bool * pAllowed);
	void SendPartyBonusValue(int nUpkeepBonusRate, int nFriendBonus, bool bUpkeepMax = false, int nBiginnerGuildBonus = 0);
#if defined(PRE_MOD_REQ_JOIN_PARTY_ANSWER_MSG_APP)
	void SendPartyInfo( int nRetCode, CDNParty* pParty );
#endif

	// Item
	void SendEquipList(CDNUserItem *pItem);
	void SendInvenList(CDNUserItem *pItem);
	void SendCashInvenList(int nTotalCount, int nCount, TItem *CashItemList);
#if defined(_VILLAGESERVER) && defined(PRE_ADD_CASH_REFUND)
	void SendPaymentItemList(CDNUserItem *pItem);
	void SendPaymentPackageItemList(CDNUserItem *pItem);
#endif

	void SendMoveItem(char cType, BYTE cSrcIndex, BYTE cDestIndex, const TItem *pSrcItem, const TItem *pDestItem, int nRetCode);
	void SendMoveCashItem(char cType, BYTE cEquipIndex, const TItem *pEquipItem, INT64 biInvenSerial, const TItem *pInvenItem, int nRetCode);
#if defined(PRE_ADD_SERVER_WAREHOUSE)
	void SendServerWareHouseList(CDNUserItem* pItem);
	void SendServerWareHouseCashList(CDNUserItem* pItem);
	void SendMoveServerWareItem(char cType, BYTE cInvenIndex, INT64 biWareSerial, const TItem *pInvenItem, const TItem *pServerWareItem, int nRetCode);
	void SendMoveServerWareCashItem(char cType, INT64 biSrcItemSerial, const TItem *pSrcItem, const TItem *pDstItem, int nRetCode);
#endif //#if defined(PRE_ADD_SERVER_WAREHOUSE)

	void SendRefreshInvenPickUpByType(char cType, int nRetCode, BYTE cInvenIndex, TItem *pPickUpItem, short wCount);
	void SendPickUp(int nRetCode, BYTE cInvenIndex, TItem *pPickUpItem, short wCount);
	void SendRemoveItem(char cType, BYTE cSrcIndex, TItem *pDropItem, int nRetCode);
	// void SendMoveCashItem(char cType, BYTE cSrcIndex, BYTE cDestIndex, TItem *pSrcItem, TItem *pDestItem, int nRetCode);
	void SendChangeEquip(UINT nSessionID, TItemInfo &ItemInfo);
	void SendChangeCashEquip(UINT nSessionID, TItemInfo &ItemInfo);
	void SendRefreshInven(BYTE cInvenIndex, const TItem *pInvenItem, bool bNewSign = true);
	void SendRefreshCashInven(const TItem &InvenItem, bool bNewSign = true);
	void SendRefreshCashInven(std::vector<TItem> VecItemList, bool bNewSign = true);
	void SendRefreshWarehouse(BYTE cWarehouseIndex, const TItem *pWarehouseItem);
	void SendWarehouseList(INT64 nWarehouseCoin, CDNUserItem *pItem);
	void SendUseItem(int nInvenType, BYTE cInvenIndex, INT64 biItemSerial);
	void SendEmblemCompoundOpen( int nRet );
	void SendItemCompoundOpen( int nRet );
	void SendEmblemCompoundRes( int nSuccess, int nResultItemID );
	void SendItemCompoundRes( int nSuccess, int nResultItemID, char cLevel, char cOption );
	void SendEmblemCompoundCancel( int nRet );
	void SendItemCompoundCancel(int nRet );
	void SendInventoryMaxCount(BYTE cInvenMaxCount);
	void SendGlyphExtendCount( int iCount );
	void SendGlyphExpireData( TCashGlyphData *GlyphExpireData );	//Send timed heraldry slot info

	void SendWarehouseMaxCount(BYTE cWareMaxCount);
	void SendRebirthCoin(int nRet, int nUsableRebirthCoin, eRebirthType type, UINT nSessionID);
	void SendRefreshRebirthCoin(UINT nSessionID, BYTE cRebirthCoin, BYTE cPCBangRebirthCoin, short nCashRebirthCoin, BYTE cVIPRebirthCoin);
	void SendQuestInvenList(CDNUserItem *pItem);
	void SendRefreshQuestInven(BYTE cIndex, TQuestItem &QuestItem);
#if defined(PRE_ADD_EQUIPED_ITEM_ENCHANT)
	void SendRefreshEquip(char cEquipIndex, const TItem *pEquipItem, bool bRefreshtate=false);
#else
	void SendRefreshEquip(char cEquipIndex, const TItem *pEquipItem);
#endif
	void SendRepairEquip(int nRet);
	void SendRepairAll(int nRet);
	void SendMoveCoin(char cType, INT64 nAddCoin, INT64 nInventoryCoin, INT64 nWarehouseCoin, int nRet);
	void SendRequestRandomItem(int nInvenType, BYTE cInvenIndex, INT64 biItemSerial, short nRetCode);
	void SendCompleteRandomItem( int nInvenType, char cLevel, int nResultItemID, int nItemCount, short nRetCode );
#if defined(PRE_ADD_EQUIPED_ITEM_ENCHANT)
	void SendEnchant(int nItemIndex, bool bEnchantEquipedItem, int nRet);
#else
	void SendEnchant(int nInvenIndex, int nRet);
#endif
	void SendEnchantComplete(int nItemID, char cLevel, char cOption, int nRet);
	void SendEnchantCancel(int nRet);
	void SendGlyphTimeInfo(int nDelayArray, int nRemainArray);
	void SendSortInventory(int nRet);
	void SendDecreaseDurabilityInventory( char cType, void *pValue );
	void SendBroadcastingEffect(UINT nSessionID, char cType, char cState);
	void SendChangeGlyph(UINT nSessionID, TItemInfo &ItemInfo);
	void SendVehicleEquipList(TVehicle *pVehicleEquip);
	void SendVehicleInvenList(int nTotalCount, int nCount, TVehicleCompact *VehicleList);
	void SendChangeVehicleParts(UINT nSessionID, int nIndex, const TItem &Item);
	void SendRefreshVehicleInven(TVehicle &InvenItem, bool bNewSign = true);
	void SendRefreshVehicleInven(std::vector<TVehicle> VecItemList, bool bNewSign = true);
	void SendChangeVehicleColor(UINT nSessionID, INT64 biSerial, DWORD dwColor, char cPetPartsColor);
	void SendPetEquipList(const TVehicle *pPetEquip);
	void SendChangePetParts(UINT nSessionID, int nIndex, TItem &Item);
	void SendChangePetBody(UINT nSessionID, TVehicleCompact &PetInfo);
	void SendAddPetExp(UINT nSessionID, INT64 biPetSerial, int nExp);
	void SendChangePetNameResult(int nRet, UINT nUserSessionID, INT64 biItemSerial, const WCHAR* wszName);
	void SendPetRename(UINT sessionID, const wchar_t* pName);
	void SendSortWarehouse(int nRet);
	void SendRemoveCash(const TItem *pItem, int nRet);
	void SendPetCurrentSatiety(INT64 biPetSerial, int nCurrentSatiety);
	void SendPetFoodEat(int nResult, int nIncSatiety );
#if defined(PRE_ADD_EXPUP_ITEM)
	void SendDailyLimitItemErr(int nWeightTableIndex);
#endif

#if defined( PRE_ADD_LIMITED_SHOP )
#if defined( PRE_FIX_74404 )
	void SendLimitedShopItemData(std::list<LimitedShop::LimitedItemData> &listLimitedShop, bool bAllClear=false);
	void SendLimitedShopItemData(int nShopID, int nItemID, int nBuyCount, int nResetCycle);
#else
	void SendLimitedShopItemData(std::map<int, LimitedShop::LimitedItemData> &mapLimitedShop, bool bAllClear=false);
	void SendLimitedShopItemData(int nItemID, int nBuyCount, int nResetCycle);
#endif
#endif

#if defined(PRE_PERIOD_INVENTORY)
	void SendPeriodInventory(bool bEnable, __time64_t tExpireDate);
	void SendPeriodWarehouse(bool bEnable, __time64_t tExpireDate);
	void SendSortPeriodInventory(int nRet);
	void SendSortPeriodWarehouse(int nRet);
#endif	// #if defined(PRE_PERIOD_INVENTORY)

#if defined(PRE_ADD_TALISMAN_SYSTEM)
	void SendTalismanOpenSlot(int nRet, int nTalismanOpenFlag);
	void SendChangeTalisman(UINT nSessionID, char cSlotIndex, TItem &TalismanItem);
	void SendTalismanExpireData(bool bOpenFlag, __time64_t tExpireDate);
#endif	// #if defined(PRE_ADD_TALISMAN_SYSTEM)

#if defined( PRE_ADD_STAGE_USECOUNT_ITEM )
	void SendStageUseLimitItem( int nItemID, int nUseCount );
#endif
	void SendSpecialItemList( const int* pItemIDs, int nCount );
	void SendAutoUnpackResult( int nRet );
	void SendDeleteExpireItemList( const std::vector<INT64>& vecSerials );

	// Item Goods
	void SendRequestCashWorldMsg(INT64 biItemSerial, char cType);
	void SendChangeColor(UINT nSessionID, int nItemType, DWORD dwColor);
	void SendUnsealItemResult(int nRet, int nSlotCount);
	void SendSealItemResult(int nRet, int nSlotCount);
	void SendGuildRenameResult(int nRet);
	void SendGuildMarkResult(int nRet, short wGuildMark, short wGuildMarkBG, short wGuildMarkBorder);
	void SendCharacterRenameResult(int nRet, UINT nUserSessionID, const WCHAR* wszName, __time64_t tNextChangeDate = 0);	
#ifdef PRE_ADD_GACHA_JAPAN
	void SendGachaShopOpen_JP( int nGachaShopIndex );
	void SendGachaResult_JP( int nResult, int nResultItemID );
#endif // PRE_ADD_GACHA_JAPAN
	void SendCosMixOpen(int nRet);
	void SendCosMixClose(int nRet, int nSubCmd);
	void SendCosMixComplete(int nRet, int nItemId, char cOption);
	void SendCosDesignMixOpen(int nRet);
	void SendCosDesignMixClose(int nRet, int nSubCmd);
	void SendCosDesignMixComplete(int nRet, INT64 biItemSertial = 0, int nLookItemID = 0);
#ifdef PRE_ADD_COSRANDMIX
	#ifdef PRE_ADD_COSRANDMIX_ACCESSORY
	void SendCosRandomMixOpen(int nRet, CostumeMix::RandomMix::eOpenType type);
	#else
	void SendCosRandomMixOpen(int nRet);
	#endif
	void SendCosRandomMixClose(int nRet, int nSubCmd);
	void SendCosRandomMixComplete(int nRet, int nResultItemID);
#endif
	void SendPotentialItemResult( int nRet, int nSlotIndex );
	void SendCharmItemRequest(int nInvenType, short sInvenIndex, INT64 biItemSerial, short nRetCode, std::vector<TCharmItem> * pvRandomGiveItemList = NULL);
	void SendCharmItemComplete(int nInvenType, int nResultItemID, int nResultItemCount, int nResultPeriod, short nRetCode);
	void SendPetalTokenResult(int nUseItemID, int nTotalPetal);
	void SendAppellationGainResult(int nAppellationArrayIndex, int nRet);
	void SendEnchantJewelItemResult( int nRet, int nSlotIndex );
	void SendExpandSkillPageResult(int nRet);
#if defined(PRE_MOD_POTENTIAL_JEWEL_RENEWAL)
	void SendPotentialItemRollbackResult(int nRet);
#endif	// #if defined(PRE_MOD_POTENTIAL_JEWEL_RENEWAL)
#if defined (PRE_ADD_CHAOSCUBE)
	void SendChaosCubeRequest(char cInvenType, int nCount, TChaosItem ChaosItem[MAX_CHAOSCUBE_STUFF], short nRetCode);
	void SendChaosCubeComplete(char cInvenType, int nResultItemID, int nResultItemCount, int nResultPeriod, short nRetCode);
	void SendChaosGaugeCubeAddStuff(int nChaosGaugeType, int nCurrentGauge, int nRetCode);
	void SendChaosGaugeCubeComplete(int nChaosGaugeType, int nResultItemID, int nResultItemCount, int nResultPeriod, int nRetCode);
	void SendChaosGaugeCubeList(const std::map<int, int> &mapChaosGauge);
#endif

	// Npc Reputation
#if defined( PRE_ADD_NPC_REPUTATION_SYSTEM )
	void SendOpenGiveNpcPresent( UINT nNpcID );
	void SendRequestSendSelectedPresent( UINT nNpcID );
	void SendShowNpcEffect( UINT nNpcID, int nEffectIndex );	// Request client to play effect on specific NPC. 0: Angry, 1: Disappointed
#endif // #if defined( PRE_ADD_NPC_REPUTATION_SYSTEM )

	void SendOpenDarkLairRankBoard( UINT uiNpcID, int iMapIndex, int iPlayerCount );
	void SendDarkLairRankBoard( TAGetDarkLairRankBoard* pPacket );
	void SendOpenPvPLadderRankBoard( UINT uiNpcID, LadderSystem::MatchType::eCode MatchType );
	void SendPvPLadderRankBoard( TAGetListPvPLadderRanking* pPacket );	
#if defined(PRE_ADD_PVP_RANKING)
	void SendPvPRankBoard(TAGetPvPRankBoard* pPacket);
	void SendPvPRankList( TAGetPvPRankList* pPacket );
	void SendPvPRankInfo(TAGetPvPRankInfo* pPacket);

	void SendPvPLadderRankBoard(TAGetPvPLadderRankBoard* pPacket);
	void SendPvPLadderRankList( TAGetPvPLadderRankList* pPacket );
	void SendPvPLadderRankInfo(TAGetPvPLadderRankInfo* pPacket);
#endif

	// Npc
	void SendTalkEnd(UINT nNpcObjectID);
	void SendNextTalk(UINT nNpcObjectID, WCHAR* wszIndex, WCHAR* wszTarget, std::vector<TalkParam>& talkParam );
	void SendNextTalkError();
	void SendShowWarehouse( int iItemID=0 );
	void SendOpenQuestReward(int nTableIndex, bool bActivate);
	void SendOpenCompoundEmblem();
	void SendOpenUpdagrageJewel();
	void SendOpenMailBox();
	void SendOpenFarmWareHouse();
	void SendOpenDisjointItem();
	void SendOpenUpgradeItem( int iItemID/*=0*/ );
	void SendOpenCompoundItem(int nCompoundShopID);
	void SendOpenCompound2Item(int nCompoundGroupID, int iItemID );
	void SendOpenCashShop();
	void SendOpenGuildMgrBox(int nGuildMgrNo);
	void SendOpenMovieBrowser();
	void SendOpenBrowser(const char* szUrl, int nX, int nY, int width, int height);
	void SendOpenGlyphLift();
	void SendOpenInventory();
#ifdef PRE_ADD_OPEN_QUEST_TEXTURE_DIALOG
	void SendOpenTextureDialog(SCOpenTextureDialog sDialogData);
	void SendCloseTextureDialog(int nDlgID);
#endif

	// Quest
	void SendQuestInfo(TUserData &UserData);
	void SendQuestInfo(TQuestGroup* pQuestGroup);
	void SendQuestCompleteInfo(TQuestGroup &Quest);
	void SendQuestResult(int nRet);

	// Update quest info whose state changed to the client.
	void SendQuestRefresh(char cSlot, int nRefreshType, int nQuestID, TQuest& Quest);
	void SendCompleteQuest(bool bDeletePlayList, char cSlot, UINT nQuestID, bool bNeedMarking);
	void SendMarkingCompleteQuest(UINT nQuestID);
	void SendRemoveQuest(char cSlot, UINT nQuestID, short nRet);
	void SendPlayCutScene( int nCutSceneTableID, int nQuestIndex, int nQuestStep, UINT nNpcObjectID, bool bQuestAutoFadeIn, DWORD dwQuestPlayerUniqueID );
	void SendCompleteCutScene( bool bFadeIn );
	void SendSkipCutScene( UINT nSessionID );
	void SendSkipAllCutScene();
	void SendAssginPeriodQuest(UINT nQuestID, bool bFlag);
	void SendScorePeriodQuest(int nCurrentCount, int nMaxCount);
	void SendPeriodQuestNotice(int nItemID, int nNoticeCount);

#if defined(PRE_ADD_REMOTE_QUEST)
	void SendGainRemoteQuest( int nQuestID);
	void SendRemoveRemoteQuest( int nQuestID);
	void SendCompleteRemoteQuest( int nQuestID);
#endif

	// Chat and Message
	void SendChat(char cType, int cLen, const WCHAR *pwszCharacterName, const WCHAR *pwszChatMsg, TParamData* pParamData=NULL, int nRet = ERROR_NONE);
	void SendNotice(const WCHAR * pMsg, int nLen, int nSlideShowSec);
	void SendWorldSystemMsg(char cType, int nID, const WCHAR *pwszCharacterName, int nValue, const WCHAR* pwszToCharacterName=NULL);
	void SendDebugChat(const WCHAR* szMsg, eChatType eType=CHATTYPE_NORMAL );
	void SendNoticeCancel();
	void SendCloseService(__time64_t _tNow, __time64_t _tOderedTime, __time64_t _tCloseTime);
	void SendServerMsg( char cType, char cStringType, int nMsgIndex, char cCaptionType=-1, BYTE cFadeTime=0 );
	void SendTcpPing( DWORD dwTick );
	void SendClientPing( DWORD dwTick );

	// RadioMessage
	void SendUseRadio(UINT nSessionID, USHORT nRadioID);

	// Gesture
	void SendCashGestureList();
	void SendCashGestureAdd( USHORT nGestureID );
	void SendUseGesture( USHORT nGestureID );
	void SendEffectItemGestureList();
	void SendEffectItemGestureAdd( USHORT nGestureID );

	// Skill
	void SendAddSkill(int nSkillID, int nRet);
	void SendDelSkill(int nSkillID, int nRet);
	void SendSkillLevelUp(int nSkillID, BYTE cLevel, int nRet);
	void SendOtherPlayerSkillLevelUp( int nOtherPlayerIndex, int nSkillID, char cLevel );
	void SendPushSkillPoint( unsigned short usSkillPoint );		// Used only in setsp cheat.
	void SendUnlockSkill( int nSkillID, int nRetCode );
	void SendAcquireSkill( int nSkillID, int nRetCode );
	void SendCanUseSkillResetCashItem( INT64 biSerial, int nRewardSP, int nRetCode );
	void SendSkill(TSkill *SkillArray, char cSkillPage);
	void SendSkillReset( int nResultSkillPoint, char cSkillPage );	
	void SendUseSkillResetCashItemRes( int nRetCode, int nResultSP, char cSkillPage );
	void SendSkillPageCount(char cSkillPageCount);
	void SendChangeSkillPage(char cSkillPage);

	// Trade - Shop
	void SendShopOpen(int nShopID, Shop::Type::eCode Type=Shop::Type::Normal );
	void SendShopBuyResult(int nRet);
	void SendShopSellResult(int nRet);
	void SendShopRepurchaseList( int nRet, BYTE cCount=0, const TRepurchaseItemInfo* pList=NULL );
	void SendShopRepurchase( int nRet, int iRepurchaseID=-1 );

	// Trade - SkillShop
	void SendSkillShopOpen( void );

	// Trade - Mail
	void SendMailBox(const TMailBoxInfo *MailArray, short wTotalCount, short wUnreadCount, char cMailCount, char cDailyMailCount, int nRet);
	void SendMailSendResult(int nRet, int nDailyMailCount);
	void SendReadMailResult(const TAReadMail *pRead, int nAttachCoin);
#if defined(PRE_ADD_CADGE_CASH)
	void SendReadCadgeMailResult(const TAReadWishMail *pRead);
#endif	// #if defined(PRE_ADD_CADGE_CASH)
	void SendDeleteMailResult(int nRet);
	void SendAttachAllMailResult(int *nMailDBIDArray, int nRet);
	void SendAttachMailResult(int nRet);
	void SendNotifyMail(int nTotalMailCount, int nNotReadMailCount, int nExpirationCount, bool bNewMail);

	// Trade - Exchange (Personal Trade)
	void SendExchangeRequest(UINT nSenderSessionID, int nRet);
	void SendExchangeReject(UINT nSessionID, short nRetCode = ERROR_NONE);
	void SendExchangeStart(UINT nTargetSessionID, int nRet);
	void SendExchangeAddItem(UINT nSessionID, char cExchangeIndex, BYTE cInvenIndex, TItem &InvenItem, int nRet);
	void SendExchangeDeleteItem(UINT nSessionID, char cExchangeIndex, int nRet);
	void SendExchangeAddCoin(UINT nSessionID, INT64 nCoin, int nRet);
	void SendExchangeConfirm(UINT nSessionID, char cType);
	void SendExchangeCancel();
	void SendExchangeComplete(int nRet);

	// Trade - Market
	void SendMarketList(TMarketInfo *MarketArray, char cMarketCount, int nMarketTotalCount, int nRet);
	void SendMyMarketList(TMyMarketInfo *MarketArray, char cMarketCount, int nSellingCount, int nClosingCount, int nWeeklyRegisterCount, int nRegisterItemCount, int nRet, bool bPremiumTrade );
	void SendMarketRegister(int nMarketDBID, int nRet);
	void SendMarketInterrupt(int nMarketDBID, int nRegisterCount, int nRet);
	void SendMarketBuy(int nMarketDBID, int nRet, bool bMini);
	void SendMarketCalculationList(TMarketCalculationInfo *MarketArray, char cMarketCount, int nRet);
	void SendMarketCalculation(int nMarketDBID, int nRet);
	void SendMarketCalculationAll(int nRet);
	void SendNotifyMarket(int nItemID, short wCalculationCount);
	void SendMarketPetalBalance(int nPetalBalance, int nRet);
	void SendMarketPrice(int nRet, int nMarketDBID, TMarketPrice * pPrice);
	void SendMarketMiniList(TMarketInfo *MarketArray, char cMarketCount, int nMarketTotalCount, int nRet);

	// Trade - SpecialBox
#if defined(PRE_SPECIALBOX)
	void SendSpecialBoxList(int nRet, char cCount, TSpecialBoxInfo *InfoList);
	void SendSpecialBoxItemList(int nRet, char cCount, TSpecialBoxItemInfo *ItemList, INT64 biRewardCoin);
	void SendReceiveSpecialBoxItem(int nRet);
	void SendNotifySpecialBoxCount(int nNotifyCount, bool bNew);
#endif	// #if defined(PRE_SPECIALBOX)

	//Friend
	void SendFriendGroupList(SCFriendGroupList * pPacket, int nSize);
	void SendFriendList(SCFriendList * pPacket);
	void SendFriendLocationList(SCFriendLocationList * pPacket);

	void SendFriendGroupAdded(UINT nGroupDBID, const WCHAR * pName, int nRet);
	void SendFriendAdded(INT64 biFriendCharDBID, UINT nGroupDBID, const WCHAR * pName, const sWorldUserState * pState, int nRet);
	void SendFriendGroupDeleted(UINT nGroupDBID, int nRet);
	void SendFriendGroupUpdated(UINT nGroupDBID, const WCHAR * pName, int nRet);
	void SendFriendDeleted(BYTE cCount, const INT64 * biFriendCharacterDBID, int nRet);
	void SendFriendUpdated(BYTE cCount, INT64 * biFriendDBID, UINT nGroupDBID, int nRet);
	void SendFriendDetailInfo(INT64 biFriendDBID, UINT nBelongGroupDBID, int nClass, int nLevel, int nJob, const sWorldUserState * pState, \
		const WCHAR * pGuildName, const WCHAR * pMemo);
	void SendFriendResult(int nRet);
	void SendFriendAddNotice(const WCHAR * pName);

	//Guild
	void SendCreateGuild(UINT nSessionID, LPCWSTR lpwszGuildName, int iErrNo, const TGuildUID* pGuildUID = NULL, const TGuild* pInfo = NULL, UINT nAccountDBID = 0, INT64 nCharacterDBID = 0, TP_JOB nJob = 0, CHAR cLevel = 0, TCommunityLocation* pLocation = NULL);
	void SendDismissGuild(UINT nSessionID, int iErrNo, const TGuildUID* pGuildUID = NULL);
	void SendInviteGuildMemberReq(const TGuildUID pGuildUID, UINT nFromAccountDBID, UINT nFromSessionID, LPCWSTR lpwszFromCharacterName, int iErrNo, UINT nToAccountDBID = 0, UINT nToSessionID = 0, LPCWSTR lpwszGuildName = NULL);
	void SendInviteGuildMemberAck(UINT nToAccountDBID, INT64 nToCharacterDBID, LPCWSTR lpwszToCharacterName, int iErrNo, UINT nFromAccountDBID = 0, UINT nFromSessionID = 0, TP_JOB nJob = 0, CHAR cLevel = 0, TCommunityLocation* pLocation = NULL, const TGuildUID* pGuildUID = NULL);
	void SendLeaveGuildMember(UINT nAccountDBID, INT64 nCharacterDBID, int iErrNo, const TGuildUID* pGuildUID, bool bGraduateBeginnerGuild);
	void SendExileGuildMember(UINT nAccountDBID, INT64 nCharacterDBID, int iErrNo, const TGuildUID* pGuildUID);
	void SendChangeGuildInfo(UINT nAccountDBID, INT64 nCharacterDBID, BYTE btGuildUpdate, int iErrNo, int iInt1 = 0, int iInt2 = 0, INT64 biInt64 = 0, LPCWSTR pText = NULL, const TGuildUID *pGuildUID = NULL);
	void SendChangeGuildMemberInfo(UINT nReqAccountDBID, INT64 nReqCharacterDBID, UINT nChgAccountDBID, INT64 nChgCharacterDBID, BYTE btGuildMemberUpdate, int iErrNo, int iInt1 = 0, int iInt2 = 0, INT64 biInt64 = 0, LPCWSTR pText = NULL, const TGuildUID *pGuildUID = NULL);
	void SendGetGuildHistoryList(const TGuildUID pGuildUID, INT64 biIndex, bool bDirection, int nCurCount, int nTotCount, int iErrNo, const TGuildHistory* pHistoryList = NULL, int nCount = 0);
	void SendGuildSelfView(UINT nSessionID, const TGuildSelfView& pGuildSelfView);
	void SendGuildChat(const TGuildUID pGuildUID, UINT nAccountDBID, INT64 nCharacterDBID, LPCWSTR lpwszChatMsg, short nLen);
#ifdef PRE_ADD_DOORS_GUILDCHAT_DISCONNECT
	void SendDoorsGuildChat(const TGuildUID pGuildUID, INT64 biCharacterDBID, LPCWSTR lpwszChatMsg, short nLen);
#endif		//#ifdef PRE_ADD_DOORS_GUILDCHAT_DISCONNECT
	void SendGuildMemberLoginList(SCGuildMemberLoginList* pPacket);
	void SendGuildMemberLoginList(TGuildUID GuildUID, int nCount, INT64 *List);
	void SendChangeGuildInfoEx(UINT nAccountDBID, INT64 nCharacterDBID, BYTE btGuildUpdate, int iErrNo, int iInt1 = 0, int iInt2 = 0, int iInt3 = 0, int iInt4 = 0, INT64 biInt64 = 0, LPCWSTR pText = NULL, const TGuildUID *pGuildUID = NULL);
	void SendMoveGuildItem(char cType, INT64 biSrcSerial, INT64 biDestSerial, const TItemInfo *pSrcInfo, const TItemInfo *pDestInfo, int nRetCode, int nTakeItemCount=0);
	void SendMoveGuildCoin(char cType, INT64 nAddCoin, INT64 nInventoryCoin, INT64 nWarehouseCoin, INT64 nWithdrawCoin, int nRet);
	void SendRefreshGuildItem(char cType, INT64 biSrcSerial, INT64 biDestSerial, const TItemInfo *pSrcInfo, const TItemInfo *pDestInfo);
	void SendRefreshGuildCoin(INT64 biTotalCoin);
	void SendGetGuildWareHistoryList(const TGuildUID pGuildUID, bool bDirection, int nCurCount, int nTotCount, int iErrNo, const TGuildWareHistory* pHistoryList, int nCount);
	void SendGetGuildInfo(CDNGuildBase *pGuild, int nRet);
	void SendGetGuildMember(CDNGuildBase *pGuild, int nRet);
	void SendEnrollGuildWar(int iErrNo, short wScheduleID=0, BYTE cTeamColorCode=0);
	void SendGuildWarEvent(short wScheduleID, char cEventStep, char cEventType);
	void SendGuildWarFestivalPoint();
	void SendGuildWarPreWinGuild(bool bPreWin);
	void SendGuildWarTournamentWin(SCGuildWarTournamentWin* pPacket);
#if defined(_VILLAGESERVER)
	void SendGuildWarEventTime(TGuildWarEventInfo* pGuildWarTime, TGuildWarEventInfo* pFinalPartTime, bool bFinalProgress);
	void SendGuildWarStatusOpen(int nBluePoint, int nRedPoint, int nMyPoint, int nMyGuildPoint);
	void SendGuildWarTrialResultOpen();
	void SendGuildWarTournamentInfoOpen(bool bPopularityVote);
	void SendGuildWarVote(int nRetCode, int nAddFestivalPoint=0);	
	void SendGuildWarVoteTop(int nVoteCount, const WCHAR* pwszVoteGuildName);
	void SendGuildWarReward(int nRetCode);	
	void SendGuildWarWinSkill(int nRetCode, DWORD dwCoolTime);
	void SendGuildWarUserWinSkill(TGuildUID GuildUID, EtVector3 vPos, int nSkillID);
	void SendGuildWarTrialResultUIOpen();
	void SendGuildWarTournamentInfoUIOpen();
	void SendGuildWarBuyedItem(int itemID, int count);
	void SendGuildWarBuyedItemList(const std::map<int, int>& items);
#endif // #if defined(_VILLAGESERVER)

	void SendChangeGuildName(const wchar_t* pwszGuildName);
	void SendChangeGuildMark(const MAGuildChangeMark *pPacket);
	void SendExtendGuildWare(short wTotalSize);

	void SendGuildLevelUp(int nLevel);
	void SendUpdateGuildExp(int nError, BYTE cPointType, int nPointValue, int nGuildExp, INT64 biCharacterDBID, int nMissionID);

	void SendGuildRewardItem( TGuildRewardItem* GuildRewardItem );	
	void SendAddGuildRewardItem( TGuildRewardItem GuildRewardItem );
	void SendBuyGuildRewardItem( int iRet, int nItemID );	
	void SendExtendGuildSize(short nGuildSize);
	void SendPlayerGuildInfo( int nRet, UINT nSessionID, LPCWSTR lpwszGuildName, LPCWSTR lpwszMasterName, int nGuildLevel, int nMemberSize, int nGuildSize, TGuildRewardItem* GuildRewardItem );

	// PvP
	void	SendPvPVillageToLobby();
	void	SendPvPCreateRoom( const short nRetCode, const UINT uiPvPIndex=0, const CSPVP_CREATEROOM* pPacket=NULL );
	void	SendPvPModifyRoom( const short nRetCode, const CSPVP_MODIFYROOM* pPacket=NULL );
	void	SendPvPLeaveRoom( const short nRetCode, PvPCommon::LeaveType::eLeaveType Type, const UINT uiLeaveUserSessionID );
#if defined(_VILLAGESERVER)
	void	SendPvPRoomList( const short nRetCode );
	void	SendPvPRoomList( const short nRetCode, std::vector<CDNPvPRoom*>& vPvPRoom, const UINT uiMaxPage );
#if defined(PRE_ADD_PVP_VILLAGE_ACCESS)
	void	SendPvPRoomList( SCPVP_ROOMLIST* pPacket, int nSize);
#endif // #if defined(PRE_ADD_PVP_VILLAGE_ACCESS)
	void	SendPvPWaitUserList( const short nRetCode );
	void	SendPvPWaitUserList( const short nRetCode, std::vector<CDNUserSession*>& vUserList, const UINT uiMaxPage );
	void	SendPvPJoinRoom( const short nRetCode, CDNPvPRoom* pPvPRoom=NULL );
	void	SendPvPStart( const short nRetCode );
	void	SendPvPChangeTeam( const short nRetCode );
	void	SendPvPRoomInfo( MAVIPVP_ROOMINFO* pPacket );
#endif //#if defined(_VILLAGESERVER)
	void	SendPvPData( const TPvPGroup& PvP );

	void SendPvPMemberIndex(USHORT nTeam, BYTE cCount, const TSwapMemberIndex * pIndex, int nRetCode);
	void SendPvPGuildWarMemberGrade(UINT uiUserState, UINT nSessionID, int nRetCode);
	void SendPvPTryAcquirePoint(int nAreaID, int nRetCode);
	void SendPvPTryAcquireSkill(int nSkillID, int nLevel, int nRetCode);
	void SendPvPInitSkill(int nRetCode);
	void SendPvPUseSkill(UINT nSessionID, int nSkillID, int nRetCode);
	void SendPvPSwapSkillIndex(char cFrom, char cTo, int nRetCode);
	void SendPvPUserState(UINT nSessionID, UINT uiUserState);
#if defined(_GAMESERVER) ||(defined(_WINDOWS) && !defined(_LAUNCHER))
	void SendGuildWarConcentrateOrder(UINT nSessionID, EtVector3 vPosition);
#endif // #if defined(_GAMESERVER) ||(defined(_WINDOWS) && !defined(_LAUNCHER))

	void SendPvPAllKillGroupCaptain( UINT uiGroupCaptainSessionID );
	void SendPvPAllKillActiveActor( UINT uiActiveActorSessionID );

#if defined(PRE_ADD_PVP_TOURNAMENT)
	void SendPvPSwapTournamentIndex(int nRetCode, char cSourceIndex, char cDestIndex);
#endif // #if defined(PRE_ADD_PVP_TOURNAMENT)

#if defined(PRE_ADD_QUICK_PVP)
	void SendQuickPvPInvite(UINT nSenderSessionID);
	void SendQuickPvPResult(int nResult);
#endif
#if defined(PRE_ADD_PVP_VILLAGE_ACCESS)
	void SendPvPListOpenUI(int nResult, int nLimitLevel);
#endif
	// LadderSystem
	void	SendLadderEnterChannel( int iRet, LadderSystem::MatchType::eCode Type );
	void	SendLadderLeaveChannel( int iRet );
	void	SendLadderMatching( int iRet, bool bIsCancel );
	void	SendLadderNotifyLeaveUser( INT64 biCharDBID, const WCHAR* pwszCharName, LadderSystem::Reason::eCode Type );		// Packet to Notify existing same-team users when a user leaves a ladder room
	void	SendLadderNotifyJoinUser( INT64 biCharDBID, const WCHAR* pwszCharName, int iGradePoint, BYTE cJobID );
	void	SendLadderNotifyLeader( const WCHAR* pwszCharName );
	void	SendLadderNotifyRoomState( LadderSystem::RoomState::eCode State, LadderSystem::RoomStateReason::eCode Reason );
#if defined( _VILLAGESERVER )
	void	SendLadderPlayingRoomList( int iRet, const std::vector<LadderSystem::CRoom*>& vRoomList=std::vector<LadderSystem::CRoom*>(), UINT uiMaxPage=0 );
	void	SendLadderObserver( int iRet );
#endif // #if defined( _VILLAGESERVER )

	void SendOccupationModeState(int nState);
	void SendOccupationState(BYTE cCount, const PvPCommon::OccupationStateInfo * pInfo);
	void SendOccupationTeamState(int nTeam, int nResource, int nResourceSum);
	void SendOccupationSkillState(const PvPCommon::OccupationSkillSlotInfo * pInfo);

	void SendOccupationClimaxMode();

#if defined(PRE_ADD_REVENGE)
	void SendPvPSetRevengeTarget( UINT uiSessionID, UINT uiRevengeTargetSessionID, Revenge::TargetReason::eCode eReason );
#endif
	//voicechat
	void SendVoiceChatInfo(char *pIp, USHORT wControlPort, USHORT wAudioPort, UINT m_nAccountDBID);
	void SendTalkingUpdate(int nCount, TTalkingInfo * pInfo);
	void SendVoiceMemberInfoRefresh(int nCount, UINT * pMuteSessionList, TVoiceMemberInfo * pInfo);

	// ChannelInfo

	// Isolate
	void SendIsolateResult(int nRetCode);
	void SendIsolateList(SCIsolateList * pPacket, int nSize);
	void SendIsolateAdd(const WCHAR * pAddIsolateName);
	void SendIsolateDel(std::vector <std::wstring> * vList);
	void SendIsolateDel(const WCHAR * pDelIsolateName);

	// Trigger
	void SendTriggerCallAction( int nSectorIndex, int nTriggerIndex, int nRandomSeed );
	void SendTriggerChangeMyBGM( int nBGM, int nFadeDelta );
	void SendTriggerRadioImage( UINT nSessionID, int nFileID, DWORD nTime );
	void SendTriggerFileTableBGMOff();
	void SendTriggerForceEnableRide( const UINT nSessionID, const bool bForceEnableRide );

	// Mission
	void SendMissionList( int nScore, char *pGain, char *pAchieve, short *pLastUpdate );
	void SendMissionGain( int nArrayIndex );
	void SendMissionAchieve( int nArrayIndex );
	void SendDailyMissionList( char cType, TDailyMission *pMission );
	void SendDailyMissionCount( char cType, int nArrayIndex, int nCount );
	void SendDailyMissionAchieve( char cType, int nArrayIndex, bool bSecret = false );
	void SendDailyMissionAlarm( char cType );

	void SendMissionEventPopUp( int nArrayIndex );
	void SendMissionHelpAlarm( int nArrayIndex );

#if defined(PRE_ADD_ACTIVEMISSION)
	void SendActiveMissionGain( int nIndex );
	void SendActiveMissionAchieve( int nIndex );
#endif

	void SendTimeEventList( TTimeEventGroup *pEvent );
	void SendTimeEventAchieve( int nEventID, INT64 nRemainTime );
	void SendTimeEventExpire( int nEventID );

	// Appellation
	void SendAppellationList( char *pGain );
	void SendSelectAppellation( int nArrayIndex, int nCoverArrayIndex );
	void SendChangeAppellation( UINT nSessionID, int nArrayIndex, int nCoverArrayIndex );
	void SendAppellationGain( int nArrayIndex );
	void SendAppellationTime( BYTE nCount, TAppellationTime *AppellationTIme);
	void SendResetPcbangAppellation();

	// GameOption
	void SendGameOption(UINT nSessionID, const TGameOptions * pOption, TPARTYID PartyID = 0);
	void SendGameOptionRefreshNotifier();
	void SendSecondAuthValidate( int nRet, int nAuthCheckType, BYTE cFailCount );
	void SendSecondAuthInfo( bool bPW, bool bLock );
	void SendMaxLevelCharacterCount( int iCount );
	void SendProfile( TProfile & profile );
	void SendDisplayProfile( int nSessionID, TProfile & profile );

	// Durability
	void SendDecreaseDurabilityInventory( const int iValue );
	void SendDecreaseDurabilityInventory( const float fValue );

	//Restraint
	void SendRestrinatList(TRestraintData * pRestraint);
	void SendRestraint(TRestraint * pRestraint);
	void SendRestraintDel(UINT nRestraintKey);
	void SendRestraintRetCode(short nRetCode);

	// CashShop
	void SendCashShopBalanceInquiry(int nRet, int nPetal, int nCash);
	void SendCashShopOpen(UINT nCash, int nReserve, int nRet, WCHAR *pAccountName);
	void SendCashShopClose(int nRet);
	void SendCashShopBuy(UINT nCash, int nReserve, int nRet, char cType, char cCount, TCashShopInfo *CashShopArray, TPaymentItemInfo* pPaymentItem = NULL, INT64 nSeedPoint = 0);
	void SendCashShopPackageBuy(int nPackageSN, int nCash, int nReserve, int nRet, const TPaymentPackageItemInfo* pPaymentPackageItem = NULL, INT64 nSeedPoint = 0);
#if defined(PRE_ADD_CASH_REFUND)
	void SendCashShopMoveCashinven( INT64 biDBID, int nRet, int nReservePetal, INT64 nSeedPoint );
	void SendCashShopCashRefund( INT64 biDBID, int nRet, int nCashAmount =0);
#endif
	void SendCashShopCheckReceiver(BYTE cLevel, BYTE cJob, int nRet);
#if defined(PRE_ADD_CADGE_CASH)
	void SendCashShopCadge(int nRet);
#endif	// #if defined(PRE_ADD_CADGE_CASH)
	void SendCashShopGift(int nCash, int nReserve, int nRet, char cType, int nCount, TCashShopInfo *GiftList, INT64 nSeedPoint = 0);
	void SendCashShopPackageGift(int nPackageSN, int nCash, int nReserve, int nRet, INT64 nSeedPoint = 0);
	void SendCashShopGiftList(int nRet, int nCount, TGiftInfo *GiftInfoArray);
	void SendCashShopReceiveGift(INT64 nGiftDBID, int nRet);
	void SendCashShopReceiveGiftAll(std::vector<INT64> &VecGiftDBIDList, int nRet);
#if defined(PRE_ADD_GIFT_RETURN)
	void SendCashShopGiftReturn(INT64 nGiftDBID, int nRet);
#endif
	void SendCashshopNotifyGift(bool bNew, int nGiftCount);
	void SendCoupon(int nRet);
#if defined(PRE_ADD_VIP)
	void SendVIPBuy(UINT nCashAmount, int nReserveAmount, int nRet, int nVIPPoint, __time64_t tEndDate, bool bAutoPay, INT64 nSeedPoint);
	void SendVIPGift(UINT nCashAmount, int nReserveAmount, int nRet, INT64 nSeedPoint);
	void SendVIPInfo(int nVIPPoint, __time64_t tEndDate, bool bAutoPay, bool bVIP);
#endif	// #if defined(PRE_ADD_VIP)
	void SendSaleAbortList(BYTE cCount, int *nAbortList);
	void SendCashShopServerTimeForCharge(__time64_t tTime);
	void SendSecretCashShopNotify(int nHiddenShopID);
	void SendSecretCashShopOpen(int nRet, int nHiddenShopID);
#ifdef PRE_ADD_LIMITED_CASHITEM
	void SendQuantityLimitedItemList(std::vector<LimitedCashItem::TLimitedQuantityCashItem> &vList);
	void SendChangedQuantityItemList(std::vector<LimitedCashItem::TChangedLimitedQuantity> &vList);
#endif		//#ifdef PRE_ADD_LIMITED_CASHITEM

	//ServerControl
	void SendZeroPopulation(bool bZero);

	void SendChatRoomCreateChatRoom( int nRet, TChatRoomView & RoomView );
	void SendChatRoomEnterRoom( int nRet, int nLeaderSID, int * UserIDs, USHORT nUserCount );
	void SendChatRoomEnterUser( UINT nChatRoomID, BYTE cChatRoomType, int UserSessionID );
	void SendChatRoomChangeLeader( int nRet, int LeaderSessionID );
	void SendChatRoomChangeRoomOption( int nRet, TChatRoomView & RoomView );
	void SendChatRoomKickUser( int nRet, int UserSessionID );	
	void SendChatRoomLeaveUser( int UserSessionID, BYTE nLeaveReason );

	// Chat room info of the character entering at SC_ENTERUSER
	void SendChatRoomView( int SessionID, TChatRoomView & RoomView );

#if defined( PRE_ADD_NPC_REPUTATION_SYSTEM )
	void SendReputationList( std::vector<TNpcReputation>& vData );
	void SendModReputation( std::vector<TNpcReputation>& vData );
#endif // #if defined( PRE_ADD_NPC_REPUTATION_SYSTEM )
	void SendUnionPoint();
	void SendUnionMembershipList( CDNUserItem *pItem );
	void SendBuyUnionMembershipResult( TUnionMembership* pMembership );

	// Master-apprentice system
	void SendMasterSystemSimpleInfo( TMasterSystemSimpleInfo& SimpleInfo, bool bRefresh = false );
	void SendMasterSystemCountInfo( int iMasterCount, int iPupilCount, int iClassmateCount );
	void SendMasterSystemMasterList( int iRet, std::vector<TMasterInfo>& vMasterList );
	void SendMasterSystemMasterCharacterInfo( int iRet, TMasterCharacterInfo* pMasterCharacterInfo=NULL );
	void SendMasterSystemPupilList( int iRet, INT64 biCharacterDBID, std::vector<TPupilInfo>& vPupilList );
	void SendMasterSystemIntroductionOnOff( int iRet, bool bRegister, WCHAR* pwszText );
	void SendMasterSystemMasterApplication( int iRet );
	void SendMasterSystemJoin( int iRet, bool bIsDirectMenu, bool bIsAddPupil, bool bIsTransactor );
	void SendMasterSystemMasterAndClassmateInfo( int iRet, std::vector<TMasterAndClassmateInfo>& vList );
	void SendMasterSystemClassmateInfo( int iRet, TClassmateInfo* pClassmateInfo=NULL );
	void SendMasterSystemMyMasterInfo( int iRet, TMyMasterInfo* pMyMasterInfo=NULL );
	void SendMasterSystemLeave( int iRet, bool bIsDelPupil=false, int iPenaltyRespectPoint=0 );
	void SendMasterSystemGraduate( WCHAR* pwszCharName );
	void SendMasterSystemConnect( bool bIsConnect, WCHAR* pwszCharName );
	void SendMasterSystemRespectPoint( int iRespectPoint );
	void SendMasterSystemFavorPoint( INT64 biMasterCharacterDBID, INT64 biPupilCharacterDBID, int iFavorPoint );
	void SendMasterSystemInvitePupil( int iRet, WCHAR* pwszCharName );
	void SendMasterSystemInvitePupilConfirm( int iRet, WCHAR* pwszCharName );
	void SendMasterSystemJoinDirect( int iRet, WCHAR* pwszCharName );
	void SendMasterSystemJoinDirectConfirm( int iRet, WCHAR* pwszCharName );
	void SendMasterSystemRecallMaster( int iRet, WCHAR* pwszCharName, bool bConfirm );
	void SendMasterSystemJoinComfirm( BYTE cLevel, BYTE cJob, WCHAR* pwszCharName );

#if defined( PRE_ADD_SECONDARY_SKILL )
	// Secondary skill
	void SendCreateSecondarySkill( int iRet, int iSkillID );
	void SendDelSecondarySkill( int iRet, int iSkillID );
	void SendUpdateSecondarySkillExp( int iRet, int iSkillID, SecondarySkill::Grade::eType Grade, int iExp, int iLevel );
	void SendAddSecondarySkillRecipe( int iRet, CSecondarySkillRecipe* pRecipe );
	void SendDeleteSecondarySkillRecipe( int iRet, int iSkillID, int iItemID );
	void SendExtractSecondarySkillRecipe( int iRet, int iSkillID, int iItemID );
	void SendUpdateSecondarySkillRecipeExp( int iRet, int iSkillID, int iItemID, int iExp );
	void SendManufactureSecondarySkill( int iRet, bool bIsStart, int iSkillID, int iItemID );
	void SendCancleManufactureSecondarySkill( int iRet, int iSkillID );
#endif // #if defined( PRE_ADD_SECONDARY_SKILL )

#if defined( PRE_ADD_VIP_FARM )
	void SendFarmStart( int iRet );
	void SendEffectItemInfo( int iItemID, __time64_t tExpireDate );
#endif // #if defined( PRE_ADD_VIP_FARM )
	void SendFarmOpenList();
	void SendFarmInfo(TFarmItem * pFarmItems, BYTE cFarmCount, TFarmFieldPartial * pFieldPartial, BYTE cFieldCount, bool bRefreshGate);
	void SendFarmFieldList( const TFarmFieldPartial * pFieldPartial, BYTE cFieldCount );
	void SendFarmPlantSeed( int iRet, Farm::ActionType::eType ActionType, int iAreaIndex, int iItemID, int iAttachItemID );
	void SendFarmHarvest( int iRet, Farm::ActionType::eType ActionType, int iAreaIndex, const std::vector<int>& vResultItem=std::vector<int>() );
	void SendFarmAddWater( int iRet, Farm::ActionType::eType ActionType, int iAreaIndex );
	void SendFarmWareHouseList( int iRet, INT64 biLastUniqueID, const std::map<INT64,TFarmWareHouseItem>& mList );
	void SendFarmTakeWareHouseItem( int iRet, INT64 biUniqueID );
	void SendFarmFieldCountInfo( int iCount );
	void SendFarmWareHouseItemCount( int iCount );
	void SendFarmAddWaterAnotherUser( WCHAR* pwszCharName );

	//fishing
	void SendFishingReadyResult(int nRet);
	void SendFishingCastBaitResult(int nRet);
	void SendFishingStopFishingResult(int nRet);
#ifdef PRE_ADD_CASHFISHINGITEM
	void SendFishingPattern(int nPatternID, int nFishingAutoMaxTime);
#else		//#ifdef PRE_ADD_CASHFISHINGITEM
	void SendFishingPattern(int nPatternID);
#endif		//#ifdef PRE_ADD_CASHFISHINGITEM
	void SendFishingEnd();
	void SendFishingSync(int nReduceTimeTick, int nFishingGauge);	

	// Etc
	void SendStartDragonNest( eDragonNestType Type, int nRebirthCount );

#ifdef PRE_ADD_CHANGEJOB_CASHITEM
	void SendChangeJobCashItemRes( int nRet, USHORT wTotalSkillPoint, int nChangeFirstJobID, int nChangeSecondJobID );
#endif // #ifdef PRE_ADD_CHANGEJOB_CASHITEM

	void SendCommonVariableList( const std::map<CommonVariable::Type::eCode, CommonVariable::Data>& mList );
	void SendModCommonVariableData( const CommonVariable::Data& Data );

	void SendCheckLastDungeonInfo( const WCHAR* pwszPartyName );
	void SendConfirmLastDungeonInfo( int iRet );

	void SendAttendanceEventResult( bool bCheckAttendanceFirst );
	void SendIncreaseLife( char cType, int nIncreaseLife );

#if defined(PRE_ADD_REMOVE_PREFIX)
	void SendRemovePrefixItemResult( int nRet, int nSlotIndex );
#endif // PRE_ADD_REMOVE_PREFIX
	void SendSourceData(int nSessionID, const TSourceData& sourceData, bool bUsedInGameServer);
#if defined( PRE_ADD_NAMEDITEM_SYSTEM )
	void SendEffectSkillItemData(int nSessionID, std::vector<TEffectSkillData>& EffectSkillList, bool bUsedInGameServer);	// Send multiple.
	void SendEffectSkillItemData(int nSessionID, TEffectSkillData EffectSkillList, bool bUsedInGameServer);	// Send one
	void SendDelEffectSkillItemData(int nSessionID, int nItemID);	// Delete
	void SendShowEffect( int nItemID, int nSessionID, BYTE cEffectType);
	void SendNamedItemID(int nSessionID, int nItemID);
#endif
#if defined(PRE_ADD_TRANSFORM_POTION)
	void SendChangeTransform(int nSessionID, int nTranformID, int nExpireTIme);
#endif
	void SendPetSkillItemResult( int nRet, char cSlotNum, int nSkillID );
	void SendPetSkillExpandResult( int nRet );
	void SendPetSkillDelete( int nRet, char cSlotNum);

	void SendSpecialRebirthItem( int nItemID, int nRebirthCount );
	void SendPlayerCustomEventUI(SCGetPlayerCustomEventUI * pPacket);
	//Guild recruitment board
	void SendGuildRecruitList( int iRet, UINT uiPage, std::vector<TGuildRecruitInfo>& vGuildRecruitList );
	void SendGuildRecruitCharacter( int iRet, std::vector<TGuildRecruitCharacter>& vGuildRecruitCharacterList );
	void SendMyGuildRecruit( int iRet, std::vector<TGuildRecruitInfo>& vMyGuildRecruitList );
	void SendGuildRecruitRequestCount( int iRet, int nRequestCount, GuildRecruitSystem::Max::eType type );
#if defined( PRE_ADD_GUILD_EASYSYSTEM )
	void SendRegisterGuildRecruitInfo( BYTE* cClassCode, int nMinLevel, int nMaxLevel, WCHAR* wszGuildIntroduction, BYTE cPurposeCode, bool bCheckHomePage );
#else
	void SendRegisterGuildRecruitInfo( BYTE* cClassCode, int nMinLevel, int nMaxLevel, WCHAR* wszGuildIntroduction );
#endif
	void SendRegisterGuildRecruitList( int iRet, GuildRecruitSystem::RegisterType::eType type );
	void SendGuildRecruitRequest( int iRet, GuildRecruitSystem::RequestType::eType type );
	void SendGuildRecruitAccept( int iRet, INT64 biAcceptCharacterDBID, bool bDelGuildRecruit, GuildRecruitSystem::AcceptType::eType type );
	void SendGuildRecruitMemberResult( int iRet, WCHAR* wszGuildName, GuildRecruitSystem::AcceptType::eType type );

	void SendWarpVillageResult(int nResult);
	void SendWarpVillageList(const vector<WarpVillage::WarpVillageInfo>& vList);

#if defined (PRE_ADD_DONATION)
	void SendOpenDonation();
	void SendDonationResult(int nResult);
	void SendDonationRanking(const Donation::Ranking vRanking[], int nMyRanking, INT64 nMyCoin);
#endif // #if defined (PRE_ADD_DONATION)

#ifdef PRE_ADD_AUTOUNPACK
	void SendAutoUnpack(INT64 serial, int nRetCode);
#endif

#ifdef PRE_ADD_EXCHANGE_POTENTIAL
	void SendExchangePotoential(int nRetCode);
#endif		//#ifdef PRE_ADD_EXCHANGE_POTENTIAL
#if defined(PRE_ADD_EXCHANGE_ENCHANT)
	void SendOpenExchangeEnchant();
	void SendExchangeEnchant(int nRetCode);
#endif
#if defined(PRE_ADD_PCBANG_RENTAL_ITEM)
	void SendPcCafeRentItem(int nRetCode, int nNeedCount);
#endif

#if defined (PRE_ADD_BESTFRIEND)
	void SendGetBestFriend(int nRetCode, TBestFriendInfo& Info);
	void SendSearchBestFriendResult(int nRetCode, char cLevel, char cJob, LPCWSTR lpwszCharacterName);
	void SendRegistBestFriendReq(int nRetCode, UINT nFromAccountDBID, INT64 biFromCharacterDBID, LPCWSTR lpwszFromCharacterName, UINT nToAccountDBID, INT64 biToCharacterDBID, LPCWSTR lpwszToCharacterName);
	void SendRegistBestFriendAck(int nRetCode, UINT nFromAccountDBID, LPCWSTR lpwszFromCharacterName, UINT nToAccountDBID, LPCWSTR lpwszToCharacterName);
	void SendCompleteBestFriend(int nRetCode, LPCWSTR lpwszCharacterName);
	void SendEditBestFriendMemo(int nRetCode, bool bFromMe, LPCWSTR lpwszMemo);
	void SendCancelBestFriend(int nRetCode, LPCWSTR lpwszName, bool bCancel=false);
	void SendCloseBestFriend(int nRetCode, LPCWSTR lpwszName);
	void SendBestFriendItemRequest(INT64 biItemSerial, short nRetCode);
	void SendBestFriendItemComplete(int nResultItemID, int nResultItemCount, int nResultPeriod, short nRetCode);
#endif

#if defined( PRE_PRIVATECHAT_CHANNEL )
	void SendPrivateChatChannelResult(int nRetCode);
	void SendPrivateChatChannelJoinResult(int nRetCode, WCHAR* wszChannelName);
	void SendPrivateChatChannelInviteResult(int nRetCode);
	void SendPrivateChatChannelOutResult(int nRetCode);	
	void SendPrivateChatChannelInfo(TPrivateChatChannelInfo ChannelInfo, std::list<TPrivateChatChannelMember> &MemberList);
	void SendPrivateChatChannelMemberAdd( int nRet, TPrivateChatChannelMember Member);
	void SendPrivateChatChannelKickResult(int nRetCode, bool bMe);
	void SendPrivateChatChannelMemberDel( int nRet, INT64 biCharacterDBID, WCHAR* wszName);	
	void SendPrivateChatChannelMemberKick( int nRet, INT64 biCharacterDBID, WCHAR* wszName);	
	void SendPrivateChatChannelInfoMod( int nRet, int nModType, WCHAR* wszMasterName, INT64 biCharacterDBID );
#endif
#if defined( PRE_ADD_TOTAL_LEVEL_SKILL )
	void SendTotalLevel(UINT nSessionID, int nTotalLevelSkill);
	void SendTotalLevelSkillList(UINT nSessionID, int* nTotalLevelSKillData);
	void SendTotalLevelSkillCashSlot(UINT nSessionID, int nCashSlotIndex, bool bFlag, __time64_t tExpireDate);
	void SendAddTotalLevelSkill(UINT nSessionID, int nSlotIndex, int nSkillID, bool isInitialize = false);
	void SendDelTotalLevelSkill(UINT nSessionID, int nSlotIndex);
#endif
#ifdef PRE_ADD_PRESET_SKILLTREE
	void SendSkillPresetList(TSkillSetIndexData * pSetData, TSkillSetPartialData * pSkillData, int nRetCode);
	void SendSkillPresetAddResult(int nRetCode);
	void SendSkillPresetDelResult(int nRetCode);
#endif		//#ifdef PRE_ADD_PRESET_SKILLTREE
	void SendItemModItemExpireDate(int nRet);

#if defined( PRE_WORLDCOMBINE_PVP )
	void SendWorldPvPRoomStartMsg( bool bShow, UINT	uiPvPIndex );
	void SendWorldPvPRoomStartResult( int nRet );
	void SendWorldPvPRoomJoinResult( int nRet );
	void SendWorldPvPRoomAllKillTeamInfo( UINT* nSessionID, int* nTeam );
	void SendWorldPvPRoomTournamentUserInfo( BYTE cUserCount, PvPCommon::UserInfoList *TournamentUserInfo, int nSize );
#endif

#if defined( PRE_ADD_PVP_COMBOEXERCISE )
	void SendPvPComboExerciseRoomMasterInfo( UINT uiRoomMasterSessionID );
	void SendPvPComboExerciseChangeRoomMaster( UINT uiRoomMasterSessionID );
	void SendPvPComboExerciseRecallMonsterResult( int nRetCode );
#endif // #if defined( PRE_ADD_PVP_COMBOEXERCISE )

#if defined( PRE_ALTEIAWORLD_EXPLORE )
	void SendAlteiaWorldJoinInfo( BYTE cWeeklyClearCount, BYTE cDailyClearCount, BYTE cTicketCount );
	void SendAlteiaWorldJoinResult( int nRetCode );
	void SendAlteiaWorldSendTicket( const WCHAR* wszSendName, int nTicketCount = 0 );
	void SendAlteiaWorldSendTicketResult( int nRetCode, int nSendTicketCount );
	void SendAlteiaWorldDiceResult( int nRetCode, int nCount = 0 );
#if defined( _VILLAGESERVER )
	void SendAlteiaWorldPrivateGoldKeyRank( const TAGetAlteiaWorldPrivateGoldKeyRank *pPacket );
	void SendAlteiaWorldPrivatePlayTimeRank( const TAGetAlteiaWorldPrivatePlayTimeRank *pPacket );
	void SendAlteiaWorldGuildGoldKeyRank( const TAGetAlteiaWorldGuildGoldKeyRank *pPacket );
	void SendAlteiaWorldSendTicketInfo( INT64* biCharacterDBID, BYTE cTicketCount );
#endif // #if defined( _VILLAGESERVER )
#endif // #if defined( PRE_ALTEIAWORLD_EXPLORE )

#if defined(PRE_ADD_GAMEQUIT_REWARD)
	void SendGameQuitRewardType(GameQuitReward::RewardType::eType eRewardType);
	void SendGameQuitRewardResult(int nRet);
#endif	// #if defined(PRE_ADD_GAMEQUIT_REWARD)

#if defined( PRE_ADD_STAMPSYSTEM )
	void SendStampSystemInit(int nWeekDay, INT64 biTableStartTime, bool * pCompleteFlagData);
	void SendStampSystemAddComplete(int nChallengeIndex, int nWeekDay);
	void SendStampSystemClear();
	void SendStampSystemChangeWeekDay(int nWeekDay);
	void SendStampSystemChangeTable(INT64 biTableStartTime);
#endif // #if defined( PRE_ADD_STAMPSYSTEM )

#if defined( PRE_ADD_PVP_EXPUP_ITEM )
	void SendUsePvPExpupItem(int nRet, BYTE cLevel, UINT uiXP, int nIncrement); 
#endif // #if defined( PRE_ADD_PVP_EXPUP_ITEM )


#if defined( PRE_DRAGONBUFF )
	void SendWorldBuffMsg( WCHAR* wszCharacterName, int nItemID );
#endif
#if defined( PRE_ADD_GUILD_CONTRIBUTION )
	void SendGuildContributionPoint( int nWeeklyContributionPoint, int nTotalContributionPoint );
	void SendGuildWeeklyContributionRankList( const TAGetGuildContributionRanking *pPacket );
#endif	// #if defined( PRE_ADD_GUILD_CONTRIBUTION )

#if defined(PRE_ADD_EQUIPLOCK)
	void SendLockItems(CDNUserItem* pItem);
	void SendAddLockItem(int nRet, DBDNWorldDef::ItemLocation::eCode Code, char cSlotIndex, __time64_t tLockDate);
	void SendRequestUnlockItem(int nRet, DBDNWorldDef::ItemLocation::eCode Code, char cSlotIndex, __time64_t tUnLockDate, __time64_t tUnLockRequestDate);
#endif	// #if defined(PRE_ADD_EQUIPLOCK)

#if defined( PRE_ADD_DWC )
	void SendDWCCreateTeam(int nRetCode, UINT nTeamID, const WCHAR *wszTeamName);
	void SendDWCLeaveTeam(int nRetCode, UINT nTeamID, INT64 biLeaveCharDBID, const WCHAR *wszLeaveCharName);
	void SendDWCDismissTeam(int nRetCode, UINT nTeamID);
	void SendDWCInviteMemberReq(UINT nTeamID, UINT nFromSessionID, const WCHAR *wszTeamName, const WCHAR *wszFromCharName);
	void SendDWCInviteMemberAck(int nRetCode, const WCHAR *wszTargetCharName);
	void SendDWCGetTeamInfo(int nRetCode, const TDWCTeamInfo *pTeamInfo);
	void SendDWCGetTeamMember(int nRetCode, UINT nTeamID, const std::vector<TDWCTeamMemberInfo> &vecMembers);
	void SendDWCGetRankPage(int nRetCode, int nTotalCount, int nPageNum, const std::vector<TDWCRankInfo> &vecRanks);
	void SendDWCFindRank(int nRetCode, const TDWCRankInfo *pRankInfo);
	void SendDWCChannelInfo(const TDWCChannelInfo &info);
	void SendDWCScoreInfo(int nDWCPoint, int nTotalWin, int nTotalLose, int nTotalDraw);
	void SendDWCGradeInfo(int nRank, BYTE cGrade);
#endif // #if defined( PRE_ADD_DWC )
};
