#include "StdAfx.h"
#include "DnMainMenuDlg.h"
#include "DnInvenTabDlg.h"
#include "DnCharStatusDlg.h"
#include "DnSkillTabDlg.h"
#include "DnSkillTreeDlg.h"
#include "DnSkillTreeContentDlg.h"
#include "DnLifeSkillContent.h"
#include "DnQuestTabDlg.h"
#include "DnPGStorageTabDlg.h"
#include "DnStoreTabDlg.h"
#include "DnSkillStoreTabDlg.h"
#include "DnInterfaceDlgID.h"
#include "DnLocalPlayerActor.h"
#include "DnSystemDlg.h"
#include "DnMarketTabDlg.h"
#include "DnWorldMapDlg.h"
#include "DnZoneMapDlg.h"
#include "DnMailDlg.h"
#include "DnTradeMail.h"
#include "DnMainDlg.h"
#include "DnInterface.h"
#include "DnActor.h"
#include "DnPartyTask.h"
#include "DnTradeTask.h"
#include "DnWorld.h"
#include "GameOption.h"
#include "DnNotifyArrowDlg.h"
#include "DnItemDisjointDlg.h"
#include "DnPlateMainDlg.h"
#include "DnItemUpgradeDlg.h"
#ifdef PRE_ADD_EXCHANGE_ENCHANT
#include "DnItemUpgradeExchangeDlg.h"
#endif
#include "DnMissionDlg.h"
#include "DnItemCompoundTabDlg.h"
#include "TaskManager.h"
#include "DnGameTask.h"
#include "DnCashShopDlg.h"
#include "DnMissionDlg.h"
#include "DnMissionDailyQuestDlg.h"
#include "DnEventTodayDlg.h"
#include "DnCostumeMixDlg.h"
#include "DnCostumeDesignMixDlg.h"
#ifdef PRE_ADD_COSRANDMIX
#include "DnCostumeRandomMixDlg.h"
#endif
#include "DnChatRoomCreateDlg.h"
#ifdef PRE_ADD_NPC_REPUTATION_SYSTEM
//#include "DnNpcReputationDlg.h"
#include "DnNpcReputationTabDlg.h"
#endif // #ifdef PRE_ADD_NPC_REPUTATION_SYSTEM
#ifdef PRE_ADD_COOKING_SYSTEM
#include "DnLifeSkillCookingDlg.h"
#endif // PRE_ADD_COOKING_SYSTEM
#include "DnGlobalEventDlg.h"
#include "DnWorldMsgDlg.h"
#include "DnLifeStorageDlg.h"
#include "DnItemCompoundTab2Dlg.h"
#include "DnTimeEventTask.h"
#include "DnTimeEventDlg.h"
#include "DnAttendanceEventDlg.h"

#include "DnIntegrateEventDlg.h"
#include "DnIntegrateQuestNReputationDlg.h"

#include "DnSkillTreeButtonDlg.h"
#include "DnSkillTreePreSetDlg.h"
#include "DnSkillTreeSetMessageDlg.h"

#ifdef PRE_ADD_NOTIFY_ITEM_COMPOUND
#include "DnItemCompoundNotifyDlg.h"
#endif

#ifdef PRE_ADD_DONATION
#include "DnContributionDlg.h"
#endif

//#ifdef PRE_ADD_PVP_RANKING
//#include "DnPvPRankDlg.h"
//#endif // PRE_ADD_PVP_RANKING

#ifdef PRE_SPECIALBOX
#include "DnEventReceiverTabDlg.h"
#endif

#ifdef PRE_MOD_POTENTIAL_JEWEL_RENEWAL
#include "DnPotentialJewelDlg.h"
#endif

#ifdef PRE_MOD_ITEM_COMPOUND_DIALOG_RENEWAL
#include "DnItemCompoundRenewalTab.h"
#endif 

#ifdef PRE_ADD_STAMPSYSTEM
#include "DnStampEventDlg.h"
#endif // PRE_ADD_STAMPSYSTEM

#if defined( PRE_ALTEIAWORLD_EXPLORE )
#include "DnAlteaTabDlg.h"
#endif	// #if defined( PRE_ALTEIAWORLD_EXPLORE )

#ifdef PRE_ADD_CASHSHOP_RENEWAL
#include "DnCashShopIngameBanner.h"
#endif // PRE_ADD_CASHSHOP_RENEWAL

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnMainMenuDlg::CDnMainMenuDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
: CEtUIDialog( dialogType, pParentDialog, nID, pCallback )
{
	m_pInventory = NULL;
	m_pCharStatus = NULL;
	m_pSkillDlg = NULL;
	m_pSkillTreeDlg = NULL;
	m_pSkillTreeContentDlg = NULL;
#if defined( PRE_ADD_SKILL_LEVELUP_RESERVATION )
	m_pSkillTreeButtonDlg = NULL;
#endif	// #if defined( PRE_ADD_SKILL_LEVELUP_RESERVATION )
#if defined( PRE_ADD_PRESET_SKILLTREE )
	m_pSkillTreePreSetDlg = NULL;
	m_pSkillTreeSetMessageDlg = NULL;
#endif	// #if defined( PRE_ADD_PRESET_SKILLTREE )
	m_pCommunityDlg = NULL;
	m_pStorageDlg = NULL;
	m_pStoreDlg = NULL;
	m_pItemDisjointDlg = NULL;
	m_pPlateMainDlg = NULL;
	m_pItemUpgradeDlg = NULL;
#ifdef PRE_ADD_EXCHANGE_ENCHANT
	m_pItemUpgradeExchangeDlg = NULL;
#endif
	m_pSkillStoreDlg = NULL;
	m_pMailDlg = NULL;
	m_pButtonCtrl_01 = NULL;
	m_pButtonCtrl_02 = NULL;
	m_pButtonZ_01 = NULL;
	m_pButtonZ_02 = NULL;
	m_pSystemDlg = NULL;
	m_pMarketDlg = NULL;
	m_pQuestSumInfoDlg = NULL;
	m_pNotifyArrowDlg = NULL;
	m_pWorldMapDlg = NULL;
	m_pZoneMapDlg = NULL;
	m_pMissionDlg = NULL;
	m_pItemCompoundDlg = NULL;
	m_pItemCompound2Dlg = NULL;
	m_pEventTodayDlg = NULL;
	m_pCosMixDlg = NULL;
	m_pChatRoomCreateDlg = NULL;
	m_pGlobalEventDlg = NULL;
#ifdef PRE_ADD_COOKING_SYSTEM
	m_pDnLifeSkillCookingDlg = NULL;
#endif // PRE_ADD_COOKING_SYSTEM
	m_pButton_Cannon = NULL;
	m_pCosDMixDlg = NULL;
#ifdef PRE_ADD_COSRANDMIX
	m_pCosRandMixDlg = NULL;
#endif
	m_pLifeStorageDlg = NULL;
#if defined(PRE_ADD_INTEGERATE_EVENTUI)
	m_pIntegrateEventDlg = NULL;
#else
	m_pTimeEventDlg = NULL;
	m_pAttendanceEventDlg = NULL;
#endif // PRE_ADD_INTEGERATE_EVENTUI
#ifdef PRE_ADD_DONATION
	m_pContributionDlg = NULL;
#endif	//PRE_ADD_DONATION
#if defined(PRE_ADD_INTEGERATE_QUEST_REPUT)
	m_pIntegrateQuestNReputationDlg = NULL;
#else
	m_pQuestTabDlg = NULL;
#ifdef PRE_ADD_NPC_REPUTATION_SYSTEM
	m_pNpcReputationTabDlg = NULL;
#endif // #ifdef PRE_ADD_NPC_REPUTATION_SYSTEM
#endif // PRE_ADD_INTEGERATE_QUEST_REPUT
#ifdef PRE_ADD_NOTIFY_ITEM_COMPOUND
	m_pItemCompoundNotifyDlg = NULL;
#endif
//#ifdef PRE_ADD_PVP_RANKING
//	m_pPvPRankDlg = NULL;	
//#endif // PRE_ADD_PVP_RANKING
#ifdef PRE_SPECIALBOX
	m_pEventReceiverTabDlg = NULL;
#endif
	m_bIgnoreShowFunc = false;
#ifdef PRE_MOD_ITEM_COMPOUND_DIALOG_RENEWAL
	m_pItemCompoundRenewalDlg = NULL;
#endif 
#if defined( PRE_ALTEIAWORLD_EXPLORE )
	m_pAlteaTabDlg = NULL;
#endif	// #if defined( PRE_ALTEIAWORLD_EXPLORE )

#ifdef PRE_ADD_CASHSHOPBANNER_RENEWAL
	m_pCashShopIngameBannerDlg = NULL;
#endif // PRE_ADD_CASHSHOPBANNER_RENEWAL

}

CDnMainMenuDlg::~CDnMainMenuDlg(void)
{
	SAFE_DELETE( m_pNotifyArrowDlg );
	SAFE_DELETE( m_pSkillTreeContentDlg );
#if defined( PRE_ADD_SKILL_LEVELUP_RESERVATION )
	SAFE_DELETE( m_pSkillTreeButtonDlg );
#endif	// #if defined( PRE_ADD_SKILL_LEVELUP_RESERVATION )
#if defined( PRE_ADD_PRESET_SKILLTREE )
	SAFE_DELETE( m_pSkillTreePreSetDlg );
	SAFE_DELETE( m_pSkillTreeSetMessageDlg );
#endif	// #if defined( PRE_ADD_PRESET_SKILLTREE )

	if( CDnQuestTask::IsActive() )
		GetQuestTask().SetQuestDialog( NULL );
}

void CDnMainMenuDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "MainMenuDlg.ui" ).c_str(), bShow );
}

