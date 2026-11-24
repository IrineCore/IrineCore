
#include "stdafx.h"
#include "DnTableDB.h"
#include <boost/algorithm/string.hpp>
#if defined(_GAMESERVER)
#include "DNGameRoom.h"
#elif !defined(_VILLAGESERVER)
#include "DnGameTask.h"
#endif
#ifdef _TEST_CODE_KAL
#include "PerfCheck.h"
#endif

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnTableDB::CDnTableDB()
#ifdef PRE_ADD_REDUCE_TABLE_RESOURCE
: m_bTableUseInGame( false )
#endif // PRE_ADD_REDUCE_TABLE_RESOURCE
{
}

CDnTableDB::~CDnTableDB()
{
	Finalize();
}

#ifdef PRE_ADD_REDUCE_TABLE_RESOURCE
void CDnTableDB::SetTableFileInfo()
{
	AddTableFileInfo( TACTOR, "ActorTable", "_StaticName", true );
	AddTableFileInfo( TPARTS, "PartsTable", "_SetItemID", true, false );
	AddTableFileInfo( TWEAPON, "WeaponTable", "_SetItemID", true, false );
	AddTableFileInfo( TTILE, "TileTable", "_Name" );
	AddTableFileInfo( TENVIEFFECT, "EnviEffectTable", NULL, true );
	AddTableFileInfo( TMAP, "MapTable", "_EnterConditionTableID;_MapSubType", true, false );
	AddTableFileInfo( TPLAYERLEVEL, "PlayerLevelTable" );
	AddTableFileInfo( TMONSTER, "MonsterTable", "_ActorTableID", true, false );
	AddTableFileInfo( TMONSTERCOMPOUND, "MonsterCompoundTable" );
	AddTableFileInfo( TITEM, "ItemTable", NULL, true );
	AddTableFileInfo( TPROP, "PropTable", "_Name", true, false );
	AddTableFileInfo( TJOB, "JobTable", "_JobNumber;_BaseClass", false, false );
	AddTableFileInfo( TSKILL, "SkillTable", NULL, true );
	AddTableFileInfo( TITEMDROP, "ItemDropTable", NULL, true );
	AddTableFileInfo( TDEFAULTCREATE, "DefaultCreateTable" );
	AddTableFileInfo( TNPC, "NpcTable", "_ActorIndex", true, false );
	AddTableFileInfo( TQUEST, "QuestTable", NULL, true );
	AddTableFileInfo( TPLAYERWEIGHT, "PlayerWeightTable" );
	AddTableFileInfo( TTALKPARAM, "TalkParamTable" );
	AddTableFileInfo( TSKILLLEVEL, "SkillLevelTable", "_SkillIndex", true, false );
	AddTableFileInfo( TMAPGATE, "MapGateTable", "_PermitPlayerLevel;_MapIndex", false, false );
	AddTableFileInfo( TSOCIAL, "ActorFaceTable", NULL, true );
	AddTableFileInfo( TMONSTERSKILL, "MonsterSkillTable", NULL, true );
	AddTableFileInfo( TSHOP, "ShopTable" );
	AddTableFileInfo( TMONSTERPARTS, "MonsterPartsTable" );
	AddTableFileInfo( TSKILLSHOP, "SkillShopTable" );
	AddTableFileInfo( TSUMMONPROP, "SummonPropTable" );
	AddTableFileInfo(TDUNGEONENTER, "StageEnterTable", "_LvlMin;_AbyssLvlMin;_AbyssQuestID;_RecommendEasyLevel;_RecommendNormalLevel;_RecommendHardLevel;_RecommendVeryHardLevel;_RecommendNightmareLevel", false, false);
	AddTableFileInfo( TDUNGEONCLEAR, "StageClearTable" );
	AddTableFileInfo( TCLEARSTANDARD, "ClearStandard" );
	AddTableFileInfo( TSYMBOLITEM, "SymbolItemTable" );
	AddTableFileInfo( TCUTSCENE, "CutSceneTable" );
	AddTableFileInfo( TPLATE, "PlateTable" );
	AddTableFileInfo( TITEMCOMPOUND, "ItemCompoundTable", "_CompoundGroupID;_CompoundPotentialID", false, false );
	AddTableFileInfo( TSTATEEFFECT, "StateEffectTable" );
	AddTableFileInfo( TDUNGEONMAP, "StageSetTable" );
	AddTableFileInfo( TTODAYTIP, "TodayToolTipTable" );
	AddTableFileInfo( TFILE, "FileTable", NULL, true );
	AddTableFileInfo( TQUESTREWARD, "QuestRewardTable" );
	AddTableFileInfo( TMONSTERWEIGHT, "MonsterWeightTable", "_MonsterTableIndex", true, false );
	AddTableFileInfo( TQUESTCHAPTER, "QuestChapterTable" );
	AddTableFileInfo( TEQUIPMATCHACTION, "EquipMatchActionTable", "_Class", false, false );
	AddTableFileInfo( TFACE, "FaceAnimationTable", NULL, true );
	AddTableFileInfo( TREBIRTHCOIN, "CoinTable" );
	AddTableFileInfo( TPVPMAP, "PvPMapTable", NULL, false, false );
	AddTableFileInfo( TPVPGAMEMODE, "PvPGameModeTable" );
	AddTableFileInfo( TPVPGAMESTARTCONDITION, "PvPGameStartConditionTable" );
	AddTableFileInfo( TCP, "CPTable" );
	AddTableFileInfo( TGAMEROOMNAME, "PvPDefaultRoomNameTable" );
	AddTableFileInfo( TGLOBALWEIGHT, "GlobalWeightTable" );
	AddTableFileInfo( TGLOBALWEIGHTINT, "GlobalWeightIntTable" );
#ifdef PRE_FIX_MEMOPT_ENCHANT
	AddTableFileInfo( TENCHANT_MAIN, "enchantmaintable", "_EnchantID", true, false );
#else
	AddTableFileInfo( TENCHANT, "EnchantTable", "_EnchantID", true, false );
#endif
	AddTableFileInfo( TPOTENTIAL, "PotentialTable", "_PotentialID", true, false );
	AddTableFileInfo( TSETITEM, "SetItemTable" );
	AddTableFileInfo( TEQUALLEVEL, "EqualLevelTable", "_Mode", false, false );
	AddTableFileInfo( TMISSION, "MissionTable" );
	AddTableFileInfo( TAPPELLATION, "AppellationTable" );
	AddTableFileInfo( TPVPRANK, "PvPRankTable" );
	AddTableFileInfo( TPVPWINCONDITION, "PvPGameStartConditionTable" );
	AddTableFileInfo( TITEMCOMPOUNDSHOP, "ItemCompoundShopTable", "_ListID", false, false );
	AddTableFileInfo( TEQUALBASIS, "EqualBasisTable" );
	AddTableFileInfo( TDAILYMISSION, "DailyMissionTable" );
	AddTableFileInfo( TSCHEDULE, "ScheduleTable" );
	AddTableFileInfo( TWORLDMAPUI, "WorldMapUI" );
	AddTableFileInfo( TTAX, "TaxTable", NULL, true );
	AddTableFileInfo( TMISSIONSCHEDULE, "MissionScheduleTable" );
	AddTableFileInfo( TRADIOMSG, "RadioMsgTable", "_MacroNum", false, false );
	AddTableFileInfo( TGESTURE, "GestureTable" );
	AddTableFileInfo( TMONSTERGROUP, "MonsterGroupTable" );
	AddTableFileInfo( TMONSTERSET, "MonsterSetTable", "_SetNo", false, false );
	AddTableFileInfo( TSKILLTREE, "SkillTreeTable", "_SkillTableID", false, false );
	AddTableFileInfo( TITEMDROPGROUP, "ItemDropGroupTable", "_DropGroupID", true, false );
	AddTableFileInfo( TGUILDUSERCAP, "GuildUserCapTable" );
	AddTableFileInfo( TDLMAP, "DLMapTable" );
	AddTableFileInfo( TDLDUNGEONCLEAR, "DLStageClearTable" );
	AddTableFileInfo( TCASHCOMMODITY, "CashCommodity" );
	AddTableFileInfo( TCASHGOODSINFO, "CashGoodsInfoTable" );
	AddTableFileInfo( TCASHPACKAGE, "CashPackageTable" );
	AddTableFileInfo( TCASHLIMIT, "CashLimitTable" );
	AddTableFileInfo( TWEATHER, "WeatherTable" );
	AddTableFileInfo( TPCCAFE, "PCCafeTable" );
	AddTableFileInfo( TFATIGUEADJUST, "FatigueAdjustmentTable" );
	AddTableFileInfo( TCASHPRESENTREACT, "CashPresentReactTable" );
#ifdef PRE_ADD_GACHA_JAPAN
	AddTableFileInfo( TGACHA_JP, "GachaJP", "_GachaNum", false, false );
	AddTableFileInfo( TGACHASETITEM_JP, "GachaSetItemJP", "_GachaNum", false, false );
#endif
	AddTableFileInfo( TSEALCOUNT, "SealCountTable" );
	AddTableFileInfo( TLEVELUPEVENT, "LevelupEventTable" );
#if defined(PRE_ADD_VIP)
	AddTableFileInfo( TVIP, "VIPTable" );
	AddTableFileInfo( TVIPGRADE, "VIPGradeTable" );
#endif	// #if defined(PRE_ADD_VIP)
	AddTableFileInfo( TCASHTAB,"CashTabTable" );
	AddTableFileInfo( TFAIRY,"FairyTable" );
	AddTableFileInfo( TCOSMIX,"CashItemCompoundTable" );
	AddTableFileInfo( TCOSMIXABILITY,"CashCompoundInfoTable" );
	AddTableFileInfo( TGLYPHSKILL,"GlyphSkillTable" );
	AddTableFileInfo( TGLYPHSLOT,"GlyphSlotTable" );
	AddTableFileInfo( TLEVELPROMO,"WelfareTable" );

#if defined( PRE_ADD_NPC_REPUTATION_SYSTEM )
	AddTableFileInfo( TREPUTE,"ReputeTable", "_NpcID", false, false );
	AddTableFileInfo( TPRESENT,"PresentTable" );
	AddTableFileInfo( TNPCREACTION,"NpcReactionTable" );
	AddTableFileInfo( TSTOREBENEFIT,"storebenefittable", "_NpcID" );
#endif // #if defined( PRE_ADD_NPC_REPUTATION_SYSTEM )
	AddTableFileInfo( TMAIL,"MailTable", NULL, true );
	AddTableFileInfo( TMODDLG,"CustomDialogTable" );
	AddTableFileInfo( TMASTERSYSTEM_DECREASE,"MasterSysDecreaseTable" );
	AddTableFileInfo( TMASTERSYSTEM_GAIN,"MasterSysGainTable" );
	AddTableFileInfo( TVEHICLE,"VehicleTable", "_VehicleClassID", true, false );
	AddTableFileInfo( TVEHICLEPARTS,"VehiclePartsTable", NULL, true );
	AddTableFileInfo( TPOTENTIALJEWEL,"PotentialJewelTable" );
	AddTableFileInfo( TEVENTPOPUP,"PopUpTable" );
	AddTableFileInfo( TCHARMITEM,"CharmItemTable" );
	AddTableFileInfo( TCHARMITEMKEY,"CharmItemkeyTable" );
#if defined( PRE_ADD_SECONDARY_SKILL )
	AddTableFileInfo( TSecondarySkill,"secondaryskilltable" );
	AddTableFileInfo( TSecondarySkillLevel,"secondaryskillleveltable" );
	AddTableFileInfo( TSecondarySkillRecipe,"makeitemrecipetable" );
#endif // #if defined( PRE_ADD_SECONDARY_SKILL )
	AddTableFileInfo( TSKILLVIDEO,"SkillVideoTable" );
	AddTableFileInfo( TGUILDWAREHOUSE,"GuildWareHouse" );
	AddTableFileInfo( TFARMCULTIVATE,"farmcultivatetable" );
	AddTableFileInfo( TFARMSKIN,"farmskintable" );
	AddTableFileInfo( TFISHING,"fishingtable" );
	AddTableFileInfo( TFISHINGPOINT,"fishingpointtable" );
	AddTableFileInfo( TFISHINGAREA,"fishingareatable" );
	AddTableFileInfo( TFISHINGROD,"fishingrodtable" );
	AddTableFileInfo( TQUESTPERIOD,"QuestDailyTable" );
	AddTableFileInfo( TGLOBALEVENTQUEST,"globalevent" );
	AddTableFileInfo( TSTAFFROLL,"StaffRollTable" );
	AddTableFileInfo( TCOLLECTIONBOOK,"CollectionBookTable" );
	AddTableFileInfo( TEXCHANGETRADE,"Exchange" );
	AddTableFileInfo( TSKILLBUBBLE,"SkillBubbleTable" );
	AddTableFileInfo( TSKILLBUBBLEDEFINE,"SkillBubbleDefine" );
	AddTableFileInfo( TENCHANTJEWEL,"EnchantJewelTable" );
	AddTableFileInfo( TPVPMISSIONROOM,"PvPMissionRoomTable" );
	AddTableFileInfo( TEQUALEXPONENT,"EqualExponentValue" );
#ifdef PRE_MOD_ITEM_COMPOUND_DIALOG_RENEWAL
	AddTableFileInfo( TITEMCOMPOUNDGROUP,"ItemCompoundGroupTable", "_ListID;_MainCategory;_JobClass;_Level", false, false );
#else
	AddTableFileInfo( TITEMCOMPOUNDGROUP,"ItemCompoundGroupTable", "_ListID", false, false );
#endif
	AddTableFileInfo( TEXPADJUST,"expadjustmenttable" );
	AddTableFileInfo( TSTAGEREWARD,"StageRewardTable" );
	AddTableFileInfo( TMONSTERSKILL_TRANS,"monsterskilltable_transform" );
	AddTableFileInfo( TMONSTER_TRANS,"monstertable_transform" );
	AddTableFileInfo( TMONSTERGROUP_TRANS,"monstergrouptable_transform" );
	AddTableFileInfo( TGHOULMODE_CONDITION,"ghoulconditiontable" );
	AddTableFileInfo( TPVPGAMEMODESKILLSETTING,"pvpgamemodeskillsetting" );
	AddTableFileInfo( TMONSTER_TRANS_RADIOMSG,"monsterradiomsgtable" );
	AddTableFileInfo( TMONSTER_TRANS_RADIOBASE,"monsterradiochangetable" );
	AddTableFileInfo( TCONNECTINGTIME,"TimeEventTable" );
	AddTableFileInfo( THELP,"HelpTable" );
	AddTableFileInfo( TGUIDEPOPUP,"guidepopup" );
	AddTableFileInfo( TREPUTEBENEFIT,"ReputeBenefit" );
	AddTableFileInfo( THELPKEYWORD,"helpkeywordtable" );
	AddTableFileInfo( TINSTANTITEM,"instantitemtable" );
	AddTableFileInfo( TGUILDMARK,"guildmarktable" );
	AddTableFileInfo( TBATTLEGROUNDMODE,"battlegroundmodesetting" );
	AddTableFileInfo( TBATTLEGROUNDRESWAR,"battlegroundresourcewar" );
	AddTableFileInfo( TBATTLEGROUNDSKILLTREE,"battlegroundskilltreetable", "_SkillTableID", false, false );
	AddTableFileInfo( TBATTLEGROUNDSKILLLEVEL,"battlegroundskillleveltable", "_SkillIndex", false, false );
	AddTableFileInfo( TBATTLEGROUNDSKILL,"battlegroundskilltable" );
	AddTableFileInfo( TRESOURCEWARSKIN,"resorcewarskin" );
	AddTableFileInfo( TUNION,"uniontable", "_UnionID", false, false );
	AddTableFileInfo( TGUILDWARREWARD,"guildwarmailtable" );
	AddTableFileInfo( TPLAYERCOMMONLEVEL,"playercommonleveltable" );
	AddTableFileInfo( TGACHAINFO,"gachatable" );
	AddTableFileInfo( TPETLEVEL, "petleveltable" );
	AddTableFileInfo( TPETSKILLLEVEL, "skillleveltable_pet" );
	AddTableFileInfo( TMASTERSYSFEEL, "mastersysfeeltable" );
	AddTableFileInfo( TATTENDANCEEVENT, "everydayeventtable" );
#ifdef PRE_ADD_CASH_AMULET
	AddTableFileInfo( TCASHCHATBALLOONTABLE, "chatboxtable" );
#endif
	AddTableFileInfo( TTRIGGERVARIABLETABLE, "trigervariabletable" );
	AddTableFileInfo( TLOADINGANI, "loadinganitable" );
	AddTableFileInfo( TITEMOPTION, "itemoption" );
	AddTableFileInfo( TVILLAGEALLOWEDSKILL, "villageallowedskilltable", "_villageAllowedSkill", true, false );
	AddTableFileInfo( TCOMBINEDSHOP, "combinedshoptable" );
	AddTableFileInfo( TGLYPHCHARGE, "glyphchargetable" );
#ifdef PRE_FIX_MEMOPT_ENCHANT
	AddTableFileInfo( TENCHANT_STATE, "enchantmaxstatetable" );
	AddTableFileInfo( TENCHANT_NEEDITEM, "enchantneeditemtable" );
#endif
	AddTableFileInfo( TPLAYERCUSTOMEVENTUI, "playercustomeventui" );
	AddTableFileInfo( TGUILDLEVEL, "guildleveltable" );
	AddTableFileInfo( TGUILDWARPOINT, "guildwarpointtable" );

	AddTableFileInfo( TQUEST_LEVELCAP_REWARD, "QuestLevelCabRewardTable" );
	AddTableFileInfo( TGUILDREWARDITEM, "guildrewardtable" );
	AddTableFileInfo( TWING, "wingtable" );
#ifdef PRE_ADD_MODIFY_PLAYER_CANNON
	AddTableFileInfo( TCANNON, "actorcannontable" );
#endif
#ifdef PRE_ADD_COSRANDMIX
	AddTableFileInfo( TCOSMIXINFO, "costumemixinfo" );
	AddTableFileInfo( TCOSRANDOMMIXINFO,"costumemixtable" );
#endif // PRE_ADD_COSRANDMIX
#ifdef PRE_ADD_SALE_COUPON
	AddTableFileInfo( TSALECOUPON, "coupontable" );
#endif // PRE_ADD_SALE_COUPON
#ifdef PRE_ADD_DONATION
	AddTableFileInfo( TCONTRIBUTION, "donationcompensationtable" );
#endif
	AddTableFileInfo( TITEMPERIOD, "itemperiod" );
	AddTableFileInfo( TCONTROLKEY, "controlkey" );
#if defined(PRE_ADD_COUNTRY_BY_COUNTRY_CONTROL)
	AddTableFileInfo( TCONTROLKEY_ESP, "controlkey_ESP" );
	AddTableFileInfo( TCONTROLKEY_FRA, "controlkey_FRA" );
	AddTableFileInfo( TCONTROLKEY_GER, "controlkey_GER" );
#endif	// #if defined(PRE_ADD_COUNTRY_BY_COUNTRY_CONTROL)

#if defined( PRE_ADD_NAMEDITEM_SYSTEM )
	AddTableFileInfo( TNAMEDITEM, "nameditemtable" );
#endif
#ifdef PRE_ADD_EXCHANGE_POTENTIAL
	AddTableFileInfo( TPOTENTIAL_TRANS, "potentialtransfertable" );
#endif		//#ifdef PRE_ADD_EXCHANGE_POTENTIAL
#if defined( PRE_PARTY_DB )
	AddTableFileInfo( TPARTYSORTWEIGHT, "partylistsortweight" );
#endif // #if defined( PRE_PARTY_DB )
	AddTableFileInfo( TCASHERASABLETYPE, "casherasabletypetable" );
	AddTableFileInfo( TPETFOOD, "petfoodtable", NULL, true );
	AddTableFileInfo( TPETCHAT, "petchattable", NULL, true );
#if defined (PRE_ADD_CHAOSCUBE)
	AddTableFileInfo( TCHAOSCUBESTUFF, "chaoscubestufftable" );
	AddTableFileInfo( TCHAOSCUBERESULT, "chaoscuberesulttable" );
#endif
#if defined(PRE_MOD_SELECT_CHAR)
	AddTableFileInfo( TCAMERA, "camtable" );
	AddTableFileInfo( TTITLE, "titletable" );
	AddTableFileInfo( TDEFAULTCREATECOSTUME, "costumepreviewtable" );
#endif // PRE_MOD_SELECT_CHAR
#if defined(PRE_ADD_INSTANT_CASH_BUY)
	AddTableFileInfo( TCASHBUYSHORTCUT, "cashbuyshortcuttable" );
#endif // PRE_ADD_INSTANT_CASH_BUY
#if defined( PRE_WORLDCOMBINE_PARTY )
	AddTableFileInfo( TWORLDCOMBINEPARTY, "nestmissionpartytable" );
#endif
#if defined( PRE_ADD_PCBANG_RENTAL_ITEM )
	AddTableFileInfo( TPCBANGRENTALITEM, "pccaferenttable" );
#endif
#if defined(PRE_ADD_63603)
	AddTableFileInfo( TITEMUSEINFO, "ItemuseInfotable", NULL, false, false );
#endif // PRE_ADD_63603
#if defined(PRE_ADD_EXCHANGE_ENCHANT)
	AddTableFileInfo( TENCHANTTRANSFER, "enchanttransfertable", NULL, false, false );
#endif
#ifdef PRE_ADD_ITEMCAT_TOOLTIP_INFO
	AddTableFileInfo( TITEMCATEGORYDESC,"itemcategorytable" );
#endif

#if defined(PRE_ADD_TOTAL_LEVEL_SKILL)
	AddTableFileInfo( TTOTALLEVELSKILL,"totallevelskill", NULL, false, false );
	AddTableFileInfo( TTOTALLEVELLSKILLSLOT,"totalskillslot" );
#endif // PRE_ADD_TOTAL_LEVEL_SKILL
#if defined(PRE_ADD_WEEKLYEVENT)
	AddTableFileInfo( TWEEKLYEVENT,"weeklyevent", NULL, true );
#endif	// #if defined(PRE_ADD_WEEKLYEVENT)
#if defined( PRE_ADD_PRESET_SKILLTREE )
	AddTableFileInfo( TSKILLTREE_ADVICE,"skilltreeadvicetable" );
#endif		// #if defined( PRE_ADD_PRESET_SKILLTREE )
#if defined(PRE_SPECIALBOX)
	AddTableFileInfo( TKEEPBOXPROVIDEITEM,"keepboxprovideitemtable" );
#endif	// #if defined(PRE_SPECIALBOX)
#if defined( PRE_ADD_SHORTCUT_HELP_DIALOG )
	AddTableFileInfo( THELPSHORTCUT,"helpbutton" );
#endif
#ifdef PRE_ADD_JOINGUILD_SUPPORT
	AddTableFileInfo( TGUILDSUPPORT,"guildmembermaxlv" );
#endif		//#ifdef PRE_ADD_JOINGUILD_SUPPORT
#ifdef PRE_ADD_STAGE_DAMAGE_LIMIT
	AddTableFileInfo( TSTAGEDAMAGELIMIT,"StageDamageLimit" );
#endif
#if defined(PRE_ADD_REMOTE_QUEST)
	AddTableFileInfo( TREMOTEQUEST,"QuestRemoteTable" );
#endif

#ifdef PRE_ADD_ACTIVEMISSION
	AddTableFileInfo( TACTIVEMISSION,"ActiveMissionTable" );
	AddTableFileInfo( TACTIVESET,"ActiveSetTable" );
#endif // PRE_ADD_ACTIVEMISSON

#if defined(PRE_ADD_GETITEM_WITH_RANDOMVALUE)
	AddTableFileInfo( TCHARMCOUNT,"charmcounttable" );
#endif	// #if defined(PRE_ADD_GETITEM_WITH_RANDOMVALUE)

#if defined(PRE_ADD_STAGE_CLEAR_ADD_REWARD)
	AddTableFileInfo( TBONUSDROP,"BonusDropTable" );
#endif	// #if defined(PRE_ADD_STAGE_CLEAR_ADD_REWARD)
#if defined( PRE_WORLDCOMBINE_PVP )
	AddTableFileInfo( TWORLDPVPMISSIONROOM,"worldmissionroomtable" );
#endif
#if defined( PRE_ADD_STAGE_CLEAR_ENCHANT_REWARD)
	AddTableFileInfo( TDROPITEMENCHANT,"DropItemEnchant" );
#endif	// #if defined( PRE_ADD_STAGE_CLEAR_ENCHANT_REWARD)
#if defined( PRE_ALTEIAWORLD_EXPLORE )
	AddTableFileInfo( TALTEIAWORLDMAP,"AlteiaBoardTable" );
	AddTableFileInfo( TALTEIARANK,"alteiaranktable" );
	AddTableFileInfo( TALTEIAWEEKREWARD,"alteiaweekrewardtable" );
	AddTableFileInfo( TALTEIATIME,"alteiatimetable" );
	AddTableFileInfo( TALTEIAPREVIEWREWARD,"alteiapreviewrewardtable" );
#endif // #if defined( PRE_ALTEIAWORLD_EXPLORE )
#if defined( PRE_ADD_STAMPSYSTEM )
	AddTableFileInfo( TSTAMPCHALLENGE,"StampTable" );
#endif // #if defined( PRE_ADD_STAMPSYSTEM )
#if defined( PRE_PVP_GAMBLEROOM )
	AddTableFileInfo( TPVPGAMBLEROOM,"PvPGambleModeRoomTable" );
#endif
#ifdef PRE_ADD_ITEM_GAINTABLE
	AddTableFileInfo(TITEMGAIN, "itemgaintable");
#endif
#if defined( PRE_ADD_STAGE_WEIGHT )
	AddTableFileInfo(TSTAGEWEIGHT, "StageWeightTable", NULL, false, false);
#endif // #if defined( PRE_ADD_STAGE_WEIGHT )
#if defined( PRE_ADD_OVERLAP_SETEFFECT )
	AddTableFileInfo(TOVERLAPSETEFFECT, "SetOverlapTable");
#endif
#ifdef PRE_ADD_LEVELUP_REWARD_NOTIFIER
	AddTableFileInfo(TLEVELUPCHECKBOX, "LevelupBoxCheckTable");
#endif
	AddTableFileInfo(TPVPRANKREWARD, "PvPRankRewardTable");
#if defined(PRE_ADD_CHAT_MISSION)
	AddTableFileInfo(TMISSIONTYPING, "TypingTable");
#endif

	AddTableFileInfo(TEVENTMONSTER, "monstertable_event");

#ifdef PRE_ADD_CASHINVENTAB
	AddTableFileInfo( TCASHINVENTAB, "cashinventab" );
#endif // PRE_ADD_CASHINVENTAB
#if defined(PRE_ADD_TALISMAN_SYSTEM)
	AddTableFileInfo(TTALISMAN, "talismanTable");
	AddTableFileInfo(TTALISMANSLOT, "TalismanSlotTable");
#endif
	// Extended gameplay tables
	AddTableFileInfo(TDROPCONTROL, "DropControlTable");
	AddTableFileInfo(TDWCCREATE, "dwccreatetable");
	AddTableFileInfo(TMONSTERGOLDDROP, "monstergolddroptable");
	AddTableFileInfo(TMISSIONEVENT, "missioneventtable", NULL, true, true);
	AddTableFileInfo(TSUMMONSKILL, "summonskilltable", NULL, true, false);
	AddTableFileInfo(TPVPTEAMBALANCE, "pvpteambalancetable");
	AddTableFileInfo(TINPUTTEXTPOPUP, "InputTextPopUpTable");
	AddTableFileInfo(TDISJOINTORINFO, "disjointorinfotable");
	AddTableFileInfo(THIDDENCASHSHOPREAL, "hiddencashshopreal");
	AddTableFileInfo(THIDDENCASHSHOPCONCHECK, "hiddencashshopconcheck");
	AddTableFileInfo(TNPCCHATGUIDEUISIZE, "npcchatguideuisize");
	AddTableFileInfo(TQUESTRECYCLE, "QuestRecycleTable");
	AddTableFileInfo(TITEMNOTICE, "itemnoticetable");
}

