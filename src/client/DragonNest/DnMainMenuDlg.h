#pragma once
#include "EtUIDialog.h"
#include "EtUIDialogGroup.h"
#include "DNUserData.h"
#include "DnQuestSummaryInfoDlg.h"
#include "DNPacket.h"
#include "DnCommunityDlg.h"

class CDnInvenTabDlg;
class CDnCharStatusDlg;
class CDnSkillTabDlg;
class CDnSkillTreeDlg;
class CDnSkillTreeContentDlg;
class CDnSkillTreeButtonDlg;
class CDnSkillTreePreSetDlg;
class CDnSkillTreeSetMessageDlg;
class CDnLifeSkillContent;
class CDnCommunityDlg;
class CDnPGStorageTabDlg;
class CDnStoreTabDlg;
class CDnSkillStoreTabDlg;
class CDnMailDlg;
class CDnSystemDlg;
class CDnMarketTabDlg;
class CDnQuestSummaryInfoDlg;
class CDnEmblemDlg;
class CDnWorldMapDlg;
class CDnZoneMapDlg;
class CDnJewelCompoundDlg;
class CDnNotifyArrowDialog;
class CDnItemDisjointDlg;
class CDnPlateMainDlg;
class CDnItemUpgradeDlg;
#ifdef PRE_ADD_EXCHANGE_ENCHANT
class CDnItemUpgradeExchangeDlg;
#endif
class CDnMissionDlg;
class CDnItemCompoundTabDlg;
class CDnCashShopDlg;
class CDnEventTodayDlg;
class CDnCostumeMixDlg;
class CDnChatRoomCreateDlg;

#if defined(PRE_ADD_INTEGERATE_QUEST_REPUT)
class CDnIntegrateQuestNReputationDlg;
#else
class CDnQuestTabDlg;
#ifdef PRE_ADD_NPC_REPUTATION_SYSTEM
//class CDnNpcReputationDlg;
class CDnNpcReputationTabDlg;
#endif // PRE_ADD_NPC_REPUTATION_SYSTEM
#endif // PRE_ADD_INTEGERATE_QUEST_REPUT

class CDnDarkLairRankBoardDlg;
#ifdef PRE_ADD_COOKING_SYSTEM
class CDnLifeSkillCookingDlg;
#endif // PRE_ADD_COOKING_SYSTEM
class CDnWorldMsgDlg;
class CDnLifeStorageDlg;
class CDnItemCompoundTab2Dlg;

class CDnCostumeDesignMixDlg;
#ifdef PRE_ADD_COSRANDMIX
class CDnCostumeRandomMixDlg;
#endif
class CDnGlobalEventDlg;

#if defined(PRE_ADD_INTEGERATE_EVENTUI)
class CDnIntegrateEventDlg;
#else
class CDnTimeEventDlg;
class CDnAttendanceEventDlg;
#endif // PRE_ADD_INTEGERATE_EVENTUI


#ifdef PRE_ADD_NOTIFY_ITEM_COMPOUND
class CDnItemCompoundNotifyDlg;
#endif

#ifdef PRE_MOD_MISSION_HELPER
struct MissionInfoStruct;
#endif

#ifdef PRE_SPECIALBOX
class CDnEventReceiverTabDlg;
#endif

#ifdef PRE_MOD_ITEM_COMPOUND_DIALOG_RENEWAL
class CDnItemCompoundRenewalTabDlg;
#endif 

#if defined( PRE_ALTEIAWORLD_EXPLORE )
class CDnAlteaTabDlg;
#endif	// #if defined( PRE_ALTEIAWORLD_EXPLORE )

class CDnMainMenuDlg : public CEtUIDialog, public CEtUICallback
{
public:
	enum
	{
		INVENTORY_DIALOG,
		SKILL_DIALOG,
		SKILL_TREE_CONTENT_DIALOG,
		COMMUNITY_DIALOG,
		NOTIFIER_DIALOG,