void CDnMainMenuDlg::InitialUpdate()
{
	m_pEventTodayDlg = new CDnEventTodayDlg( UI_TYPE_TOP, this );
	m_pEventTodayDlg->Initialize( false );
	m_MiddleDlgGroup.AddDialog( EVENT_DIALOG, m_pEventTodayDlg );

	m_pSystemDlg = new CDnSystemDlg( UI_TYPE_CHILD, this, SYSTEM_DIALOG, this );
	m_pSystemDlg->Initialize( false );
	m_MiddleDlgGroup.AddDialog( SYSTEM_DIALOG, m_pSystemDlg );

	m_pWorldMapDlg = new CDnWorldMapDlg( UI_TYPE_CHILD, this, WORLDMAP_DIALOG, this );
	m_pWorldMapDlg->Initialize( false );
	m_MiddleDlgGroup.AddDialog( WORLDMAP_DIALOG, m_pWorldMapDlg );

	m_pZoneMapDlg = new CDnZoneMapDlg( UI_TYPE_CHILD, this, ZONEMAP_DIALOG, this );
	m_pZoneMapDlg->Initialize( false );
	m_MiddleDlgGroup.AddDialog( ZONEMAP_DIALOG, m_pZoneMapDlg );

	m_pNotifyArrowDlg = new CDnNotifyArrowDialog( UI_TYPE_CHILD, this );
	m_pNotifyArrowDlg->Initialize( false );

	m_pMissionDlg = new CDnMissionDlg( UI_TYPE_CHILD, this, MISSION_DIALOG, this );
	m_pMissionDlg->Initialize( false );
	m_MiddleDlgGroup.AddDialog( MISSION_DIALOG, m_pMissionDlg );

#if defined(PRE_ADD_INTEGERATE_QUEST_REPUT)
	m_pIntegrateQuestNReputationDlg = new CDnIntegrateQuestNReputationDlg(UI_TYPE_CHILD, this, InTEGRATE_QUESTNREPUTATION_DIALOG, this);
	m_pIntegrateQuestNReputationDlg->Initialize(false);
	m_MiddleDlgGroup.AddDialog( InTEGRATE_QUESTNREPUTATION_DIALOG, m_pIntegrateQuestNReputationDlg );
#else
	m_pQuestTabDlg = new CDnQuestTabDlg( UI_TYPE_CHILD, this, QUEST_DIALOG, this );
	m_pQuestTabDlg->Initialize( false );
	m_MiddleDlgGroup.AddDialog( QUEST_DIALOG, m_pQuestTabDlg );

#ifdef PRE_ADD_NPC_REPUTATION_SYSTEM
	//m_pNpcReputationDlg = new CDnNpcReputationDlg( UI_TYPE_CHILD, this, NPC_REPUTATION_DIALOG, this );
	//m_pNpcReputationDlg->Initialize( false );
	//m_MiddleDlgGroup.AddDialog( NPC_REPUTATION_DIALOG, m_pNpcReputationDlg );
	m_pNpcReputationTabDlg = new CDnNpcReputationTabDlg( UI_TYPE_CHILD, this, NPC_REPUTATION_TAB_DIALOG, this );
	m_pNpcReputationTabDlg->Initialize( false );
	m_MiddleDlgGroup.AddDialog( NPC_REPUTATION_TAB_DIALOG, m_pNpcReputationTabDlg );
#endif // #ifdef PRE_ADD_NPC_REPUTATION_SYSTEM
#endif // PRE_ADD_INTEGERATE_QUEST_REPUT

	m_pSkillTreeDlg = new CDnSkillTreeDlg( UI_TYPE_CHILD, this, SKILL_DIALOG, this );
	m_pSkillTreeDlg->Initialize( false );
	m_pSkillTreeContentDlg = new CDnSkillTreeContentDlg( UI_TYPE_CHILD, this, SKILL_TREE_CONTENT_DIALOG, this );
	m_pSkillTreeContentDlg->Initialize( false );
	m_pSkillTreeDlg->SetContentPanelDlg( m_pSkillTreeContentDlg );
#if defined( PRE_ADD_SKILL_LEVELUP_RESERVATION )
	m_pSkillTreeButtonDlg = new CDnSkillTreeButtonDlg( UI_TYPE_CHILD, this );
	m_pSkillTreeButtonDlg->Initialize( false );
	m_pSkillTreeDlg->SetSkillButtonDlg( m_pSkillTreeButtonDlg );
#endif	// #if defined( PRE_ADD_SKILL_LEVELUP_RESERVATION )
#if defined( PRE_ADD_PRESET_SKILLTREE )
	m_pSkillTreePreSetDlg = new CDnSkillTreePreSetDlg( UI_TYPE_CHILD, this );
	m_pSkillTreePreSetDlg->Initialize( false );
	m_pSkillTreeDlg->SetSkillTreePreSetDlg( m_pSkillTreePreSetDlg );

	m_pSkillTreeSetMessageDlg = new CDnSkillTreeSetMessageDlg( UI_TYPE_MODAL );
	m_pSkillTreeSetMessageDlg->Initialize( false );
	m_pSkillTreeDlg->SetSkillTreeSetMessageDlg( m_pSkillTreeSetMessageDlg );
#endif	// #if defined( PRE_ADD_PRESET_SKILLTREE )
	m_MiddleDlgGroup.AddDialog( SKILL_DIALOG, m_pSkillTreeDlg );

	m_pChatRoomCreateDlg = new CDnChatRoomCreateDlg( UI_TYPE_CHILD, this, CHATROOM_DIALOG, this );
	m_pChatRoomCreateDlg->Initialize( false );
	m_MiddleDlgGroup.AddDialog( CHATROOM_DIALOG, m_pChatRoomCreateDlg );

	m_pGlobalEventDlg = new CDnGlobalEventDlg( UI_TYPE_CHILD, this, GLOBALEVENT_DIALOG, this );
	m_pGlobalEventDlg->Initialize( false );
	m_MiddleDlgGroup.AddDialog( GLOBALEVENT_DIALOG, m_pGlobalEventDlg );


#ifdef PRE_ADD_COOKING_SYSTEM
	m_pDnLifeSkillCookingDlg = new CDnLifeSkillCookingDlg( UI_TYPE_CHILD, this, COOKING_DIALOG, this );
	m_pDnLifeSkillCookingDlg->Initialize( false );
	m_MiddleDlgGroup.AddDialog( COOKING_DIALOG, m_pDnLifeSkillCookingDlg );
#endif // PRE_ADD_COOKING_SYSTEM

	/////////////////////////////////////////////////////////////////////////////
	// Note : Create Main Dialog, right side
	m_pInventory = new CDnInvenTabDlg( UI_TYPE_CHILD, this, INVENTORY_DIALOG, this );
	m_pInventory->Initialize( false );
	m_RightDlgGroup.AddDialog( INVENTORY_DIALOG, m_pInventory );

	//m_pSkillDlg = new CDnSkillTabDlg( UI_TYPE_CHILD, this, SKILL_DIALOG, this );
	//m_pSkillDlg->Initialize( false );
	//m_RightDlgGroup.AddDialog( SKILL_DIALOG, m_pSkillDlg );

	m_pCommunityDlg = new CDnCommunityDlg( UI_TYPE_CHILD, this, COMMUNITY_DIALOG, this );
	m_pCommunityDlg->Initialize( false );
	m_RightDlgGroup.AddDialog( COMMUNITY_DIALOG, m_pCommunityDlg );

	m_pQuestSumInfoDlg = new CDnQuestSummaryInfoDlg( UI_TYPE_CHILD, this, NOTIFIER_DIALOG, this );
	m_pQuestSumInfoDlg->Initialize( false );
	m_RightDlgGroup.AddDialog( NOTIFIER_DIALOG, m_pQuestSumInfoDlg );

	/////////////////////////////////////////////////////////////////////////////
	// Note : Create Main Dialog, left side
	m_pCharStatus = new CDnCharStatusDlg( UI_TYPE_CHILD, this, CHARSTATUS_DIALOG, this );
	m_pCharStatus->Initialize( false );
	m_LeftDlgGroup.AddDialog( CHARSTATUS_DIALOG, m_pCharStatus );

	m_pMailDlg = new CDnMailDlg( UI_TYPE_CHILD, this, MAIL_DIALOG, this );
	m_pMailDlg->Initialize( false );
	m_LeftDlgGroup.AddDialog( MAIL_DIALOG, m_pMailDlg );

	m_pStorageDlg = new CDnPGStorageTabDlg( UI_TYPE_CHILD, this, PLAYER_GUILD_STORAGE_DIALOG, this );
	m_pStorageDlg->Initialize( false );
	m_LeftDlgGroup.AddDialog( PLAYER_GUILD_STORAGE_DIALOG, m_pStorageDlg );

	m_pStoreDlg = new CDnStoreTabDlg( UI_TYPE_CHILD, this, STORE_DIALOG, this );
	m_pStoreDlg->Initialize( false );
	m_LeftDlgGroup.AddDialog( STORE_DIALOG, m_pStoreDlg );

	m_pSkillStoreDlg = new CDnSkillStoreTabDlg( UI_TYPE_CHILD, this, SKILL_STORE_DIALOG, this );
	m_pSkillStoreDlg->Initialize( false );
	m_LeftDlgGroup.AddDialog( SKILL_STORE_DIALOG, m_pSkillStoreDlg );

	m_pMarketDlg = new CDnMarketTabDlg( UI_TYPE_CHILD, this, MARKET_DIALOG, this );
	m_pMarketDlg->Initialize( false );
	m_LeftDlgGroup.AddDialog( MARKET_DIALOG, m_pMarketDlg );

	m_pItemDisjointDlg = new CDnItemDisjointDlg( UI_TYPE_CHILD, this, ITEM_DISJOINT_DIALOG, this );
	m_pItemDisjointDlg->Initialize( false );
	m_LeftDlgGroup.AddDialog( ITEM_DISJOINT_DIALOG, m_pItemDisjointDlg );

	m_pPlateMainDlg = new CDnPlateMainDlg( UI_TYPE_CHILD, this, PLATE_MAIN_DIALOG, this );
	m_pPlateMainDlg->Initialize( false );
	m_LeftDlgGroup.AddDialog( PLATE_MAIN_DIALOG, m_pPlateMainDlg );

	m_pItemUpgradeDlg = new CDnItemUpgradeDlg( UI_TYPE_CHILD, this, ITEM_UPGRADE_DIALOG, this );
	m_pItemUpgradeDlg->Initialize( false );
	m_LeftDlgGroup.AddDialog( ITEM_UPGRADE_DIALOG, m_pItemUpgradeDlg );

#ifdef PRE_ADD_EXCHANGE_ENCHANT
	m_pItemUpgradeExchangeDlg = new CDnItemUpgradeExchangeDlg(UI_TYPE_CHILD, this, ITEM_UPGRADE_EXCHANGE_DIALOG, this);
	m_pItemUpgradeExchangeDlg->Initialize(false);
	m_LeftDlgGroup.AddDialog(ITEM_UPGRADE_EXCHANGE_DIALOG, m_pItemUpgradeExchangeDlg);
#endif

	m_pItemCompoundDlg = new CDnItemCompoundTabDlg( UI_TYPE_CHILD, this, ITEM_COMPOUND_DIALOG, this );
	m_pItemCompoundDlg->Initialize( false );
	m_LeftDlgGroup.AddDialog( ITEM_COMPOUND_DIALOG, m_pItemCompoundDlg );

	m_pCosMixDlg = new CDnCostumeMixDlg(UI_TYPE_CHILD, this, ITEM_COSTUMEMIX_DIALOG, this);
	m_pCosMixDlg->Initialize(false);
	m_LeftDlgGroup.AddDialog( ITEM_COSTUMEMIX_DIALOG, m_pCosMixDlg );

	m_pCosDMixDlg = new CDnCostumeDesignMixDlg(UI_TYPE_CHILD, this, ITEM_COSTUME_DMIX_DIALOG, this);
	m_pCosDMixDlg->Initialize(false);
	m_LeftDlgGroup.AddDialog( ITEM_COSTUME_DMIX_DIALOG, m_pCosDMixDlg );

#ifdef PRE_ADD_COSRANDMIX
	m_pCosRandMixDlg = new CDnCostumeRandomMixDlg(UI_TYPE_CHILD, this, ITEM_COSTUME_RANDOMMIX_DIALOG, this);
	m_pCosRandMixDlg->Initialize(false);
	m_LeftDlgGroup.AddDialog( ITEM_COSTUME_RANDOMMIX_DIALOG, m_pCosRandMixDlg );
#endif

	m_pLifeStorageDlg = new CDnLifeStorageDlg( UI_TYPE_CHILD, this, LIFESTORAGE_DIALOG, this );
	m_pLifeStorageDlg->Initialize( false );
	m_LeftDlgGroup.AddDialog( LIFESTORAGE_DIALOG, m_pLifeStorageDlg );

#ifdef PRE_ADD_DONATION
	m_pContributionDlg = new CDnContributionDlg( UI_TYPE_CHILD, this, CONTRIBUTION_DIALOG, this, true );
	m_pContributionDlg->Initialize( false );
	m_LeftDlgGroup.AddDialog( CONTRIBUTION_DIALOG, m_pContributionDlg );
#endif

#ifdef PRE_MOD_ITEM_COMPOUND_DIALOG_RENEWAL
	m_pItemCompoundRenewalDlg = new CDnItemCompoundRenewalTabDlg( UI_TYPE_CHILD, this, ITEM_COMPOUND_DIALOG, this );
	m_pItemCompoundRenewalDlg->Initialize( false );
	m_MiddleDlgGroup.AddDialog( ITEM_COMPOUND2_DIALOG, m_pItemCompoundRenewalDlg );
#else
	m_pItemCompound2Dlg = new CDnItemCompoundTab2Dlg( UI_TYPE_CHILD, this, ITEM_COMPOUND_DIALOG, this );
	m_pItemCompound2Dlg->Initialize( false );
	m_MiddleDlgGroup.AddDialog( ITEM_COMPOUND2_DIALOG, m_pItemCompound2Dlg );
#endif 

#if defined(PRE_ADD_INTEGERATE_EVENTUI)
	m_pIntegrateEventDlg = new CDnIntegrateEventDlg(UI_TYPE_CHILD, this, INTEGRATE_EVENT_DIALOG, this);
	m_pIntegrateEventDlg->Initialize(false);
	m_MiddleDlgGroup.AddDialog(INTEGRATE_EVENT_DIALOG, m_pIntegrateEventDlg);
	
	//m_MiddleDlgGroup.AddDialog(TIMEEVENT_DIALOG, m_pIntegrateEventDlg);
	//m_MiddleDlgGroup.AddDialog(ATTENDANCEEVENT_DIALOG, m_pIntegrateEventDlg);

#else
	m_pTimeEventDlg = new CDnTimeEventDlg( UI_TYPE_CHILD, this, TIMEEVENT_DIALOG, this );
	m_pTimeEventDlg->Initialize( false );
	m_MiddleDlgGroup.AddDialog( TIMEEVENT_DIALOG, m_pTimeEventDlg );

	m_pAttendanceEventDlg = new CDnAttendanceEventDlg( UI_TYPE_CHILD, this, ATTENDANCEEVENT_DIALOG, this );
	m_pAttendanceEventDlg->Initialize( false );
	m_MiddleDlgGroup.AddDialog( ATTENDANCEEVENT_DIALOG, m_pAttendanceEventDlg );
#endif // PRE_ADD_INTEGERATE_EVENTUI

#ifdef PRE_ADD_NOTIFY_ITEM_COMPOUND
	m_pItemCompoundNotifyDlg = new CDnItemCompoundNotifyDlg( UI_TYPE_CHILD, this, ITEM_COMPOUND_NOTIFY_DIALOG, this );
	m_pItemCompoundNotifyDlg->Initialize( true );
	m_RightDlgGroup.AddDialog( ITEM_COMPOUND_NOTIFY_DIALOG, m_pItemCompoundNotifyDlg );
#endif

#ifdef PRE_ADD_CASHSHOPBANNER_RENEWAL
	m_pCashShopIngameBannerDlg = new CDnCashShopIngameBanner( UI_TYPE_CHILD, this, CASHSHOP_INGAMEBANNER_DIALOG, this );
	m_pCashShopIngameBannerDlg->Initialize( true );
	m_RightDlgGroup.AddDialog( CASHSHOP_INGAMEBANNER_DIALOG, m_pCashShopIngameBannerDlg );
#endif // PRE_ADD_CASHSHOPBANNER_RENEWAL

//#ifdef PRE_ADD_PVP_RANKING
//	m_pPvPRankDlg = new CDnPvPRankDlg( UI_TYPE_FOCUS, this, PVP_RANK_DIALOG, this, true );
//	m_pPvPRankDlg->Initialize( false );	
//	m_MiddleDlgGroup.AddDialog( PVP_RANK_DIALOG, m_pPvPRankDlg );
//#endif // PRE_ADD_PVP_RANKING

#ifdef PRE_SPECIALBOX
	m_pEventReceiverTabDlg = new CDnEventReceiverTabDlg( UI_TYPE_CHILD, this, EVENT_RECEIVER_DIALOG, this, true );
	m_pEventReceiverTabDlg->Initialize( false );	
	m_LeftDlgGroup.AddDialog( EVENT_RECEIVER_DIALOG, m_pEventReceiverTabDlg );
#endif

#if defined( PRE_ALTEIAWORLD_EXPLORE )
	m_pAlteaTabDlg = new CDnAlteaTabDlg( UI_TYPE_CHILD, this, ALTEA_TAB_DIALOG, this );
	m_pAlteaTabDlg->Initialize( false );
	m_MiddleDlgGroup.AddDialog( ALTEA_TAB_DIALOG, m_pAlteaTabDlg );
#endif	// #if defined( PRE_ALTEIAWORLD_EXPLORE )

	/////////////////////////////////////////////////////////////////////////////
	//
	m_pButtonCtrl_01 = GetControl<CEtUIButton>("ID_BUTTON_CTRL01");
	m_pButtonCtrl_02 = GetControl<CEtUIButton>("ID_BUTTON_CTRL02");
	m_pButtonZ_01 = GetControl<CEtUIButton>("ID_BUTTON_Z01");
	m_pButtonZ_02 = GetControl<CEtUIButton>("ID_BUTTON_Z02");
	m_pButton_Cannon = GetControl<CEtUIButton>("ID_BUTTON_ESC");

	ToggleButtonCtrl( 1 );
	ToggleButtonZ( 2 );
}