void CDnTableDB::AddTableFileInfo( TableEnum Index, const char* strFileName, const char* strGenerationLabel, bool bUseGame, bool bDontUseItemIDData )
{
	stFileInfo fileInfo;
	fileInfo.m_strFileName = strFileName;
	if( strGenerationLabel != NULL )
		fileInfo.m_strGenerationLabel = strGenerationLabel;

	fileInfo.m_bUseGame = bUseGame;
#ifdef _CLIENT
	fileInfo.m_bDontUseItemIDData = bDontUseItemIDData;
#else // _CLIENT
	fileInfo.m_bDontUseItemIDData = false;
#endif // _CLIENT

	m_MapTableFileInfo.insert( std::map<TableEnum, stFileInfo>::value_type( Index, fileInfo ) );
}

void CDnTableDB::AddTableByIndex( TableEnum eTableIndex )
{
	std::map<TableEnum, DNTableFileFormat*>::iterator iter = m_MapTables.find( eTableIndex );

	if( iter == m_MapTables.end() )
	{
		std::map<TableEnum, stFileInfo>::iterator iter2 = m_MapTableFileInfo.find( eTableIndex );
		if( iter2 != m_MapTableFileInfo.end() )
		{
			stFileInfo fileInfo = iter2->second;
			AddTable( iter2->first, LoadTable( fileInfo.m_strFileName.c_str(), fileInfo.m_strGenerationLabel, fileInfo.m_bDontUseItemIDData ) );
			if( !fileInfo.m_bUseGame )
				SetTableAutoUnloadByIndex( iter2->first, 40.0f );
		}
	}
}

