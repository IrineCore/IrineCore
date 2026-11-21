#include "StdAfx.h"
#include "DnItemTask.h"
#include "DnDebugConsole.h"
#include "DNPacket.h"
#include "DNProtocol.h"
#include "ItemSendPacket.h"
#include "TradeSendPacket.h"
#include "DnCharStatusDlg.h"
#include "DnWeapon.h"
#include "DnParts.h"
#include "DnInvenTabDlg.h"
#include "DnInvenDlg.h"
#include "DnPlayerStorageDlg.h"
#include "DnInterface.h"
#include "DnLocalPlayerActor.h"
#include "DnDropItem.h"
#include "GameSendPacket.h"
#include "DnMainDlg.h"
#include "VillageSendPacket.h"
#include "DnSkillTask.h"
#include "DnLoadingTask.h"
#include "DnMainMenuDlg.h"
#include "DnQuestTask.h"
#include "DnGameTask.h"
#include "TaskManager.h"
#include "DnWorld.h"
#include "DnPlateMainDlg.h"
#include "DnItemUpgradeDlg.h"
#include "DnItemCompoundTabDlg.h"
#include "DnItemCompoundTab2Dlg.h"
#include "DnTableDB.h"
#include "DnTradeTask.h"
#include "DnTradeMail.h"
#include "DnVillageTask.h"
#include "DnCommonTask.h"
#include "DnItemSealDlg.h"
#include "DnItemUnSealDlg.h"
#include "DnFadeInOutDlg.h"
#include "DnMainFrame.h"
#include "DnQuickSlotDlg.h"
#include "DnPvPGameTask.h"
#include "DnCostumeMixDlg.h"
#include "DnCostumeDesignMixDlg.h"
#ifdef PRE_ADD_COSRANDMIX
#include "DnCostumeRandomMixDlg.h"
#endif
#include "DnPotentialJewelDlg.h"

#include "DnGlyph.h"
#if defined(PRE_ADD_TALISMAN_SYSTEM)
#include "DnTalisman.h"
#endif
#include "DnPotentialJewel.h"
#include "DnEnchantJewel.h"
#include "DnEnchantJewelDlg.h"
#include "DnCharRenameDlg.h"
#include "DnCharmItemKeyDlg.h"
#include "DnGuildMarkCreateDlg.h"
#include "DnItemsOwnedAlarm.h"
#include "DnPotenCharmItemProgressDlg.h"
#include "DnPackageBoxResultAutoDlg.h"
#include "DnInputContextMessageBox.h"
#ifdef _USE_VOICECHAT
#include "DNVoiceChatTask.h"
#endif

#include "DnPartsVehicleEffect.h"
#include "DnCharVehicleDlg.h"
#include "DnVehicleTask.h"

#include "DnNestInfoTask.h"
#include "DnAppellationTask.h"
#include "DNGestureTask.h"

#include "DnPetTask.h"
#include "SyncTimer.h"
#include "DnPetActor.h"
#include "DnCharPetDlg.h"

#ifdef PRE_ADD_SECONDARY_SKILL
#include "DnSecondarySkillRecipeItem.h"
#endif // PRE_ADD_SECONDARY_SKILL
#if defined PRE_FIX_MEMOPT_EXT
#include "DnCommonUtil.h"
#endif

#if defined(PRE_ADD_REMOVE_PREFIX)
#include "DnRemovePrefixDlg.h"
#endif // PRE_ADD_REMOVE_PREFIX

#include "DnSkillTreeDlg.h"

#include "DnInspectPlayerDlg.h"

#ifdef PRE_ADD_AUTOUNPACK
#include "DnAutoUnPackDlg.h"
#endif

#ifdef PRE_ADD_EXCHANGE_POTENTIAL
#include "DnPotentialTransferDlg.h"
#endif

#include "DnItemDisjointDlg.h"
#include "DnItemDisjointResultDlg.h"
#ifdef PRE_ADD_CHAOSCUBE
#include "DnChaosCubeDlg.h"
#include "DnChaosGaugeCubeDlg.h"
#endif

#ifdef PRE_ADD_EXCHANGE_ENCHANT
#include "DnItemUpgradeExchangeDlg.h"
#endif
#ifdef PRE_ADD_SERVER_WAREHOUSE
#include "DnPGStorageTabDlg.h"
#include "DnWorldServerStorageInventoryDlg.h"
#endif

#ifdef PRE_MOD_POTENTIAL_JEWEL_RENEWAL
#include "DnPotentialJewelCleanDlg.h"
#endif



#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif 

#ifdef PRE_MOD_ITEM_COMPOUND_DIALOG_RENEWAL
#include "DnItemCompoundRenewalTab.h"
#endif

CDnItemTask::CDnItemTask() : m_pItemCompounder( new CDnItemCompounder ), CTaskListener(true)
{
	m_nCoin = 0;
	m_nCoinStorage = 0;
	m_nCoinGuildWare = 0;
	m_nTakeGuildWareItemCount = 0;
	m_biWithdrawGuildWareCoin = 0;
	m_bReceivedGuildWare = false;

	m_nRebirthCoinCount = 0;
	m_nRebirthCashCoinCount = 0;
	m_nRebirthPCBangCoinCount = 0;
	m_nUsableRebirthCoinCount = -1;
	m_nMaxUsableRebirthCoincount = 0;

	memset( m_pEquip, 0, sizeof(m_pEquip) );
	memset( m_pCashEquip, 0, sizeof(m_pCashEquip) );
	memset( m_pGlyph, 0, sizeof(m_pGlyph) );
#if defined(PRE_ADD_TALISMAN_SYSTEM)
	memset( m_pTalisman, 0, sizeof(m_pTalisman) );
	m_nTalismanOpenFlag = 0;
	m_bCashTalismanOpen = false;
	m_tCashTalismanExpireDate = -1;
#endif
	memset( m_bActiveInventory, 0, sizeof(m_bActiveInventory) );

	m_bRequestWait = false;
	m_nRequestSrcItemSlot = m_nRequestDestItemSlot = -1;
	m_bRequestQuickSlot = true;
	m_bRequestRepair = false;

	m_nRepairSound = m_nMoneySound = m_nItemSortSound = -1;
	m_pItemCompounder->CheckPossibility( false );

	m_pVehicleEquip = NULL;
	m_pVehicleEquipParts = NULL;
	m_pPetEquip = NULL;
	m_pPetEquipParts1 = NULL;
	m_pPetEquipParts2 = NULL;

#ifdef PRE_ADD_GACHA_JAPAN
	m_bSendGachaponReq = false;
	SecureZeroMemory( &m_GachaResultCashItemPacket, sizeof(m_GachaResultCashItemPacket) );
#endif // PRE_ADD_GACHA_JAPAN
	m_nCashInvenCounter = 0;
#ifdef PRE_ADD_SERVER_WAREHOUSE
	m_nWorldServerStorageInvenCounter = 0;
	m_nWorldServerStorageCashInvenCounter = 0;
#endif
#ifdef PRE_ADD_VIP
	m_nVIPRebirthCoinCount = 0;
#endif
	ResetSpecialRebirthItemID();

	m_bLockCostumeSendPacket = false;
	m_bLockCosDesignMixSendPacket = false;
#ifdef PRE_ADD_COSRANDMIX
	m_bLockCosRandomMixSendPacket = false;
#endif

	m_nPetInvenCounter = 0;

#ifdef PRE_ADD_CHANGEJOB_CASHITEM
	m_bSendChangeJobReq = false;
#endif // #ifdef PRE_ADD_CHANGEJOB_CASHITEM

#if defined(PRE_ADD_VIP_FARM) || defined(PRE_ADD_FARM_DOWNSCALE)
	m_tFarmVIPTime = 0;
#endif	//#if defined(PRE_ADD_VIP_FARM) || defined(PRE_ADD_FARM_DOWNSCALE)

#if defined( PRE_USA_FATIGUE )
	m_nNoFatigueExpValue = 0;
	m_nFatigueExpValue = 100;
#endif	// #if defined( PRE_USA_FATIGUE )

	m_pSourceItem = NULL;

#ifdef PRE_MOD_NESTREBIRTH
	m_bOnChangeRebirthCoinOrItem = false;
#endif

#ifdef PRE_ADD_BESTFRIEND
	m_pBestfriendInfo = NULL;
#endif

	m_nCashGlyphCount = 0;
#if defined(PRE_ADD_TALISMAN_SYSTEM)
	m_nTalismanOpenFlag = 0;
	m_bCashTalismanOpen = false;
	m_tCashTalismanExpireDate = -1;
#endif
}

CDnItemTask::~CDnItemTask()
{
	Finalize();
}

bool CDnItemTask::Initialize()
{
	m_pItemCompounder->InitializeTable();

	m_CharInventory.ClearInventory();
	m_QuestInventory.ClearInventory();
	m_CashInventory.ClearInventory();
	m_StorageInventory.ClearInventory();
	m_GuildInventory.ClearInventory();
#ifdef PRE_ADD_CASHREMOVE
	m_CashRemoveInventory.ClearInventory();
#endif
#ifdef PRE_ADD_SERVER_WAREHOUSE
	m_WorldServerStorageInventory.ClearInventory();
	m_WorldServerStorageCashInventory.ClearInventory();
#endif

	m_CharInventory.SetItemTask( this );
	m_QuestInventory.SetItemTask( this );
	m_CashInventory.SetItemTask( this );
	m_QuestInventory.SetInvenType( CDnCharInventory::typeQuestInven );
	m_StorageInventory.SetItemTask( this );
	m_GuildInventory.SetItemTask( this );

#ifdef PRE_ADD_CASHREMOVE
	m_CashRemoveInventory.SetItemTask( this );
#endif

#ifdef PRE_ADD_SERVER_WAREHOUSE
	m_WorldServerStorageInventory.SetItemTask(this);
	m_WorldServerStorageCashInventory.SetItemTask(this);
#endif

	m_PetInventory.ClearInventory();
	m_PetInventory.SetItemTask(this);

	m_nCashGlyphCount = 0;
#if defined(PRE_ADD_TALISMAN_SYSTEM)
	m_nTalismanOpenFlag = 0;
	m_bCashTalismanOpen = false;
	m_tCashTalismanExpireDate = -1;
#endif

	const char *szFileName = CDnTableDB::GetInstance().GetFileName( 10016 );
	if( strlen( szFileName ) > 0 )
		m_nRepairSound = CEtSoundEngine::GetInstance().LoadSound( CEtResourceMng::GetInstance().GetFullName( szFileName ).c_str(), false, false );
	szFileName = CDnTableDB::GetInstance().GetFileName( 10017 );
	if( strlen( szFileName ) > 0 )
		m_nMoneySound = CEtSoundEngine::GetInstance().LoadSound( CEtResourceMng::GetInstance().GetFullName( szFileName ).c_str(), false, false );
	szFileName = CDnTableDB::GetInstance().GetFileName( 10018 );
	if( strlen( szFileName ) > 0 )
		m_nItemSortSound = CEtSoundEngine::GetInstance().LoadSound( CEtResourceMng::GetInstance().GetFullName( szFileName ).c_str(), false, false );
	m_CosMixDataMgr.LoadData();
#ifdef PRE_ADD_COSRANDMIX
	m_CosRandMixDataMgr.LoadData();
#endif
#if defined( PRE_ADD_LIMITED_SHOP )
#if defined( PRE_FIX_74404 )
	m_listLimitedShopItem.clear();
#else // #if defined( PRE_FIX_74404 )
	m_mapLimitedShopItem.clear();
#endif // #if defined( PRE_FIX_74404 )
#endif

	return true;
}


void CDnItemTask::Finalize()
{
#ifdef PRE_ADD_BESTFRIEND
	SAFE_DELETE( m_pBestfriendInfo );
	m_pBestfriendInfo = NULL;
#endif

	SAFE_DELETE( m_pItemCompounder );
	RemoveAllEquipItem();
	RemoveAllCashEquipItem();
	RemoveAllGlyphItem();
#if defined(PRE_ADD_TALISMAN_SYSTEM)
	RemoveAllTalismanItem();
#endif
	RemoveAllVehicleItem();
	RemoveAllPetItem();
	CDnSlotButton::ReleaseIconTexture();

	SAFE_DELETE( m_pSourceItem );

	if( m_nRepairSound != -1 )
		CEtSoundEngine::GetInstance().RemoveSound( m_nRepairSound );
	if( m_nMoneySound != -1 )
		CEtSoundEngine::GetInstance().RemoveSound( m_nMoneySound );
	if( m_nItemSortSound != -1 )
		CEtSoundEngine::GetInstance().RemoveSound( m_nItemSortSound );
}

void CDnItemTask::Process( LOCAL_TIME LocalTime, float fDelta )
{
	m_CharInventory.Process( LocalTime, fDelta );
	m_CashInventory.Process( LocalTime, fDelta );
	m_StorageInventory.Process( LocalTime, fDelta );
	m_GuildInventory.Process( LocalTime, fDelta );
	m_PetInventory.Process( LocalTime , fDelta );

#ifdef PRE_ADD_CASHREMOVE
	m_CashRemoveInventory.Process( LocalTime, fDelta );
#endif

#ifdef PRE_ADD_SERVER_WAREHOUSE
	m_WorldServerStorageInventory.Process(LocalTime, fDelta);
	m_WorldServerStorageCashInventory.Process(LocalTime, fDelta);
#endif

#ifdef PRE_MONITOR_SUPER_NOTE
	m_IAMAMgr.Process(fDelta);
#endif

	CDnSlotButton::ProcessMemoryOptimize( fDelta );
}

void CDnItemTask::OnDisconnectTcp( bool bValidDisconnect )
{
}

void CDnItemTask::OnDisconnectUdp()
{
}

void CDnItemTask::OnDispatchMessage( int nMainCmd, int nSubCmd, char *pData, int nSize )
{
	if( CDnLoadingTask::IsActive() ) {
		if( CDnLoadingTask::GetInstance().InsertLoadPacket( this, nMainCmd, nSubCmd, (void*)pData, nSize ) ) return;
	}
	bool bProcessDispatch = false;
	switch( nMainCmd ) 
	{
		case SC_CHAR: OnRecvCharMessage( nSubCmd, pData, nSize ), bProcessDispatch = true;		break;
		case SC_ITEM: OnRecvItemMessage( nSubCmd, pData, nSize ), bProcessDispatch = true;		break;
		case SC_ITEMGOODS: OnRecvItemGoodsMessage( nSubCmd, pData, nSize ), bProcessDispatch = true; break;
		case SC_ETC: OnRecvEtcMessage( nSubCmd, pData, nSize ), bProcessDispatch = true;		break;
#ifdef PRE_ADD_BESTFRIEND
		case SC_BESTFRIEND: OnRecvBestfriend( nSubCmd, pData, nSize ); bProcessDispatch = true; break;
#endif
	}
	if( bProcessDispatch ) {
		WriteLog( 1, ", Info, CDnItemTask::OnDispatchMessage End : nMainCmd(%d), nSubCmd(%d), nSize(%d)\n", nMainCmd, nSubCmd, nSize );
	}
}

void CDnItemTask::OnRecvItemMessage( int nSubCmd, char *pData, int nSize )
{
	switch( nSubCmd ) {
		case eItem::SC_INVENLIST:		OnRecvItemInventory( (SCInvenList *)pData );			break;
		case eItem::SC_CASHINVENLIST:	OnRecvItemCashInventory( (SCCashInvenList *)pData );	break;
		case eItem::SC_MOVEITEM:		OnRecvItemMove( (SCMoveItem *)pData );					break;
		case eItem::SC_MOVECASHITEM:	OnRecvItemCashMove( (SCMoveCashItem *)pData );			break;
		case eItem::SC_CHANGEEQUIP:		OnRecvItemUserChangeEquip( (SCChangeEquip *)pData );	break;
		case eItem::SC_CHANGECASHEQUIP: OnRecvItemUserChangeCashEquip( (SCChangeCashEquip *)pData );	break;
		case eItem::SC_PICKUP:			OnRecvItemPickup( (SCPickUp *)pData );					break;
		case eItem::SC_REMOVEITEM:		OnRecvItemRemove( (SCRemoveItem *)pData );				break;
		case eItem::SC_REFRESHINVEN:	OnRecvItemRefreshInven( (SCRefreshInven *)pData );		break;
		case eItem::SC_REFRESHCASHINVEN: OnRecvItemRefreshCashInven( (SCRefreshCashInven *)pData ); break;
		case eItem::SC_DROPITEM_MSG:	OnRecvItemDropItemMsg( (SCDropItem *)pData );			break;
		case eItem::SC_CREATE_DROPITEM:	OnRecvItemCreateDropItem( (SCCreateDropItem*)pData );	break;
		case eItem::SC_CREATE_DROPITEMLIST:	OnRecvItemCreateDropItemList( (SCCreateDropItemList*)pData );	break;
		case eItem::SC_WAREHOUSELIST:	OnRecvItemStorage( (SCWarehouseList*)pData );			break;
		case eItem::SC_USEITEM:			OnRecvItemUse( (SCUseItem*)pData );						break;
		case eItem::SC_EMBLEM_COMPOUND_OPEN_RES: OnRecvEmblemCompoundOpenRes( (SCEmblemCompundOpenRes*)pData );	break;
		case eItem::SC_ITEM_COMPOUND_OPEN_RES: OnRecvItemCompoundOpenRes( (SCItemCompoundOpenRes*)pData ); break;
		case eItem::SC_EMBLEM_COMPOUND_RES: OnRecvEmblemCompoundRes( (SCCompoundEmblemRes*)pData ); break;
		case eItem::SC_ITEM_COMPOUND_RES: OnRecvItemCompoundRes( (SCCompoundItemRes*)pData ); break;
		case eItem::SC_EMBLEM_COMPOUND_CANCEL_RES: OnRecvEmblemCompoundCancelRes( (SCEmblemCompoundCancelRes*)pData ); break;
		case eItem::SC_ITEM_COMPOUND_CANCEL_RES: OnRecvItemCompoundCancelRes( (SCItemCompoundCancelRes*)pData ); break;
		case eItem::SC_DISJOINT_RES:	OnRecvItemDisjointRes( (SCItemDisjointRes*)pData ); break;
		case eItem::SC_INVENTORYMAXCOUNT: OnRecvInventoryMaxCount( (SCInventoryMaxCount*)pData ); break;  
		case eItem::SC_WAREHOUSEMAXCOUNT: OnRecvWarehouseMaxCount( (SCWarehouseMaxCount*)pData ); break;
		case eItem::SC_QUESTINVENLIST:		OnRecvQuestItemInventory( (SCQuestInvenList*)pData ); break;
		case eItem::SC_REFRESHQUESTINVEN:	OnRecvQuestItemRefreshInven( (SCRefreshQuestInven*)pData ); break;
		case eItem::SC_REBIRTH_COIN:		OnRecvRebirthCoin( (SCRebirthCoin*)pData );				break;
		case eItem::SC_REFRESHEQUIP:	OnRecvItemRefreshEquip( (SCRefreshEquip*)pData );			break;
		case eItem::SC_REPAIR_EQUIP:	OnRecvItemRepairEquip( (SCRepairResult*)pData );		break;
		case eItem::SC_REPAIR_ALL:		OnRecvItemRepairAll( (SCRepairResult*)pData );			break;
		case eItem::SC_MOVECOIN:		OnRecvItemMoveCoin( (SCMoveCoin*)pData );				break;
		case eItem::SC_MOVEGUILDITEM:	OnRecvItemMoveGuildItem( (SCMoveGuildItem*)pData );		break;
		case eItem::SC_MOVEGUILDCOIN:	OnRecvItemMoveGuildCoin( (SCMoveGuildCoin*)pData );		break;
		case eItem::SC_REFRESH_GUILDITEM:	OnRecvItemRefreshGuildItem( (SCRefreshGuildItem*)pData );	break;
		case eItem::SC_REFRESH_GUILDCOIN:	OnRecvItemRefreshGuildCoin( (SCRefreshGuildCoin*)pData );	break;
		case eItem::SC_REQUEST_RANDOMITEM: OnRecvItemRequestRandomItem( (SCRequestRandomItem*)pData ); break;
		case eItem::SC_COMPLETE_RANDOMITEM: OnRecvItemCompleteRandomItem( (SCCompleteRandomItem*)pData ); break;
		case eItem::SC_ENCHANT:			OnRecvItemEnchant( (SCEnchantItem*)pData );		break;
		case eItem::SC_ENCHANTCOMPLETE:	OnRecvItemEnchantComplete( (SCEnchantComplete*)pData );	break;
		case eItem::SC_ENCHANTCANCEL:	OnRecvItemEnchantCancel( (SCEnchantCancel*)pData );		break;
		case eItem::SC_GLYPHTIMEINFO: OnRecvGlyphSlotCoolTime( (SCGlyphTimeInfo*)pData ); break;
		case eItem::SC_SORTINVENTORY:	OnRecvSortInventory( (SCSortInventory*)pData );			break;
		case eItem::SC_DECREASE_DURABILITY_INVENTORY: OnRecvDecreaseDurabilityInventory( (SCDecreaseDurabilityInventory *)pData ); break;
		case eItem::SC_EQUIPLIST: OnRecvItemEquipList( (SCEquipList *)pData ); break;
		case eItem::SC_BROADCASTINGEFFECT:	OnRecvBroadcastingEffect( (SCBroadcastingEffect*)pData );	break;
		case eItem::SC_CHANGEGLYPH: OnRecvItemUserChangeGlyph((SCChangeGlyph*)pData); break;
		
		case eItem::SC_VEHICLEEQUIPLIST: OnRecvVehicleEquipListMessage((SCVehicleEquipList*)pData); break;
		case eItem::SC_VEHICLEINVENLIST: OnRecvVehicleInvenListMessage((SCVehicleInvenList*)pData); break;
		case eItem::SC_CHANGEVEHICLEPARTS: OnRecvChangeVehiclePartsMessage((SCChangeVehicleParts*)pData); break;
		case eItem::SC_CHANGEVEHICLECOLOR: OnRecvChangeVehicleColorMessagse((SCChangeVehicleColor*)pData); break;
		case eItem::SC_REFRESHVEHICLEINVEN: OnRecvRefreshVehicleInvenMessage((SCRefreshVehicleInven*)pData); break;

		case eItem::SC_CHANGEPETPARTS:	OnRecvChangePetPartsMessage( (SCChangeVehicleParts*)pData ); break;
		case eItem::SC_CHANGEPETBODY:	OnRecvChangePetBodyMessage( (SCChangePetBody*)pData ); break;
		case eItem::SC_PETEQUIPLIST:	OnRecvPetEquipList( (SCVehicleEquipList*)pData ); break;
		case eItem::SC_ADDPETEXP:		OnRecvAddPetExp( (SCAddPetExp*)pData ); break;

#if defined(PRE_ADD_VIP_FARM)
		case eItem::SC_EFFECTITEM_INFO:	OnRecvEffectItemInfo( (SCEffectItemInfo *)pData ); break;
#endif	//#if defined(PRE_ADD_VIP_FARM)
		case eItem::SC_GLYPHEXTENDCOUNT:	OnRecvGlyphExtendCount( (SCGlyphExtendCount*)pData ); break;
		case eItem::SC_GLYPHEXPIREDATA:		OnRecvGlyphExpireData( (SCGlyphExpireData*)pData ); break;
#if defined(PRE_ADD_TALISMAN_SYSTEM)
		case eItem::SC_TALISMAN_OPENSLOT:	OnRecvTalismanOpenSlot( (SCTalismanOpenSlot*)pData ); break;
		case eItem::SC_CHANGETALISMAN:		OnRecvUserChangeTalisman( (SCChangeTalisman*)pData ); break;
		case eItem::SC_TALISMANEXPIREDATA:	OnRecvTalismanExpireData( (SCTalismanExpireData*)pData ); break;
#endif
		case eItem::SC_INCREASELIFE: OnRecvIncreaseLife( (SCIncreaseLife*)pData ); break;
#ifndef PRE_ADD_NAMEDITEM_SYSTEM
		case eItem::SC_USE_SOURCE: OnRecvItemUseSource( (SCSourceItem*)pData ); break;
#endif // #ifndef PRE_ADD_NAMEDITEM_SYSTEM
		case eItem::SC_REFRESHWAREHOUSE:	OnRecvItemRefreshWarehouse( (SCRefreshWarehouse *)pData );		break;
		case eItem::SC_SORTWAREHOUSE:	OnRecvSortWarehouse( (SCSortWarehouse *)pData ); break;
#if defined(PRE_ADD_CASHREMOVE)
		case eItem::SC_REMOVECASH:		OnRecvItemRemoveCash( (SCRemoveCash *)pData ); break;
#endif	// #if defined(PRE_ADD_CASHREMOVE)
#ifdef PRE_ADD_AUTOUNPACK
		case eItem::SC_USE_AUTOUNPACK_CASHITEM:OnRecvAutoUnpack( (SCAutoUnpack *)pData ); break;
#endif
#if defined( PRE_ADD_NAMEDITEM_SYSTEM )
		case eItem::SC_EFFECTSKILLITEM: OnRecvEffectSkillItem( (SCEffectSkillItem *)pData ); break;
		case eItem::SC_DELEFFECTSKILLITEM: OnRecvRemoveEffectSkillItem( (SCDelEffectSkillItem *)pData ); break;
		case eItem::SC_SHOWEFFECT: OnRecvShowEffect( (SCShowEffect *)pData ); break;
		case eItem::SC_SENDNAMEDITEMID: OnRecvRefreshNamedItem( (SCNamedItemID *)pData ); break;
#endif
#if defined(PRE_ADD_EXPUP_ITEM)
		case eItem::SC_DAILY_LIMIT_ITEM : OnRecvDailyItemError( (SCDailyLimitItemError*)pData); break;
#endif
#if defined( PRE_ADD_LIMITED_SHOP )
		case eItem::SC_LIMITEDSHOPITEMDATA : OnRecvLimitedShopItemData( (SCLimitedShopItemData*)pData); break;
#endif
#if defined( PRE_ADD_TRANSFORM_POTION)
		case eItem::SC_CHANGE_TRANSFORM : OnRecvTransformItem( (SCChangeTransform*)pData); break;
#endif
#ifdef PRE_ADD_SERVER_WAREHOUSE
		case eItem::SC_SERVERWARE_LIST : OnRecvItemWorldServerStorageList((SCServerWareList*)pData); break;
		case eItem::SC_CASHSERVERWARE_LIST : OnRecvItemWorldServerCashStorageList((SCCashServerWareList*)pData); break;
		case eItem::SC_MOVE_SERVERWARE: OnRecvItemMoveWorldServerStorage((SCMoveServerWare*)pData); break;
		case eItem::SC_MOVE_CASHSERVERWARE: OnRecvItemMoveWorldServerCashStorage((SCMoveCashServerWare*)pData); break;
#endif
#ifdef PRE_ADD_PVP_EXPUP_ITEM
		case eItem::SC_USE_PVPEXPUP: OnRecvItemPvPExpUp( (SCUsePvPExpup*)pData ); break;
#endif
#ifdef PRE_ADD_EQUIPLOCK
		case eItem::SC_EQUIPITEM_LOCKLIST: OnRecvEquipItemLockList((SCEquipItemLockList*)pData); break;
		case eItem::SC_ITEM_LOCK_RES: OnRecvEquipItemLock((SCItemLockRes*)pData); break;
		case eItem::SC_ITEM_UNLOCK_RES: OnRecvEquipItemUnLock((SCItemLockRes*)pData); break;
#endif
#if defined(PRE_PERIOD_INVENTORY)
		case eItem::SC_PERIODINVENTORY:		OnRecvPeriodInventory( (SCPeriodInventory*)pData ); break;
		case eItem::SC_PERIODWAREHOUSE:		OnRecvPeriodWarehouse( (SCPeriodWarehouse*)pData ); break;
		case eItem::SC_SORTPERIODINVENTORY:	OnRecvSortPeriodInventory( (SCSortInventory*)pData ); break;
		case eItem::SC_SORTPERIODWAREHOUSE:	OnRecvSortPeriodWarehouse( (SCSortWarehouse*)pData ); break;
#endif	// #if defined(PRE_PERIOD_INVENTORY)

#if defined(PRE_ADD_STAGE_USECOUNT_ITEM)
		case eItem::SC_STAGE_USELIMITITEM:	OnRecvStageUseLimitItem( (SCStageUseLimitItem*)pData ); break;
#endif // #if defined(PRE_ADD_STAGE_USECOUNT_ITEM)
		case eItem::SC_SPECIAL_ITEM_LIST:	OnRecvSpecialItemList( (SCSpecialItemList*)pData ); break;
		case eItem::SC_AUTO_UNPACK_RESULT:	OnRecvAutoUnpackResult( (SCAutoUnpackResult*)pData ); break;
		case eItem::SC_DELETE_EXPIRE_ITEM_LIST:	OnRecvDeleteExpireItemList( (SCDeleteExpireItemList*)pData ); break;
	}

	// This must also be called, so do not return in an upper layer.
	if( CDnQuestTask::IsActive() )
		CDnQuestTask::GetInstance().RefreshQuestNotifierInfo();
}

void CDnItemTask::OnRecvItemGoodsMessage( int nSubCmd, char *pData, int nSize )
{
	switch( nSubCmd ) {
		case eItemGoods::SC_CHANGECOLOR:		OnRecvItemUserChangeColor( (SCChangeColor *)pData );	break;
		case eItemGoods::SC_UNSEAL:	OnRecvUnsealItem((SCUnSealItem*)pData); break;
		case eItemGoods::SC_SEAL:	OnRecvSealItem((SCSealItem*)pData); break;
		case eItemGoods::SC_REQUEST_CASHWORLDMSG:OnRecvItemRequestCashWorldMsg( (SCRequestCashWorldMsg*)pData );	break;
#ifdef PRE_ADD_GACHA_JAPAN
		case eItemGoods::SC_JP_GACHAPON_OPEN_SHOP_RES: OnRecvItemGachaOpen_JP( (SCGachaShopOpen_JP*)pData ); break;
		case eItemGoods::SC_JP_GACHAPON_RES: OnRecvItemGachaRes_JP( (SCGachaRes_JP*)pData ); break;
#endif // PRE_ADD_GACHA_JAPAN
		case eItemGoods::SC_COSMIXOPEN:				OnRecvItemCosMixOpen((SCCosMixOpen*)pData); break;
		case eItemGoods::SC_COSMIXCLOSE:			OnRecvItemCosMixClose((SCCosMixClose*)pData); break;
		case eItemGoods::SC_COSMIXCOMPLETECLOSE:	OnRecvItemCosMixCloseComplete((SCCosMixClose*)pData); break;
		case eItemGoods::SC_COSMIXCOMPLETE:			OnRecvItemCosMixComplete((SCCosMix*)pData); break;
		case eItemGoods::SC_DESIGNMIXOPEN:			OnRecvItemCosDesignMixOpen((SCCosDesignMixOpen*)pData); break;
		case eItemGoods::SC_DESIGNMIXCLOSE:			OnRecvItemCosDesignMixClose((SCCosDesignMixClose*)pData); break;
		case eItemGoods::SC_DESIGNMIXCOMPLETECLOSE: OnRecvItemCosDesignMixCloseComplete((SCCosDesignMixClose*)pData); break;
		case eItemGoods::SC_DESIGNMIXCOMPLETE:		OnRecvItemCosDesignMixComplete((SCCosDesignMix*)pData); break;
#ifdef PRE_ADD_COSRANDMIX
		case eItemGoods::SC_RANDOMMIXOPEN:			OnRecvItemCosRandomMixOpen((SCCosRandomMixOpen*)pData); break;
		case eItemGoods::SC_RANDOMMIXCLOSE:			OnRecvItemCosRandomMixClose((SCCosRandomMixClose*)pData); break;
		case eItemGoods::SC_RANDOMMIXCOMPLETECLOSE:	OnRecvItemCosRandomMixCloseComplete((SCCosRandomMixClose*)pData); break;
		case eItemGoods::SC_RANDOMMIXCOMPLETE:		OnRecvItemCosRandomMixComplete((SCCosRandomMix*)pData); break;
#endif
		case eItemGoods::SC_POTENTIALJEWEL:			OnRecvItemPotentialItem( (SCPotentialItem*)pData ); break;
#ifdef PRE_MOD_POTENTIAL_JEWEL_RENEWAL
		case eItemGoods::SC_POTENTIALJEWEL_ROLLBACK: OnRecvItemPotentialItemRollBack( (SCPotentialItem*)pData ); break;
#endif
		case eItemGoods::SC_CHARMITEMREQUEST:		OnRecvItemRequestCharmItem( (SCCharmItemRequest*)pData ); break;
		case eItemGoods::SC_CHARMITEMCOMPLETE:		OnRecvItemCompleteCharmItem( (SCCharmItemComplete*)pData ); break;
		case eItemGoods::SC_PETALTOKENRESULT:		OnRecvItemPetalTokenResult( (SCPetalTokenResult*)pData ); break;
		case eItemGoods::SC_APPELLATIONGAINRESULT:	OnRecvItemAppellationGainResult( (SCAppellationGainResult*)pData ); break;
		case eItemGoods::SC_GUILDRENAME :			OnRecvItemGuildRename( (SCGuildRename*)pData ); break;
		case eItemGoods::SC_CHARACTERRENAME:		OnRecvItemCharacterRename( (SCCharacterRename*)pData ); break;
		case eItemGoods::SC_ENCHANTJEWEL:			OnRecvItemEnchantJewelItem( (SCEnchantJewelItem*)pData ); break;
		case eItemGoods::SC_GUILDMARK:				OnRecvItemGuildMark( (SCGuildMark*)pData ); break;
#ifdef PRE_ADD_CHANGEJOB_CASHITEM
		case eItemGoods::SC_USE_CHANGEJOB_CASHITEM_RES: OnRecvChangeJobCashItem( (SCUseChangeJobCashItemRes*)pData ); break;
#endif // #ifdef PRE_ADD_CHANGEJOB_CASHITEM

#if defined(PRE_ADD_REMOVE_PREFIX)
		case eItemGoods::SC_REMOVE_PREFIX:			OnRecvItemRemovePrefix((SCEnchantJewelItem*)pData); break;
#endif // PRE_ADD_REMOVE_PREFIX
		case eItemGoods::SC_EXPANDSKILLPAGERESULT:	OnRecvItemExpandSkillPage((SCExpandSkillPageItem*)pData); break;
		case eItemGoods::SC_WARP_VILLAGE_LIST: OnRecvWarpVillageList( (SCWarpVillageList*)pData ); break;
		case eItemGoods::SC_WARP_VILLAGE: OnRecvWarpVillageRes( (SCWarpVillage *)pData ); break;
#ifdef PRE_ADD_EXCHANGE_POTENTIAL
		case eItemGoods::SC_EXCHANGE_POTENTIAL:		OnRecvExchangePotential( (SCExchangePotential*)pData ); break;
#endif
#if defined(PRE_ADD_PCBANG_RENTAL_ITEM)
		case eItemGoods::SC_PCCAFE_RENT_ITEM:		OnRecvPcCafeRentItem((SCPcCafeRentItem*)pData); break;
#endif

#ifdef PRE_ADD_CHAOSCUBE
		case eItemGoods::SC_CHAOSCUBEREQUEST: OnRecvChaosCubeRequest( (SCChaosCubeRequest*)pData ); break;
		case eItemGoods::SC_CHAOSCUBECOMPLETE : OnRecvChaosCubeComplete( (SCChaosCubeComplete*)pData ); break;
		case eItemGoods::SC_CHAOSGAUGECUBE_ADDSTUFF: OnRecvChaosGaugeCubeAddStuff( (SCChaosGaugeCubeAddStuff*)pData ); break;
		case eItemGoods::SC_CHAOSGAUGECUBE_COMPLETE: OnRecvChaosGaugeCubeComplete( (SCChaosGaugeCubeComplete*)pData ); break;
		case eItemGoods::SC_CHAOSGAUGECUBE_LIST: OnRecvChaosGaugeCubeList( (SCChaosGaugeCubeList*)pData ); break;
#endif

#ifdef PRE_ADD_BESTFRIEND
		case eItemGoods::SC_BESTFRIENDREQUEST:  OnRecvBestFriendItemReq( (SCBestFriendItemRequest*)pData ); break;
		case eItemGoods::SC_BESTFRIENDCOMPLETE: OnRecvBestFriendItemComp( (SCBestFriendItemComplete*)pData ); break;
#endif
#ifdef PRE_ADD_EXCHANGE_ENCHANT
		case eItemGoods::SC_EXCHANGE_ENCHANT: OnRecvExchangeEnchant((SCExchangeEnchant*)pData); break;
#endif
	}

	// This must also be called, so do not return in an upper layer.
	if( CDnQuestTask::IsActive() )
		CDnQuestTask::GetInstance().RefreshQuestNotifierInfo();
}

void CDnItemTask::OnRecvCharMessage( int nSubCmd, char *pData, int nSize )
{
	switch( nSubCmd ) 
	{
	case eChar::SC_QUICKSLOTLIST:	OnRecvCharQuickSlotList( (SCQuickSlotList*)pData );	break;
//	case eChar::SC_ADDQUICKSLOT:	OnRecvCharAddQuickSlot((SCAddQuickSlot*) pData); break;
//	case eChar::SC_DELQUICKSLOT:	OnRecvCharDelQuickSlot((SCDelQuickSlot*) pData); break;
	case eChar::SC_FATIGUE:			OnRecvCharFatigue( (SCFatigue*)pData ); break;
	}
}

void CDnItemTask::OnRecvEtcMessage( int nSubCmd, char *pData, int nSize )
{
#if defined( PRE_USA_FATIGUE )
	switch( nSubCmd ) 
	{
	case eEtc::eSCEtc::SC_CHANNELINFO_FATIGUEINFO:	OnRecvEtcChannelInfoFatigueInfo( (SCChannelInfoFatigueInfo*)pData ); break;
	}
#endif	// #if defined( PRE_USA_FATIGUE )
}

#if defined( PRE_USA_FATIGUE )
void CDnItemTask::OnRecvEtcChannelInfoFatigueInfo( SCChannelInfoFatigueInfo *pPacket )
{
	m_nNoFatigueExpValue = pPacket->iNoFatigueExpValue;
	m_nFatigueExpValue = pPacket->iFatigueExpValue;
}
#endif	// #if defined( PRE_USA_FATIGUE )

void CDnItemTask::OnRecvItemInventory( SCInvenList *pPacket )
{
	m_CharInventory.ClearInventory();
	GetItemTask().GetCharInventory().SetUsableSlotCount( pPacket->cInvenMaxCount );

	for( int i=0; i<pPacket->cInvenCount; i++ ) 
	{
		m_CharInventory.CreateItem( pPacket->Inven[i] );
	}

#ifdef PRE_ADD_NOTIFY_ITEM_COMPOUND
	CDnMainMenuDlg *pMainMenuDlg = GetInterface().GetMainMenuDialog();
	if( pMainMenuDlg )
	{
		pMainMenuDlg->RefreshCompoundNotifyPercentage();
	}
#endif

}

void CDnItemTask::OnRecvItemCashInventory( SCCashInvenList *pPacket )
{

#ifdef PRE_ADD_CASHREMOVE
	
	__time64_t tBegin = CSyncTimer::GetInstance().GetCurTime(); // Current time.
	
	for( int i=0; i<pPacket->cInvenCount; i++ ) 
	{
		TItem & rItem = pPacket->Inven[i];

		// Add to cash pending tab.
		if( rItem.bExpireComplete )
		{			
			m_CashRemoveInventory.CreateCashItem( rItem, m_CashRemoveInventory.FindFirstEmptyIndex(),
												  tBegin, (rItem.tExpireDate-tBegin), rItem.bExpireComplete ); // Expiration time.
		}

		// Add to cash tab.
		else
			m_CashInventory.CreateCashItem( rItem, m_nCashInvenCounter + i );
	}

	m_nCashInvenCounter += pPacket->cInvenCount;
	if( m_nCashInvenCounter == pPacket->nTotalCashInventoryCount )
		m_nCashInvenCounter = 0;

	if( !m_nCashInvenCounter )
	{
		m_CashInventory.SortCashInventory();
		OnFinishRecvCashInventory();
	}

	OnRefreshCashInventory();

	// #54093 [Cash Delete] After sorting items in the cash delete slot, returning to character select and reconnecting loses the sort order.
	m_CashRemoveInventory.SortCashInventory();

#ifdef PRE_ADD_LEVELUP_REWARD_NOTIFIER
	if( GetExistLevelUpBox( CDnActor::s_hLocalActor->GetLevel() ) )
	{
		CDnMainDlg* pMainBarDlg = GetInterface().GetMainBarDialog();
		if( pMainBarDlg && pMainBarDlg->IsShow() )
		{
			pMainBarDlg->OnLevelUpBoxNotify( true );	
		}
	}
#endif 

#else
	for( int i=0; i<pPacket->cInvenCount; i++ ) 
		m_CashInventory.CreateCashItem( pPacket->Inven[i], m_nCashInvenCounter + i );


	m_nCashInvenCounter += pPacket->cInvenCount;
	if( m_nCashInvenCounter == pPacket->nTotalCashInventoryCount )
		m_nCashInvenCounter = 0;

	if( !m_nCashInvenCounter )
	{
		m_CashInventory.SortCashInventory();
		OnFinishRecvCashInventory();
	}

	OnRefreshCashInventory();
#endif
}

void CDnItemTask::OnRefreshCashInventory()
{
	CDnNestInfoTask::GetInstance().RefreshExpandTryCount();
	GetInterface().RefreshPetFoodCount();
}

void CDnItemTask::OnFinishRecvCashInventory()
{
#ifdef _USE_VOICECHAT
	if( CDnVoiceChatTask::IsActive() ) GetVoiceChatTask().CheckVoiceFont();
#endif
}

void CDnItemTask::OnRecvItemStorage( SCWarehouseList *pPacket )
{
	GetItemTask().SetCoinStorage( pPacket->nWarehouseCoin );

	GetItemTask().GetStorageInventory().SetUsableSlotCount( pPacket->cWareMaxCount );

	for( int i=0; i<pPacket->cWareCount; i++ )
	{
		m_StorageInventory.CreateItem( pPacket->Warehouse[i] );
	}
}

#ifdef PRE_ADD_SERVER_WAREHOUSE
void CDnItemTask::OnRecvItemMoveWorldServerStorage(SCMoveServerWare* pPacket)
{
	m_bRequestWait = false;

	if (pPacket->nRetCode != ERROR_NONE)
	{
		GetInterface().ServerMessageBox(pPacket->nRetCode);
		return;
	}

	if (pPacket->cMoveType == MoveType_InvenToServerWare)
	{
		TItemInfo worldServerStorageItemInfo;
		CDnItem* pServerWareItem = m_WorldServerStorageInventory.FindItemFromSerialID(pPacket->ServerWareItem.nSerial);
		if (pServerWareItem == NULL)
			worldServerStorageItemInfo.cSlotIndex = m_WorldServerStorageInventory.FindFirstEmptyIndex();
		else
			worldServerStorageItemInfo.cSlotIndex = pServerWareItem->GetSlotIndex();
		worldServerStorageItemInfo.Item = pPacket->ServerWareItem;

		m_nRequestDestItemSlot = -1;
		MoveInventoryItem(m_CharInventory, m_WorldServerStorageInventory, pPacket->InvenItem, worldServerStorageItemInfo);

		CDnPGStorageTabDlg* pPGStorageTabDlg = static_cast<CDnPGStorageTabDlg*>(GetInterface().GetMainMenuDialog(CDnMainMenuDlg::PLAYER_GUILD_STORAGE_DIALOG));
		if (pPGStorageTabDlg && pPGStorageTabDlg->IsShow())
		{
			CDnWorldServerStorageInventoryDlg* pWorldServerStorageInvenDlg = static_cast<CDnWorldServerStorageInventoryDlg*>(pPGStorageTabDlg->GetStorageDlg(StorageUIDef::eTAB_WORLDSERVER_NORMAL));
			if( pWorldServerStorageInvenDlg )
				pWorldServerStorageInvenDlg->UpdateCoin();
		}
	}
	else if (pPacket->cMoveType == MoveType_ServerWareToInven)
	{
		TItemInfo worldServerStorageItemInfo;
		CDnItem* pServerWareItem = m_WorldServerStorageInventory.FindItemFromSerialID(pPacket->ServerWareItem.nSerial);
		if (pServerWareItem == NULL)
			worldServerStorageItemInfo.cSlotIndex = m_WorldServerStorageInventory.FindFirstEmptyIndex();
		else
			worldServerStorageItemInfo.cSlotIndex = pServerWareItem->GetSlotIndex();
		worldServerStorageItemInfo.Item = pPacket->ServerWareItem;

		MoveInventoryItem(m_WorldServerStorageInventory, m_CharInventory, worldServerStorageItemInfo, pPacket->InvenItem);

		CDnPGStorageTabDlg* pPGStorageTabDlg = static_cast<CDnPGStorageTabDlg*>(GetInterface().GetMainMenuDialog(CDnMainMenuDlg::PLAYER_GUILD_STORAGE_DIALOG));
		if (pPGStorageTabDlg && pPGStorageTabDlg->IsShow())
		{
			CDnWorldServerStorageInventoryDlg* pWorldServerStorageInvenDlg = static_cast<CDnWorldServerStorageInventoryDlg*>(pPGStorageTabDlg->GetStorageDlg(StorageUIDef::eTAB_WORLDSERVER_NORMAL));
			if( pWorldServerStorageInvenDlg )
				pWorldServerStorageInvenDlg->UpdateCoin();
		}
	}

	m_nRequestSrcItemSlot = m_nRequestDestItemSlot = -1;
	CDnInterface::GetInstance().SetCharStatusSlotEvent();	
	CDnInterface::GetInstance().RefreshPlayerDurabilityIcon();
}

void CDnItemTask::OnRecvItemMoveWorldServerCashStorage(SCMoveCashServerWare* pPacket)
{
	m_bRequestWait = false;

	if (pPacket->nRetCode != ERROR_NONE)
	{
		GetInterface().ServerMessageBox(pPacket->nRetCode);
		return;
	}

	if (pPacket->cMoveType == MoveType_CashToServerWare)
	{
		int nDestItemSlotIndex = m_WorldServerStorageCashInventory.FindFirstEmptyIndex();
		MoveCashInventoryItem(m_CashInventory, m_WorldServerStorageCashInventory, pPacket->SrcItem, m_nRequestSrcItemSlot, pPacket->DestItem, nDestItemSlotIndex);

		CDnPGStorageTabDlg* pPGStorageTabDlg = static_cast<CDnPGStorageTabDlg*>(GetInterface().GetMainMenuDialog(CDnMainMenuDlg::PLAYER_GUILD_STORAGE_DIALOG));
		if (pPGStorageTabDlg && pPGStorageTabDlg->IsShow())
		{
			CDnWorldServerStorageCashInventoryDlg* pWorldServerStorageInvenDlg = static_cast<CDnWorldServerStorageCashInventoryDlg*>(pPGStorageTabDlg->GetStorageDlg(StorageUIDef::eTAB_WORLDSERVER_CASH));
			if( pWorldServerStorageInvenDlg )
				pWorldServerStorageInvenDlg->UpdateCoin();
		}
	}
	else if (pPacket->cMoveType == MoveType_ServerWareToCash)
	{
		int nWorldServerStorageSlotIndex = m_WorldServerStorageCashInventory.FindFirstEmptyIndex();
		int nCashInventorySlotIndex = m_CashInventory.FindFirstEmptyIndex();

		m_nRequestDestItemSlot = -1;
		MoveCashInventoryItem(m_WorldServerStorageCashInventory, m_CashInventory, pPacket->SrcItem, nWorldServerStorageSlotIndex, pPacket->DestItem, nCashInventorySlotIndex);
		CDnPGStorageTabDlg* pPGStorageTabDlg = static_cast<CDnPGStorageTabDlg*>(GetInterface().GetMainMenuDialog(CDnMainMenuDlg::PLAYER_GUILD_STORAGE_DIALOG));
		if (pPGStorageTabDlg && pPGStorageTabDlg->IsShow())
		{
			CDnWorldServerStorageCashInventoryDlg* pWorldServerStorageInvenDlg = static_cast<CDnWorldServerStorageCashInventoryDlg*>(pPGStorageTabDlg->GetStorageDlg(StorageUIDef::eTAB_WORLDSERVER_CASH));
			if( pWorldServerStorageInvenDlg )
				pWorldServerStorageInvenDlg->UpdateCoin();
		}
	}

	m_nRequestSrcItemSlot = m_nRequestDestItemSlot = -1;
	CDnInterface::GetInstance().SetCharStatusSlotEvent();	
	CDnInterface::GetInstance().RefreshPlayerDurabilityIcon();
}
#endif

void CDnItemTask::OnRecvItemOpenGuildWare( SCOpenGuildWare *pPacket )
{
	// Because the server manages versioning,
	// From the client perspective, when sending the list, clear existing inventory and display it,
	// When not clearing the list, just open storage and show existing inventory.

	if( pPacket->nResult == 0 )
	{
		GetItemTask().GetGuildInventory().ClearInventory();	// This also initializes the slot. Calls ReleaseOwnerShip in the destructor.

		// Follow slot assignments, but check for duplicate slots on the client since the data is not completely trustworthy.
		bool bSlot[GUILD_WAREHOUSE_MAX] = {0,};
		memset( bSlot, 0, sizeof(bSlot) );

		// Find items with duplicate slots (e.g., if 0, 2, 2, 3, 3 arrive, index 2 and index 4 are duplicates).
		std::vector<int> vecDuplicatedSlot;
		for( int i = 0; i < pPacket->cWareCount; ++i )
		{
			if( bSlot[pPacket->WareList[i].cSlotIndex] == false )
			{
				bSlot[pPacket->WareList[i].cSlotIndex] = true;
			}
			else
			{
				vecDuplicatedSlot.push_back( i );
			}
		}

		// Fill duplicate item indices from the front (change indices to 3, 0, 4, 1).
		int nFirstEmptySlotIndex = 0;
		for( int i = 0; i < (int)vecDuplicatedSlot.size(); ++i )
		{
			for( int j = 0; j < GUILD_WAREHOUSE_MAX; ++j )
			{
				if( bSlot[j] == false )
				{
					nFirstEmptySlotIndex = j;
					break;
				}
			}
			pPacket->WareList[vecDuplicatedSlot[i]].cSlotIndex = nFirstEmptySlotIndex;
			bSlot[nFirstEmptySlotIndex] = true;
		}

		// Put into inventory according to the reset index and done.
		for( int i = 0; i < pPacket->cWareCount; ++i )
		{
			TItemInfo Info;
			Info.cSlotIndex = pPacket->WareList[i].cSlotIndex;
			Info.Item = pPacket->WareList[i].Item;
			m_GuildInventory.CreateItem( Info );
		}
	}
	else if( pPacket->nResult == 1 )
	{
		// When 1, it used to just open the window and show the current guild storage,
		// Check the slot because items sometimes get stored beyond the slot count.
		m_GuildInventory.CheckInvalidSlot( pPacket->wWareSize );
	}

	// Data received from server is always the latest.
	m_nTakeGuildWareItemCount = pPacket->nTakeWareItemCount;
	m_biWithdrawGuildWareCoin = pPacket->nWithdrawCoin;
	m_bReceivedGuildWare = true;
	SetGuildWareCoin( pPacket->nGuildCoin );

	GetInterface().OpenStorageDialog( 0, true );
}

void CDnItemTask::OnRecvQuestItemInventory(SCQuestInvenList* pPacket)
{
	CDnCharInventory& inven = GetQuestInventory();
	inven.SetUsableSlotCount(QUESTINVENTORYMAX);

	for( int i=0; i< pPacket->cQuestInvenCount ; i++ ) 
	{
		inven.CreateItem( pPacket->QuestInventory[i] );
	}
}
void CDnItemTask::OnRecvQuestItemRefreshInven(SCRefreshQuestInven* pPacket)
{
	CDnCharInventory& inven = GetQuestInventory();
	if( pPacket->ItemInfo.Item.nItemID > 0 )
	{
		CDnItem *pItem = CreateItem( pPacket->ItemInfo );
		inven.InsertItem( pItem );

	}
	else
	{
		inven.RemoveItem( pPacket->ItemInfo.cSlotIndex );
	}
}

void CDnItemTask::OnRecvItemUse( SCUseItem* pPacket )
{
	CDnItem* pUsingItem = NULL;
	switch (pPacket->cInvenType)
	{
	case ITEMPOSITION_INVEN:
		pUsingItem = m_CharInventory.GetItem( pPacket->cInvenIndex );
		break;

	case ITEMPOSITION_CASHINVEN:
		pUsingItem = m_CashInventory.FindItemFromSerialID( pPacket->biInvenSerial );
		break;
	}

	_ASSERT( pUsingItem && "Item use rejected by server." );

	if( pUsingItem )
	{
		bool bUseItemSkill = pUsingItem->ActivateSkillEffect( CDnActor::s_hLocalActor );
		_ASSERT( bUseItemSkill && "Failed to use item skill!" );

		// Handle sound here.
		CEtSoundEngine::GetInstance().PlaySound( "2D", ((CDnItem*)pUsingItem)->GetUseSoundIndex() );

		// TODO: Find items using the same skill ID, insert them as well, and activate cooldown. 
		// Also search items in storage and apply cooldown if same type..
		int nSkillID = pUsingItem->GetSkillID();

		m_CharInventory.BeginSameSkillCoolTime( pUsingItem, nSkillID );
		m_StorageInventory.BeginSameSkillCoolTime( pUsingItem, nSkillID );
		m_CashInventory.BeginSameSkillCoolTime( pUsingItem, nSkillID );

		if( pUsingItem->GetItemType() == ITEMTYPE_EXPAND_SKILLPAGE )
		{
			DnEtcHandle hHandle = CDnInCodeResource::GetInstance().CreatePlayerCommonEffect();
			if( hHandle )
			{
				hHandle->SetPosition( *(CDnActor::s_hLocalActor->GetPosition()) );
				hHandle->SetActionQueue( "BuySkill" );
				EtcObjectSignalStruct *pResult = CDnActor::s_hLocalActor->TSmartPtrSignalImp<DnEtcHandle, EtcObjectSignalStruct>::InsertSignalHandle( -1, -1, hHandle );

				pResult->vOffset = EtVector3( 0.f, 0.f, 0.f );
				pResult->vRotate = EtVector3( 0, 0, 0);
				pResult->bLinkObject = true;
				pResult->bDefendenceParent = true;
			}
		}
	}

	CDnTradeMail& tradeMail = GetTradeTask().GetTradeMail();
	if (tradeMail.IsOnMailMode())
		tradeMail.LockMailDlg(false);
}

void CDnItemTask::OnRecvItemMove( SCMoveItem *pPacket )
{
	CDnItem *pSrcItem = NULL;
	CDnItem *pDestItem = NULL;

	// Note : If moving the item failed, send a message explaining why.
	//
	if( pPacket->nRetCode != ERROR_NONE ) 
	{
		m_nRequestSrcItemSlot = m_nRequestDestItemSlot = -1;
		GetInterface().SetCharStatusSlotEvent();
		GetInterface().ServerMessageBox(pPacket->nRetCode);
		m_bRequestWait = false;
		return;
	}

	if( m_bRequestWait ) {
		m_bRequestWait = false;
		if( m_nRequestSrcItemSlot != pPacket->SrcItem.cSlotIndex ) {
			assert(0&&"Strange");
			m_nRequestSrcItemSlot = m_nRequestDestItemSlot = -1;
			GetInterface().SetCharStatusSlotEvent();
			return;
		}
		if( m_nRequestDestItemSlot != pPacket->DestItem.cSlotIndex ) {
			assert(0&&"Strange");
			m_nRequestSrcItemSlot = m_nRequestDestItemSlot = -1;
			GetInterface().SetCharStatusSlotEvent();
			return;
		}
	}

	m_nRequestSrcItemSlot = pPacket->SrcItem.cSlotIndex;
	m_nRequestDestItemSlot = pPacket->DestItem.cSlotIndex;

	switch( pPacket->cMoveType ) {
		case MoveType_Equip:
			{
				CDnLocalPlayerActor *pActor = (CDnLocalPlayerActor*)CDnActor::s_hLocalActor.GetPointer();
				if( m_nRequestSrcItemSlot != -1 ) {
					if( pActor ) {
						if( m_nRequestSrcItemSlot < EQUIP_WEAPON1 )
							pActor->DetachParts( (CDnParts::PartsTypeEnum)m_nRequestSrcItemSlot );
						else 
							pActor->DetachWeapon( m_nRequestSrcItemSlot - EQUIP_WEAPON1 );
					}

					RemoveEquipItem( m_nRequestSrcItemSlot );
				} 
				if( m_nRequestDestItemSlot != -1 ) {
					if( pActor ) {
						if( m_nRequestDestItemSlot < EQUIP_WEAPON1 )
							pActor->DetachParts( (CDnParts::PartsTypeEnum)m_nRequestDestItemSlot );
						else 
							pActor->DetachWeapon( m_nRequestDestItemSlot - EQUIP_WEAPON1 );
					}

					RemoveEquipItem( m_nRequestDestItemSlot );
				}

				if( pPacket->SrcItem.Item.nItemID > 0 ) {
					pSrcItem = CreateItem( pPacket->SrcItem );
					InsertEquipItem( pSrcItem );

					if( pActor ) {
						if( pPacket->SrcItem.cSlotIndex < EQUIP_WEAPON1 )
							pActor->AttachParts( ((CDnParts*)pSrcItem)->GetMySmartPtr(), (CDnParts::PartsTypeEnum)pPacket->SrcItem.cSlotIndex );
						else
							pActor->AttachWeapon( ((CDnWeapon*)pSrcItem)->GetMySmartPtr(), pPacket->SrcItem.cSlotIndex - EQUIP_WEAPON1 );
					}
				}
				if( pPacket->DestItem.Item.nItemID > 0 ) {
					pDestItem = CreateItem( pPacket->DestItem );
					InsertEquipItem( pDestItem );

					if( pActor ) {
						if( pPacket->DestItem.cSlotIndex < EQUIP_WEAPON1 )
							pActor->AttachParts( ((CDnParts*)pDestItem)->GetMySmartPtr(), (CDnParts::PartsTypeEnum)pPacket->DestItem.cSlotIndex );
						else
							pActor->AttachWeapon( ((CDnWeapon*)pDestItem)->GetMySmartPtr(), pPacket->DestItem.cSlotIndex - EQUIP_WEAPON1 );
					}
				}
				if( pActor ) {
					pActor->CombineParts();
					pActor->RefreshState();
				}
			}
			break;
		case MoveType_EquipToInven:
			{
				CDnLocalPlayerActor *pActor = (CDnLocalPlayerActor*)CDnActor::s_hLocalActor.GetPointer();

				if( m_nRequestSrcItemSlot != -1 ) {
					if( pActor ) {
						if( m_nRequestSrcItemSlot < EQUIP_WEAPON1 )
							pActor->DetachParts( (CDnParts::PartsTypeEnum)m_nRequestSrcItemSlot );
						else 
							pActor->DetachWeapon( m_nRequestSrcItemSlot - EQUIP_WEAPON1 );
					}
					RemoveEquipItem( m_nRequestSrcItemSlot );
				}
				if( m_nRequestDestItemSlot != -1 ) m_CharInventory.RemoveItem( m_nRequestDestItemSlot );

				if( pPacket->SrcItem.Item.nItemID > 0 ) {
					pSrcItem = CreateItem( pPacket->SrcItem );
					InsertEquipItem( pSrcItem );

					if( pActor ) {
						if( pPacket->SrcItem.cSlotIndex < EQUIP_WEAPON1 ) {
							pActor->AttachParts( ((CDnParts*)pSrcItem)->GetMySmartPtr(), (CDnParts::PartsTypeEnum)pPacket->SrcItem.cSlotIndex );
							InsertChangePartsThread( CDnActor::s_hLocalActor );
						}
						else
							pActor->AttachWeapon( ((CDnWeapon*)pSrcItem)->GetMySmartPtr(), pPacket->SrcItem.cSlotIndex - EQUIP_WEAPON1 );
					}
				}
				else {
					if( pActor ) {
						if( (CDnParts::PartsTypeEnum)pPacket->SrcItem.cSlotIndex < EQUIP_WEAPON1 ) {
							InsertChangePartsThread( CDnActor::s_hLocalActor );
						}
					}
				}
				if( pPacket->DestItem.Item.nItemID > 0 ) {
					pDestItem = CreateItem( pPacket->DestItem );
					m_CharInventory.InsertItem( pDestItem, true );
				}
				if( pActor ) {
					pActor->RefreshState();
					CDnInterface::GetInstance().UpdateMyPortrait();
				}
			}
			break;
		case MoveType_InvenToEquip:
			{
				CDnLocalPlayerActor *pActor = (CDnLocalPlayerActor*)CDnActor::s_hLocalActor.GetPointer();

				if( m_nRequestSrcItemSlot != -1 ) m_CharInventory.RemoveItem( m_nRequestSrcItemSlot );
				if( m_nRequestDestItemSlot != -1 ) {
					if( pActor ) {
						if( m_nRequestDestItemSlot < EQUIP_WEAPON1 )
							pActor->DetachParts( (CDnParts::PartsTypeEnum)m_nRequestDestItemSlot );
						else 
							pActor->DetachWeapon( m_nRequestDestItemSlot - EQUIP_WEAPON1 );
					}
					RemoveEquipItem( m_nRequestDestItemSlot );
				}

				if( pPacket->SrcItem.Item.nItemID > 0 ) {
					pSrcItem = CreateItem( pPacket->SrcItem );
					m_CharInventory.InsertItem( pSrcItem, true );
				}

				if( pPacket->DestItem.Item.nItemID > 0 ) {
					pDestItem = CreateItem( pPacket->DestItem );
					InsertEquipItem( pDestItem );

					if( pActor ) {
						if( pPacket->DestItem.cSlotIndex < EQUIP_WEAPON1 ) {
							pActor->AttachParts( ((CDnParts*)pDestItem)->GetMySmartPtr(), (CDnParts::PartsTypeEnum)pPacket->DestItem.cSlotIndex );
							InsertChangePartsThread( CDnActor::s_hLocalActor );
						}
						else
							pActor->AttachWeapon( ((CDnWeapon*)pDestItem)->GetMySmartPtr(), pPacket->DestItem.cSlotIndex - EQUIP_WEAPON1 );
					}

				}
				if( pActor ) {
					pActor->RefreshState();
					CDnInterface::GetInstance().UpdateMyPortrait();
				}
			}
			break;
		case MoveType_Glyph:
			{
				CDnLocalPlayerActor *pActor = (CDnLocalPlayerActor*)CDnActor::s_hLocalActor.GetPointer();
				if( m_nRequestSrcItemSlot != -1 ) {
					if( pActor ) pActor->DetachGlyph( (CDnGlyph::GlyphSlotEnum)m_nRequestSrcItemSlot );

					RemoveGlyphItem( m_nRequestSrcItemSlot );
				}
				if( m_nRequestDestItemSlot != -1 ) {
					if( pActor ) pActor->DetachGlyph( (CDnGlyph::GlyphSlotEnum)m_nRequestDestItemSlot );

					RemoveGlyphItem( m_nRequestDestItemSlot );
				}

				if( pPacket->SrcItem.Item.nItemID > 0 ) {
					pSrcItem = CreateItem( pPacket->SrcItem );
					InsertGlyphItem( pSrcItem );

					if( pActor ) pActor->AttachGlyph( ((CDnGlyph*)pSrcItem)->GetMySmartPtr(), (CDnGlyph::GlyphSlotEnum)pPacket->SrcItem.cSlotIndex );
				}
				if( pPacket->DestItem.Item.nItemID > 0 ) {
					pDestItem = CreateItem( pPacket->DestItem );
					InsertGlyphItem( pDestItem );

					if( pActor ) pActor->AttachGlyph( ((CDnGlyph*)pDestItem)->GetMySmartPtr(), (CDnGlyph::GlyphSlotEnum)pPacket->DestItem.cSlotIndex );
				}
				if( pActor ) {
					pActor->RefreshState();
				}
			}
			break;
		case MoveType_GlyphToInven:
			{
				CDnLocalPlayerActor *pActor = (CDnLocalPlayerActor*)CDnActor::s_hLocalActor.GetPointer();

				if( m_nRequestSrcItemSlot != -1 ) {
					if( pActor ) pActor->DetachGlyph( (CDnGlyph::GlyphSlotEnum)m_nRequestSrcItemSlot );

					RemoveGlyphItem( m_nRequestSrcItemSlot );
				}
				if( m_nRequestDestItemSlot != -1 ) m_CharInventory.RemoveItem( m_nRequestDestItemSlot );

				if( pPacket->SrcItem.Item.nItemID > 0 ) {
					pSrcItem = CreateItem( pPacket->SrcItem );
					InsertGlyphItem( pSrcItem );

					if( pActor ) pActor->AttachGlyph( ((CDnGlyph*)pSrcItem)->GetMySmartPtr(), (CDnGlyph::GlyphSlotEnum)pPacket->SrcItem.cSlotIndex );
				}

				if( pPacket->DestItem.Item.nItemID > 0 ) {
					pDestItem = CreateItem( pPacket->DestItem );
					m_CharInventory.InsertItem( pDestItem, true );
				}
				if( pActor ) {
					pActor->RefreshState();
				}
			}
			break;
		case MoveType_InvenToGlyph:
			{
				CDnLocalPlayerActor *pActor = (CDnLocalPlayerActor*)CDnActor::s_hLocalActor.GetPointer();

				if( m_nRequestSrcItemSlot != -1 ) m_CharInventory.RemoveItem( m_nRequestSrcItemSlot );
				if( m_nRequestDestItemSlot != -1 ) {
					if( pActor ) pActor->DetachGlyph( (CDnGlyph::GlyphSlotEnum)m_nRequestDestItemSlot );

					RemoveGlyphItem( m_nRequestDestItemSlot );
				}

				if( pPacket->SrcItem.Item.nItemID > 0 ) {
					pSrcItem = CreateItem( pPacket->SrcItem );
					m_CharInventory.InsertItem( pSrcItem, true );
				}

				if( pPacket->DestItem.Item.nItemID > 0 ) {
					pDestItem = CreateItem( pPacket->DestItem );
					InsertGlyphItem( pDestItem );

					if( pActor ) pActor->AttachGlyph( ((CDnGlyph*)pDestItem)->GetMySmartPtr(), (CDnGlyph::GlyphSlotEnum)pPacket->DestItem.cSlotIndex );

					CDnItem * pSpecailGlyph = GetItemTask().GetGlyphItem( GLYPH_SPECIALSKILL );

					if( pSpecailGlyph && pSpecailGlyph->GetClassID() == pDestItem->GetClassID() )
						GetInterface().AddChatMessage( CHATTYPE_SYSTEM, NULL, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 723) );
				}
				if( pActor ) {
					pActor->RefreshState();
				}
			}
			break;
#if defined(PRE_ADD_TALISMAN_SYSTEM)
		case MoveType_Talisman:
			{
				CDnLocalPlayerActor *pActor = (CDnLocalPlayerActor*)CDnActor::s_hLocalActor.GetPointer();
				if( m_nRequestSrcItemSlot != -1 ) {
					if( pActor ) pActor->DetachTalisman( m_nRequestSrcItemSlot );
					RemoveTalismanItem( m_nRequestSrcItemSlot );
				}
				if( m_nRequestDestItemSlot != -1 ) {
					if( pActor ) pActor->DetachTalisman( m_nRequestDestItemSlot );
					RemoveTalismanItem( m_nRequestDestItemSlot );
				}

				if( pPacket->SrcItem.Item.nItemID > 0 ) {
					pSrcItem = CreateItem( pPacket->SrcItem );
					InsertTalismanItem( pSrcItem );
					if( pActor ) pActor->AttachTalisman( ((CDnTalisman*)pSrcItem)->GetMySmartPtr(), pPacket->SrcItem.cSlotIndex, GetTalismanEfficiency( pPacket->SrcItem.cSlotIndex ) );
				}
				if( pPacket->DestItem.Item.nItemID > 0 ) {
					pDestItem = CreateItem( pPacket->DestItem );
					InsertTalismanItem( pDestItem );
					if( pActor ) pActor->AttachTalisman( ((CDnTalisman*)pDestItem)->GetMySmartPtr(), pPacket->DestItem.cSlotIndex, GetTalismanEfficiency( pPacket->DestItem.cSlotIndex ) );
				}
				if( pActor ) {
					pActor->RefreshState();
				}

				int nMoveCost = GetTalismanMoveCost();
				if( nMoveCost > 0 )
				{
					int nGold = nMoveCost / 10000;
					int nSilver = ( nMoveCost % 10000 ) / 100;
					int nCopper = nMoveCost % 100;
					std::wstring wszMsg = FormatW( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 8350 ), nGold, nSilver, nCopper );
					GetInterface().AddChatMessage( CHATTYPE_SYSTEM, L"", wszMsg.c_str(), false );
				}
			}
			break;
		case MoveType_TalismanToInven:
			{
				CDnLocalPlayerActor *pActor = (CDnLocalPlayerActor*)CDnActor::s_hLocalActor.GetPointer();
				if( m_nRequestSrcItemSlot != -1 ) {
					if( pActor ) pActor->DetachTalisman( m_nRequestSrcItemSlot );
					RemoveTalismanItem( m_nRequestSrcItemSlot );
				}
				if( m_nRequestDestItemSlot != -1 ) m_CharInventory.RemoveItem( m_nRequestDestItemSlot );

				if( pPacket->SrcItem.Item.nItemID > 0 ) {
					pSrcItem = CreateItem( pPacket->SrcItem );
					InsertTalismanItem( pSrcItem );
					if( pActor ) pActor->AttachTalisman( ((CDnTalisman*)pSrcItem)->GetMySmartPtr(), pPacket->SrcItem.cSlotIndex, GetTalismanEfficiency( pPacket->SrcItem.cSlotIndex ) );
				}
				if( pPacket->DestItem.Item.nItemID > 0 ) {
					pDestItem = CreateItem( pPacket->DestItem );
					m_CharInventory.InsertItem( pDestItem, true );
				}
				if( pActor ) {
					pActor->RefreshState();
				}

				int nMoveCost = GetTalismanMoveCost();
				if( nMoveCost > 0 )
				{
					int nGold = nMoveCost / 10000;
					int nSilver = ( nMoveCost % 10000 ) / 100;
					int nCopper = nMoveCost % 100;
					std::wstring wszMsg = FormatW( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 8350 ), nGold, nSilver, nCopper );
					GetInterface().AddChatMessage( CHATTYPE_SYSTEM, L"", wszMsg.c_str(), false );
				}
			}
			break;
		case MoveType_InvenToTalisman:
			{
				CDnLocalPlayerActor *pActor = (CDnLocalPlayerActor*)CDnActor::s_hLocalActor.GetPointer();
				if( m_nRequestSrcItemSlot != -1 ) m_CharInventory.RemoveItem( m_nRequestSrcItemSlot );
				if( m_nRequestDestItemSlot != -1 ) {
					if( pActor ) pActor->DetachTalisman( m_nRequestDestItemSlot );
					RemoveTalismanItem( m_nRequestDestItemSlot );
				}

				if( pPacket->SrcItem.Item.nItemID > 0 ) {
					pSrcItem = CreateItem( pPacket->SrcItem );
					m_CharInventory.InsertItem( pSrcItem, true );
				}
				if( pPacket->DestItem.Item.nItemID > 0 ) {
					pDestItem = CreateItem( pPacket->DestItem );
					InsertTalismanItem( pDestItem );
					if( pActor ) pActor->AttachTalisman( ((CDnTalisman*)pDestItem)->GetMySmartPtr(), pPacket->DestItem.cSlotIndex, GetTalismanEfficiency( pPacket->DestItem.cSlotIndex ) );
				}
				if( pActor ) {
					pActor->RefreshState();
				}
			}
			break;
#endif	// #if defined(PRE_ADD_TALISMAN_SYSTEM)
		case MoveType_Inven:
			{
				MoveInventoryItem( m_CharInventory, m_CharInventory, pPacket->SrcItem, pPacket->DestItem );
			}
			break;
		case MoveType_QuestInven:
			{
				MoveInventoryItem( m_QuestInventory, m_QuestInventory, pPacket->SrcItem, pPacket->DestItem );
			}
			break;
		case MoveType_InvenToWare: // Note : Inventory to Storage
			{
				MoveInventoryItem( m_CharInventory, m_StorageInventory, pPacket->SrcItem, pPacket->DestItem );
			}
			break;
		case MoveType_InvenToGuildWare:
			{
				MoveInventoryItem( m_CharInventory, m_GuildInventory, pPacket->SrcItem, pPacket->DestItem );
			}
			break;
		case MoveType_Ware:	// Note: Storage to/from Storage
			{
				MoveInventoryItem( m_StorageInventory, m_StorageInventory, pPacket->SrcItem, pPacket->DestItem );
			}
			break;
		case MoveType_WareToInven:	// Note: Storage to Inventory
			{
				MoveInventoryItem( m_StorageInventory, m_CharInventory, pPacket->SrcItem, pPacket->DestItem );
			}
			break;
		case MoveType_GuildWare:			// GUILDINVEN <-> GUILDINVEN
			{
				MoveInventoryItem( m_GuildInventory, m_GuildInventory, pPacket->SrcItem, pPacket->DestItem );
			}
			break;
		case MoveType_GuildWareToInven:		// GUILDINVEN -> INVEN
			{
				MoveInventoryItem( m_GuildInventory, m_CharInventory, pPacket->SrcItem, pPacket->DestItem );
			}
			break;
	}

	m_nRequestSrcItemSlot = m_nRequestDestItemSlot = -1;
	CDnInterface::GetInstance().SetCharStatusSlotEvent();	
	CDnInterface::GetInstance().RefreshPlayerDurabilityIcon();

	PlayItemMoveSound( pPacket );
}

void CDnItemTask::OnRecvItemMoveGuildItem( SCMoveGuildItem *pPacket )
{
	CDnItem *pSrcItem = NULL;
	CDnItem *pDestItem = NULL;

	// Note : If moving the item failed, send a message explaining why.
	//
	if( pPacket->nRetCode != ERROR_NONE ) 
	{
		// #40935 Guild storage DB error; too risky to overhaul in live service, so handle it on the client.
		if( pPacket->nRetCode == 103255 )
		{
			drag::Command(UI_DRAG_CMD_CANCEL);
			drag::ReleaseControl();
		}

		m_nRequestSrcItemSlot = m_nRequestDestItemSlot = -1;
		GetInterface().SetCharStatusSlotEvent();
		((CDnInvenTabDlg *)GetInterface().GetMainMenuDialog(CDnMainMenuDlg::INVENTORY_DIALOG))->DisablePressedButtonSplitMode();
		GetInterface().ServerMessageBox(pPacket->nRetCode);
		m_bRequestWait = false;
		return;
	}

	if( m_bRequestWait )
	{
		m_bRequestWait = false;

		// If the request was processed without issues, we probably wouldn't need to find the index by serial,
		// Handle the guild side first since its structure differs slightly from normal inventory.
		// Might not even be needed later.
		CDnItem *pTempItem = NULL;
		bool bCheckDestSerial = false;
		bool bCheckSrcSerial = false;
		switch( pPacket->cMoveType )
		{
		case MoveType_InvenToGuildWare:	bCheckDestSerial = true; break;
		case MoveType_GuildWare:		bCheckDestSerial = true; bCheckSrcSerial = true; break;
		case MoveType_GuildWareToInven:	bCheckSrcSerial = true; break;
		}

		if( bCheckDestSerial )
		{
			if( pPacket->biDestSerial > 0 )
			{
				pTempItem = m_GuildInventory.FindItemFromSerialID( pPacket->biDestSerial );
				if( pTempItem )
				{
					if( pPacket->DestItem.cSlotIndex == pTempItem->GetSlotIndex() )
					{
						// Since the server does not know the slot index, it will rarely enter here.
					}
					else
					{
						_ASSERT(0&&"Slot mismatch occurred when moving guild item GuildWareToInven.");
						pPacket->DestItem.cSlotIndex = pTempItem->GetSlotIndex();
					}
				}
				else
				{
					// If DestSerial exists, look up the item; if it doesn't exist, find an empty slot and place it there (this should rarely happen).
					// The function below cannot check blocked slots.
					// So as below, we need to either get it from the guild task or get the available slot count from the dialog,
					pPacket->DestItem.cSlotIndex = m_GuildInventory.FindFirstEmptyIndex();
				}
			}
			else
			{
				pTempItem = m_GuildInventory.GetItem( pPacket->DestItem.cSlotIndex );
				if( pTempItem && pTempItem->GetSerialID() != pPacket->DestItem.Item.nSerial )
				{
					// If there is already an item in the target slot,
					pPacket->DestItem.cSlotIndex = m_GuildInventory.FindFirstEmptyIndex();
				}
			}
		}

		if( bCheckSrcSerial )
		{
			if( pPacket->biSrcSerial > 0 )
			{
				pTempItem = m_GuildInventory.FindItemFromSerialID( pPacket->biSrcSerial );
				if( pTempItem )
				{
					if( pPacket->SrcItem.cSlotIndex == pTempItem->GetSlotIndex() )
					{
						// Should normally be equal.
					}
					else
					{
						_ASSERT(0&&"Slot mismatch occurred when moving guild item GuildWareToInven.");
						pPacket->SrcItem.cSlotIndex = pTempItem->GetSlotIndex();
					}
				}
			}
		}
	}

	m_nRequestSrcItemSlot = pPacket->SrcItem.cSlotIndex;
	m_nRequestDestItemSlot = pPacket->DestItem.cSlotIndex;

	switch( pPacket->cMoveType ) {
		case MoveType_InvenToGuildWare:
			{
				MoveInventoryItem( m_CharInventory, m_GuildInventory, pPacket->SrcItem, pPacket->DestItem );
			}
			break;
		case MoveType_GuildWare:
			{
				MoveInventoryItem( m_GuildInventory, m_GuildInventory, pPacket->SrcItem, pPacket->DestItem );
			}
			break;
		case MoveType_GuildWareToInven:
			{
				MoveInventoryItem( m_GuildInventory, m_CharInventory, pPacket->SrcItem, pPacket->DestItem );
			}
			break;
	}

	m_nRequestSrcItemSlot = m_nRequestDestItemSlot = -1;
	CDnInterface::GetInstance().SetCharStatusSlotEvent();

	if( pPacket->cMoveType == MoveType_GuildWareToInven )
		m_nTakeGuildWareItemCount = pPacket->nTakeItemCount;

	SCMoveItem MoveItemPacket;
	MoveItemPacket.cMoveType = pPacket->cMoveType;
	MoveItemPacket.nRetCode = 0;
	MoveItemPacket.SrcItem = pPacket->SrcItem;
	MoveItemPacket.DestItem = pPacket->DestItem;
	PlayItemMoveSound( &MoveItemPacket );
}

void CDnItemTask::OnRecvItemRefreshGuildItem( SCRefreshGuildItem *pPacket )
{
	// Receive another guild member's item move handling as is.
	// If the guild storage window data is not up to date, just return.
	if( !m_bReceivedGuildWare ) return;

	CDnItem *pSrcItem = NULL;
	CDnItem *pDestItem = NULL;

	CDnItem *pTempItem = NULL;
	bool bCheckDestSerial = false;
	bool bCheckSrcSerial = false;
	switch( pPacket->cMoveType )
	{
	case MoveType_InvenToGuildWare:	bCheckDestSerial = true; break;
	case MoveType_GuildWare:		bCheckDestSerial = true; bCheckSrcSerial = true; break;
	case MoveType_GuildWareToInven:	bCheckSrcSerial = true; break;
#ifdef PRE_ADD_JOINGUILD_SUPPORT
	case MoveType_MailToGuildWare:	bCheckDestSerial = true; bCheckSrcSerial = false; break;
#endif
	}

	if( bCheckDestSerial )
	{
		if( pPacket->biDestSerial > 0 )
		{
			pTempItem = m_GuildInventory.FindItemFromSerialID( pPacket->biDestSerial );
			if( pTempItem )
			{
				if( pPacket->DestItem.cSlotIndex == pTempItem->GetSlotIndex() )
				{
					// Since the server does not know the slot index, it will rarely enter here.
				}
				else
				{
					_ASSERT(0&&"Slot mismatch occurred when moving guild item GuildWareToInven.");
					pPacket->DestItem.cSlotIndex = pTempItem->GetSlotIndex();
				}
			}
			else
			{
				// If DestSerial exists, look up the item; if it doesn't exist, find an empty slot and place it there (this should rarely happen).
				// The function below cannot check blocked slots.
				// So as below, we need to either get it from the guild task or get the available slot count from the dialog,
				pPacket->DestItem.cSlotIndex = m_GuildInventory.FindFirstEmptyIndex();
			}
		}
		else
		{
			pTempItem = m_GuildInventory.GetItem( pPacket->DestItem.cSlotIndex );
			if( pTempItem && pTempItem->GetSerialID() != pPacket->DestItem.Item.nSerial )
			{
				// If there is already an item in the target slot,
				pPacket->DestItem.cSlotIndex = m_GuildInventory.FindFirstEmptyIndex();
			}
		}
	}

	if( bCheckSrcSerial )
	{
		if( pPacket->biSrcSerial > 0 )
		{
			pTempItem = m_GuildInventory.FindItemFromSerialID( pPacket->biSrcSerial );
			if( pTempItem )
			{
				if( pPacket->SrcItem.cSlotIndex == pTempItem->GetSlotIndex() )
				{
					// Should normally be equal.
				}
				else
				{
					_ASSERT(0&&"Slot mismatch occurred when moving guild item GuildWareToInven.");
					pPacket->SrcItem.cSlotIndex = pTempItem->GetSlotIndex();
				}
			}
		}
	}

	// Source item processing must precede destination item processing.
	if( bCheckSrcSerial )
	{
		if( pPacket->SrcItem.cSlotIndex != -1 ) m_GuildInventory.RemoveItem( pPacket->SrcItem.cSlotIndex );
		if( pPacket->SrcItem.Item.nItemID > 0 )
		{
			CDnItem *pSrcItem = CreateItem( pPacket->SrcItem );
			m_GuildInventory.InsertItem( pSrcItem, true );
		}
	}

	if( bCheckDestSerial )
	{
		if( pPacket->DestItem.cSlotIndex != -1 ) m_GuildInventory.RemoveItem( pPacket->DestItem.cSlotIndex );
		if( pPacket->DestItem.Item.nItemID > 0 )
		{
			CDnItem *pDestItem = CreateItem( pPacket->DestItem );
			m_GuildInventory.InsertItem( pDestItem, true );
		}
	}

	if( drag::IsValid() )
	{
		CDnSlotButton *pDragButton = ( CDnSlotButton * )drag::GetControl();
		pDragButton->DisableSplitMode( true );
		drag::ReleaseControl();
	}
}

void CDnItemTask::OnRecvItemCashMove( SCMoveCashItem *pPacket )
{
	CDnItem *pSrcItem = NULL;
	CDnItem *pDestItem = NULL;


	// Note : If moving the item failed, send a message explaining why.
	//
	if( pPacket->nRetCode != ERROR_NONE ) 
	{
		m_nRequestSrcItemSlot = m_nRequestDestItemSlot = -1;
		GetInterface().SetCharStatusSlotEvent();
		GetInterface().ServerMessageBox(pPacket->nRetCode);
		m_bRequestWait = false;
		return;
	}

	if( m_bRequestWait ) {
		m_bRequestWait = false;
		if( pPacket->cMoveType == MoveType_CashEquipToCashInven )
		{
			if( m_nRequestSrcItemSlot != pPacket->CashEquipItem.cSlotIndex )
			{
				assert(0&&"Strange");
				m_nRequestSrcItemSlot = m_nRequestDestItemSlot = -1;
				GetInterface().SetCharStatusSlotEvent();
				return;
			}
		}
		else if( pPacket->cMoveType == MoveType_CashInvenToCashEquip )
		{
			if( m_nRequestDestItemSlot != pPacket->CashEquipItem.cSlotIndex )
			{
				assert(0&&"Strange");
				m_nRequestSrcItemSlot = m_nRequestDestItemSlot = -1;
				GetInterface().SetCharStatusSlotEvent();
				return;
			}
		}
		else if(pPacket->cMoveType == MoveType_VehicleBodyToVehicleInven)
		{
			if( m_nRequestSrcItemSlot != pPacket->CashEquipItem.cSlotIndex )
			{
				assert(0&&"Strange");
				m_nRequestSrcItemSlot = m_nRequestDestItemSlot = -1;
				GetInterface().SetCharStatusSlotEvent();
				return;
			}
		}
		else if(pPacket->cMoveType == MoveType_VehicleInvenToVehicleBody)
		{
			if( m_nRequestDestItemSlot != pPacket->CashEquipItem.cSlotIndex )
			{
				assert(0&&"Strange");
				m_nRequestSrcItemSlot = m_nRequestDestItemSlot = -1;
				GetInterface().SetCharStatusSlotEvent();
				return;
			}
		}
		else if( pPacket->cMoveType == MoveType_CashInvenToVehicleParts )
		{
			if( m_nRequestDestItemSlot != pPacket->CashEquipItem.cSlotIndex )
			{
				assert(0&&"Strange");
				m_nRequestSrcItemSlot = m_nRequestDestItemSlot = -1;
				GetInterface().SetCharStatusSlotEvent();
				return;
			}
		}
		else if( pPacket->cMoveType == MoveType_VehiclePartsToCashInven )
		{
			if( m_nRequestSrcItemSlot != pPacket->CashEquipItem.cSlotIndex )
			{
				assert(0&&"Strange");
				m_nRequestSrcItemSlot = m_nRequestDestItemSlot = -1;
				GetInterface().SetCharStatusSlotEvent();
				return;
			}
		}
		else if( pPacket->cMoveType == MoveType_PetBodyToPetInven )
		{
			if( m_nRequestSrcItemSlot != pPacket->CashEquipItem.cSlotIndex )
			{
				assert(0&&"Strange");
				m_nRequestSrcItemSlot = m_nRequestDestItemSlot = -1;
				GetInterface().SetCharStatusSlotEvent();
				return;
			}
		}
		else if( pPacket->cMoveType == MoveType_PetInvenToPetBody )
		{
			if( m_nRequestDestItemSlot != pPacket->CashEquipItem.cSlotIndex )
			{
				assert(0&&"Strange");
				m_nRequestSrcItemSlot = m_nRequestDestItemSlot = -1;
				GetInterface().SetCharStatusSlotEvent();
				return;
			}
		}
		else if( pPacket->cMoveType == MoveType_CashInvenToPetParts )
		{
			if( m_nRequestDestItemSlot != pPacket->CashEquipItem.cSlotIndex )
			{
				assert(0&&"Strange");
				m_nRequestSrcItemSlot = m_nRequestDestItemSlot = -1;
				GetInterface().SetCharStatusSlotEvent();
				return;
			}
		}
		else if( pPacket->cMoveType == MoveType_PetPartsToCashInven )
		{
			if( m_nRequestSrcItemSlot != pPacket->CashEquipItem.cSlotIndex )
			{
				assert(0&&"Strange");
				m_nRequestSrcItemSlot = m_nRequestDestItemSlot = -1;
				GetInterface().SetCharStatusSlotEvent();
				return;
			}
		}
	}

	if( pPacket->cMoveType == MoveType_CashEquipToCashInven )
	{
		m_nRequestSrcItemSlot = pPacket->CashEquipItem.cSlotIndex;
		m_nRequestDestItemSlot = m_CashInventory.FindFirstEmptyIndex();
	}
	else if( pPacket->cMoveType == MoveType_CashInvenToCashEquip )
	{
		// The position of the item to equip is the slot position for InvenToEquip.
		CDnItem *pInvenCashItem = m_CashInventory.FindItemFromSerialID( pPacket->CashEquipItem.Item.nSerial );
		m_nRequestSrcItemSlot = pInvenCashItem ? pInvenCashItem->GetSlotIndex() : m_CashInventory.FindFirstEmptyIndex();
		m_nRequestDestItemSlot = pPacket->CashEquipItem.cSlotIndex;
	}
	else if(pPacket->cMoveType == MoveType_VehicleBodyToVehicleInven)
	{
		m_nRequestSrcItemSlot = pPacket->CashEquipItem.cSlotIndex;
		m_nRequestDestItemSlot = m_PetInventory.FindFirstEmptyIndex();
	}
	else if(pPacket->cMoveType == MoveType_VehicleInvenToVehicleBody)
	{
		// The position of the item to equip is the slot position for InvenToEquip.
		CDnItem *pInvenPetItem = m_PetInventory.FindItemFromSerialID( pPacket->CashEquipItem.Item.nSerial );
		m_nRequestSrcItemSlot = pInvenPetItem ? pInvenPetItem->GetSlotIndex() : m_PetInventory.FindFirstEmptyIndex();
		m_nRequestDestItemSlot = pPacket->CashEquipItem.cSlotIndex;
	}
	else if(pPacket->cMoveType == MoveType_VehiclePartsToCashInven )
	{
		m_nRequestSrcItemSlot = pPacket->CashEquipItem.cSlotIndex;
		m_nRequestDestItemSlot = m_CashInventory.FindFirstEmptyIndex();
	}
	else if(pPacket->cMoveType == MoveType_CashInvenToVehicleParts)
	{
		// The position of the item to equip is the slot position for InvenToEquip.
		CDnItem *pInvenCashItem = m_CashInventory.FindItemFromSerialID( pPacket->CashEquipItem.Item.nSerial );
		m_nRequestSrcItemSlot = pInvenCashItem ? pInvenCashItem->GetSlotIndex() : m_CashInventory.FindFirstEmptyIndex();
		m_nRequestDestItemSlot = pPacket->CashEquipItem.cSlotIndex;
	}
	else if( pPacket->cMoveType == MoveType_PetBodyToPetInven )
	{
		m_nRequestSrcItemSlot = pPacket->CashEquipItem.cSlotIndex;
		m_nRequestDestItemSlot = m_PetInventory.FindFirstEmptyIndex();
	}
	else if( pPacket->cMoveType == MoveType_PetInvenToPetBody )
	{
		// The position of the item to equip is the slot position for InvenToEquip.
		CDnItem *pInvenPetItem = m_PetInventory.FindItemFromSerialID( pPacket->CashEquipItem.Item.nSerial );
		m_nRequestSrcItemSlot = pInvenPetItem ? pInvenPetItem->GetSlotIndex() : m_PetInventory.FindFirstEmptyIndex();
		m_nRequestDestItemSlot = pPacket->CashEquipItem.cSlotIndex;
	}
	else if( pPacket->cMoveType == MoveType_PetPartsToCashInven )
	{
		m_nRequestSrcItemSlot = pPacket->CashEquipItem.cSlotIndex;
		m_nRequestDestItemSlot = m_CashInventory.FindFirstEmptyIndex();
	}
	else if( pPacket->cMoveType == MoveType_CashInvenToPetParts )
	{
		// The position of the item to equip is the slot position for InvenToEquip.
		CDnItem *pInvenCashItem = m_CashInventory.FindItemFromSerialID( pPacket->CashEquipItem.Item.nSerial );
		m_nRequestSrcItemSlot = pInvenCashItem ? pInvenCashItem->GetSlotIndex() : m_CashInventory.FindFirstEmptyIndex();
		m_nRequestDestItemSlot = pPacket->CashEquipItem.cSlotIndex;
	}

	switch( pPacket->cMoveType ) {
		case MoveType_CashEquip:
			{
				// Currently MoveType_CashEquip is not allowed.
				/*
				CDnLocalPlayerActor *pActor = (CDnLocalPlayerActor*)CDnActor::s_hLocalActor.GetPointer();
				if( m_nRequestSrcItemSlot != -1 ) {
					if( pActor ) {
						if( m_nRequestSrcItemSlot < CASHEQUIP_WEAPON1 )
							pActor->DetachCashParts( (CDnParts::PartsTypeEnum)m_nRequestSrcItemSlot );
						else 
							pActor->DetachCashWeapon( m_nRequestSrcItemSlot - CASHEQUIP_WEAPON1 );
					}

					RemoveCashEquipItem( m_nRequestSrcItemSlot );
				}
				if( m_nRequestDestItemSlot != -1 ) {
					if( pActor ) {
						if( m_nRequestDestItemSlot < CASHEQUIP_WEAPON1 )
							pActor->DetachCashParts( (CDnParts::PartsTypeEnum)m_nRequestDestItemSlot );
						else 
							pActor->DetachCashWeapon( m_nRequestDestItemSlot - CASHEQUIP_WEAPON1 );
					}

					RemoveCashEquipItem( m_nRequestDestItemSlot );
				}

				if( pPacket->SrcItem.Item.nItemID > 0 ) {
					pSrcItem = CreateItem( pPacket->SrcItem );
					InsertCashEquipItem( pSrcItem );

					if( pActor ) {
						if( pPacket->SrcItem.cSlotIndex < CASHEQUIP_WEAPON1 )
							pActor->AttachCashParts( ((CDnParts*)pSrcItem)->GetMySmartPtr(), (CDnParts::PartsTypeEnum)pPacket->SrcItem.cSlotIndex );
						else
							pActor->AttachCashWeapon( ((CDnWeapon*)pSrcItem)->GetMySmartPtr(), pPacket->SrcItem.cSlotIndex - CASHEQUIP_WEAPON1 );
					}
				}
				if( pPacket->DestItem.Item.nItemID > 0 ) {
					pDestItem = CreateItem( pPacket->DestItem );
					InsertCashEquipItem( pDestItem );

					if( pActor ) {
						if( pPacket->DestItem.cSlotIndex < CASHEQUIP_WEAPON1 )
							pActor->AttachCashParts( ((CDnParts*)pDestItem)->GetMySmartPtr(), (CDnParts::PartsTypeEnum)pPacket->DestItem.cSlotIndex );
						else
							pActor->AttachCashWeapon( ((CDnWeapon*)pDestItem)->GetMySmartPtr(), pPacket->DestItem.cSlotIndex - CASHEQUIP_WEAPON1 );
					}
				}
				pActor->CombineParts();
				pActor->RefreshState();
				*/
			}
			break;
		case MoveType_CashEquipToCashInven:
			{
				CDnLocalPlayerActor *pActor = (CDnLocalPlayerActor*)CDnActor::s_hLocalActor.GetPointer();

				if( m_nRequestSrcItemSlot != -1 ) {

					// Local character does not need a lock.
					//ScopeLock< CSyncLock > Lock( pActor->GetMemberLock() );				

					if( pActor ) {
						if( m_nRequestSrcItemSlot < CASHEQUIP_WEAPON1 )
							pActor->DetachCashParts( (CDnParts::PartsTypeEnum)m_nRequestSrcItemSlot );
						else if(m_nRequestSrcItemSlot == CASHEQUIP_WEAPON1 || m_nRequestSrcItemSlot == CASHEQUIP_WEAPON2)
						{
							pActor->DetachCashWeapon( m_nRequestSrcItemSlot - CASHEQUIP_WEAPON1 );
						}
						else if(m_nRequestSrcItemSlot == CASHEQUIP_EFFECT)
						{
							pActor->SetPartsVehicleEffect(CDnPartsVehicleEffect::Identity());
						}
					}

					RemoveCashEquipItem( m_nRequestSrcItemSlot );
				}
				if( m_nRequestDestItemSlot != -1 ) m_CashInventory.RemoveItem( m_nRequestDestItemSlot );

				if( pPacket->CashEquipItem.Item.nItemID > 0 ) {
					pSrcItem = CreateItem( pPacket->CashEquipItem );
					InsertCashEquipItem( pSrcItem );

					if( pActor ) {
						if( pPacket->CashEquipItem.cSlotIndex < CASHEQUIP_WEAPON1 ) {
							pActor->AttachCashParts( ((CDnParts*)pSrcItem)->GetMySmartPtr(), (CDnParts::PartsTypeEnum)pPacket->CashEquipItem.cSlotIndex );
							InsertChangePartsThread( CDnActor::s_hLocalActor );
						}
						else if(pPacket->CashEquipItem.cSlotIndex == CASHEQUIP_WEAPON1 || pPacket->CashEquipItem.cSlotIndex == CASHEQUIP_WEAPON2)
						{
							pActor->AttachCashWeapon( ((CDnWeapon*)pSrcItem)->GetMySmartPtr(), pPacket->CashEquipItem.cSlotIndex - CASHEQUIP_WEAPON1 );
						}
						else if(m_nRequestSrcItemSlot == CASHEQUIP_EFFECT)
						{
							pActor->SetPartsVehicleEffect(((CDnPartsVehicleEffect*)pSrcItem)->GetMySmartPtr());
						}
					}
				}
				else {
					if( pActor ) {
						if( (CDnParts::PartsTypeEnum)pPacket->CashEquipItem.cSlotIndex < CASHEQUIP_WEAPON1 ) {
							InsertChangePartsThread( CDnActor::s_hLocalActor );
						}
					}
				}
				if( pPacket->CashInvenItem.nItemID > 0 ) {
					pDestItem = CreateItem( pPacket->CashInvenItem );
					pDestItem->SetSlotIndex( m_nRequestDestItemSlot );
					m_CashInventory.InsertItem( pDestItem, true );
				}
				if( pActor ) {
					pActor->RefreshState();
					CDnInterface::GetInstance().UpdateMyPortrait();
				}
			}
			break;
		case MoveType_CashInvenToCashEquip:
			{
				CDnLocalPlayerActor *pActor = (CDnLocalPlayerActor*)CDnActor::s_hLocalActor.GetPointer();

				if( m_nRequestSrcItemSlot != -1 ) m_CashInventory.RemoveItem( m_nRequestSrcItemSlot );
				if( m_nRequestDestItemSlot != -1 ) {
					if( pActor ) {
						if( m_nRequestDestItemSlot < CASHEQUIP_WEAPON1 )
							pActor->DetachCashParts( (CDnParts::PartsTypeEnum)m_nRequestDestItemSlot );
						else if( m_nRequestDestItemSlot == CASHEQUIP_WEAPON1 || m_nRequestDestItemSlot == CASHEQUIP_WEAPON2)
						{
							pActor->DetachCashWeapon( m_nRequestDestItemSlot - CASHEQUIP_WEAPON1 );
						}
						else if (m_nRequestDestItemSlot == CASHEQUIP_EFFECT )
						{
							pActor->SetPartsVehicleEffect(CDnPartsVehicleEffect::Identity());
						}
					}

					RemoveCashEquipItem( m_nRequestDestItemSlot );
				}

				if( pPacket->CashInvenItem.nItemID > 0 ) {
					pSrcItem = CreateItem( pPacket->CashInvenItem );
					pSrcItem->SetSlotIndex( m_nRequestSrcItemSlot );
					m_CashInventory.InsertItem( pSrcItem, true );
				}

				if( pPacket->CashEquipItem.Item.nItemID > 0 ) {
					pDestItem = CreateItem( pPacket->CashEquipItem );
					InsertCashEquipItem( pDestItem );

					if( pActor ) {
						if( pPacket->CashEquipItem.cSlotIndex < CASHEQUIP_WEAPON1 ) {
							pActor->AttachCashParts( ((CDnParts*)pDestItem)->GetMySmartPtr(), (CDnParts::PartsTypeEnum)pPacket->CashEquipItem.cSlotIndex );
							InsertChangePartsThread( CDnActor::s_hLocalActor );
						}
						else if(pPacket->CashEquipItem.cSlotIndex == CASHEQUIP_WEAPON1 || pPacket->CashEquipItem.cSlotIndex == CASHEQUIP_WEAPON2) 
						{
							pActor->AttachCashWeapon( ((CDnWeapon*)pDestItem)->GetMySmartPtr(), pPacket->CashEquipItem.cSlotIndex - CASHEQUIP_WEAPON1 );
						}
						else if(pPacket->CashEquipItem.cSlotIndex == CASHEQUIP_EFFECT)
						{
							pActor->SetPartsVehicleEffect(((CDnPartsVehicleEffect*)pDestItem)->GetMySmartPtr());
						}
					}
				}
				if( pActor ) {
					pActor->RefreshState();
					CDnInterface::GetInstance().UpdateMyPortrait();
				}
			}
			break;
		case MoveType_CashInven:
			// Currently MoveType_CashInven is not allowed.
			/*
			MoveInventoryItem( m_CashInventory, m_CashInventory, pPacket->SrcItem, pPacket->DestItem );
			*/
			break;
		case MoveType_CashGlyph:
			// Currently MoveType_CashGlyph is not allowed.
			break;
		case MoveType_CashGlyphToCashInven:
			{
				CDnLocalPlayerActor *pActor = (CDnLocalPlayerActor*)CDnActor::s_hLocalActor.GetPointer();

				if( m_nRequestSrcItemSlot != -1 ) {
					if( pActor ) pActor->DetachGlyph( (CDnGlyph::GlyphSlotEnum)m_nRequestSrcItemSlot );
					RemoveGlyphItem( m_nRequestSrcItemSlot );
				}
				if( m_nRequestDestItemSlot != -1 ) m_CashInventory.RemoveItem( m_nRequestDestItemSlot );

				if( pPacket->CashEquipItem.Item.nItemID > 0 ) {
					pSrcItem = CreateItem( pPacket->CashEquipItem );
					InsertGlyphItem( pSrcItem );

					if( pActor ) pActor->AttachGlyph( ((CDnGlyph*)pSrcItem)->GetMySmartPtr(), (CDnGlyph::GlyphSlotEnum)pPacket->CashEquipItem.cSlotIndex );
				}

				if( pPacket->CashInvenItem.nItemID > 0 ) {
					pDestItem = CreateItem( pPacket->CashInvenItem );
					pDestItem->SetSlotIndex( m_nRequestDestItemSlot );
					m_CashInventory.InsertItem( pDestItem, true );
				}
				if( pActor ) {
					pActor->RefreshState();
				}
			}
			break;
		case MoveType_CashInvenToCashGlyph:
			{
				CDnLocalPlayerActor *pActor = (CDnLocalPlayerActor*)CDnActor::s_hLocalActor.GetPointer();

				if( m_nRequestSrcItemSlot != -1 ) m_CashInventory.RemoveItem( m_nRequestSrcItemSlot );
				if( m_nRequestDestItemSlot != -1 ) {
					if( pActor ) pActor->DetachGlyph( (CDnGlyph::GlyphSlotEnum)m_nRequestDestItemSlot );
					RemoveGlyphItem( m_nRequestDestItemSlot );
				}

				if( pPacket->CashInvenItem.nItemID > 0 ) {
					pSrcItem = CreateItem( pPacket->CashInvenItem );
					pSrcItem->SetSlotIndex( m_nRequestSrcItemSlot );
					m_CashInventory.InsertItem( pSrcItem, true );
				}

				if( pPacket->CashEquipItem.Item.nItemID > 0 ) {
					pDestItem = CreateItem( pPacket->CashEquipItem );
					InsertGlyphItem( pDestItem );

					if( pActor ) pActor->AttachGlyph( ((CDnGlyph*)pDestItem)->GetMySmartPtr(), (CDnGlyph::GlyphSlotEnum)pPacket->CashEquipItem.cSlotIndex );
				}
				if( pActor ) {
					pActor->RefreshState();
				}
			}
			break;
		case MoveType_VehicleBodyToVehicleInven: // Mount inventory -> Mount equip
			{
				if( m_nRequestSrcItemSlot != -1 ) {
					RemoveVehicleItem( m_nRequestSrcItemSlot );
					if(m_pVehicleEquipParts)
						RemoveVehicleItem(Vehicle::Slot::Saddle);
				}
				if( m_nRequestDestItemSlot != -1 ) m_PetInventory.RemoveItem( m_nRequestDestItemSlot );

				if( pPacket->CashEquipItem.Item.nItemID > 0 ) {
					pSrcItem = CreateItem( pPacket->CashEquipItem );
					InsertVehicleItem( pSrcItem );
				}

				if( pPacket->CashInvenItem.nItemID > 0 ) {
					pDestItem = CreateItem( pPacket->CashInvenItem );
					pDestItem->SetSlotIndex( m_nRequestDestItemSlot );
					m_PetInventory.InsertItem( pDestItem, true );
				}
			}
			break;
		case MoveType_VehicleInvenToVehicleBody: // Mount inventory -> Mount equip
			{
				if( m_nRequestSrcItemSlot != -1 ) m_PetInventory.RemoveItem( m_nRequestSrcItemSlot );
				if( m_nRequestDestItemSlot != -1 ) {
					RemoveVehicleItem( m_nRequestDestItemSlot );
				}

				if( pPacket->CashInvenItem.nItemID > 0 ) {
					pSrcItem = CreateItem( pPacket->CashInvenItem );
					pSrcItem->SetSlotIndex( m_nRequestSrcItemSlot );
					m_PetInventory.InsertItem( pSrcItem, true );
				}

				if( pPacket->CashEquipItem.Item.nItemID > 0 ) {
					pDestItem = CreateItem( pPacket->CashEquipItem );
					InsertVehicleItem( pDestItem );
				}
			}
			break;
		case MoveType_VehiclePartsToCashInven: // Mount equip -> Cash inventory: in this case there is only part 1.
			{
				if( m_nRequestSrcItemSlot != -1 ) {
					RemoveVehicleItem( m_nRequestSrcItemSlot );
				}
			
				if( m_nRequestDestItemSlot != -1 ) m_CashInventory.RemoveItem( m_nRequestDestItemSlot );
				
				if( pPacket->CashEquipItem.Item.nItemID > 0 ) {
					pSrcItem = CreateItem( pPacket->CashEquipItem );
					InsertCashEquipItem( pSrcItem );
				}

				if( pPacket->CashInvenItem.nItemID > 0 ) {
					pDestItem = CreateItem( pPacket->CashInvenItem );
					pDestItem->SetSlotIndex( m_nRequestDestItemSlot );
					m_CashInventory.InsertItem( pDestItem, true );
				}
			}
			break;
		case MoveType_CashInvenToVehicleParts: // Equip mount from cash inventory
			{
				if( m_nRequestSrcItemSlot != -1 ) m_CashInventory.RemoveItem( m_nRequestSrcItemSlot );

				if(m_nRequestDestItemSlot == Vehicle::Slot::Hair) // For cash equip, it does not affect the mount slot dialog.
					break;                                                // Note that mount effect slots do not arrive through this packet.
					
				if( m_nRequestDestItemSlot != -1 ) {
					RemoveVehicleItem( m_nRequestDestItemSlot );
				}

				if( pPacket->CashInvenItem.nItemID > 0 ) {
					pSrcItem = CreateItem( pPacket->CashInvenItem );
					pSrcItem->SetSlotIndex( m_nRequestSrcItemSlot );
					m_CashInventory.InsertItem( pSrcItem, true );
				}

				if( pPacket->CashEquipItem.Item.nItemID > 0 ) {
					pDestItem = CreateItem( pPacket->CashEquipItem );
					InsertVehicleItem( pDestItem );
				}
			}
			break;
		case MoveType_PetBodyToPetInven: // Pet equip to Pet inventory
			{
				if( m_nRequestSrcItemSlot != -1 ) {
					RemovePetItem( m_nRequestSrcItemSlot );
					if( m_pPetEquipParts1 )
						RemovePetItem( Pet::Slot::Accessory1 );
					if( m_pPetEquipParts2 )
						RemovePetItem( Pet::Slot::Accessory2 );
				}
				if( m_nRequestDestItemSlot != -1 ) m_PetInventory.RemoveItem( m_nRequestDestItemSlot );

				if( pPacket->CashEquipItem.Item.nItemID > 0 ) {
					pSrcItem = CreateItem( pPacket->CashEquipItem );
					InsertPetItem( pSrcItem );
				}

				if( pPacket->CashInvenItem.nItemID > 0 ) {
					pDestItem = CreateItem( pPacket->CashInvenItem );
					pDestItem->SetSlotIndex( m_nRequestDestItemSlot );
					m_PetInventory.InsertItem( pDestItem, true );
				}
			}
			break;
		case MoveType_PetInvenToPetBody: // Pet inventory to Pet equip
			{
				if( m_nRequestSrcItemSlot != -1 ) m_PetInventory.RemoveItem( m_nRequestSrcItemSlot );
				if( m_nRequestDestItemSlot != -1 ) {
					RemovePetItem( m_nRequestDestItemSlot );
				}

				if( pPacket->CashInvenItem.nItemID > 0 ) {
					pSrcItem = CreateItem( pPacket->CashInvenItem );
					pSrcItem->SetSlotIndex( m_nRequestSrcItemSlot );
					m_PetInventory.InsertItem( pSrcItem, true );
				}

				if( pPacket->CashEquipItem.Item.nItemID > 0 ) {
					pDestItem = CreateItem( pPacket->CashEquipItem );
					InsertPetItem( pDestItem );
				}
			}
			break;
		case MoveType_PetPartsToCashInven: // Pet equip to Cash inventory
			{
				if( m_nRequestSrcItemSlot != -1 ) {
					RemovePetItem( m_nRequestSrcItemSlot );
				}

				if( m_nRequestDestItemSlot != -1 ) m_CashInventory.RemoveItem( m_nRequestDestItemSlot );

				if( pPacket->CashEquipItem.Item.nItemID > 0 ) {
					pSrcItem = CreateItem( pPacket->CashEquipItem );
					InsertCashEquipItem( pSrcItem );
				}

				if( pPacket->CashInvenItem.nItemID > 0 ) {
					pDestItem = CreateItem( pPacket->CashInvenItem );
					pDestItem->SetSlotIndex( m_nRequestDestItemSlot );
					m_CashInventory.InsertItem( pDestItem, true );
				}

				CDnLocalPlayerActor* pActor = (CDnLocalPlayerActor*)CDnActor::s_hLocalActor.GetPointer();
				if( pActor )
					pActor->RefreshState();
			}
			break;
		case MoveType_CashInvenToPetParts: // Equip pet from cash inventory
			{
				if( m_nRequestSrcItemSlot != -1 ) m_CashInventory.RemoveItem( m_nRequestSrcItemSlot );

				if( m_nRequestDestItemSlot != -1 ) {
					RemovePetItem( m_nRequestDestItemSlot );
				}

				if( pPacket->CashInvenItem.nItemID > 0 ) {
					pSrcItem = CreateItem( pPacket->CashInvenItem );
					pSrcItem->SetSlotIndex( m_nRequestSrcItemSlot );
					m_CashInventory.InsertItem( pSrcItem, true );
				}

				if( pPacket->CashEquipItem.Item.nItemID > 0 ) {
					pDestItem = CreateItem( pPacket->CashEquipItem );
					InsertPetItem( pDestItem );
				}

				CDnLocalPlayerActor* pActor = (CDnLocalPlayerActor*)CDnActor::s_hLocalActor.GetPointer();
				if( pActor )
					pActor->RefreshState();
			}
			break;
	}

	m_nRequestSrcItemSlot = m_nRequestDestItemSlot = -1;
	CDnInterface::GetInstance().SetCharStatusSlotEvent();

	SCMoveItem PacketForSound;
	PacketForSound.cMoveType = pPacket->cMoveType;
	switch( pPacket->cMoveType )
	{
	case MoveType_CashEquipToCashInven:
		PacketForSound.SrcItem.Item.nItemID = pPacket->CashEquipItem.Item.nItemID;
		PacketForSound.DestItem.Item.nItemID = pPacket->CashInvenItem.nItemID;
		break;
	case MoveType_CashInvenToCashEquip:
		PacketForSound.SrcItem.Item.nItemID = pPacket->CashInvenItem.nItemID;
		PacketForSound.DestItem.Item.nItemID = pPacket->CashEquipItem.Item.nItemID;
		break;
	case MoveType_VehicleBodyToVehicleInven:
		PacketForSound.SrcItem.Item.nItemID = pPacket->CashEquipItem.Item.nItemID;
		PacketForSound.DestItem.Item.nItemID = pPacket->CashInvenItem.nItemID;
		break;
	case MoveType_VehicleInvenToVehicleBody:
		PacketForSound.SrcItem.Item.nItemID = pPacket->CashInvenItem.nItemID;
		PacketForSound.DestItem.Item.nItemID = pPacket->CashEquipItem.Item.nItemID;
		break;
	case MoveType_VehiclePartsToCashInven:
		PacketForSound.SrcItem.Item.nItemID = pPacket->CashEquipItem.Item.nItemID;
		PacketForSound.DestItem.Item.nItemID = pPacket->CashInvenItem.nItemID;
		break;
	case MoveType_CashInvenToVehicleParts:
		PacketForSound.SrcItem.Item.nItemID = pPacket->CashInvenItem.nItemID;
		PacketForSound.DestItem.Item.nItemID = pPacket->CashEquipItem.Item.nItemID;
		break;
	case MoveType_PetBodyToPetInven:
		PacketForSound.SrcItem.Item.nItemID = pPacket->CashEquipItem.Item.nItemID;
		PacketForSound.DestItem.Item.nItemID = pPacket->CashInvenItem.nItemID;
		break;
	case MoveType_PetInvenToPetBody:
		PacketForSound.SrcItem.Item.nItemID = pPacket->CashInvenItem.nItemID;
		PacketForSound.DestItem.Item.nItemID = pPacket->CashEquipItem.Item.nItemID;
		break;
	case MoveType_PetPartsToCashInven:
		PacketForSound.SrcItem.Item.nItemID = pPacket->CashEquipItem.Item.nItemID;
		PacketForSound.DestItem.Item.nItemID = pPacket->CashInvenItem.nItemID;
		break;
	case MoveType_CashInvenToPetParts:
		PacketForSound.SrcItem.Item.nItemID = pPacket->CashInvenItem.nItemID;
		PacketForSound.DestItem.Item.nItemID = pPacket->CashEquipItem.Item.nItemID;
		break;
	}
	PlayItemMoveSound( &PacketForSound );
}

void CDnItemTask::OnRecvItemUserChangeColor( SCChangeColor *pPacket )
{
	DnActorHandle hActor = CDnActor::FindActorFromUniqueID( pPacket->nSessionID );
	if( !hActor ) return;

	CDnPlayerActor *pPlayer = dynamic_cast<CDnPlayerActor *>(hActor.GetPointer());
	if( !pPlayer ) return;

	MAPartsBody::PartsColorEnum ColorType;
	switch( pPacket->cItemType ) {
		case ITEMTYPE_EYEDYE: ColorType = MAPartsBody::EyeColor; break;
		case ITEMTYPE_SKINDYE:ColorType = MAPartsBody::SkinColor; break;
		case ITEMTYPE_HAIRDYE:ColorType = MAPartsBody::HairColor; break;
		default: return;
	}

	pPlayer->SetPartsColor( ColorType, pPacket->dwColor );

	if( CDnPartyTask::IsActive() && CDnPartyTask::GetInstance().GetPartyRole() == CDnPartyTask::SINGLE ) {
		if( CDnActor::s_hLocalActor ) 
			CDnPartyTask::GetInstance().SetLocalData( CDnActor::s_hLocalActor );
	}
}

void CDnItemTask::OnRecvItemUserChangeEquip( SCChangeEquip *pPacket )
{
	DnActorHandle hActor = CDnActor::FindActorFromUniqueID( pPacket->nSessionID );
	if( !hActor ) return;

	CDnPlayerActor *pActor = dynamic_cast<CDnPlayerActor *>(hActor.GetPointer());
	CDnItem *pItem = CreateItem( pPacket->ItemInfo );
	if( pPacket->ItemInfo.cSlotIndex < EQUIP_WEAPON1 ) {
		ScopeLock< CSyncLock > Lock( pActor->GetMemberPartsLock() );
		pActor->DetachParts( (CDnParts::PartsTypeEnum)pPacket->ItemInfo.cSlotIndex );
		if( pItem && dynamic_cast<CDnParts*>(pItem) && pItem->GetItemType() == ITEMTYPE_PARTS ) {
			DnPartsHandle hParts = ((CDnParts*)pItem)->GetMySmartPtr();
			pActor->AttachParts( hParts, (CDnParts::PartsTypeEnum)pPacket->ItemInfo.cSlotIndex, true );			
		}
		InsertChangePartsThread( hActor );
	}
	else {
		pActor->DetachWeapon( pPacket->ItemInfo.cSlotIndex - EQUIP_WEAPON1 );

		if( pItem && dynamic_cast<CDnWeapon*>(pItem) && pItem->GetItemType() == ITEMTYPE_WEAPON ) {
			DnWeaponHandle hWeapon = ((CDnWeapon*)pItem)->GetMySmartPtr();
			pActor->AttachWeapon( hWeapon, pPacket->ItemInfo.cSlotIndex - EQUIP_WEAPON1, true );
		}
	}
	pActor->RefreshState();
}

void CDnItemTask::OnRecvItemUserChangeCashEquip( SCChangeCashEquip *pPacket )
{
	DnActorHandle hActor = CDnActor::FindActorFromUniqueID( pPacket->nSessionID );
	if( !hActor ) return;

	CDnPlayerActor *pActor = dynamic_cast<CDnPlayerActor *>(hActor.GetPointer());
	CDnItem *pItem = CreateItem( pPacket->ItemInfo );
	if( pPacket->ItemInfo.cSlotIndex < CASHEQUIP_WEAPON1 ) {
		ScopeLock< CSyncLock > Lock( pActor->GetMemberPartsLock() );
		pActor->DetachCashParts( (CDnParts::PartsTypeEnum)pPacket->ItemInfo.cSlotIndex );

		if( pItem && dynamic_cast<CDnParts*>(pItem) && pItem->GetItemType() == ITEMTYPE_PARTS ) {
			DnPartsHandle hParts = ((CDnParts*)pItem)->GetMySmartPtr();
			pActor->AttachCashParts( hParts, (CDnParts::PartsTypeEnum)pPacket->ItemInfo.cSlotIndex, true );
		}

		InsertChangePartsThread( hActor );
	}
	else if(pPacket->ItemInfo.cSlotIndex == CASHEQUIP_WEAPON1 || pPacket->ItemInfo.cSlotIndex == CASHEQUIP_WEAPON2){
		pActor->DetachCashWeapon( pPacket->ItemInfo.cSlotIndex - CASHEQUIP_WEAPON1 );
		if( pItem && dynamic_cast<CDnWeapon*>(pItem) && pItem->GetItemType() == ITEMTYPE_WEAPON ) {
			DnWeaponHandle hWeapon = ((CDnWeapon*)pItem)->GetMySmartPtr();
			pActor->AttachCashWeapon( hWeapon, pPacket->ItemInfo.cSlotIndex - CASHEQUIP_WEAPON1, true );
		}
	}
	else if(pPacket->ItemInfo.cSlotIndex == CASHEQUIP_EFFECT)
	{
		pActor->SetPartsVehicleEffect(CDnPartsVehicleEffect::Identity());
		if( pItem && dynamic_cast<CDnPartsVehicleEffect*>(pItem) && pItem->GetItemType() == ITEMTYPE_VEHICLEEFFECT ) {
			DnPartsVehicleEffectHandle hVehicleEffect = ((CDnPartsVehicleEffect*)pItem)->GetMySmartPtr();
			pActor->SetPartsVehicleEffect(hVehicleEffect, true);
		}
	}

	pActor->RefreshState();
}

void CDnItemTask::OnRecvItemPickup( SCPickUp *pPacket )
{
	if (pPacket->nRetCode != ERROR_NONE)
	{
		//GetInterface().ServerMessageBox(pPacket->nRetCode);

		// Show message only when inventory is full or gold is full.
		if( pPacket->nRetCode == 405 || pPacket->nRetCode == 420 )
		{
			//GetInterface().AddMessageText( GetEtUIXML().GetUIString( CEtUIXML::idCategory2, pPacket->nRetCode ) );
			WCHAR wszMsg[128];
			swprintf_s( wszMsg, _countof(wszMsg), L"\n\n%s", GetEtUIXML().GetUIString( CEtUIXML::idCategory2, pPacket->nRetCode ) );
			GetInterface().ShowItemAlarmDialog( wszMsg, NULL, 0, textcolor::WHITE, 2.0f );
		}
		else
		{
			CDebugSet::ToLogFile( "CDnItemTask::OnRecvItemPickup, Unknown Error(%d)", pPacket->nRetCode );
		}
	}
	else
	{
		CDnItem *pItem = CreateItem( pPacket->PickUpItem );
		if( pItem )
		{
			DnDebugConsole::LogW( L"LOOT", L"Local Pickup ItemID=%d Count=%d", pItem->GetClassID(), pItem->GetOverlapCount() );
			PlayPickupSound( pItem );

			// Check if going into an existing slot (stackable item quantity increase)
			bool bExistItem = false;
			if( m_CharInventory.GetItem( pItem->GetSlotIndex() ) )
				bExistItem = true;

			if( m_CharInventory.InsertItem( pItem ) )
			{
				// Indicates via the second argument whether it is a quantity increase or a newly arrived item.
				pItem->SetNewGain( true, bExistItem );

				CDnInvenTabDlg* pInvenTabDlg = (CDnInvenTabDlg*)GetInterface().GetMainMenuDialog( CDnMainMenuDlg::INVENTORY_DIALOG );
				if( pInvenTabDlg )
					pInvenTabDlg->ShowTabNew( ST_INVENTORY );
			}

			if( CDnActor::s_hLocalActor) 
			{
				CDnLocalPlayerActor *pPlayer = static_cast<CDnLocalPlayerActor *>(CDnActor::s_hLocalActor.GetPointer());
				pPlayer->UpdateGetItem();
#ifdef PRE_MOD_NESTREBIRTH
				SetUpdateRebirthCoinOrItem(true);
#endif

				// Set up the event slot here.
				CDnMainDlg *pMainBarDlg = GetInterface().GetMainBarDialog();
				if(pMainBarDlg)
				{
					CEtUIDialogGroup *pQuickSlotDlgGroup = pMainBarDlg->GetQuickSlotDialogGroup();
					if(pQuickSlotDlgGroup)
					{
						CDnGameTask *pGameTask = (CDnGameTask*)CTaskManager::GetInstance().GetTask( "GameTask" );
						if( pGameTask && pGameTask->GetGameTaskType() == GameTaskType::PvP && ((CDnPvPGameTask*)pGameTask)->GetGameMode() == PvPCommon::GameMode::PvP_Zombie_Survival) 
						{
							CDnQuickSlotDlg *pEventQuickSlotDlg = static_cast<CDnQuickSlotDlg*>(pQuickSlotDlgGroup->GetDialog( CDnMainDlg::QUICKSLOT_EVENT ));
							pEventQuickSlotDlg->SetEventSlot(pItem);
						}
					}
				}
			}
		}
	}
}

void CDnItemTask::OnRecvItemRefreshInven( SCRefreshInven *pPacket )
{
	OutputDebug( "CDnItemTask::OnRecvItemRefreshInven\n" );

	if( pPacket->ItemInfo.Item.nItemID > 0 )
	{
		CDnItem *pItem = CreateItem( pPacket->ItemInfo );
		if (pItem == NULL)
		{
			_ASSERT(0 && "FAIL TO CREATE ITEM!");
			return;
		}
		// Check if going into an existing slot (stackable item quantity increase)
		bool bExistItem = false;
		CDnItem *pPreItem = NULL;
		if( pPreItem = m_CharInventory.GetItem( pItem->GetSlotIndex() ) )
			bExistItem = true;

		// If going into an existing slot, check OverlapCount to determine whether it is an increment.
		bool bAddOverlapCount = false;
		if( bExistItem ) {
			if( pItem->GetOverlapCount() > pPreItem->GetOverlapCount() )
				bAddOverlapCount = true;
		}

		// Because the second argument is true, unlike Pickup, no item acquisition window or new mark is shown.
		// So handle only the 'new' indicator via a separate function call.
		// Since this packet arrives after repair as well as exchange/trade, check whether to show the new indicator too.
		if( m_CharInventory.InsertItem( pItem, true ) && pPacket->bNewSign )
		{
			// Indicates via the second argument whether it is a quantity increase or a newly arrived item.
			pItem->SetNewGain( true, bExistItem );

			// Play RootSound only when properly inserted into inventory and placed into a new slot.
			if( bExistItem == false || bAddOverlapCount == true )
				PlayItemSound( pPacket->ItemInfo.Item.nItemID, ItemSound_Root );

			CDnInvenTabDlg* pInvenTabDlg = (CDnInvenTabDlg*)GetInterface().GetMainMenuDialog( CDnMainMenuDlg::INVENTORY_DIALOG );
			if( pInvenTabDlg )
				pInvenTabDlg->ShowTabNew( ST_INVENTORY );

			if( GetInterface().GetItemsOwnedAlarmMng() )
				GetInterface().GetItemsOwnedAlarmMng()->CheckOwnedItemNotice( pPacket->ItemInfo.Item.nItemID );
		}

#ifdef PRE_ADD_CHAOSCUBE
		// Update if Chaos Cube is open.
		if( GetInterface().IsOpenChaosCubeDlg() )
		{
			GetInterface().UpdateChaosCubeDlg();
		}
		if( GetInterface().IsOpenChaosGaugeCubeDlg() )
		{
			GetInterface().UpdateChaosGaugeCubeDlg();
		}
#endif

#ifdef PRE_ADD_CHOICECUBE
		// Refresh if Choice Cube is open.
		if( GetInterface().IsOpenChoiceCubeDlg() )
		{
			GetInterface().UpdateChoiceCubeDlg();
		}		
#endif

	}
	else
	{
		m_CharInventory.RemoveItem( pPacket->ItemInfo.cSlotIndex );
	}

	GetInterface().RefreshPetFoodCount();
}

void CDnItemTask::OnRecvItemRefreshCashInven( SCRefreshCashInven *pPacket )
{
#ifdef PRE_ADD_GACHA_JAPAN
	// When a newly acquired item packet arrives from gachapon result, buffer it temporarily in memory and process it after the dialog animation finishes.
	if( m_bSendGachaponReq )
	{
		if( pPacket->bNewSign )
		{
			// Copying the entire memory occasionally crashes unexpectedly. Maybe a synchronization issue...
			//m_GachaResultCashItemPacket = *pPacket;
			//memcpy_s( &m_GachaResultCashItemPacket, sizeof(SCRefreshCashInven), pPacket, sizeof(SCRefreshCashInven) );

			m_GachaResultCashItemPacket.bNewSign = pPacket->bNewSign;
			m_GachaResultCashItemPacket.nCount = pPacket->nCount;

			_ASSERT( 1 == m_GachaResultCashItemPacket.nCount );
			for( int i = 0; i < pPacket->nCount; ++i )
				m_GachaResultCashItemPacket.ItemList[ i ] = pPacket->ItemList[ i ];

			GetInterface().OnRecvGachaRes_JP( pPacket );

			m_bSendGachaponReq = false;
			return;
		}
	}
#endif

	OutputDebug( "CDnItemTask::OnRecvItemRefreshInven\n" );

	for( int i = 0; i < pPacket->nCount; ++i )
	{
		if( pPacket->ItemList[i].wCount > 0 )
		{
			CDnItem *pItem = CreateItem( pPacket->ItemList[i] );
			if (pItem == NULL)
			{
				_ASSERT(0 && "FAIL TO CREATE ITEM!");
				return;
			}
			// Check if going into an existing slot (stackable item quantity increase)
			bool bExistItem = false;
			CDnItem *pPreItem = NULL;
			if( pPreItem = m_CashInventory.FindItemFromSerialID( pItem->GetSerialID() ) )
				bExistItem = true;

			// Unlike normal inventory, cash refresh does not include slot index, so determine it manually.
			if( bExistItem )
				pItem->SetSlotIndex( pPreItem->GetSlotIndex() );
			else
				pItem->SetSlotIndex( m_CashInventory.FindFirstEmptyIndex() );

			// If going into an existing slot, check OverlapCount to determine whether it is an increment.
			bool bAddOverlapCount = false;
			if( bExistItem ) {
				if( pItem->GetOverlapCount() > pPreItem->GetOverlapCount() )
					bAddOverlapCount = true;
			}

			// Because the second argument is true, unlike Pickup, no item acquisition window or new mark is shown.
			// So handle only the 'new' indicator via a separate function call.
			// Since this packet arrives after repair as well as exchange/trade, check whether to show the new indicator too.
			if( m_CashInventory.InsertItem( pItem, true ) && pPacket->bNewSign )
			{
				// Indicates via the second argument whether it is a quantity increase or a newly arrived item.
				pItem->SetNewGain( true, bExistItem );

				// Play RootSound only when properly inserted into inventory and placed into a new slot.
				if( bExistItem == false || bAddOverlapCount == true )
					PlayItemSound( pPacket->ItemList[i].nItemID, ItemSound_Root );
				
#ifdef PRE_ADD_LEVELUP_REWARD_NOTIFIER
				if( IsLevelUpBoxItem( pItem ) )
				{
					CDnMainDlg* pMainBarDlg = GetInterface().GetMainBarDialog();
					if( ! GetExistLevelUpBox( CDnActor::s_hLocalActor->GetLevel() ) )
					{			
						if( pMainBarDlg && pMainBarDlg->IsShow() )
						{
							pMainBarDlg->OnLevelUpBoxNotify( false );	
						}
					}
					else
					{
						if( pMainBarDlg && pMainBarDlg->IsShow() )
						{
							pMainBarDlg->OnLevelUpBoxNotify( true );	
						}
					}
				}
#endif 

				CDnInvenTabDlg* pInvenTabDlg = (CDnInvenTabDlg*)GetInterface().GetMainMenuDialog( CDnMainMenuDlg::INVENTORY_DIALOG );
				if( pInvenTabDlg )
					pInvenTabDlg->ShowTabNew( ST_INVENTORY_CASH );
			}
		}
		else
		{
			CDnItem *pItem = m_CashInventory.FindItemFromSerialID( pPacket->ItemList[i].nSerial );
			if( pItem )
			{
				m_CashInventory.RemoveItem( pItem->GetSlotIndex() );
			}
			else
			{
#ifdef PRE_ADD_CASHREMOVE
				CDnItem *pRemoveItem = m_CashRemoveInventory.FindItemFromSerialID( pPacket->ItemList[i].nSerial );
				if( pRemoveItem )
					m_CashRemoveInventory.RemoveItem( pRemoveItem->GetSlotIndex() );
#endif	// #ifdef PRE_ADD_CASHREMOVE
			}
		}
	}
	OnRefreshCashInventory();

#ifdef PRE_ADD_CHAOSCUBE
	// Update if Chaos Cube is open.
	if( GetInterface().IsOpenChaosCubeDlg() )
	{
		GetInterface().UpdateChaosCubeDlg();
	}
	if( GetInterface().IsOpenChaosGaugeCubeDlg() )
	{
		GetInterface().UpdateChaosGaugeCubeDlg();
	}
#endif

#ifdef PRE_ADD_CHOICECUBE
	// Refresh if Choice Cube is open.
	if( GetInterface().IsOpenChoiceCubeDlg() )
	{
		GetInterface().UpdateChoiceCubeDlg();
	}		
#endif

}

void CDnItemTask::OnRecvItemRefreshEquip( SCRefreshEquip *pPacket )
{
	if( !CDnActor::s_hLocalActor ) return;
	CDnLocalPlayerActor *pActor = (CDnLocalPlayerActor*)CDnActor::s_hLocalActor.GetPointer();
	int nIndex = pPacket->ItemInfo.cSlotIndex;
	if( m_pEquip[nIndex] ) {
		if( nIndex < EQUIP_WEAPON1 )
			pActor->DetachParts( (CDnParts::PartsTypeEnum)nIndex );
		else pActor->DetachWeapon( nIndex - EQUIP_WEAPON1 );
	}
	RemoveEquipItem( nIndex );

	if( pPacket->ItemInfo.Item.nItemID > 0 ) {
		CDnItem *pItem = CreateItem( pPacket->ItemInfo );
		InsertEquipItem( pItem );
		if( nIndex < EQUIP_WEAPON1 )
			pActor->AttachParts( ((CDnParts*)pItem)->GetMySmartPtr(), (CDnParts::PartsTypeEnum)nIndex );
		else
			pActor->AttachWeapon( ((CDnWeapon*)pItem)->GetMySmartPtr(), nIndex - EQUIP_WEAPON1 );
	}
	GetInterface().UpdateMyPortrait();
#if defined(PRE_ADD_EQUIPED_ITEM_ENCHANT)
	if(pPacket->bRefreshState)
		pActor->RefreshState(CDnActorState::RefreshEquip);
#endif
}

void CDnItemTask::OnRecvItemRefreshWarehouse( SCRefreshWarehouse *pPacket )
{
	OutputDebug( "CDnItemTask::OnRecvItemRefreshWarehouse\n" );

	if( pPacket->ItemInfo.Item.nItemID > 0 )
	{
		CDnItem *pItem = CreateItem( pPacket->ItemInfo );
		if (pItem == NULL)
		{
			_ASSERT(0 && "FAIL TO CREATE ITEM!");
			return;
		}
		m_StorageInventory.InsertItem( pItem );
	}
	else
	{
		m_StorageInventory.RemoveItem( pPacket->ItemInfo.cSlotIndex );
	}
}

void CDnItemTask::OnRecvItemRepairEquip( SCRepairResult *pPacket )
{
	if (pPacket->nRet != ERROR_NONE)
	{
		GetInterface().ServerMessageBox(pPacket->nRet);
	}
	else
	{
		if( CDnActor::s_hLocalActor )
		{
			CDnLocalPlayerActor *pActor = (CDnLocalPlayerActor*)CDnActor::s_hLocalActor.GetPointer();
			pActor->RefreshState();
		}

		if( m_nRepairSound != -1 )
			CEtSoundEngine::GetInstance().PlaySound( "2D", m_nRepairSound, false );

		WCHAR wszTemp[128] = { 0, };
		wsprintf( wszTemp, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 1732 ) );
		GetInterface().AddChatMessage( CHATTYPE_SYSTEM, L"", wszTemp );
		GetInterface().RefreshPlayerDurabilityIcon();
	}
	m_bRequestRepair = false;
}

void CDnItemTask::OnRecvItemRepairAll( SCRepairResult *pPacket )
{
	if (pPacket->nRet != ERROR_NONE)
	{
		GetInterface().ServerMessageBox(pPacket->nRet);
	}
	else
	{
		if( CDnActor::s_hLocalActor )
		{
			CDnLocalPlayerActor *pActor = (CDnLocalPlayerActor*)CDnActor::s_hLocalActor.GetPointer();
			pActor->RefreshState();
		}

		if( m_nRepairSound != -1 )
			CEtSoundEngine::GetInstance().PlaySound( "2D", m_nRepairSound, false );

		WCHAR wszTemp[128] = { 0, };
		wsprintf( wszTemp, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 1733 ) );
		GetInterface().AddChatMessage( CHATTYPE_SYSTEM, L"", wszTemp );
		GetInterface().RefreshPlayerDurabilityIcon();
	}
	m_bRequestRepair = false;
}

void CDnItemTask::OnRecvItemMoveCoin( SCMoveCoin *pPacket )
{
	if (pPacket->nRet != ERROR_NONE)
	{
		GetInterface().ServerMessageBox(pPacket->nRet);
	}
	else
	{
		PlayMoneySound();


		SetCoin( pPacket->nInventoryCoin );
		SetCoinStorage( pPacket->nWarehouseCoin );
	}
}

void CDnItemTask::OnRecvItemMoveGuildCoin( SCMoveGuildCoin *pPacket )
{
	if (pPacket->nRet != ERROR_NONE)
	{
		GetInterface().ServerMessageBox(pPacket->nRet);
	}
	else
	{
		PlayMoneySound();
		SetCoin( pPacket->nInventoryCoin );
		SetGuildWareCoin( pPacket->nWarehouseCoin );
		if( pPacket->cMoveType == MoveType_GuildWareToInven )
			m_biWithdrawGuildWareCoin = pPacket->nWithdrawCoin;
	}
}

void CDnItemTask::OnRecvItemRefreshGuildCoin( SCRefreshGuildCoin *pPacket )
{
	SetGuildWareCoin( pPacket->biTotalCoin );
}

void CDnItemTask::OnRecvItemRemove( SCRemoveItem *pPacket )
{
	if (pPacket->nRetCode == ERROR_NONE)
	{
		if( m_nRequestSrcItemSlot != pPacket->ItemInfo.cSlotIndex ) {
			assert(0&&"Strange");
		}
		switch( pPacket->cType ) {
			case ITEMPOSITION_EQUIP:
				{
					CDnLocalPlayerActor *pActor = (CDnLocalPlayerActor*)CDnActor::s_hLocalActor.GetPointer();

					if( pActor ) {
						if( pPacket->ItemInfo.cSlotIndex < EQUIP_WEAPON1 )
							pActor->DetachParts( (CDnParts::PartsTypeEnum)pPacket->ItemInfo.cSlotIndex );
						else 
							pActor->DetachWeapon( pPacket->ItemInfo.cSlotIndex - EQUIP_WEAPON1 );
					}

					RemoveEquipItem( pPacket->ItemInfo.cSlotIndex );

					if( pPacket->ItemInfo.Item.nItemID > 0 ) {
						CDnItem *pItem = CreateItem( pPacket->ItemInfo );
						InsertEquipItem( pItem );

						if( pActor ) {
							if( pPacket->ItemInfo.cSlotIndex < EQUIP_WEAPON1 )
								pActor->AttachParts( ((CDnParts*)pItem)->GetMySmartPtr(), (CDnParts::PartsTypeEnum)pPacket->ItemInfo.cSlotIndex );
							else
								pActor->AttachWeapon( ((CDnWeapon*)pItem)->GetMySmartPtr(), pPacket->ItemInfo.cSlotIndex - EQUIP_WEAPON1 );
						}
					}
					if( pActor ) {
						pActor->CombineParts();
						pActor->RefreshState();
					}
				}
				break;
			case ITEMPOSITION_INVEN:
				m_CharInventory.RemoveItem( pPacket->ItemInfo.cSlotIndex );

				if( pPacket->ItemInfo.Item.nItemID > 0 ) {
					CDnItem *pItem = CreateItem( pPacket->ItemInfo );
					m_CharInventory.InsertItem( pItem, true );
				}
				break;
			case ITEMPOSITION_WARE:
				{
					// Note: Storage items do not include items picked up from the ground.
					//		Can only be moved in and out from the inventory.
				}
				break;
			case ITEMPOSITION_QUESTINVEN:
				{
					m_QuestInventory.RemoveItem( pPacket->ItemInfo.cSlotIndex );

					if (pPacket->ItemInfo.Item.nItemID > 0 && pPacket->ItemInfo.Item.wCount > 0)
					{
						CDnItem *pItem = CreateItem( pPacket->ItemInfo );
						m_QuestInventory.InsertItem( pItem, true );
					}
				}
				break;
			case ITEMPOSITION_GLYPH:
				{
					CDnLocalPlayerActor *pActor = (CDnLocalPlayerActor*)CDnActor::s_hLocalActor.GetPointer();

					if( pActor ) pActor->DetachGlyph( (CDnGlyph::GlyphSlotEnum)pPacket->ItemInfo.cSlotIndex );

					RemoveGlyphItem( pPacket->ItemInfo.cSlotIndex );

					if( pActor ) pActor->RefreshState();
				}
				break;
#if defined(PRE_ADD_TALISMAN_SYSTEM)
			case ITEMPOSITION_TALISMAN:
				{
					CDnLocalPlayerActor *pActor = (CDnLocalPlayerActor*)CDnActor::s_hLocalActor.GetPointer();

					if( pActor ) pActor->DetachTalisman( pPacket->ItemInfo.cSlotIndex );

					RemoveTalismanItem( pPacket->ItemInfo.cSlotIndex );

					if( pActor ) pActor->RefreshState();
				}
				break;
#endif
			
		}
	}
	else
		GetInterface().ServerMessageBox(pPacket->nRetCode);
	
	m_nRequestSrcItemSlot = -1;
	m_bRequestWait = false;
}


#ifdef PRE_ADD_CASHREMOVE

void CDnItemTask::RequestRemoveCashItem( INT64 biSerial, bool bRecovery )
{
	// Check item.
	CDnItem * pItem = NULL;

	// Restore - cash removal waiting tab.
	if( bRecovery ) 
		pItem = m_CashRemoveInventory.FindItemFromSerialID( biSerial );

	// Remove - Cash tab.
	else 
		pItem = m_CashInventory.FindItemFromSerialID( biSerial );

	if( pItem == NULL )
		return;

	SendRemoveCash( biSerial, bRecovery );	
}


void CDnItemTask::OnRecvItemRemoveCash( SCRemoveCash * pPacket )
{
	//
	if( pPacket->nRet == ERROR_NONE )
	{
		// Remove - Move to cash pending tab.
		if( pPacket->bExpireComplete )
		{
			CDnItem * pItem = m_CashInventory.FindItemFromSerialID( pPacket->biItemSerial );
			if( pItem )
			{
				// Validation - When removing and moving to the cash tab, if an item with the same sn already exists in the cash tab, the item was duplicated, so it is invalid.
				CDnItem * pItemInvalid = m_CashRemoveInventory.FindItemFromSerialID( pPacket->biItemSerial );
				if( pItemInvalid )
				{
					g_Log.Log(LogType::_ERROR, L"RemoveCash Same item already exists..\n");
					return;
				}


				// Set pending-delete item.
				pItem->SetCashRemoveItem( true );

				// Create and add item to cash pending tab - recreated because Item memory is deleted when removed from cash tab.
				TItemInfo tItemInfo;				 
				pItem->GetTItemInfo( tItemInfo );
				tItemInfo.Item.nCoolTime = (int)pItem->GetCoolTime(); // Cooldown.
				tItemInfo.Item.bEternity = pPacket->bEternity;
				tItemInfo.Item.tExpireDate = pPacket->tExpireDate;
				tItemInfo.Item.bExpireComplete = pPacket->bExpireComplete;
				__time64_t tCrr = CSyncTimer::GetInstance().GetCurTime();
				m_CashRemoveInventory.CreateCashItem( tItemInfo.Item, m_CashRemoveInventory.FindFirstEmptyIndex(),
													  tCrr, (pPacket->tExpireDate-tCrr), pPacket->bExpireComplete );

				
				// #_1 : CDnInvenTabDlg::ResetSlot() distinguishes between CDnInvenCashDlg and CDnInvenStandByCashRemoveDlg when calling ResetSlot().
				// For [Delete], since the item moves from the previous tab to the current tab (prev tab => curr tab), ResetSlot() must be called on the previous tab.
				// However, since this function is used for both [Update] and [Delete], it cannot be handled in ResetSlot(), so arbitrarily change item state here 
				// Call ResetSlot() for the previous tab.
				// ---- Otherwise the item slot area of the current tab becomes empty.
				pItem->SetCashRemoveItem( false ); 

				// Remove from cash tab.				
				m_CashInventory.RemoveItem( pItem->GetSlotIndex() );				
			}
		}

		// Restore - move to cash tab.
		else
		{
			CDnItem * pItem = m_CashRemoveInventory.FindItemFromSerialID( pPacket->biItemSerial );
			if( pItem )
			{
				// Validation - When removing and moving to the cash standby tab, if an item with the same sn already exists in the cash standby tab, the item was duplicated, so it is invalid.
				CDnItem * pItemInvalid = m_CashInventory.FindItemFromSerialID( pPacket->biItemSerial );
				if( pItemInvalid )
				{
					g_Log.Log(LogType::_ERROR, L"RemoveCash Same item already exists..\n");
					return;
				}

				// Set pending-delete item.
				pItem->SetCashRemoveItem( false );

				// Create and add item to cash tab.
				TItemInfo tItemInfo;				 
				pItem->GetTItemInfo( tItemInfo );				
				tItemInfo.Item.nCoolTime = (int)pItem->GetCoolTime(); // Cooldown.
				tItemInfo.Item.bEternity = pPacket->bEternity;
				tItemInfo.Item.tExpireDate = pPacket->tExpireDate;
				tItemInfo.Item.bExpireComplete = pPacket->bExpireComplete;
				m_CashInventory.CreateCashItem( tItemInfo.Item, m_CashInventory.FindFirstEmptyIndex() );

				// Remove from cash pending tab.			

				// Same as #_1 above.
				pItem->SetCashRemoveItem( true );
				m_CashRemoveInventory.RemoveItem( pItem->GetSlotIndex() );
			}
		}

		// Sort.
		//SortCashInven(); // Cash Inven.
		//m_CashRemoveInventory.SortCashInventory(); // CashRemove Inven.
	}

	//
	else
	{
		GetInterface().MessageBox( GetEtUIXML().GetUIString( CEtUIXML::idCategory2, pPacket->nRet ) );
	}
}


__time64_t CDnItemTask::GetTimeBySN( INT64 nSerial )
{
	return m_CashRemoveInventory.GetTimeBySN( nSerial );
}

#endif


void CDnItemTask::OnRecvItemDropItemMsg( SCDropItem *pPacket )
{
	DnDropItemHandle hDropItem = CDnDropItem::FindItemFromUniqueID(pPacket->nSessionID);
	if( !hDropItem ) return;
	CDnBaseTransAction *pTrans = dynamic_cast<CDnBaseTransAction *>(hDropItem.GetPointer());
	if( pTrans ) pTrans->OnDispatchMessage( pPacket->nItemProtocol, (BYTE*)pPacket->cBuf );
}

void CDnItemTask::OnRecvItemCreateDropItem( SCCreateDropItem *pPacket )
{
	EtVector3 vec;
	vec.x = pPacket->fpos[0];
	vec.y = pPacket->fpos[1];
	vec.z = pPacket->fpos[2];
	DnDebugConsole::LogW( L"LOOT", L"DropItem UniqueID=%u ItemID=%d Count=%d OwnerSessionID=%d",
		pPacket->nSessionID, pPacket->nItemID, pPacket->nCount, pPacket->nOwnerSessionID );
	CDnDropItem::s_dwUniqueCount = pPacket->nSessionID + 1;
	CDnDropItem::DropItem( vec, pPacket->nSessionID, pPacket->nItemID, pPacket->nRandomSeed, pPacket->cOption, pPacket->nCount, pPacket->nRotate, pPacket->nOwnerSessionID );
}

void CDnItemTask::OnRecvItemCreateDropItemList( SCCreateDropItemList *pPacket )
{
	DnDebugConsole::LogW( L"LOOT", L"DropItemList Count=%d", pPacket->nCount );
	for( int i=0 ; i<pPacket->nCount ; ++i )
	{
		EtVector3 vec;
		vec.x = pPacket->sDropItem[i].fpos[0];
		vec.y = pPacket->sDropItem[i].fpos[1];
		vec.z = pPacket->sDropItem[i].fpos[2];
		DnDebugConsole::LogW( L"LOOT", L"  Drop[%d] UniqueID=%u ItemID=%d Count=%d OwnerSessionID=%d",
			i, pPacket->sDropItem[i].nSessionID, pPacket->sDropItem[i].nItemID, pPacket->sDropItem[i].nCount, pPacket->sDropItem[i].nOwnerSessionID );
		CDnDropItem::DropItem( vec, pPacket->sDropItem[i].nSessionID, pPacket->sDropItem[i].nItemID, pPacket->sDropItem[i].nRandomSeed, pPacket->sDropItem[i].cOption, pPacket->sDropItem[i].nCount, pPacket->sDropItem[i].nRotate, pPacket->sDropItem[i].nOwnerSessionID );
	}
}

void CDnItemTask::OnRecvItemRequestRandomItem( SCRequestRandomItem *pPacket )
{
	switch( pPacket->nRet ) {
		case -1:
			GetInterface().MessageBox( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 1925 ), MB_OK );
			break;
		case -2:
			GetInterface().MessageBox( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 100050 ), MB_OK );
			break;
		default:
			GetInterface().OpenRandomItemDlg( pPacket->cInvenType, pPacket->cInvenIndex, pPacket->biInvenSerial );
			m_bRequestWait = true;
			break;
	}
}

void CDnItemTask::OnRecvItemCompleteRandomItem( SCCompleteRandomItem *pPacket )
{
	switch( pPacket->nRet ) {
		case -1:
			GetInterface().MessageBox( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 1925 ), MB_OK );
			break;
		case -2:
			GetInterface().MessageBox( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 100050 ), MB_OK );
			break;
		default:
			GetInterface().OpenRandomItemResultDlg( pPacket->cInvenType, pPacket->nResultItemID, pPacket->cLevel, pPacket->nItemCount );
			break;
	}

	m_bRequestWait = false;

	CDnTradeMail& tradeMail = GetTradeTask().GetTradeMail();
	if (tradeMail.IsOnMailMode())
		tradeMail.LockMailDlg(false);
}

void CDnItemTask::OnRecvItemRequestCharmItem( SCCharmItemRequest *pPacket )
{
	if( pPacket->nRet == ERROR_NONE )
	{
#ifdef PRE_ADD_AUTOUNPACK
		// Treasure chest auto-open window.
		CDnAutoUnPackDlg * pAutoUnPackDlg = GetInterface().GetAutoUnPackDlg(); // Treasure chest auto-open window.		
		if( pAutoUnPackDlg->IsShow() )
			pAutoUnPackDlg->OnRecvRequestCharmItem( pPacket->cInvenType, pPacket->sInvenIndex, pPacket->biInvenSerial ); 
		else
		{
#if defined( PRE_ADD_EASYGAMECASH )
			CDnItem *pCharmItem = NULL;
			if (pPacket->cInvenType == ITEMPOSITION_CASHINVEN)
				pCharmItem = GetCashInventory().GetItem( pPacket->sInvenIndex );
			else
				pCharmItem = GetCharInventory().GetItem( pPacket->sInvenIndex );

			if ( pCharmItem && pCharmItem->GetItemType() == ITEMTYPE_ALLGIVECHARM )
			{
				GetInterface().ShowPackageProgressDlg( pPacket->cInvenType, (BYTE)pPacket->sInvenIndex, pPacket->biInvenSerial );
				if( GetInterface().GetPackageBoxResultAutoDlg() && pAutoUnPackDlg && pAutoUnPackDlg->IsShow() )
					GetInterface().GetPackageBoxResultAutoDlg()->StartAutoOpen( pCharmItem->GetName(), 1.5f );
			}
#ifdef PRE_ADD_GETITEM_WITH_RANDOMVALUE
			else if( pCharmItem && pCharmItem->GetItemType() == ITEMTYPE_CHARMRANDOM )
			{	
				GetInterface().ShowRandomCharmItemProgressDialog( true, pPacket->cInvenType, pPacket->sInvenIndex, pPacket->biInvenSerial, pPacket->RandomGiveItems, (int)pPacket->cRandomGiveCount);
			}
#endif 
			else if( pCharmItem )
			{
				DNTableFileFormat *pCharmSox = GetDNTable( CDnTableDB::TCHARMITEM );
				if( pCharmSox )
				{
					int nCharmNum = pCharmItem->GetTypeParam( 0 );
					if( pCharmSox->IsExistItem( nCharmNum ) )
					{
						DNTableCell *pPotenLookCell = pCharmSox->GetFieldFromLablePtr( nCharmNum, "_PotenLook" );
						if( pPotenLookCell && pPotenLookCell->GetInteger() > 0 && GetInterface().GetPotenCharmItemProgressDlg() )
						{
							GetInterface().GetPotenCharmItemProgressDlg()->SetPotenGauge( 100.0f, false, pCharmItem->GetName() );
							GetInterface().GetPotenCharmItemProgressDlg()->Show( true );
						}
					}
				}
				GetInterface().GetCharmItemKeyDlg()->OnRecvRequestCharmItem( pPacket->cInvenType, pPacket->sInvenIndex, pPacket->biInvenSerial );
			}

#else
				GetInterface().GetCharmItemKeyDlg()->OnRecvRequestCharmItem( pPacket->cInvenType, pPacket->sInvenIndex, pPacket->biInvenSerial );
#endif	// #if defined( PRE_ADD_EASYGAMECASH )
		}
#else
		GetInterface().GetCharmItemKeyDlg()->OnRecvRequestCharmItem( pPacket->cInvenType, pPacket->sInvenIndex, pPacket->biInvenSerial );
#endif
	}
	else
	{
		ASSERT(0&&"Unexpected line in China treasure box item result packet.");
		GetInterface().ServerMessageBox( pPacket->nRet );
	}
}

void CDnItemTask::OnRecvItemCompleteCharmItem( SCCharmItemComplete *pPacket )
{
	if( pPacket->nRet == ERROR_NONE )
	{
#if defined( PRE_ADD_EASYGAMECASH )
		// -1 means there are multiple item types.
		// For level-up reward box type (must handle here!): handle as -1 for ITEMTYPE_CHARMRANDOM. 
		if( -1 == pPacket->nResultItemID )
		{
			GetInterface().ShowPackageBoxDlg( true );
		}
		else
#endif	// #if defined( PRE_ADD_EASYGAMECASH )
		// If Pandora's box type 
			GetInterface().ShowCharmItemResultDialog( true, pPacket->nResultItemID, pPacket->nItemCount, pPacket->nPeriod );
	}
	else
	{
		ASSERT(0&&"Unexpected line in China treasure box item result packet.");
		GetInterface().ServerMessageBox( pPacket->nRet );
	}

	CDnTradeMail& tradeMail = GetTradeTask().GetTradeMail();
	if (tradeMail.IsOnMailMode())
		tradeMail.LockMailDlg(false);
}

void CDnItemTask::OnRecvItemPetalTokenResult( SCPetalTokenResult *pPacket )
{
	if (pPacket->nUseItemID <= 0) return;

	WCHAR wszStr[256];
#ifdef PRE_MOD_PETAL_WRITE
	swprintf_s( wszStr, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 2030003 ), CDnItem::GetItemName( pPacket->nUseItemID ).c_str(), DN_INTERFACE::UTIL::GetAddCommaString( pPacket->nTotalPetal ).c_str() );
#else // PRE_MOD_PETAL_WRITE
	swprintf_s( wszStr, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3230 ), CDnItem::GetItemName( pPacket->nUseItemID ).c_str(), pPacket->nTotalPetal );
#endif // PRE_MOD_PETAL_WRITE

	GetInterface().AddChatMessage( CHATTYPE_SYSTEM, L"", wszStr, false );
}

void CDnItemTask::OnRecvItemAppellationGainResult( SCAppellationGainResult *pPacket )
{
	WCHAR wszStr[256];
	if( pPacket->nRet == ERROR_NONE ) {
	}
	else {
		swprintf_s( wszStr, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7134 ), CDnAppellationTask::GetAppellationName( pPacket->nArrayIndex ).c_str() );
	}

	GetInterface().AddChatMessage( CHATTYPE_SYSTEM, L"", wszStr, false );
}

void CDnItemTask::OnRecvItemGuildRename( SCGuildRename *pPacket )
{
	if( pPacket->nRet != ERROR_NONE )
	{
		GetInterface().MessageBox( GetEtUIXML().GetUIString( CEtUIXML::idCategory2, pPacket->nRet ), MB_OK );
		if( GetInterface().GetCharRenameDlg() ) GetInterface().GetCharRenameDlg()->EnableControl( true );
	}
	else
	{
		GetInterface().ShowGuildRenameDlg(false);
	}
}

void CDnItemTask::OnRecvItemCharacterRename( SCCharacterRename *pPacket )
{
	if( pPacket->nRet != ERROR_NONE ) {
		if( GetInterface().GetCharRenameDlg() )
			GetInterface().GetCharRenameDlg()->EnableControl( true );
		if( pPacket->nRet == ERROR_DB_CHANGENAME_NEXTDAY ) {
			std::wstring wszTime;
			DN_INTERFACE::STRING::GetTimeText( wszTime, pPacket->tNextChangeDate );
			WCHAR wszTemp[256]={0,};
			swprintf_s( wszTemp, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 4569 ), wszTime.c_str() );
			GetInterface().MessageBox( wszTemp );
		}
		else {
			GetInterface().MessageBox( GetEtUIXML().GetUIString( CEtUIXML::idCategory2, pPacket->nRet ), MB_OK );
		}
	}
	else {
		if( CDnActor::s_hLocalActor ) {
			CDnPlayerActor* pPlayer = dynamic_cast<CDnPlayerActor*>(CDnActor::s_hLocalActor.GetPointer());
			if( pPacket->nUserSessionID == pPlayer->GetUniqueID() ) {
				GetInterface().ShowCharRenameDlg(false);
				CDnActor::s_hLocalActor->SetName( pPacket->wszCharacterName );

				WCHAR wszStr[256] = {0,};
				swprintf_s( wszStr, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 4830 ), pPacket->wszCharacterName );
				GetInterface().AddChatMessage( CHATTYPE_SYSTEM, L"", wszStr, false );
			}
			else {
				DnActorHandle hActor = CDnActor::FindActorFromUniqueID( pPacket->nUserSessionID );
				if( hActor ) {
					CDnPlayerActor *pActor = dynamic_cast<CDnPlayerActor *>(hActor.GetPointer());
					pActor->SetName( pPacket->wszCharacterName );
				}
			}
		}
	}
}

void CDnItemTask::OnRecvEmblemCompoundOpenRes( SCEmblemCompundOpenRes* pPacket )
{
	if( ERROR_NONE == pPacket->nResult )
	{
		((CDnPlateMainDlg *)GetInterface().GetMainMenuDialog(CDnMainMenuDlg::PLATE_MAIN_DIALOG))->OnRecvEmblemCompoundOpen();
	}
}

void CDnItemTask::OnRecvItemCompoundOpenRes( SCItemCompoundOpenRes* pPacket )
{
	if( ERROR_NONE == pPacket->nResult )
	{
		if( GetInterface().GetMainMenuDialog(CDnMainMenuDlg::ITEM_COMPOUND_DIALOG)->IsShow() )
			((CDnItemCompoundTabDlg *)GetInterface().GetMainMenuDialog(CDnMainMenuDlg::ITEM_COMPOUND_DIALOG))->OnRecvItemCompoundOpen();
		else if( GetInterface().GetMainMenuDialog(CDnMainMenuDlg::ITEM_COMPOUND2_DIALOG)->IsShow() )
#ifdef PRE_MOD_ITEM_COMPOUND_DIALOG_RENEWAL
			((CDnItemCompoundRenewalTabDlg *)GetInterface().GetMainMenuDialog(CDnMainMenuDlg::ITEM_COMPOUND2_DIALOG))->OnRecvItemCompoundOpen();
#else
			((CDnItemCompoundTab2Dlg *)GetInterface().GetMainMenuDialog(CDnMainMenuDlg::ITEM_COMPOUND2_DIALOG))->OnRecvItemCompoundOpen();
#endif 
	}
}

void CDnItemTask::OnRecvEmblemCompoundRes( SCCompoundEmblemRes* pPacket )
{
	if( ERROR_NONE == pPacket->sResult )
	{
		// Alarm dialog
		GetInterface().OpenMovieAlarmDlg( "PlateSuccess.avi", 2.0f, pPacket->nResultItemID, 0, -1/*Result.Output.vlItemID[0]*/ );

		// Reset Crest Jewel UI STEP
		((CDnPlateMainDlg *)GetInterface().GetMainMenuDialog(CDnMainMenuDlg::PLATE_MAIN_DIALOG))->OnRecvEmblemCompound();

		// System message.
		GetInterface().AddChatMessage( CHATTYPE_SYSTEM, L"", GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 8036 ), false );
	}
	else if( ERROR_ITEM_EMBLEM_COMPOUND_POSSIBILITY_FAIL == pPacket->sResult )
	{
		GetInterface().OpenMovieAlarmDlg( "PlateFail.avi", 2.0f, pPacket->nResultItemID, 0, 0, 8037 );

		// Reset Crest Jewel UI STEP
		((CDnPlateMainDlg *)GetInterface().GetMainMenuDialog(CDnMainMenuDlg::PLATE_MAIN_DIALOG))->OnRecvEmblemCompound();

		if( pPacket->nResultItemID )
			GetInterface().AddChatMessage( CHATTYPE_SYSTEM, L"", GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 9012 ), false );
		else
			GetInterface().AddChatMessage( CHATTYPE_SYSTEM, L"", GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 8037 ), false );
	}
	else
	{
		if( ERROR_ITEM_EMBLEM_COMPOUND_NOT_ENOUGH_MONEY == pPacket->sResult ||	// Insufficient gold in Crest Jewel combination.
			ERROR_ITEM_EMBLEM_COMPOUND_NOT_ENOUGH_ITEM == pPacket->sResult ||	// Insufficient material items in Crest Jewel combination
			ERROR_ITEM_EMBLEM_COMPOUND_NOT_MATCH_SLOT == pPacket->sResult ||	// Slot mismatch in Crest Jewel combination.
			ERROR_ITEM_EMBLEM_COMPOUND_NOT_ENOUGH_ITEM_COUNT == pPacket->sResult ||	// Slot matches in Crest Jewel combination, but insufficient item count.
			ERROR_ITEM_EMBLEM_COMPOUND_NOT_MATCH_PLATE_WITH_ITEM_COMPOUND_INDEX == pPacket->sResult )
		{
			ASSERT(0&&"Unexpected line in heraldry jewel result packet.");
		}
	}

	m_dqWaitEmblemCompoundReq.pop_back();
	m_bRequestWait = false;
}

void CDnItemTask::OnRecvItemCompoundRes( SCCompoundItemRes* pPacket )
{
	if( ERROR_NONE == pPacket->sResult )
	{
		// Alarm dialog
		GetInterface().OpenMovieAlarmDlg( "ItemCombineSuccess.avi", 2.0f, pPacket->nResultItemID, pPacket->cLevel, pPacket->cOption );

		// Initialize item synthesis UI STEP
		if( GetInterface().GetMainMenuDialog(CDnMainMenuDlg::ITEM_COMPOUND_DIALOG)->IsShow() )
			((CDnItemCompoundTabDlg *)GetInterface().GetMainMenuDialog(CDnMainMenuDlg::ITEM_COMPOUND_DIALOG))->OnRecvItemCompound();
		else if( GetInterface().GetMainMenuDialog(CDnMainMenuDlg::ITEM_COMPOUND2_DIALOG)->IsShow() )
#ifdef PRE_MOD_ITEM_COMPOUND_DIALOG_RENEWAL
			((CDnItemCompoundRenewalTabDlg*)GetInterface().GetMainMenuDialog(CDnMainMenuDlg::ITEM_COMPOUND2_DIALOG))->RefreshItemList();
#else
			((CDnItemCompoundTab2Dlg *)GetInterface().GetMainMenuDialog(CDnMainMenuDlg::ITEM_COMPOUND2_DIALOG))->OnRecvItemCompound();
#endif 

		// System message.
		GetInterface().AddChatMessage( CHATTYPE_SYSTEM, L"", GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7010 ), false );
	}
	else if( ERROR_ITEM_ITEM_COMPOUND_POSSIBILITY_FAIL == pPacket->sResult )
	{
		GetInterface().OpenMovieAlarmDlg( "ItemCombineFail.avi", 2.0f, pPacket->nResultItemID, 0, 0, 7011 );

		// Initialize item synthesis UI STEP
		if( GetInterface().GetMainMenuDialog(CDnMainMenuDlg::ITEM_COMPOUND_DIALOG)->IsShow() )
			((CDnItemCompoundTabDlg *)GetInterface().GetMainMenuDialog(CDnMainMenuDlg::ITEM_COMPOUND_DIALOG))->OnRecvItemCompound();
		else if( GetInterface().GetMainMenuDialog(CDnMainMenuDlg::ITEM_COMPOUND2_DIALOG)->IsShow() )
#ifdef PRE_MOD_ITEM_COMPOUND_DIALOG_RENEWAL
			((CDnItemCompoundRenewalTabDlg*)GetInterface().GetMainMenuDialog(CDnMainMenuDlg::ITEM_COMPOUND2_DIALOG))->GetCurrentItemCompoundList()->OnRecvItemCompound();
#else
			((CDnItemCompoundTab2Dlg *)GetInterface().GetMainMenuDialog(CDnMainMenuDlg::ITEM_COMPOUND2_DIALOG))->OnRecvItemCompound();
#endif

		if( pPacket->nResultItemID )
			GetInterface().AddChatMessage( CHATTYPE_SYSTEM, L"", GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 9012 ), false );
		else
			GetInterface().AddChatMessage( CHATTYPE_SYSTEM, L"", GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7011 ), false );
	}
	else
	{
		if( ERROR_ITEM_ITEM_COMPOUND_NOT_ENOUGH_MONEY == pPacket->sResult ||	// Insufficient gold in Crest Jewel combination.
			ERROR_ITEM_ITEM_COMPOUND_NOT_ENOUGH_ITEM == pPacket->sResult ||	// Insufficient material items in Crest Jewel combination
			ERROR_ITEM_ITEM_COMPOUND_NOT_MATCH_SLOT == pPacket->sResult ||	// Slot mismatch in Crest Jewel combination.
			ERROR_ITEM_ITEM_COMPOUND_NOT_ENOUGH_ITEM_COUNT == pPacket->sResult )	// Slot matches in Crest Jewel combination, but insufficient item count.
		{
			ASSERT(0&&"Unexpected line in item compound result packet.");
		}
	}

	m_dqWaitEmblemCompoundReq.pop_back();
	m_bRequestWait = false;

#ifdef PRE_MOD_ITEM_COMPOUND_DIALOG_RENEWAL
	CDnItemCompoundRenewalTabDlg* pCompoundRenewalDlg = ((CDnItemCompoundRenewalTabDlg*)GetInterface().GetMainMenuDialog(CDnMainMenuDlg::ITEM_COMPOUND2_DIALOG));
	if( pCompoundRenewalDlg && pCompoundRenewalDlg->IsShow() )
	{
		int nRemoteItemID = pCompoundRenewalDlg->GetRemoteItemID();	
		if( nRemoteItemID > 0 )
		{
			CDnItem* pCashItem = FindItem( nRemoteItemID, ITEM_SLOT_TYPE::ST_INVENTORY_CASH );
			if( ! pCashItem )
			{
				pCompoundRenewalDlg->Show( false );
			}
		}
	}
#endif 

	//if( ERROR_NONE == pPacket->sResult )
	//{
	//	// Heraldry jewel combination success
	//	EmblemCompoundResult& Result = m_dqWaitEmblemCompoundReq.back();

	//	GetInterface().MessageBox( GetEtUIXML().GetUIString( CEtUIXML::idCategory2, 412 ), MB_OK );
	//	GetInterface().ShowEmblemDialog( false );
	//}
	//else
	//	GetInterface().ServerMessageBox(pPacket->sResult);

	//m_dqWaitEmblemCompoundReq.pop_back();
	//m_bRequestWait = false;
}

void CDnItemTask::OnRecvEmblemCompoundCancelRes( SCEmblemCompoundCancelRes* pPacket )
{
	if( ERROR_NONE == pPacket->nResult )
		GetInterface().CloseMovieProcessDlg( true );

	m_bRequestWait = false;
}

void CDnItemTask::OnRecvItemCompoundCancelRes( SCItemCompoundCancelRes* pPacket )
{
	if( ERROR_NONE == pPacket->nResult )
		GetInterface().CloseMovieProcessDlg( true );

	m_bRequestWait = false;
}

void CDnItemTask::OnRecvItemDisjointRes( SCItemDisjointRes* pPacket )
{
	if( pPacket->bSuccess )
	{
		CDnItemDisjointDlg *pDisjointDlg = (CDnItemDisjointDlg*)GetInterface().GetMainMenuDialog( CDnMainMenuDlg::ITEM_DISJOINT_DIALOG );
		if( pDisjointDlg && pDisjointDlg->GetDisjointResultDlg() && pPacket->nItemID > 0 )
		{
			pDisjointDlg->GetDisjointResultDlg()->AddResultItem( pPacket->nItemID, 1 );
		}
	}
	else
	{
		// Item disassembly failed.
		WCHAR wszStr[256];
		wsprintf( wszStr, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 8101 ), CDnItem::GetItemFullName( pPacket->nItemID ).c_str() );
		GetInterface().AddChatMessage( CHATTYPE_SYSTEM, L"", wszStr, false );
	}
}



void CDnItemTask::OnRecvGlyphSlotCoolTime( SCGlyphTimeInfo* pPacket )
{
	_ASSERT( CDnActor::s_hLocalActor && "Equipment skill cooldown setup: Local actor object is invalid." );
	if( CDnActor::s_hLocalActor )
	{
		CDnLocalPlayerActor* pLocalPlayerActor = static_cast<CDnLocalPlayerActor*>( CDnActor::s_hLocalActor.GetPointer() );
		pLocalPlayerActor->SetEquipCoolTime( pPacket->nDelayTime, pPacket->nRemainTime, EQUIPMAX );
	}
}

void CDnItemTask::DetachAllParts( MAPartsBody* pActor )
{
	if( pActor == NULL )
		return;

	for( int i=0; i<EQUIPMAX; i++ )
		pActor->DetachParts( (CDnParts::PartsTypeEnum)i );

	for( int i=CASHEQUIP_HELMET; i<=CASHEQUIP_FAIRY; i++ )
		pActor->DetachCashParts( (CDnParts::PartsTypeEnum)i );
}

void CDnItemTask::OnRecvItemEquipList( SCEquipList *pPacket )
{
	if( CDnWorld::IsActive() && CDnWorld::GetInstance().GetMapType() == CDnWorld::MapTypeVillage ) {
		CDnVillageTask *pTask = (CDnVillageTask *)CTaskManager::GetInstance().GetTask( "VillageTask" );
		if( pTask && pTask->IsChannelMove() ) return;
	}

	CDnLocalPlayerActor *pActor = NULL;
	if( CDnActor::s_hLocalActor ) {
		pActor = dynamic_cast<CDnLocalPlayerActor *>(CDnActor::s_hLocalActor.GetPointer());
	}
	if( pActor ) pActor->SetBattleMode( true );

	DetachAllParts( pActor );
	RemoveAllEquipItem();
	RemoveAllCashEquipItem();
	RemoveAllGlyphItem();
#if defined(PRE_ADD_TALISMAN_SYSTEM)
	RemoveAllTalismanItem();
#endif
	TItemInfo *pInfo = &pPacket->EquipArray[0];
	for ( int i = 0; i < pPacket->cCount; i++ ) {
		if( pInfo->Item.nItemID <= 0 ) {
			pInfo++;
			continue;
		}

		CDnItem *pItem = CDnItemTask::GetInstance().CreateItem( *pInfo );
		if( pItem == NULL ) {
			pInfo++;
			continue;
		}

		if( pInfo->cSlotIndex < EQUIP_WEAPON1) {
			CDnItemTask::GetInstance().InsertEquipItem( pItem );
			if( dynamic_cast<CDnParts*>(pItem) && pItem->GetItemType() == ITEMTYPE_PARTS ) {
				DnPartsHandle hParts = ((CDnParts*)pItem)->GetMySmartPtr();
				if( pActor ) pActor->AttachParts( hParts, (CDnParts::PartsTypeEnum)pInfo->cSlotIndex );
			}
		}
		else {
			CDnItemTask::GetInstance().InsertEquipItem( pItem );
			if( dynamic_cast<CDnWeapon*>(pItem) && pItem->GetItemType() == ITEMTYPE_WEAPON ) {
				DnWeaponHandle hWeapon = ((CDnWeapon*)pItem)->GetMySmartPtr();
				if( pActor ) pActor->AttachWeapon( hWeapon, pInfo->cSlotIndex - EQUIP_WEAPON1 );
			}
		}
		pInfo++;
	}
	for ( int i = 0; i < pPacket->cCashCount; i++ ) {
		if( pInfo->Item.nItemID <= 0 ) {
			pInfo++;
			continue;
		}

		CDnItem *pItem = CDnItemTask::GetInstance().CreateItem( *pInfo );
		if( pItem == NULL ) {
			pInfo++;
			continue;
		}

		if( pInfo->cSlotIndex < CASHEQUIP_WEAPON1) {
			CDnItemTask::GetInstance().InsertCashEquipItem( pItem );
			if( dynamic_cast<CDnParts*>(pItem) && pItem->GetItemType() == ITEMTYPE_PARTS ) {
				DnPartsHandle hParts = ((CDnParts*)pItem)->GetMySmartPtr();
				if( pActor ) pActor->AttachCashParts( hParts, (CDnParts::PartsTypeEnum)pInfo->cSlotIndex );
			}
		}
		else if(pInfo->cSlotIndex == CASHEQUIP_WEAPON1 || pInfo->cSlotIndex == CASHEQUIP_WEAPON2)
		{
			CDnItemTask::GetInstance().InsertCashEquipItem( pItem );
			if( dynamic_cast<CDnWeapon*>(pItem) && pItem->GetItemType() == ITEMTYPE_WEAPON ) {
				DnWeaponHandle hWeapon = ((CDnWeapon*)pItem)->GetMySmartPtr();
				if( pActor ) pActor->AttachCashWeapon( hWeapon, pInfo->cSlotIndex - CASHEQUIP_WEAPON1, false, false );
			}
		}
		else if(pInfo->cSlotIndex == CASHEQUIP_EFFECT)
		{
			CDnItemTask::GetInstance().InsertCashEquipItem( pItem );

			if(dynamic_cast<CDnPartsVehicleEffect*>(pItem) && pItem->GetItemType() == ITEMTYPE_VEHICLEEFFECT)
			{
				DnPartsVehicleEffectHandle hVehicleEffect = ((CDnPartsVehicleEffect*)pItem)->GetMySmartPtr();
				if(pActor) pActor->SetPartsVehicleEffect(hVehicleEffect);
			}
		}

		pInfo++;
	}
	if( pActor ) pActor->RefreshHideHelmet();

	for( int i=0; i<pPacket->cGlyphCount; i++ ) {
		if( pInfo->Item.nItemID <= 0 ) {
			pInfo++;
			continue;
		}

		CDnItem *pItem = CDnItemTask::GetInstance().CreateItem( *pInfo );
		if( pItem == NULL ) {
			pInfo++;
			continue;
		}
		CDnItemTask::GetInstance().InsertGlyphItem( pItem );
		if( dynamic_cast<CDnGlyph*>(pItem) && pItem->GetItemType() == ITEMTYPE_GLYPH ) {
			DnGlyphHandle hGlyph = ((CDnGlyph*)pItem)->GetMySmartPtr();
			if( pActor ) pActor->AttachGlyph( hGlyph, (CDnGlyph::GlyphSlotEnum)pInfo->cSlotIndex );
		}
		pInfo++;
	}

#if defined(PRE_ADD_TALISMAN_SYSTEM)
	m_nTalismanOpenFlag = pPacket->nTalismanOpenFlag;
	for( int i=0; i<pPacket->cTalismanCount; i++ ) {
		if( pInfo->Item.nItemID <= 0 ) {
			pInfo++;
			continue;
		}

		CDnItem *pItem = CDnItemTask::GetInstance().CreateItem( *pInfo );
		if( pItem == NULL ) {
			pInfo++;
			continue;
		}
		CDnItemTask::GetInstance().InsertTalismanItem( pItem );
		if( dynamic_cast<CDnTalisman*>(pItem) && pItem->GetItemType() == ITEMTYPE_TALISMAN ) {
			DnTalismanHandle hTalisman = ((CDnTalisman*)pItem)->GetMySmartPtr();
			if( pActor ) pActor->AttachTalisman( hTalisman, pInfo->cSlotIndex, GetTalismanEfficiency(pInfo->cSlotIndex) );
		}
		pInfo++;
	}
	GetInterface().RefreshTalismanOpenSlot();
#endif


	if( pActor ) {
		pActor->RefreshState();
		pActor->CombineParts();
		if( CDnWorld::IsActive() && CDnWorld::GetInstance().GetMapType() == CDnWorld::MapTypeVillage ) {
			pActor->SetBattleMode( false );
		}

		// Since Entered and EquipData were split on the Village side, let's set it here once too.
		CDnLocalPlayerActor::LockInput( true );
		pActor->ResetActor();
		pActor->SetVelocityY( -1000.f );
		pActor->SetResistanceY( -1000.f );
		pActor->Process( 0, 0.f );
		CDnLocalPlayerActor::LockInput( false );

		if( CDnMainFrame::IsActive() )
			CDnMainFrame::GetInstance().FlushWndMessage();

		GetInterface().ShowDungeonOpenNoticeDialog( true );
	}

	GetInterface().UpdateMyPortrait();
	GetInterface().RefreshPlayerDurabilityIcon();
}

void CDnItemTask::OnRecvItemUserChangeGlyph(SCChangeGlyph *pPacket)
{
	DnActorHandle hActor = CDnActor::FindActorFromUniqueID( pPacket->nSessionID );
	if( !hActor ) return;

	CDnPlayerActor *pActor = dynamic_cast<CDnPlayerActor *>(hActor.GetPointer());
	CDnItem *pItem = CreateItem( pPacket->ItemInfo );

	pActor->DetachGlyph( (CDnGlyph::GlyphSlotEnum)pPacket->ItemInfo.cSlotIndex );

	if( pItem && dynamic_cast<CDnGlyph*>(pItem) && pItem->GetItemType() == ITEMTYPE_GLYPH ) {
		DnGlyphHandle hGlyph = ((CDnGlyph*)pItem)->GetMySmartPtr();
		pActor->AttachGlyph( hGlyph, (CDnGlyph::GlyphSlotEnum)pPacket->ItemInfo.cSlotIndex, true );			
	}

	pActor->RefreshState();
}

void CDnItemTask::OnRecvItemEnchant( SCEnchantItem* pPacket )
{
	m_bRequestWait = false;
	if (ERROR_NONE == pPacket->nRet)
	{
#if defined(PRE_ADD_EQUIPED_ITEM_ENCHANT)
		//Handle appropriately according to pPacket->bEnchantEquipedItem.		
		((CDnItemUpgradeDlg*)GetInterface().GetMainMenuDialog(CDnMainMenuDlg::ITEM_UPGRADE_DIALOG))->OnRecvEnchant(pPacket->cItemIndex);
#else
		((CDnItemUpgradeDlg*)GetInterface().GetMainMenuDialog(CDnMainMenuDlg::ITEM_UPGRADE_DIALOG))->OnRecvEnchant(pPacket->cInvenIndex);
#endif
	}
	else
	{
		OutputDebug("EnchantResult : %d",pPacket->nRet);
		// Treat this item as non-enhanceable.
		// This won't happen unless the client is tampered with, since the UI checks conditions before requesting.
		GetInterface().MessageBox( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 8007 ), MB_OK );
	}
}

void CDnItemTask::OnRecvItemEnchantComplete( SCEnchantComplete* pPacket )
{
	OutputDebug("%s result: %d",__FUNCTION__, pPacket->nRet);
	m_bRequestWait = false;
	switch( pPacket->nRet ) 
	{
		case ERROR_NONE:
			{
				// Initialize enhancement UI and handle notifications
				((CDnItemUpgradeDlg*)GetInterface().GetMainMenuDialog(CDnMainMenuDlg::ITEM_UPGRADE_DIALOG))->OnRecvEnchantComplete(1, pPacket->nItemID, pPacket->cLevel, pPacket->cOption);
			// System message.
				GetInterface().AddChatMessage( CHATTYPE_SYSTEM, L"", GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 8011 ), false );
				// Play sound
				PlayEnchantResultSound(10036);
			}
			break;
		case ERROR_ITEM_ENCHANT_POSSIBILITYFAIL_BREAKITEM:
			{

				((CDnItemUpgradeDlg*)GetInterface().GetMainMenuDialog(CDnMainMenuDlg::ITEM_UPGRADE_DIALOG))->OnRecvEnchantComplete(0, pPacket->nItemID, 0, 0);
			
				if( pPacket->nItemID )
					GetInterface().AddChatMessage( CHATTYPE_SYSTEM, L"", GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 8009 ), false );
				else
					GetInterface().AddChatMessage( CHATTYPE_SYSTEM, L"", GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 8010 ), false );

				// Play sound
				PlayEnchantResultSound(10920);
			}
			break;
		case ERROR_ITEM_ENCHANT_POSSIBILITYFAIL_DOWNLEVEL:
			{
				((CDnItemUpgradeDlg*)GetInterface().GetMainMenuDialog(CDnMainMenuDlg::ITEM_UPGRADE_DIALOG))->OnRecvEnchantComplete(2, pPacket->nItemID, pPacket->cLevel, pPacket->cOption);

				WCHAR wszStr[256];
				swprintf_s( wszStr, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 8023 ), pPacket->cLevel );
				GetInterface().AddChatMessage( CHATTYPE_SYSTEM, L"", wszStr, false );

				// Play sound
				PlayEnchantResultSound(10037);
			}
			break;
		case ERROR_ITEM_ENCHANT_POSSIBILITYFAIL_DOWNLEVELZERO:
			{
				((CDnItemUpgradeDlg*)GetInterface().GetMainMenuDialog(CDnMainMenuDlg::ITEM_UPGRADE_DIALOG))->OnRecvEnchantComplete(3, pPacket->nItemID, pPacket->cLevel, pPacket->cOption);
			
				WCHAR wszStr[256];
				swprintf_s( wszStr, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 8025 ) );
				GetInterface().AddChatMessage( CHATTYPE_SYSTEM, L"", wszStr, false );

				// Play sound
				PlayEnchantResultSound(10037);
			}
			break;
		case ERROR_ITEM_ENCHANTFAIL:
		case ERROR_ITEM_NOTFOUND:
		case ERROR_ITEM_INSUFFICIENCY_MONEY:
			ASSERT(0&&"Unexpected line in enhancement result packet.");
			((CDnItemUpgradeDlg*)GetInterface().GetMainMenuDialog(CDnMainMenuDlg::ITEM_UPGRADE_DIALOG))->OnRecvEnchantComplete( false, 0, 0, 0 );
			break;
		default:
			((CDnItemUpgradeDlg*)GetInterface().GetMainMenuDialog(CDnMainMenuDlg::ITEM_UPGRADE_DIALOG))->OnRecvEnchantComplete( false, 0, 0, 0 );
			break;
	}

	if( pPacket->nRet == ERROR_NONE )
		GetPetTask().DoPetChat( PET_CHAT_USER_ENCHANT_SUCCESS );
	else
		GetPetTask().DoPetChat( PET_CHAT_USER_ENCHANT_FAIL );
}

void CDnItemTask::OnRecvItemEnchantCancel( SCEnchantCancel* pPacket )
{
	m_bRequestWait = false;
	if( ERROR_NONE == pPacket->nRet )
		GetInterface().CloseMovieProcessDlg( true );
}

void CDnItemTask::OnRecvInventoryMaxCount( SCInventoryMaxCount* pPacket )
{
	if (pPacket->cInvenMaxCount > 0)
	{
		GetItemTask().GetCharInventory().SetUsableSlotCount( pPacket->cInvenMaxCount );
	}
}

void CDnItemTask::OnRecvWarehouseMaxCount( SCWarehouseMaxCount* pPacket )
{
	if (pPacket->cWareMaxCount > 0)
	{
		GetItemTask().GetStorageInventory().SetUsableSlotCount( pPacket->cWareMaxCount );
	}
}

void CDnItemTask::OnRecvRebirthCoin( SCRebirthCoin* pPacket )
{
	if (pPacket->nRet == ERROR_NONE)
	{
		SetRebirthCoin(eDAILYCOIN, pPacket->cRebirthCoin);
		SetRebirthCoin(eCASHCOIN, pPacket->wRebirthCashCoin);
		SetRebirthCoin(ePCBANGCOIN, pPacket->cPCBangRebirthCoin);
#ifdef PRE_ADD_VIP
		SetRebirthCoin(eVIPCOIN, pPacket->cVIPRebirthCoin);
#endif
		SetUsableRebirthCoin( pPacket->nUsableRebirthCoin );

		if (pPacket->nRebirthType != _REBIRTH_SELF)
		{
			std::wstring str, rebirth_name;
			DnActorHandle hOtherMember = CDnActor::FindActorFromUniqueID( pPacket->nSessionID );
			if( !hOtherMember ) return;
			rebirth_name = hOtherMember->GetName();
			if (pPacket->nRebirthType == _REBIRTH_REBIRTHEE)
				str = FormatW(GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 914), rebirth_name.c_str());	// Revived %s.
			else if (pPacket->nRebirthType == _REBIRTH_REBIRTHER)
				str = FormatW(GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 915), rebirth_name.c_str());	// Resurrected by %s.

			GetInterface().AddChatMessage(CHATTYPE_SYSTEM, L"", str.c_str(), false);
		}

#ifdef PRE_FIX_PARTYRESTORE_DGNCLEAR
		if (CDnActor::s_hLocalActor)
		{
			CDnLocalPlayerActor *pActor = ((CDnLocalPlayerActor *)CDnActor::s_hLocalActor.GetPointer());
			if (pActor && pActor->IsGhost())
			{
				if (IsCanRebirth())
				{
					if (GetInterface().IsShowRebirthCationDlg() == false)
						GetInterface().OpenRebirthCaptionDlg();
				}
				else
				{
					if (GetInterface().IsShowRebirthCationDlg())
						GetInterface().CloseRebirthCaptionDlg();
				}
			}
		}
#endif

#ifdef PRE_MOD_NESTREBIRTH
		SetUpdateRebirthCoinOrItem(true);
#endif
	}
	else
	{
		//blondy handled as requested in bug #4739
		// Requested to only remove the message in PvP
		if( !IsPVP() )
			GetInterface().AddChatMessage( CHATTYPE_SYSTEM, L"", GetEtUIXML().GetUIString( CEtUIXML::idCategory2, pPacket->nRet ), false );
		//blondy
	}
}

void CDnItemTask::OnRecvSortInventory( SCSortInventory *pPacket )
{
	if( pPacket->nRet == ERROR_NONE )
	{
		if( m_CharInventory.Sort( true ) )
		{
			if( m_nItemSortSound != -1 )
				CEtSoundEngine::GetInstance().PlaySound( "2D", m_nItemSortSound, false );
			m_bRequestWait = false;
			return;
		}
		else
		{
			// If the server sent OK but the client failed, just disconnect. Such cases rarely happen in practice.
			GetInterface().MessageBox( 5061, MB_OK, MESSAGEBOX_10, CDnBridgeTask::GetInstancePtr(), true, false, false, true );
		}
	}
	else
	{
		m_CharInventory.Sort( false );
	}

	m_bRequestWait = false;

	// Sort failed (either server failed or client m_CharInventory.Sort(true) failed)
	GetInterface().AddChatMessage( CHATTYPE_SYSTEM, L"", GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 526 ), true );
}

void CDnItemTask::OnRecvDecreaseDurabilityInventory( SCDecreaseDurabilityInventory *pPacket )
{
	switch( pPacket->cType ) {
		case 0: m_CharInventory.DecreaseDurability( pPacket->nAbsolute ); break;
		case 1: m_CharInventory.DecreaseDurability( pPacket->fRatio ); break;
	}
}

void CDnItemTask::OnRecvSortWarehouse( SCSortWarehouse * pPacket )
{
	if( pPacket->nRet == ERROR_NONE )
	{
		if( m_StorageInventory.Sort( true ) )
		{
			if( m_nItemSortSound != -1 )
				CEtSoundEngine::GetInstance().PlaySound( "2D", m_nItemSortSound, false );
			m_bRequestWait = false;
			return;
		}
		else
		{
			// If the server sent OK but the client failed, just disconnect. Such cases rarely happen in practice.
			GetInterface().MessageBox( 5061, MB_OK, MESSAGEBOX_10, CDnBridgeTask::GetInstancePtr(), true, false, false, true );
		}
	}
	else
	{
		m_StorageInventory.Sort( false );
	}

	m_bRequestWait = false;

	// Sort failed (either server failed or client m_CharInventory.Sort(true) failed)
	GetInterface().AddChatMessage( CHATTYPE_SYSTEM, L"", GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 526 ), true );
}

void CDnItemTask::OnRecvUnsealItem(SCUnSealItem* pData)
{
	if (ERROR_NONE == pData->nRet)
	{
		CDnItem* pUsingItem = m_CharInventory.GetItem(pData->cInvenIndex);
		if (pUsingItem)
		{
			std::wstring str;
			str = FormatW(GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 4131), CDnItem::GetItemFullName(pUsingItem->GetClassID()).c_str());	// UISTRING : Item %s has been bound.
			GetInterface().AddChatMessage(CHATTYPE_SYSTEM, L"", str.c_str(), false);

			GetInterface().GetItemUnsealDlg()->OnRecvUnSealItem(pData->nRet);
			GetInterface().CloseItemUnsealDialog();
		}
	}
	else
	{
		GetInterface().ServerMessageBox(pData->nRet);
		GetInterface().CloseItemUnsealDialog();
	}
}

void CDnItemTask::OnRecvSealItem(SCSealItem* pData)
{
	GetInterface().GetItemSealDlg()->OnRecvSealItem( pData->nRet, pData->cInvenIndex );

	if (ERROR_NONE == pData->nRet)
	{
		CDnItem* pItem = m_CharInventory.GetItem(pData->cInvenIndex);
		if (pItem)
		{
			std::wstring str;
			str = FormatW(GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 4132), pItem->GetName());
			GetInterface().AddChatMessage(CHATTYPE_SYSTEM, L"", str.c_str(), false);
		}
	}
	else if (ERROR_ITEM_SEAL_FAIL_BY_RANDOM == pData->nRet)
	{
		CDnItem* pItem = m_CharInventory.GetItem(pData->cInvenIndex);
		if (pItem)
		{
			std::wstring str;
			str = FormatW(GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 4139), pItem->GetName());
			GetInterface().AddChatMessage(CHATTYPE_SYSTEM, L"", str.c_str(), false);
		}
	}
	else if (ERROR_ITEM_UNSEAL_ALREADY == pData->nRet)
	{
		ASSERT(0&&"Unexpected line in seal result packet.");
	}
	else
	{
		GetInterface().ServerMessageBox(pData->nRet);
	}
}

void CDnItemTask::OnRecvBroadcastingEffect( SCBroadcastingEffect *pPacket )
{
	DnActorHandle hActor = CDnActor::FindActorFromUniqueID( pPacket->nSessionID );
	if( !hActor ) return;

	hActor->SetHeadEffect( (eBroadcastingEffect)pPacket->cType, (eBroadcastingEffect)pPacket->cState );
}

void CDnItemTask::OnRecvItemRequestCashWorldMsg( SCRequestCashWorldMsg *pPacket )
{
	if( !m_CashInventory.FindItemFromSerialID( pPacket->biItemSerial ) )
		return;

	GetInterface().OpenWorldMsgDialog( pPacket->cType, pPacket->biItemSerial );
}

#ifdef PRE_ADD_GACHA_JAPAN
void CDnItemTask::OnRecvItemGachaOpen_JP( SCGachaShopOpen_JP* pPacket )
{
	GetInterface().ShowGachaDialog( pPacket->nGachaShopIndex );
}

void CDnItemTask::OnRecvItemGachaRes_JP( SCGachaRes_JP* pPacket )
{
	// Show a message box when gacha roll fails in dev build only.
	// Clear all contents of the gacha roulette dialog.
#ifndef _FINAL_BUILD
	switch( pPacket->nRet )
	{
		case ERROR_NONE:
			//GetInterface().OnRecvGachaRes_JP( pPacket->nResultItemID );
			break;

		// Error messages from here on. If the table is not broken, packet tampering is likely...

			// An item for an incompatible class was returned as the result.
		case ERROR_GACHAPON_JP_MISMATCH_JOBCLASS:
			GetInterface().MessageBox( L"An item for an incompatible class was returned as the result." );
			break;

			// Request packet arrived despite insufficient coins.
		case ERROR_GACHAPON_JP_NOT_ENOUGH_COIN:
			GetInterface().MessageBox( L"Gacha was executed despite insufficient coins." );
			break;

			// Item not found.
		case ERROR_GACHAPON_JP_INVALID_RESULT_ITEM:
			GetInterface().MessageBox( L"Item data not found.");
			break;
	}
#endif
}

void CDnItemTask::OnStopGachaRoulette( void )
{
	OnRecvItemRefreshCashInven( &m_GachaResultCashItemPacket );
	SecureZeroMemory( &m_GachaResultCashItemPacket, sizeof(m_GachaResultCashItemPacket) );
}
#endif // PRE_ADD_GACHA_JAPAN

void CDnItemTask::OnRecvItemCosMixOpen(SCCosMixOpen* pPacket)
{
	CDnCostumeMixDlg* pDlg = GetInterface().GetCostumeMixDlg();
	if (pDlg)
		pDlg->DisableAllDlgs(false, GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 130317)); // UISTRING : Preparing costume synthesis.

	m_bLockCostumeSendPacket = false;

	if (pPacket->nRet == ERROR_NONE)
	{
		GetInterface().ShowCostumeMixDlg(true, false);
	}
	else
	{
		CDnLocalPlayerActor::LockInput( false );
		GetInterface().ServerMessageBox(pPacket->nRet);
		GetInterface().ShowCostumeMixDlg(false, false);
	}
}

void CDnItemTask::OnRecvItemCosMixClose(SCCosMixClose* pPacket)
{
	m_bLockCostumeSendPacket = false;

	if (pPacket->nRet != ERROR_NONE)
		GetInterface().ServerMessageBox(pPacket->nRet);
		GetInterface().ShowCostumeMixDlg(false, true);
}

void CDnItemTask::OnRecvItemCosMixCloseComplete(SCCosMixClose* pPacket)
{
	m_bLockCostumeSendPacket = false;

	if (pPacket->nRet == ERROR_NONE)
	{
		GetInterface().ShowCostumeMixDlg(false, false);
	}
	else
	{
		GetInterface().ShowCostumeMixDlg(false, true);
		GetInterface().ServerMessageBox(pPacket->nRet);
	}
}

void CDnItemTask::OnRecvItemCosMixComplete(SCCosMix* pPacket)
{
	m_bLockCostumeSendPacket = false;

	if (pPacket->nRet == ERROR_NONE)
	{
		CDnCostumeMixDlg* pDlg = GetInterface().GetCostumeMixDlg();
		if (pDlg)
			pDlg->CompleteMix(pPacket->nResultItemID, pPacket->cOption);
	}
	else
	{
		GetInterface().ShowCostumeMixDlg(false, false);

		GetInterface().ServerMessageBox(pPacket->nRet);
	}
}

void CDnItemTask::OnRecvItemCosDesignMixOpen(SCCosDesignMixOpen* pData)
{
	CDnCostumeDesignMixDlg* pDlg = GetInterface().GetCostumeDesignMixDlg();
	if (pDlg)
		pDlg->DisableAllDlgs(false, GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 130317)); // UISTRING : Preparing costume synthesis.

	m_bLockCosDesignMixSendPacket = false;

	if (pData->nRet == ERROR_NONE)
	{
		GetInterface().ShowCostumeDesignMixDlg(true, false);
	}
	else
	{
		CDnLocalPlayerActor::LockInput( false );
		GetInterface().ServerMessageBox(pData->nRet);
		GetInterface().ShowCostumeMixDlg(false, false);
	}
}

void CDnItemTask::OnRecvItemCosDesignMixClose(SCCosDesignMixClose* pPacket)
{
	m_bLockCosDesignMixSendPacket = false;

		GetInterface().ShowCostumeDesignMixDlg(false, true);
	if (pPacket->nRet != ERROR_NONE)
		GetInterface().ServerMessageBox(pPacket->nRet);
}

void CDnItemTask::OnRecvItemCosDesignMixCloseComplete(SCCosDesignMixClose* pPacket)
{
	m_bLockCosDesignMixSendPacket = false;

	GetInterface().ShowCostumeDesignMixDlg(false, false);
	if (pPacket->nRet != ERROR_NONE)
		GetInterface().ServerMessageBox(pPacket->nRet);
}

void CDnItemTask::OnRecvItemCosDesignMixComplete(SCCosDesignMix* pPacket)
{
	m_bLockCosDesignMixSendPacket = false;

	if (pPacket->nRet == ERROR_NONE)
	{
		CDnItem* pCompleteItem = m_CashInventory.FindItemFromSerialID(pPacket->biItemSertial);
		if (pCompleteItem)
		{
			pCompleteItem->SetLookItemID(pPacket->nLookItemID, true);
			pCompleteItem->SetNewGain(true, true);

			CDnCostumeDesignMixDlg* pDlg = GetInterface().GetCostumeDesignMixDlg();
			if (pDlg)
				pDlg->CompleteMix(pCompleteItem);

			CDnInvenTabDlg* pInvenTabDlg = (CDnInvenTabDlg*)GetInterface().GetMainMenuDialog( CDnMainMenuDlg::INVENTORY_DIALOG );
			if( pInvenTabDlg )
				pInvenTabDlg->ShowTabNew( ST_INVENTORY_CASH );
		}
	}
	else
	{
		GetInterface().ShowCostumeDesignMixDlg(false, false);
		GetInterface().ServerMessageBox(pPacket->nRet);
	}
}

#ifdef PRE_ADD_COSRANDMIX
void CDnItemTask::OnRecvItemCosRandomMixOpen(SCCosRandomMixOpen* pData)
{
	CDnCostumeRandomMixDlg* pDlg = GetInterface().GetCostumeRandomMixDlg();
	if (pDlg)
		pDlg->DisableAllDlgs(false, GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 130317)); // UISTRING : Preparing costume synthesis.

	m_bLockCosRandomMixSendPacket = false;

#ifdef PRE_ADD_COSRANDMIX_ACCESSORY
	if (pData->nRet == ERROR_NONE || pData->nOpenType != CostumeMix::RandomMix::OpenFail)
	{
		m_CosRandMixDataMgr.OnStartMixDlg((CostumeMix::RandomMix::eOpenType)pData->nOpenType);
		GetInterface().ShowCostumeRandomMixDlg(true, false);
	}
	else
	{
		m_CosRandMixDataMgr.OnStartMixDlg(CostumeMix::RandomMix::OpenFail);
		CDnLocalPlayerActor::LockInput( false );
		if (pData->nRet != ERROR_NONE)
		{
			GetInterface().ServerMessageBox(pData->nRet);
		}
		else
		{
			if (pData->nOpenType == CostumeMix::RandomMix::OpenFail)
				GetInterface().MessageBox(130342, MB_OK); // UISTRING : Cannot open the synthesis window
		}

		GetInterface().ShowCostumeMixDlg(false, false);
	}
#else // PRE_ADD_COSRANDMIX_ACCESSORY
	if (pData->nRet == ERROR_NONE)
	{
		GetInterface().ShowCostumeRandomMixDlg(true, false);
	}
	else
	{
		CDnLocalPlayerActor::LockInput( false );
		GetInterface().ServerMessageBox(pData->nRet);
		GetInterface().ShowCostumeMixDlg(false, false);
	}
#endif
}

void CDnItemTask::OnRecvItemCosRandomMixClose(SCCosRandomMixClose* pData)
{
	m_bLockCosRandomMixSendPacket = false;

	GetInterface().ShowCostumeRandomMixDlg(false, true);

	if (pData->nRet != ERROR_NONE)
	{
		GetInterface().ServerMessageBox(pData->nRet);
	}
#ifdef PRE_ADD_COSRANDMIX_ACCESSORY
	else
	{
		CostumeMix::RandomMix::eOpenType openType = m_CosRandMixDataMgr.GetCurrentOpenType();
		if (openType == CostumeMix::RandomMix::OpenByNpc)
			GetInterface().CloseBlind();
	}
#endif
}

void CDnItemTask::OnRecvItemCosRandomMixCloseComplete(SCCosRandomMixClose* pData)
{
	m_bLockCosRandomMixSendPacket = false;

#ifdef PRE_ADD_COSRANDMIX_ACCESSORY
	CostumeMix::RandomMix::eOpenType openType = m_CosRandMixDataMgr.GetCurrentOpenType();
	if (openType == CostumeMix::RandomMix::OpenByNpc)
		GetInterface().CloseBlind();
#endif

	if (pData->nRet == ERROR_NONE)
	{
#ifdef PRE_ADD_COSRANDMIX_ACCESSORY
		bool bCloseInventory = (openType == CostumeMix::RandomMix::OpenByNpc);
		GetInterface().ShowCostumeRandomMixDlg(false, bCloseInventory);
#else
		GetInterface().ShowCostumeRandomMixDlg(false, false);
#endif
	}
	else
	{
		GetInterface().ShowCostumeRandomMixDlg(false, true);
		GetInterface().ServerMessageBox(pData->nRet);
	}
}

void CDnItemTask::OnRecvItemCosRandomMixComplete(SCCosRandomMix* pPacket)
{
	m_bLockCosRandomMixSendPacket = false;

	if (pPacket->nRet == ERROR_NONE)
	{
		CDnCostumeRandomMixDlg* pDlg = GetInterface().GetCostumeRandomMixDlg();
		if (pDlg)
			pDlg->CompleteMix(pPacket->nResultItemID);
	}
	else
	{
#ifdef PRE_ADD_COSRANDMIX_ACCESSORY
		CostumeMix::RandomMix::eOpenType openType = m_CosRandMixDataMgr.GetCurrentOpenType();
		if (openType == CostumeMix::RandomMix::OpenByNpc)
			GetInterface().CloseBlind();

		bool bCloseInventory = (openType == CostumeMix::RandomMix::OpenByNpc);
		GetInterface().ShowCostumeRandomMixDlg(false, bCloseInventory);
#else
		GetInterface().ShowCostumeRandomMixDlg(false, false);
#endif
		GetInterface().ServerMessageBox(pPacket->nRet);
	}
}
#endif // PRE_ADD_COSRANDMIX

CDnItem *CDnItemTask::FindItem( INT64 nSerialID )
{
	CDnItem *pItem;
	for( DWORD i=0; i<EQUIPMAX; i++ ) {
		pItem = m_pEquip[i];
		if( !pItem ) continue;

		if( pItem->GetSerialID() == nSerialID ) return pItem;
	}
	for( DWORD i=0; i<CASHEQUIPMAX; i++ ) {
		pItem = m_pCashEquip[i];
		if( !pItem ) continue;

		if( pItem->GetSerialID() == nSerialID ) return pItem;
	}
	for( DWORD i=0; i<GLYPHMAX; i++ ) {
		pItem = m_pGlyph[i];
		if( !pItem ) continue;

		if( pItem->GetSerialID() == nSerialID ) return pItem;
	}

	pItem = m_CharInventory.FindItemFromSerialID( nSerialID );
	if( pItem ) return pItem;

	return NULL;
}

void CDnItemTask::OnRecvCharQuickSlotList( SCQuickSlotList *pPacket )
{
	m_bRequestQuickSlot = false;

	for( int i=0; i<pPacket->cSlotCount; i++ ) 
	{
		int nSlotIndex = pPacket->QuickSlot[i].cIndex;
		INT64 nID = pPacket->QuickSlot[i].nID;

		switch( pPacket->QuickSlot[i].cType )
		{
		case QUICKSLOTTYPE_ITEM:
			{
				ITEM_SLOT_TYPE type = ( CDnItem::IsCashItem( (int)nID ) ) ? ST_INVENTORY_CASH : ST_INVENTORY;
				CDnItem *pItem = FindItem( (int)nID, type );
				if( pItem ) 
				{
					//pItem->SetSlotIndex( nSlotIndex );
					if( GetInterface().GetMainBarDialog() ) GetInterface().GetMainBarDialog()->InitQuickSlot( nSlotIndex, pItem );
				}
			}
			break;
		case QUICKSLOTTYPE_SKILL:
			{
				DnSkillHandle hSkill = CDnSkillTask::GetInstance().FindSkillBySkillPage( (int)nID , CDnSkillTask::GetInstance().GetCurrentSkillPage() , true );
				if( !hSkill && CDnActor::s_hLocalActor)
					hSkill = CDnActor::s_hLocalActor->FindSkill( (int)nID );
				if( hSkill && GetInterface().GetMainBarDialog() )
				{
					//hSkill->SetSlotIndex( nSlotIndex );
					GetInterface().GetMainBarDialog()->InitQuickSlot( nSlotIndex, hSkill.GetPointer() );
				}
			}
			break;
		case QUICKSLOTTYPE_GESTURE:
			{
				// Check if valid.
				bool bValid = true;
				CDnGestureTask::SGestureInfo *pInfo = GetGestureTask().GetGestureInfoFromID( (int)pPacket->QuickSlot[i].nID );
				if( !pInfo ) bValid = false;
				if( pInfo && !pInfo->bUsable ) bValid = false;	// RefreshGestureList has already been called when SC_ENTER arrives, so usability can be checked via bUsable.
#if defined(PRE_ADD_GUILD_GESTURE)
				if( pInfo && pInfo->_Type == CDnGestureTask::eGestureType::GestureType_GuildWarWin ) bValid = true;
#else
				if( pInfo && pInfo->nUnlockLevel == -2 ) bValid = true;
#endif // PRE_ADD_GUILD_GESTURE

				if( !bValid ) {
					RequestDelQuickSlot( pPacket->QuickSlot[i].cIndex );
					continue;
				}

				if(pPacket->QuickSlot[i].cIndex - LIFESKILL_QUICKSLOT_OFFSET >= CDnMainDlg::QUICKSLOT_BUTTON_MAX || pPacket->QuickSlot[i].cIndex - LIFESKILL_QUICKSLOT_OFFSET < 0)
					break;
				// Register quickslot.
				if( GetInterface().GetMainBarDialog() ) GetInterface().GetMainBarDialog()->InitLifeSkillQuickSlot( pPacket->QuickSlot[i].cIndex - LIFESKILL_QUICKSLOT_OFFSET, (int)pPacket->QuickSlot[i].nID, NULL, MIInventoryItem::Gesture );
			}
			break;
		case QUICKSLOTTYPE_SECONDARYSKILL:
			{
#if defined( PRE_ADD_SECONDARY_SKILL )
				bool bValid = true;
				CSecondarySkillRepository& pLifeSkillRepository = GetSkillTask().GetLifeSkillRepository();
				CSecondarySkill *pSecondarySkill = pLifeSkillRepository.Get( (int)pPacket->QuickSlot[i].nID );
				if( !pSecondarySkill ) bValid = false;

				if( !bValid ) {
					RequestDelQuickSlot( pPacket->QuickSlot[i].cIndex );
					continue;
				}

				if( (pPacket->QuickSlot[i].cIndex - LIFESKILL_QUICKSLOT_OFFSET) >= CDnMainDlg::QUICKSLOT_BUTTON_MAX || (pPacket->QuickSlot[i].cIndex - LIFESKILL_QUICKSLOT_OFFSET) < 0)
					break;

				if( GetInterface().GetMainBarDialog() ) GetInterface().GetMainBarDialog()->InitLifeSkillQuickSlot( pPacket->QuickSlot[i].cIndex - LIFESKILL_QUICKSLOT_OFFSET, (int)pPacket->QuickSlot[i].nID, NULL, MIInventoryItem::SecondarySkill );
#endif	// #if defined( PRE_ADD_SECONDARY_SKILL )
			}
			break;
		case QUICKSLOTTYPE_VEHICLE_PET:
			{
				CDnItem *pItem = m_PetInventory.FindItemFromSerialID( nID );
				if( pItem == NULL )
				{
					if( m_pVehicleEquip && m_pVehicleEquip->GetSerialID() == nID )
						pItem = m_pVehicleEquip;
					if( m_pPetEquip && m_pPetEquip->GetSerialID() == nID )
						pItem = m_pPetEquip;
				}

				if( (pPacket->QuickSlot[i].cIndex - LIFESKILL_QUICKSLOT_OFFSET) >= CDnMainDlg::QUICKSLOT_BUTTON_MAX || (pPacket->QuickSlot[i].cIndex - LIFESKILL_QUICKSLOT_OFFSET) < 0)
					break;
				if( pItem && GetInterface().GetMainBarDialog() )
					GetInterface().GetMainBarDialog()->InitLifeSkillQuickSlot( pPacket->QuickSlot[i].cIndex - LIFESKILL_QUICKSLOT_OFFSET, (int)pPacket->QuickSlot[i].nID, pItem, MIInventoryItem::Item );
			}
			break;
		}
	}

	m_bRequestQuickSlot = true;
}

void CDnItemTask::OnRecvCharAddQuickSlot(SCAddQuickSlot * pData)
{
	if (pData->nRet == ERROR_NONE)
	{
	}
	else
		GetInterface().ServerMessageBox(pData->nRet);
}

void CDnItemTask::OnRecvCharDelQuickSlot(SCDelQuickSlot * pData)
{
}

void CDnItemTask::SetCoin( INT64 nValue )
{
	m_nCoin = nValue;
}

void CDnItemTask::AddCoin( INT64 nValue )
{
	m_nCoin += nValue;
	GetInterface().AddMessageCoin( nValue );
}

void CDnItemTask::SetCoinStorage( INT64 nValue )
{
	m_nCoinStorage = nValue;
}

void CDnItemTask::SetGuildWareCoin( INT64 nValue )
{
	m_nCoinGuildWare = nValue;
}

CDnItem *CDnItemTask::CreateItem( TItem &Info )
{
	int nItemID = Info.nItemID;
	eItemTypeEnum Type = CDnItem::GetItemType( nItemID );
	CDnItem *pItem = NULL;

	if( nItemID == 0 )
	{
		OutputDebug("nItemID is 0!!!");
		return NULL;
	}

	switch( Type ) {
		case ITEMTYPE_WEAPON:
			pItem = CDnWeapon::CreateWeapon( nItemID, Info.nRandomSeed, Info.cOption, Info.cLevel, Info.cPotential, Info.cSealCount, Info.bSoulbound, false, true, Info.nLookItemID );
			break;

		case ITEMTYPE_PARTS:
			pItem = CDnParts::CreateParts( nItemID, Info.nRandomSeed, Info.cOption, Info.cLevel, Info.cPotential, Info.cSealCount, Info.bSoulbound, Info.nLookItemID );
			break;

		case ITEMTYPE_GLYPH:
			pItem = CDnGlyph::CreateGlyph( nItemID, Info.nRandomSeed, Info.cOption, Info.cLevel, Info.cPotential, Info.cSealCount, Info.bSoulbound, Info.nLookItemID );
			break;
#if defined(PRE_ADD_TALISMAN_SYSTEM)
		case ITEMTYPE_TALISMAN:
			pItem = CDnTalisman::CreateTalisman( nItemID, Info.nRandomSeed, Info.cOption, Info.cLevel, Info.cPotential, Info.cSealCount, Info.bSoulbound, Info.nLookItemID );
			break;
#endif


		case ITEMTYPE_POTENTIAL_JEWEL:
			pItem = new CDnPotentialJewel;
			if( pItem->Initialize( nItemID, Info.nRandomSeed, 0, 0, 0, 0, Info.bSoulbound, Info.nLookItemID ) == false ) {
				SAFE_DELETE( pItem );
			}
			break;

		case ITEMTYPE_VEHICLEEFFECT:
			pItem = CDnPartsVehicleEffect::CreatePartsVehicleEffect(nItemID, Info.nRandomSeed, Info.cOption, Info.cLevel, Info.cPotential, Info.cSealCount, Info.bSoulbound, Info.nLookItemID );
			break;

#ifdef PRE_ADD_SECONDARY_SKILL
		case ITEMTYPE_SECONDARYSKILL_RECIPE:
			{
				pItem = new CDnSecondarySkillRecipeItem;
				if( pItem )
				{
					if( pItem->Initialize( nItemID, Info.nRandomSeed, 0, 0, 0, 0, Info.bSoulbound, Info.nLookItemID ) )
						static_cast<CDnSecondarySkillRecipeItem*>(pItem)->SetDurability( Info.wDur );	
					else
						SAFE_DELETE( pItem );
				}
			}
			break;
#endif // PRE_ADD_SECONDARY_SKILL

		case ITEMTYPE_ENCHANT_JEWEL:
			{
				pItem = new CDnEnchantJewel;
				if( pItem->Initialize( nItemID, Info.nRandomSeed, 0, 0, 0, 0, Info.bSoulbound, Info.nLookItemID ) == false ) {
					SAFE_DELETE( pItem );
				}
			}
			break;


		default:
			pItem = CDnItem::CreateItem( nItemID, Info.nRandomSeed, Info.bSoulbound, Info.nLookItemID );
			break;
	}

	if( pItem == NULL ) {
		_ASSERT( 0 && "Failed to create item!!" );
		return NULL;
	}

	*pItem = Info;

	if (Info.bEternity == false)
	{
		pItem->SetEternityItem(false);
		pItem->SetExpireDate(Info.tExpireDate);
	}

	return pItem;
}

CDnItem *CDnItemTask::CreateItem( TItemInfo &Info )
{
	CDnItem *pItem = CDnItemTask::CreateItem( Info.Item );
	if( pItem ) pItem->SetSlotIndex( Info.cSlotIndex );
	return pItem;
}

CDnItem *CDnItemTask::CreateItem( TVehicleItem &Info )
{
	// Information passed for mounts does not include all fields that Titem has.
	TItem Temp_Item;
	memset( &Temp_Item, 0, sizeof(Temp_Item) );

	Temp_Item.bEternity = Info.bEternity;
	Temp_Item.nItemID = Info.nItemID;
	Temp_Item.nSerial = Info.nSerial;
	Temp_Item.tExpireDate = Info.tExpireDate;
	Temp_Item.wCount = Info.wCount;

	CDnItem *pItem = CDnItemTask::CreateItem( Temp_Item );
	if(pItem)
		pItem->SetSoulBound(true); // Also set the bound state.

	return pItem;
}


bool CDnItemTask::CheckRequestMoveItem( bool bDelete )
{
	if( !CDnActor::s_hLocalActor ) return false;
	CDnLocalPlayerActor *pActor = ((CDnLocalPlayerActor *)CDnActor::s_hLocalActor.GetPointer());
	if( !pActor ) return false;
	if( CheckDie( bDelete ) ) return false;
	bool bNotChangeEquip = false;
	if( pActor->IsHit() || pActor->IsAir() || pActor->IsDown() || pActor->IsAttack()
#if defined(PRE_ADD_PART_SWAP_RESTRICT_TRIGGER)
		|| GetInterface().IsPartSwapRestrict()
#endif
		) 
	{
		bNotChangeEquip = true;
	}
	if( !pActor->IsCanToggleBattleMode() ) bNotChangeEquip = true;
	//	if( !pActor->IsStay() ) // Let's use only one of the two later. 
	if( CTaskManager::IsActive() ) {
		CDnGameTask *pTask = (CDnGameTask *)CTaskManager::GetInstance().GetTask( "GameTask" );
		if( pTask && CDnPartyTask::IsActive() && !CDnPartyTask::GetInstance().IsSyncComplete() ) {
			bNotChangeEquip = true;
		}
	}

	if( bNotChangeEquip ) {
		GetInterface().AddChatMessage( CHATTYPE_SYSTEM, L"", GetEtUIXML().GetUIString( CEtUIXML::idCategory1, ( bDelete ) ? 5075 : 5072 ), false );
		return false;
	}
	if( GetInterface().GetFadeDlg() && (GetInterface().GetFadeDlg()->GetFadeMode() != CDnFadeInOutDlg::modeEnd) ) return false;
	return true;
}

bool CDnItemTask::CheckRequestEquipItem( char cMoveType )
{
	if( !CDnActor::s_hLocalActor ) return false;
	CDnLocalPlayerActor *pActor = ((CDnLocalPlayerActor *)CDnActor::s_hLocalActor.GetPointer());

	if( pActor )
	{
		switch( cMoveType )
		{
		case MoveType_CashEquipToCashInven:
		case MoveType_CashInvenToCashEquip :
		case MoveType_EquipToInven :
		case MoveType_InvenToEquip :
			{
#ifdef PRE_ADD_TRANSFORM_POTION
				if( pActor->IsSwapSingleSkin() && pActor->IsVillageTransformMode() )
				{
					GetInterface().MessageBox( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 120081 ), MB_OK ); 
					return false;
				}
#endif
			}
		}
	}

	return true;
}

bool CDnItemTask::RequestMoveItem( char cMoveType, int nSrcIndex, INT64 biSrcSerial, int nDestIndex, int nCount )
{
	if( !CheckRequestMoveItem( false ) ) return false;
	if( CheckRequestEquipItem( cMoveType ) == false ) return false;

	if( cMoveType == MoveType_CashEquipToCashInven )
	{
		CDnItem *pDestCashItem = m_CashInventory.GetItem( nDestIndex );
		INT64 biDestItemSerial = pDestCashItem ? pDestCashItem->GetSerialID() : 0;
		INT64 biEquipItemSerial = m_pCashEquip[nSrcIndex] ? m_pCashEquip[nSrcIndex]->GetSerialID() : 0;
		
#if defined(PRE_FIX_44486)
		CDnItem *pDestItem = pDestCashItem;
		CDnItem *pSrcItem = m_pCashEquip[nSrcIndex];
		if (CheckSkillLevelUpInfo(pSrcItem, pDestItem) == false)
			return false;
#endif // PRE_FIX_44486

		SendMoveCashItem( cMoveType, nSrcIndex, biEquipItemSerial, biDestItemSerial, nCount );

#if defined(PRE_FIX_45899)
		DisalbeSkillByItemMove(pDestItem, pSrcItem);
#endif // PRE_FIX_45899
	}
	else if( cMoveType == MoveType_CashInvenToCashEquip )
	{
		CDnItem *pSrcCashItem = m_CashInventory.GetItem( nSrcIndex );
		INT64 biSrcItemSerial = pSrcCashItem ? pSrcCashItem->GetSerialID() : 0;
		INT64 biEquipItemSerial = m_pCashEquip[nDestIndex] ? m_pCashEquip[nDestIndex]->GetSerialID() : 0;

#if defined(PRE_FIX_44486)
		CDnItem *pDestItem = pSrcCashItem;
		CDnItem *pSrcItem = m_pCashEquip[nDestIndex];
		if (CheckSkillLevelUpInfo(pSrcItem, pDestItem) == false)
			return false;
#endif // PRE_FIX_44486
		SendMoveCashItem( cMoveType, nDestIndex, biEquipItemSerial, biSrcItemSerial, nCount );

#if defined(PRE_FIX_45899)
		DisalbeSkillByItemMove(pDestItem, pSrcItem);
#endif // PRE_FIX_45899
	}
	else if ( cMoveType == MoveType_VehicleInvenToVehicleBody )
	{
		CDnItem *pSrcVehicleItem = m_PetInventory.GetItem( nSrcIndex );
		INT64 biSrcItemSerial = pSrcVehicleItem ? pSrcVehicleItem->GetSerialID() : 0;
		INT64 biEquipItemSerial = m_pVehicleEquip ? m_pVehicleEquip->GetSerialID() : 0;

#if defined(PRE_FIX_44486)
		CDnItem *pDestItem = pSrcVehicleItem;
		CDnItem *pSrcItem = m_pVehicleEquip;
		if (CheckSkillLevelUpInfo(pSrcItem, pDestItem) == false)
			return false;
#endif // PRE_FIX_44486
		SendMoveCashItem( cMoveType, 0 , biEquipItemSerial, biSrcItemSerial, nCount );

#if defined(PRE_FIX_45899)
		DisalbeSkillByItemMove(pDestItem, pSrcItem);
#endif // PRE_FIX_45899
	}
	else if ( cMoveType == MoveType_VehicleBodyToVehicleInven )
	{
		CDnItem *pDestVehicleItem = m_PetInventory.GetItem( nDestIndex );
		INT64 biDestItemSerial = pDestVehicleItem ? pDestVehicleItem->GetSerialID() : 0;
		INT64 biEquipItemSerial = m_pVehicleEquip ? m_pVehicleEquip->GetSerialID() : 0;

#if defined(PRE_FIX_44486)
		CDnItem *pDestItem = pDestVehicleItem;
		CDnItem *pSrcItem = m_pVehicleEquip;
		if (CheckSkillLevelUpInfo(pSrcItem, pDestItem) == false)
			return false;
#endif // PRE_FIX_44486
		SendMoveCashItem( cMoveType, nSrcIndex, biEquipItemSerial, biDestItemSerial, nCount );

#if defined(PRE_FIX_45899)
		DisalbeSkillByItemMove(pDestItem, pSrcItem);
#endif // PRE_FIX_45899
	}
	else if( cMoveType == MoveType_VehiclePartsToCashInven )
	{
		CDnItem *pDestCashItem = m_CashInventory.GetItem( nDestIndex );
		INT64 biDestItemSerial = pDestCashItem ? pDestCashItem->GetSerialID() : 0;
		INT64 biEquipItemSerial = m_pVehicleEquipParts ? m_pVehicleEquipParts->GetSerialID() : 0;

#if defined(PRE_FIX_44486)
		CDnItem *pDestItem = pDestCashItem;
		CDnItem *pSrcItem = m_pVehicleEquipParts;
		if (CheckSkillLevelUpInfo(pSrcItem, pDestItem) == false)
			return false;
#endif // PRE_FIX_44486
		SendMoveCashItem( cMoveType, nSrcIndex, biEquipItemSerial, biDestItemSerial, nCount );
		
#if defined(PRE_FIX_45899)
		DisalbeSkillByItemMove(pDestItem, pSrcItem);
#endif // PRE_FIX_45899
	}
	else if( cMoveType == MoveType_CashInvenToVehicleParts )
	{
		CDnItem *pSrcCashItem = m_CashInventory.GetItem( nSrcIndex );
		INT64 biSrcItemSerial = pSrcCashItem ? pSrcCashItem->GetSerialID() : 0;
		INT64 biEquipItemSerial = m_pCashEquip[nDestIndex] ? m_pCashEquip[nDestIndex]->GetSerialID() : 0;

#if defined(PRE_FIX_44486)
		CDnItem *pDestItem = pSrcCashItem;
		CDnItem *pSrcItem = m_pCashEquip[nDestIndex];
		if (CheckSkillLevelUpInfo(pSrcItem, pDestItem) == false)
			return false;
#endif // PRE_FIX_44486
		SendMoveCashItem( cMoveType, nDestIndex, biEquipItemSerial, biSrcItemSerial, nCount );

#if defined(PRE_FIX_45899)
		DisalbeSkillByItemMove(pDestItem, pSrcItem);
#endif // PRE_FIX_45899
	}
	else if ( cMoveType == MoveType_PetInvenToPetBody )
	{
		CDnItem *pSrcVehicleItem = m_PetInventory.GetItem( nSrcIndex );
		INT64 biSrcItemSerial = pSrcVehicleItem ? pSrcVehicleItem->GetSerialID() : 0;
		INT64 biEquipItemSerial = m_pPetEquip ? m_pPetEquip->GetSerialID() : 0;

#if defined(PRE_FIX_44486)
		CDnItem *pDestItem = pSrcVehicleItem;
		CDnItem *pSrcItem = m_pPetEquip;
		if (CheckSkillLevelUpInfo(pSrcItem, pDestItem) == false)
			return false;
#endif // PRE_FIX_44486
		SendMoveCashItem( cMoveType, 0 , biEquipItemSerial, biSrcItemSerial, nCount );

#if defined(PRE_FIX_45899)
		DisalbeSkillByItemMove(pDestItem, pSrcItem);
#endif // PRE_FIX_45899
	}
	else if ( cMoveType == MoveType_PetBodyToPetInven )
	{
		CDnItem *pDestVehicleItem = m_PetInventory.GetItem( nDestIndex );
		INT64 biDestItemSerial = pDestVehicleItem ? pDestVehicleItem->GetSerialID() : 0;
		INT64 biEquipItemSerial = m_pPetEquip ? m_pPetEquip->GetSerialID() : 0;

#if defined(PRE_FIX_44486)
		CDnItem *pDestItem = pDestVehicleItem;
		CDnItem *pSrcItem = m_pPetEquip;
		if (CheckSkillLevelUpInfo(pSrcItem, pDestItem) == false)
			return false;
#endif // PRE_FIX_44486
		SendMoveCashItem( cMoveType, nSrcIndex, biEquipItemSerial, biDestItemSerial, nCount );

#if defined(PRE_FIX_45899)
		DisalbeSkillByItemMove(pDestItem, pSrcItem);
#endif // PRE_FIX_45899
	}
	else if( cMoveType == MoveType_PetPartsToCashInven )
	{
		CDnItem *pDestCashItem = m_CashInventory.GetItem( nDestIndex );
		INT64 biDestItemSerial = pDestCashItem ? pDestCashItem->GetSerialID() : 0;
		INT64 biEquipItemSerial = 0;
		
		if( nSrcIndex == 1 ) 
			biEquipItemSerial = m_pPetEquipParts1->GetSerialID();
		else if( nSrcIndex == 2 )
			biEquipItemSerial = m_pPetEquipParts2->GetSerialID();

#if defined(PRE_FIX_44486)
		CDnItem *pDestItem = pDestCashItem;
		CDnItem *pSrcItem = (nSrcIndex == 1 ? m_pPetEquipParts1 : (nSrcIndex == 2 ? m_pPetEquipParts2 : NULL));
		if (CheckSkillLevelUpInfo(pSrcItem, pDestItem) == false)
			return false;
#endif // PRE_FIX_44486
		SendMoveCashItem( cMoveType, nSrcIndex, biEquipItemSerial, biDestItemSerial, nCount );

#if defined(PRE_FIX_45899)
		DisalbeSkillByItemMove(pDestItem, pSrcItem);
#endif // PRE_FIX_45899
	}
	else if( cMoveType == MoveType_CashInvenToPetParts )
	{
		CDnItem *pSrcCashItem = m_CashInventory.GetItem( nSrcIndex );
		INT64 biSrcItemSerial = pSrcCashItem ? pSrcCashItem->GetSerialID() : 0;
		INT64 biEquipItemSerial = m_pCashEquip[nDestIndex] ? m_pCashEquip[nDestIndex]->GetSerialID() : 0;

#if defined(PRE_FIX_44486)
		CDnItem *pDestItem = pSrcCashItem;
		CDnItem *pSrcItem = m_pCashEquip[nDestIndex];
		if (CheckSkillLevelUpInfo(pSrcItem, pDestItem) == false)
			return false;
#endif // PRE_FIX_44486
		SendMoveCashItem( cMoveType, nDestIndex, biEquipItemSerial, biSrcItemSerial, nCount );

#if defined(PRE_FIX_45899)
		DisalbeSkillByItemMove(pDestItem, pSrcItem);
#endif // PRE_FIX_45899
	}
	else if( cMoveType == MoveType_GuildWare )
	{
		CDnItem *pDestItem = m_GuildInventory.GetItem( nDestIndex );
		INT64 biDestItemSerial = pDestItem ? pDestItem->GetSerialID() : 0;
		SendMoveGuildItem( cMoveType, nSrcIndex, biSrcSerial, nDestIndex, biDestItemSerial, nCount );
	}
	else if( cMoveType == MoveType_InvenToGuildWare )
	{
		CDnItem *pDestItem = m_GuildInventory.GetItem( nDestIndex );
		INT64 biDestItemSerial = pDestItem ? pDestItem->GetSerialID() : 0;
		SendMoveGuildItem( cMoveType, nSrcIndex, biSrcSerial, nDestIndex, biDestItemSerial, nCount );
	}
	else if( cMoveType == MoveType_GuildWareToInven )
	{
		CDnItem *pDestItem = m_CharInventory.GetItem( nDestIndex );
		INT64 biDestItemSerial = pDestItem ? pDestItem->GetSerialID() : 0;
		SendMoveGuildItem( cMoveType, nSrcIndex, biSrcSerial, nDestIndex, biDestItemSerial, nCount );
	}
#ifdef PRE_ADD_SERVER_WAREHOUSE
	else if (cMoveType == MoveType_InvenToServerWare)
	{
		CDnItem *pSrcItem = m_CharInventory.GetItem(nSrcIndex);
		INT64 biSrcItemSerial = pSrcItem ? pSrcItem->GetSerialID() : 0;
		SendMoveItemServerWarehouse(cMoveType, nSrcIndex, biSrcItemSerial, 0, nCount);
	}
	else if (cMoveType == MoveType_ServerWareToInven)
	{
		CDnItem *pDestItem = m_CharInventory.GetItem(nDestIndex);
		INT64 biDestItemSerial = pDestItem ? pDestItem->GetSerialID() : 0;
		SendMoveItemServerWarehouse(cMoveType, nDestIndex, biDestItemSerial, biSrcSerial, nCount);
	}
	else if (cMoveType == MoveType_CashToServerWare ||
			 cMoveType == MoveType_ServerWareToCash)
	{
		SendMoveCashItemServerWarehouse(cMoveType, nSrcIndex, biSrcSerial);
	}
#endif
	else
	{
#if defined(PRE_FIX_44486)
		if (CheckSkillLevelUpInfo(cMoveType, nSrcIndex, nDestIndex) == false)
			return false;
#endif // PRE_FIX_44486
		SendMoveItem( cMoveType, nSrcIndex, biSrcSerial, nDestIndex, nCount );

#if defined(PRE_FIX_45899)
		CDnItem *pDestItem = NULL;
		CDnItem *pSrcItem = NULL;
		GetItemByMoveType(cMoveType, nSrcIndex, nDestIndex, &pSrcItem, &pDestItem);
		DisalbeSkillByItemMove(pDestItem, pSrcItem);
#endif // PRE_FIX_45899
	}

	//There are cases where the packet is not sent due to CheckSkillLevelUpInfo.
	//The variables below are modified after calling SendMoveItem...

	// Considered creating a new RequestMoveCashItem function, but since the structure is already far along, kept changes minimal
	// Just handle cash items as an exception like this.
	m_nRequestSrcItemSlot = nSrcIndex;
	m_nRequestDestItemSlot = nDestIndex;
	m_bRequestWait = true;

	return true;
}

void CDnItemTask::RequestRemoveItem( char cMoveType, int nSrcIndex, int nCount, INT64 biSerial )
{
	if( !CheckRequestMoveItem( true ) ) return;

	m_bRequestWait = true;
	m_nRequestSrcItemSlot = nSrcIndex;

	SendRemoveItem( cMoveType, nSrcIndex, nCount, biSerial );
}


bool CDnItemTask::InsertEquipItem( CDnItem *pItem )
{
	if( !pItem || m_pEquip[pItem->GetSlotIndex()] ) 
		return false;

	m_pEquip[pItem->GetSlotIndex()] = pItem;

	CDnCharStatusDlg *pCharStatusDlg = (CDnCharStatusDlg*)GetInterface().GetMainMenuDialog( CDnMainMenuDlg::CHARSTATUS_DIALOG );
	if( pCharStatusDlg ) pCharStatusDlg->SetEquipItem( pItem->GetSlotIndex(), pItem );

#ifdef PRE_ADD_EQUIPED_ITEM_ENCHANT
	GetInterface().GetMainMenuDialog()->GetItemUpgradeDlg()->SetEquippedItem( pItem->GetSlotIndex(), pItem );	
#endif 

	return true;		
}

bool CDnItemTask::RemoveEquipItem( int nEquipIndex )
{
	if( !m_pEquip[nEquipIndex] ) 
		return false;

	SAFE_DELETE( m_pEquip[nEquipIndex] );

	CDnCharStatusDlg *pCharStatusDlg = (CDnCharStatusDlg*)GetInterface().GetMainMenuDialog( CDnMainMenuDlg::CHARSTATUS_DIALOG );
	if( pCharStatusDlg ) pCharStatusDlg->ResetEquipSlot( nEquipIndex );

	return true;
}

void CDnItemTask::RemoveAllEquipItem()
{
	for( int i=0; i<EQUIPMAX; i++ ) 
	{
		SAFE_DELETE( m_pEquip[i] );
	}
}

bool CDnItemTask::InsertCashEquipItem( CDnItem *pItem )
{
	if( !pItem || m_pCashEquip[pItem->GetSlotIndex()] ) 
		return false;

	m_pCashEquip[pItem->GetSlotIndex()] = pItem;

	if(pItem->GetItemType() == ITEMTYPE_VEHICLEEFFECT)
	{
		InsertVehicleItem(pItem);
		return true;
	}

	CDnCharStatusDlg *pCharStatusDlg = (CDnCharStatusDlg*)GetInterface().GetMainMenuDialog( CDnMainMenuDlg::CHARSTATUS_DIALOG );
	if( pCharStatusDlg ) pCharStatusDlg->SetCashEquipItem( pItem->GetSlotIndex(), pItem );

	return true;
}

bool CDnItemTask::RemoveCashEquipItem( int nEquipIndex )
{
	if( !m_pCashEquip[nEquipIndex] ) 
		return false;
	
	SAFE_DELETE( m_pCashEquip[nEquipIndex] );

	if(nEquipIndex == CASHEQUIP_EFFECT)
	{
		RemoveVehicleItem(CDnCharVehicleDlg::VehicleEffectSlot);
		return true;
	}

	CDnCharStatusDlg *pCharStatusDlg = (CDnCharStatusDlg*)GetInterface().GetMainMenuDialog( CDnMainMenuDlg::CHARSTATUS_DIALOG );
	if( pCharStatusDlg ) pCharStatusDlg->ResetCashEquipSlot( nEquipIndex );

	return true;
}

void CDnItemTask::RemoveAllCashEquipItem()
{
	for( int i=0; i<CASHEQUIPMAX; i++ ) 
	{
		SAFE_DELETE( m_pCashEquip[i] );
	}
}

void CDnItemTask::RefreshCashEquip()
{
	CDnCharStatusDlg *pCharStatusDlg = (CDnCharStatusDlg*)GetInterface().GetMainMenuDialog( CDnMainMenuDlg::CHARSTATUS_DIALOG );
	if( !pCharStatusDlg ) return;

	for( DWORD i=0; i<CASHEQUIPMAX; i++ )
	{
		
		if(i==CASHEQUIP_EFFECT)
		{
			pCharStatusDlg->ResetVehicleEquipSlot(CDnCharVehicleDlg::VehicleEffectSlot);
			pCharStatusDlg->SetVehicleEquipItem(CDnCharVehicleDlg::VehicleEffectSlot,m_pCashEquip[i]);
			continue;
		}

		pCharStatusDlg->ResetCashEquipSlot( i );
		pCharStatusDlg->SetCashEquipItem( i, m_pCashEquip[i] );
	}
}

bool CDnItemTask::InsertGlyphItem( CDnItem *pItem )
{
	if( !pItem || m_pGlyph[pItem->GetSlotIndex()] ) 
		return false;

	m_pGlyph[pItem->GetSlotIndex()] = pItem;

	CDnCharStatusDlg *pCharStatusDlg = (CDnCharStatusDlg*)GetInterface().GetMainMenuDialog( CDnMainMenuDlg::CHARSTATUS_DIALOG );
	if( pCharStatusDlg ) pCharStatusDlg->SetGlyphEquipItem( pItem->GetSlotIndex(), pItem );

	return true;
}

bool CDnItemTask::RemoveGlyphItem( int nEquipIndex )
{
	if( !m_pGlyph[nEquipIndex] ) 
		return false;

	SAFE_DELETE( m_pGlyph[nEquipIndex] );

	CDnCharStatusDlg *pCharStatusDlg = (CDnCharStatusDlg*)GetInterface().GetMainMenuDialog( CDnMainMenuDlg::CHARSTATUS_DIALOG );
	if( pCharStatusDlg ) pCharStatusDlg->ResetGlyphEquipSlot( nEquipIndex );

	return true;
}

void CDnItemTask::RemoveAllGlyphItem()
{
	for( int i=0; i<GLYPHMAX; i++ ) 
	{
		SAFE_DELETE( m_pGlyph[i] );
	}
}

void CDnItemTask::RefreshGlyph()
{
	CDnCharStatusDlg *pCharStatusDlg = (CDnCharStatusDlg*)GetInterface().GetMainMenuDialog( CDnMainMenuDlg::CHARSTATUS_DIALOG );
	if( !pCharStatusDlg ) return;

	for( DWORD i=0; i<GLYPHMAX; i++ )
	{
		pCharStatusDlg->ResetGlyphEquipSlot( i );
		pCharStatusDlg->SetGlyphEquipItem( i, m_pGlyph[i] );
	}
}

#if defined(PRE_ADD_TALISMAN_SYSTEM)
bool CDnItemTask::InsertTalismanItem( CDnItem *pItem )
{
	if( !pItem ) return false;
	int nSlotIndex = pItem->GetSlotIndex();
	if( nSlotIndex < 0 || nSlotIndex >= TALISMAN_MAX ) return false;
	if( m_pTalisman[nSlotIndex] ) return false;

	m_pTalisman[nSlotIndex] = pItem;

	CDnCharStatusDlg *pCharStatusDlg = (CDnCharStatusDlg*)GetInterface().GetMainMenuDialog( CDnMainMenuDlg::CHARSTATUS_DIALOG );
	if( pCharStatusDlg ) pCharStatusDlg->SetTalismanEquipItem( nSlotIndex, pItem );

	return true;
}

bool CDnItemTask::RemoveTalismanItem( int nSlotIndex )
{
	if( nSlotIndex < 0 || nSlotIndex >= TALISMAN_MAX ) return false;
	if( !m_pTalisman[nSlotIndex] ) return false;

	SAFE_DELETE( m_pTalisman[nSlotIndex] );

	CDnCharStatusDlg *pCharStatusDlg = (CDnCharStatusDlg*)GetInterface().GetMainMenuDialog( CDnMainMenuDlg::CHARSTATUS_DIALOG );
	if( pCharStatusDlg ) pCharStatusDlg->ResetTalismanEquipSlot( nSlotIndex );

	return true;
}

void CDnItemTask::RemoveAllTalismanItem()
{
	for( int i=0; i<TALISMAN_MAX; i++ )
	{
		SAFE_DELETE( m_pTalisman[i] );
	}
}

CDnItem *CDnItemTask::GetTalismanItem( int nSlotIndex )
{
	if( nSlotIndex < 0 || nSlotIndex >= TALISMAN_MAX ) return NULL;
	return m_pTalisman[nSlotIndex];
}

void CDnItemTask::RefreshTalisman()
{
	CDnCharStatusDlg *pCharStatusDlg = (CDnCharStatusDlg*)GetInterface().GetMainMenuDialog( CDnMainMenuDlg::CHARSTATUS_DIALOG );
	if( !pCharStatusDlg ) return;

	for( int i=0; i<TALISMAN_MAX; i++ )
	{
		pCharStatusDlg->ResetTalismanEquipSlot( i );
		pCharStatusDlg->SetTalismanEquipItem( i, m_pTalisman[i] );
	}
}

bool CDnItemTask::IsOpenTalismanSlot( int nSlotIndex )
{
	if( nSlotIndex < 0 || nSlotIndex >= TALISMAN_MAX ) return false;

	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TTALISMANSLOT );
	if( !pSox || nSlotIndex >= pSox->GetItemCount() ) return false;

	int nID = pSox->GetItemID( nSlotIndex );
	if( pSox->GetFieldFromLablePtr( nID, "_Service" )->GetInteger() == 0 )
		return false;

	int nType = pSox->GetFieldFromLablePtr( nID, "_Type" )->GetInteger();
	if( nType == TALISMAN_BASIC )
		return true;
	if( nType == TALISMAN_EXPANSION )
		return ( m_nTalismanOpenFlag & ( 1 << ( nSlotIndex - TALISMAN_EXPANSION_START ) ) ) ? true : false;
	if( nType == TALISMAN_CASH_EXPANSION )
		return m_bCashTalismanOpen;

	return false;
}

bool CDnItemTask::IsExistCashTalisman() const
{
	for( int i = TALISMAN_CASH1; i <= TALISMAN_CASH_MAX; ++i )
	{
		if( m_pTalisman[i] )
			return true;
	}
	return false;
}

bool CDnItemTask::IsExistTalismanType( int nTalismanType, int &nSlotIndex ) const
{
	for( int i = 0; i < TALISMAN_MAX; ++i )
	{
		CDnTalisman *pTalisman = dynamic_cast<CDnTalisman*>( m_pTalisman[i] );
		if( pTalisman && pTalisman->GetTalismanType() == nTalismanType )
		{
			nSlotIndex = i;
			return true;
		}
	}
	return false;
}

bool CDnItemTask::IsEquipTalismanItem( int nItemID, int &nEquipIndex ) const
{
	for( int i = 0; i < TALISMAN_MAX; ++i )
	{
		if( m_pTalisman[i] && m_pTalisman[i]->GetClassID() == nItemID )
		{
			nEquipIndex = i;
			return true;
		}
	}
	return false;
}

bool CDnItemTask::IsEquipTalismanType( const CDnItem *pItem ) const
{
	if( !pItem ) return false;
	int nTypeParam = pItem->GetTypeParam( 1 );
	for( int i = 0; i < TALISMAN_MAX; ++i )
	{
		if( !m_pTalisman[i] ) continue;
		if( nTypeParam > 0 )
		{
			if( m_pTalisman[i]->GetTypeParam( 1 ) == nTypeParam )
				return true;
		}
		else if( m_pTalisman[i]->GetClassID() == pItem->GetClassID() )
		{
			return true;
		}
	}
	return false;
}

float CDnItemTask::GetTalismanEfficiency( int nSlotIndex ) const
{
	if( nSlotIndex < 0 || nSlotIndex >= TALISMAN_MAX ) return 1.0f;

	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TTALISMANSLOT );
	if( !pSox || nSlotIndex >= pSox->GetItemCount() ) return 1.0f;

	int nID = pSox->GetItemID( nSlotIndex );
	return pSox->GetFieldFromLablePtr( nID, "_Efficiency" )->GetFloat();
}

int CDnItemTask::GetTalismanMoveCost() const
{
	if( !CDnActor::s_hLocalActor ) return 0;

	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TPLAYERCOMMONLEVEL );
	if( !pSox ) return 0;

	int nLevel = CDnActor::s_hLocalActor->GetLevel();
	if( !pSox->IsExistItem( nLevel ) ) return 0;

	return pSox->GetFieldFromLablePtr( nLevel, "_TalismanCost" )->GetInteger();
}
#endif	// #if defined(PRE_ADD_TALISMAN_SYSTEM)

bool CDnItemTask::InsertVehicleItem( CDnItem *pItem )
{
	if( !pItem )//|| m_pVehicleEquip ) 
		return false;

	if(pItem->GetSlotIndex() == Vehicle::Slot::Body)
		m_pVehicleEquip = pItem;

	if(pItem->GetSlotIndex() == Vehicle::Slot::Saddle)
		m_pVehicleEquipParts = pItem;

	CDnCharStatusDlg *pCharStatusDlg = (CDnCharStatusDlg*)GetInterface().GetMainMenuDialog( CDnMainMenuDlg::CHARSTATUS_DIALOG );

	// The mount effect slot has a completely different structure from the mount slot; it is a player cash equip type.
	if(pItem->GetItemType() == ITEMTYPE_VEHICLEEFFECT)
	{
		if( pCharStatusDlg ) pCharStatusDlg->SetVehicleEquipItem( CDnCharVehicleDlg::VehicleEffectSlot, pItem );
		return true;
	}

	if( pCharStatusDlg ) pCharStatusDlg->SetVehicleEquipItem( pItem->GetSlotIndex(), pItem );

	return true;
}

bool CDnItemTask::RemoveVehicleItem( int nEquipIndex )
{
	if( !m_pVehicleEquip && !m_pVehicleEquipParts) 
		return false;

	if(m_pVehicleEquip && nEquipIndex == Vehicle::Slot::Body)
		SAFE_DELETE( m_pVehicleEquip );

	if(m_pVehicleEquipParts && nEquipIndex == Vehicle::Slot::Saddle)
		SAFE_DELETE( m_pVehicleEquipParts );

	CDnCharStatusDlg *pCharStatusDlg = (CDnCharStatusDlg*)GetInterface().GetMainMenuDialog( CDnMainMenuDlg::CHARSTATUS_DIALOG );
	if( pCharStatusDlg ) pCharStatusDlg->ResetVehicleEquipSlot( nEquipIndex );

	return true;
}

void CDnItemTask::RemoveAllVehicleItem()
{
	SAFE_DELETE( m_pVehicleEquip );
	SAFE_DELETE( m_pVehicleEquipParts );
}

bool CDnItemTask::InsertPetItem( CDnItem *pItem )
{
	if( !pItem )
		return false;

	if( pItem->GetSlotIndex() == Pet::Slot::Body )
		m_pPetEquip = pItem;

	if( pItem->GetSlotIndex() == Pet::Slot::Accessory1 )
		m_pPetEquipParts1 = pItem;

	if( pItem->GetSlotIndex() == Pet::Slot::Accessory2 )
		m_pPetEquipParts2 = pItem;

	CDnCharStatusDlg* pCharStatusDlg = (CDnCharStatusDlg*)GetInterface().GetMainMenuDialog( CDnMainMenuDlg::CHARSTATUS_DIALOG );
	if( pCharStatusDlg ) 
		pCharStatusDlg->SetPetEquipItem( pItem );

	return true;
}

bool CDnItemTask::RemovePetItem( int nEquipIndex )
{
	if( !m_pPetEquip && !m_pPetEquipParts1 && !m_pPetEquipParts2 ) 
		return false;

	if( m_pPetEquip && nEquipIndex == Pet::Slot::Body )
		SAFE_DELETE( m_pPetEquip );

	if( m_pPetEquipParts1 && nEquipIndex == Pet::Slot::Accessory1 )
		SAFE_DELETE( m_pPetEquipParts1 );

	if( m_pPetEquipParts2 && nEquipIndex == Pet::Slot::Accessory2 )
		SAFE_DELETE( m_pPetEquipParts2 );

	CDnCharStatusDlg *pCharStatusDlg = (CDnCharStatusDlg*)GetInterface().GetMainMenuDialog( CDnMainMenuDlg::CHARSTATUS_DIALOG );
	if( pCharStatusDlg ) 
		pCharStatusDlg->ResetPetEquipSlot( nEquipIndex );

	return true;
}

void CDnItemTask::RemoveAllPetItem()
{
	SAFE_DELETE( m_pPetEquip );
	SAFE_DELETE( m_pPetEquipParts1 );
	SAFE_DELETE( m_pPetEquipParts2 );
}

bool CDnItemTask::IsEquipItem( const int nSlotIndex, const CDnItem *pItem, std::wstring &strErrorMsg )
{
	if( !CDnActor::s_hLocalActor ) return false;
	strErrorMsg.clear();

	std::vector<int> nVecJobList;
	((CDnLocalPlayerActor *)CDnActor::s_hLocalActor.GetPointer())->GetJobHistory( nVecJobList );

	if( !pItem->IsPermitPlayer( nVecJobList ) )
	{
		strErrorMsg = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 5069 );
		return false;
	}

	switch( pItem->GetItemType() ) {
		case ITEMTYPE_WEAPON:
			{
				CDnWeapon *pWeapon = (CDnWeapon *)pItem;
				if( pItem->IsCashItem() ) {
					if( nSlotIndex < CASHEQUIP_WEAPON1 ) {
						strErrorMsg = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 5056 );
						return false;
					}
					switch( nSlotIndex ) {
						case CASHEQUIP_WEAPON1:
							if( pWeapon->GetEquipType() == CDnWeapon::Shield ||
								pWeapon->GetEquipType() == CDnWeapon::Arrow ||
								pWeapon->GetEquipType() == CDnWeapon::Book ||
								pWeapon->GetEquipType() == CDnWeapon::Orb || 
								pWeapon->GetEquipType() == CDnWeapon::Puppet ||
								pWeapon->GetEquipType() == CDnWeapon::Gauntlet ||
								pWeapon->GetEquipType() == CDnWeapon::Glove || 
								pWeapon->GetEquipType() == CDnWeapon::Charm ||
								pWeapon->GetEquipType() == CDnWeapon::Crook ||
								pWeapon->GetEquipType() == CDnWeapon::Bracelet ||
								pWeapon->GetEquipType() == CDnWeapon::Claw ) 
							{
								strErrorMsg = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 5056 );
								return false;
							}
							break;
						case CASHEQUIP_WEAPON2:
							if( pWeapon->GetEquipType() != CDnWeapon::Shield &&
								pWeapon->GetEquipType() != CDnWeapon::Arrow &&
								pWeapon->GetEquipType() != CDnWeapon::Book &&
								pWeapon->GetEquipType() != CDnWeapon::Orb &&
								pWeapon->GetEquipType() != CDnWeapon::Puppet &&
								pWeapon->GetEquipType() != CDnWeapon::Gauntlet &&
								pWeapon->GetEquipType() != CDnWeapon::Glove && 
								pWeapon->GetEquipType() != CDnWeapon::Charm &&
								pWeapon->GetEquipType() != CDnWeapon::Crook &&
								pWeapon->GetEquipType() != CDnWeapon::Bracelet &&
								pWeapon->GetEquipType() != CDnWeapon::Claw ) 
							{
								strErrorMsg = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 5056 );
								return false;
							}
							break;
						default:
							strErrorMsg = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 5057 );
							return false;
					}
					if( pWeapon->IsOneType() ) {
						bool bEnable = true;
						CDnItem *pNormalWeaponItem = NULL;
						if( nSlotIndex == CASHEQUIP_WEAPON1 ) pNormalWeaponItem = m_pEquip[EQUIP_WEAPON1];
						else if( nSlotIndex == CASHEQUIP_WEAPON2 ) pNormalWeaponItem = m_pEquip[EQUIP_WEAPON2];
						if( !pNormalWeaponItem ) bEnable = false;
						if( pNormalWeaponItem ) {
							CDnWeapon *pNormalWeapon = (CDnWeapon *)pNormalWeaponItem;
							if( pNormalWeapon->GetEquipType() != pWeapon->GetEquipType() ) bEnable = false;
						}
						if( !bEnable ) {
							strErrorMsg = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 130254 );
							return false;
						}
					}
				}
				else {
					if( nSlotIndex < EQUIP_WEAPON1 ) {
						strErrorMsg = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 5056 );
						return false;
					}
					switch( nSlotIndex ) {
						case EQUIP_WEAPON1:
							if( pWeapon->GetEquipType() == CDnWeapon::Shield ||
								pWeapon->GetEquipType() == CDnWeapon::Arrow ||
								pWeapon->GetEquipType() == CDnWeapon::Book ||
								pWeapon->GetEquipType() == CDnWeapon::Orb || 
								pWeapon->GetEquipType() == CDnWeapon::Puppet ||
								pWeapon->GetEquipType() == CDnWeapon::Gauntlet ||
								pWeapon->GetEquipType() == CDnWeapon::Glove ||
								pWeapon->GetEquipType() == CDnWeapon::Charm ||
								pWeapon->GetEquipType() == CDnWeapon::Crook ||
								pWeapon->GetEquipType() == CDnWeapon::Bracelet ||
								pWeapon->GetEquipType() == CDnWeapon::Claw ) 
							{
								strErrorMsg = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 5056 );
								return false;
							}
							break;
						case EQUIP_WEAPON2:
							if( pWeapon->GetEquipType() != CDnWeapon::Shield &&
								pWeapon->GetEquipType() != CDnWeapon::Arrow &&
								pWeapon->GetEquipType() != CDnWeapon::Book &&
								pWeapon->GetEquipType() != CDnWeapon::Orb &&
								pWeapon->GetEquipType() != CDnWeapon::Puppet &&
								pWeapon->GetEquipType() != CDnWeapon::Gauntlet &&
								pWeapon->GetEquipType() != CDnWeapon::Glove && 
								pWeapon->GetEquipType() != CDnWeapon::Charm &&
								pWeapon->GetEquipType() != CDnWeapon::Crook &&
								pWeapon->GetEquipType() != CDnWeapon::Bracelet &&
								pWeapon->GetEquipType() != CDnWeapon::Claw ) 
							{
								strErrorMsg = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 5056 );
								return false;
							}
							break;
						default:
							strErrorMsg = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 5057 );
							return false;
					}
				}
			}
			break;
		case ITEMTYPE_PARTS:
			{
				CDnParts *pParts = (CDnParts *)pItem;
				if( pItem->IsCashItem() ) {
					if( pParts->GetPartsType() == CDnParts::CashRing || pParts->GetPartsType() == CDnParts::CashRing2 ) {
						if( nSlotIndex != CDnParts::CashRing && nSlotIndex != CDnParts::CashRing2 ) 
						{
							strErrorMsg = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 5056 );
							return false;
						}
						break;
					}
				}
				else {
					if( pParts->GetPartsType() == CDnParts::Ring || pParts->GetPartsType() == CDnParts::Ring2 ) {
						if( nSlotIndex != CDnParts::Ring && nSlotIndex != CDnParts::Ring2 ) 
						{
							strErrorMsg = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 5056 );
							return false;
						}
						break;
					}
				}
				if( pParts->GetPartsType() != nSlotIndex ) 
				{
					strErrorMsg = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 5056 );
					return false;
				}
			}
			break;

		case ITEMTYPE_GLYPH:
			{
				CDnGlyph *pGlyph = (CDnGlyph*)pItem;

				// Extended slots do not require type checking.
				if( nSlotIndex >= GLYPH_CASH1 && nSlotIndex <= GLYPH_CASH3 
#if defined(PRE_ADD_DRAGON_FELLOWSHIP_GLYPH)
					&& CDnGlyph::DragonFellowship != pGlyph->GetGlyphType()
#endif	// #if defined(PRE_ADD_DRAGON_FELLOWSHIP_GLYPH)
					)
				{
					break;
				}

				switch( pGlyph->GetGlyphType() ) {
					case CDnGlyph::Enchant:
						if( !( nSlotIndex >= GLYPH_ENCHANT1 && nSlotIndex <= GLYPH_ENCHANT8 ) ) {
							strErrorMsg = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 9015 );
							return false;
						}
						break;
					case CDnGlyph::PassiveSkill:
						if( !( nSlotIndex >= GLYPH_SKILL1 && nSlotIndex <= GLYPH_SKILL4 ) ) {
							strErrorMsg = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 9015 );
							return false;
						}
						break;
					case CDnGlyph::ActiveSkill:
						if( nSlotIndex != GLYPH_SPECIALSKILL ) {
							strErrorMsg = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 9015 );
							return false;
						}
						break;
#if defined(PRE_ADD_DRAGON_FELLOWSHIP_GLYPH)
					case CDnGlyph::DragonFellowship:
						if( !( nSlotIndex >= GLYPH_DRAGON1 && nSlotIndex <= GLYPH_DRAGON4 ) ) {
							strErrorMsg = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 9015 );
							return false;
						}
						break;
#endif	// #if defined(PRE_ADD_DRAGON_FELLOWSHIP_GLYPH)
				}
			}
			break;

#if defined(PRE_ADD_TALISMAN_SYSTEM)
		case ITEMTYPE_TALISMAN:
			{
				if( !IsOpenTalismanSlot( nSlotIndex ) ) {
					strErrorMsg = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 8361 );
					return false;
				}
			}
			break;
#endif	// #if defined(PRE_ADD_TALISMAN_SYSTEM)


		default:
			strErrorMsg = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 5058 );
			return false;
	}

	if( CDnActor::s_hLocalActor->GetLevel() < pItem->GetLevelLimit() )
	{
		strErrorMsg = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 5070 );
		return false;
	}

	if( pItem->GetNeedPvpRank() )
	{
		TPvPGroup *pPvPInfo = CDnPartyTask::GetInstance().GetPvPInfo();
		if( pPvPInfo->cLevel < pItem->GetNeedPvpRank() )
		{
			strErrorMsg = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 5071 );
			return false;
		}
	}

	return true;
}

bool CDnItemTask::CheckDie( bool bDeleteItem )
{
	if( !CDnActor::s_hLocalActor ) return false;
	CDnLocalPlayerActor *pActor = ((CDnLocalPlayerActor *)CDnActor::s_hLocalActor.GetPointer());
	if( !pActor ) return false;
	if( pActor->IsDie() || pActor->IsGhost() )
	{
		if( bDeleteItem ) 
			GetInterface().AddChatMessage( CHATTYPE_SYSTEM, L"", GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 938 ), false );
		else 
			GetInterface().MessageBox( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 910 ), MB_OK );
		return true;
	}

	if(pActor->IsLockItemMove())
	{
		if( bDeleteItem ) 
			GetInterface().AddChatMessage( CHATTYPE_SYSTEM, L"", GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 120081 ), false ); 
		else 
			GetInterface().MessageBox( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 120081 ), MB_OK ); // Probably need to ask to change the UIString.
		return true;
	}

	return false;
}

void CDnItemTask::RefreshEquip()
{
	CDnCharStatusDlg *pCharStatusDlg = (CDnCharStatusDlg*)GetInterface().GetMainMenuDialog( CDnMainMenuDlg::CHARSTATUS_DIALOG );
	if( !pCharStatusDlg ) return;


	for( DWORD i=0; i<EQUIPMAX; i++ )
	{
		pCharStatusDlg->ResetEquipSlot( i );
		pCharStatusDlg->SetEquipItem( i, m_pEquip[i] );
	}

	for( DWORD i=0; i<CASHEQUIPMAX; i++ )
	{

		// Refresh equip
		if(i==CASHEQUIP_EFFECT)
		{
			pCharStatusDlg->ResetVehicleEquipSlot(CDnCharVehicleDlg::VehicleEffectSlot);
			pCharStatusDlg->SetVehicleEquipItem(CDnCharVehicleDlg::VehicleEffectSlot,m_pCashEquip[i]);
			continue;
		}

		pCharStatusDlg->ResetCashEquipSlot( i );
		pCharStatusDlg->SetCashEquipItem( i, m_pCashEquip[i] );

	}
}



bool CDnItemTask::RequestEmblemCompoundOpen( CDnItem* pPlateItem, int iCompoundTableID, vector<ItemCompoundInfo>& vlEmblemCompoundInfo )
{
	_ASSERT( pPlateItem );

	bool bResult = false;

	// Divided into Item Craft / Heraldry Gem depending on whether a plate item object exists
	CDnItemCompounder::S_ITEM_SETTING_INFO ItemSettingInfo;
	CDnItemCompounder::S_OUTPUT Output;

	// Note: Use INT64 later if needed.
	ItemSettingInfo.iHasMoney = GetCoin();
	ItemSettingInfo.iCompoundTableID = iCompoundTableID;

	int iNumItemInfos = (int)vlEmblemCompoundInfo.size();
	for( int iItem = 0; iItem < iNumItemInfos; ++iItem )
	{
		ItemCompoundInfo& Info = vlEmblemCompoundInfo.at( iItem );
		ItemSettingInfo.aiItemID[ iItem ] = Info.iItemID;
		ItemSettingInfo.aiItemCount[ iItem ] = Info.iUseCount;
	}

	m_pItemCompounder->EmblemCompound( pPlateItem->GetClassID(), ItemSettingInfo, &Output );

	// First check on the client side if it is valid
	if( CDnItemCompounder::R_SUCCESS == Output.eResultCode )
	{
		CSEmblemCompoundOpenReq CompoundItemReq;
		ZeroMemory( &CompoundItemReq, sizeof(CSEmblemCompoundOpenReq) );
		CompoundItemReq.nCompoundTableID = iCompoundTableID;

		CompoundItemReq.cPlateItemSlotIndex = (char)pPlateItem->GetSlotIndex();
		CompoundItemReq.biPlateItemSerialID = pPlateItem->GetSerialID();

		SendEmblemCompoundOpenReq( CompoundItemReq );

		EmblemCompoundResult Result;
		Result.vlItemlInfosToRemove = vlEmblemCompoundInfo;
		Result.Output = Output;
		m_dqWaitEmblemCompoundReq.push_back( Result );

		bResult = true;
		m_bRequestWait = true;
	}
	else
	{
		// TODO: Failed to create crest on client side, display UI message.
	}

	return bResult;
}


bool CDnItemTask::RequestItemCompoundOpen( int iCompoundTableID, vector<ItemCompoundInfo>& vlItemCompoundInfo, INT64 biNeedItemSerialID/* = 0*/, INT64 iDiscountedCost/* = 0.0f*/ )
{
	bool bResult = false;

	CDnItemCompounder::S_ITEM_SETTING_INFO ItemSettingInfo;
	CDnItemCompounder::S_OUTPUT Output;

	// Note: Use INT64 later if needed.
	ItemSettingInfo.iHasMoney = GetCoin();
	ItemSettingInfo.iCompoundTableID = iCompoundTableID;
	ItemSettingInfo.iDiscountedCost = iDiscountedCost;

	int iNumItemInfos = (int)vlItemCompoundInfo.size();
	for( int iItem = 0; iItem < iNumItemInfos; ++iItem )
	{
		ItemCompoundInfo& Info = vlItemCompoundInfo.at( iItem );
		ItemSettingInfo.aiItemID[ iItem ] = Info.iItemID;
		ItemSettingInfo.aiItemCount[ iItem ] = Info.iUseCount;
	}

	m_pItemCompounder->Compound( ItemSettingInfo, &Output );

	// First check on the client side if it is valid
	if( CDnItemCompounder::R_SUCCESS == Output.eResultCode )
	{
		CSItemCompoundOpenReq CompoundItemReq;
		ZeroMemory( &CompoundItemReq, sizeof(CSItemCompoundOpenReq) );
		CompoundItemReq.nCompoundTableID = iCompoundTableID;
		CompoundItemReq.biNeedItemSerialID = biNeedItemSerialID;
		
		SendItemCompoundOpenReq( CompoundItemReq );

		EmblemCompoundResult Result;
		Result.vlItemlInfosToRemove = vlItemCompoundInfo;
		Result.Output = Output;
		m_dqWaitEmblemCompoundReq.push_back( Result );

		bResult = true;
		m_bRequestWait = true;
	}
	else
	{
		// TODO: Failed to create crest on client side, display UI message.
	}

	return bResult;
}


// Request Crest Jewel synthesis
bool CDnItemTask::RequestEmblemCompound( CDnItem* pPlateItem, int iCompoundTableID, vector<ItemCompoundInfo>& vlEmblemCompoundInfo )
{
	_ASSERT( pPlateItem );

	bool bResult = false;

	// Divided into Item Craft / Heraldry Gem depending on whether a plate item object exists
	CDnItemCompounder::S_ITEM_SETTING_INFO ItemSettingInfo;
	CDnItemCompounder::S_OUTPUT Output;

	// Note: Use INT64 later if needed.
	ItemSettingInfo.iHasMoney = GetCoin();
	ItemSettingInfo.iCompoundTableID = iCompoundTableID;

	int iNumItemInfos = (int)vlEmblemCompoundInfo.size();
	for( int iItem = 0; iItem < iNumItemInfos; ++iItem )
	{
		ItemCompoundInfo& Info = vlEmblemCompoundInfo.at( iItem );
		ItemSettingInfo.aiItemID[ iItem ] = Info.iItemID;
		ItemSettingInfo.aiItemCount[ iItem ] = Info.iUseCount;
	}

	m_pItemCompounder->EmblemCompound( pPlateItem->GetClassID(), ItemSettingInfo, &Output );

	// First check on the client side if it is valid
	if( CDnItemCompounder::R_SUCCESS == Output.eResultCode )
	{
		CSCompoundEmblemReq CompoundItemReq;
		ZeroMemory( &CompoundItemReq, sizeof(CSCompoundEmblemReq) );
		CompoundItemReq.nCompoundTableID = iCompoundTableID;

		CompoundItemReq.cPlateItemSlotIndex = (char)pPlateItem->GetSlotIndex();
		CompoundItemReq.biPlateItemSerialID = pPlateItem->GetSerialID();

		SendEmblemCompoundReq( CompoundItemReq );

		EmblemCompoundResult Result;
		Result.vlItemlInfosToRemove = vlEmblemCompoundInfo;
		Result.Output = Output;
		m_dqWaitEmblemCompoundReq.push_back( Result );

		bResult = true;
		m_bRequestWait = true;
	}
	else
	{
		// TODO: Failed to create crest on client side, display UI message.
	}

	return bResult;
}


// Item synthesis request
bool CDnItemTask::RequestItemCompound( int iCompoundTableID, vector<ItemCompoundInfo>& vlItemCompoundInfo, INT64 biNeedItemSerialID/* = 0*/, INT64 iDiscountedCost/* = 0.0f*/ )
{
	bool bResult = false;

	CDnItemCompounder::S_ITEM_SETTING_INFO ItemSettingInfo;
	CDnItemCompounder::S_OUTPUT Output;

	// Note: Use INT64 later if needed.
	ItemSettingInfo.iHasMoney = GetCoin();
	ItemSettingInfo.iCompoundTableID = iCompoundTableID;
	ItemSettingInfo.iDiscountedCost = iDiscountedCost;

	int iNumItemInfos = (int)vlItemCompoundInfo.size();
	for( int iItem = 0; iItem < iNumItemInfos; ++iItem )
	{
		ItemCompoundInfo& Info = vlItemCompoundInfo.at( iItem );
		ItemSettingInfo.aiItemID[ iItem ] = Info.iItemID;
		ItemSettingInfo.aiItemCount[ iItem ] = Info.iUseCount;
	}

	m_pItemCompounder->Compound( ItemSettingInfo, &Output );

	// First check on the client side if it is valid
	if( CDnItemCompounder::R_SUCCESS == Output.eResultCode )
	{
		CSCompoundItemReq CompoundItemReq;
		ZeroMemory( &CompoundItemReq, sizeof(CSCompoundItemReq) );
		CompoundItemReq.nCompoundTableID = iCompoundTableID;
		CompoundItemReq.biNeedItemSerialID = biNeedItemSerialID;

		/*
		for( int iItem = 0; iItem < iNumItemInfos; ++iItem )
		{
			ItemCompoundInfo& Info = vlItemCompoundInfo.at( iItem );
			_ASSERT( Info.iItemID > 0 );
			if( Info.iItemID > 0 )
			{
				CompoundItemReq.aiItemSettingID[ iItem ] = Info.iItemID;
				CompoundItemReq.aiItemSettingCount[ iItem ]= Info.iUseCount;
			}
		}
		*/

		SendItemCompoundReq( CompoundItemReq );

		EmblemCompoundResult Result;
		Result.vlItemlInfosToRemove = vlItemCompoundInfo;
		Result.Output = Output;
		m_dqWaitEmblemCompoundReq.push_back( Result );

		bResult = true;
		m_bRequestWait = true;
	}
	else
	{
		// TODO: Failed to create crest on client side, display UI message.
	}

	return bResult;
}


void CDnItemTask::RequestEmblemCompoundCancel( void )
{
	CSEmblemCompoundCancelReq req;
	req.nCompoundTableID = -1;
	SendEmblemCompoundCancelReq( req );
}


void CDnItemTask::RequestItemCompoundCancel( void )
{
	CSItemCompoundCancelReq req;
	req.nCompoundTableID = -1;
	SendItemCompoundCancelReq( req );
}

void CDnItemTask::RequestRandomItemCancel()
{
	SendCancelRandomItem();
	m_bRequestWait = false;
}

// Request item disassembly
void CDnItemTask::RequestItemDisjoint( CDnItem* pItem, UINT nNpcUniqueID )
{
	if( NULL == pItem )
		return;

	CSItemDisjointReq Req;
	Req.cSlotIndex = pItem->GetSlotIndex();
	Req.biItemSerial = pItem->GetSerialID();
	Req.nCount = pItem->GetOverlapCount();

	// Send only the slot index since there is no need to send too much data; TItem is large - siva
//	Req.ItemInfo.Item.nItemID = pItem->GetClassID();
//	Req.ItemInfo.Item.nSerial = pItem->GetSerialID();
//	Req.ItemInfo.Item.nRandomSeed = pItem->GetSeed();
//	Req.ItemInfo.Item.wCount = 1; // Set to 1 for now.
	Req.nNpcUniqueID = nNpcUniqueID;

	SendItemDisjointReq( Req );

	// Already checked everything at the dialog level...
	//if( pItem->CanDisjoint() )
	//{
	//	if( pItem->GetDisjointCost() <= GetCoin() )
	//	{
	//
	//	}
	//	else
	//	{
	//		// Not enough money.
	//	}
	//}
	//else
	//{
	//	// Non-disassemblable item
	//}
}

//void CDnItemTask::RequestItemDisjoint( float fDelayTime, int nCount, CDnItem *pItems )
//{
//	if( fDelayTime < 0.0f )
//		return;
//
//	m_fDelayDisjoint = fDelayTime;
//}

//void CDnItemTask::ProcessDisjointReqeust()
//{
//	if( m_fDelayDisjoint < 0.0f )
//		return;
//}

//
//bool CDnItemTask::RequestJewelUpgrade( CDnItem* pJewelItems )
//{
//	bool bResult = false;
//	_ASSERT( NULL != pJewelItems );
//
//	int iJewelItemID = pJewelItems->GetClassID();
//
//	int iCost = 0;
//	int iNumUse = 0;
//	int iResultJewelItemID = m_pEmblemFactory->UpgradeJewel( iJewelItemID, pJewelItems->GetOverlapCount(), &iCost, &iNumUse );
//
//	if( (UINT)iCost < GetCoin() && iResultJewelItemID > 0 )
//	{
//		CSUpgradeJewelReq UpgradeJewel;
//		ZeroMemory( &UpgradeJewel, sizeof(CSUpgradeJewelReq) );
//		UpgradeJewel.JewelItemInfo.cSlotIndex = pJewelItems->GetSlotIndex();
//		UpgradeJewel.JewelItemInfo.Item.nItemID = iJewelItemID;
//		UpgradeJewel.JewelItemInfo.Item.nRandomSeed = pJewelItems->GetSeed();
//		UpgradeJewel.JewelItemInfo.Item.nSerial = pJewelItems->GetSerialID();
//		UpgradeJewel.JewelItemInfo.Item.wCount = pJewelItems->GetOverlapCount();
//
//		SendJewelUpgradeReq( UpgradeJewel );
//
//		// Looks like jewel synthesis doesn't need to be saved separately..
//		JewelUpgradeResult UpgradeResult;
//		UpgradeResult.iCost = iCost;
//		UpgradeResult.iNewJewelItemID = iResultJewelItemID;
//		m_dqWaitJewelUpgradeReq.push_back( UpgradeResult );
//
//		bResult = true;
//		m_bRequestWait = true;
//	}
//	else
//	{
//		// TODO: Jewel synthesis failed on client side. Display UI.
//	}
//
//	return bResult;
//}


void CDnItemTask::PlayPickupSound( CDnItem *pItem )
{
	if( pItem->GetRootSoundIndex() == -1 ) return;

	CEtSoundEngine::GetInstance().PlaySound( "2D", pItem->GetRootSoundIndex() );
}

void CDnItemTask::PlayItemMoveSound( SCMoveItem *pPacket )
{
	// If it failed, it will not be called.
	//pPacket->nRetCode;

	// Determines whether it is drag or use depending on Move type.
	bool bDrag = true;
	switch( pPacket->cMoveType )
	{
	case MoveType_Equip:
	case MoveType_InvenToEquip:
	case MoveType_CashEquip:
	case MoveType_CashInvenToCashEquip:
#if defined(PRE_ADD_TALISMAN_SYSTEM)
	case MoveType_InvenToTalisman:
#endif
		bDrag = false;
		break;
	case MoveType_Inven:
	case MoveType_Ware:
	case MoveType_QuestInven:
	case MoveType_EquipToInven:
	case MoveType_InvenToWare:
	case MoveType_WareToInven:
	case MoveType_GuildWare:
	case MoveType_InvenToGuildWare:
	case MoveType_GuildWareToInven:
	case MoveType_CashInven:
	case MoveType_CashEquipToCashInven:
#if defined(PRE_ADD_TALISMAN_SYSTEM)
	case MoveType_Talisman:
	case MoveType_TalismanToInven:
#endif
		bDrag = true;
		break;
	}

	// If both Src and Dest exist, use DestItem.
	int nPlayItemID = 0;
	if( pPacket->SrcItem.Item.nItemID > 0 ) nPlayItemID = pPacket->SrcItem.Item.nItemID;
	if( pPacket->DestItem.Item.nItemID > 0 ) nPlayItemID = pPacket->DestItem.Item.nItemID;
	if( nPlayItemID == 0 ) return;

	PlayItemSound( nPlayItemID, bDrag ? ItemSound_Drag : ItemSound_Use );
}

void CDnItemTask::PlayItemSound( int nItemID, eItemSoundType Type )
{
	// Return if invalid.
	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TITEM );
	if( !pSox ) return;
	if( !pSox->IsExistItem( nItemID ) ) return;

	// Creating an item to play does too much work, so just obtain the sound handle separately.
	const char *szFileName;
	char szLabel[32] = {0,};
	int nSoundIndex = -1;
	switch( Type )
	{
	case ItemSound_Root:	sprintf_s( szLabel, _countof(szLabel), "_RootSound" );	break;
	case ItemSound_Drag:	sprintf_s( szLabel, _countof(szLabel), "_DragSound" );	break;
	case ItemSound_Use:		sprintf_s( szLabel, _countof(szLabel), "_UseSound" );	break;
	default:				_ASSERT(0&&"ItemTask::PlayItemSound Invalid type");		return;
	}

#ifdef PRE_FIX_MEMOPT_EXT
	szFileName = CommonUtil::GetFileNameFromFileEXT(pSox, nItemID, szLabel);
#else
	szFileName = pSox->GetFieldFromLablePtr( nItemID, szLabel )->GetString();
#endif
	if( szFileName && strlen( szFileName ) > 0 ) {
		nSoundIndex = CEtSoundEngine::GetInstance().LoadSound( CEtResourceMng::GetInstance().GetFullName( szFileName ).c_str(), false, false );
	}

	// Return if no sound.
	if( nSoundIndex != -1 ) {
		CEtSoundEngine::GetInstance().PlaySound( "2D", nSoundIndex );
		CEtSoundEngine::GetInstance().RemoveSound( nSoundIndex );
	}
}

CDnItem* CDnItemTask::FindItem( int nItemTableID, ITEM_SLOT_TYPE slotType )
{
	switch( slotType )
	{
	case ITEM_SLOT_TYPE::ST_CHARSTATUS:		return FindItemEquip( nItemTableID );
	case ITEM_SLOT_TYPE::ST_QUICKSLOT:
	case ITEM_SLOT_TYPE::ST_INVENTORY:		return m_CharInventory.FindItem( nItemTableID );
	case ITEM_SLOT_TYPE::ST_STORAGE_PLAYER:	return m_StorageInventory.FindItem( nItemTableID );
	case ITEM_SLOT_TYPE::ST_STORAGE_GUILD:	return m_GuildInventory.FindItem( nItemTableID );
	case ITEM_SLOT_TYPE::ST_INVENTORY_CASH:	return m_CashInventory.FindItem( nItemTableID );
#ifdef PRE_ADD_SERVER_WAREHOUSE
	case ITEM_SLOT_TYPE::ST_STORAGE_WORLDSERVER_NORMAL: return m_WorldServerStorageInventory.FindItem(nItemTableID);
	case ITEM_SLOT_TYPE::ST_STORAGE_WORLDSERVER_CASH: return m_WorldServerStorageCashInventory.FindItem(nItemTableID);
#endif

	case ITEM_SLOT_TYPE::ST_INVENTORY_VEHICLE: return m_PetInventory.FindItem( nItemTableID );

	default:
		ASSERT(0&&"CDnItemTask::FindItem");
		break;
	}

	return NULL;
}

CDnItem* CDnItemTask::FindItemEquip( int nItemTableID )
{
	CDnItem *pItem(NULL);

	for( DWORD i=0; i<EQUIPMAX; i++ )
	{
		pItem = m_pEquip[i];

		if( pItem && (pItem->GetClassID() == nItemTableID) )
		{
			return pItem;
		}
	}

	// Also search the cash EQUIP window - by LEEKH 2010-08-03
	for( DWORD i=0; i<CASHEQUIPMAX; i++ )
	{
		pItem = m_pCashEquip[i];

		if( pItem && (pItem->GetClassID() == nItemTableID) )
		{
			return pItem;
		}
	}

	return NULL;
}

int CDnItemTask::FindItemFromItemType( eItemTypeEnum Type, ITEM_SLOT_TYPE slotType, std::vector<CDnItem *> &pVecResult )
{
	switch( slotType )
	{
	case ITEM_SLOT_TYPE::ST_CHARSTATUS:		return 0;
	case ITEM_SLOT_TYPE::ST_QUICKSLOT:		return 0;
	case ITEM_SLOT_TYPE::ST_INVENTORY:		m_CharInventory.FindItemFromItemType( Type, pVecResult ); break;
	case ITEM_SLOT_TYPE::ST_STORAGE_PLAYER:	m_StorageInventory.FindItemFromItemType( Type, pVecResult ); break;
	case ITEM_SLOT_TYPE::ST_STORAGE_GUILD:	m_GuildInventory.FindItemFromItemType( Type, pVecResult ); break;
	case ITEM_SLOT_TYPE::ST_INVENTORY_CASH:m_CashInventory.FindItemFromItemType( Type, pVecResult ); break;
#ifdef PRE_ADD_SERVER_WAREHOUSE
	case ITEM_SLOT_TYPE::ST_STORAGE_WORLDSERVER_NORMAL: m_WorldServerStorageInventory.FindItemFromItemType(Type, pVecResult); break;
	case ITEM_SLOT_TYPE::ST_STORAGE_WORLDSERVER_CASH: m_WorldServerStorageCashInventory.FindItemFromItemType(Type, pVecResult); break;
#endif
	default:
		ASSERT(0&&"CDnItemTask::FindItem");
		break;
	}

	return (int)pVecResult.size();
}

int CDnItemTask::FindItemCountFromItemType( eItemTypeEnum Type, int nTypeParam1 )
{
	// Check both normal and cash inventories.
	std::vector<CDnItem*> pVecResult;
	m_CharInventory.FindItemFromItemType( Type, pVecResult );
	m_CashInventory.FindItemFromItemType( Type, pVecResult );
	int nCount = 0;
	for( int i = 0; i < (int)pVecResult.size(); ++i )
	{
		if( nTypeParam1 != -1 && pVecResult[i]->GetTypeParam() != nTypeParam1 ) continue;
		nCount += pVecResult[i]->GetOverlapCount();
	}
	return nCount;
}

int CDnItemTask::FindItemListFromItemTypeEx( eItemTypeEnum Type, std::vector<CDnItem *> &pVecResult, int nTypeParam1 )
{
	// Check both normal and cash inventories.
	std::vector<CDnItem*> pTempVecResult;
	m_CharInventory.FindItemFromItemType( Type, pTempVecResult );
	m_CashInventory.FindItemFromItemType( Type, pTempVecResult );
	int nCount = 0;
	for( int i = 0; i < (int)pTempVecResult.size(); ++i )
	{
		if( nTypeParam1 != -1 && pTempVecResult[i]->GetTypeParam() != nTypeParam1 ) continue;
		nCount += pTempVecResult[i]->GetOverlapCount();
		pVecResult.push_back( pTempVecResult[i] );
	}
	return nCount;
}


static bool CompareItemID( CDnItem *s1, CDnItem *s2 )
{
	if( s1->GetSealCount() < s2->GetSealCount() ) return true;
	else if( s1->GetSealCount() > s2->GetSealCount() ) return false;

	if( s1->GetSerialID() < s2->GetSerialID() ) return true;
	else if( s1->GetSerialID() > s2->GetSerialID() ) return false;

	if( s1->GetClassID() < s2->GetClassID() ) return true;
	else if( s1->GetClassID() > s2->GetClassID() ) return false;

	return false;
}

void CDnItemTask::GetItemListForExtendUseItem( std::vector<CDnItem *> &pVecResult, std::vector<int> &VecItemID, std::vector<bool> &VecEternity )
{
	// Build consume list when normal and cash item materials of the same type exist (like Jelly or Seal).
	// Unlike the server, check as minimally as possible (do not check quantity either).
	std::vector<CDnItem *> pVecNoEternityCashItem;	// Time-limited cash
	std::vector<CDnItem *> pVecItem;				// Normal item
	std::vector<CDnItem *> pVecEternityCashItem;	// Cash
	for( int i = 0; i < (int)pVecResult.size(); ++i ) {
		if( pVecResult[i]->IsCashItem() ) {
			if( pVecResult[i]->IsEternityItem() ) pVecEternityCashItem.push_back( pVecResult[i] );
			else pVecNoEternityCashItem.push_back( pVecResult[i] );	// Add here if client-side expiration check for timed items is added.
		}
		else pVecItem.push_back( pVecResult[i] );
	}
	std::sort( pVecNoEternityCashItem.begin(), pVecNoEternityCashItem.end(), CompareItemID );
	std::sort( pVecItem.begin(), pVecItem.end(), CompareItemID );
	std::sort( pVecEternityCashItem.begin(), pVecEternityCashItem.end(), CompareItemID );

	int nItemID = 0;
	for( int i = 0; i < (int)pVecNoEternityCashItem.size(); ++i ) {
		if( nItemID != pVecNoEternityCashItem[i]->GetClassID() ) {
			VecItemID.push_back( pVecNoEternityCashItem[i]->GetClassID() );
			VecEternity.push_back( false );
			nItemID = pVecNoEternityCashItem[i]->GetClassID();
		}
	}
	nItemID = 0;
	for( int i = 0; i < (int)pVecItem.size(); ++i ) {
		if( nItemID != pVecItem[i]->GetClassID() ) {
			VecItemID.push_back( pVecItem[i]->GetClassID() );
			VecEternity.push_back( true );
			nItemID = pVecItem[i]->GetClassID();
		}
	}
	nItemID = 0;
	for( int i = 0; i < (int)pVecEternityCashItem.size(); ++i ) {
		if( nItemID != pVecEternityCashItem[i]->GetClassID() ) {
			VecItemID.push_back( pVecEternityCashItem[i]->GetClassID() );
			VecEternity.push_back( true );
			nItemID = pVecEternityCashItem[i]->GetClassID();
		}
	}
}

void CDnItemTask::RequestAddQuickSlot( int nSlotID, MIInventoryItem::InvenItemTypeEnum Type, int nID )
{
	if( !m_bRequestQuickSlot ) 
		return;

	BYTE cSlotType(QUICKSLOTTYPE_NONE);

	switch( Type ) 
	{
		case MIInventoryItem::Item: cSlotType = QUICKSLOTTYPE_ITEM; break;
		case MIInventoryItem::Skill:cSlotType = QUICKSLOTTYPE_SKILL; break;
		default:
			CDebugSet::ToLogFile( "CDnItemTask::RequestAddQuickSlot, Error!" );
			break;
	}

	SendAddQuickSlot( nSlotID, cSlotType, nID );
}

void CDnItemTask::RequestDelQuickSlot( int nSlotID )
{
	SendDelQuickSlot( nSlotID );
}

void CDnItemTask::RequestAddLifeSkillQuickSlot( int nSlotID, MIInventoryItem::InvenItemTypeEnum Type, INT64 biID )
{
	if( !m_bRequestQuickSlot ) 
		return;

	BYTE cSlotType(QUICKSLOTTYPE_NONE);

	switch( Type ) 
	{
	case MIInventoryItem::Gesture:cSlotType = QUICKSLOTTYPE_GESTURE; break;
	case MIInventoryItem::SecondarySkill:cSlotType = QUICKSLOTTYPE_SECONDARYSKILL; break;
	case MIInventoryItem::Item:cSlotType = QUICKSLOTTYPE_VEHICLE_PET; break;
	default:
		_ASSERT(0&&"Why is an item or skill being requested?");
		CDebugSet::ToLogFile( "CDnItemTask::RequestAddLifeSkillQuickSlot, Error!" );
		break;
	}

	SendAddQuickSlot( nSlotID + LIFESKILL_QUICKSLOT_OFFSET, cSlotType, biID );
}

void CDnItemTask::RequestDelLifeSkillQuickSlot( int nSlotID )
{
	SendDelQuickSlot( nSlotID + LIFESKILL_QUICKSLOT_OFFSET );
}

const CDnItemCompounder::S_PLATE_INFO* CDnItemTask::GetPlateInfoByItemID( int iItemID )
{
	if( !m_pItemCompounder )
		return NULL;

	return m_pItemCompounder->GetPlateInfoByItemID( iItemID );
}

//const CDnEmblemFactory::S_JEWEL_INFO* CDnItemTask::GetJewelInfoByItemID( int iItemID )
//{
//	if( !m_pEmblemFactory )
//		return NULL;
//
//	return m_pEmblemFactory->GetJewelInfoByItemID( iItemID );
//}

void CDnItemTask::GetCompoundInfo( int iCompoundTableID, CDnItemCompounder::S_COMPOUND_INFO_EXTERN* pInfo )
{
	if( !m_pItemCompounder )
		return;

	return m_pItemCompounder->GetCompoundInfo( iCompoundTableID, pInfo );
}

#ifdef PRE_ADD_PARTSITEM_TOOLTIP_INFO
bool CDnItemTask::IsEnableCompoundItem(int itemId) const
{
	if (!m_pItemCompounder)
		return false;

	return m_pItemCompounder->IsEnableCompoundItem(itemId);
}
#endif

void CDnItemTask::MoveInventoryItem( CDnInventory &srcInven, CDnInventory &destInven, TItemInfo &SrcItem, TItemInfo &DestItem )
{
	if( m_nRequestSrcItemSlot != -1 )
	{
		srcInven.RemoveItem( m_nRequestSrcItemSlot );
	}

	if( m_nRequestDestItemSlot != -1 )
	{
		destInven.RemoveItem( m_nRequestDestItemSlot );
	}

	if( SrcItem.Item.nItemID > 0 )
	{
		CDnItem *pSrcItem = CreateItem( SrcItem );
		srcInven.InsertItem( pSrcItem, true );
	}

	if( DestItem.Item.nItemID > 0 )
	{
		CDnItem *pDestItem = CreateItem( DestItem );
		destInven.InsertItem( pDestItem, true );
	}
}

#ifdef PRE_ADD_SERVER_WAREHOUSE
void CDnItemTask::MoveCashInventoryItem(CDnInventory& srcInven, CDnInventory& destInven, TItem& srcItem, const int nSrcSlotIndex, TItem& destItem, const int nDestSlotIndex)
{
	if (m_nRequestSrcItemSlot != -1)
	{
		srcInven.RemoveItem(m_nRequestSrcItemSlot);
	}

	if (m_nRequestDestItemSlot != -1)
	{
		destInven.RemoveItem(m_nRequestDestItemSlot);
	}

	if (srcItem.nItemID > 0)
	{
		CDnItem *pSrcItem = CreateItem(srcItem);
		if (pSrcItem == NULL || nSrcSlotIndex < 0)
		{
			_ASSERT("WSS");
			return;
		}

		pSrcItem->SetSlotIndex(nSrcSlotIndex);
		srcInven.InsertItem(pSrcItem, true);
	}

	if (destItem.nItemID > 0)
	{
		CDnItem *pDestItem = CreateItem(destItem);
		if (pDestItem == NULL || nDestSlotIndex < 0)
		{
			_ASSERT("WSS");
			return;
		}

		pDestItem->SetSlotIndex(nDestSlotIndex);
		destInven.InsertItem(pDestItem, true);
	}
}
#endif

//	todo by kalliste : vectorize coin count variables
void CDnItemTask::SetRebirthCoin(eRebirthCoinType type, int nCoinCount)
{
	if (type == eDAILYCOIN)
		m_nRebirthCoinCount = nCoinCount;
	else if (type == eCASHCOIN)
		m_nRebirthCashCoinCount = nCoinCount;
	else if (type == ePCBANGCOIN)
		m_nRebirthPCBangCoinCount = nCoinCount;
#ifdef PRE_ADD_VIP
	else if (type == eVIPCOIN)
		m_nVIPRebirthCoinCount = nCoinCount;
#endif	// #ifdef PRE_ADD_VIP
}

int CDnItemTask::GetRebirthCoin(eRebirthCoinType type) const
{
	if (type == eDAILYCOIN)			return m_nRebirthCoinCount;
	else if (type == eCASHCOIN)		return m_nRebirthCashCoinCount;
	else if (type == ePCBANGCOIN)	return m_nRebirthPCBangCoinCount;
#ifdef PRE_ADD_VIP
	else if (type == eVIPCOIN)		return m_nVIPRebirthCoinCount;
#else
	else if (type == eVIPCOIN)		return 0;
#endif	// #ifdef PRE_ADD_VIP
	else _ASSERT(0);

	return ILLEGAL_REBIRTH_COIN_COUNT;
}

const WCHAR* CDnItemTask::GetRebirthCoinString(eRebirthCoinType type) const
{
	if (type == ePCBANGCOIN)		return GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 121054 );	// UISTRING : PC Cafe
	else if (type == eDAILYCOIN)	return GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 2020029 );	// UISTRING : Daily
#ifdef PRE_ADD_VIP
	else if (type == eVIPCOIN)		return GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 2020000 );	// UISTRING : VIP
#endif	// #ifdef PRE_ADD_VIP

	return NULL;
}

void CDnItemTask::InsertChangePartsThread( DnActorHandle hActor )
{
	if( !hActor ) return;
	DWORD dwUniqueID = hActor->GetUniqueID();
	CDnLoadingTask::GetInstance().InsertLoadObject( 
		OnLoadRecvChangeParts, 
		NULL,
		NULL, 
		NULL,
		this, (void*)&dwUniqueID, sizeof(DWORD), m_LocalTime );

	/*
	if( !hActor ) return;
	MAPartsBody *pBody = dynamic_cast<MAPartsBody *>(hActor.GetPointer());
	if( !pBody ) return;
	pBody->CombineParts();
	*/
}

int CDnItemTask::CalcRepairEquipPrice()
{
	float fTotalPrice = 0.f;
	CDnItem *pItem = NULL;

	for (int i = 0; i < EQUIPMAX; i++)
	{
		pItem = m_pEquip[i];
		if( !pItem ) continue;
		if( pItem->GetItemType() == ITEMTYPE_WEAPON )
		{
			CDnWeapon *pWeapon = dynamic_cast<CDnWeapon *>(pItem);
			if( pWeapon )
				fTotalPrice += pWeapon->GetRepairPrice();
		}
		else if( pItem->GetItemType() == ITEMTYPE_PARTS )
		{
			CDnParts *pParts = dynamic_cast<CDnParts *>(pItem);
			if( pParts )
				fTotalPrice += pParts->GetRepairPrice();
		}
	}
	if( fTotalPrice != 0.f && fTotalPrice < 1.f ) fTotalPrice = 1.f;
	return (int)fTotalPrice;
}

int CDnItemTask::CalcRepairCharInvenPrice()
{
	return m_CharInventory.GetRepairInvenPrice();
}

CDnItemTask::EquipDurabilityStateEnum CDnItemTask::GetEquipDurabilityState()
{
	float fMinValue = 1.0f;
	CDnItem *pItem = NULL;
	for (int i = 0; i < EQUIPMAX; i++)
	{
		pItem = m_pEquip[i];
		if( !pItem ) continue;
		if( pItem->GetItemType() == ITEMTYPE_WEAPON )
		{
			CDnWeapon *pWeapon = dynamic_cast<CDnWeapon *>(pItem);
			if( pWeapon && pWeapon->GetMaxDurability() > 0 )
			{
				float fRate = (float)pWeapon->GetDurability() / pWeapon->GetMaxDurability();
				fMinValue = MIN(fRate, fMinValue);
			}
		}
		else if( pItem->GetItemType() == ITEMTYPE_PARTS )
		{
			CDnParts *pParts = dynamic_cast<CDnParts *>(pItem);
			if( pParts && pParts->GetMaxDurability() > 0 )
			{
				if( pParts->GetPartsType() >= CDnParts::Helmet && pParts->GetPartsType() <= CDnParts::Foot )
				{
					float fRate = (float)pParts->GetDurability() / pParts->GetMaxDurability();
					fMinValue = MIN(fRate, fMinValue);
				}
			}
		}
	}

	EquipDurabilityStateEnum eResult = Durability_None;
	if( fMinValue == 0.0f )			eResult = CDnItemTask::Durability_Red;
	else if( fMinValue <= 0.1f )	eResult = CDnItemTask::Durability_Orange;
	else if( fMinValue <= 0.2f )	eResult = CDnItemTask::Durability_Yellow;
	else							eResult = CDnItemTask::Durability_None;
	return eResult;
}

void CDnItemTask::RequestRepairEquip()
{
	SendRepairEquip();
	m_bRequestRepair = true;
}

void CDnItemTask::RequestRepairAll()
{
	SendRepairAll();
	m_bRequestRepair = true;
}

void CDnItemTask::RequestInvenToWare( INT64 nMoney )
{
	SendMoveCoin( MoveType_InvenToWare, nMoney );
}

void CDnItemTask::RequestWareToInven( INT64 nMoney )
{
	SendMoveCoin( MoveType_WareToInven, nMoney );
}

void CDnItemTask::RequestInvenToGuildWare( INT64 nMoney )
{
	if( nMoney > 0 )
		SendMoveGuildCoin( MoveType_InvenToGuildWare, nMoney );
}

void CDnItemTask::RequestGuildWareToInven( INT64 nMoney )
{
	if( nMoney > 0 )
		SendMoveGuildCoin( MoveType_GuildWareToInven, nMoney );
}

void CDnItemTask::RequestSortInven()
{
	if( m_CharInventory.PrepareSort() )
	{
		m_bRequestWait = true;
		SendSortInventory( m_CharInventory.GetSortArray(), m_CharInventory.GetSortCount() );
	}
	else
	{
		// Already sorted.
		GetInterface().AddChatMessage( CHATTYPE_SYSTEM, L"", GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 527 ), false );
	}
}

void CDnItemTask::SortCashInven()
{
	if( m_CashInventory.SortCashInventory() )
	{
	}
	else
	{
		// Already sorted.
		GetInterface().AddChatMessage( CHATTYPE_SYSTEM, L"", GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 527 ), false );
	}
}


#ifdef PRE_ADD_CASHREMOVE	
// #52830
void CDnItemTask::SortCashRemoveInven()
{
	if( !m_CashRemoveInventory.SortCashInventory() )
		GetInterface().AddChatMessage( CHATTYPE_SYSTEM, L"", GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 527 ), false ); // "Already sorted."
}
#endif

void CDnItemTask::RequestSortStorage()
{
	if( m_StorageInventory.PrepareSort() )
	{
		m_bRequestWait = true;
		SendSortWarehouse( m_StorageInventory.GetSortArray(), m_StorageInventory.GetSortCount() );
	}
	else
	{
		// Already sorted.
		GetInterface().AddChatMessage( CHATTYPE_SYSTEM, L"", GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 527 ), false );
	}
}

void CDnItemTask::SortVehicleInven()
{
	if( !m_PetInventory.SortCashInventory() )
		GetInterface().AddChatMessage( CHATTYPE_SYSTEM, L"", GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 527 ), false );
}

#ifdef PRE_ADD_GACHA_JAPAN
// Japan gachapon
void CDnItemTask::RequestGacha_JP( int iJobClassID, int iSelectePart )
{
	SendGacha_JP( iJobClassID, iSelectePart );

	m_bSendGachaponReq = true;
}
#endif

#ifdef PRE_ADD_COSRANDMIX

bool CDnItemTask::ReadyItemCostumeMixOpen(eItemTypeEnum type)
{
	if (CDnPartyTask::IsActive() && GetPartyTask().IsLocalActorEnterGateReady())
		return false;

	if( CDnActor::s_hLocalActor )
	{
		CDnLocalPlayerActor *pLocalActor = static_cast<CDnLocalPlayerActor *>(CDnLocalPlayerActor::s_hLocalActor.GetPointer());
		if (pLocalActor != NULL)
		{
			if ((CDnActor::s_hLocalActor->IsMove() && CDnActor::s_hLocalActor->IsMovable()) || pLocalActor->IsAutoRun())
				CDnActor::s_hLocalActor->CmdStop( "Stand", 0, 3.f, 0.f, false, true );

			if (pLocalActor->IsFollowing())
			{
				CDnActor::s_hLocalActor->CmdStop( "Stand", 0, 3.f, 0.f, false, true );
				pLocalActor->ResetAutoRun();
			}
		}
	}

	GetInterface().CloseNpcDialog();
	CDnLocalPlayerActor::LockInput( true );

	CDnMainDlg *pMainDlg = GetInterface().GetMainBarDialog();
	if (pMainDlg)
	{
		pMainDlg->EnableButtons(false);
#ifdef PRE_FIX_HOLD_COSTUMEMIX_INVEN
		pMainDlg->HoldToggleShowDialogOnProcessButton(true, CDnMainMenuDlg::INVENTORY_DIALOG);
#endif
	}

	switch(type)
	{
	case ITEMTYPE_COSTUMEMIX:
		{
			CDnCostumeMixDlg* pDlg = GetInterface().GetCostumeMixDlg();
			if (pDlg)
				pDlg->DisableAllDlgs(true, GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 130317)); // UISTRING : Preparing costume synthesis.
		}
		break;

	case ITEMTYPE_COSTUMEDESIGN_MIX:
		{
			CDnCostumeDesignMixDlg* pDlg = GetInterface().GetCostumeDesignMixDlg();
			if (pDlg)
				pDlg->DisableAllDlgs(true, GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 130317)); // UISTRING : Preparing costume synthesis.
		}
		break;

	case ITEMTYPE_COSTUMERANDOM_MIX:
		{
			CDnCostumeRandomMixDlg* pDlg = GetInterface().GetCostumeRandomMixDlg();
			if (pDlg)
				pDlg->DisableAllDlgs(true, GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 130317)); // UISTRING : Preparing costume synthesis.
		}
		break;

	default:
		{
			_ASSERT(0);
			OutputDebug("[COSMIX] ReadyItemCostumeMixOpen : there is NO MixType(%d)\n", type);
		}
		break;
	}

	return true;
}

#else // PRE_ADD_COSRANDMIX

bool CDnItemTask::ReadyItemCostumeMixOpen(bool bDesignMix)
{
	if (CDnPartyTask::IsActive() && GetPartyTask().IsLocalActorEnterGateReady())
		return false;

	if( CDnActor::s_hLocalActor )
	{
		CDnLocalPlayerActor *pLocalActor = static_cast<CDnLocalPlayerActor *>(CDnLocalPlayerActor::s_hLocalActor.GetPointer());
		if (pLocalActor != NULL)
		{
			if ((CDnActor::s_hLocalActor->IsMove() && CDnActor::s_hLocalActor->IsMovable()) || pLocalActor->IsAutoRun())
				CDnActor::s_hLocalActor->CmdStop( "Stand", 0, 3.f, 0.f, false, true );

			if (pLocalActor->IsFollowing())
			{
				CDnActor::s_hLocalActor->CmdStop( "Stand", 0, 3.f, 0.f, false, true );
				pLocalActor->ResetAutoRun();
			}
		}
	}

	GetInterface().CloseNpcDialog();
	CDnLocalPlayerActor::LockInput( true );

	CDnMainDlg *pMainDlg = GetInterface().GetMainBarDialog();
	if (pMainDlg)
	{
		pMainDlg->EnableButtons(false);
#ifdef PRE_FIX_HOLD_COSTUMEMIX_INVEN
		pMainDlg->HoldToggleShowDialogOnProcessButton(true, CDnMainMenuDlg::INVENTORY_DIALOG);
#endif
	}

	if (bDesignMix)
	{
		CDnCostumeDesignMixDlg* pDlg = GetInterface().GetCostumeDesignMixDlg();
		if (pDlg)
			pDlg->DisableAllDlgs(true, GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 130317)); // UISTRING : Preparing costume synthesis.
	}
	else
	{
		CDnCostumeMixDlg* pDlg = GetInterface().GetCostumeMixDlg();
		if (pDlg)
			pDlg->DisableAllDlgs(true, GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 130317)); // UISTRING : Preparing costume synthesis.
	}

	return true;
}

#endif // PRE_ADD_COSRANDMIX

bool CDnItemTask::RequestItemCostumeMixOpen()
{
	if (m_bLockCostumeSendPacket)
		return false;

	if (GetInterface().IsShowCostumeMixDlg())
		return false;

	m_bLockCostumeSendPacket = true;
	GetInterface().CloseNpcDialog();
	CDnLocalPlayerActor::LockInput( true );

	SendCostumeMixOpen();

	return true;
}

bool CDnItemTask::RequestItemCostumeMixClose()
{
	if (m_bLockCostumeSendPacket)
		return false;

	if (GetInterface().IsShowCostumeMixDlg() == false)
		return false;

	m_bLockCostumeSendPacket = true;
	SendCostumeMixClose();

	return true;
}

bool CDnItemTask::RequestItemCostumeMixCloseComplete()
{
	if (m_bLockCostumeSendPacket)
		return false;

	if (GetInterface().IsShowCostumeMixDlg() == false)
		return false;

	m_bLockCostumeSendPacket = true;
	SendCostumeMixCloseComplete();

	return true;
}

bool CDnItemTask::RequestItemCostumeMix(INT64* pSerials, int count, UINT resultItemId, char ability)
{
	if (m_bLockCostumeSendPacket)
		return false;

	if (GetInterface().IsShowCostumeMixDlg() == false)
		return false;

	m_bLockCostumeSendPacket = true;

	SendCostumeMix(pSerials, count, resultItemId, ability);

	return true;
}

bool CDnItemTask::RequestItemCostumeDesignMixCloseComplete()
{
	if (m_bLockCosDesignMixSendPacket)
		return false;

	if (GetInterface().IsShowCostumeDesignMixDlg() == false)
		return false;

	m_bLockCosDesignMixSendPacket = true;
	SendCostumeDesignMixCloseComplete();

	return true;
}

bool CDnItemTask::RequestItemCostumeDesignMixClose()
{
	if (m_bLockCosDesignMixSendPacket)
		return false;

	if (GetInterface().IsShowCostumeDesignMixDlg() == false)
		return false;

	m_bLockCosDesignMixSendPacket = true;
	SendCostumeDesignMixClose();

	return true;
}

bool CDnItemTask::RequestItemCostumeDesignMix(INT64* pSerials)
{
	if (m_bLockCosDesignMixSendPacket)
		return false;

	if (GetInterface().IsShowCostumeDesignMixDlg() == false)
		return false;

	m_bLockCosDesignMixSendPacket = true;

	SendCostumeDesignMix(pSerials);

	return true;
}

#ifdef PRE_ADD_COSRANDMIX
bool CDnItemTask::RequestItemCostumeRandomMixClose()
{
	if (m_bLockCosRandomMixSendPacket)
		return false;

	if (GetInterface().IsShowCostumeRandomMixDlg() == false)
		return false;

	m_bLockCosRandomMixSendPacket = true;
	SendCostumeRandomMixClose();

	return true;
}

bool CDnItemTask::RequestItemCostumeRandomMixCloseComplete()
{
	if (m_bLockCosRandomMixSendPacket)
		return false;

	if (GetInterface().IsShowCostumeRandomMixDlg() == false)
		return false;

	m_bLockCosRandomMixSendPacket = true;
	SendCostumeRandomMixCloseComplete();

	return true;
}

bool CDnItemTask::RequestItemCostumeRandomMix(INT64* pSerials)
{
	if (m_bLockCosRandomMixSendPacket)
		return false;

	if (GetInterface().IsShowCostumeRandomMixDlg() == false)
		return false;

	m_bLockCosRandomMixSendPacket = true;

	SendCostumeRandomMix(pSerials);

	return true;
}
#endif

#if defined(PRE_PERIOD_INVENTORY)
void CDnItemTask::RequestSortPeriodInven()
{
	if( m_CharInventory.PrepareSort( true ) )
	{
		m_bRequestWait = true;
		SendSortPeriodInventory( m_CharInventory.GetSortArray(), m_CharInventory.GetSortCount() );
	}
	else
	{
		// Already sorted.
		GetInterface().AddChatMessage( CHATTYPE_SYSTEM, L"", GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 527 ), false );
	}

}

void CDnItemTask::RequestSortPeriodStorage()
{
	if( m_StorageInventory.PrepareSort( true ) )
	{
		m_bRequestWait = true;
		SendSortPeriodWarehouse( m_StorageInventory.GetSortArray(), m_StorageInventory.GetSortCount() );
	}
	else
	{
		// Already sorted.
		GetInterface().AddChatMessage( CHATTYPE_SYSTEM, L"", GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 527 ), false );
	}

}
#endif	// #if defined(PRE_PERIOD_INVENTORY)

#ifdef PRE_ADD_CHANGEJOB_CASHITEM
bool CDnItemTask::RequestUseChangeJobCashItem( INT64 biItemSerial, int iJobID )
{
	if( m_bSendChangeJobReq )
		return false;

	SendUseChangeJobCashItem( biItemSerial, iJobID );
	m_bSendChangeJobReq = true;

	return true;
}
#endif // #ifdef PRE_ADD_CHANGEJOB_CASHITEM


bool __stdcall CDnItemTask::OnLoadRecvChangeParts( void *pThis, void *pParam, int nSize, LOCAL_TIME LocalTime )
{
	DWORD dwUniqueID;
	memcpy( &dwUniqueID, pParam, sizeof(DWORD) );

	DnActorHandle hActor = CDnActor::FindActorFromUniqueID( dwUniqueID );
	if( !hActor ) return false;
	MAPartsBody *pBody = dynamic_cast<MAPartsBody *>(hActor.GetPointer());
	if( !pBody ) return false;

	if( CDnLoadingTask::GetInstance().CheckSameLoadObject( pParam, nSize ) ) return true;

	pBody->CombineParts();
	return true;
}

void CDnItemTask::SetFatigue( int nFatigue, int nWeekFatigue, int nPCBangFatigue, int nEventFatigue )
{
	m_sFatigue.nFatigue = nFatigue;
	m_sWeekFatigue.nFatigue = nWeekFatigue;
	m_sPCBangFatigue.nFatigue = nPCBangFatigue;
	m_sEventFatigue.nFatigue = nEventFatigue;
}

void CDnItemTask::GetFatigue( int &nFatigue, int &nWeekFatigue, int &nPCBangFatigue, int &nEventFatigue )
{
	nFatigue = m_sFatigue.nFatigue;
	nWeekFatigue = m_sWeekFatigue.nFatigue;
	nPCBangFatigue = m_sPCBangFatigue.nFatigue;
	nEventFatigue = m_sEventFatigue.nFatigue;
}

void CDnItemTask::SetMaxFatigue( int nFatigue, int nWeekFatigue, int nPCBangFatigue, int nEventFatigue )
{
	m_sFatigue.nMaxFatigue = nFatigue;
	m_sWeekFatigue.nMaxFatigue = nWeekFatigue;
	m_sPCBangFatigue.nMaxFatigue = nPCBangFatigue;
	m_sEventFatigue.nMaxFatigue = nEventFatigue;
}

void CDnItemTask::GetMaxFatigue( int &nFatigue, int &nWeekFatigue, int &nPCBangFatigue, int &nEventFatigue )
{
	nFatigue = m_sFatigue.nMaxFatigue;
	nWeekFatigue = m_sWeekFatigue.nMaxFatigue;
	nPCBangFatigue = m_sPCBangFatigue.nMaxFatigue;
	nEventFatigue = m_sEventFatigue.nMaxFatigue;
}

void CDnItemTask::SetFatigue(eFatigueType type, int value)
{
	if (type == eDAILYFTG)			m_sFatigue.nFatigue			= value;
	else if (type == eWEEKLYFTG)	m_sWeekFatigue.nFatigue		= value;
	else if (type == ePCBANGFTG)	m_sPCBangFatigue.nFatigue	= value;
	else if (type == eEVENTFTG)	m_sEventFatigue.nFatigue	= value;
#ifdef PRE_ADD_VIP
	else if (type == eVIPFTG)		m_sVIPFatigue.nFatigue		= value;
#endif	// #ifdef PRE_ADD_VIP
}

void CDnItemTask::SetMaxFatigue(eFatigueType type, int value)
{
	if (type == eDAILYFTG)			m_sFatigue.nMaxFatigue		= value;
	else if (type == eWEEKLYFTG)	m_sWeekFatigue.nMaxFatigue	= value;
	else if (type == ePCBANGFTG)	m_sPCBangFatigue.nMaxFatigue = value;
	else if (type == eEVENTFTG)	m_sEventFatigue.nMaxFatigue	= value;
#ifdef PRE_ADD_VIP
	else if (type == eVIPFTG)		m_sVIPFatigue.nMaxFatigue	= value;
#endif	// #ifdef PRE_ADD_VIP
}

int	CDnItemTask::GetFatigue(eFatigueType type) const
{
	if (type == eDAILYFTG)			return m_sFatigue.nFatigue;
	else if (type == eWEEKLYFTG)	return m_sWeekFatigue.nFatigue;
	else if (type == ePCBANGFTG)	return m_sPCBangFatigue.nFatigue;
	else if (type == eEVENTFTG)	return m_sEventFatigue.nFatigue;
#ifdef PRE_ADD_VIP
	else if (type == eVIPFTG)		return m_sVIPFatigue.nFatigue;
#endif	// #ifdef PRE_ADD_VIP

	return -1;
}

int	CDnItemTask::GetMaxFatigue(eFatigueType type) const
{
	if (type == eDAILYFTG)			return m_sFatigue.nMaxFatigue;
	else if (type == eWEEKLYFTG)	return m_sWeekFatigue.nMaxFatigue;
	else if (type == ePCBANGFTG)	return m_sPCBangFatigue.nMaxFatigue;
	else if (type == eEVENTFTG)	return m_sEventFatigue.nMaxFatigue;
#ifdef PRE_ADD_VIP
	else if (type == eVIPFTG)		return m_sVIPFatigue.nMaxFatigue;
#endif	// #ifdef PRE_ADD_VIP

	return -1;
}

void CDnItemTask::OnChangedFatigue( int nFatigue, int nWeekFatigue, int nPCBangFatigue, int nEventFatigue, int nVIPFatigue )
{
	// Do not check with else if.
#ifdef PRE_ADD_VIP
	if( m_sVIPFatigue.nFatigue > 0 && nVIPFatigue == 0 )
	{
		GetInterface().AddChatMessage( CHATTYPE_SYSTEM, L"", GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 2020026 ), false );	// UISTRING : You have used all your VIP FTG.
	}
#endif	// #ifdef PRE_ADD_VIP

	if( m_sEventFatigue.nFatigue > 0 && nEventFatigue == 0 )
	{
		GetInterface().AddChatMessage( CHATTYPE_SYSTEM, L"", GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 101009 ), false );
	}

	if( m_sPCBangFatigue.nFatigue > 0 && nPCBangFatigue == 0 )
	{
		GetInterface().AddChatMessage( CHATTYPE_SYSTEM, L"", GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 100305 ), false );
	}

	if( m_sFatigue.nFatigue > 0 && nFatigue == 0 )
	{
		GetInterface().AddChatMessage( CHATTYPE_SYSTEM, L"", GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 100306 ), false );
	}

	if( m_sWeekFatigue.nFatigue > 0 && nWeekFatigue == 0 )
	{
		GetInterface().AddChatMessage( CHATTYPE_SYSTEM, L"", GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 100307 ), false );
	}
}

void CDnItemTask::OnRecvCharFatigue( SCFatigue *pData )
{
	if( ERROR_NONE != pData->iResult )
	{
		if (pData->iResult == ERROR_CANT_USE_ITEM_DAYILY){
			int iFatigueLimit = static_cast<int>(CGlobalWeightTable::GetInstance().GetValue(CGlobalWeightTable::FatigueLimit));
			WCHAR wzStrTmp[1024]={0,};
			swprintf_s(wzStrTmp, _countof(wzStrTmp), GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 114044), iFatigueLimit);
			GetInterface().MessageBox( wzStrTmp, MB_OK );
		}
		else
			GetInterface().ServerMessageBox(pData->iResult);

		return;
	}

	ASSERT( pData&&"CDnItemTask::OnRecvCharFatigue" );
	for( DWORD i=0; i<CDnPartyTask::GetInstance().GetPartyCount(); i++ ) {
		CDnPartyTask::PartyStruct *pStruct = CDnPartyTask::GetInstance().GetPartyData(i);
		if( !pStruct ) continue;
		if( pStruct->nSessionID == pData->nSessionID ) {
			// wFatigue in PartyStruct does not need to be stored separately, so sum it up.
#ifdef PRE_ADD_VIP
			pStruct->wFatigue = pData->wFatigue + pData->wWeekFatigue + pData->wPCBangFatigue + pData->wEventFatigue + pData->wVIPFatigue;
#else
			pStruct->wFatigue = pData->wFatigue + pData->wWeekFatigue + pData->wPCBangFatigue + pData->wEventFatigue;
#endif
		}
	}
	if( pData->nSessionID == CDnBridgeTask::GetInstance().GetSessionID() ) {
#ifdef PRE_ADD_VIP
		OnChangedFatigue( pData->wFatigue, pData->wWeekFatigue, pData->wPCBangFatigue, pData->wEventFatigue, pData->wVIPFatigue );
#else	// #ifdef PRE_ADD_VIP
		OnChangedFatigue( pData->wFatigue, pData->wWeekFatigue, pData->wPCBangFatigue, pData->wEventFatigue, 0 );
#endif	// #ifdef PRE_ADD_VIP

		SetFatigue( pData->wFatigue, pData->wWeekFatigue, pData->wPCBangFatigue, pData->wEventFatigue );
#ifdef PRE_ADD_VIP
		SetFatigue(eVIPFTG, pData->wVIPFatigue);
#endif	// #ifdef PRE_ADD_VIP
	}
}

bool CDnItemTask::IsCanRebirth()
{
	if( IsPVP() )
		return IsCanPVPRebirth();

	int nRebirthCoinCount = GetRebirthCoinCount();

	int nUsableCoinCount = GetUsableRebirthCoin();
	CDnItem* pRebirthItem = FindRebirthItem(0);

	if ( nRebirthCoinCount <= 0 )
		return (pRebirthItem != NULL);
	else
		return ( nUsableCoinCount > 0 || CommonUtil::IsInfiniteRebirthOnDungeon() || pRebirthItem != NULL );

	return false;
}

#ifdef PRE_ADD_INSTANT_CASH_BUY
bool CDnItemTask::IsCanRebirthIfHaveCoin()
{
	if( GetUsableRebirthCoin() > 0 || CommonUtil::IsInfiniteRebirthOnDungeon() )
		return true;

	return false;
}
#endif // PRE_ADD_INSTANT_CASH_BUY

int CDnItemTask::GetRebirthCoinCount()
{
	int nCoinCount = 0;

	int nRebirthCoin, nRebirthCashCoin, nRebirthPCBangCoin;
	nRebirthCoin		= GetRebirthCoin(CDnItemTask::eDAILYCOIN);
	nRebirthCashCoin	= GetRebirthCoin(CDnItemTask::eCASHCOIN);
	nRebirthPCBangCoin	= GetRebirthCoin(CDnItemTask::ePCBANGCOIN);
#ifdef PRE_ADD_VIP
	int nRebirthVIPCoin = GetRebirthCoin(CDnItemTask::eVIPCOIN);

	nCoinCount = ((nRebirthCoin == ILLEGAL_REBIRTH_COIN_COUNT) ? 0 : nRebirthCoin) +
				 ((nRebirthCashCoin == ILLEGAL_REBIRTH_COIN_COUNT) ? 0 : nRebirthCashCoin) +
				 ((nRebirthPCBangCoin == ILLEGAL_REBIRTH_COIN_COUNT) ? 0 : nRebirthPCBangCoin) +
				 ((nRebirthVIPCoin == ILLEGAL_REBIRTH_COIN_COUNT) ? 0 : nRebirthVIPCoin);

#else	// #ifdef PRE_ADD_VIP
	nCoinCount = ((nRebirthCoin		 == ILLEGAL_REBIRTH_COIN_COUNT) ? 0 : nRebirthCoin) +
				 ((nRebirthCashCoin	 == ILLEGAL_REBIRTH_COIN_COUNT) ? 0 : nRebirthCashCoin) +
				 ((nRebirthPCBangCoin == ILLEGAL_REBIRTH_COIN_COUNT) ? 0 : nRebirthPCBangCoin);
#endif	// #ifdef PRE_ADD_VIP

	return nCoinCount;
}

//blondy
bool CDnItemTask::IsPVP()
{
	CDnGameTask *pGameTask = (CDnGameTask *)CTaskManager::GetInstance().GetTask( "GameTask" );
	if( pGameTask && pGameTask->GetGameTaskType() == GameTaskType::PvP ) 
		return true;
	return false;
}

bool CDnItemTask::IsCanPVPRebirth()
{
	//Currently there is no design for reviving via items in PVP 
	return false;
}
//blondyend

//blondy
//Is this item usable in the current game mode?

int CDnItemTask::GetAllowGameMode() const
{	
	return CDnWorld::GetInstance().GetAllowMapType();
};

bool CDnItemTask::IsDisableItemInCharInven( CDnItem * pItem )
{
	if( !pItem )
		return false;

#ifdef PRE_ADD_COSRANDMIX
	bool bEnable = (pItem->GetAllowedGameType() & GetAllowGameMode()) ? true : false;

	if (bEnable)
	{
		if (GetInterface().IsShowCostumeRandomMixDlg())
		{
			bEnable = m_CosRandMixDataMgr.IsEnableCostumeRandomMixStuff(pItem);
		}

		if (GetInterface().IsShowCostumeDesignMixDlg())
		{
			bEnable = m_CosMixDataMgr.IsEnableCostumeDesignMixStuff(pItem);
		}

		if (GetInterface().IsShowCostumeMixDlg())
		{
			if (pItem->GetItemType() == ITEMTYPE_PARTS)
			{
				const CDnParts* pParts = static_cast<CDnParts*>(pItem);
				bEnable = m_CosMixDataMgr.IsEnableCostumeMixStuff(pParts);
			}
			else
			{
				bEnable = false;
			}
		}
	}

	return !bEnable;
#else
	return (pItem->GetAllowedGameType() & GetAllowGameMode() ? false : true);
#endif
}

#ifdef PRE_ADD_SERVER_WAREHOUSE
StorageUIDef::eError CDnItemTask::IsEnableStoreOnWorldServerStorage(CDnItem* pItem)
{
	CDnPGStorageTabDlg* pPGStorageTabDlg = static_cast<CDnPGStorageTabDlg*>(GetInterface().GetMainMenuDialog(CDnMainMenuDlg::PLAYER_GUILD_STORAGE_DIALOG));
	if (pPGStorageTabDlg && pPGStorageTabDlg->IsShow())
	{
		CDnWorldServerStorageInventoryDlg* pWorldServerStorageInvenDlg = static_cast<CDnWorldServerStorageInventoryDlg*>(pPGStorageTabDlg->GetStorageDlg(StorageUIDef::eTAB_WORLDSERVER_NORMAL));
		CDnWorldServerStorageCashInventoryDlg* pWorldServerStorageCashInvenDlg = static_cast<CDnWorldServerStorageCashInventoryDlg*>(pPGStorageTabDlg->GetStorageDlg(StorageUIDef::eTAB_WORLDSERVER_CASH));
		if (pWorldServerStorageInvenDlg && pWorldServerStorageCashInvenDlg)
		{
			if (pWorldServerStorageInvenDlg->IsShow() || pWorldServerStorageCashInvenDlg->IsShow())
			{
				if (pItem->IsEternityItem() == false)
					return StorageUIDef::eERR_NOETERNITY;

				if (pItem->IsTradable(true) == false)
					return StorageUIDef::eERR_SERVERWARE;
			}
		}
	}

	return StorageUIDef::eERR_NONE;
}
#endif 

void CDnItemTask::PlayMoneySound()
{
	if( m_nMoneySound != -1 )
		CEtSoundEngine::GetInstance().PlaySound( "2D", m_nMoneySound, false );
}
//blondy end

void CDnItemTask::OnAddCoin(CDnPlayerActor* pWinner)
{
	if (pWinner->IsLocalActor())
		static_cast<CDnLocalPlayerActor*>(pWinner)->UpdateGetItem();
}

#ifdef PRE_MONITOR_SUPER_NOTE
void CDnItemTask::AddIllegalActivityMonitor(eIAMA_CheckType type, float fTerm)
{
	m_IAMAMgr.AddCheckMonitor(type, fTerm);
}
#endif

void CDnItemTask::ResetRequestWait()
{
	m_bRequestWait = false;
}

void CDnItemTask::PlayEnchantResultSound(int nFileIndex)
{
	const char *szFileName = CDnTableDB::GetInstance().GetFileName(nFileIndex);
	if(strlen( szFileName ) > 0)
	{
		int nSoundIndex = CEtSoundEngine::GetInstance().LoadSound( CEtResourceMng::GetInstance().GetFullName( szFileName ).c_str(), false, false );
		if( nSoundIndex != -1)
			CEtSoundEngine::GetInstance().PlaySound("2D", nSoundIndex, false);
	}
}

#if defined(PRE_ADD_ENCHANTSHIELD_CASHITEM)
#ifdef PRE_ADD_EQUIPED_ITEM_ENCHANT
void CDnItemTask::RequestEnchant( CDnItem *pItem, char cGuardType, bool bEquip )
{
	SendEnchant( pItem->GetSlotIndex(), pItem->GetSerialID(), cGuardType, bEquip );
	m_bRequestWait = true;
}
#else 
void CDnItemTask::RequestEnchant( CDnItem *pItem, char cGuardType )
{
	SendEnchant( pItem->GetSlotIndex(), pItem->GetSerialID(), cGuardType );
	m_bRequestWait = true;
}
#endif // PRE_ADD_EQUIPED_ITEM_ENCHANT
#else 
#ifdef PRE_ADD_EQUIPED_ITEM_ENCHANT
void CDnItemTask::RequestEnchant( CDnItem *pItem, bool bGuard, bool bEquip )
{
	SendEnchant( pItem->GetSlotIndex(), pItem->GetSerialID(), bGuard, bEquip );
	m_bRequestWait = true;
}
#else 
void CDnItemTask::RequestEnchant( CDnItem *pItem, bool bGuard )
{
	SendEnchant( pItem->GetSlotIndex(), pItem->GetSerialID(), bGuard );
	m_bRequestWait = true;
}
#endif // PRE_ADD_EQUIPED_ITEM_ENCHANT
#endif // #if defined(PRE_ADD_ENCHANTSHIELD_CASHITEM) 

#ifdef PRE_ADD_EXCHANGE_ENCHANT
void CDnItemTask::RequestExchangeEnchant(const CDnItem* pSourceItem, const CDnItem* pTargetItem)
{
	if (pSourceItem == NULL || pTargetItem == NULL)
	{
		_ASSERT(0);
		return;
	}

	SendItemExchangeEnchant(pSourceItem->GetSerialID(), pSourceItem->GetSlotIndex(), pTargetItem->GetSerialID(), pTargetItem->GetSlotIndex());
	m_bRequestWait = true;
}
#endif

#ifdef PRE_MOD_POTENTIAL_JEWEL_RENEWAL	
void CDnItemTask::OnRecvItemPotentialItemRollBack( SCPotentialItem *pPacket )
{
	if( ERROR_NONE == pPacket->nRet )
	{
		GetInterface().GetItemPotentialDlg()->OnRecvPotentialItemRollBack( pPacket->nRet, pPacket->cInvenIndex );

		CDnItem* pItem = m_CharInventory.GetItem(pPacket->cInvenIndex);
		if (pItem)
		{
			int nUIStringID = 9189; // Potential has been protected.

			std::wstring str;
			//str = FormatW(GetEtUIXML().GetUIString(CEtUIXML::idCategory1, nUIStringID), pItem->GetName());
			str = FormatW(GetEtUIXML().GetUIString(CEtUIXML::idCategory1, nUIStringID));
			GetInterface().AddChatMessage(CHATTYPE_SYSTEM, L"", str.c_str(), false);
		}
	}
	else
	{
		GetInterface().ServerMessageBox(pPacket->nRet);
	}
}
#endif

void CDnItemTask::OnRecvItemPotentialItem( SCPotentialItem *pPacket )
{
#ifdef PRE_MOD_POTENTIAL_JEWEL_RENEWAL
	if( GetInterface().IsRemovePotentialItemDlg() )
		GetInterface().GetItemPotentialCleanDlg()->OnRecvPotentialItem( pPacket->nRet, pPacket->cInvenIndex );  // Remove potential
	else
		GetInterface().GetItemPotentialDlg()->OnRecvPotentialItem( pPacket->nRet, pPacket->cInvenIndex );		// Grant potential

	if (ERROR_NONE != pPacket->nRet)
	{
		GetInterface().ServerMessageBox(pPacket->nRet);
	}
#else
	GetInterface().GetItemPotentialDlg()->OnRecvPotentialItem( pPacket->nRet, pPacket->cInvenIndex );
	if (ERROR_NONE == pPacket->nRet)
	{
		CDnItem* pItem = m_CharInventory.GetItem(pPacket->cInvenIndex);
		if (pItem)
		{
			int nUIStringID = 8054;
			if( pItem->GetPotentialIndex() <= 0 ) nUIStringID = 8055;

			std::wstring str;
			str = FormatW(GetEtUIXML().GetUIString(CEtUIXML::idCategory1, nUIStringID), pItem->GetName());
			GetInterface().AddChatMessage(CHATTYPE_SYSTEM, L"", str.c_str(), false);
		}
	}
	else
	{
		GetInterface().ServerMessageBox(pPacket->nRet);
	}
#endif
}

void CDnItemTask::OnRecvItemEnchantJewelItem( SCEnchantJewelItem *pPacket )
{
	GetInterface().GetEnchantJewelDlg()->OnRecvEnchantJewelItem( pPacket->nRet, pPacket->cInvenIndex );

	if (ERROR_NONE == pPacket->nRet)
	{
		CDnItem* pItem = m_CharInventory.GetItem(pPacket->cInvenIndex);
		if (pItem)
		{
			std::wstring str;
			str = FormatW(GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 8057), pItem->GetName(), pItem->GetEnchantLevel());
			GetInterface().AddChatMessage(CHATTYPE_SYSTEM, L"", str.c_str(), false);
		}
	}
	else
	{
		GetInterface().ServerMessageBox(pPacket->nRet);
	}
}

void CDnItemTask::OnRecvItemGuildMark( SCGuildMark *pPacket )
{
	if( pPacket->nRet == ERROR_NONE )
		GetInterface().GetGuildMarkCreateDlg()->OnRecvGuildMark( pPacket->wGuildMarkBG, pPacket->wGuildMarkBorder, pPacket->wGuildMark );
	else
		GetInterface().ServerMessageBox( pPacket->nRet );
}

#ifdef PRE_ADD_CHANGEJOB_CASHITEM
void CDnItemTask::OnRecvChangeJobCashItem( SCUseChangeJobCashItemRes* pPacket )
{
	if( m_bSendChangeJobReq )
	{
		m_bSendChangeJobReq = false;
		if( ERROR_NONE == pPacket->nRet )
		{
			vector<int> vlLegacyJobHistory;
			CDnLocalPlayerActor* pLocalPlayerActor = static_cast<CDnLocalPlayerActor*>(CDnActor::s_hLocalActor.GetPointer());
			pLocalPlayerActor->GetJobHistory( vlLegacyJobHistory );

			// Reset skills based on previous class first.

			for(int nSkillPage = DualSkill::Type::Primary; nSkillPage < DualSkill::Type::MAX; nSkillPage++ )
				GetSkillTask().ResetSkillsByJob( 1, (int)(vlLegacyJobHistory.size()), pPacket->nResultSkillPoint , nSkillPage );

			vector<int> vlNewJobHistory;
			vlNewJobHistory.push_back( vlLegacyJobHistory.front() );
			vlNewJobHistory.push_back( pPacket->nFirstJobID );

			if( 0 < pPacket->nSecondJobID && 3 == vlLegacyJobHistory.size() )
			{
				vlNewJobHistory.push_back( pPacket->nSecondJobID );
			}

			pLocalPlayerActor->SetJobHistory( vlNewJobHistory );

			//// Change job first and process skill reset. Skill points are also set automatically,
			//SCSkillReset SkillResetPacket;
			//SkillResetPacket.nResultSkillPoint = pPacket->nResultSkillPoint;
			//GetSkillTask().OnRecvSkillReset( &SkillResetPacket );

			// In ResetSkillsByJob(), the tree has already been updated based on the previous job info, so 
			// Refresh skill tree based on updated job history.
#if defined( PRE_ADD_PRESET_SKILLTREE )
			GetSkillTask().SendPresetSkillTreeList();
#endif	// #if defined( PRE_ADD_PRESET_SKILLTREE )

			GetSkillTask().RefreshSkillTree();

			// Since 1st and 2nd job advancements can also change, call OnChangeJob on CDnPlayerActor.
			pLocalPlayerActor->CDnPlayerActor::OnChangeJob( vlNewJobHistory.back() );

			// Apply if any equipped heraldry/crest affects a default skill.
			// (Things like Cleric kick action speed increase heraldry)
			DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TGLYPHSKILL );
			for( int iSlot = (int)CDnGlyph::Enchant1; iSlot < (int)CDnGlyph::GlyphSlotEnum_Amount; ++iSlot )
			{
				DnGlyphHandle hGlyph = pLocalPlayerActor->GetGlyph( (CDnGlyph::GlyphSlotEnum)iSlot );
				if( hGlyph )
				{
					int eType = pSox->GetFieldFromLablePtr( hGlyph->GetClassID(), "_GlyphType" )->GetInteger();

					// If the crest has a skill, determine whether it adds a skill or adds a skill effect.
					if( CDnGlyph::PassiveSkill == eType )
					{
						int iSkillID = pSox->GetFieldFromLablePtr( hGlyph->GetClassID(), "_SkillID" )->GetInteger();
						DnSkillHandle hSkill = GetSkillTask().FindSkill( iSkillID );
						if( hSkill )
						{
							hSkill->AddGlyphStateEffect( hGlyph->GetClassID() );
						}
					}
				}
			}

			// Update the separately copied local actor data held from PartyTask.
			if( CDnPartyTask::IsActive() && CDnPartyTask::GetInstance().GetPartyRole() == CDnPartyTask::SINGLE ) 
			{
				if( CDnActor::s_hLocalActor ) 
					CDnPartyTask::GetInstance().SetLocalData( CDnActor::s_hLocalActor );
			}

			// Class change item used successfully.
			GetInterface().MessageBox( GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 4848), MB_OK );
		}
		else
		{
			GetInterface().MessageBox( GetEtUIXML().GetUIString(CEtUIXML::idCategory2, pPacket->nRet), MB_OK );
		}
	}

}
#endif // #ifdef PRE_ADD_CHANGEJOB_CASHITEM


void CDnItemTask::OnRecvVehicleEquipListMessage(SCVehicleEquipList* pPacket)
{
	for(int i=0;i<=Vehicle::Slot::Saddle;i++) // Equipped parts are up to part 1.
	{
		CDnItem *Item_Temp = CreateItem(pPacket->VehicleEquip.Vehicle[i]);
		if(Item_Temp)
		{
			Item_Temp->SetSlotIndex(i);
			InsertVehicleItem(Item_Temp);
		}
	}

	if(CDnActor::s_hLocalActor)
	{
		CDnPlayerActor *pPlayer = dynamic_cast<CDnPlayerActor*>(CDnActor::s_hLocalActor.GetPointer());

		if(pPlayer && pPacket->VehicleEquip.Vehicle[Vehicle::Slot::Body].nItemID !=0)
		{
			pPlayer->SetVehicleInfo(pPacket->VehicleEquip);
			pPlayer->InsertRideVehicleThread(pPlayer->GetActorHandle());
		}
		else
			_ASSERT( "Error [VehicleEquipMessage] Have No Vehicle Info" );
	}

}// Receive own item setup info => need to put the icon into own mount slot.

void CDnItemTask::OnRecvVehicleInvenListMessage(SCVehicleInvenList* pPacket)
{
	for( int i=0; i<pPacket->cInvenCount; i++ ) 
		m_PetInventory.CreatePetItem( pPacket->VehicleInven[i] , m_nPetInvenCounter + i );

	m_nPetInvenCounter += pPacket->cInvenCount;
	if( m_nPetInvenCounter == pPacket->nTotalInventoryCount )
		m_nPetInvenCounter = 0;

	if( !m_nPetInvenCounter )
		m_PetInventory.SortCashInventory();

	// Receive own Inventory list.
}

void CDnItemTask::OnRecvChangeVehiclePartsMessage(SCChangeVehicleParts *pPacket)
{
	DnActorHandle hActor = CDnActor::FindActorFromUniqueID( pPacket->nSessionID ); // Corresponding session ID
	CDnPlayerActor *pPlayer = dynamic_cast<CDnPlayerActor*>(hActor.GetPointer());
	
	if(!pPlayer)
		return;

	if( pPlayer->IsVehicleMode() && pPlayer->GetMyVehicleActor() )
	{
		if( (pPacket->cSlotIndex > Vehicle::Slot::Body) && (pPacket->cSlotIndex < Vehicle::Slot::Max) ) // Equipment range
		{
			if(pPacket->Equip.nItemID > 0)
			{
				pPlayer->GetMyVehicleActor()->EquipItem(pPacket->Equip); // Equip the gear.
				pPlayer->GetVehicleInfo().Vehicle[pPacket->cSlotIndex] = pPacket->Equip; // The player object knows about the mount information.

				if(pPlayer->GetActorHandle() == CDnActor::s_hLocalActor && pPacket->cSlotIndex == Vehicle::Slot::Saddle) // If the saddle is equipped, put it into the slot.
				{
					CDnItem *pItem = CreateItem( pPacket->Equip );
					pItem->SetSlotIndex(pPacket->cSlotIndex);
				
					if(pItem)
						InsertVehicleItem(pItem); // Must also register the item in the mount slot.
				}
			}
			else if(pPacket->Equip.nItemID == 0) // Message to unequip
			{
				Vehicle::Parts::eVehicleParts ePartsType = Vehicle::Parts::Default;
				
				switch(pPacket->cSlotIndex)
				{
				case Vehicle::Slot::Saddle:
					ePartsType = Vehicle::Parts::Saddle;
					break;
				case Vehicle::Slot::Hair:
					ePartsType = Vehicle::Parts::Hair;
					break;
				default:
					ePartsType = Vehicle::Parts::Default;
					break;
				}

				pPlayer->GetMyVehicleActor()->UnEquipItem(ePartsType); // In this case, it is a message to unequip gear, i.e., set to default equipment.
				pPlayer->GetVehicleInfo().Vehicle[pPacket->cSlotIndex] = pPacket->Equip;
			}

			// Also refresh preview equipment when changing equipment.
			if(pPlayer->GetActorHandle() == CDnActor::s_hLocalActor)
			{
				CDnCharStatusDlg *pCharStatusDlg = (CDnCharStatusDlg*)GetInterface().GetMainMenuDialog( CDnMainMenuDlg::CHARSTATUS_DIALOG );
				if( pCharStatusDlg ) pCharStatusDlg->RefreshVehiclePreview();
			}
		}
		else if(pPacket->cSlotIndex == Vehicle::Slot::Body && pPlayer->IsVehicleMode()) // If a mount message arrives while already mounted, dismount first and then mount.
		{
			if(pPacket->Equip.nItemID != 0)
				pPlayer->UnRideVehicle(true); // Switch immediately after dismounting.
			else
				pPlayer->UnRideVehicle(); // Just lower it.


			TVehicleCompact Temp;
			memset(&Temp,0,sizeof(Temp));
			Temp.Vehicle[Vehicle::Slot::Body] = pPacket->Equip;
			pPlayer->SetVehicleInfo(Temp);
			CDnCharStatusDlg *pCharStatusDlg = (CDnCharStatusDlg*)GetInterface().GetMainMenuDialog( CDnMainMenuDlg::CHARSTATUS_DIALOG );
			if( pCharStatusDlg && pPlayer->GetActorHandle() == CDnActor::s_hLocalActor) pCharStatusDlg->ResetVehicleEquipSlot( Vehicle::Slot::Saddle );

			if(pPacket->Equip.nItemID != 0)
			{
				pPlayer->CallVehicle();
			}
		}
	}
	else if(!pPlayer->IsVehicleMode())
	{
		if(pPacket->cSlotIndex == Vehicle::Slot::Body) // If a mount body message arrives when not mounted, mount the player.
		{
			TVehicleCompact Temp;
			memset(&Temp,0,sizeof(Temp));
			Temp.Vehicle[Vehicle::Slot::Body] = pPacket->Equip;
			pPlayer->SetVehicleInfo(Temp);
			CDnCharStatusDlg *pCharStatusDlg = (CDnCharStatusDlg*)GetInterface().GetMainMenuDialog( CDnMainMenuDlg::CHARSTATUS_DIALOG );
			if( pCharStatusDlg && pPlayer->GetActorHandle() == CDnActor::s_hLocalActor) pCharStatusDlg->ResetVehicleEquipSlot( Vehicle::Slot::Saddle );

			if(pPacket->Equip.nItemID != 0)
			{
				pPlayer->CallVehicle();
			}
			else if(pPacket->Equip.nItemID == 0)
			{
				if(pPlayer->IsCallingVehicle())
				{
					pPlayer->CancelCallVehicle();
					pPlayer->SetActionQueue("Stand");
				}
			}
		}
		else
		{
			pPlayer->GetVehicleInfo().Vehicle[pPacket->cSlotIndex] = pPacket->Equip;

			if(pPlayer->GetActorHandle() == CDnActor::s_hLocalActor && pPacket->cSlotIndex == Vehicle::Slot::Saddle) // If the saddle is equipped, put it into the slot.
			{
				if(pPacket->Equip.nItemID > 0)
				{
					CDnItem *pItem = CreateItem( pPacket->Equip );
					pItem->SetSlotIndex(pPacket->cSlotIndex);
					
					if(pItem)
						InsertVehicleItem(pItem); // Must also register the item in the mount slot.
				}
			}
		}
	}
	// Here, update the equipment appearance of the corresponding SessionID User. // Including self
}

void CDnItemTask::OnRecvChangeVehicleColorMessagse(SCChangeVehicleColor *pPacket)
{
	DnActorHandle hActor = CDnActor::FindActorFromUniqueID( pPacket->nSessionID ); // Corresponding session ID
	CDnPlayerActor *pPlayer = dynamic_cast<CDnPlayerActor*>(hActor.GetPointer());

	if( !pPlayer )
		return;

	if( pPlayer->IsSummonPet() && pPacket->biSerial == pPlayer->GetPetInfo().Vehicle[Pet::Slot::Body].nSerial )	// If changing pet color, call pet color change function
	{
		OnRecvChangePetColorMessagse( pPacket );
		return;
	}

	pPlayer->GetVehicleInfo().dwPartsColor1 = pPacket->dwColor; // The player object knows about the mount information.

	if(pPlayer->IsVehicleMode() && pPlayer->GetMyVehicleActor())
	{
		if( pPacket->dwColor != -1 && pPacket->dwColor != 0) // If there is a color to change to, set the color.
		{
			pPlayer->GetMyVehicleActor()->ChangeHairColor(pPacket->dwColor);

			// Also refresh preview parts when dyeing.
			if(pPlayer->GetActorHandle() == CDnActor::s_hLocalActor)
			{
				CDnCharStatusDlg *pCharStatusDlg = (CDnCharStatusDlg*)GetInterface().GetMainMenuDialog( CDnMainMenuDlg::CHARSTATUS_DIALOG );
				if( pCharStatusDlg ) pCharStatusDlg->RefreshVehiclePreview();
			}
		}
		else
			_ASSERT("OnRecvChangeVehicleColor Error");
	}
	// Here, update the mount color of the corresponding SessionId User. // Including self
}

void CDnItemTask::OnRecvRefreshVehicleInvenMessage(SCRefreshVehicleInven *pPacket)
{
	OutputDebug( "CDnItemTask::OnRecvItemRefreshVehicleInven\n" );
	for( int i = 0; i < pPacket->nCount; ++i )
	{
		if( pPacket->ItemList[i].Vehicle[Vehicle::Slot::Body].wCount > 0 )
		{
			CDnItem *pItem = CreateItem( pPacket->ItemList[i].Vehicle[Vehicle::Slot::Body]);
			if (pItem == NULL)
			{
				_ASSERT(0 && "FAIL TO CREATE ITEM!");
				return;
			}
			// Check if going into an existing slot (stackable item quantity increase)
			bool bExistItem = false;
			CDnItem *pPreItem = NULL;
			if( pPreItem = m_PetInventory.FindItemFromSerialID( pItem->GetSerialID() ) )
				bExistItem = true;

			// Unlike normal inventory, cash refresh does not include slot index, so determine it manually.
			if( bExistItem )
				pItem->SetSlotIndex( pPreItem->GetSlotIndex() );
			else
				pItem->SetSlotIndex( m_PetInventory.FindFirstEmptyIndex() );

			// If going into an existing slot, check OverlapCount to determine whether it is an increment.
			bool bAddOverlapCount = false;
			if( bExistItem ) {
				if( pItem->GetOverlapCount() > pPreItem->GetOverlapCount() )
					bAddOverlapCount = true;
			}

			// Because the second argument is true, unlike Pickup, no item acquisition window or new mark is shown.
			// So handle only the 'new' indicator via a separate function call.
			// Since this packet arrives after repair as well as exchange/trade, check whether to show the new indicator too.
			if( m_PetInventory.InsertItem( pItem, true ) && pPacket->bNewSign )
			{
				// Indicates via the second argument whether it is a quantity increase or a newly arrived item.
				pItem->SetNewGain( true, bExistItem );

				// Play RootSound only when properly inserted into inventory and placed into a new slot.
				if( bExistItem == false || bAddOverlapCount == true )
					PlayItemSound( pPacket->ItemList[i].Vehicle->nItemID, ItemSound_Root );

				CDnInvenTabDlg* pInvenTabDlg = (CDnInvenTabDlg*)GetInterface().GetMainMenuDialog( CDnMainMenuDlg::INVENTORY_DIALOG );
				if( pInvenTabDlg )
					pInvenTabDlg->ShowTabNew( ST_INVENTORY_VEHICLE );
			}
		}
		else
		{
			CDnItem *pItem = m_PetInventory.FindItemFromSerialID( pPacket->ItemList[i].Vehicle->nSerial );
			if( pItem )
				m_PetInventory.RemoveItem( pItem->GetSlotIndex() );
		}
	}
	// Case where a new item is added and inserted. Ex> MakeItem message
}

void CDnItemTask::OnRecvChangePetPartsMessage( SCChangeVehicleParts* pPacket )
{
	DnActorHandle hActor = CDnActor::FindActorFromUniqueID( pPacket->nSessionID ); // Corresponding session ID
	CDnPlayerActor* pPlayer = dynamic_cast<CDnPlayerActor*>( hActor.GetPointer() );

	if( !pPlayer ) return;
	if( (pPacket->cSlotIndex <= Pet::Slot::Body) || (pPacket->cSlotIndex >= Pet::Slot::Max) ) // Equipment range
		return;

	if( pPlayer->IsSummonPet() )	// When a pet is summoned
	{
		if( pPacket->Equip.nItemID > 0 )	// Equip gear
		{
			pPlayer->GetPetInfo().Vehicle[pPacket->cSlotIndex] = pPacket->Equip;					// Update pet info
			pPlayer->GetMyPetActor()->EquipItem( pPacket->Equip );									// Equip gear
			pPlayer->GetMyPetActor()->GetPetInfo().Vehicle[pPacket->cSlotIndex] = pPacket->Equip;	// Update pet info

			if( pPlayer->GetActorHandle() == CDnActor::s_hLocalActor )
			{
				CDnItem *pItem = CreateItem( pPacket->Equip );
				
				if( pItem )
				{
					pItem->SetSlotIndex( pPacket->cSlotIndex );
					InsertPetItem( pItem );
				}
			}
		}
		else if( pPacket->Equip.nItemID == 0 ) // Unequip gear
		{
			Pet::Parts::ePetParts ePartsType = Pet::Parts::Default;

			switch( pPacket->cSlotIndex )
			{
				case Pet::Slot::Accessory1:
					ePartsType = Pet::Parts::PetAccessory1;
					break;
				case Pet::Slot::Accessory2:
					ePartsType = Pet::Parts::PetAccessory2;
					break;
				default:
					ePartsType = Pet::Parts::Default;
					break;
			}

			pPlayer->GetPetInfo().Vehicle[pPacket->cSlotIndex] = pPacket->Equip;					// Update pet info
			pPlayer->GetMyPetActor()->UnEquipItem( ePartsType );									// Unequip gear
			pPlayer->GetMyPetActor()->GetPetInfo().Vehicle[pPacket->cSlotIndex] = pPacket->Equip;	// Update pet info
		}

		pPlayer->RefreshState();

		// Also refresh preview equipment when changing equipment.
		if( pPlayer->GetActorHandle() == CDnActor::s_hLocalActor )
		{
			CDnCharStatusDlg *pCharStatusDlg = (CDnCharStatusDlg*)GetInterface().GetMainMenuDialog( CDnMainMenuDlg::CHARSTATUS_DIALOG );
			if( pCharStatusDlg ) pCharStatusDlg->RefreshPetPreview();
		}
	}
}

void CDnItemTask::OnRecvChangePetBodyMessage( SCChangePetBody* pPacket )
{
	DnActorHandle hActor = CDnActor::FindActorFromUniqueID( pPacket->nSessionID ); // Corresponding session ID
	CDnPlayerActor* pPlayer = dynamic_cast<CDnPlayerActor*>( hActor.GetPointer() );

	if( !pPlayer ) return;

	if( pPacket->PetInfo.Vehicle[Pet::Slot::Body].nItemID == 0 )
	{
		GetPetTask().OnRecvPetSummonedOff( pPacket->nSessionID, true );
		pPlayer->SetPetInfo( pPacket->PetInfo );
		return;
	}

	if( pPlayer->IsSummonPet() )	// When a pet is summoned
	{
		GetPetTask().OnRecvPetSummonedOff( pPacket->nSessionID );
		pPlayer->SetPetInfo( pPacket->PetInfo );

		CDnCharStatusDlg *pCharStatusDlg = (CDnCharStatusDlg*)GetInterface().GetMainMenuDialog( CDnMainMenuDlg::CHARSTATUS_DIALOG );
		if( pCharStatusDlg && pPlayer->GetActorHandle() == CDnActor::s_hLocalActor )
		{
			pCharStatusDlg->ResetPetEquipSlot( Pet::Slot::Accessory1 );
			pCharStatusDlg->ResetPetEquipSlot( Pet::Slot::Accessory2 );
		}

		GetPetTask().OnRecvPetSummon( pPacket->nSessionID, pPacket->PetInfo.Vehicle[Pet::Slot::Body].nItemID, pPacket->PetInfo.wszNickName, true );
	}
	else	// When no pet is summoned
	{
		pPlayer->SetPetInfo( pPacket->PetInfo );

		CDnCharStatusDlg *pCharStatusDlg = (CDnCharStatusDlg*)GetInterface().GetMainMenuDialog( CDnMainMenuDlg::CHARSTATUS_DIALOG );
		if( pCharStatusDlg && pPlayer->GetActorHandle() == CDnActor::s_hLocalActor )
		{
			pCharStatusDlg->ResetPetEquipSlot( Pet::Slot::Accessory1 );
			pCharStatusDlg->ResetPetEquipSlot( Pet::Slot::Accessory2 );
		}

		GetPetTask().OnRecvPetSummon( pPacket->nSessionID, pPacket->PetInfo.Vehicle[Pet::Slot::Body].nItemID, pPacket->PetInfo.wszNickName, true );
	}

	// Equip gear
	if( pPacket->PetInfo.Vehicle[Pet::Slot::Accessory1].nItemID > 0 )	
	{
		pPlayer->GetMyPetActor()->EquipItem( pPacket->PetInfo.Vehicle[Pet::Slot::Accessory1] );
		pPlayer->GetMyPetActor()->GetPetInfo().Vehicle[Pet::Slot::Accessory1] = pPacket->PetInfo.Vehicle[Pet::Slot::Accessory1];

		if( pPlayer->GetActorHandle() == CDnActor::s_hLocalActor )
		{
			CDnItem *pItem = CreateItem( pPacket->PetInfo.Vehicle[Pet::Slot::Accessory1] );

			if( pItem )
			{
				pItem->SetSlotIndex( Pet::Slot::Accessory1 );
				InsertPetItem( pItem );
			}
		}
	}

	if( pPacket->PetInfo.Vehicle[Pet::Slot::Accessory2].nItemID > 0 )
	{
		pPlayer->GetMyPetActor()->EquipItem( pPacket->PetInfo.Vehicle[Pet::Slot::Accessory2] );
		pPlayer->GetMyPetActor()->GetPetInfo().Vehicle[Pet::Slot::Accessory2] = pPacket->PetInfo.Vehicle[Pet::Slot::Accessory2];

		if( pPlayer->GetActorHandle() == CDnActor::s_hLocalActor )
		{
			CDnItem *pItem = CreateItem( pPacket->PetInfo.Vehicle[Pet::Slot::Accessory2] );

			if( pItem )
			{
				pItem->SetSlotIndex( Pet::Slot::Accessory2 );
				InsertPetItem( pItem );
			}
		}
	}
}

void CDnItemTask::OnRecvChangePetColorMessagse( SCChangeVehicleColor* pPacket )
{
	_ASSERT( pPacket );

	DnActorHandle hActor = CDnActor::FindActorFromUniqueID( pPacket->nSessionID );
	CDnPlayerActor *pPlayer = dynamic_cast<CDnPlayerActor*>( hActor.GetPointer() );

	if( !pPlayer )
		return;

	if( pPlayer->IsSummonPet() && pPlayer->GetMyPetActor() )
	{
		if( pPacket->cSlotIndex == ePetParts::PET_PARTS_BODY )
		{
			pPlayer->GetPetInfo().dwPartsColor1 = pPacket->dwColor;
			pPlayer->GetMyPetActor()->GetPetInfo().dwPartsColor1 = pPacket->dwColor;
			pPlayer->GetMyPetActor()->ChangeColor( ePetParts::PET_PARTS_BODY, pPacket->dwColor );
		}
		else if( pPacket->cSlotIndex == ePetParts::PET_PARTS_NOSE )
		{
			pPlayer->GetPetInfo().dwPartsColor2 = pPacket->dwColor;
			pPlayer->GetMyPetActor()->GetPetInfo().dwPartsColor2 = pPacket->dwColor;
			pPlayer->GetMyPetActor()->ChangeColor( ePetParts::PET_PARTS_NOSE, pPacket->dwColor );
		}

		// Also refresh preview parts when dyeing.
		if( pPlayer->GetActorHandle() == CDnActor::s_hLocalActor )
		{
			CDnCharStatusDlg *pCharStatusDlg = (CDnCharStatusDlg*)GetInterface().GetMainMenuDialog( CDnMainMenuDlg::CHARSTATUS_DIALOG );
			if( pCharStatusDlg ) 
				pCharStatusDlg->RefreshPetPreview();
		}

		DnEtcHandle hCommonEffect = CDnInCodeResource::GetInstance().CreatePlayerCommonEffect();	
		if( hCommonEffect ) 
		{
			hCommonEffect->SetPosition( *( pPlayer->GetMyPetActor()->GetPosition() ) );
			hCommonEffect->SetActionQueue( "MissionAchieve" );
		}
	}
}

void CDnItemTask::OnRecvPetEquipList( SCVehicleEquipList* pPacket )
{
	_ASSERT( pPacket );

	if( CDnActor::s_hLocalActor )
	{
		CDnPlayerActor* pPlayer = dynamic_cast<CDnPlayerActor*>(CDnActor::s_hLocalActor.GetPointer());

		if( pPlayer )
			GetPetTask().OnRecvPetEquipList( pPlayer, pPacket->VehicleEquip );
	}
}

void CDnItemTask::OnRecvAddPetExp( SCAddPetExp *pPacket )
{
	DnActorHandle hActor = CDnActor::FindActorFromUniqueID( pPacket->nSessionID );
	if( !hActor ) return;

	CDnPlayerActor *pPlayer = dynamic_cast<CDnPlayerActor *>(hActor.GetPointer());
	if( !pPlayer ) return;

	if( pPlayer->IsSummonPet() && pPlayer->GetSummonPet() )
	{
		int nPetLevelTableID = GetPetTask().GetPetLevelTableIndex( pPlayer->GetUniqueID() );
		pPlayer->GetPetInfo().nExp = pPacket->nExp;
		TVehicleCompact* pPetCompact = GetVehicleTask().GetVehicleInfoFromSerial( pPacket->biPetSerial );
		if( pPetCompact )
			pPetCompact->nExp = pPacket->nExp;

		bool bPetLevelUp = false;
		if( nPetLevelTableID != -1 && nPetLevelTableID < GetPetTask().GetPetLevelTableIndex( pPlayer->GetUniqueID() ) )
		{
			bPetLevelUp = true;
			pPlayer->SetPetEffectAction( "LevelUp_Pet" );
			pPlayer->RefreshState();
			GetPetTask().DoNotPetChat( PET_CHAT_NORMAL );
			GetPetTask().DoPetChat( PET_CHAT_LEVELUP );
		}

		if( CDnActor::s_hLocalActor->GetUniqueID() == pPacket->nSessionID )
		{
			if( bPetLevelUp )
				GetInterface().AddChatMessage( CHATTYPE_SYSTEM, L"", GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 9242 ), false );

			CDnCharStatusDlg* pCharStatusDlg = (CDnCharStatusDlg *)GetInterface().GetMainMenuDialog( CDnMainMenuDlg::CHARSTATUS_DIALOG );
			if( pCharStatusDlg )
			{
				CDnCharPetDlg* pCharPetDlg = pCharStatusDlg->GetPetDlg();
				if( pCharPetDlg ) pCharPetDlg->SetPetInfoDetail();
			}
		}
	}
}

#if defined(PRE_ADD_VIP_FARM)
void CDnItemTask::OnRecvEffectItemInfo( SCEffectItemInfo * pPacket )
{
	if( Farm::Common::VIP_FARM_ITEMID == pPacket->iItemID )
		m_tFarmVIPTime = pPacket->tExpireDate;
}
#endif	//#if defined(PRE_ADD_VIP_FARM)

void CDnItemTask::OnRecvGlyphExtendCount( SCGlyphExtendCount* pPacket )
{
	m_nCashGlyphCount = pPacket->cCount;

	// Update Crest cover
	//GetInterface().RefreshGlyphCover();
}

int CDnItemTask::GetEquipCashGlyphIndex( CDnItem * pItem )
{
	for( int itr = GLYPH_CASH1; itr <= GLYPH_CASH3; ++itr )
	{
		if( m_pGlyph[itr] && m_pGlyph[itr]->GetSerialID() == pItem->GetSerialID() )
			return itr - GLYPH_CASH1;
	}

	return -1;
}

void CDnItemTask::OnRecvGlyphExpireData( SCGlyphExpireData * pPacket )
{
	// Since all slot info is received at once, reset previous info.
	for( int itr = 0; itr < CASHGLYPHSLOTMAX; ++itr )
		m_CashGlyph[itr].Clear();

	for( int itr = 0; itr < pPacket->cCount; ++itr )
	{
		m_CashGlyph[ pPacket->TGlyphData[itr].cActiveGlyph -1 ].bOpen = true;
		m_CashGlyph[ pPacket->TGlyphData[itr].cActiveGlyph -1 ].tTime = pPacket->TGlyphData[itr].tGlyphExpireDate;
	}

	m_nCashGlyphCount = pPacket->cCount;

	// Update Crest cover
	GetInterface().RefreshGlyphCover();
	GetInterface().RefreshPlateDialog();

	//Update player state
	if( !CDnActor::s_hLocalActor )
		return;

	CDnLocalPlayerActor *pLocalActor = (CDnLocalPlayerActor*)CDnActor::s_hLocalActor.GetPointer();

	if( !pLocalActor )
		return;

	pLocalActor->RefreshState();
	int nCashIndex = CDnGlyph::GlyphSlotEnum::Cash1;
	for( int itr = 0; itr < CASHGLYPHSLOTMAX; ++itr )
	{
		if( m_CashGlyph[itr].bOpen && m_pGlyph[nCashIndex] )
		{
			DnGlyphHandle hGlyph = ((CDnGlyph*)m_pGlyph[nCashIndex])->GetMySmartPtr();
			pLocalActor->DetachGlyph( (CDnGlyph::GlyphSlotEnum)nCashIndex );
			pLocalActor->AttachGlyph( hGlyph, (CDnGlyph::GlyphSlotEnum)nCashIndex );
		}
		++nCashIndex;
	}
}

#if defined(PRE_ADD_TALISMAN_SYSTEM)
void CDnItemTask::OnRecvTalismanOpenSlot( SCTalismanOpenSlot *pPacket )
{
	if( pPacket->nRet == ERROR_NONE )
	{
		m_nTalismanOpenFlag = pPacket->nTalismanOpenFlag;
		GetInterface().RefreshTalismanOpenSlot();
	}
	else
	{
		GetInterface().ServerMessageBox( pPacket->nRet );
	}
}

void CDnItemTask::OnRecvUserChangeTalisman( SCChangeTalisman *pPacket )
{
	DnActorHandle hActor = CDnActor::FindActorFromUniqueID( pPacket->nSessionID );
	if( !hActor ) return;

	CDnPlayerActor *pActor = dynamic_cast<CDnPlayerActor *>(hActor.GetPointer());
	if( !pActor ) return;

	pActor->DetachTalisman( pPacket->cSlotIndex );

	if( pPacket->Item.nItemID > 0 )
	{
		CDnItem *pItem = CreateItem( pPacket->Item );
		if( pItem && dynamic_cast<CDnTalisman*>(pItem) && pItem->GetItemType() == ITEMTYPE_TALISMAN )
		{
			DnTalismanHandle hTalisman = ((CDnTalisman*)pItem)->GetMySmartPtr();
			pActor->AttachTalisman( hTalisman, pPacket->cSlotIndex, GetTalismanEfficiency( pPacket->cSlotIndex ), true );
		}
		else
		{
			SAFE_DELETE( pItem );
		}
	}

	pActor->RefreshState();

	CDnInspectPlayerDlg* pInspectPlayerDlg = GetInterface().GetInspectDialog();
	if( pInspectPlayerDlg && pInspectPlayerDlg->IsShow() && pInspectPlayerDlg->GetSessionID() == pPacket->nSessionID )
	{
		pInspectPlayerDlg->RefreshTalisman();
	}
}

void CDnItemTask::OnRecvTalismanExpireData( SCTalismanExpireData *pPacket )
{
	m_bCashTalismanOpen = pPacket->bOpenFlag;
	m_tCashTalismanExpireDate = pPacket->tExpireDate;

	GetInterface().RefreshTalismanCashSlot();
	GetInterface().RefreshTalismanDialog();

	if( !CDnActor::s_hLocalActor )
		return;

	CDnLocalPlayerActor *pLocalActor = (CDnLocalPlayerActor*)CDnActor::s_hLocalActor.GetPointer();
	if( !pLocalActor )
		return;

	pLocalActor->RefreshState();
}
#endif	// #if defined(PRE_ADD_TALISMAN_SYSTEM)

void CDnItemTask::OnRecvIncreaseLife( SCIncreaseLife* pPacket )
{
	//Output the message
	WCHAR wszString[256];

	if(pPacket->cType == IncreaseLifeType::Type::LifeStone)
	{
		swprintf_s( wszString, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 100072 ), pPacket->nIncreaseLife );
	}
	else if(pPacket->cType == IncreaseLifeType::Type::RebirthStone)
	{
		swprintf_s( wszString, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 100073 ), pPacket->nIncreaseLife );
	}	
	
	GetInterface().AddChatMessage( CHATTYPE_SYSTEM, L"", wszString, false );
}

CDnItem* CDnItemTask::FindRebirthItem(int nAllowMapType)
{
	for( DWORD itr = 0; itr < m_vSpecailRebirthItemIDs.size(); ++itr )
	{
		CDnItem * pItem = FindItem( m_vSpecailRebirthItemIDs[itr], ITEM_SLOT_TYPE::ST_INVENTORY );

		if( pItem && 0 < m_nSpecialRebirthItemCount )
			return pItem;
	}

	//Cannot resurrect if there is no usable item for this map
	return NULL;
}

#ifdef PRE_MOD_NESTREBIRTH
int CDnItemTask::GetRebirthItemCount_MineOnly()
{
	if (m_nSpecialRebirthItemCount <= 0)
		return 0;

	DWORD dwCount = 0;
	for (DWORD itr = 0; itr < m_vSpecailRebirthItemIDs.size(); ++itr)
	{
		const CDnItem* pItem = FindItem(m_vSpecailRebirthItemIDs[itr], ITEM_SLOT_TYPE::ST_INVENTORY);

		if (pItem && (DWORD)m_nSpecialRebirthItemCount >= dwCount)
		{
			int nCount = pItem->GetOverlapCount();
			CommonUtil::ClipNumber(nCount, 0, nCount);
			dwCount += (DWORD)nCount;
		}
	}

	return dwCount;
}
#endif

#if defined(PRE_ADD_REMOVE_PREFIX)
void CDnItemTask::OnRecvItemRemovePrefix( SCEnchantJewelItem *pPacket )
{
	GetInterface().GetRemovePrefixDlg()->OnRecvRemovePrefixItem( pPacket->nRet, pPacket->cInvenIndex );

	if (ERROR_NONE == pPacket->nRet)
	{
		CDnItem* pItem = m_CharInventory.GetItem(pPacket->cInvenIndex);
		if (pItem)
		{
			std::wstring str;
			str = FormatW(GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 3323), pItem->GetName());
			//str = FormatW(L"Suffix removed from item %s.", pItem->GetName());
			GetInterface().AddChatMessage(CHATTYPE_SYSTEM, L"", str.c_str(), false);
		}
	}
	else
	{
		GetInterface().ServerMessageBox(pPacket->nRet);
	}
}
#endif // PRE_ADD_REMOVE_PREFIX

void CDnItemTask::OnRecvItemExpandSkillPage(SCExpandSkillPageItem *pPacket)
{
	if( pPacket->nRet != ERROR_NONE )
		return;

	CDnSkillTreeDlg* pSkillTreeDlg = static_cast<CDnSkillTreeDlg*>( GetInterface().GetMainMenuDialog( CDnMainMenuDlg::SKILL_DIALOG ) );
	if( pSkillTreeDlg )
	{
		pSkillTreeDlg->EnableSkillPage(DualSkill::Type::Secondary); // When triple skill comes out, this will likely need to be added with packet updates.
		GetInterface().ShowCaptionDialog(CDnInterface::typeCaption2, GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 741 ) , textcolor::YELLOW);
		GetInterface().GetMainBarDialog()->BlinkMenuButton( CDnMainMenuDlg::SKILL_DIALOG );
	}
}

#ifndef PRE_ADD_NAMEDITEM_SYSTEM
void CDnItemTask::OnRecvItemUseSource( SCSourceItem* pPacket )
{
	// Overwrite if an origin item is already active.
	// Distinguish between own and others'.
	DnActorHandle hActor = CDnActor::FindActorFromUniqueID( pPacket->nSessionID );
	if( !hActor )
		return;
	
	if( CDnActor::s_hLocalActor && hActor == CDnActor::s_hLocalActor )
	{
		CDnLocalPlayerActor *pLocalActor = NULL;
		pLocalActor = (CDnLocalPlayerActor*)CDnActor::s_hLocalActor.GetPointer();
		if( pLocalActor && m_pSourceItem )
		{
			pLocalActor->DelEffectSkill( m_pSourceItem->GetClassID() );
			RemoveUsedSourceItemReferenceObjectAndIcon( m_pSourceItem->GetClassID() );
		}
	
		
		// Since it is a temp item object just for showing tooltips and filling quickslots, seed value doesn't matter. Set to 1.
		m_pSourceItem = CDnItem::CreateItem( pPacket->nItemID, 1 );
		_ASSERT( m_pSourceItem );

		if( m_pSourceItem )
		{
			// When equipped, replace with icon index from skill data. (Stored based on item icon index, so just swap it)
			m_pSourceItem->ChangeToSkillIconIndex();

			CDnCharStatusDlg *pCharStatusDlg = (CDnCharStatusDlg*)GetInterface().GetMainMenuDialog( CDnMainMenuDlg::CHARSTATUS_DIALOG );
			if( pCharStatusDlg ) 
				pCharStatusDlg->SetSourceItem( m_pSourceItem );

			// Add usage info for Source item.
			if( pLocalActor )
			{
				pLocalActor->AddEffectSkill( pPacket->nItemID, m_pSourceItem->GetSkillID(), pPacket->nRemainTime*1000, pPacket->bUsedInGameServer );	// Packet is in seconds, so convert to ms.
			}
		}
	}
	else if( hActor->IsPlayerActor() )
	{
		CDnPlayerActor* pPlayerActor = static_cast<CDnPlayerActor*>(hActor.GetPointer());
		pPlayerActor->DelEffectSkill( pPacket->nItemID );

		// #45646 Need a skill item used by items to fill HP/MP to max caused by state effects from origin items that increase base stats..
		CDnItem* pTempSourceItem = CDnItem::CreateItem( pPacket->nItemID, 1 );
		if( pTempSourceItem )
		{
			int iSkillID = pTempSourceItem->GetSkillID();
			pPlayerActor->AddEffectSkill( pPacket->nItemID, iSkillID, pPacket->nRemainTime * 1000 , pPacket->bUsedInGameServer );
			SAFE_DELETE( pTempSourceItem );

			// If currently inspecting, update it.
			CDnInspectPlayerDlg* pInspectPlayerDlg = GetInterface().GetInspectDialog();
			if( pInspectPlayerDlg->IsShow() )
			{
				pInspectPlayerDlg->RefreshItemSlots();
			}
		}
	}
}
#endif // #ifndef PRE_ADD_NAMEDITEM_SYSTEM

void CDnItemTask::RemoveUsedSourceItemReferenceObjectAndIcon( int iItemID )
{
	if( m_pSourceItem )
	{
		if( m_pSourceItem->GetClassID() == iItemID )
		{
			CDnCharStatusDlg *pCharStatusDlg = (CDnCharStatusDlg*)GetInterface().GetMainMenuDialog( CDnMainMenuDlg::CHARSTATUS_DIALOG );
			if( pCharStatusDlg ) 
				pCharStatusDlg->SetSourceItem( NULL );

#ifdef PRE_FIX_SOURCEITEM_TOOLTIP
			CDnTooltipDlg* pTooltip = GetInterface().GetTooltipDialog();
			if (pTooltip)
				pTooltip->HideTooltip();
#endif

			SAFE_DELETE( m_pSourceItem );
		}
	}
}

int CDnItemTask::GetUsedSourceItemDurationTime( int nItemID )
{
	int iResult = 0;

	if( CDnActor::s_hLocalActor )
	{
		CDnLocalPlayerActor *pLocalActor = (CDnLocalPlayerActor*)CDnActor::s_hLocalActor.GetPointer();
		int iNumUsedSourceItem = pLocalActor->GetNumEffectSkill();
		for( int i = 0; i < iNumUsedSourceItem; ++i )
		{
			const CDnLocalPlayerActor::S_EFFECT_SKILL* pUsedSourceItem = pLocalActor->GetEffectSkillFromIndex( i );
			if( pUsedSourceItem && nItemID == pUsedSourceItem->iItemID )
			{
				iResult = pUsedSourceItem->nEffectSkillLeftTime;
				break;
			}
		}
	}

	return iResult;
}


#if defined(PRE_FIX_44486)
bool CDnItemTask::CheckSkillLevelUpInfo(char cMoveType, int nSrcIndex, int nDestIndex)
{
	CDnItem *pDestItem = NULL;
	CDnItem *pSrcItem = NULL;

	//Also check if it is a Movetype that needs checking...
	bool bCheck = false;
	switch(cMoveType)
	{
	case MoveType_Equip:
		{
			pDestItem = m_pEquip[nDestIndex];
			pSrcItem = m_pEquip[nSrcIndex];
			bCheck = true;
		}
		break;
	case MoveType_EquipToInven:
		{
			pDestItem = m_CharInventory.GetItem(nDestIndex);
			pSrcItem = m_pEquip[nSrcIndex];
			bCheck = true;
		}
		break;
	case MoveType_InvenToEquip:
		{
			pDestItem = m_pEquip[nDestIndex];
			pSrcItem = m_CharInventory.GetItem(nSrcIndex);
			bCheck = true;
		}
		break;
	case MoveType_GlyphToInven:
		{
			pDestItem = m_CharInventory.GetItem(nDestIndex);
			pSrcItem = m_pGlyph[nSrcIndex];
			bCheck = true;
		}
		break;
	case MoveType_InvenToGlyph:
		{
			pDestItem = m_pGlyph[nDestIndex];
			pSrcItem = m_CharInventory.GetItem(nSrcIndex);
			bCheck = true;
		}
		break;
#if defined(PRE_ADD_TALISMAN_SYSTEM)
	case MoveType_Talisman:
		{
			pDestItem = m_pTalisman[nDestIndex];
			pSrcItem = m_pTalisman[nSrcIndex];
			bCheck = true;
		}
		break;
	case MoveType_TalismanToInven:
		{
			pDestItem = m_CharInventory.GetItem(nDestIndex);
			pSrcItem = m_pTalisman[nSrcIndex];
			bCheck = true;
		}
		break;
	case MoveType_InvenToTalisman:
		{
			pDestItem = m_pTalisman[nDestIndex];
			pSrcItem = m_CharInventory.GetItem(nSrcIndex);
			bCheck = true;
		}
		break;
#endif
	}
	if (bCheck && CheckSkillLevelUpInfo(pSrcItem, pDestItem) == false)
		return false;

	return true;
}

bool CDnItemTask::CheckSkillLevelUpInfo(CDnItem *pSrcItem, CDnItem* pDestItem)
{
	int nSkillID = 0;
	int nSkillLevel = 0;
	int nSkillUsingType = 0;

	CDnLocalPlayerActor *pLocalActor = NULL;
	if( CDnActor::s_hLocalActor )
		pLocalActor = (CDnLocalPlayerActor*)CDnActor::s_hLocalActor.GetPointer();

	if (pLocalActor == NULL)
		return false;

	//Check if the skill in SrcItem's skill level-up info is currently in use.
	if (pSrcItem && pSrcItem->HasLevelUpInfo(nSkillID, nSkillLevel, nSkillUsingType))
	{
		if (pLocalActor->CheckSkillLevelUp(nSkillID) == false)
			return false;
	}

	//Check whether the skill in DestItem's skill level-up info is currently in use.
	if (pDestItem && pDestItem->HasLevelUpInfo(nSkillID, nSkillLevel, nSkillUsingType))
	{
		if (pLocalActor->CheckSkillLevelUp(nSkillID) == false)
			return false;
	}

	return true;
}
#endif // PRE_FIX_44486

void CDnItemTask::RequestWarpVillageList( INT64 iUsedItemSerial )
{
	SendWarpVillageList( iUsedItemSerial );
}

void CDnItemTask::RequestUseWarpVillageCashItem( int iMapID, INT64 iUsedItemSerial )
{
	SendWarpVillage( iMapID, iUsedItemSerial );
}

void CDnItemTask::OnRecvWarpVillageList( SCWarpVillageList* pPacket )
{
	if( 0 < pPacket->btCount && pPacket->btCount <= _countof(pPacket->VillageList) )
	{
		GetInterface().ShowMapMoveCashItemDlg( true, pPacket->VillageList, pPacket->btCount );
	}
}

void CDnItemTask::OnRecvWarpVillageRes( SCWarpVillage *pPacket )
{
	if( ERROR_NONE != pPacket->nResult )
	{
		GetInterface().ServerMessageBox( pPacket->nResult );
		GetInterface().ResetMapMoveOKBtn();
	}
}

#ifdef PRE_ADD_EXCHANGE_POTENTIAL
void CDnItemTask::OnRecvExchangePotential( SCExchangePotential *pPacket )
{
	GetInterface().GetItemPotentialTransferDlg()->OnRecvExchangePotential( pPacket->nRetCode );

	if( ERROR_NONE == pPacket->nRetCode )
	{
		GetInterface().AddChatMessage( CHATTYPE_SYSTEM, L"", GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 8085 ), false );
	}
	else
	{
		GetInterface().ServerMessageBox( pPacket->nRetCode );
	}
}
#endif

#ifdef PRE_ADD_EXCHANGE_ENCHANT
void CDnItemTask::OnRecvExchangeEnchant(SCExchangeEnchant* pPakcet)
{
	int retCode = pPakcet->nRetCode;
	m_bRequestWait = false;

	CDnItemUpgradeExchangeDlg* pDlg = static_cast<CDnItemUpgradeExchangeDlg*>(GetInterface().GetMainMenuDialog(CDnMainMenuDlg::ITEM_UPGRADE_EXCHANGE_DIALOG));
	if (pDlg)
		pDlg->OnComplete();

	if (retCode == ERROR_NONE)
	{
		GetInterface().MessageBox( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 4941 ), MB_OK ); // UISTRING : Enhancement transfer succeeded.
	}
	else
	{
		GetInterface().ServerMessageBox(retCode);
	}
}
#endif

#if defined(PRE_ADD_PCBANG_RENTAL_ITEM)
void CDnItemTask::OnRecvPcCafeRentItem( SCPcCafeRentItem* pPacket)
{
	if( ERROR_NONE == pPacket->nRetCode )
	{
		// UISTRING : Rented a PC Cafe item. Please check your inventory.
		GetInterface().ShowCaptionDialog( CDnInterface::typeCaption4, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 6212 ), textcolor::YELLOW, 4.0f);

#ifdef PRE_FIX_63975
		CDnCommonTask *pCommonTask = static_cast<CDnCommonTask *>(CTaskManager::GetInstance().GetTask("CommonTask"));
		if( pCommonTask )
		{
			pCommonTask->NotifyRecvPcBangRentalItem();
		}
#endif
	}
	else if( ERROR_ITEM_INVENTORY_NOTENOUGH == pPacket->nRetCode)
	{
		std::wstring str = FormatW(GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 6213), pPacket->nNeedCount);	// UISTRING : Inventory is short by %d slot(s). Free up space and try again.
		GetInterface().ShowCaptionDialog( CDnInterface::typeCaption4, str.c_str(), textcolor::YELLOW, 4.0f);
	}
}
#endif //#if defined(PRE_ADD_PCBANG_RENTAL_ITEM)


#if defined(PRE_FIX_45899)
//If skill level-up info changes due to moving an item, and a packet to use that skill is sent to the server before the item move completion packet arrives
//When using a skill on the server side, level changes may prevent skill usage.
//So after requesting an item move, block using skills whose level info could change due to that item.
void CDnItemTask::DisalbeSkillByItemMove(CDnItem *pSrcItem, CDnItem* pDestItem)
{
	int nSkillID = 0;
	int nSkillLevel = 0;
	int nSkillUsingType = 0;

	CDnLocalPlayerActor *pLocalActor = NULL;
	if( CDnActor::s_hLocalActor )
		pLocalActor = (CDnLocalPlayerActor*)CDnActor::s_hLocalActor.GetPointer();

	if (pLocalActor == NULL)
		return;

	//Check if the skill in SrcItem's skill level-up info is currently in use.
	if (pSrcItem && pSrcItem->HasLevelUpInfo(nSkillID, nSkillLevel, nSkillUsingType))
	{
		pLocalActor->DisableSkillByItemMove(nSkillID);
	}

	//Check whether the skill in DestItem's skill level-up info is currently in use.
	if (pDestItem && pDestItem->HasLevelUpInfo(nSkillID, nSkillLevel, nSkillUsingType))
	{
		pLocalActor->DisableSkillByItemMove(nSkillID);
	}
}

void CDnItemTask::GetItemByMoveType(char cMoveType, int nSrcIndex, int nDestIndex, CDnItem** pDestItem, CDnItem** pSrcItem)
{
	*pDestItem = NULL;
	*pSrcItem = NULL;

	switch(cMoveType)
	{
	case MoveType_Equip:
		{
			*pDestItem = m_pEquip[nDestIndex];
			*pSrcItem = m_pEquip[nSrcIndex];
		}
		break;
	case MoveType_EquipToInven:
		{
			*pDestItem = m_CharInventory.GetItem(nDestIndex);
			*pSrcItem = m_pEquip[nSrcIndex];
		}
		break;
	case MoveType_InvenToEquip:
		{
			*pDestItem = m_pEquip[nDestIndex];
			*pSrcItem = m_CharInventory.GetItem(nSrcIndex);
		}
		break;
	case MoveType_GlyphToInven:
		{
			*pDestItem = m_CharInventory.GetItem(nDestIndex);
			*pSrcItem = m_pGlyph[nSrcIndex];
		}
		break;
	case MoveType_InvenToGlyph:
		{
			*pDestItem = m_pGlyph[nDestIndex];
			*pSrcItem = m_CharInventory.GetItem(nSrcIndex);
		}
		break;
#if defined(PRE_ADD_TALISMAN_SYSTEM)
	case MoveType_Talisman:
		{
			*pDestItem = m_pTalisman[nDestIndex];
			*pSrcItem = m_pTalisman[nSrcIndex];
		}
		break;
	case MoveType_TalismanToInven:
		{
			*pDestItem = m_CharInventory.GetItem(nDestIndex);
			*pSrcItem = m_pTalisman[nSrcIndex];
		}
		break;
	case MoveType_InvenToTalisman:
		{
			*pDestItem = m_pTalisman[nDestIndex];
			*pSrcItem = m_CharInventory.GetItem(nSrcIndex);
		}
		break;
#endif
	}
}

#endif // PRE_FIX_45899

#ifdef PRE_MOD_NESTREBIRTH
void CDnItemTask::SetSpecialRebirthItemID( int nSpecailRebirthItemID )
{
	m_vSpecailRebirthItemIDs.push_back( nSpecailRebirthItemID );
	SetUpdateRebirthCoinOrItem(true);
}

void CDnItemTask::SetSpecialRebirthItemCount( int nSpecialRebirthItemCount )
{
  	m_nSpecialRebirthItemCount = nSpecialRebirthItemCount;
	SetUpdateRebirthCoinOrItem(true);
}
#endif


#ifdef PRE_ADD_AUTOUNPACK

// Send Packet.
void CDnItemTask::RequestAutoUnpackItem( INT64 biInvenSerial, bool bShow )
{
	SendRequestAutoUnpackItem( biInvenSerial, bShow );
}

// Receive Packet.
void CDnItemTask::OnRecvAutoUnpack( SCAutoUnpack * pData )
{
	// OK.
	if( pData->nRetCode == ERROR_NONE )
	{
		CDnItem * pItem = m_CashInventory.FindItemFromSerialID( pData->serial );
		if( pItem )
		{
			//GetInterface().OpenAutoUnPack( true, pItem->GetTypeParam(1) );
			GetInterface().UnPacking();	
		}
	}

	// Cannot process.
	else
	{	
		if( pData->nRetCode == ERROR_ITEM_INSUFFICIENCY_ITEM ||
			pData->nRetCode == ERROR_ITEM_NOTFOUND )
		{					
			GetInterface().MessageBox( 4872 ); // "Usage canceled due to insufficient %s."
			GetInterface().OpenAutoUnPack( 0, false );
		}
	}
}

#endif

#if defined( PRE_ADD_NAMEDITEM_SYSTEM )

void CDnItemTask::OnRecvEffectSkillItem( SCEffectSkillItem *pPacket )
{
	if( pPacket )
	{
		bool bSuccess = false;

		SCEffectSkillItem pData;
		memset( &pData, 0, sizeof(pData) );

		pData.bUsedInGameServer = pPacket->bUsedInGameServer;
		pData.nSessionID = pPacket->nSessionID;
		pData.nCount = pPacket->nCount;

		if( pPacket->nCount <= 0 || EFFECTSKILLMAX <= pPacket->nCount )
			return;
		
		for( int i=0; i< pPacket->nCount ; i++ )
		{
			memcpy( &pData.EffectSkill[i] , &pPacket->EffectSkill[i] , sizeof(TEffectSkillData) );
		}
	
		DnActorHandle hActor = CDnActor::FindActorFromUniqueID( pData.nSessionID );
		bSuccess = ApplyEffectSkillData( &pData , hActor ); // Or failed to find actor 

		if( !bSuccess )
		{
			m_vecBackUp_EffectSkillInfo.push_back(pData);
		}
	}
}

void CDnItemTask::OnRecvRemoveEffectSkillItem( SCDelEffectSkillItem *pPacket )
{
	if( pPacket )
	{
		DnActorHandle hActor = CDnActor::FindActorFromUniqueID( pPacket->nSessionID );
		RemoveEffectItem( hActor , pPacket->nItemID );
	}
}

bool CDnItemTask::ApplyEffectSkillData( SCEffectSkillItem *pData , DnActorHandle hActor )
{
	DNTableFileFormat *pItemSox = GetDNTable( CDnTableDB::TITEM );
	if( !pItemSox || !pData )
		return false;		

	bool bSuccess = false;

	for(int i = 0; i<pData->nCount, i<EFFECTSKILLMAX; i++)
	{
		if( pData->EffectSkill[i].nItemID > 0 && !pItemSox->IsExistItem( pData->EffectSkill[i].nItemID ) ) continue;
		eItemTypeEnum type = (eItemTypeEnum)pItemSox->GetFieldFromLablePtr( pData->EffectSkill[i].nItemID, "_Type" )->GetInteger();

		switch( (eItemTypeEnum)type )
		{
		case ITEMTYPE_SOURCE:
			{
				if( hActor )
				{
					AddSourceItem( hActor, pData->EffectSkill[i], pData->bUsedInGameServer );
					bSuccess = true;
				}
			}
			break;
		case ITEMTYPE_GLOBAL_PARTY_BUFF:
		case ITEMTYPE_BESTFRIENDBUFFITEM:		
		case ITEMTYPE_DIRECT_PARTYBUFF:
		case ITEMTYPE_TRANSFORMPOTION:
			{
				if( CTaskManager::GetInstance().GetTask("VillageTask") ) 
				{
					if( CDnActor::s_hLocalActor && hActor && CDnActor::s_hLocalActor == hActor )
					{
						AddEffectItem( hActor, pData->EffectSkill[i], pData->bUsedInGameServer );
						bSuccess = true;
					}
					else
					{
						AddVillagePartyEffectItem( pData->nSessionID , pData->EffectSkill[i] );
						bSuccess = true;
					}
				}
				else
				{
					if( hActor )
					{
						AddEffectItem( hActor, pData->EffectSkill[i], pData->bUsedInGameServer );
						bSuccess = true;
					}
				}
			}
			break;
		}		
	}

	if( !CTaskManager::GetInstance().GetTask("VillageTask") ) 
	{
		if( hActor && hActor->IsPlayerActor() )
		{
			CDnPlayerActor *pPlayer = static_cast<CDnPlayerActor*>(hActor.GetPointer());
			pPlayer->SyncEffectSkill();
		}
	}

	return bSuccess;
}

void CDnItemTask::ApplyBackUpEffectSkillData( DnActorHandle hActor )
{	
	std::vector<SCEffectSkillItem>::iterator iteratorPos = m_vecBackUp_EffectSkillInfo.begin();

	for( iteratorPos = m_vecBackUp_EffectSkillInfo.begin(); iteratorPos != m_vecBackUp_EffectSkillInfo.end(); )
	{
		if( iteratorPos->nSessionID == hActor->GetUniqueID() )
		{
			ApplyEffectSkillData( &(*iteratorPos) , hActor );
			iteratorPos = m_vecBackUp_EffectSkillInfo.erase(iteratorPos);
		}
		else
		{
			iteratorPos++;
		}
	}
}

void CDnItemTask::OnRecvShowEffect( SCShowEffect * pPacket )
{
	DnActorHandle hActor = CDnActor::FindActorFromUniqueID( pPacket->nSessionID );
	if( hActor && hActor->IsPlayerActor() )
	{
		CDnPlayerActor *pPlayer = static_cast<CDnPlayerActor*>(hActor.GetPointer());
		DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TNAMEDITEM );
		if( pSox )
		{
			for( int i=0; i<pSox->GetItemCount(); ++i )
			{
				int nIndex = pSox->GetItemID( i );
				int nItemIndex = pSox->GetFieldFromLablePtr( nIndex, "_NamedItem_ID" )->GetInteger();
				if( nItemIndex == pPacket->nItemID )
				{
					std::string strEffectActionName;
					if(pPacket->bUseSession)
						strEffectActionName = pSox->GetFieldFromLablePtr( nIndex , "_Skill_Effect" )->GetString();
					else
						strEffectActionName = pSox->GetFieldFromLablePtr( nIndex , "_Target_Effect" )->GetString();

					if( strEffectActionName.empty() == false )
					{
						DnEtcHandle hHandle = CDnInCodeResource::GetInstance().CreatePlayerCommonEffect();
						if( hHandle )
						{
							hHandle->SetActionQueue( strEffectActionName.c_str() );
							EtMatrix AxisMat = pPlayer->GetBoneMatrix("Bip01");
							if(pPlayer->IsVehicleMode() && pPlayer->GetMyVehicleActor() )
								hHandle->SetPosition( *pPlayer->GetMyVehicleActor()->GetPosition() );
							hHandle->SetPosition( (EtVector3)&AxisMat._41 );
						}
					}
					break;
				}
			}
		}
	}

	return;
}

void CDnItemTask::OnRecvRefreshNamedItem( SCNamedItemID *pPacket )
{
	DnActorHandle hActor = CDnActor::FindActorFromUniqueID( pPacket->nSessionID );
	if( hActor && hActor->IsPlayerActor() )
	{
		CDnPlayerActor *pPlayer = static_cast<CDnPlayerActor*>(hActor.GetPointer());

		if( pPacket->nItemID > 0 )
		{
			DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TNAMEDITEM );
			if( pSox )
			{
				for( int i=0; i<pSox->GetItemCount(); ++i )
				{
					int nIndex = pSox->GetItemID( i );
					int nItemIndex = pSox->GetFieldFromLablePtr( nIndex, "_NamedItem_ID" )->GetInteger();
					if( nItemIndex == pPacket->nItemID )
					{
						std::string strLoopEffectName = pSox->GetFieldFromLablePtr( nIndex , "_Loop_Effect" )->GetString();
						pPlayer->ToggleLinkedPlayerEffect( true , strLoopEffectName.c_str() );
						break;
					}
				}
			}
		}
		else
			pPlayer->ToggleLinkedPlayerEffect( false );
	}
}

void CDnItemTask::AddSourceItem( DnActorHandle hActor, TEffectSkillData EffectSkill, bool bUsedInGameServer )
{
	// Overwrite if an origin item is already active.
	// Distinguish between own and others'.	
	if( CDnActor::s_hLocalActor && hActor == CDnActor::s_hLocalActor )
	{
		CDnLocalPlayerActor *pLocalActor = NULL;
		pLocalActor = (CDnLocalPlayerActor*)CDnActor::s_hLocalActor.GetPointer();
		if( pLocalActor && m_pSourceItem )
		{
			pLocalActor->DelEffectSkill( m_pSourceItem->GetClassID() );
			RemoveUsedSourceItemReferenceObjectAndIcon( m_pSourceItem->GetClassID() );
		}

		// Since it is a temp item object just for showing tooltips and filling quickslots, seed value doesn't matter. Set to 1.
		m_pSourceItem = CDnItem::CreateItem( EffectSkill.nItemID, 1 );
		_ASSERT( m_pSourceItem );

		if( m_pSourceItem )
		{
			// When equipped, replace with icon index from skill data. (Stored based on item icon index, so just swap it)
			m_pSourceItem->ChangeToSkillIconIndex();

			CDnCharStatusDlg *pCharStatusDlg = (CDnCharStatusDlg*)GetInterface().GetMainMenuDialog( CDnMainMenuDlg::CHARSTATUS_DIALOG );
			if( pCharStatusDlg ) 
				pCharStatusDlg->SetSourceItem( m_pSourceItem );

			// Add usage info for Source item.
			if( pLocalActor )
			{
				pLocalActor->AddEffectSkill( EffectSkill.nItemID, m_pSourceItem->GetSkillID(), EffectSkill.nRemainTime*1000, bUsedInGameServer );	// Packet is in seconds, so convert to ms.
			}
		}
	}
	else if( hActor->IsPlayerActor() )
	{
		CDnPlayerActor* pPlayerActor = static_cast<CDnPlayerActor*>(hActor.GetPointer());
		pPlayerActor->DelEffectSkill( EffectSkill.nItemID );

		// #45646 Need a skill item used by items to fill HP/MP to max caused by state effects from origin items that increase base stats..
		CDnItem* pTempSourceItem = CDnItem::CreateItem( EffectSkill.nItemID, 1 );
		if( pTempSourceItem )
		{
			int iSkillID = pTempSourceItem->GetSkillID();
			pPlayerActor->AddEffectSkill( EffectSkill.nItemID, iSkillID, EffectSkill.nRemainTime*1000 , bUsedInGameServer , EffectSkill.bEternity );
			SAFE_DELETE( pTempSourceItem );

			CDnInspectPlayerDlg* pInspectPlayerDlg = GetInterface().GetInspectDialog();
			if( pInspectPlayerDlg->IsShow() )
				pInspectPlayerDlg->RefreshItemSlots();
		}
	}
}

void CDnItemTask::AddEffectItem( DnActorHandle hActor, TEffectSkillData EffectSkill, bool bUsedInGameServer )
{
	if( hActor && hActor->IsPlayerActor() )
	{
		CDnPlayerActor* pPlayerActor = static_cast<CDnPlayerActor*>(hActor.GetPointer());
		pPlayerActor->DelEffectSkill(EffectSkill.nItemID);
		pPlayerActor->AddEffectSkill( EffectSkill.nItemID, EffectSkill.nSkillID , EffectSkill.nRemainTime*1000, bUsedInGameServer , EffectSkill.bEternity );
	}
}

void CDnItemTask::AddVillagePartyEffectItem( UINT nUniqueID , TEffectSkillData EffectSkill )
{
	if( CDnPartyTask::IsActive() )  // Since actors are dynamic in village, they must be managed in PartyTask.
	{
		CDnPartyTask::GetInstance().RemoveEffectSkillInfo(nUniqueID , EffectSkill.nSkillID );

		CDnPartyTask::VillagePartyEffectSkillInfo eSkillInfo;
		eSkillInfo.nSkillID = EffectSkill.nSkillID;
		eSkillInfo.nItemID = EffectSkill.nItemID;
		eSkillInfo.fRemainTime = (float)EffectSkill.nRemainTime;
		eSkillInfo.bEternity = EffectSkill.bEternity;

		CDnPartyTask::GetInstance().AddEffectSkillInfo( nUniqueID , &eSkillInfo );
	}
}

void CDnItemTask::RemoveEffectItem( DnActorHandle hActor, int nSkillItemIndex )
{
	if( hActor && hActor->IsPlayerActor() )
	{
		CDnPlayerActor* pPlayerActor = static_cast<CDnPlayerActor*>(hActor.GetPointer());
		pPlayerActor->DelEffectSkill( nSkillItemIndex );
	}
}

#endif

#if defined(PRE_ADD_EXPUP_ITEM)
void CDnItemTask::OnRecvDailyItemError( SCDailyLimitItemError *pPacket)
{
	int LimitCount = static_cast<int>(CGlobalWeightTable::GetInstance().GetValue((CGlobalWeightTable::WeightTableIndex)pPacket->nWeightTableIndex));
	WCHAR wzStrTmp[1024]={0,};
	swprintf_s(wzStrTmp , _countof(wzStrTmp) , GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 114044) , LimitCount ); //This item can only be used %d times per day.
	GetInterface().MessageBox( wzStrTmp, MB_OK );
	return;
}
#endif

#ifdef PRE_ADD_CHAOSCUBE

void CDnItemTask::RequestCancelChaosCube()
{
	SendCancelChaosCube();

	if( GetInterface().IsOpenChaosCubeDlg() )
		GetInterface().RecvMixStart( true );

#ifdef PRE_ADD_CHOICECUBE
	else if( GetInterface().IsOpenChoiceCubeDlg() )
		GetInterface().RecvChoiceMixStart( true );
#endif

}

void CDnItemTask::RequestChaosCube( struct CSChaosCubeRequest & packet )
{
	// Send.
	SendRequestChaosCube( packet );
}


void CDnItemTask::CompleteChaosCube( struct CSChaosCubeComplete & packet )
{
	SendCompleteChaosCube( packet );
}


void CDnItemTask::OnRecvChaosCubeRequest( SCChaosCubeRequest * pData )
{
	if( pData->nRet == ERROR_NONE )
	{
		// ChaosCube.
		if( GetInterface().IsOpenChaosCubeDlg() )
			GetInterface().OpenChaosCubeProgressDlg( true, pData );
	
#ifdef PRE_ADD_CHOICECUBE
		// ChoiceCube.
		else if( GetInterface().IsOpenChoiceCubeDlg() )
			GetInterface().OpenChoiceCubeProgressDlg( true, pData );
#endif

	}

	else
	{
		GetInterface().ServerMessageBox( pData->nRet );
		GetInterface().RecvMixStart( true );
	}

	
}


void CDnItemTask::OnRecvChaosCubeComplete( SCChaosCubeComplete * pData )
{
	if( pData->nRet == ERROR_NONE )	
		GetInterface().OpenResultDlg( true, pData->nResultItemID, pData->nItemCount );
	
	else
		GetInterface().ServerMessageBox( pData->nRet );
}

int CDnItemTask::GetChaosGauge( int nChaosGaugeType ) const
{
	std::map<int, int>::const_iterator it = m_mapChaosGauge.find( nChaosGaugeType );
	if( it != m_mapChaosGauge.end() )
		return it->second;
	return 0;
}

void CDnItemTask::SetChaosGauge( int nChaosGaugeType, int nGauge )
{
	m_mapChaosGauge[nChaosGaugeType] = nGauge;
}

void CDnItemTask::RequestChaosGaugeCubeAddStuff( CSChaosGaugeCubeAddStuff & packet )
{
	SendChaosGaugeCubeAddStuff( packet );
}

void CDnItemTask::RequestChaosGaugeCubeComplete( CSChaosGaugeCubeComplete & packet )
{
	SendChaosGaugeCubeComplete( packet );
}

void CDnItemTask::OnRecvChaosGaugeCubeAddStuff( SCChaosGaugeCubeAddStuff * pData )
{
	if( !pData )
		return;

	if( pData->nRet == ERROR_NONE )
	{
		SetChaosGauge( pData->nChaosGaugeType, pData->nCurrentGauge );
		if( GetInterface().GetChaosGaugeCubeDlg() )
			GetInterface().GetChaosGaugeCubeDlg()->OnRecvAddStuff( pData->nChaosGaugeType, pData->nCurrentGauge );
	}
	else
	{
		GetInterface().ServerMessageBox( pData->nRet );
	}
}

void CDnItemTask::OnRecvChaosGaugeCubeComplete( SCChaosGaugeCubeComplete * pData )
{
	if( !pData )
		return;

	if( pData->nRet == ERROR_NONE )
	{
		SetChaosGauge( pData->nChaosGaugeType, 0 );
		if( GetInterface().GetChaosGaugeCubeDlg() )
			GetInterface().GetChaosGaugeCubeDlg()->OnRecvComplete( pData->nChaosGaugeType, pData->nResultItemID, pData->nItemCount );
	}
	else
	{
		GetInterface().ServerMessageBox( pData->nRet );
	}
}

void CDnItemTask::OnRecvChaosGaugeCubeList( SCChaosGaugeCubeList * pData )
{
	if( !pData )
		return;

	m_mapChaosGauge.clear();
	int nCount = pData->nCount;
	if( nCount > 20 )
		nCount = 20;
	for( int i = 0; i < nCount; ++i )
	{
		m_mapChaosGauge[pData->GaugeList[i].nChaosGaugeType] = pData->GaugeList[i].nItemGauge;
	}
}

#endif


#ifdef PRE_ADD_BESTFRIEND

// Best friend packet response.
void CDnItemTask::OnRecvBestfriend( int nSubCmd, char *pData, int nSize ) 
{
	switch( nSubCmd ) 
	{
	case eBestFriend::eSCBestFriend::SC_SEARCH:	OnRecvSearchBF( pData );	break;	
	case eBestFriend::eSCBestFriend::SC_REGISTREQ: OnRecvRegistREQ( pData ); break;
	case eBestFriend::eSCBestFriend::SC_REGISTACK: OnRecvRegistACK( pData ); break;
	case eBestFriend::eSCBestFriend::SC_COMPLETE: OnRecvComplerBF( pData ); break;
	case eBestFriend::eSCBestFriend::SC_GETINFO: OnRecvInfoBF( pData ); break;
	case eBestFriend::eSCBestFriend::SC_EDITMEMO: OnRecvMemo( pData ); break;	
	case eBestFriend::eSCBestFriend::SC_CLOSEBF: OnRecvCloseBF(); break;
	case eBestFriend::eSCBestFriend::SC_CANCELBF: OnRecvCancelBF( pData ); break;
	}
}

// Best friend confirm request.
void CDnItemTask::RequestSearchBF( std::wstring & strID )
{
	BestFriend::CSSearch packet;
	_wcscpy( packet.wszName, _countof(packet.wszName), strID.c_str(), (int)wcslen(strID.c_str()) );
	SendRequestSearchBF( packet );
}

// Best friend confirm result.
void CDnItemTask::OnRecvSearchBF( char * pData )
{
	BestFriend::SCSearch * packet = (BestFriend::SCSearch *)( pData );

	if( packet->iRet == ERROR_NONE )
	{
		GetInterface().SetSearchBF( packet ); // Best friend confirm info.
		GetInterface().OpenIdentifyBF( true );
	}

	else
	{
		GetInterface().ServerMessageBox( packet->iRet );
	}
}

// Best friend registration request.
void CDnItemTask::RequestRegistBF( std::wstring &strID )
{
	BestFriend::CSRegist packet;
	packet.biInvenSerial = GetInterface().GetBFserial();	
	_wcscpy( packet.wszBestFriendName, _countof(packet.wszBestFriendName), strID.c_str(), (int)wcslen(strID.c_str()));

	SendRegistBF( packet );
}

// Confirm best friend registration request.
void CDnItemTask::OnRecvRegistREQ( char * pData )
{
	BestFriend::SCRegistReq * packet = (BestFriend::SCRegistReq *)( pData );

	if( packet->iRet == ERROR_NONE )
	{
		bool bOpen = true;
		// Ignore if in Cash Shop or blind state.
		if( GetInterface().IsShowCashShopDlg() || 
			GetInterface().IsOpenBlind() )
		{
			bOpen = false;	
		}	

		// Best friend accept ProgressDlg.
		GetInterface().SetBFProgressData( packet, bOpen );
		GetInterface().OpenBFProgressDlg( bOpen ); 
	}

	else
	{
		GetInterface().ServerMessageBox( packet->iRet );
	}
}


// Best friend accept request.
void CDnItemTask::OnRecvRegistACK( char * pData )
{	
	BestFriend::SCRegistAck * packet = (BestFriend::SCRegistAck*)( pData );
	
	if( packet->iRet == ERROR_NONE )
	{			
	}

	else
	{
		GetInterface().ServerMessageBox( packet->iRet );
	}

}

void CDnItemTask::OnRecvComplerBF( char * pData )
{
	BestFriend::SCComplete * packet = (BestFriend::SCComplete*)( pData );

	if( packet->iRet == ERROR_NONE )
	{			
	}

	else
	{
		GetInterface().ServerMessageBox( packet->iRet );
	}
}


// Accept or cancel best friend request.
void CDnItemTask::RequestAcceptCancelBF( bool AcceptorCancel, UINT nFromAccountDBID, INT64 biFromCharacterDBID, WCHAR * wszFromName )
{
	BestFriend::CSAccept packet;
	packet.bAccept = AcceptorCancel;
	packet.nFromAccountDBID = nFromAccountDBID;
	packet.biFromCharacterDBID = biFromCharacterDBID;
	_wcscpy( packet.wszFromName, _countof(packet.wszFromName), wszFromName, (int)wcslen(wszFromName) );

	SendAcceptBF( packet );
}


void CDnItemTask::InitBestfriendInfo()
{	
	bool bFlag = false;
	CTask * pTask = CTaskManager::GetInstance().GetTask( "VillageTask" );
	if( pTask )
		bFlag = ((CDnVillageTask *)pTask)->IsCreateLocalActor();
	else
	{
		pTask = CTaskManager::GetInstance().GetTask( "GameTask" );
		if( pTask )
			bFlag = ((CDnGameTask *)pTask)->IsCreateLocalActor();
	}


	if( m_pBestfriendInfo != NULL )
	{
		GetInterface().SetBFData( m_pBestfriendInfo->Info );

		if( bFlag && CDnActor::s_hLocalActor.GetPointer() )
		{
			CDnPlayerActor *pPlayer = dynamic_cast<CDnPlayerActor*>( CDnActor::s_hLocalActor.GetPointer() );
			if( pPlayer )
			{				
				pPlayer->SetBFserial( m_pBestfriendInfo->Info.biItemSerial );
				pPlayer->SetBestfriendName( m_pBestfriendInfo->Info.wszName );

				// Best friend title.
				CDnAppellationTask::GetInstance().SetBFappellation();


				// Register my ID to best friend.				
				DnActorHandle hBFActor = CDnActor::FindActorFromUniqueID( m_pBestfriendInfo->Info.nSessionID );
				if( hBFActor )
				{
					CDnPlayerActor *pBFplayer = dynamic_cast<CDnPlayerActor*>( hBFActor.GetPointer() );
					if( pBFplayer )
						pBFplayer->SetBestfriendName( pPlayer->GetName() );
				}
			}
		}
		//GetInterface().SetBFData( m_pBestfriendInfo->Info );

		// Output best friend title system message.
		if( CDnAppellationTask::GetInstance().IsAddChatBestFriend() )
		{
			CDnAppellationTask::GetInstance().ResetAddChatBestFriend();

			DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TAPPELLATION );
			int nItemID = pSox->GetItemID( CDnAppellationTask::GetInstance().GetBFAppellationIndex() );
			if( nItemID == -1 )
				return;
		
			DNTableCell * pCell = pSox->GetFieldFromLablePtr( nItemID, "_Type" );			
			if( pCell && pCell->GetInteger() == AppellationType::Type::eCode::BestFriend )	
			{
				wchar_t strName[256] = {0,};		
				const TCHAR * pBFname = GetInterface().GetNameBF();
				if( pBFname )
				{
					wchar_t buf[256] = {0,};
					swprintf_s( buf, 256, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, pSox->GetFieldFromLablePtr( nItemID, "_NameID" )->GetInteger() ), pBFname );					
					swprintf_s( strName, 256, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7130 ), buf );
				}
				else
					swprintf_s( strName, 256, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 4349 ) );

				GetInterface().AddChatMessage( CHATTYPE_SYSTEM, L"", strName, false );
			}
		}

	}

}

// Best friend info.
//void CDnItemTask::OnRecvInfoBF( char * pData )
//{
//	BestFriend::SCGetInfo * packet = (BestFriend::SCGetInfo *)( pData );
//
//	if( packet->iRet == ERROR_NONE )
//	{
//		if( packet->Info.biCharacterDBID != 0 )
//		{
//			if( CDnActor::s_hLocalActor.GetPointer() )
//			{
//				CDnPlayerActor *pPlayer = dynamic_cast<CDnPlayerActor*>( CDnActor::s_hLocalActor.GetPointer() );
//				if( pPlayer )
//				{
//					//pPlayer->SetAppellationName( packet->Info.wszName );
//					pPlayer->SetBestfriendName( packet->Info.wszName );
//
//					// Best friend title.
//					CDnAppellationTask::GetInstance().SetBFappellation();
//				}
//			}
//			GetInterface().SetBFData( packet->Info );
//		}
//	}
//
//	else
//	{
//		GetInterface().ServerMessageBox( packet->iRet );
//	}
//}
void CDnItemTask::OnRecvInfoBF( char * pData )
{
	BestFriend::SCGetInfo * packet = (BestFriend::SCGetInfo *)( pData );

	if( packet->iRet == ERROR_NONE &&
		packet->Info.biCharacterDBID != 0 )
	{		
		SAFE_DELETE( m_pBestfriendInfo );				
		m_pBestfriendInfo = new BestFriend::SCGetInfo;
		memcpy_s( m_pBestfriendInfo, sizeof(BestFriend::SCGetInfo), packet, sizeof(BestFriend::SCGetInfo) );

		//m_pBestfriendInfo = packet;	

		InitBestfriendInfo();
	}

	else
	{		
		GetInterface().ServerMessageBox( packet->iRet );
	}
}


// Send reward item index.
void CDnItemTask::RequestRewardItemBF( INT64 giftSerial, int itemID )
{
	//BestFriend::CSModItem packet;
	//packet.nItemID = itemID;
	CSBestFriendItemRequest packet;
	packet.biGiftSerial = giftSerial;
	packet.nMemuItemID = itemID;
	SendRewardItemBF( packet );
}

// Reward item result.
void CDnItemTask::OnRecvBestFriendItemReq( SCBestFriendItemRequest * packet )
{
	if( packet->nRet == ERROR_NONE )
	{		
		GetInterface().OpenBFRewardProgressDlg( true, packet->biGiftSerial );
	}

	else
	{
		GetInterface().ServerMessageBox( packet->nRet );
	}
}


// Reward item complete.
void CDnItemTask::SendBFItemComplet( INT64 giftSerial, int itemID )
{
	CSBestFriendItemComplete packet;
	packet.biGiftSerial = giftSerial;
	packet.nMemuItemID = itemID;
	SendRewardItemCompleteBF( packet );
}


// Reward item completion result.
void CDnItemTask::OnRecvBestFriendItemComp( SCBestFriendItemComplete * packet )
{
	int nResultItemID = 0;

	if( packet->nRet == ERROR_NONE )
	{		
		nResultItemID = packet->nResultItemID;	
	}

	else
	{
		GetInterface().ServerMessageBox( packet->nRet );
	}

	GetInterface().SendRewardComplet( nResultItemID );
}


// Cancel reward item selection.
void CDnItemTask::RequestBFItemCancel()
{	
	SendRewardItemCancelBF();
}


// Send best friend memo.
void CDnItemTask::RequestMemoBF( LPCWSTR strMemo )
{
	BestFriend::CSEditMemo packet;
	wcscpy_s( packet.wszMemo, BESTFRIENDMEMOMAX, strMemo );
	SendMemoBF( packet );
}


// Receive memo.	
void CDnItemTask::OnRecvMemo( char * pData )
{
	BestFriend::SCEditMemo * packet = (BestFriend::SCEditMemo *)pData;
	if( packet->iRet == ERROR_NONE )
	{
		GetInterface().SetMemoBF( packet );
	}

	else
	{
		GetInterface().ServerMessageBox( packet->iRet );
	}
}

// TRUE: break best-friend, FALSE: cancel breaking best-friend.
void CDnItemTask::RequestBrokeupOrCancelBF( bool bCancel )
{
	BestFriend::CSCancel packet;
	packet.bCancel = bCancel;

	SendBrokeupOrCancelBF( packet );
}


// Best friend or break best friend.
void CDnItemTask::OnRecvCancelBF( char * pData )
{
	BestFriend::SCCancel * packet = (BestFriend::SCCancel *)pData;
	if( packet->iRet == ERROR_NONE )
	{
		GetInterface().BrokeupOrCancelBF( packet->bCancel, packet->wszName );
	}

	else
	{
		GetInterface().ServerMessageBox( packet->iRet );
	}
}


// Dissolve best friend.
void CDnItemTask::OnRecvCloseBF()
{
	GetInterface().DestroyBF();

	/*CDnPlayerActor *pPlayer = dynamic_cast<CDnPlayerActor*>( CDnActor::s_hLocalActor.GetPointer() );
	if( pPlayer )
	{		
		pPlayer->SetBFserial( 0 );
		pPlayer->SetBestfriendName( L("") );
	}*/
	
	CDnActor::s_hLocalActor->SetBFserial( 0 );
	CDnActor::s_hLocalActor->SetBestfriendName( L"" );
}

// Request best friend info.
void CDnItemTask::RequestBFInfo()
{
	SendRequestBFInfo();
}

#endif

#ifdef PRE_FIX_MAKECHARMITEM
void CDnItemTask::MakeCharmItemList( std::vector<boost::tuple<int, int, int>> & vCharmItemList, const short sInvenIndex, const BYTE cInvenType, const INT64& nCharmItemSerial, CDnItem * pCharmItem )
#else
void CDnItemTask::MakeCharmItemList( std::vector<boost::tuple<int, int, int>> & vCharmItemList, const short sInvenIndex, const BYTE cInvenType, CDnItem * pCharmItem )
#endif
{
	vCharmItemList.clear();

	if( NULL == pCharmItem )
	{
		if (cInvenType == ITEMPOSITION_CASHINVEN)
		{
#ifdef PRE_FIX_MAKECHARMITEM
			pCharmItem = GetCashInventory().FindItemFromSerialID(nCharmItemSerial);
#else
			pCharmItem = GetCashInventory().GetItem(sInvenIndex);
#endif
		}
		else
		{
			pCharmItem = GetCharInventory().GetItem(sInvenIndex);
	}
	}
#ifdef PRE_ADD_GETITEM_WITH_RANDOMVALUE
	if (pCharmItem == NULL || (pCharmItem->GetItemType() != ITEMTYPE_CHARM && 
					           pCharmItem->GetItemType() != ITEMTYPE_ALLGIVECHARM && 
							   pCharmItem->GetItemType() != ITEMTYPE_CHARMRANDOM))
		return;
#else
#if defined( PRE_ADD_EASYGAMECASH )
	if (pCharmItem == NULL || (pCharmItem->GetItemType() != ITEMTYPE_CHARM && pCharmItem->GetItemType() != ITEMTYPE_ALLGIVECHARM) )
		return;
#else
	if (pCharmItem == NULL || pCharmItem->GetItemType() != ITEMTYPE_CHARM)
		return;
#endif
#endif
	
	int nCurItemCharmNum = pCharmItem->GetTypeParam(0);

	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TCHARMITEM );

	if( NULL == pSox )
		return;

	int nTableCount = pSox->GetItemCount();
	
	for (int i = 0; i < nTableCount; ++i)
	{
		int nTableItemID = pSox->GetItemID(i);

		const DNTableCell *pCharmNumField = pSox->GetFieldFromLablePtr(nTableItemID, "_CharmNum");
		if (pCharmNumField == NULL)
			continue;

		int nCharmNo = pCharmNumField->GetInteger();
		if (nCharmNo != nCurItemCharmNum) continue;

		const DNTableCell *pItemIDField = pSox->GetFieldFromLablePtr(nTableItemID, "_ItemID");
		if (pItemIDField == NULL)
			continue;

#if defined(PRE_ADD_54166)
		//#54166 2nd Pandora Box system overhaul - Added feature to show selectable contents
		const DNTableCell *pLookField = pSox->GetFieldFromLablePtr(nTableItemID, "_Look");
		if (pLookField == NULL || pLookField->GetBool() == false)
			continue;
#endif // PRE_ADD_54166

		int nItemID = pItemIDField->GetInteger();

		const DNTableCell *pCountField = pSox->GetFieldFromLablePtr(nTableItemID, "_Count");
		if (pCountField == NULL || pCountField->GetInteger() <= 0)
			continue;

		const DNTableCell *pPeriodField = pSox->GetFieldFromLablePtr(nTableItemID, "_Period");
		if( NULL == pPeriodField )
			continue;

		int nCount = pCountField->GetInteger();
		int nPeriod = pPeriodField->GetInteger();
		
		vCharmItemList.push_back( boost::make_tuple<int, int, int>( nItemID, nCount, nPeriod ) );
	}
	
}

#if defined( PRE_ADD_LIMITED_SHOP )
void CDnItemTask::OnRecvLimitedShopItemData( SCLimitedShopItemData *pPacket)
{
	if(pPacket->bAllClear)
		ClearLimitedShopItemCount();

	for(int i=0;i<pPacket->count;i++)
	{
		AddLimitedShopItemCount(pPacket->items[i]);
	}
}
void CDnItemTask::AddLimitedShopItemCount(LimitedShop::LimitedItemData ItemData)
{
#if defined( PRE_FIX_74404 )
	bool bFlag = false;
	for( std::list<LimitedShop::LimitedItemData>::iterator itor=m_listLimitedShopItem.begin();itor!=m_listLimitedShopItem.end();itor++)
	{
		if( itor->nShopID == ItemData.nShopID && itor->nItemID == ItemData.nItemID )
		{
			itor->nBuyCount = ItemData.nBuyCount;
			bFlag = true;
		}
	}
	if( !bFlag )
	{
		m_listLimitedShopItem.push_back(ItemData);
	}
#else// #if defined( PRE_FIX_74404 )
	std::map<int, LimitedShop::LimitedItemData>::iterator itor = m_mapLimitedShopItem.find(ItemData.nItemID);
	if(itor != m_mapLimitedShopItem.end())
		itor->second.nBuyCount = ItemData.nBuyCount;
	else
		m_mapLimitedShopItem.insert(std::make_pair(ItemData.nItemID, ItemData));
#endif// #if defined( PRE_FIX_74404 )
}

#if defined( PRE_FIX_74404 )
int CDnItemTask::GetLimitedShopItemCount(int nShopID, int nItemID)
{
	for( std::list<LimitedShop::LimitedItemData>::iterator itor=m_listLimitedShopItem.begin();itor!=m_listLimitedShopItem.end();itor++)
	{
		if( itor->nShopID == nShopID && itor->nItemID == nItemID )
		{
			return itor->nBuyCount;
		}
	}
	return 0;
}
#else// #if defined( PRE_FIX_74404 )
int CDnItemTask::GetLimitedShopItemCount(int nItemID)
{
	std::map<int, LimitedShop::LimitedItemData>::iterator itor = m_mapLimitedShopItem.find(nItemID);
	if(itor != m_mapLimitedShopItem.end())
		return itor->second.nBuyCount;
	return 0;
}
#endif// #if defined( PRE_FIX_74404 )

void CDnItemTask::ClearLimitedShopItemCount()
{
#if defined( PRE_FIX_74404 )
	m_listLimitedShopItem.clear();	
#else// #if defined( PRE_FIX_74404 )
	m_mapLimitedShopItem.clear();	
#endif// #if defined( PRE_FIX_74404 )
}
#endif //PRE_ADD_LIMITED_SHOP


#if defined( PRE_ADD_TRANSFORM_POTION)
void CDnItemTask::OnRecvTransformItem( SCChangeTransform *pPacket )
{
	if( CDnWorld::IsActive() && CDnWorld::GetInstance().GetMapType() == CDnWorld::MapTypeVillage ) 
	{
		if( pPacket )
		{
			DnActorHandle hActor = CDnActor::FindActorFromUniqueID( pPacket->nSessionID );
			if( hActor && hActor->IsPlayerActor() )
			{
				CDnPlayerActor *pPlayer = static_cast<CDnPlayerActor *>( hActor.GetPointer() );
				pPlayer->ToggleVillageTransformMode( ( pPacket->nTransformID > 0 ) , pPacket->nTransformID , pPacket->nExpireTime );
			}
		}
	}
}
#endif

#ifdef PRE_ADD_SERVER_WAREHOUSE
void CDnItemTask::OnRecvItemWorldServerStorageList(SCServerWareList* pPacket)
{
	int nMaxInvenCount = GetItemTask().GetWorldServerStorageInventory().GetMaxStorageCount(false);
	GetItemTask().GetWorldServerStorageInventory().SetUsableSlotCount(nMaxInvenCount);

	// todo by kalliste : Case where warecount is larger than max inven (server merge)

	int i = 0;
	for (; i < pPacket->cWareCount; i++)
	{
		m_WorldServerStorageInventory.CreateItem(pPacket->WareItems[i], m_nWorldServerStorageInvenCounter + i);
	}

	m_nWorldServerStorageInvenCounter += pPacket->cWareCount;
	if (m_nWorldServerStorageInvenCounter == nMaxInvenCount)
		m_nWorldServerStorageInvenCounter = 0;

	if (!m_nWorldServerStorageInvenCounter)
	{
		m_WorldServerStorageInventory.SortCashInventory();
		//OnFinishRecvCashInventory();
	}
}

void CDnItemTask::OnRecvItemWorldServerCashStorageList(SCCashServerWareList* pPacket)
{
	int nMaxInvenCount = GetItemTask().GetWorldServerStorageCashInventory().GetMaxStorageCount(true);
	GetItemTask().GetWorldServerStorageCashInventory().SetUsableSlotCount(nMaxInvenCount);

	// todo by kalliste : Case where warecount is larger than max inven (server merge)

	int i = 0;
	for (; i < pPacket->cWareCount; i++)
	{
		m_WorldServerStorageCashInventory.CreateItem(pPacket->WareCashItems[i], m_nWorldServerStorageCashInvenCounter + i);
	}

	m_nWorldServerStorageCashInvenCounter += pPacket->cWareCount;
	if (m_nWorldServerStorageCashInvenCounter == nMaxInvenCount)
		m_nWorldServerStorageCashInvenCounter = 0;

	if (!m_nWorldServerStorageCashInvenCounter)
	{
		m_WorldServerStorageInventory.SortCashInventory();
		//OnFinishRecvCashInventory();
	}
}

#endif


#ifdef PRE_ADD_GAMEQUIT_REWARD
// Find Item from Table.
void CDnItemTask::FindEventItems( int nEventType, int nLevel, int nClass, std::vector< CDnItem * > & vecItem )
{
	DNTableCell * pCell = NULL;		

	// LevelupEventTable.
	DNTableFileFormat * pTable = GetDNTable( CDnTableDB::TLEVELUPEVENT ); 
	if( !pTable )
		return;
	int size = pTable->GetItemCount();

	// MailTable.
	DNTableFileFormat * pTableMail = GetDNTable( CDnTableDB::TMAIL ); 
	if( !pTableMail )
		return;

	// CashCommodity.
	DNTableFileFormat * pTableCashCommodity = GetDNTable( CDnTableDB::TCASHCOMMODITY ); 
	if( !pTableCashCommodity )
		return;

	// ItemTable.
	DNTableFileFormat * pTableItem = GetDNTable( CDnTableDB::TITEM ); 
	if( !pTableItem )
		return;

	// CharmItemTable.
	DNTableFileFormat * pTableCharmItem = GetDNTable( CDnTableDB::TCHARMITEM ); 
	if( !pTableCharmItem )
		return;
	int sizeCharm = pTableCharmItem->GetItemCount();

	// LevelupEventTable //		
	for( int i=0; i<size; ++i )
	{
		int nID = pTable->GetItemID( i );

		pCell = pTable->GetFieldFromLablePtr( nID, "_EventType" );
		if( !pCell || pCell->GetInteger() != nEventType )
			continue;

		if( nLevel > 0 )
		{
			pCell = pTable->GetFieldFromLablePtr( nID, "_PCLevel" );
			if( !pCell )
				continue;

			if( pCell->GetInteger() != nLevel )
				continue;
		}

		if( nClass > -1 )
		{
			pCell = pTable->GetFieldFromLablePtr( nID, "_ClassID" );
			if( !pCell )
				continue;
			if( pCell->GetInteger() != 0 && pCell->GetInteger() != nClass )
				continue;
		}

		pCell = pTable->GetFieldFromLablePtr( nID, "_CashMailID" );	
		if( !pCell)
			continue;
		int nMailID = pCell->GetInteger();


		// MailTable //
		pCell = pTableMail->GetFieldFromLablePtr( nMailID, "_MailPresentItem1" );	
		if( !pCell )
			continue;
		int nMailPresentItem = pCell->GetInteger();

		// CashCommodityTable //
		pCell = pTableCashCommodity->GetFieldFromLablePtr( nMailPresentItem, "_SN" );
		if( !pCell )
			continue;
		pCell = pTableCashCommodity->GetFieldFromLablePtr( nMailPresentItem, "_ItemID01" );
		if( !pCell )
			continue;		
		int nItemID = pCell->GetInteger();

		// ItemTable //
		pCell = pTableItem->GetFieldFromLablePtr( nItemID, "_TypeParam1" );
		if( !pCell )
			continue;		
		int nCharmNum = pCell->GetInteger();

		// CharmItemTable //
		for( int k=0; k<sizeCharm; ++k )
		{
			int idCharm = pTableCharmItem->GetItemID( k );
			pCell = pTableCharmItem->GetFieldFromLablePtr( idCharm, "_CharmNum" );
			if( !pCell )
				continue;
			if( pCell->GetInteger() != nCharmNum )
				continue;

			pCell = pTableCharmItem->GetFieldFromLablePtr( idCharm, "_ItemID" );
			if( !pCell )
				continue;
			nItemID = pCell->GetInteger();

			// ItemTable - WEAPON,PARTS //
			pCell = pTableItem->GetFieldFromLablePtr( nItemID, "_Type" );
			if( !pCell )
				continue;
			if( pCell->GetInteger() == eItemTypeEnum::ITEMTYPE_WEAPON || pCell->GetInteger() == eItemTypeEnum::ITEMTYPE_PARTS )
			{
				pCell = pTableItem->GetFieldFromLablePtr( nItemID, "_NeedJobClass" );
				if( !pCell )
					continue;

				std::vector<int> vPermitJobList;
				char * szPermitStr = pCell->GetString();
				for( int i=0;; i++ )
				{
					std::string strValue = _GetSubStrByCountSafe( i, szPermitStr, ';' );
					if( strValue.size() == 0 )
						break;

					int nJobIndex = atoi(strValue.c_str());

					if( nJobIndex < 1 )
						continue;

					vPermitJobList.push_back( nJobIndex );
				}

				bool bJob = false;
				int jobCnt = (int)vPermitJobList.size();
				for( int x=0; x<jobCnt; ++x )
				{
					if( vPermitJobList[ x ] == nClass )
					{
						bJob = true;
						break;
					}
				}

				if( !bJob )
					continue;

			}

			int itemCnt = 1;
			pCell = pTableCharmItem->GetFieldFromLablePtr( idCharm, "_Count" );
			if( !pCell )
				continue;
			itemCnt = pCell->GetInteger();		


			// Create item.
			CDnItem * pItem = CDnItem::CreateItem( nItemID, itemCnt );
			pItem->SetOverlapCount( itemCnt );

			vecItem.push_back( pItem );
		}


	}// LevelupEventTable //	
}
#endif // PRE_ADD_GAMEQUIT_REWARD=======


#ifdef PRE_ADD_PVP_EXPUP_ITEM
void CDnItemTask::OnRecvItemPvPExpUp( SCUsePvPExpup* pPacket )
{
	if( pPacket->nRet != ERROR_NONE ) {
		if( pPacket->nRet == ERROR_CANT_USE_PVPEXPUPITEM_DAILY ) {
			int nLimit = static_cast<int>( CGlobalWeightTable::GetInstance().GetValue( CGlobalWeightTable::PvPExpUpItem ) );
			WCHAR wzStrTmp[1024]={0,};
			swprintf_s( wzStrTmp, _countof(wzStrTmp), GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 114044), nLimit );
			GetInterface().MessageBox( wzStrTmp, MB_OK );
		}
		return;
	}

	WCHAR wzStrTmp[1024]={0,};
	swprintf_s( wzStrTmp, _countof(wzStrTmp), GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 8363), pPacket->nIncrement );
	GetInterface().AddChatMessage( CHATTYPE_SYSTEM, L"", wzStrTmp, false );

	if( CDnPartyTask::IsActive() )
	{
		if( CDnPartyTask::GetInstance().GetPvPInfo() )
		{
			CDnPartyTask::GetInstance().GetPvPInfo()->cLevel = pPacket->cLevel;
			CDnPartyTask::GetInstance().GetPvPInfo()->uiXP = pPacket->uiXP;
		}
	}
	if( CDnActor::s_hLocalActor )
	{
		CDnPlayerActor* pLocalActor = static_cast<CDnPlayerActor*>( CDnActor::s_hLocalActor.GetPointer() );
		pLocalActor->SetPvPLevel( pPacket->cLevel );
	}
}
#endif


#ifdef PRE_ADD_EQUIPLOCK
void CDnItemTask::OnRecvEquipItemLockList(SCEquipItemLockList* pData)
{
	m_ItemLockMgr.SetEquipLockItemList(*pData);
}

void CDnItemTask::OnRecvEquipItemLock(SCItemLockRes* pData)
{
	m_ItemLockMgr.RefreshEquipLockItem(*pData);
}

void CDnItemTask::OnRecvEquipItemUnLock(SCItemLockRes* pData)
{
	m_ItemLockMgr.RefreshEquipLockItem(*pData);
}
#endif

#if defined(PRE_PERIOD_INVENTORY)
void CDnItemTask::OnRecvPeriodInventory( SCPeriodInventory * pData )
{
	CDnInvenTabDlg* pInvenTabDlg = (CDnInvenTabDlg*)GetInterface().GetMainMenuDialog( CDnMainMenuDlg::INVENTORY_DIALOG );
	if( NULL == pInvenTabDlg )
		return;

	CDnInvenDlg * pIvenDlg = pInvenTabDlg->GetInvenDlg( ITEM_SLOT_TYPE::ST_INVENTORY );
	if( NULL == pIvenDlg )
		return;

	pIvenDlg->EnablePeriodInven( pData->bEnable, pData->tExpireDate );
}

void CDnItemTask::OnRecvPeriodWarehouse( SCPeriodWarehouse * pData )
{
	CDnPGStorageTabDlg* pPGStorageDlg = (CDnPGStorageTabDlg*)GetInterface().GetMainMenuDialog( CDnMainMenuDlg::PLAYER_GUILD_STORAGE_DIALOG );
	if( NULL == pPGStorageDlg )
		return;

#if defined( PRE_ADD_SERVER_WAREHOUSE )
	CDnPlayerStorageDlg * pPlayerStorageDlg = (CDnPlayerStorageDlg *)pPGStorageDlg->GetStorageDlg( StorageUIDef::eTAB_PLAYER );
#else
	CDnPlayerStorageDlg * pPlayerStorageDlg = (CDnPlayerStorageDlg *)pPGStorageDlg->GetStorageDlg( CDnPGStorageTabDlg::PLAYER_STORAGE_TAB );
#endif

	if( NULL == pPlayerStorageDlg )
		return;

	pPlayerStorageDlg->EnablePeriodStorage( pData->bEnable, pData->tExpireDate );
}

void CDnItemTask::OnRecvSortPeriodInventory( SCSortInventory * pData )
{
	if( pData->nRet == ERROR_NONE )
	{
		if( m_CharInventory.Sort( true ) )
		{
			if( m_nItemSortSound != -1 )
				CEtSoundEngine::GetInstance().PlaySound( "2D", m_nItemSortSound, false );
			m_bRequestWait = false;
			return;
		}
		else
		{
			// If the server sent OK but the client failed, just disconnect. Such cases rarely happen in practice.
			GetInterface().MessageBox( 5061, MB_OK, MESSAGEBOX_10, CDnBridgeTask::GetInstancePtr(), true, false, false, true );
		}
	}
	else
	{
		m_CharInventory.Sort( false );
	}

	m_bRequestWait = false;

	// Sort failed (either server failed or client m_CharInventory.Sort(true) failed)
	GetInterface().AddChatMessage( CHATTYPE_SYSTEM, L"", GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 526 ), true );
}

void CDnItemTask::OnRecvSortPeriodWarehouse( SCSortWarehouse * pData )
{
	if( pData->nRet == ERROR_NONE )
	{
		if( m_StorageInventory.Sort( true ) )
		{
			if( m_nItemSortSound != -1 )
				CEtSoundEngine::GetInstance().PlaySound( "2D", m_nItemSortSound, false );
			m_bRequestWait = false;
			return;
		}
		else
		{
			// If the server sent OK but the client failed, just disconnect. Such cases rarely happen in practice.
			GetInterface().MessageBox( 5061, MB_OK, MESSAGEBOX_10, CDnBridgeTask::GetInstancePtr(), true, false, false, true );
		}
	}
	else
	{
		m_StorageInventory.Sort( false );
	}

	m_bRequestWait = false;

	// Sort failed (either server failed or client m_CharInventory.Sort(true) failed)
	GetInterface().AddChatMessage( CHATTYPE_SYSTEM, L"", GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 526 ), true );
}
#endif	// #if defined(PRE_PERIOD_INVENTORY)

#ifdef PRE_ADD_STAGE_USECOUNT_ITEM
void CDnItemTask::OnRecvStageUseLimitItem( SCStageUseLimitItem* pData )
{
	ASSERT(pData);
	CDnItem* pFindItem = m_CharInventory.FindItem(pData->nItemID);
	if(pFindItem)
	{
		bool bEnable = (pData->nUseCount > 0);
		pFindItem->SetUseLimit(!bEnable);
	}
}
#endif // PRE_ADD_STAGE_USECOUNT_ITEM

#ifdef PRE_ADD_LEVELUP_REWARD_NOTIFIER
bool CDnItemTask::IsLevelUpBoxItem( const CDnItem* pItem )
{
	if( pItem == NULL ) return false;

	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TLEVELUPCHECKBOX );
	if( ! pSox ) return false;

	char szTemp[256] = { 0, };
	sprintf_s( szTemp, "_CheckItemID0%d", 1 );
	
	int nItemID = 0;
	for( int i = 0; i < pSox->GetItemCount(); ++i )
	{
		nItemID = pSox->GetFieldFromLablePtr( i+1, szTemp )->GetInteger();
		if( pItem->GetClassID() == nItemID )
		{
			return true;
		}
	}
	
	return false;
}

CDnItem* CDnItemTask::GetExistLevelUpBox( const int& nLevel )
{	
	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TLEVELUPCHECKBOX );

	int nID = nLevel - 1; // Reward box is granted starting from level 2. As a level 2 box 	
	if( ! pSox || ! pSox->IsExistItem( nID ) ) return false;
	
	char szTemp[256] = { 0, };	
	sprintf_s( szTemp, "_CheckItemID0%d", 1 );

	for( int i = 0 ; i < nLevel; i++ )
	{
		if( nID < 1 ) return NULL;

		int nItemID = pSox->GetFieldFromLablePtr( nID-- , szTemp )->GetInteger();
		if( nItemID > 0 )
		{
			CDnItem* pFindItem = GetItemTask().FindItem( nItemID, ITEM_SLOT_TYPE::ST_INVENTORY_CASH );
			if( pFindItem ) return pFindItem;
			else continue;
		}
	}

	return NULL;
}
#endif 

void CDnItemTask::OnRecvSpecialItemList( SCSpecialItemList* pPacket )
{
	if( !pPacket ) return;
	m_vecSpecialItemIDs.clear();
	for( int i = 0; i < 10; ++i )
	{
		if( pPacket->nItemIDs[i] != 0 )
			m_vecSpecialItemIDs.push_back( pPacket->nItemIDs[i] );
	}
}

bool CDnItemTask::HasSpecialItemID( int nItemID ) const
{
	if( nItemID == 0 || m_vecSpecialItemIDs.empty() )
		return false;
	for( size_t i = 0; i < m_vecSpecialItemIDs.size(); ++i )
	{
		if( m_vecSpecialItemIDs[i] == nItemID )
			return true;
	}
	return false;
}

void CDnItemTask::OnRecvAutoUnpackResult( SCAutoUnpackResult* pPacket )
{
	if( !pPacket ) return;
	if( CDnInterface::IsActive() )
	{
		if( GetInterface().GetPotenCharmItemProgressDlg() && GetInterface().GetPotenCharmItemProgressDlg()->IsShow() )
			GetInterface().GetPotenCharmItemProgressDlg()->Show( false );
		if( GetInterface().GetPackageBoxResultAutoDlg() && GetInterface().GetPackageBoxResultAutoDlg()->IsShow() )
			GetInterface().GetPackageBoxResultAutoDlg()->Show( false );
		if( pPacket->nRet != ERROR_NONE )
			GetInterface().ServerMessageBox( pPacket->nRet );
	}
}

void CDnItemTask::OnRecvDeleteExpireItemList( SCDeleteExpireItemList* pPacket )
{
	if( !pPacket || pPacket->wCount == 0 ) return;
	WORD wCount = pPacket->wCount;
	if( wCount > CASHINVENTORYMAX ) wCount = CASHINVENTORYMAX;

	for( WORD i = 0; i < wCount; ++i )
	{
		INT64 biSerial = pPacket->biSerial[i];
		if( biSerial <= 0 ) continue;

		CDnItem* pItem = m_CashInventory.FindItemFromSerialID( biSerial );
		if( pItem )
		{
			m_CashInventory.RemoveItem( pItem->GetSlotIndex() );
			continue;
		}
		pItem = m_StorageInventory.FindItemFromSerialID( biSerial );
		if( pItem )
		{
			m_StorageInventory.RemoveItem( pItem->GetSlotIndex() );
			continue;
		}
		pItem = m_CharInventory.FindItemFromSerialID( biSerial );
		if( pItem )
		{
			m_CharInventory.RemoveItem( pItem->GetSlotIndex() );
			continue;
		}

		bool bFound = false;
		for( int j = 0; j < CASHEQUIPMAX; ++j )
		{
			if( m_pCashEquip[j] && m_pCashEquip[j]->GetSerialID() == biSerial )
			{
				RemoveCashEquipItem( j );
				bFound = true;
				break;
			}
		}
		if( bFound ) continue;

		for( int j = 0; j < EQUIPMAX; ++j )
		{
			if( m_pEquip[j] && m_pEquip[j]->GetSerialID() == biSerial )
			{
				RemoveEquipItem( j );
				break;
			}
		}
	}
}