void CDnMainMenuDlg::Show( bool bShow )
{
	if( m_bShow == bShow )
		return;

	if( m_bIgnoreShowFunc )
	{
		m_bIgnoreShowFunc = false;
		return;
	}

	if( bShow )
	{
	}
	else
	{
		CloseMenuDialog();
	}

	CEtUIDialog::Show( bShow );
	//CDnMouseCursor::GetInstance().ShowCursor( bShow );
}

void CDnMainMenuDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	CEtUIDialog::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );

	SetCmdControlName( pControl->GetControlName() );

	if( nCommand == EVENT_BUTTON_CLICKED )
	{
		if( GetInterface().IsShowPrivateMarketDlg() || GetInterface().IsShowCashShopDlg() 
			|| GetInterface().IsShowCostumeMixDlg() 
			|| CDnLocalPlayerActor::IsLockInput()
			)
			return;

		if( IsCmdControl("ID_BUTTON_CTRL01" ) )
		{
			CDnMouseCursor::GetInstance().ShowCursor( true, true );
			ToggleButtonCtrl( 2 );
			return;
		}

		if( IsCmdControl("ID_BUTTON_CTRL02" ) )
		{
			CDnMouseCursor::GetInstance().ShowCursor( false, true );
			ToggleButtonCtrl( 1 );
			return;
		}

		if( IsCmdControl("ID_BUTTON_Z01" ) )
		{
			CDnLocalPlayerActor *pPlayer = dynamic_cast<CDnLocalPlayerActor*>((CDnActor*)CDnActor::s_hLocalActor);
			if( pPlayer && !pPlayer->IsDie() && ( pPlayer->IsStay() || pPlayer->IsMove() ) )
			{
				if( pPlayer->IsMove() ) pPlayer->CmdStop( "Stand" );
				pPlayer->CmdToggleBattle(false);
				ToggleButtonZ( 2 );
			}
			return;
		}

		if( IsCmdControl("ID_BUTTON_Z02" ) )
		{
			CDnLocalPlayerActor *pPlayer = dynamic_cast<CDnLocalPlayerActor*>((CDnActor*)CDnActor::s_hLocalActor);
			if( pPlayer && !pPlayer->IsDie() && ( pPlayer->IsStay() || pPlayer->IsMove() ) )
			{
				if( pPlayer->IsCanBattleMode() )
				{
					if( pPlayer->IsMove() ) pPlayer->CmdStop( "Stand" );
					pPlayer->CmdToggleBattle( true );
					ToggleButtonZ( 1 );
				}
				else {
					CDnInterface::GetInstance().ShowCaptionDialog( CDnInterface::typeCaption3, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 100056 ), textcolor::YELLOW, 4.0f );
				}
			}
			return;
		}
		if( IsCmdControl("ID_BUTTON_ESC" ) )
		{
			CDnLocalPlayerActor *pPlayer = dynamic_cast<CDnLocalPlayerActor*>((CDnActor*)CDnActor::s_hLocalActor);
			if( pPlayer && !pPlayer->IsDie() && ( pPlayer->IsStay() || pPlayer->IsMove() ) )
			{
				if( pPlayer->IsCannonMode() )
				{
					pPlayer->ExitCannonMode();
				}
			}
			return;
		}
	}
}

void CDnMainMenuDlg::Render( float fElapsedTime )
{
	if( !IsAllowRender() ) return;

	if( GetInterface().IsOpenBlind() 
		|| GetInterface().IsShowCostumeMixDlg()
		|| GetInterface().IsShowCostumeDesignMixDlg()
		|| CDnLocalPlayerActor::IsLockInput()
		)
	{
		m_pButtonCtrl_01->Show( false );
		m_pButtonCtrl_02->Show( false );
		m_pButtonZ_01->Show( false );
		m_pButtonZ_02->Show( false );
	}
	else
	{
		bool bShowCursor = CDnMouseCursor::GetInstance().IsShowCursor();
		ToggleButtonCtrl( bShowCursor ? 2 : 1 );

		CDnLocalPlayerActor *pPlayer = dynamic_cast<CDnLocalPlayerActor*>((CDnActor*)CDnActor::s_hLocalActor);
		if( pPlayer )
		{
			bool bBattle = pPlayer->IsBattleMode();

			if(pPlayer->IsCannonMode())
			{
				ToggleButtonZ( 3 );
			}
			else
				ToggleButtonZ( bBattle ? 1 : 2 );

		}
	}

	CEtUIDialog::Render( fElapsedTime );
}

void CDnMainMenuDlg::RefreshPartyList()
{
	if( !m_pCommunityDlg ) return;
	m_pCommunityDlg->RefreshPartyList();
}

void CDnMainMenuDlg::RefreshPartyInfoList()
{
	if( !m_pCommunityDlg ) return;
	m_pCommunityDlg->RefreshPartyInfoList();
}

void CDnMainMenuDlg::RefreshPartyMemberList()
{
	if( !m_pCommunityDlg ) return;
	m_pCommunityDlg->RefreshPartyMemberList();
}

void CDnMainMenuDlg::SetPartyInfoLeader( int nLeaderIndex )
{
	if( !m_pCommunityDlg ) return;
	m_pCommunityDlg->SetPartyInfoLeader( nLeaderIndex );
}

void CDnMainMenuDlg::SwapPartyDialog()
{
	switch( CDnPartyTask::GetInstance().GetPartyRole() ) 
	{
	case CDnPartyTask::LEADER:
	case CDnPartyTask::MEMBER:
		m_pCommunityDlg->ShowPartyDialog( CDnCommunityDlg::typePartyInfoList );
		break;
	default:
		m_pCommunityDlg->ShowPartyDialog( CDnCommunityDlg::typePartyList );
		break;
	}
}

int CDnMainMenuDlg::GetPartyListStartIndex()
{
	if( !m_pCommunityDlg ) return 0;
	return m_pCommunityDlg->GetPartyListStartIndex();
}

void CDnMainMenuDlg::OnUICallbackProc( int nID, UINT nCommand, CEtUIControl *pControl, UINT uiMsg )
{
	if( pControl == NULL )
	{
		CDnMainDlg *pMainDlg = GetInterface().GetMainBarDialog();
		if( !pMainDlg ) return;

		if( nCommand == EVENT_DIALOG_SHOW )
		{
			pMainDlg->ToggleMenuButton( nID, true );
			return;
		}

		if( nCommand == EVENT_DIALOG_HIDE )
		{
			pMainDlg->ToggleMenuButton( nID, false );

			switch( nID )
			{
			case INVENTORY_DIALOG:
			case COMMUNITY_DIALOG:
				// For the world map and zone map, since the notifier is not hidden,
				// No need to call separately like this.
				// This call actually turns on the RightGroup notifier when trying to open inventory with the zone map open, preventing inventory from opening immediately.
				//case WORLDMAP_DIALOG:
				//case ZONEMAP_DIALOG:
				// Since the mission window hides the tracker, closing it with Esc requires explicitly turning the tracker back on.
			case MISSION_DIALOG:
				// However, because it is handled as above here, when opening the inventory window (right group) while the mission window is open
				// When the RightGroup notifier turns on, the inventory doesn't open immediately. Guess it can't be helped.
				// So decided to additionally check the condition below.
				{
					// If any window in RightGroup is open,
					if( m_RightDlgGroup.IsShowDialog(INVENTORY_DIALOG) ||
						m_RightDlgGroup.IsShowDialog(COMMUNITY_DIALOG) )
						break;
					SetQuestSumInfoDlg();
				}
				break;
			}
			return;
		}
	}
	else
	{
		SetCmdControlName( pControl->GetControlName() );

		if( nCommand == EVENT_BUTTON_CLICKED )
		{
			if( IsCmdControl( "ID_CLOSE_DIALOG" ) )
			{
				if( CDnItemTask::IsActive() && CDnItemTask::GetInstance().IsRequestWait() ) return;
				if( uiMsg == WM_KEYDOWN )
				{
					// When the ESC key is pressed...
					CloseMenuDialog();
				}
				else
				{
					// When blind is open and ID_CLOSE_DIALOG is pressed, close everything for now.
					// Code to prevent closing only inventory by clicking X on inventory when shop, storage, disassemble, etc. windows are open,
					// Looks like completely temporary code.
					//
					// For trade, there is no blind, so it goes into InvenTab's Callback handling.
					if( GetInterface().IsOpenBlind() )
						CloseMenuDialog();

					m_MiddleDlgGroup.ShowDialog(nID, false, false );
					m_LeftDlgGroup.ShowDialog( nID, false, false );
					m_RightDlgGroup.ShowDialog( nID, false, false );

					switch( nID )
					{
#ifdef PRE_ADD_MAIL_UI_IMPROVEMENTS
					case MAIL_DIALOG:
						{
							CDnMainDlg *pMainDlg = GetInterface().GetMainBarDialog();
							pMainDlg->CloseBlinkMenuButton( nID );
						}break;			
#endif
					case INVENTORY_DIALOG:
					case COMMUNITY_DIALOG:
						//case WORLDMAP_DIALOG:
						//case ZONEMAP_DIALOG:
					case MISSION_DIALOG:
						SetQuestSumInfoDlg();
						break;
					}
				}

				return;
			}

			if( nID == SYSTEM_DIALOG )
			{
				ShowSystemDialog( false );
				return;
			}
		}
	}
}