void CDnTableDB::SetTableAutoUnloadForGame( bool bTableUseInGame )
{
	m_bTableUseInGame = bTableUseInGame;

	if( m_bTableUseInGame )
	{
		float fHoldingTime = 0.0f;
		std::map<TableEnum, stFileInfo>::iterator iter = m_MapTableFileInfo.begin();
		for( ; iter != m_MapTableFileInfo.end(); iter++ )
		{
			if( !iter->second.m_bUseGame )
			{
				SetTableAutoUnloadByIndex( iter->first, fHoldingTime );
				fHoldingTime += 0.1f;
			}
		}
	}
}

void CDnTableDB::SetTableAutoUnloadByIndex( TableEnum eTableIndex, float fHoldingTime )
{
	std::map<TableEnum, DNTableFileFormat*>::iterator iter = m_MapTables.find( eTableIndex );

	if( iter != m_MapTables.end() )
	{
		DNTableFileFormat* pDnTableFile = iter->second;
		if( pDnTableFile )
		{
			pDnTableFile->m_bAutoUnload = true;
			pDnTableFile->m_fHoldingTime = fHoldingTime;
		}
	}
}

#ifdef _DEBUG
float g_fGetTableTime = 0.0f;
int g_nTableSize = 0;
#endif // _DEBUG

void CDnTableDB::Process( float fElapsedTime )
{
	if( m_bTableUseInGame )
	{
		std::map<TableEnum, DNTableFileFormat*>::iterator iter = m_MapTables.begin();
		for( ; iter != m_MapTables.end(); iter++ )
		{
			DNTableFileFormat* pDnTableFile = iter->second;
			if( pDnTableFile && pDnTableFile->m_bAutoUnload && pDnTableFile->m_fHoldingTime >= 0.0f )
			{
				pDnTableFile->m_fHoldingTime -= fElapsedTime;
				if( pDnTableFile->m_fHoldingTime <= 0.0f )
				{
					pDnTableFile->m_fHoldingTime = 0.0f;
					UnloadTable( iter->first );
					break;
				}
			}
		}
#ifdef _DEBUG
		if( g_fGetTableTime < 0.0f )
		{
			g_nTableSize = m_MapTables.size();
			g_fGetTableTime = 1.0f;
		}
		else
			g_fGetTableTime -= fElapsedTime;
#endif // _DEBUG
	}
}
#endif // PRE_ADD_REDUCE_TABLE_RESOURCE