		MAIL_DIALOG,
		CHARSTATUS_DIALOG,
		PLAYER_GUILD_STORAGE_DIALOG,
		STORE_DIALOG,
		SKILL_STORE_DIALOG,
		MARKET_DIALOG,
		ITEM_DISJOINT_DIALOG,
		PLATE_MAIN_DIALOG,
		ITEM_UPGRADE_DIALOG,
		ITEM_COMPOUND_DIALOG,
		ITEM_COSTUMEMIX_DIALOG,
		SYSTEM_DIALOG,
		WORLDMAP_DIALOG,
		ZONEMAP_DIALOG,
		MISSION_DIALOG,
		QUEST_DIALOG,
		EVENT_DIALOG,
		CHATROOM_DIALOG,
		NPC_REPUTATION_TAB_DIALOG,
		GLOBALEVENT_DIALOG,
		DARKLAIR_RANK_BOARD,
#ifdef PRE_ADD_COOKING_SYSTEM
		COOKING_DIALOG,
#endif // PRE_ADD_COOKING_SYSTEM
		WORLDMSG_DIALOG,
		LIFESLOT_DIALOG,
		LIFESTORAGE_DIALOG,
		ITEM_COMPOUND2_DIALOG,
		TIMEEVENT_DIALOG,
		ITEM_COSTUME_DMIX_DIALOG,
		ATTENDANCEEVENT_DIALOG,
#ifdef PRE_ADD_COSRANDMIX
		ITEM_COSTUME_RANDOMMIX_DIALOG,
#endif

#ifdef PRE_ADD_DONATION
		CONTRIBUTION_DIALOG, // Donation.
#endif
#ifdef PRE_ADD_PVP_VILLAGE_ACCESS
		PVP_VILLAGE_ACCESS,
#endif

		INTEGRATE_EVENT_DIALOG,	//Merge events
		InTEGRATE_QUESTNREPUTATION_DIALOG,	//Quest/affinity integration
		ITEM_UPGRADE_EXCHANGE_DIALOG,
		ITEM_COMPOUND_NOTIFY_DIALOG,

#ifdef PRE_ADD_PVP_RANKING
		PVP_RANK_DIALOG, // PVP ranking.
#endif // PRE_ADD_PVP_RANKING
#ifdef PRE_SPECIALBOX
		EVENT_RECEIVER_DIALOG,
#endif
#if defined( PRE_ALTEIAWORLD_EXPLORE )
		ALTEA_TAB_DIALOG,
#endif	// #if defined( PRE_ALTEIAWORLD_EXPLORE )

#ifdef PRE_ADD_CASHSHOPBANNER_RENEWAL
		CASHSHOP_INGAMEBANNER_DIALOG,
#endif // PRE_ADD_CASHSHOPBANNER_RENEWAL
		MENUNEW_DIALOG,
	};

public:
	CDnMainMenuDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnMainMenuDlg(void);

protected:
	// Note : Left window
	//
	CDnCharStatusDlg *m_pCharStatus;
	CDnPGStorageTabDlg* m_pStorageDlg;
	CDnStoreTabDlg *m_pStoreDlg;
	CDnSkillStoreTabDlg *m_pSkillStoreDlg;
	CDnMarketTabDlg *m_pMarketDlg;
	CDnItemDisjointDlg *m_pItemDisjointDlg;
	CDnPlateMainDlg *m_pPlateMainDlg;
	CDnItemUpgradeDlg *m_pItemUpgradeDlg;
#ifdef PRE_ADD_EXCHANGE_ENCHANT
	CDnItemUpgradeExchangeDlg* m_pItemUpgradeExchangeDlg;
#endif
	CDnItemCompoundTabDlg *m_pItemCompoundDlg;
	CDnMailDlg *m_pMailDlg;
	CDnLifeStorageDlg * m_pLifeStorageDlg;
	CDnItemCompoundTab2Dlg *m_pItemCompound2Dlg;
#ifdef PRE_MOD_ITEM_COMPOUND_DIALOG_RENEWAL
	CDnItemCompoundRenewalTabDlg* m_pItemCompoundRenewalDlg;
#endif 

#ifdef PRE_ADD_DONATION
	class CDnContributionDlg * m_pContributionDlg;
#endif

	// Note : Right window
	//
	CDnInvenTabDlg *m_pInventory;
	CDnSkillTabDlg *m_pSkillDlg;
	CDnCommunityDlg *m_pCommunityDlg;
	CDnQuestSummaryInfoDlg *m_pQuestSumInfoDlg;