bool CDnMainMenuDlg::IsOpenDialog()
{
	CEtUIDialogGroup::DIALOG_VEC &vecLeftDialog = m_LeftDlgGroup.GetDialogList();
	for( int i=0; i<(int)vecLeftDialog.size(); ++i )
	{
		CEtUIDialogGroup::ID_DIALOG_PAIR &id_dlg_pair = vecLeftDialog[i];

		if( id_dlg_pair.second->IsShow() )
			return true;
	}

	CEtUIDialogGroup::DIALOG_VEC &vecRightDialog = m_RightDlgGroup.GetDialogList();
	for( int i=0; i<(int)vecRightDialog.size(); ++i )
	{
		CEtUIDialogGroup::ID_DIALOG_PAIR &id_dlg_pair = vecRightDialog[i];

		if( id_dlg_pair.second->GetDialogID() == NOTIFIER_DIALOG )
			continue;

#ifdef PRE_ADD_NOTIFY_ITEM_COMPOUND
		if( id_dlg_pair.second->GetDialogID() == ITEM_COMPOUND_NOTIFY_DIALOG )
			continue;
#endif

#ifdef PRE_ADD_CASHSHOPBANNER_RENEWAL
		if( id_dlg_pair.second->GetDialogID() == CASHSHOP_INGAMEBANNER_DIALOG )
			continue;
#endif // PRE_ADD_CASHSHOPBANNER_RENEWAL

		if( id_dlg_pair.second->IsShow() )
			return true;
	}

	CEtUIDialogGroup::DIALOG_VEC &vecMiddleDialog = m_MiddleDlgGroup.GetDialogList();
	for( int i=0; i<(int)vecMiddleDialog.size(); ++i )
	{
		CEtUIDialogGroup::ID_DIALOG_PAIR &id_dlg_pair = vecMiddleDialog[i];

		if( id_dlg_pair.second->IsShow() )
			return true;
	}

	return false;
}

//void CDnMainMenuDlg::ShowButtonInfoDialog( bool bShow, float fX, float fY, LPCWSTR szwInfo, LPCWSTR szwKey )
//{
//	if( bShow )
//	{
//		m_pButtonInfoDlg->SetInfo( szwInfo, szwKey );
//		//m_pButtonInfoDlg->Show(true);
//		ShowChildDialog( m_pButtonInfoDlg, true );
//
//		SUICoord dlgCoord;
//		m_pButtonInfoDlg->GetDlgCoord( dlgCoord );
//		dlgCoord.fX = fX + GetXCoord();
//		dlgCoord.fY = fY + GetYCoord() - dlgCoord.fHeight - 0.004f;
//		m_pButtonInfoDlg->SetDlgCoord( dlgCoord );
//	}
//	else
//	{
//		//m_pButtonInfoDlg->Show(false);
//		ShowChildDialog( m_pButtonInfoDlg, false );
//	}
//}

#ifdef PRE_ADD_DONATION
void CDnMainMenuDlg::OpenContributionDialog( bool bShow )
{	
	// Open Inventory.
	m_RightDlgGroup.ShowDialog(INVENTORY_DIALOG, bShow);

	// Open donation window.
	m_LeftDlgGroup.ShowDialog( CONTRIBUTION_DIALOG, bShow);	
}

bool CDnMainMenuDlg::IsOpenDonation()
{
	if( !m_pContributionDlg )
		false;

	return m_pContributionDlg->IsShow();
}

#endif

void CDnMainMenuDlg::OpenStorageDialog( int iRemoteItemID, bool bGuildStorageOpened /*= false*/ )
{
	m_pStorageDlg->SetRemoteItemID( iRemoteItemID );

	CDnPGStorageTabDlg* pPGStorageDlg = static_cast<CDnPGStorageTabDlg*>(m_LeftDlgGroup.GetDialog( PLAYER_GUILD_STORAGE_DIALOG ));
	pPGStorageDlg->SetOpenGuildStorage( bGuildStorageOpened );
	m_LeftDlgGroup.ShowDialog(PLAYER_GUILD_STORAGE_DIALOG, true);

	m_RightDlgGroup.ShowDialog(INVENTORY_DIALOG, true);
}

#ifdef PRE_ADD_PVPRANK_INFORM
void CDnMainMenuDlg::OpenStoreDialog(int nType, int nShopID)
{
	m_LeftDlgGroup.ShowDialog(STORE_DIALOG, true);
	m_RightDlgGroup.ShowDialog(INVENTORY_DIALOG, true);
	m_pStoreDlg->SetStoreType(nType);
	m_pStoreDlg->CheckPvPRankButton(nShopID);
}
#else
void CDnMainMenuDlg::OpenStoreDialog(int nType)
{
	m_LeftDlgGroup.ShowDialog(STORE_DIALOG, true);
	m_RightDlgGroup.ShowDialog(INVENTORY_DIALOG, true);
	m_pStoreDlg->SetStoreType(nType);
}
#endif

void CDnMainMenuDlg::OpenSkillStoreDialog()
{
	m_pSkillTreeDlg->ResetCheckedButton();
	m_MiddleDlgGroup.ShowDialog(SKILL_DIALOG, true);
	m_pSkillTreeDlg->SetUnlockByMoneyMode();
}

void CDnMainMenuDlg::OpenMarketDialog()
{	
	m_RightDlgGroup.ShowDialog(INVENTORY_DIALOG, true);	// Open inventory first...
	m_LeftDlgGroup.ShowDialog(MARKET_DIALOG, true);

	DWORD nUID = CDnLocalPlayerActor::GetTakeNpcUID();
	DnActorHandle hNpc = CDnActor::FindActorFromUniqueID(nUID);

	if ( hNpc ) {
		CDnNPCActor* pActor = dynamic_cast<CDnNPCActor*>(hNpc.GetPointer());
		if ( pActor ) {
			pActor->OnSoundPlay( CDnNPCActor::OpenShop );
		}
	}
}

void CDnMainMenuDlg::OpenItemDisjointDialog()
{
	m_LeftDlgGroup.ShowDialog(ITEM_DISJOINT_DIALOG, true);
	m_RightDlgGroup.ShowDialog(INVENTORY_DIALOG, true);
}

void CDnMainMenuDlg::OpenPlateMainDialog()
{
	m_LeftDlgGroup.ShowDialog(PLATE_MAIN_DIALOG, true);
	m_RightDlgGroup.ShowDialog(INVENTORY_DIALOG, true);
}

void CDnMainMenuDlg::OpenItemUpgradeDialog( int nRemoteItemID )
{
	m_pItemUpgradeDlg->SetRemoteItemID( nRemoteItemID );
	m_LeftDlgGroup.ShowDialog(ITEM_UPGRADE_DIALOG, true);
	m_RightDlgGroup.ShowDialog(INVENTORY_DIALOG, true);
}

#ifdef PRE_ADD_EXCHANGE_ENCHANT
void CDnMainMenuDlg::OpenItemUpgradeExchangeDialog()
{
	m_LeftDlgGroup.ShowDialog(ITEM_UPGRADE_EXCHANGE_DIALOG, true);
	m_RightDlgGroup.ShowDialog(INVENTORY_DIALOG, true);
}
#endif

void CDnMainMenuDlg::OpenItemCompoundDialog( int nCompoundShopID )
{
	m_pItemCompoundDlg->SetCompoundListID( nCompoundShopID );

	m_LeftDlgGroup.ShowDialog(ITEM_COMPOUND_DIALOG, true);
	m_RightDlgGroup.ShowDialog(INVENTORY_DIALOG, true);
}

void CDnMainMenuDlg::OpenItemCompound2Dialog( int nCompoundShopID, int nRemoteItemID )
{
#ifdef PRE_MOD_ITEM_COMPOUND_DIALOG_RENEWAL
	m_pItemCompoundRenewalDlg->ResetForceOpenMode();
	m_pItemCompoundRenewalDlg->SetRemoteItemID( nRemoteItemID );
#else
	m_pItemCompound2Dlg->SetRemoteItemID( nRemoteItemID );
	m_pItemCompound2Dlg->SetCompoundListID( nCompoundShopID );
#endif 
	m_MiddleDlgGroup.ShowDialog( ITEM_COMPOUND2_DIALOG, true );
}

#if defined(PRE_ADD_INTEGERATE_EVENTUI)
void CDnMainMenuDlg::ShowIntegrateEventDialog(bool bShow)
{
	if (m_pIntegrateEventDlg == NULL)
		return;
#ifdef PRE_ADD_STAMPSYSTEM 

	// Show the stamp tab first.
	DWORD selectTabIndex = CDnIntegrateEventDlg::STAMP_EVENT_TAB;	

	// Show the stamp tab with priority if max level.
	bool bMaxLevel = false;
	if( CDnActor::s_hLocalActor )
	{
		int nLimitLevel = (int)CGlobalWeightTable::GetInstance().GetValue( CGlobalWeightTable::PlayerLevelLimit );
		if( CDnActor::s_hLocalActor->GetLevel() == nLimitLevel )
			bMaxLevel = true;	
	}
	
	DWORD timeEventCount = 0;
	DWORD attendanceEventCount = 0;
	bool isActiveEvent = m_pIntegrateEventDlg->GetActiveEventCount(timeEventCount, attendanceEventCount);

	// Show stamps if max level.
	if( bMaxLevel )
	{		
		CDnStampEventDlg * pStampDlg = m_pIntegrateEventDlg->GetStampEventDlg();
		if( pStampDlg && pStampDlg->IsExistStamp() == false )
		{
			if( isActiveEvent )
			{
				selectTabIndex = timeEventCount > 0 ? CDnIntegrateEventDlg::TIME_EVENT_TAB : CDnIntegrateEventDlg::ATTENDANCE_EVENT_TAB;
			}
			else
			{
				//Display message that there is no event and return...
				GetInterface().MessageBox( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7736 ) );
				return;
			}
		}
	}
	
	// Hide the stamp tab if not max level.
	else
	{
		if( isActiveEvent )
		{
			selectTabIndex = CDnIntegrateEventDlg::TIME_EVENT_TAB;
			//If both have events, activate TIME_EVENT_TAB
			if (timeEventCount > 0)
				selectTabIndex = CDnIntegrateEventDlg::TIME_EVENT_TAB;
			else
				selectTabIndex = CDnIntegrateEventDlg::ATTENDANCE_EVENT_TAB;
		}
		else
		{
			//Display message that there is no event and return...
			GetInterface().MessageBox( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7736 ) );
			return;
		}
	}