bool CDnTableDB::Initialize( )
{
	try {
#ifdef PRE_ADD_REDUCE_TABLE_RESOURCE
		SetTableFileInfo();

#ifdef _CLIENT
		AddTableByIndex( TACTOR );
		AddTableByIndex( TPARTS );
		AddTableByIndex( TWEAPON );
		AddTableByIndex( TENCHANT );
		AddTableByIndex( TPOTENTIAL );
		AddTableByIndex( TITEM );
		AddTableByIndex( TSKILL );
		AddTableByIndex( TSKILLLEVEL );

		AddTableByIndex( TPLAYERWEIGHT );
		AddTableByIndex( TGLOBALWEIGHTINT );
		AddTableByIndex( TPLAYERLEVEL );
		AddTableByIndex( TEQUALLEVEL );
		AddTableByIndex( TEQUALBASIS );
		AddTableByIndex( TEQUALEXPONENT );
		AddTableByIndex( TFATIGUEADJUST );
#ifdef PRE_ADD_VIP
		AddTableByIndex( TVIPGRADE );
#endif // PRE_ADD_VIP
#ifdef PRE_ADD_ITEMCAT_TOOLTIP_INFO
		AddTableByIndex( TITEMCATEGORYDESC );
#endif // PRE_ADD_ITEMCAT_TOOLTIP_INFO
		AddTableByIndex( TGUILDREWARDITEM );
#else // _CLIENT
		std::map<TableEnum, stFileInfo>::iterator iter = m_MapTableFileInfo.begin();
		for( ; iter != m_MapTableFileInfo.end(); iter++ )
		{
			AddTableByIndex( iter->first );
		}
#endif // _CLIENT
#else // PRE_ADD_REDUCE_TABLE_RESOURCE
		AddTable( TACTOR, LoadTable( "ActorTable", "_StaticName" ) );
		AddTable( TPARTS, LoadTable( "PartsTable", "_SetItemID" ) );
		AddTable( TWEAPON, LoadTable( "WeaponTable", "_SetItemID" ) );
		AddTable( TTILE, LoadTable( "TileTable", "_Name" ) );
		AddTable( TENVIEFFECT, LoadTable( "EnviEffectTable" ) );
		AddTable( TMAP, LoadTable( "MapTable", "_EnterConditionTableID;_MapSubType" ) );
		AddTable( TPLAYERLEVEL, LoadTable( "PlayerLevelTable" ) );
		AddTable( TMONSTER, LoadTable( "MonsterTable", "_ActorTableID" ) );
		AddTable( TMONSTERCOMPOUND, LoadTable( "MonsterCompoundTable" ) );
		AddTable( TITEM, LoadTable( "ItemTable" ) );
		AddTable( TPROP, LoadTable( "PropTable", "_Name" ) );
		AddTable( TJOB, LoadTable( "JobTable", "_JobNumber;_BaseClass" ) );
		AddTable( TSKILL, LoadTable( "SkillTable" ) );
		AddTable( TITEMDROP, LoadTable( "ItemDropTable" ) );
		AddTable( TDEFAULTCREATE, LoadTable( "DefaultCreateTable" ) );
		AddTable( TNPC, LoadTable( "NpcTable", "_ActorIndex" ) );
		AddTable( TQUEST, LoadTable( "QuestTable" ) );
		AddTable( TPLAYERWEIGHT, LoadTable( "PlayerWeightTable" ) );
		AddTable( TTALKPARAM, LoadTable( "TalkParamTable" ) );
		AddTable( TSKILLLEVEL, LoadTable( "SkillLevelTable", "_SkillIndex" ) );
		AddTable( TMAPGATE, LoadTable( "MapGateTable", "_PermitPlayerLevel;_MapIndex" ) );
		AddTable( TSOCIAL, LoadTable( "ActorFaceTable" ) );
		AddTable( TMONSTERSKILL, LoadTable( "MonsterSkillTable" ) );
		AddTable( TSHOP, LoadTable( "ShopTable" ) );
		AddTable( TMONSTERPARTS, LoadTable( "MonsterPartsTable" ) );
		AddTable( TSKILLSHOP, LoadTable( "SkillShopTable" ) );
		AddTable( TSUMMONPROP, LoadTable( "SummonPropTable" ) );
		AddTable( TDUNGEONENTER, LoadTable( "StageEnterTable", "_LvlMin;_AbyssLvlMin;_AbyssQuestID;_RecommendEasyLevel;_RecommendNormalLevel;_RecommendHardLevel;_RecommendVeryHardLevel;_RecommendNightmareLevel" ) );
		AddTable( TDUNGEONCLEAR, LoadTable( "StageClearTable" ) );
		AddTable( TCLEARSTANDARD, LoadTable( "ClearStandard" ) );
		AddTable( TSYMBOLITEM, LoadTable( "SymbolItemTable" ) );
		AddTable( TCUTSCENE, LoadTable( "CutSceneTable" ) );
		AddTable( TPLATE, LoadTable( "PlateTable" ) );
		AddTable( TITEMCOMPOUND, LoadTable( "ItemCompoundTable", "_CompoundGroupID;_CompoundPotentialID") );
		AddTable( TSTATEEFFECT, LoadTable( "StateEffectTable" ) );
		AddTable( TDUNGEONMAP, LoadTable( "StageSetTable" ) );
		AddTable( TTODAYTIP, LoadTable( "TodayToolTipTable" ) );
		AddTable( TFILE, LoadTable( "FileTable" ) );
		AddTable( TQUESTREWARD, LoadTable( "QuestRewardTable" ) );
		AddTable( TMONSTERWEIGHT, LoadTable( "MonsterWeightTable", "_MonsterTableIndex" ) );
		AddTable( TQUESTCHAPTER, LoadTable( "QuestChapterTable" ) );
		AddTable( TEQUIPMATCHACTION, LoadTable( "EquipMatchActionTable", "_Class" ) );
		AddTable( TFACE, LoadTable( "FaceAnimationTable" ) );
		AddTable( TREBIRTHCOIN, LoadTable( "CoinTable" ) );
		AddTable( TPVPMAP, LoadTable( "PvPMapTable" ) );
		AddTable( TPVPGAMEMODE, LoadTable( "PvPGameModeTable" ) );
		AddTable( TPVPGAMESTARTCONDITION, LoadTable( "PvPGameStartConditionTable" ) );
		AddTable( TCP, LoadTable( "CPTable" ) );
		AddTable( TGAMEROOMNAME, LoadTable( "PvPDefaultRoomNameTable" ) );
		AddTable( TGLOBALWEIGHT, LoadTable( "GlobalWeightTable" ) );
		AddTable( TGLOBALWEIGHTINT, LoadTable( "GlobalWeightIntTable" ) );
#ifdef PRE_FIX_MEMOPT_ENCHANT
		AddTable( TENCHANT_MAIN, LoadTable( "enchantmaintable", "_EnchantID" ) );
#else
		AddTable( TENCHANT, LoadTable( "EnchantTable", "_EnchantID" ) );
#endif
		AddTable( TPOTENTIAL, LoadTable( "PotentialTable", "_PotentialID" ) );
		AddTable( TSETITEM, LoadTable( "SetItemTable" ) );
		AddTable( TEQUALLEVEL, LoadTable( "EqualLevelTable", "_Mode" ) );
		AddTable( TMISSION, LoadTable( "MissionTable" ) );
		AddTable( TAPPELLATION, LoadTable( "AppellationTable" ) );
		AddTable( TPVPRANK, LoadTable( "PvPRankTable" ) );
		AddTable( TPVPWINCONDITION, LoadTable( "PvPGameStartConditionTable" ) );
		AddTable( TITEMCOMPOUNDSHOP, LoadTable( "ItemCompoundShopTable", "_ListID" ) );
		AddTable( TEQUALBASIS, LoadTable( "EqualBasisTable" ) );
		AddTable( TDAILYMISSION, LoadTable( "DailyMissionTable" ) );
		AddTable( TSCHEDULE, LoadTable( "ScheduleTable" ) );
		AddTable( TWORLDMAPUI, LoadTable( "WorldMapUI" ) );
		AddTable( TTAX, LoadTable( "TaxTable" ) );
		AddTable( TMISSIONSCHEDULE, LoadTable( "MissionScheduleTable" ) );
		AddTable( TRADIOMSG, LoadTable( "RadioMsgTable", "_MacroNum" ) );
		AddTable( TGESTURE, LoadTable( "GestureTable" ) );
		AddTable( TMONSTERGROUP, LoadTable( "MonsterGroupTable" ) );
		AddTable( TMONSTERSET, LoadTable( "MonsterSetTable", "_SetNo" ) );
		AddTable( TSKILLTREE, LoadTable( "SkillTreeTable", "_SkillTableID" ) );
		AddTable( TITEMDROPGROUP, LoadTable( "ItemDropGroupTable", "_DropGroupID" ) );
		AddTable( TGUILDUSERCAP, LoadTable( "GuildUserCapTable" ) );
		AddTable( TDLMAP, LoadTable( "DLMapTable" ) );
		AddTable( TDLDUNGEONCLEAR, LoadTable( "DLStageClearTable" ) );
		AddTable( TCASHCOMMODITY, LoadTable( "CashCommodity" ) );
		AddTable( TCASHGOODSINFO, LoadTable( "CashGoodsInfoTable" ) );
		AddTable( TCASHPACKAGE, LoadTable( "CashPackageTable" ) );
		AddTable( TCASHLIMIT, LoadTable( "CashLimitTable" ) );
		AddTable( TWEATHER, LoadTable( "WeatherTable" ) );
		AddTable( TPCCAFE, LoadTable( "PCCafeTable" ) );
		AddTable( TFATIGUEADJUST, LoadTable( "FatigueAdjustmentTable" ) );
		AddTable( TCASHPRESENTREACT, LoadTable( "CashPresentReactTable" ) );
#ifdef PRE_ADD_GACHA_JAPAN
		AddTable( TGACHA_JP, LoadTable( "GachaJP", "_GachaNum" ) );
		AddTable( TGACHASETITEM_JP, LoadTable( "GachaSetItemJP", "_GachaNum" ) );
#endif
		AddTable( TSEALCOUNT, LoadTable( "SealCountTable" ) );
		AddTable( TLEVELUPEVENT, LoadTable( "LevelupEventTable" ) );
#if defined(PRE_ADD_VIP)
		AddTable( TVIP, LoadTable( "VIPTable" ) );
		AddTable( TVIPGRADE, LoadTable( "VIPGradeTable" ) );
#endif	// #if defined(PRE_ADD_VIP)
		AddTable( TCASHTAB, LoadTable("CashTabTable" ) );
		AddTable( TFAIRY, LoadTable("FairyTable" ) );
		AddTable( TCOSMIX, LoadTable("CashItemCompoundTable" ) );
		AddTable( TCOSMIXABILITY, LoadTable("CashCompoundInfoTable" ) );
		AddTable( TGLYPHSKILL, LoadTable("GlyphSkillTable" ) );
		AddTable( TGLYPHSLOT, LoadTable("GlyphSlotTable" ) );
		AddTable( TLEVELPROMO, LoadTable("WelfareTable" ) );

#if defined( PRE_ADD_NPC_REPUTATION_SYSTEM )
		AddTable( TREPUTE, LoadTable("ReputeTable", "_NpcID") );
		AddTable( TPRESENT, LoadTable("PresentTable" ) );
		AddTable( TNPCREACTION, LoadTable("NpcReactionTable" ) );
		AddTable( TSTOREBENEFIT, LoadTable("storebenefittable", "_NpcID") );
#endif // #if defined( PRE_ADD_NPC_REPUTATION_SYSTEM )
		AddTable( TMAIL, LoadTable("MailTable") );
		AddTable( TMODDLG, LoadTable("CustomDialogTable") );
		AddTable( TMASTERSYSTEM_DECREASE, LoadTable("MasterSysDecreaseTable") );
		AddTable( TMASTERSYSTEM_GAIN, LoadTable("MasterSysGainTable") );
		AddTable( TVEHICLE, LoadTable("VehicleTable", "_VehicleClassID"));
		AddTable( TVEHICLEPARTS, LoadTable("VehiclePartsTable"));
		AddTable( TPOTENTIALJEWEL, LoadTable("PotentialJewelTable") );
		AddTable( TEVENTPOPUP, LoadTable("PopUpTable") );
		AddTable( TCHARMITEM, LoadTable("CharmItemTable") );
		AddTable( TCHARMITEMKEY, LoadTable("CharmItemkeyTable") );
#if defined( PRE_ADD_SECONDARY_SKILL )
		AddTable( TSecondarySkill, LoadTable("secondaryskilltable") );
		AddTable( TSecondarySkillLevel, LoadTable("secondaryskillleveltable") );
		AddTable( TSecondarySkillRecipe, LoadTable("makeitemrecipetable") );
#endif // #if defined( PRE_ADD_SECONDARY_SKILL )
		AddTable( TSKILLVIDEO, LoadTable("SkillVideoTable") );
		AddTable( TGUILDWAREHOUSE, LoadTable("GuildWareHouse") );
		AddTable( TFARMCULTIVATE, LoadTable("farmcultivatetable") );
		AddTable( TFARMSKIN, LoadTable("farmskintable") );
		AddTable( TFISHING, LoadTable("fishingtable") );
		AddTable( TFISHINGPOINT, LoadTable("fishingpointtable") );
		AddTable( TFISHINGAREA, LoadTable("fishingareatable") );
		AddTable( TFISHINGROD, LoadTable("fishingrodtable") );
		AddTable( TQUESTPERIOD, LoadTable("QuestDailyTable") );
		AddTable( TGLOBALEVENTQUEST, LoadTable("globalevent") );
		AddTable( TSTAFFROLL, LoadTable("StaffRollTable") );
		AddTable( TCOLLECTIONBOOK, LoadTable("CollectionBookTable") );
		AddTable( TEXCHANGETRADE, LoadTable("Exchange") );
		AddTable( TSKILLBUBBLE, LoadTable("SkillBubbleTable") );
		AddTable( TSKILLBUBBLEDEFINE, LoadTable("SkillBubbleDefine") );
		AddTable( TENCHANTJEWEL, LoadTable("EnchantJewelTable") );
		AddTable( TPVPMISSIONROOM, LoadTable("PvPMissionRoomTable") );
		AddTable( TEQUALEXPONENT, LoadTable("EqualExponentValue") );
#ifdef PRE_MOD_ITEM_COMPOUND_DIALOG_RENEWAL
		AddTable( TITEMCOMPOUNDGROUP, LoadTable("ItemCompoundGroupTable", "_ListID;_MainCategory;_JobClass;_Level" ) );
#else
		AddTable( TITEMCOMPOUNDGROUP, LoadTable("ItemCompoundGroupTable", "_ListID" ) );
#endif
		AddTable( TEXPADJUST, LoadTable("expadjustmenttable" ) );
		AddTable( TSTAGEREWARD, LoadTable("StageRewardTable" ) );
		AddTable( TMONSTERSKILL_TRANS, LoadTable("monsterskilltable_transform"));
		AddTable( TMONSTER_TRANS, LoadTable("monstertable_transform"));
		AddTable( TMONSTERGROUP_TRANS, LoadTable("monstergrouptable_transform"));
		AddTable( TGHOULMODE_CONDITION, LoadTable("ghoulconditiontable"));
		AddTable( TPVPGAMEMODESKILLSETTING, LoadTable("pvpgamemodeskillsetting"));
		AddTable( TMONSTER_TRANS_RADIOMSG, LoadTable("monsterradiomsgtable"));
		AddTable( TMONSTER_TRANS_RADIOBASE, LoadTable("monsterradiochangetable"));
		AddTable( TCONNECTINGTIME, LoadTable("TimeEventTable") );
		AddTable( THELP, LoadTable("HelpTable") );
		AddTable( TGUIDEPOPUP, LoadTable("guidepopup") );
		AddTable( TREPUTEBENEFIT, LoadTable("ReputeBenefit") );
		AddTable( THELPKEYWORD, LoadTable("helpkeywordtable") );
		AddTable( TINSTANTITEM, LoadTable("instantitemtable") );
		AddTable( TGUILDMARK, LoadTable("guildmarktable") );
		AddTable( TBATTLEGROUNDMODE, LoadTable("battlegroundmodesetting") );
		AddTable( TBATTLEGROUNDRESWAR, LoadTable("battlegroundresourcewar") );
		AddTable( TBATTLEGROUNDSKILLTREE, LoadTable("battlegroundskilltreetable", "_SkillTableID" ) );
		AddTable( TBATTLEGROUNDSKILLLEVEL, LoadTable("battlegroundskillleveltable", "_SkillIndex") );
		AddTable( TBATTLEGROUNDSKILL, LoadTable("battlegroundskilltable") );
		AddTable( TRESOURCEWARSKIN, LoadTable("resorcewarskin") );
		AddTable( TUNION, LoadTable("uniontable", "_UnionID") );
		AddTable( TGUILDWARREWARD, LoadTable("guildwarmailtable") );
		AddTable( TPLAYERCOMMONLEVEL, LoadTable("playercommonleveltable") );
		AddTable( TGACHAINFO, LoadTable("gachatable") );
		AddTable( TPETLEVEL, LoadTable( "petleveltable" ) );
		AddTable( TPETSKILLLEVEL, LoadTable( "skillleveltable_pet" ) );
		AddTable( TMASTERSYSFEEL, LoadTable( "mastersysfeeltable" ) );
		AddTable( TATTENDANCEEVENT, LoadTable( "everydayeventtable" ) );
#ifdef PRE_ADD_CASH_AMULET
		AddTable( TCASHCHATBALLOONTABLE, LoadTable( "chatboxtable" ) );
#endif
		AddTable( TTRIGGERVARIABLETABLE, LoadTable( "trigervariabletable" ) );
		AddTable( TLOADINGANI, LoadTable( "loadinganitable" ) );
		AddTable( TITEMOPTION, LoadTable( "itemoption" ) );
		AddTable( TVILLAGEALLOWEDSKILL, LoadTable( "villageallowedskilltable", "_villageAllowedSkill" ) );
		AddTable( TCOMBINEDSHOP, LoadTable( "combinedshoptable" ) );
		AddTable( TGLYPHCHARGE, LoadTable( "glyphchargetable" ) );
#ifdef PRE_FIX_MEMOPT_ENCHANT
		AddTable( TENCHANT_STATE, LoadTable( "enchantmaxstatetable" ) );
		AddTable( TENCHANT_NEEDITEM, LoadTable( "enchantneeditemtable" ) );
#endif
		AddTable( TPLAYERCUSTOMEVENTUI, LoadTable( "playercustomeventui" ) );
		AddTable( TGUILDLEVEL, LoadTable( "guildleveltable" ) );
		AddTable( TGUILDWARPOINT, LoadTable( "guildwarpointtable" ) );

		AddTable( TQUEST_LEVELCAP_REWARD, LoadTable( "QuestLevelCabRewardTable") );
		AddTable( TGUILDREWARDITEM, LoadTable( "guildrewardtable") );		
		AddTable( TWING, LoadTable( "wingtable" ) );
#ifdef PRE_ADD_MODIFY_PLAYER_CANNON
		AddTable( TCANNON, LoadTable( "actorcannontable" ) );
#endif
#ifdef PRE_ADD_COSRANDMIX
		AddTable( TCOSMIXINFO, LoadTable( "costumemixinfo" ) );
		AddTable( TCOSRANDOMMIXINFO, LoadTable("costumemixtable") );
#endif // PRE_ADD_COSRANDMIX
#ifdef PRE_ADD_SALE_COUPON
		AddTable( TSALECOUPON, LoadTable( "coupontable" ) );
#endif // PRE_ADD_SALE_COUPON
#ifdef PRE_ADD_DONATION
		AddTable( TCONTRIBUTION, LoadTable( "donationcompensationtable" ) );
#endif
		AddTable( TITEMPERIOD, LoadTable( "itemperiod" ) );
		AddTable( TCONTROLKEY, LoadTable( "controlkey" ) );
#if defined(PRE_ADD_COUNTRY_BY_COUNTRY_CONTROL)
		AddTable( TCONTROLKEY_ESP, LoadTable( "controlkey_ESP" ) );
		AddTable( TCONTROLKEY_FRA, LoadTable( "controlkey_FRA" ) );
		AddTable( TCONTROLKEY_GER, LoadTable( "controlkey_GER" ) );
#endif	// #if defined(PRE_ADD_COUNTRY_BY_COUNTRY_CONTROL)

#if defined( PRE_ADD_NAMEDITEM_SYSTEM )
		AddTable( TNAMEDITEM, LoadTable( "nameditemtable" ) );
#endif
#ifdef PRE_ADD_EXCHANGE_POTENTIAL
		AddTable( TPOTENTIAL_TRANS, LoadTable( "potentialtransfertable" ) );
#endif		//#ifdef PRE_ADD_EXCHANGE_POTENTIAL
#if defined( PRE_PARTY_DB )
		AddTable( TPARTYSORTWEIGHT, LoadTable( "partylistsortweight" ) );
#endif // #if defined( PRE_PARTY_DB )
		AddTable( TCASHERASABLETYPE, LoadTable( "casherasabletypetable" ) );
		AddTable( TPETFOOD, LoadTable( "petfoodtable") );
		AddTable( TPETCHAT, LoadTable( "petchattable") );
#if defined (PRE_ADD_CHAOSCUBE)
		AddTable( TCHAOSCUBESTUFF, LoadTable( "chaoscubestufftable") );
		AddTable( TCHAOSCUBERESULT, LoadTable( "chaoscuberesulttable") );
#endif
#if defined(PRE_MOD_SELECT_CHAR)
		AddTable( TCAMERA, LoadTable( "camtable") );
		AddTable( TTITLE, LoadTable( "titletable") );
		AddTable( TDEFAULTCREATECOSTUME, LoadTable( "costumepreviewtable") );
#endif // PRE_MOD_SELECT_CHAR
#if defined(PRE_ADD_INSTANT_CASH_BUY)
		AddTable( TCASHBUYSHORTCUT, LoadTable( "cashbuyshortcuttable") );
#endif // PRE_ADD_INSTANT_CASH_BUY
#if defined( PRE_WORLDCOMBINE_PARTY )
		AddTable( TWORLDCOMBINEPARTY, LoadTable( "nestmissionpartytable") );
#endif
#if defined( PRE_ADD_PCBANG_RENTAL_ITEM )
		AddTable( TPCBANGRENTALITEM, LoadTable( "pccaferenttable"));
#endif
#if defined(PRE_ADD_63603)
		AddTable( TITEMUSEINFO, LoadTable( "ItemuseInfotable") );
#endif // PRE_ADD_63603
#if defined(PRE_ADD_EXCHANGE_ENCHANT)
		AddTable( TENCHANTTRANSFER, LoadTable( "enchanttransfertable"));
#endif
#ifdef PRE_ADD_ITEMCAT_TOOLTIP_INFO
		AddTable(TITEMCATEGORYDESC, LoadTable("itemcategorytable"));
#endif

#if defined(PRE_ADD_TOTAL_LEVEL_SKILL)
		AddTable(TTOTALLEVELSKILL, LoadTable("totallevelskill"));
		AddTable(TTOTALLEVELLSKILLSLOT, LoadTable("totalskillslot"));
#endif // PRE_ADD_TOTAL_LEVEL_SKILL
#if defined(PRE_ADD_WEEKLYEVENT)
		AddTable(TWEEKLYEVENT, LoadTable("weeklyevent"));
#endif	// #if defined(PRE_ADD_WEEKLYEVENT)
#if defined( PRE_ADD_PRESET_SKILLTREE )
		AddTable(TSKILLTREE_ADVICE, LoadTable("skilltreeadvicetable"));
#endif		// #if defined( PRE_ADD_PRESET_SKILLTREE )
#if defined(PRE_SPECIALBOX)
		AddTable(TKEEPBOXPROVIDEITEM, LoadTable("keepboxprovideitemtable"));
#endif	// #if defined(PRE_SPECIALBOX)
#if defined( PRE_ADD_SHORTCUT_HELP_DIALOG )
		AddTable(THELPSHORTCUT, LoadTable("helpbutton"));
#endif
#ifdef PRE_ADD_JOINGUILD_SUPPORT
		AddTable(TGUILDSUPPORT, LoadTable("guildmembermaxlv"));
#endif		//#ifdef PRE_ADD_JOINGUILD_SUPPORT
#ifdef PRE_ADD_STAGE_DAMAGE_LIMIT
		AddTable(TSTAGEDAMAGELIMIT, LoadTable("StageDamageLimit"));
#endif
#if defined(PRE_ADD_REMOTE_QUEST)
		AddTable(TREMOTEQUEST, LoadTable("QuestRemoteTable"));
#endif

#ifdef PRE_ADD_ACTIVEMISSION
		AddTable(TACTIVEMISSION, LoadTable("ActiveMissionTable"));
		AddTable(TACTIVESET, LoadTable("ActiveSetTable"));
#endif // PRE_ADD_ACTIVEMISSON

#if defined(PRE_ADD_GETITEM_WITH_RANDOMVALUE)
		AddTable(TCHARMCOUNT, LoadTable("charmcounttable"));
#endif	// #if defined(PRE_ADD_GETITEM_WITH_RANDOMVALUE)

#if defined(PRE_ADD_STAGE_CLEAR_ADD_REWARD)
		AddTable(TBONUSDROP, LoadTable("BonusDropTable"));
#endif	// #if defined(PRE_ADD_STAGE_CLEAR_ADD_REWARD)
#if defined( PRE_WORLDCOMBINE_PVP )
		AddTable(TWORLDPVPMISSIONROOM, LoadTable("worldmissionroomtable"));
#endif
#if defined( PRE_ADD_STAGE_CLEAR_ENCHANT_REWARD)
		AddTable(TDROPITEMENCHANT, LoadTable("DropItemEnchant"));
#endif	// #if defined( PRE_ADD_STAGE_CLEAR_ENCHANT_REWARD)
#if defined( PRE_ALTEIAWORLD_EXPLORE )
		AddTable(TALTEIAWORLDMAP, LoadTable("AlteiaBoardTable"));
		AddTable(TALTEIARANK, LoadTable("alteiaranktable"));
		AddTable(TALTEIAWEEKREWARD, LoadTable("alteiaweekrewardtable"));
		AddTable(TALTEIATIME, LoadTable("alteiatimetable"));
#endif // #if defined( PRE_ALTEIAWORLD_EXPLORE )
#if defined( PRE_ADD_STAMPSYSTEM )
		AddTable(TSTAMPCHALLENGE, LoadTable("StampTable"));
#endif // #if defined( PRE_ADD_STAMPSYSTEM )
#if defined( PRE_PVP_GAMBLEROOM )
		AddTable(TPVPGAMBLEROOM, LoadTable("PvPGambleModeRoomTable"));
#endif
#if defined(PRE_ADD_CHAT_MISSION)
		AddTable(TMISSIONTYPING, LoadTable("TypingTable"));
#endif
#if defined( PRE_ADD_STAGE_WEIGHT )
		AddTable(TSTAGEWEIGHT, LoadTable("StageWeightTable"));
#endif // #if defined( PRE_ADD_STAGE_WEIGHT )
#if defined( PRE_ADD_OVERLAP_SETEFFECT )
		AddTable(TOVERLAPSETEFFECT, LoadTable("SetOverlapTable"));
#endif
		AddTable(TPVPRANKREWARD, LoadTable("PvPRankRewardTable"));
		AddTable(TEVENTMONSTER, LoadTable("monstertable_event"));
#ifdef PRE_ADD_CASHINVENTAB
		AddTable(TCASHINVENTAB, LoadTable("cashinventab"));
#endif
#if defined(PRE_ADD_TALISMAN_SYSTEM)
		AddTable(TTALISMAN, LoadTable("talismanTable"));
		AddTable(TTALISMANSLOT, LoadTable("TalismanSlotTable"));
#endif
		AddTable(TDROPCONTROL, LoadTable("DropControlTable"));
		AddTable(TDWCCREATE, LoadTable("dwccreatetable"));
		AddTable(TMONSTERGOLDDROP, LoadTable("monstergolddroptable"));
		AddTable(TMISSIONEVENT, LoadTable("missioneventtable"));
		AddTable(TSUMMONSKILL, LoadTable("summonskilltable"));
		AddTable(TPVPTEAMBALANCE, LoadTable("pvpteambalancetable"));
		AddTable(TINPUTTEXTPOPUP, LoadTable("InputTextPopUpTable"));
		AddTable(TDISJOINTORINFO, LoadTable("disjointorinfotable"));
		AddTable(THIDDENCASHSHOPREAL, LoadTable("hiddencashshopreal"));
		AddTable(THIDDENCASHSHOPCONCHECK, LoadTable("hiddencashshopconcheck"));
		AddTable(TNPCCHATGUIDEUISIZE, LoadTable("npcchatguideuisize"));
		AddTable(TQUESTRECYCLE, LoadTable("QuestRecycleTable"));
		AddTable(TITEMNOTICE, LoadTable("itemnoticetable"));
#endif // PRE_ADD_REDUCE_TABLE_RESOURCE
	}
	catch(const char* ex)
	{
		wchar_t filename[256] = {0};
		MultiByteToWideChar(CP_ACP, 0, ex, -1, filename, 256);

		wchar_t msg[512] = {0};
		::wsprintf(msg, L"Resource not found.\n\'%s\'", filename);
		g_Log.Log( LogType::_FILELOG, L"Resource not found. \'%s\'\r\n", filename);
		MessageBox( NULL, msg, L"Critical Error", MB_ICONEXCLAMATION );
		return false;
	}
	catch(...) {
		MessageBox( NULL, L"Resource not found.\nPlease Re-install.", L"Critical Error", MB_OK );
		return false;
	}

// #if defined (_CLIENT) && defined (_TEST_CODE_KAL)
// 	g_ProfileMemCheck.OutputResult(NULL, _WriteLogA);
// #endif

	// Create a class for values preloaded to use directly without referencing the table.
	CPlayerWeightTable::CreateInstance();
	CGlobalWeightTable::CreateInstance();
	CGlobalWeightIntTable::CreateInstance();
	CPlayerLevelTable::CreateInstance();
	CEqualLevelTable::CreateInstance();
	CEqualBasisTable::CreateInstance();
	CEqualExponentTable::CreateInstance();
	CFatigueAdjustmentTable::CreateInstance();
#ifdef PRE_ADD_VIP
	CVIPGradeTable::CreateInstance();
#endif
#ifdef PRE_ADD_ITEMCAT_TOOLTIP_INFO
	CItemCategoryInfo::CreateInstance();
#endif
	CGuildRewardTable::CreateInstance();


	UnloadTable(TPLAYERWEIGHT);
	UnloadTable(TGLOBALWEIGHT);
	UnloadTable(TGLOBALWEIGHTINT);
	UnloadTable(TPLAYERLEVEL);
	UnloadTable(TEQUALLEVEL);
	UnloadTable(TEQUALBASIS);
	UnloadTable(TEQUALEXPONENT);
	UnloadTable(TFATIGUEADJUST);
#ifdef PRE_ADD_VIP
	UnloadTable(TVIPGRADE);
#endif
#ifdef PRE_ADD_ITEMCAT_TOOLTIP_INFO
	UnloadTable(TITEMCATEGORYDESC);
#endif // PRE_ADD_ITEMCAT_TOOLTIP_INFO
	UnloadTable(TGUILDREWARDITEM);

	return true;
}