	// Note : Center
	CDnSystemDlg *m_pSystemDlg;
	CDnWorldMapDlg *m_pWorldMapDlg;
	CDnZoneMapDlg *m_pZoneMapDlg;
	CDnNotifyArrowDialog* m_pNotifyArrowDlg;
	CDnMissionDlg *m_pMissionDlg;
#if defined(PRE_ADD_INTEGERATE_QUEST_REPUT)
	CDnIntegrateQuestNReputationDlg* m_pIntegrateQuestNReputationDlg;
#else
	CDnQuestTabDlg *m_pQuestTabDlg;
#ifdef PRE_ADD_NPC_REPUTATION_SYSTEM
	//CDnNpcReputationDlg* m_pNpcReputationDlg;
	CDnNpcReputationTabDlg* m_pNpcReputationTabDlg;
#endif // #ifdef PRE_ADD_NPC_REPUTATION_SYSTEM
#endif // PRE_ADD_INTEGERATE_QUEST_REPUT

	CDnSkillTreeDlg* m_pSkillTreeDlg;
	CDnSkillTreeContentDlg* m_pSkillTreeContentDlg;
#if defined( PRE_ADD_SKILL_LEVELUP_RESERVATION )
	CDnSkillTreeButtonDlg * m_pSkillTreeButtonDlg;
#endif	// #if defined( PRE_ADD_SKILL_LEVELUP_RESERVATION )
#if defined( PRE_ADD_PRESET_SKILLTREE )
	CDnSkillTreePreSetDlg * m_pSkillTreePreSetDlg;
	CDnSkillTreeSetMessageDlg * m_pSkillTreeSetMessageDlg;
#endif	// #if defined( PRE_ADD_PRESET_SKILLTREE )

	CDnEventTodayDlg *m_pEventTodayDlg;
	CDnCostumeMixDlg* m_pCosMixDlg;
	CDnChatRoomCreateDlg *m_pChatRoomCreateDlg;
	CDnGlobalEventDlg *m_pGlobalEventDlg;
	CDnCostumeDesignMixDlg* m_pCosDMixDlg;

#ifdef PRE_ADD_COSRANDMIX
	CDnCostumeRandomMixDlg* m_pCosRandMixDlg;
#endif

#ifdef PRE_ADD_COOKING_SYSTEM
	CDnLifeSkillCookingDlg* m_pDnLifeSkillCookingDlg;
#endif // PRE_ADD_COOKING_SYSTEM
#if defined(PRE_ADD_INTEGERATE_EVENTUI)
	CDnIntegrateEventDlg* m_pIntegrateEventDlg;
#else
	CDnTimeEventDlg *m_pTimeEventDlg;
	CDnAttendanceEventDlg* m_pAttendanceEventDlg;
#endif // PRE_ADD_INTEGERATE_EVENTUI

#ifdef PRE_ADD_NOTIFY_ITEM_COMPOUND
	CDnItemCompoundNotifyDlg *m_pItemCompoundNotifyDlg;
#endif

#ifdef PRE_ADD_CASHSHOPBANNER_RENEWAL
	class CDnCashShopIngameBanner * m_pCashShopIngameBannerDlg;
#endif // PRE_ADD_CASHSHOPBANNER_RENEWAL

//#ifdef PRE_ADD_PVP_RANKING
//	class CDnPvPRankDlg * m_pPvPRankDlg; // PvP Ranking Dlg.
//#endif // PRE_ADD_PVP_RANKING

#ifdef PRE_SPECIALBOX
	CDnEventReceiverTabDlg *m_pEventReceiverTabDlg;
#endif

#if defined( PRE_ALTEIAWORLD_EXPLORE )
	CDnAlteaTabDlg * m_pAlteaTabDlg;
#endif	// #if defined( PRE_ALTEIAWORLD_EXPLORE )

	CEtUIDialogGroup m_LeftDlgGroup;
	CEtUIDialogGroup m_RightDlgGroup;
	CEtUIDialogGroup m_MiddleDlgGroup;

	//CDnButtonInfoDlg *m_pButtonInfoDlg;

	CEtUIButton *m_pButtonCtrl_01;
	CEtUIButton *m_pButtonCtrl_02;
	CEtUIButton *m_pButtonZ_01;
	CEtUIButton *m_pButtonZ_02;
	CEtUIButton *m_pButton_Cannon;