#else
	DWORD timeEventCount = 0;
	DWORD attendanceEventCount = 0;

	bool isActiveEvent = m_pIntegrateEventDlg->GetActiveEventCount(timeEventCount, attendanceEventCount);
	if (isActiveEvent == false)
	{
		//Display message that there is no event and return...
		GetInterface().MessageBox( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7736 ) );
		return;
	}

	DWORD selectTabIndex = CDnIntegrateEventDlg::TIME_EVENT_TAB;
	//If both have events, activate TIME_EVENT_TAB
	if (timeEventCount > 0)
		selectTabIndex = CDnIntegrateEventDlg::TIME_EVENT_TAB;
	else
		selectTabIndex = CDnIntegrateEventDlg::ATTENDANCE_EVENT_TAB;
#endif // PRE_ADD_STAMPSYSTEM

	m_pIntegrateEventDlg->SelectTab(selectTabIndex);

	m_MiddleDlgGroup.ShowDialog( INTEGRATE_EVENT_DIALOG, bShow );

	if (bShow == false)
		m_pIntegrateEventDlg->Show(false);
}
#endif // PRE_ADD_INTEGERATE_EVENTUI

#ifdef PRE_ADD_NOTIFY_ITEM_COMPOUND
void CDnMainMenuDlg::ShowItemCompoundNotifyDlg( bool bShow )
{
	m_pItemCompoundNotifyDlg->Show( bShow );
}
void CDnMainMenuDlg::SetItemCompoundNotify( int nItemIndex , int nOptionIndex )
{
	m_pItemCompoundNotifyDlg->SetItemNotify( nItemIndex , nOptionIndex  );
#ifdef PRE_MOD_ITEM_COMPOUND_DIALOG_RENEWAL
#else
	if( m_pItemCompound2Dlg->IsShow() )
	{
		m_pItemCompound2Dlg->RefreshGroupList( false );
		m_pItemCompound2Dlg->RefreshOptionList( false );
	}
#endif 
}
void CDnMainMenuDlg::ResetItemCompoundNotify()
{
	m_pItemCompoundNotifyDlg->ResetItemNotify();
#ifdef PRE_MOD_ITEM_COMPOUND_DIALOG_RENEWAL
#else
	if( m_pItemCompound2Dlg->IsShow() )
	{
		m_pItemCompound2Dlg->RefreshGroupList( false );
		m_pItemCompound2Dlg->RefreshOptionList( false );
	}
#endif 
}
void CDnMainMenuDlg::RefreshCompoundNotifyPercentage()
{
	m_pItemCompoundNotifyDlg->RefreshItemPercentage();
}
#endif

void CDnMainMenuDlg::ShowTimeEventDialog( bool bShow )
{
	m_MiddleDlgGroup.ShowDialog( TIMEEVENT_DIALOG, bShow );
}

void CDnMainMenuDlg::ShowAttendanceEventDialog( bool bShow )
{
	m_MiddleDlgGroup.ShowDialog( ATTENDANCEEVENT_DIALOG, bShow );
}

void CDnMainMenuDlg::ShowCostumeMixDialog(bool bShow, bool bCloseInvenTab)
{
#ifdef PRE_ADD_COSRANDMIX
	OnShowMixDialog(ITEM_COSTUMEMIX_DIALOG, bShow, bCloseInvenTab);
#else
	m_LeftDlgGroup.ShowDialog(ITEM_COSTUMEMIX_DIALOG, bShow);
	if (bShow == false && bCloseInvenTab)
		m_RightDlgGroup.ShowDialog(INVENTORY_DIALOG, bShow);

	CDnMainDlg *pMainDlg = GetInterface().GetMainBarDialog();
	if (bShow)
	{
		m_pInventory->EnableInvenTab(CDnInvenTabDlg::eCASH);
		m_pInventory->ShowTab(ST_INVENTORY_CASH);
	}
	else
	{
		m_pInventory->EnableInvenTab(CDnInvenTabDlg::eCOMMON |
									 CDnInvenTabDlg::eCASH |
									 CDnInvenTabDlg::eQUEST |
									 CDnInvenTabDlg::eGESTURE | 
									 CDnInvenTabDlg::ePET
#ifdef PRE_ADD_CASHREMOVE
									 | CDnInvenTabDlg::eCASHREMOVE
#endif
									 );

		if (pMainDlg)
		{
			pMainDlg->EnableButtons(true);
#ifdef PRE_FIX_HOLD_COSTUMEMIX_INVEN
			pMainDlg->HoldToggleShowDialogOnProcessButton(false, CDnMainMenuDlg::INVENTORY_DIALOG);
#endif
		}
	}
#endif // PRE_ADD_COSRANDMIX
}

void CDnMainMenuDlg::ShowCostumMergeDialog(bool bShow, bool bCloseInvenTab)
{
#ifdef PRE_ADD_COSRANDMIX
	OnShowMixDialog(ITEM_COSTUME_DMIX_DIALOG, bShow, bCloseInvenTab);
#else
	m_LeftDlgGroup.ShowDialog(ITEM_COSTUME_DMIX_DIALOG, bShow);
	if (bShow == false && bCloseInvenTab)
		m_RightDlgGroup.ShowDialog(INVENTORY_DIALOG, bShow);

	CDnMainDlg *pMainDlg = GetInterface().GetMainBarDialog();
	if (bShow)
	{
		m_pInventory->EnableInvenTab(CDnInvenTabDlg::eCASH);
		//ToggleShowDialog(INVENTORY_DIALOG);
		m_pInventory->ShowTab(ST_INVENTORY_CASH);
	}
	else
	{
		m_pInventory->EnableInvenTab(CDnInvenTabDlg::eCOMMON |
									 CDnInvenTabDlg::eCASH |
									 CDnInvenTabDlg::eQUEST |
									 CDnInvenTabDlg::eGESTURE | 
									 CDnInvenTabDlg::ePET 
#ifdef PRE_ADD_CASHREMOVE
									 | CDnInvenTabDlg::eCASHREMOVE
#endif
									 );

		if (pMainDlg)
		{
			pMainDlg->EnableButtons(true);
#ifdef PRE_FIX_HOLD_COSTUMEMIX_INVEN
			pMainDlg->HoldToggleShowDialogOnProcessButton(false, CDnMainMenuDlg::INVENTORY_DIALOG);
#endif
		}
	}
#endif // PRE_ADD_COSRANDMIX
}

#ifdef PRE_ADD_COSRANDMIX
void CDnMainMenuDlg::ShowCostumeRandomMixDialog(bool bShow, bool bCloseInvenTab)
{
	OnShowMixDialog(ITEM_COSTUME_RANDOMMIX_DIALOG, bShow, bCloseInvenTab);
}

void CDnMainMenuDlg::OnShowMixDialog(int nDlgId, bool bShow, bool bCloseInvenTab)
{
	m_LeftDlgGroup.ShowDialog(nDlgId, bShow);
	if (bShow == false && bCloseInvenTab)
		m_RightDlgGroup.ShowDialog(INVENTORY_DIALOG, bShow);

	CDnMainDlg *pMainDlg = GetInterface().GetMainBarDialog();
	if (bShow)
	{
		m_pInventory->EnableInvenTab(CDnInvenTabDlg::eCASH);
#ifdef PRE_ADD_COSRANDMIX_ACCESSORY
		if (nDlgId == ITEM_COSTUME_RANDOMMIX_DIALOG)
		{
			if (CDnItemTask::IsActive())
			{
				const CDnCostumeRandomMixDataMgr& mgr = CDnItemTask::GetInstance().GetCostumeRandomMixDataMgr();
				CostumeMix::RandomMix::eOpenType type = mgr.GetCurrentOpenType();
				if (type == CostumeMix::RandomMix::OpenByNpc)
					m_RightDlgGroup.ShowDialog(INVENTORY_DIALOG, bShow);
			}
		}
#endif
		m_pInventory->ShowTab(ST_INVENTORY_CASH);
	}
	else
	{
		m_pInventory->EnableInvenTab(CDnInvenTabDlg::eCOMMON |
			CDnInvenTabDlg::eCASH |
			CDnInvenTabDlg::eQUEST |
			CDnInvenTabDlg::eGESTURE | 
			CDnInvenTabDlg::ePET 
#ifdef PRE_ADD_CASHREMOVE
			| CDnInvenTabDlg::eCASHREMOVE
#endif
			);

		if (pMainDlg)
		{
			pMainDlg->EnableButtons(true);
#ifdef PRE_FIX_HOLD_COSTUMEMIX_INVEN
			pMainDlg->HoldToggleShowDialogOnProcessButton(false, CDnMainMenuDlg::INVENTORY_DIALOG);
#endif
		}
	}
}
#endif

void CDnMainMenuDlg::ShowLifeStorageDialog( bool bShow )
{
	m_LeftDlgGroup.ShowDialog(LIFESTORAGE_DIALOG, bShow);
	m_RightDlgGroup.ShowDialog(INVENTORY_DIALOG, bShow);
}

void CDnMainMenuDlg::OpenInvenDialog()
{
	m_RightDlgGroup.ShowDialog(INVENTORY_DIALOG, true);
}

void CDnMainMenuDlg::CloseInvenDialog()
{
	m_RightDlgGroup.ShowDialog(INVENTORY_DIALOG, false);
}

void CDnMainMenuDlg::OpenCharStatusDialog()
{
	m_LeftDlgGroup.ShowDialog(CHARSTATUS_DIALOG, true);
}

void CDnMainMenuDlg::OpenEventToday()
{
	if( !m_pEventTodayDlg ) return;
	m_pEventTodayDlg->ReadEventToday();

	if( m_pEventTodayDlg->GetShowEvent() )
		m_MiddleDlgGroup.ShowDialog(EVENT_DIALOG, true);
}

void CDnMainMenuDlg::CloseEventToday()
{
	if( m_pEventTodayDlg && m_pEventTodayDlg->IsShow() )
		m_MiddleDlgGroup.ShowDialog(EVENT_DIALOG, false);
}

void CDnMainMenuDlg::OpenGlobalEventDlg(SCScorePeriodQuest* pData)
{
	if(m_pGlobalEventDlg)
	{
		m_pGlobalEventDlg->SetEventInfo(pData);
		m_MiddleDlgGroup.ShowDialog(GLOBALEVENT_DIALOG, true);
	}
}

bool CDnMainMenuDlg::IsOpenDialog( DWORD dwDialogID )
{
	if( m_LeftDlgGroup.IsShowDialog(dwDialogID) )
		return true;

	if( m_RightDlgGroup.IsShowDialog(dwDialogID) )
		return true;

	if( m_MiddleDlgGroup.IsShowDialog(dwDialogID) )
		return true;

	return false;
}

bool CDnMainMenuDlg::IsForceOpenNormalInvenTabDialog()
{
	if(
#ifdef PRE_ADD_SERVER_WAREHOUSE
#else
		IsOpenDialog(PLAYER_GUILD_STORAGE_DIALOG) ||
#endif
		IsOpenDialog(MARKET_DIALOG) ||
		IsOpenDialog(ITEM_DISJOINT_DIALOG) ||
		IsOpenDialog(PLATE_MAIN_DIALOG) ||
		IsOpenDialog(ITEM_UPGRADE_DIALOG) ||
		IsOpenDialog(ITEM_COMPOUND_DIALOG) ||
		IsOpenDialog(LIFESTORAGE_DIALOG) ||
		GetInterface().IsShowPrivateMarketDlg()
		|| IsWithDrawGlyphOpen()
		)
	{
		return true;
	}
	return false;
}

void CDnMainMenuDlg::ToggleButtonCtrl( int nMode )	// Note : 1 is Crosshair, 2 is Cursor
{
	if( !m_pButtonCtrl_01 || !m_pButtonCtrl_02 ) return;
	if( nMode == 1 )
	{
		m_pButtonCtrl_01->Show( true );
		m_pButtonCtrl_02->Show( false );
	}
	else if( nMode == 2 )
	{
		m_pButtonCtrl_01->Show( false );
		m_pButtonCtrl_02->Show( true );
	}
}