void CDnTableDB::Finalize()
{
	if( CPlayerWeightTable::GetInstancePtr() )
		CPlayerWeightTable::DestroyInstance();
	if( CGlobalWeightTable::GetInstancePtr() )
		CGlobalWeightTable::DestroyInstance();
	if( CGlobalWeightIntTable::GetInstancePtr() )
		CGlobalWeightIntTable::DestroyInstance();
	if( CPlayerLevelTable::GetInstancePtr() )
		CPlayerLevelTable::DestroyInstance();
	if( CEqualLevelTable::GetInstancePtr() )
		CEqualLevelTable::DestroyInstance();
	if( CEqualBasisTable::GetInstancePtr() )
		CEqualBasisTable::DestroyInstance();
	if( CEqualExponentTable::GetInstancePtr() )
		CEqualExponentTable::DestroyInstance();
	if( CFatigueAdjustmentTable::GetInstancePtr() )
		CFatigueAdjustmentTable::DestroyInstance();
#ifdef PRE_ADD_VIP
	if(CVIPGradeTable::GetInstancePtr())
		CVIPGradeTable::DestroyInstance();
#endif

	if (CGuildRewardTable::GetInstancePtr())
		CGuildRewardTable::DestroyInstance();

#ifdef PRE_ADD_ITEMCAT_TOOLTIP_INFO
	if (CItemCategoryInfo::GetInstancePtr())
		CItemCategoryInfo::DestroyInstance();
#endif

	for (const std::map<TableEnum, DNTableFileFormat*>::value_type& v : m_MapTables)
	{
		delete v.second;
	}
	m_MapTables.clear();
#ifdef PRE_ADD_REDUCE_TABLE_RESOURCE
	m_MapTableFileInfo.clear();
#endif // PRE_ADD_REDUCE_TABLE_RESOURCE
}