	bool m_bIgnoreShowFunc;

protected:
	//void ShowButtonInfoDialog( bool bShow, float fX = 0.0f, float fY = 0.0f, LPCWSTR szwInfo = L"", LPCWSTR szwKey = L"" );

public:
	// Note: Party window related functions
	//
	int GetPartyListStartIndex();
	void RefreshPartyList();
	void RefreshPartyInfoList();
	void RefreshPartyMemberList();
	void SwapPartyDialog();
	void SetPartyInfoLeader( int nLeaderIndex );
	void OnPartyJoin();
	void SetPartyCreateDifficulties(const BYTE* pDifficulties);
	void OnRecvPartyMemberInfo(ePartyType type);
	void OnPartyJoinFailed();

	void OpenStorageDialog( int iRemoteItemID, bool bGuildStorageOpened = false );
#ifdef PRE_ADD_PVPRANK_INFORM
	void OpenStoreDialog(int nType = Shop::Type::Normal, int nShopID = -1);
#else
	void OpenStoreDialog(int nType = Shop::Type::Normal);
#endif
	void OpenSkillStoreDialog();
	void OpenInvenDialog();
	void CloseInvenDialog();
	void OpenCharStatusDialog();
	void OpenMarketDialog();
	void OpenItemDisjointDialog();
	void OpenPlateMainDialog();
	void OpenItemUpgradeDialog( int nRemoteItemID = 0 );
#ifdef PRE_ADD_EXCHANGE_ENCHANT
	void OpenItemUpgradeExchangeDialog();
#endif
	void OpenItemCompoundDialog( int nCompoundShopID );
	void OpenItemCompound2Dialog( int nCompoundShopID, int nRemoteItemID = 0 );
	void OpenMailDialog(bool bWriteDialog, const wchar_t *wszName, const wchar_t* wszTitle);
	void ShowPrivateMarketDialog( bool bShow, bool bCancel = false );
	void OpenEventToday();
	void CloseEventToday();
	void ShowCostumeMixDialog(bool bShow, bool bCloseInvenTab = true);
    void OpenGlobalEventDlg(SCScorePeriodQuest* pData);
	void ShowCostumMergeDialog(bool bShow, bool bCloseInvenTab);
#ifdef PRE_ADD_COSRANDMIX
	void ShowCostumeRandomMixDialog(bool bShow, bool bCloseInvenTab);
	void OnShowMixDialog(int nDlgId, bool bShow, bool bCloseInvenTab);
#endif
#ifdef PRE_ADD_COOKING_SYSTEM
	void OpenCookingDialog( bool bClearSelectedRecipe );
	void CloseCookingDialog();
#endif // PRE_ADD_COOKING_SYSTEM
	void ShowLifeStorageDialog( bool bShow );
	void OpenGlyphLiftDialog();

	void ShowWorldMapDialog( bool bShow );
	void ShowZoneMapDialog( bool bShow, int nMapIndex );
	void RefreshZoneMapNPCActor();	
	void RefreshZoneMapPartyActor();
	void RefreshZoneMapFarmPlant();

	void ShowQuestDialog( bool bShow, int nChapterIndex = -1, int nQuestIndex = -1, bool bMainQuest = true);
#ifdef PRE_MOD_MISSION_HELPER
	void ShowMissionDialog( MissionInfoStruct *pStruct );
#else
	void ShowMissionDialog( bool bDaily, int nMissionItemID = -1 );
#endif
	void ShowChatRoomCreateDlg( bool bShow );
	void SetChatRoomFlag( bool bShowByHotKey );
#ifdef PRE_ADD_NPC_REPUTATION_SYSTEM
	void ShowNpcReputationTabDlg( bool bShow );
	void UpdateNpcReputationDlg( void );
#endif // #ifdef PRE_ADD_NPC_REPUTATION_SYSTEM
	// Note: Quest
	//
	bool IsOpenQuestDlg();

	void SetQuestSumInfoDlg();
	void RefreshQuestSumInfo();
	void ClearAllSumInfo();

	void OpenNotifyArrowDialog( EtVector3 vTarget, LOCAL_TIME nTime, bool bImmediately );
	void CloseNotifyArrowDialog();
	void ShowTimeEventDialog( bool bShow );
	void ShowAttendanceEventDialog( bool bShow );
	void EnalbeQuestSumPriority(bool bTrue);

#ifdef PRE_ADD_DONATION
	void OpenContributionDialog( bool Show );
	bool IsOpenDonation();
#endif