void CDnMainMenuDlg::ToggleButtonZ( int nMode )	// Note : 1 is Attack mode, 2 is Peace mode, 3 is Cannon mode
{
	if( nMode == 1 )
	{
		m_pButtonZ_01->Show( true );
		m_pButtonZ_02->Show( false );
		m_pButton_Cannon->Show(false);
	}
	else if( nMode == 2 )
	{
		m_pButtonZ_01->Show( false );
		m_pButtonZ_02->Show( true );
		m_pButton_Cannon->Show(false);
	}
	else if( nMode == 3)
	{
		m_pButton_Cannon->Show(true);
		m_pButtonZ_01->Show( false );
		m_pButtonZ_02->Show( false );
	}

}

void CDnMainMenuDlg::ShowSystemDialog( bool bShow )
{
	if( bShow )
	{
		if( CDnLocalPlayerActor::IsLockInput() ) return;

		if (CDnPartyTask::IsActive() && GetPartyTask().IsLocalActorEnterGateReady() && !GetPartyTask().IsGMTracing())
			return;

		m_RightDlgGroup.CloseAllDialog();
		m_LeftDlgGroup.CloseAllDialog();

		SetQuestSumInfoDlg();

		if( CDnActor::s_hLocalActor )
		{
			if( CDnActor::s_hLocalActor->IsMove() && CDnActor::s_hLocalActor->IsMovable() ) 
			{
				CDnActor::s_hLocalActor->CmdStop( "Stand", 0, 3.f, 0.f, false, true );
			}
		}
		GetInterface().GetMainBarDialog()->ShowMinimapOption( false );
#ifdef PRE_ADD_AUTO_DICE
		GetInterface().GetMainBarDialog()->ShowAutoDiceOption( false );
#endif
	}

	m_MiddleDlgGroup.ShowDialog( SYSTEM_DIALOG, bShow );
}

void CDnMainMenuDlg::CloseSystemDialogCompletely()
{
	ShowSystemDialog(false);

	CEtUIDialog* pDialog = GetMainDialog(SYSTEM_DIALOG);
	if (pDialog)
		pDialog->OnCloseCompletely();
}

void CDnMainMenuDlg::ShowPrivateMarketDialog( bool bShow, bool bCancel )
{
	if( !m_pInventory )
		return;

	if( bShow )
	{
		CDnInterface::GetInstance().CloseWorldMsgDialog();

		m_RightDlgGroup.ShowDialog( INVENTORY_DIALOG, true );
		m_LeftDlgGroup.CloseAllDialog();
		m_MiddleDlgGroup.CloseAllDialog();
	}

	m_pInventory->ShowPrivateMarketDialog( bShow, bCancel );
}

void CDnMainMenuDlg::OpenMailDialog(bool bWriteDialog, const wchar_t *wszName, const wchar_t* wszTitle)
{
	m_LeftDlgGroup.ShowDialog(MAIL_DIALOG, true);
	m_RightDlgGroup.ShowDialog(INVENTORY_DIALOG, true);

	if (bWriteDialog && (wszName != NULL) && (wszTitle != NULL))
	{
		m_pMailDlg->OpenMailWriteDialog(wszName, wszTitle, true);
	}
}

void CDnMainMenuDlg::OpenGlyphLiftDialog()
{
	m_RightDlgGroup.ShowDialog(INVENTORY_DIALOG, true);
	m_LeftDlgGroup.ShowDialog(CHARSTATUS_DIALOG, true);

	m_pCharStatus->ChangeEquipPage( CDnCharStatusDlg::EquipPageType::EquipPagePlate );

	m_pCharStatus->SetWithDarwOpen( true );
}

void CDnMainMenuDlg::OpenNotifyArrowDialog( EtVector3 vTarget, LOCAL_TIME nTime, bool bImmediately)
{
	m_pNotifyArrowDlg->SetDirection(vTarget, nTime);
	// Call immediately when the direction indicator needs to be shown right away.
	if ( bImmediately )
	{
		m_pNotifyArrowDlg->OnBlindClosed();
	}

	// Otherwise called when the blind closes.
	else
	{
		GetInterface().GetBlindDialog()->AddBlindCallBack( m_pNotifyArrowDlg );
	}
}

void CDnMainMenuDlg::CloseNotifyArrowDialog()
{
	ShowChildDialog( m_pNotifyArrowDlg, false );
}

bool CDnMainMenuDlg::IsOpenQuestDlg()
{
	bool bShow = false;
#ifdef PRE_ADD_INTEGERATE_QUEST_REPUT
	if( m_pIntegrateQuestNReputationDlg )
		bShow = m_pIntegrateQuestNReputationDlg->IsShow();
#else // PRE_ADD_INTEGERATE_QUEST_REPUT
	if( m_pQuestTabDlg )
		bShow = m_pQuestTabDlg->IsShow();
#endif // PRE_ADD_INTEGERATE_QUEST_REPUT

	return bShow;
}

void CDnMainMenuDlg::SetQuestSumInfoDlg()
{
	bool bShow = true;

	if( CTaskManager::IsActive() ) 
	{
		CDnGameTask *pGameTask = (CDnGameTask *)CTaskManager::GetInstance().GetTask( "GameTask" );
		if( pGameTask && pGameTask->GetGameTaskType() == GameTaskType::PvP ) bShow = false;
	}
	CDnMainDlg *pMainBarDlg = GetInterface().GetMainBarDialog();
	if( pMainBarDlg && pMainBarDlg->IsShowGuildWarSkillSlotDlg() )
		bShow = false;

	if( m_RightDlgGroup.IsShowDialog(NOTIFIER_DIALOG) == false )
		m_RightDlgGroup.ShowDialog( NOTIFIER_DIALOG, bShow );

#ifdef PRE_ADD_NOTIFY_ITEM_COMPOUND // Uses the same space as quest notifications, so handle them together.
	if( m_pItemCompoundNotifyDlg && !m_pItemCompoundNotifyDlg->IsShow() )
	{
#ifdef PRE_MOD_ITEM_COMPOUND_DIALOG_RENEWAL
		int nLimitLevel = (int)CGlobalWeightTable::GetInstance().GetValue(CGlobalWeightTable::CompoundLimitMaxLevel) ;
		if( CDnActor::s_hLocalActor && CDnActor::s_hLocalActor->GetLevel() >= nLimitLevel )
#endif 
		m_pItemCompoundNotifyDlg->Show( bShow );
	}
#endif

#ifdef PRE_ADD_CASHSHOPBANNER_RENEWAL
	if( m_pCashShopIngameBannerDlg && !m_pCashShopIngameBannerDlg->IsShow() )
	{
		if( GetInterface().GetInterfaceType() != CDnInterface::Village )
			bShow = false;
		m_pCashShopIngameBannerDlg->Show( bShow );	
	}
#endif // PRE_ADD_CASHSHOPBANNER_RENEWAL	

}

void CDnMainMenuDlg::RefreshQuestSumInfo()
{
	if( !m_pQuestSumInfoDlg ) return;
	m_pQuestSumInfoDlg->RefreshNotifier();
}

void CDnMainMenuDlg::EnalbeQuestSumPriority(bool bTrue)
{
	if( !m_pQuestSumInfoDlg ) return;
	m_pQuestSumInfoDlg->SetPriorityMode(bTrue);
}

void CDnMainMenuDlg::ClearAllSumInfo()
{
	if( !m_pQuestSumInfoDlg ) return;
	m_pQuestSumInfoDlg->ClearAll();
}

void CDnMainMenuDlg::AddChannelList( sChannelInfo *pChannelInfo )
{
	if( !m_pSystemDlg ) return;
	m_pSystemDlg->AddChannelList( pChannelInfo );
}

void CDnMainMenuDlg::ShowWorldMapDialog( bool bShow )
{
	if( bShow )
	{
		m_RightDlgGroup.CloseAllDialog();
		m_LeftDlgGroup.CloseAllDialog();

		SetQuestSumInfoDlg();
	}

	m_MiddleDlgGroup.ShowDialog( WORLDMAP_DIALOG, bShow );
}

void CDnMainMenuDlg::ShowZoneMapDialog( bool bShow, int nMapIndex )
{
	if( CDnWorld::GetInstance().GetMapType() == CDnWorld::MapTypeUnknown )	return;
	if( CDnWorld::GetInstance().GetMapType() == CDnWorld::MapTypeDungeon )	
	{
		CDnGameTask * pGameTask = (CDnGameTask *)CTaskManager::GetInstance().GetTask( "GameTask" );
		if( pGameTask && pGameTask->GetGameTaskType() != GameTaskType::Farm )
			return;
	}

	if( !m_pZoneMapDlg->IsChanged( nMapIndex ) )	return;

	if( bShow )
	{
		m_RightDlgGroup.CloseAllDialog();
		m_LeftDlgGroup.CloseAllDialog();

		SetQuestSumInfoDlg();
	}

	m_pZoneMapDlg->SetMapIndex( nMapIndex );
	m_MiddleDlgGroup.ShowDialog( ZONEMAP_DIALOG, bShow );	
}

void CDnMainMenuDlg::RefreshZoneMapNPCActor()
{
	if( !m_pZoneMapDlg ) return;
	m_pZoneMapDlg->RefreshNPCActor();
}

void CDnMainMenuDlg::RefreshZoneMapPartyActor()
{
	if( !m_pZoneMapDlg ) return;
	m_pZoneMapDlg->RefreshPartyActor();
}

void CDnMainMenuDlg::RefreshZoneMapFarmPlant()
{
	if( !m_pZoneMapDlg ) return;
	m_pZoneMapDlg->RefreshFarmControl();
}

void CDnMainMenuDlg::ShowQuestDialog( bool bShow, int nChapterIndex, int nQuestIndex, bool bMainQuest )
{
	if( bShow ) {
		m_RightDlgGroup.CloseAllDialog();
		m_LeftDlgGroup.CloseAllDialog();

		SetQuestSumInfoDlg();
	}

#if defined(PRE_ADD_INTEGERATE_QUEST_REPUT)
	m_pIntegrateQuestNReputationDlg->SelectTab(CDnIntegrateQuestNReputationDlg::QUEST_TAB);
	m_MiddleDlgGroup.ShowDialog( InTEGRATE_QUESTNREPUTATION_DIALOG, bShow );
	if( nChapterIndex != -1 ) 
	{
		m_pIntegrateQuestNReputationDlg->ShowQuestFromNotifier( nChapterIndex, nQuestIndex, bMainQuest );
	}
#else
	m_MiddleDlgGroup.ShowDialog( QUEST_DIALOG, bShow );
	if( nChapterIndex != -1 ) {
		m_pQuestTabDlg->ShowQuestFromNotifier( nChapterIndex, nQuestIndex, bMainQuest );
	}
#endif // PRE_ADD_INTEGERATE_QUEST_REPUT
}

void CDnMainMenuDlg::ShowChatRoomCreateDlg( bool bShow )
{
	if( bShow )
	{
		m_RightDlgGroup.CloseAllDialog();
		m_LeftDlgGroup.CloseAllDialog();

		SetQuestSumInfoDlg();
	}

	m_MiddleDlgGroup.ShowDialog( CHATROOM_DIALOG, bShow );
}

void CDnMainMenuDlg::SetChatRoomFlag( bool bShowByHotKey )
{
	if( m_pChatRoomCreateDlg )
		m_pChatRoomCreateDlg->SetChatRoomFlag( bShowByHotKey );
}