#ifdef PRE_ADD_REDUCE_TABLE_RESOURCE
DNTableFileFormat *CDnTableDB::LoadTable( const char *szFileName, std::string strGenerationLabel/*=std::string()*/, bool bDontUseItemIDData )
#else // PRE_ADD_REDUCE_TABLE_RESOURCE
DNTableFileFormat *CDnTableDB::LoadTable( const char *szFileName, std::string strGenerationLabel/*=std::string()*/ )
#endif // PRE_ADD_REDUCE_TABLE_RESOURCE
{
	std::string strFileName = szFileName;
	strFileName += ".dnt";

// #if defined (_CLIENT) && defined (_TEST_CODE_KAL)
// 	g_ProfileMemCheck.StartBlock(szFileName);
// #endif

	DNTableFileFormat *pSox = NULL;
	try 
	{
#ifdef PRE_ADD_REDUCE_TABLE_RESOURCE
		std::vector<CFileNameString> szVecList;
		FindExtFileList( strFileName.c_str(), szVecList );
		for( DWORD i=0; i<szVecList.size(); i++ ) 
			CEtResourceMng::GetInstance().AddCacheMemoryStream( CEtResourceMng::GetInstance().GetFullName( szVecList[i].c_str() ).c_str() );
#endif // PRE_ADD_REDUCE_TABLE_RESOURCE

		std::vector<CStream *> pVecStream;
		pSox = RawLoadSox(strFileName.c_str(), pVecStream);	
		pSox->SetGenerationInverseLabel( const_cast<char*>(strGenerationLabel.c_str()) );
		
#ifdef PRE_ADD_REDUCE_TABLE_RESOURCE
		pSox->Load( pVecStream, bDontUseItemIDData );
		SAFE_DELETE_PVEC( pVecStream );

		for( DWORD i=0; i<szVecList.size(); i++ ) 
			CEtResourceMng::GetInstance().RemoveCacheMemoryStream( CEtResourceMng::GetInstance().GetFullName( szVecList[i].c_str() ).c_str() );
#else // PRE_ADD_REDUCE_TABLE_RESOURCE
		pSox->Load( pVecStream );
		SAFE_DELETE_PVEC( pVecStream );
#endif // PRE_ADD_REDUCE_TABLE_RESOURCE
		pSox->SetLoadName(strFileName.c_str());
	}
	catch(int) {
		throw strFileName.c_str();
	}

// #if defined (_CLIENT) && defined (_TEST_CODE_KAL)
// 	g_ProfileMemCheck.EndBlock(szFileName);
// #endif

	return pSox;
}

void CDnTableDB::UnloadTable(TableEnum Index)
{
	std::map<TableEnum, DNTableFileFormat*>::iterator it = m_MapTables.find(Index);
	if (it == m_MapTables.end())
		return;

	delete it->second;
	m_MapTables.erase(it);
}

DNTableFileFormat * CDnTableDB::ForceLoadSox( const char *szFileName, std::string &szGenerationLabelStr)
{
	DNTableFileFormat *pSox = NULL;
	try {
		std::vector<CStream *> pVecStream;
		pSox = RawLoadSox(szFileName, pVecStream);
		/////////////////////////////////////////////
		pSox->SetGenerationInverseLabel( const_cast<char*>(szGenerationLabelStr.c_str()) );
		pSox->Load( pVecStream );

		SAFE_DELETE_PVEC( pVecStream );

		pSox->SetLoadName(szFileName);
	}
	catch(int) {
		throw szFileName;
	}
	return pSox;
}

DNTableFileFormat * CDnTableDB::RawLoadSox(const char *szFileName, std::vector<CStream *> &pVecStream)
{
	DNTableFileFormat *pSox = NULL;
	std::vector<CFileNameString> szVecList;
	FindExtFileList( szFileName, szVecList );
	pSox = new DNTableFileFormat;
	
	std::vector<int> nVecFieldCount;
	
	for( DWORD i=0; i<szVecList.size(); i++ ) 
	{
		CResMngStream *pStream = new CResMngStream( szVecList[i].c_str(), true );
		pVecStream.push_back( pStream );
		if( !pStream->IsValid() ) {
			nVecFieldCount.push_back(-1);
			continue;
		}
		short nReserveLen = 0;
		short FieldCount = 0;
		pStream->Seek( CDNTableFile::HeaderSize::Version, SEEK_SET );
		pStream->Read( &nReserveLen, CDNTableFile::HeaderSize::ReserveLen );
		if( nReserveLen > 0 )
			pStream->Seek( nReserveLen, SEEK_CUR );
		pStream->Read( &FieldCount, CDNTableFile::HeaderSize::FieldCount );

		nVecFieldCount.push_back( FieldCount );
	}
	if( !nVecFieldCount.empty() ) {
		bool bInvalidCount = false;
		for( DWORD i=1; i<nVecFieldCount.size(); i++ ) {
			if( nVecFieldCount[0] != nVecFieldCount[i] ) {
				bInvalidCount = true;
				break;
			}
		}
		if( bInvalidCount ) {
			WCHAR szTemp[1024];
			std::wstring szError = L"Invalid Table Field Count\n\n";
			for( DWORD i=0; i<nVecFieldCount.size(); i++ ) {
				swprintf_s( szTemp, L"FileName : %S, FieldCount : %d\n", szVecList[i].c_str(), nVecFieldCount[i] );
				szError += szTemp;
			}
			MessageBox( NULL, szError.c_str(), L"Critical Error!!", MB_OK );
		}
	}
	return pSox;
}

void CDnTableDB::FindExtFileList( const char *szFileName, std::vector<CFileNameString> &szVecList )
{
	char szTemp[_MAX_PATH] = { 0, };
	char szName[256] = { 0, };
	char szExt[256] = { 0, };
	_GetFileName( szName, _countof(szName), szFileName );
	_GetExt( szExt, _countof(szExt), szFileName );
	sprintf_s( szTemp, "%s*.%s", szName, szExt );
	CEtResourceMng::GetInstance().FindFileListAll_IgnoreExistFile( "ext", szTemp, szVecList );
}

void CDnTableDB::AddTable(TableEnum Index, DNTableFileFormat *pSox )
{
	if( !pSox ) return;
	m_MapTables.insert(std::map<TableEnum, DNTableFileFormat*>::value_type(Index, pSox));
}

void CDnTableDB::RemoveTable(TableEnum Index)
{
	m_MapTables.erase(Index);
}

DNTableFileFormat *CDnTableDB::GetTable( TableEnum Index )
{
	std::map<TableEnum, DNTableFileFormat*>::iterator it = m_MapTables.find(Index);

#ifdef PRE_ADD_REDUCE_TABLE_RESOURCE
	if( it == m_MapTables.end() )
	{
		AddTableByIndex( Index );
		it = m_MapTables.find( Index );
	}
#else // PRE_ADD_REDUCE_TABLE_RESOURCE
	if (it == m_MapTables.end())
		return NULL;
#endif // PRE_ADD_REDUCE_TABLE_RESOURCE

	return it->second;
}

DNTableFileFormat * CDnTableDB::ReLoadSox(TableEnum Index)
{
	//Applies only to TCASHCOMMODITY and TCASHPACKAGE among cash items. Check sync issues when extending
	if(Index != TCASHCOMMODITY && Index != TCASHPACKAGE) 
		return NULL;
	std::string strLoadName;
	DNTableFileFormat * pOldSox = GetTable(Index);
	if(pOldSox)
		pOldSox->GetLoadName(strLoadName);
	else
		return NULL;

	//reload!
	std::string strGenerationLevelList;
	if (pOldSox->GetVecGenerationInverseLabelList()->size() > 0)
	{
		std::vector<std::string> * pVecString = pOldSox->GetVecGenerationInverseLabelList();
		if (pVecString)
		{
			std::vector<std::string>::iterator ii;
			for (ii = pVecString->begin(); ii != pVecString->end(); ii++)
			{
				if (ii != pVecString->begin())
					strGenerationLevelList.append(";");
				strGenerationLevelList.append((*ii));
			}
		}
	}

	DNTableFileFormat * pSox = ForceLoadSox(strLoadName.c_str(), strGenerationLevelList);
	if(pSox)
	{
		RemoveTable(Index);
		AddTable(Index, pSox);


		return pSox;
	}
	return NULL;
}