	// Note : Master/Apprentice system
	//
	void RefreshMasterListDialog();
	void RefreshMasterSimpleInfo();
	void RefreshMasterCharacterInfo();
	void RefreshMasterClassmate();
	void RefreshPupilList();
	void RefreshMyMasterInfo();
	void RefreshClassmateInfo();
	void RefreshPupilLeave();
	void RefreshRespectPoint();

	void RefreshLifeStorage(SCFarmWareHouseList * pData);
	void RequestTakeWareHouseItem( int nSlotIndex );
	void OnRecvTakeWareHouseItem(INT64 biUniqueID);

	// Note : System menu
	//
	void ShowSystemDialog( bool bShow );
	void CloseSystemDialogCompletely();
	void AddChannelList( sChannelInfo *pChannelInfo );

	bool IsOpenDialog();
	bool IsOpenDialog( DWORD dwDialogID );
	void CloseMenuDialog();

	bool IsWithDrawGlyphOpen();

	// Check if only the normal tab of inventory should be open
	bool IsForceOpenNormalInvenTabDialog();

	CEtUIDialog *GetMainDialog( DWORD dwDialogID );

	// Prevent closing when blind opens. Works once per call.
	void SetIgnoreShowFunc( bool bIgnore ) { m_bIgnoreShowFunc = bIgnore; }

	void ToggleButtonCtrl( int nMode );	// Note : 1 is Crosshair, 2 is Cursor
	void ToggleButtonZ( int nMode );	// Note : 1 is Attack mode, 2 is Peace mode, 3 is Cannon mode

	void ToggleShowDialog( DWORD dwDialogID );

	void OnLevelUp( int nLevel );
	void OnDie();

	// Event popup
#ifdef PRE_ADD_START_POPUP_QUEUE
	void OnEventPopUp(const char* szUrlAddress);
#else
	void OnEventPopUp( char* szUrlAddress);
#endif

	CDnSystemDlg *GetSystemDlg() { return m_pSystemDlg; }


	bool IsEquipped( class CDnItem * pItem, bool bCash ); // Return whether it is equipped.

#ifdef PRE_ADD_COMBINEDSHOP_PERIOD
	bool IsOpenStore(); // Shop Dlg.
#endif // PRE_ADD_COMBINEDSHOP_PERIOD
#if defined(PRE_ADD_SERVER_WAREHOUSE)
	bool IsOpenStorage(); // Storage Dlg.
#endif //

#if defined(PRE_ADD_INTEGERATE_EVENTUI)
	void ShowIntegrateEventDialog(bool bShow);
#endif // PRE_ADD_INTEGERATE_EVENTUI
#ifdef PRE_ADD_NOTIFY_ITEM_COMPOUND
	void ShowItemCompoundNotifyDlg( bool bShow );
	void SetItemCompoundNotify( int nItemIndex , int nOptionIndex );
	void ResetItemCompoundNotify();
	void RefreshCompoundNotifyPercentage();
#endif

#ifdef PRE_ADD_EQUIPED_ITEM_ENCHANT
	void SetEquippedItemUpgradeDlg(); 
	CDnItemUpgradeDlg* GetItemUpgradeDlg()		{ return m_pItemUpgradeDlg; }
#endif 

//#ifdef PRE_ADD_PVP_RANKING
//	void OpenPvPRankDlg();
//#endif // PRE_ADD_PVP_RANKING


#ifdef PRE_ADD_NEWCOMEBACK
	void SetComebackAppellation( UINT sessionID, bool bComeback );
#endif // PRE_ADD_NEWCOMEBACK

#ifdef PRE_ADD_STAMPSYSTEM
	class CDnStampEventDlg * GetStampEventDlg();
	void ShowDailyQuestTab();
#endif // PRE_ADD_STAMPSYSTEM

#ifdef PRE_ADD_CASHSHOPBANNER_RENEWAL
	void ShowIngameBanner( bool bShow );
#endif // PRE_ADD_CASHSHOPBANNER_RENEWAL

public:
	virtual void InitialUpdate();
	virtual void Initialize( bool bShow );
	virtual void OnUICallbackProc( int nID, UINT nCommand, CEtUIControl *pControl, UINT uiMsg = 0 );
	virtual void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg = 0 );
	virtual void Render( float fElapsedTime );
	virtual void Show( bool bShow );

};