#ifdef PRE_ADD_NPC_REPUTATION_SYSTEM
void CDnMainMenuDlg::ShowNpcReputationTabDlg( bool bShow )
{
#if defined(PRE_ADD_INTEGERATE_QUEST_REPUT)
	if( m_bShow )
	{
		m_RightDlgGroup.CloseAllDialog();
		m_LeftDlgGroup.CloseAllDialog();

		SetQuestSumInfoDlg();
	}

	m_pIntegrateQuestNReputationDlg->SelectTab(CDnIntegrateQuestNReputationDlg::REPUTATION_TAB);
	m_MiddleDlgGroup.ShowDialog( InTEGRATE_QUESTNREPUTATION_DIALOG, bShow );
	if (bShow == false)
		m_pIntegrateQuestNReputationDlg->Show(false);
#else
#ifdef PRE_ADD_NO_REPUTATION_DLG
#else
	if( m_bShow )
	{
		m_RightDlgGroup.CloseAllDialog();
		m_LeftDlgGroup.CloseAllDialog();

		SetQuestSumInfoDlg();
	}

	m_MiddleDlgGroup.ShowDialog( NPC_REPUTATION_TAB_DIALOG, bShow );
#endif
#endif // PRE_ADD_INTEGERATE_QUEST_REPUT
}

void CDnMainMenuDlg::UpdateNpcReputationDlg( void )
{
	// Refresh if NPC favorability dialog is currently shown.. though that will rarely happen.
	//if( m_pNpcReputationDlg && m_pNpcReputationDlg->IsShow() )
	//	m_pNpcReputationDlg->RefreshToCurrentPage();
}
#endif // #ifdef PRE_ADD_NPC_REPUTATION_SYSTEM


#ifdef PRE_MOD_MISSION_HELPER
void CDnMainMenuDlg::ShowMissionDialog( MissionInfoStruct *pStruct )
{
	if (CDnPartyTask::IsActive() && GetPartyTask().IsLocalActorEnterGateReady())
		return;

	m_pMissionDlg->SelectMissionItemFromNotifier( pStruct );
	m_MiddleDlgGroup.ShowDialog( MISSION_DIALOG, true );
}
#else
void CDnMainMenuDlg::ShowMissionDialog( bool bDaily, int nMissionItemID )
{
	if (CDnPartyTask::IsActive() && GetPartyTask().IsLocalActorEnterGateReady())
		return;

	m_pMissionDlg->SetCheckedTab( CDnMissionDlg::TabEnum::DailyQuest );
	m_pMissionDlg->GetDailyDlg()->SetCategory( bDaily ? CDnMissionTask::Daily : CDnMissionTask::Weekly );
	m_pMissionDlg->GetDailyDlg()->RefreshSubCategory();
	m_pMissionDlg->GetDailyDlg()->SelectMissionItemFromNotifier( nMissionItemID );
	m_MiddleDlgGroup.ShowDialog( MISSION_DIALOG, true );
}
#endif

void CDnMainMenuDlg::CloseMenuDialog()
{
	if (IsOpenDialog(ITEM_COSTUMEMIX_DIALOG) && CDnItemTask::IsActive())
	{
		CDnItemTask::GetInstance().RequestItemCostumeMixClose();
		return;
	}

	if (IsOpenDialog(ITEM_COSTUME_DMIX_DIALOG) && CDnItemTask::IsActive())
	{
		CDnItemTask::GetInstance().RequestItemCostumeDesignMixClose();
		return;
	}

#ifdef PRE_ADD_COSRANDMIX
	if (IsOpenDialog(ITEM_COSTUME_RANDOMMIX_DIALOG) && CDnItemTask::IsActive())
	{
		CDnItemTask::GetInstance().RequestItemCostumeRandomMixClose();
		return;
	}
#endif
	if (IsOpenDialog(MAIL_DIALOG) && m_pMailDlg && m_pMailDlg->IsLockedDlgs())
		return;

#ifdef PRE_MOD_POTENTIAL_JEWEL_RENEWAL
	if ( GetInterface().GetItemPotentialDlg() )
	{
		CDnPotentialJewelDlg* pJewelDlg = GetInterface().GetItemPotentialDlg();
		if(pJewelDlg->IsShow())
			return;
	}
#endif

	CDnInterface::GetInstance().CloseWorldMsgDialog();

	m_LeftDlgGroup.CloseAllDialog();
	m_MiddleDlgGroup.CloseAllDialog();
	m_RightDlgGroup.CloseAllDialog();

	SetQuestSumInfoDlg();
	GetInterface().CloseNpcTalkReturnDlg();

}

CEtUIDialog *CDnMainMenuDlg::GetMainDialog( DWORD dwDialogID )
{
	CEtUIDialog *pDialog(NULL);

	pDialog = m_LeftDlgGroup.GetDialog( dwDialogID );
	if( pDialog ) return pDialog;

	pDialog = m_MiddleDlgGroup.GetDialog( dwDialogID );
	if( pDialog ) return pDialog;

	pDialog = m_RightDlgGroup.GetDialog( dwDialogID );
	if( pDialog ) return pDialog;

	return NULL;
}


void CDnMainMenuDlg::ToggleShowDialog( DWORD dwDialogID )
{
	bool bShow(true);

#ifdef PRE_ADD_PVP_VILLAGE_ACCESS
	if(GetInterface().IsShowPVPVillageAccessDlg())
		GetInterface().ShowPVPVillageAccessDlg(false);
#endif

#ifdef PRE_ADD_PVP_RANKING
	// Force-close.
	if( dwDialogID != PVP_RANK_DIALOG )
		GetInterface().ClosePvPRankDlg();
#endif // PRE_ADD_PVP_RANKING

	switch( dwDialogID )
	{
	case INVENTORY_DIALOG:
		if( m_pInventory ) {
			bShow = m_pInventory->IsShow();
			m_RightDlgGroup.ShowDialog(dwDialogID, !bShow);
		}
		break;
	case COMMUNITY_DIALOG:
		if( m_pCommunityDlg ) {
			bShow = m_pCommunityDlg->IsShow();
			m_RightDlgGroup.ShowDialog( dwDialogID, !bShow );
		}
		break;
	case CHARSTATUS_DIALOG:
		if( m_pCharStatus ) {
			bShow = m_pCharStatus->IsShow();
			m_LeftDlgGroup.ShowDialog(dwDialogID, !bShow );
#ifdef PRE_MOD_APPELLATIONBOOK_RENEWAL
			CDnMainDlg* pMainBarDlg = GetInterface().GetMainBarDialog();
			if( pMainBarDlg ) 
				pMainBarDlg->ShowAppellationNotify(false);
#endif
		}
		break;
	}

	if( !bShow )		// If already shown above
	{
		ShowSystemDialog( false );
		GetInterface().GetMainBarDialog()->ShowMinimapOption( false );
#ifdef PRE_ADD_AUTO_DICE
		GetInterface().GetMainBarDialog()->ShowAutoDiceOption( false );
#endif
	}

	switch( dwDialogID )
	{
	case SYSTEM_DIALOG:
		bShow = m_pSystemDlg->IsShow();
		ShowSystemDialog( !bShow );
		break;
	case ZONEMAP_DIALOG:
		if( m_pWorldMapDlg->IsShow() ) {
			ShowWorldMapDialog( false );
		}
		else {
			bShow = m_pZoneMapDlg->IsShow();
			if (bShow == false && CDnPartyTask::IsActive() && GetPartyTask().IsLocalActorEnterGateReady())
				break;
			ShowZoneMapDialog( !bShow, -1 );			
		}
		break;
	case MISSION_DIALOG:
		bShow = m_pMissionDlg->IsShow();
		if (bShow == false && CDnPartyTask::IsActive() && GetPartyTask().IsLocalActorEnterGateReady())
			break;

		if( !bShow ) {
			m_RightDlgGroup.CloseAllDialog();
			m_LeftDlgGroup.CloseAllDialog();
			SetQuestSumInfoDlg();
		}
		// Mission window must cover the quest tracker, so call Show below
		m_MiddleDlgGroup.ShowDialog(MISSION_DIALOG, !bShow );
		break;
	case QUEST_DIALOG:
#if defined(PRE_ADD_INTEGERATE_QUEST_REPUT)
		{
			bShow = m_pIntegrateQuestNReputationDlg->IsEventTabShow(CDnIntegrateQuestNReputationDlg::QUEST_TAB);
			if (bShow == false && CDnPartyTask::IsActive() && GetPartyTask().IsLocalActorEnterGateReady())
				break;
			ShowQuestDialog( !bShow );
		}
#else
		bShow = m_pQuestTabDlg->IsShow();
		if (bShow == false && CDnPartyTask::IsActive() && GetPartyTask().IsLocalActorEnterGateReady())
			break;
		ShowQuestDialog( !bShow );
#endif // PRE_ADD_INTEGERATE_QUEST_REPUT
		break;
#ifdef PRE_ADD_NPC_REPUTATION_SYSTEM
	case NPC_REPUTATION_TAB_DIALOG:
#if defined(PRE_ADD_INTEGERATE_QUEST_REPUT)
		{
			bShow = m_pIntegrateQuestNReputationDlg->IsEventTabShow(CDnIntegrateQuestNReputationDlg::REPUTATION_TAB);
			if( bShow == false && CDnPartyTask::IsActive() && GetPartyTask().IsLocalActorEnterGateReady())
				break;
			ShowNpcReputationTabDlg( !bShow );
		}
#else
		//bShow = m_pNpcReputationDlg->IsShow();
		//if( bShow == false && CDnPartyTask::IsActive() && GetPartyTask().IsLocalActorEnterGateReady())
		//	break;
		//ShowNpcReputationTabDlg( !bShow );
		bShow = m_pNpcReputationTabDlg->IsShow();
		if( bShow == false && CDnPartyTask::IsActive() && GetPartyTask().IsLocalActorEnterGateReady())
			break;
		ShowNpcReputationTabDlg( !bShow );
#endif // PRE_ADD_INTEGERATE_QUEST_REPUT
		break;
#endif // #ifdef PRE_ADD_NPC_REPUTATION_SYSTEM

	case SKILL_DIALOG:
		bShow = m_pSkillTreeDlg->IsShow();
		if (bShow == false && CDnPartyTask::IsActive() && GetPartyTask().IsLocalActorEnterGateReady())
			break;

		if( !bShow ) {
			m_RightDlgGroup.CloseAllDialog();
			m_LeftDlgGroup.CloseAllDialog();
			SetQuestSumInfoDlg();
		}
		m_MiddleDlgGroup.ShowDialog(dwDialogID, !bShow);
		break;
	case CHATROOM_DIALOG:
		bShow = m_pChatRoomCreateDlg->IsShow();
		if (bShow == false && CDnPartyTask::IsActive() && GetPartyTask().IsLocalActorEnterGateReady())
			break;
		ShowChatRoomCreateDlg( !bShow );
		break;
	case ITEM_COMPOUND2_DIALOG:
		{
#ifdef PRE_MOD_ITEM_COMPOUND_DIALOG_RENEWAL
			bShow = m_pItemCompoundRenewalDlg->IsShow();
			if( ! bShow ) GetInterface().ForceShowCashShopMenuDlg(false);
#else
			bShow = m_pItemCompound2Dlg->IsShow();
#endif 
			if( !bShow ) 
			{
				m_RightDlgGroup.CloseAllDialog();
				m_LeftDlgGroup.CloseAllDialog();
				SetQuestSumInfoDlg();
			}
			m_MiddleDlgGroup.ShowDialog(dwDialogID, !bShow);
		}
		break;
#if defined(PRE_ADD_INTEGERATE_EVENTUI)
	case INTEGRATE_EVENT_DIALOG:
		{
			bShow = m_pIntegrateEventDlg->IsShow();
			if( bShow == false && CDnTimeEventTask::IsActive() && GetPartyTask().IsLocalActorEnterGateReady() )
				break;
			if( !bShow ) 
			{
				m_RightDlgGroup.CloseAllDialog();
				m_LeftDlgGroup.CloseAllDialog();
				SetQuestSumInfoDlg();
			}
			
			ShowIntegrateEventDialog(!bShow);
		}
		break;
#else
	case TIMEEVENT_DIALOG:
		bShow = m_pTimeEventDlg->IsShow();
		if( bShow == false && CDnTimeEventTask::IsActive() && GetPartyTask().IsLocalActorEnterGateReady() )
			break;
		if( !bShow ) {
			m_RightDlgGroup.CloseAllDialog();
			m_LeftDlgGroup.CloseAllDialog();
			SetQuestSumInfoDlg();
		}
		ShowTimeEventDialog( !bShow );
		break;

	case ATTENDANCEEVENT_DIALOG:
		bShow = m_pAttendanceEventDlg->IsShow();

		if( !bShow ) 
		{
			m_RightDlgGroup.CloseAllDialog();
			m_LeftDlgGroup.CloseAllDialog();
			SetQuestSumInfoDlg();
		}
		ShowAttendanceEventDialog( !bShow );
		break;
#endif // PRE_ADD_INTEGERATE_EVENTUI
#if defined(PRE_ADD_MAILBOX_OPEN)
	case MAIL_DIALOG:
		{
			bShow = m_pMailDlg->IsShow();
			if (!bShow)
			{
				m_RightDlgGroup.CloseAllDialog();
				m_LeftDlgGroup.CloseAllDialog();
			}

			OpenMailDialog( !bShow, _T(""), _T("") );
		}
		break;
#endif // PRE_ADD_MAILBOX_OPEN

//#ifdef PRE_ADD_PVP_RANKING
//	case PVP_RANK_DIALOG:
//		{
//			bShow = GetInterface().IsShowPvPRankDlg();
//			if (!bShow)
//			{
//				m_RightDlgGroup.CloseAllDialog();
//				m_LeftDlgGroup.CloseAllDialog();
//				m_MiddleDlgGroup.CloseAllDialog();
//			}		
//		}
//		break;
//#endif // PRE_ADD_PVP_RANKING

#ifdef PRE_SPECIALBOX
	case EVENT_RECEIVER_DIALOG:
		{
			bShow = m_pEventReceiverTabDlg->IsShow();
			if (!bShow)
			{
				m_LeftDlgGroup.CloseAllDialog();
				SetQuestSumInfoDlg();
			}

			m_LeftDlgGroup.ShowDialog(dwDialogID, !bShow );
		}
		break;
#endif

#if defined( PRE_ALTEIAWORLD_EXPLORE )
	case ALTEA_TAB_DIALOG:
		bShow = m_pAlteaTabDlg->IsShow();
		if (bShow == false && CDnPartyTask::IsActive() && GetPartyTask().IsLocalActorEnterGateReady())
			break;

		if( !bShow ) {
			m_RightDlgGroup.CloseAllDialog();
			m_LeftDlgGroup.CloseAllDialog();
			SetQuestSumInfoDlg();
		}
		
		m_MiddleDlgGroup.ShowDialog(ALTEA_TAB_DIALOG, !bShow );
		break;
#endif	// #if defined( PRE_ALTEIAWORLD_EXPLORE )

#ifdef PRE_ADD_CASHSHOPBANNER_RENEWAL
	case CASHSHOP_INGAMEBANNER_DIALOG:
		if( m_pCashShopIngameBannerDlg )
		{						
			bShow = m_pCashShopIngameBannerDlg->IsShow();
			if( GetInterface().GetInterfaceType() != CDnInterface::Village )
				bShow = true;
			m_pCashShopIngameBannerDlg->Show( !bShow );			
		}
		break;
#endif // PRE_ADD_CASHSHOPBANNER_RENEWAL

	}

	if( !bShow )
	{
		GetInterface().GetMainBarDialog()->ShowMinimapOption( false );
#ifdef PRE_ADD_AUTO_DICE
		GetInterface().GetMainBarDialog()->ShowAutoDiceOption( false );
#endif
		// If the main window is opening, close the inspect window.
		GetInterface().CloseInspectPlayerInfoDialog();

		if( m_MiddleDlgGroup.GetShowDialog() == NULL )
			GetInterface().CloseEventToday();

		CDnInterface::GetInstance().CloseWorldMsgDialog();
	}


#ifdef PRE_ADD_QUEST_BACK_DIALOG_BUTTON
	GetInterface().CloseNpcTalkReturnDlg();
#endif
}