const char *CDnTableDB::GetFileName( int nIndex )
{
	DNTableFileFormat *pSox = GetTable( TFILE );
	if( !pSox->IsExistItem( nIndex ) ) return NULL;
	return pSox->GetFieldFromLablePtr( nIndex, "_FileName" )->GetString();
}

int CDnTableDB::GetMailInfo( int nIndex, MailTableInfoEnum eMailInfo, bool bCashInfo, int nInfoIndex )
{
	DNTableFileFormat *pSox = GetTable( TMAIL );
	if( !pSox ) return 0;
	if( !pSox->IsExistItem( nIndex ) ) return 0;
	bool bCash = ( pSox->GetFieldFromLablePtr( nIndex, "_IsCash" )->GetInteger() == TRUE ) ? true : false;
	if( bCash != bCashInfo ) return 0;
	int nResult = 0;
	if( nInfoIndex < 0 || nInfoIndex > 4 ) nInfoIndex = 0;
	char szLabel[32] = {0,};
	switch( eMailInfo )
	{
	case MailInfoRewardCoin:
		{
			nResult = pSox->GetFieldFromLablePtr( nIndex, "_MailPresentMoney" )->GetInteger();
		}
		break;
	case MailInfoRewardItemID:
		{
			sprintf_s( szLabel, 32, "_MailPresentItem%d", nInfoIndex+1 );
			nResult = pSox->GetFieldFromLablePtr( nIndex, szLabel )->GetInteger();
		}
		break;
	case MailInfoRewardItemCount:
		{
			sprintf_s( szLabel, 32, "_Count%d", nInfoIndex+1 );
			nResult = pSox->GetFieldFromLablePtr( nIndex, szLabel )->GetInteger();
		}
		break;
	}
	return nResult;
}

// PlayerWeightTable preload class
CPlayerWeightTable::CPlayerWeightTable()
{
	DNTableFileFormat *pSox = GetDNTable( CDnTableDB::TPLAYERWEIGHT );
	if( !pSox )
	{
		g_Log.Log( LogType::_FILELOG, L"layerWeightTable.ext failed\r\n");
		return;
	}

	char szLabel[32];
	for( int i=0; i<10; i++ ) {
		sprintf_s( szLabel, "_Character%d", i + 1 );
		for( int j=0; j<Amount; j++ ) {
			m_fPlayerWeightValue[i][j] = pSox->GetFieldFromLablePtr( j + 1, szLabel )->GetFloat();
		};
	}
}

CPlayerWeightTable::~CPlayerWeightTable()
{
}

CGlobalWeightTable::CGlobalWeightTable()
{
	memset( m_fGlobalWeightValue, 0, sizeof(m_fGlobalWeightValue) );
	DNTableFileFormat *pSox = GetDNTable( CDnTableDB::TGLOBALWEIGHT );
	if( !pSox )
	{
		g_Log.Log( LogType::_FILELOG, L"GlobalTable.ext failed\r\n");
		return;
	}

	for( int i=0; i<Amount; i++ ) {
		if( !pSox->IsExistItem( i + 1 ) ) {
			m_fGlobalWeightValue[i] = 0.f;
			continue;
		}

		m_fGlobalWeightValue[i] = pSox->GetFieldFromLablePtr( i + 1, "_Value" )->GetFloat();
	};
}

CGlobalWeightTable::~CGlobalWeightTable()
{
}

float CGlobalWeightTable::GetValue( WeightTableIndex Index )
{ 
	return m_fGlobalWeightValue[Index];
}

CGlobalWeightIntTable::CGlobalWeightIntTable()
{
	memset( m_nGlobalWeightValue, 0, sizeof(m_nGlobalWeightValue) );
	DNTableFileFormat *pSox = GetDNTable( CDnTableDB::TGLOBALWEIGHTINT );
	if( !pSox )
	{
		g_Log.Log( LogType::_FILELOG, L"GlobalTableInt.ext failed\r\n");
		return;
	}

	for( int i=0; i<Amount; i++ ) {
		if( !pSox->IsExistItem( i + 1 ) ) {
			m_nGlobalWeightValue[i] = 0;
			continue;
		}

		m_nGlobalWeightValue[i] = pSox->GetFieldFromLablePtr( i + 1, "_Value" )->GetInteger();
	};
}

CGlobalWeightIntTable::~CGlobalWeightIntTable()
{
}

int CGlobalWeightIntTable::GetValue( WeightTableIndex Index )
{ 
	return m_nGlobalWeightValue[Index];
}

CPlayerLevelTable::CPlayerLevelTable()
{
	DNTableFileFormat *pSox = GetDNTable( CDnTableDB::TPLAYERLEVEL );
	if( !pSox ) 
	{
		g_Log.Log( LogType::_FILELOG, L"PlayerLevelTable.ext failed\r\n");
		return;
	}

	char *szLabelValue[Amount] = {
		"_Strength",
		"_Agility",
		"_Intelligence",
		"_Stamina",
		"_Experience",
		"_SuperAmmor",
		"_SkillPoint",
		"_Fatigue",
		"_DeadDurabilityRatio",
		"_KillScore",
		"_Assistscore",
		"_Assistdecision",
		"_Aggroper",
		"_WeekFatigue",
		"_SKillUsageRatio",
#ifdef PRE_MONITOR_SUPER_NOTE
		"_IllegalAmount",
		"_BanAmount",
#endif
		"_AggroperPvE",
#if defined(PRE_ADD_VIP)
		"_VIPFatigue",
		"_VIPExp",
#endif	// #if defined(PRE_ADD_VIP)
		"_LevelDExp",
	
	};

	for( int i=0; i<pSox->GetItemCount(); i++ ) {
		int nItemID = pSox->GetItemID(i);
		if( nItemID % PLAYER_MAX_LEVEL == 1 ) {
			LevelValue *pValue = new LevelValue[PLAYER_MAX_LEVEL];
			for( DWORD j=0; j<PLAYER_MAX_LEVEL; j++ ) {
				nItemID = pSox->GetItemID(i+j);
				for( int k=0; k<Amount; k++ ) {
					switch( k ) {
						case DeadDurabilityRatio:
						case AggroPer:
						case AggroPerPvE:
							pValue[j].nValue[k] = (int)(( pSox->GetFieldFromLablePtr( nItemID, szLabelValue[k] )->GetFloat() + 0.0001f ) * 100.f );
							break;
						case SPDecreaseRatio:
							pValue[j].fValue[k] = pSox->GetFieldFromLablePtr( nItemID, szLabelValue[k] )->GetFloat();
							break;
						default:
							pValue[j].nValue[k] = pSox->GetFieldFromLablePtr( nItemID, szLabelValue[k] )->GetInteger();
							break;
					}
				}
			}
			int nJobID = ( pSox->GetItemID(i) / PLAYER_MAX_LEVEL ) + 1;
			m_nMapTable.insert( make_pair( nJobID, pValue ) );
			i += ( PLAYER_MAX_LEVEL - 1 ); // Subtract 1 because the for loop increments i++ on next iteration.
		}
	}
}

CPlayerLevelTable::~CPlayerLevelTable()
{
	typedef std::map<int, LevelValue *> TMapPlayerLevelTable;
	SAFE_DELETEA_PMAP( TMapPlayerLevelTable, m_nMapTable );
}

int CPlayerLevelTable::GetValue( int nClassID, int nLevel, PlayerLevelTableIndex Index )
{
	std::map<int, LevelValue *>::iterator it = m_nMapTable.find( nClassID );
	if( it == m_nMapTable.end() ) return -1;
	return it->second[nLevel-1].nValue[Index];
}

float CPlayerLevelTable::GetValueFloat( int nClassID, int nLevel, PlayerLevelTableIndex Index )
{
	std::map<int, LevelValue *>::iterator it = m_nMapTable.find( nClassID );
	if( it == m_nMapTable.end() ) return -1.f;
	return it->second[nLevel-1].fValue[Index];
}

CEqualLevelTable::CEqualLevelTable()
{
	m_iMaxMode = -1;
	m_pMapTable = NULL;

	DNTableFileFormat *pSox = GetDNTable( CDnTableDB::TEQUALLEVEL );
	if( !pSox ) 
	{
		g_Log.Log( LogType::_FILELOG, L"EqualLevelTable.ext failed\r\n");
		return;
	}

	char *szLabelValue[Amount] = {
		"_PhysicalAttackMinRevision",
		"_PhysicalAttackMaxRevision",
		"_MagicAttackMinRevision",
		"_MagicAttackMaxRevision",
		"_PhysicalDefenseRevision",
		"_MagicDefenseRevision",
		"_MaxHPRevision",
		"_MaxSPRevision",
		"_RecoverySPRevision",
		"_StiffRevision",
		"_StiffResistanceRevision",
		"_CriticalRevision",
		"_CriticalResistanceRevision",
		"_StunRevision",
		"_StunResistanceRevision",
		"_SuperAmmorRevision",
		"_MoveSpeedRevision",
		"_ElementAttackRevision",
		"_ElementDefenseRevision",
		"_FinalDamageRevision",
		"_StrengthRevision",
		"_AgilityRevision",
		"_IntelligenceRevision",
		"_StaminaRevision",
	};

	for( int i=0; i<pSox->GetItemCount(); i++ ) 
	{
		int nItemID = pSox->GetItemID(i);

		int iMode = pSox->GetFieldFromLablePtr(nItemID, "_Mode")->GetInteger();
		if( iMode > m_iMaxMode )
		{
			m_iMaxMode = iMode;
		}
	}
	if( m_iMaxMode <= 0 )
		return;

	m_pMapTable = new std::map<int, LevelValue *>[m_iMaxMode];

	for( int j=0; j<m_iMaxMode; j++ ) 
	{
		std::vector<int> nVecItemList;
		pSox->GetItemIDListFromField( "_Mode", (j+1) , nVecItemList );

		for( DWORD i=0; i<nVecItemList.size(); i++ ) 
		{
			int nJobID = pSox->GetFieldFromLablePtr( nVecItemList[i], "_JobID" )->GetInteger();
			if( m_pMapTable[j].find( nJobID ) != m_pMapTable[j].end() ) 
				continue;

			LevelValue *pValue = new LevelValue;
			for( int k=0; k<Amount; k++ ) 
			{
				if( k < ElementAttackRevision )
					pValue->nValue[k] = pSox->GetFieldFromLablePtr( nVecItemList[i], szLabelValue[k] )->GetInteger();
				else 
					pValue->fValue[k] = pSox->GetFieldFromLablePtr( nVecItemList[i], szLabelValue[k] )->GetFloat();
			}
			m_pMapTable[j].insert( make_pair( nJobID, pValue ) );
		}
	}
}

CEqualLevelTable::~CEqualLevelTable()
{
	if( m_pMapTable )
	{
		typedef std::map<int, LevelValue *> TMapEqualLevelTable;
		for( int i=0; i<m_iMaxMode; i++ ) 
		{
			SAFE_DELETE_PMAP( TMapEqualLevelTable, m_pMapTable[i] );
		}
		delete[] m_pMapTable;
		m_pMapTable = NULL;
	}
}

int CEqualLevelTable::GetValue( int nMode, int nJobID, EqualLevelTableIndex Index )
{
	nMode -= 1;
	if( nMode < 0 || nMode >= m_iMaxMode || m_pMapTable == NULL ) 
		return 0;

	std::map<int, LevelValue *>::iterator it = m_pMapTable[nMode].find( nJobID );
	if( it == m_pMapTable[nMode].end() ) 
		return 0;
	return it->second->nValue[Index];
}

float CEqualLevelTable::GetValueFloat( int nMode, int nJobID, EqualLevelTableIndex Index )
{
	nMode -= 1;
	if( nMode < 0 || nMode >= m_iMaxMode || m_pMapTable == NULL ) 
		return 0.f;

	std::map<int, LevelValue *>::iterator it = m_pMapTable[nMode].find( nJobID );
	if( it == m_pMapTable[nMode].end() ) 
		return 0.f;
	return it->second->fValue[Index];
}