void CDnMainMenuDlg::OnPartyJoin()
{
	m_pCommunityDlg->OnPartyJoin();
}

void CDnMainMenuDlg::OnLevelUp( int nLevel )
{
	if( m_pWorldMapDlg ) {
		m_pWorldMapDlg->OnLevelUp( nLevel );
	}
}

void CDnMainMenuDlg::OnDie()
{
	if( m_pCharStatus ) {
		CEtUIDialog *pDlg = (CEtUIDialog *)m_pCharStatus->GetAppellationDlg();
		if( pDlg ) pDlg->Show( false );
	}
}

void CDnMainMenuDlg::SetPartyCreateDifficulties(const BYTE* pDifficulties)
{
	m_pCommunityDlg->SetPartyCreateDifficulties(pDifficulties);
}

void CDnMainMenuDlg::OnRecvPartyMemberInfo(ePartyType type)
{
	m_pCommunityDlg->OnRecvPartyMemberInfo(type);
}

void CDnMainMenuDlg::OnPartyJoinFailed()
{
	m_pCommunityDlg->OnPartyJoinFailed();
}

void CDnMainMenuDlg::RefreshMasterListDialog()
{
	m_pCommunityDlg->RefreshMasterListDialog();
}

void CDnMainMenuDlg::RefreshMasterSimpleInfo()
{
	m_pCommunityDlg->RefreshMasterSimpleInfo();
}

void CDnMainMenuDlg::RefreshMasterCharacterInfo()
{
	m_pCommunityDlg->RefreshMasterCharacterInfo();
}

void CDnMainMenuDlg::RefreshMasterClassmate()
{
	m_pCommunityDlg->RefreshMasterClassmate();
}

void CDnMainMenuDlg::RefreshPupilList()
{
	m_pCommunityDlg->RefreshPupilList();
}

void CDnMainMenuDlg::RefreshMyMasterInfo()
{
	m_pCommunityDlg->RefreshMyMasterInfo();
}

void CDnMainMenuDlg::RefreshClassmateInfo()
{
	m_pCommunityDlg->RefreshClassmateInfo();
}

void CDnMainMenuDlg::RefreshPupilLeave()
{
	m_pCommunityDlg->RefreshPupilLeave();
}

void CDnMainMenuDlg::RefreshRespectPoint()
{
	m_pCommunityDlg->RefreshRespectPoint();
}

#ifdef PRE_ADD_START_POPUP_QUEUE
void CDnMainMenuDlg::OnEventPopUp(const char* szUrlAddress)
#else
void CDnMainMenuDlg::OnEventPopUp( char* szUrlAddress)
#endif
{
	if (m_pEventTodayDlg)
	{
		m_pEventTodayDlg->ReadEventToday();

		if(m_pEventTodayDlg->GetShowEvent() )
			m_pEventTodayDlg->DisplayEventPopUp (szUrlAddress);
	}
}

#ifdef PRE_ADD_COOKING_SYSTEM
void CDnMainMenuDlg::OpenCookingDialog( bool bClearSelectedRecipe )
{
	bool bShow = m_pDnLifeSkillCookingDlg->IsShow();
	if( bShow == false && CDnPartyTask::IsActive() && GetPartyTask().IsLocalActorEnterGateReady() )
		return;

	if( !bShow )
	{
		m_RightDlgGroup.CloseAllDialog();
		m_LeftDlgGroup.CloseAllDialog();
		SetQuestSumInfoDlg();
	}

	if( bClearSelectedRecipe )
		m_pDnLifeSkillCookingDlg->ClearSelectedRecipe();

	m_pDnLifeSkillCookingDlg->UpdateCookingList();
	m_MiddleDlgGroup.ShowDialog( COOKING_DIALOG, !bShow );
}

void CDnMainMenuDlg::CloseCookingDialog()
{
	bool bShow = m_pDnLifeSkillCookingDlg->IsShow();
	if( bShow == false )
		return;

	m_MiddleDlgGroup.ShowDialog( COOKING_DIALOG, !bShow );
}
#endif // PRE_ADD_COOKING_SYSTEM


void CDnMainMenuDlg::RefreshLifeStorage(SCFarmWareHouseList *pData)
{
	m_pLifeStorageDlg->RefreshLifeStorage(pData);
}

void CDnMainMenuDlg::RequestTakeWareHouseItem( int nSlotIndex )
{
	m_pLifeStorageDlg->RequestTakeWareHouseItem( nSlotIndex );
}

void CDnMainMenuDlg::OnRecvTakeWareHouseItem(INT64 biUniqueID)
{
	m_pLifeStorageDlg->OnRecvTakeWareHouseItem(biUniqueID);
}

bool CDnMainMenuDlg::IsWithDrawGlyphOpen()
{
	if( !m_pCharStatus )
		return false;

	return m_pCharStatus->GetWithDrawOpen();
}

#ifdef PRE_ADD_EQUIPED_ITEM_ENCHANT
void CDnMainMenuDlg::SetEquippedItemUpgradeDlg()
{
	CDnItem* pItem = NULL;
	for( DWORD i = 0; i < EQUIPMAX; i++ ) {
		pItem = GetItemTask().GetEquipItem(i);
		if( pItem )
			m_pItemUpgradeDlg->SetEquippedItem( pItem->GetSlotIndex(), pItem );
	}
}
#endif 

// Return whether it is equipped.
bool CDnMainMenuDlg::IsEquipped( CDnItem * pItem, bool bCash )
{
	if( m_pCharStatus == NULL )
		return false;

	return m_pCharStatus->IsEquipped( pItem, bCash );
}


#ifdef PRE_ADD_COMBINEDSHOP_PERIOD
// Shop Dlg.
bool CDnMainMenuDlg::IsOpenStore()
{	
	if( m_pStoreDlg )
		return m_pStoreDlg->IsShow();

	return false;
}
#endif // PRE_ADD_COMBINEDSHOP_PERIOD

#if defined(PRE_ADD_SERVER_WAREHOUSE)
bool CDnMainMenuDlg::IsOpenStorage()
{
	if( m_pStorageDlg )
		return m_pStorageDlg->IsShow();
	return false;
}
#endif //#if defined(PRE_ADD_SERVER_WAREHOUSE)

//#ifdef PRE_ADD_PVP_RANKING
//void CDnMainMenuDlg::OpenPvPRankDlg()
//{
//	if( m_pPvPRankDlg )
//		m_pPvPRankDlg->Show( !m_pPvPRankDlg->IsShow() );
//}
//#endif // PRE_ADD_PVP_RANKING


#ifdef PRE_ADD_NEWCOMEBACK
void CDnMainMenuDlg::SetComebackAppellation( UINT sessionID, bool bComeback )
{
	if( m_pCommunityDlg )
		m_pCommunityDlg->SetComebackAppellation( sessionID, bComeback );
}
#endif // PRE_ADD_NEWCOMEBACK


#ifdef PRE_ADD_STAMPSYSTEM
class CDnStampEventDlg * CDnMainMenuDlg::GetStampEventDlg()
{	
	if( m_pIntegrateEventDlg ) 
		return m_pIntegrateEventDlg->GetStampEventDlg();

	return NULL;
}

void CDnMainMenuDlg::ShowDailyQuestTab()
{
	ShowQuestDialog( true, -1, -1, false );

#if defined(PRE_ADD_INTEGERATE_QUEST_REPUT)
	m_pIntegrateQuestNReputationDlg->SetCheckDailyTab();
#else
	m_pQuestTabDlg->SetCheckDailyTab();
#endif // PRE_ADD_INTEGERATE_QUEST_REPUT

}

#endif // PRE_ADD_STAMPSYSTEM

#ifdef PRE_ADD_CASHSHOPBANNER_RENEWAL
void CDnMainMenuDlg::ShowIngameBanner( bool bShow )
{
	if( m_pCashShopIngameBannerDlg )
	{
		if( GetInterface().GetInterfaceType() != CDnInterface::Village )
			bShow = false;

		// Open.
		if( bShow )
		{
			if( !m_pCashShopIngameBannerDlg->IsShow() )
				ToggleShowDialog( CASHSHOP_INGAMEBANNER_DIALOG );
		}

		// Close.
		else
			m_pCashShopIngameBannerDlg->Show( bShow );
	}
}
#endif // PRE_ADD_CASHSHOPBANNER_RENEWAL