CEqualBasisTable::CEqualBasisTable()
{
	DNTableFileFormat *pSox = GetDNTable( CDnTableDB::TEQUALBASIS );
	if( !pSox ) 
	{
		g_Log.Log( LogType::_FILELOG, L"EqualBasis.ext failed\r\n");
		return;
	}

	char *szLabelValue[Amount] = {
		"_PhysicalAttackMinRevision",
		"_PhysicalAttackMaxRevision",
		"_MagicAttackMinRevision",
		"_MagicAttackMaxRevision",
		"_PhysicalDefenseRevision",
		"_MagicDefenseRevision",
		"_MaxHPRevision",
		"_MaxSPRevision",
		"_RecoverySPRevision",
		"_StiffRevision",
		"_StiffResistanceRevision",
		"_CriticalRevision",
		"_CriticalResistanceRevision",
		"_StunRevision",
		"_StunResistanceRevision",
		"_SuperAmmorRevision",
		"_MoveSpeedRevision",
		"_FinalDamageRevision",
		"_ElementAttackRevision",
		"_ElementDefenseRevision",
		"_StrengthRevision",
		"_AgilityRevision",
		"_IntelligenceRevision",
		"_StaminaRevision",
	};

	for( int i=0; i<pSox->GetItemCount(); i++ ) {
		int nItemID = pSox->GetItemID(i);
		if( nItemID % PLAYER_MAX_LEVEL == 1 ) {
			LevelValue *pValue = new LevelValue[PLAYER_MAX_LEVEL];
			for( DWORD j=0; j<PLAYER_MAX_LEVEL; j++ ) {
				nItemID = pSox->GetItemID(i+j);
				for( int k=0; k<Amount; k++ ) {
					pValue[j].nValue[k] = pSox->GetFieldFromLablePtr( nItemID, szLabelValue[k] )->GetInteger();
				}
			}
			int nJobID = ( pSox->GetItemID(i) / PLAYER_MAX_LEVEL ) + 1;
			m_nMapTable.insert( make_pair( nJobID, pValue ) );
			i += ( PLAYER_MAX_LEVEL - 1 ); // Subtract 1 because the for loop increments i++ on next iteration.
		}
	}
}

CEqualBasisTable::~CEqualBasisTable()
{
	typedef std::map<int, LevelValue *> TMapPlayerLevelTable;
	SAFE_DELETEA_PMAP( TMapPlayerLevelTable, m_nMapTable );
}

int CEqualBasisTable::GetValue( int nClassID, int nLevel, EqualBasisTableIndex Index )
{
	std::map<int, LevelValue *>::iterator it = m_nMapTable.find( nClassID );
	if( it == m_nMapTable.end() ) return -1;
	return it->second[nLevel-1].nValue[Index];
}

CFatigueAdjustmentTable::CFatigueAdjustmentTable()
{
	DNTableFileFormat *pSox = GetDNTable( CDnTableDB::TFATIGUEADJUST );
	if( !pSox ) 
	{
		g_Log.Log( LogType::_FILELOG, L"FatigueAdjustTable.ext failed\r\n");
		return;
	}

	char *szLabelValue[5] = {
		"_Easy",
		"_Normal",
		"_Hard",
		"_Master",
		"_Abyss",
	};
	for( int i=0; i<pSox->GetItemCount(); i++ ) {
		int nItemID = pSox->GetItemID(i);
		float *fValue = new float[5];
		for( int j=0; j<5; j++ ) {
			fValue[j] = pSox->GetFieldFromLablePtr( nItemID, szLabelValue[j] )->GetFloat();
		}
		m_fVecValue.push_back( fValue );
	}
}

CFatigueAdjustmentTable::~CFatigueAdjustmentTable()
{
	SAFE_DELETE_AVEC( m_fVecValue );
}

float CFatigueAdjustmentTable::GetValue( int nPartyCount, int nStageConstructionLevel )
{
	if( nPartyCount < 1 || nPartyCount >(int)m_fVecValue.size() ) return 1.f;
	if( nStageConstructionLevel < 0 || nStageConstructionLevel >= 5 ) return 1.f;

	return m_fVecValue[nPartyCount-1][nStageConstructionLevel];
}

#ifdef PRE_ADD_VIP
CVIPGradeTable::CVIPGradeTable()
{
	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TVIPGRADE );
	if(pSox)
	{
		for (int i = 0; i < pSox->GetItemCount(); ++i)
		{
			int nItemID = pSox->GetItemID(i);

			SVIPGradeUnit unit;
			unit.level			= pSox->GetFieldFromLablePtr(nItemID, "_VipLevel")->GetInteger();
			unit.nameUIStringID = pSox->GetFieldFromLablePtr(nItemID, "_VipNameID")->GetInteger();
			unit.ptsMin			= pSox->GetFieldFromLablePtr(nItemID, "_VipPointMin")->GetInteger();
			unit.ptsMax			= pSox->GetFieldFromLablePtr(nItemID, "_VipPointMax")->GetInteger();

			m_VIPGradeList.push_back(unit);
		}
	}
}

const CVIPGradeTable::SVIPGradeUnit* CVIPGradeTable::GetValue(int pts) const
{
	if(pts <= 0)
		return NULL;

#ifdef PRE_FIX_VIP_MAXPTS
#define MAX_VIP_PTS 10000
	if(pts >= MAX_VIP_PTS)
		return &(m_VIPGradeList.back());
#endif

	std::vector<SVIPGradeUnit>::const_iterator iter = m_VIPGradeList.begin();
	for(; iter != m_VIPGradeList.end(); ++iter)
	{
		const SVIPGradeUnit& unit = *iter;
		if(unit.ptsMin <= pts && unit.ptsMax >= pts)
			return &unit;
	}

	return NULL;
}
#endif // PRE_ADD_VIP


CEqualExponentTable::CEqualExponentTable()
{
	DNTableFileFormat *pSox = GetDNTable( CDnTableDB::TEQUALEXPONENT );
	if( !pSox ) 
	{
		g_Log.Log( LogType::_FILELOG, L"EqualExponentTable.ext failed\r\n");
		return;
	}

	char *szLabelValue[Amount] = {
		"_PhysicalAttackMinRevision",
		"_PhysicalAttackMaxRevision",
		"_MagicAttackMinRevision",
		"_MagicAttackMaxRevision",
		"_PhysicalDefenseRevision",
		"_MagicDefenseRevision",
		"_MaxHPRevision",
		"_MaxSPRevision",
		"_RecoverySPRevision",
		"_StiffRevision",
		"_StiffResistanceRevision",
		"_CriticalRevision",
		"_CriticalResistanceRevision",
		"_StunRevision",
		"_StunResistanceRevision",
		"_SuperAmmorRevision",
		"_MoveSpeedRevision",
		"_ElementAttackRevision",
		"_ElementDefenseRevision",
		"_FinalDamageRevision",
		"_StrengthRevision",
		"_AgilityRevision",
		"_IntelligenceRevision",
		"_StaminaRevision",
	};

	for( int i=0; i<pSox->GetItemCount(); i++ ) {
		int nItemID = pSox->GetItemID(i);
		ModeValue Struct;
		for( int k=0; k<Amount; k++ ) {
			Struct.fValue[k] = pSox->GetFieldFromLablePtr( nItemID, szLabelValue[k] )->GetFloat();
		}
		m_VecTable.push_back( Struct );
	}
}

CEqualExponentTable::~CEqualExponentTable()
{
	SAFE_DELETE_VEC( m_VecTable );
}

float CEqualExponentTable::GetValue( int nMode, EqualExponentTableIndex Index )
{
	if( nMode < 1 || nMode > (int)m_VecTable.size() ) return 1.f;

	return m_VecTable[nMode-1].fValue[Index];
}


CGuildRewardTable::CGuildRewardTable()
{
	DNTableFileFormat *pSox = GetDNTable( CDnTableDB::TGUILDREWARDITEM );
	if( !pSox ) 
	{
		g_Log.Log( LogType::_FILELOG, L"guildrewardtable.ext failed\r\n");
		return;
	}

	for( int i=0; i<pSox->GetItemCount(); i++ ) 
	{
		int nItemID = pSox->GetItemID(i);

		GuildReward::GuildRewardInfo newInfo;
		newInfo.ID = nItemID;

		newInfo._NameID = pSox->GetFieldFromLablePtr( nItemID, "_NameID" )->GetInteger();
		newInfo._NameIDParam = pSox->GetFieldFromLablePtr( nItemID, "_NameIDParam" )->GetString();
		newInfo._DescriptionID = pSox->GetFieldFromLablePtr( nItemID, "_DescriptionID" )->GetInteger();
		newInfo._DescriptionIDParam = pSox->GetFieldFromLablePtr( nItemID, "_DescriptionIDParam" )->GetString();
		newInfo._TooltipItemID = pSox->GetFieldFromLablePtr( nItemID, "_TooltipItemID" )->GetInteger();
		newInfo._NeedGold = pSox->GetFieldFromLablePtr( nItemID, "_NeedGold" )->GetInteger();
		newInfo._NeedGuildLevel = pSox->GetFieldFromLablePtr( nItemID, "_NeedGuildLevel" )->GetInteger();
		newInfo._Type = pSox->GetFieldFromLablePtr( nItemID, "_Type" )->GetInteger();
		newInfo._TypeParam1 = pSox->GetFieldFromLablePtr( nItemID, "_TypeParam1" )->GetInteger();
		newInfo._TypeParam2 = pSox->GetFieldFromLablePtr( nItemID, "_TypeParam2" )->GetInteger();
		newInfo._Period = pSox->GetFieldFromLablePtr( nItemID, "_Period" )->GetInteger();
		newInfo._GuildMasterLimit = pSox->GetFieldFromLablePtr( nItemID, "_GuildMasterLimit" )->GetInteger() == 1;
		newInfo._IsApplicable = pSox->GetFieldFromLablePtr( nItemID, "_IsApplicable" )->GetInteger() == 1;
		newInfo._PrerequisiteType = pSox->GetFieldFromLablePtr( nItemID, "_PrerequisiteType" )->GetInteger();
		newInfo._PrerequisiteID = pSox->GetFieldFromLablePtr( nItemID, "_PrerequisiteID" )->GetInteger();
		newInfo._NeedJobClass = pSox->GetFieldFromLablePtr( nItemID, "_NeedJobClass" )->GetInteger();
		newInfo._CheckInven = pSox->GetFieldFromLablePtr( nItemID, "_CheckInven" )->GetInteger() == 1;

		AddGuildRewardInfo(nItemID, newInfo);

	}
}
CGuildRewardTable::~CGuildRewardTable()
{

}


bool CGuildRewardTable::AddGuildRewardInfo(int nID, const GuildReward::GuildRewardInfo& info)
{
	return m_GuildRewardList.insert(make_pair(nID, info)).second;
}

GuildReward::GuildRewardInfo* CGuildRewardTable::GetGuildRewardInfo(int nID)
{
	GUILDREWARD_LIST::iterator findIter = m_GuildRewardList.find(nID);
	if (findIter != m_GuildRewardList.end())
		return &findIter->second;

	return NULL;
}

#ifdef PRE_ADD_ITEMCAT_TOOLTIP_INFO
CItemCategoryInfo::CItemCategoryInfo()
{
	DNTableFileFormat *pSox = GetDNTable(CDnTableDB::TITEMCATEGORYDESC);
	if (!pSox)
	{
		g_Log.Log(LogType::_FILELOG, L"itemcategorytable.dnt failed\r\n");
		return;
	}

	m_ItemCategoryInfo.clear();

	for( int i=0; i<pSox->GetItemCount(); i++ ) 
	{
		int nItemID = pSox->GetItemID(i);

		int itemCategoryTypeIndex = 0, descStringIndex = 0;
		CDNTableFile::Cell* pCatCell = pSox->GetFieldFromLablePtr(nItemID, "_ItemCategoryType");
		if (pCatCell != NULL)
		{
			itemCategoryTypeIndex = pCatCell->GetInteger();
		}
		else
		{
			_ASSERT(0);
			continue;
		}

		CDNTableFile::Cell* pNameCell = pSox->GetFieldFromLablePtr(nItemID, "_NameID");
		if (pNameCell != NULL)
		{
			descStringIndex = pNameCell->GetInteger();
		}
		else
		{
			_ASSERT(0);
			continue;
		}

		if (itemCategoryTypeIndex > 0 && descStringIndex > 0)
			m_ItemCategoryInfo.insert(std::make_pair(itemCategoryTypeIndex, descStringIndex));
	}
}

int CItemCategoryInfo::GetDescIndex(int itemCategoryType) const
{
	std::map<int, int>::const_iterator iter = m_ItemCategoryInfo.find(itemCategoryType);
	if (iter != m_ItemCategoryInfo.end())
		return (*iter).second;

	return -1;
}
#endif // PRE_ADD_ITEMCAT_TOOLTIP_INFO
