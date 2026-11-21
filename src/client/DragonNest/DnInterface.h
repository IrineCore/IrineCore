#pragma once
#include "DnInterfaceDlgID.h"
#include "Timer.h"
#include "DnLoginTask.h"
#include "DnMissionTask.h"
#include "InputReceiver.h"
#include "DnWorld.h"
#include "DnWorldDataManager.h"
#ifdef PRE_ADD_VIP
#include "DnVIPDataMgr.h"
#endif
#include "DnRenderAvatar.h"
#include "ReputationSystem.h"
#include "DnLifeSkillPlantTask.h"
#include "DnCashShopDefine.h"
#ifdef PRE_MOD_SYSTEM_STATE
#include "DnSystemStateManager.h"
#endif
#ifdef PRE_MOD_BROWSER
#include "DnInternetBrowserDefine.h"
#include "DnInternetBrowserManager.h"
#endif
#ifdef PRE_ADD_DOORS
#include "DnDoorsMobileDefine.h"
#endif
#ifdef PRE_ADD_START_POPUP_QUEUE
#include "DnStartPopupMgr.h"
#endif

class CDnLoginDlg;
class CDnMainDlg;
class CDnGaugeDlg;
class CDnGuildWarBossGaugeDlg;
class CDnChatTabDlg;
class CDnTooltipDlg;
class CDnNpcDlg;
class CDnNoticeDlg;
class CTask;
class CDnMessageBox;
class CDnMessageBoxTextBox;
class CDnBigMessageBox;
class CDnMiddleMessageBox;
class CDnDungeonEnterDlg;
class CDnEventDungeonEnterDlg;
class CDnStageClearReportDlg;
class CDnStageClearDlg;
class CDnRebirthDlg;
class CDnCaptionDlg_01;
class CDnCaptionDlg_02;
class CDnCaptionDlg_03;
class CDnCaptionDlg_04;
class CDnCaptionDlg_05;
class CDnCaptionDlg_06;
class CDnCharCreateDlg;
class CDnCharCreateBackDlg;
#ifdef PRE_PRIVATECHAT_CHANNEL
class CDnPrivateChannelInviteDlg;
class CDnPrivateChannelPasswordDlg;
#ifdef PRE_ADD_PRIVATECHAT_CHANNEL
class CDnPrivateChannelPasswordChangeDlg;
#endif // PRE_ADD_PRIVATECHAT_CHANNEL
#endif // PRE_PRIVATECHAT_CHANNEL
#ifdef PRE_MOD_SELECT_CHAR
class CDnCharLoginTitleDlg;
class CDnCharCreatePlayerInfo;
class CDnCharCreateSelectDlg;
class CDnCharRotateDlg;
class CDnCharGestureDlg;
#else // PRE_MOD_SELECT_CHAR
class CDnCharCreateTitleDlg;
#endif // PRE_MOD_SELECT_CHAR
class CDnCharCreateServerNameDlg;
class CDnFadeInOutDlg;
class CDnFadeForBlowDlg;
class CDnAssertDlg;
class CDnDamageMng;
class CDnComboMng;
#ifdef PRE_ADD_CRAZYDUC_UI
class CDnScoreMng;
#endif 
class CDnChainMng;
class CDnCpJudgeMng;
class CDnCpScoreMng;
class CDnCpDownMng;
class CDnServerListDlg;
class CDnWaitUserDlg;
class CDnChannelListDlg;
class CDnSimpleTooltipDlg;
class CDnCustomTooltipDlg;
class CDnDirectDlg;
class CDnPerfHudDlg;
class CDnInspectPlayerDlg;
class CDnAcceptDlg;
class CDnPrivateChatDlg;
class CDnStickAniDlg;
class CDnEnemyGaugeMng;
class CDnGateQuestionDlg;
class CDnMainMenuDlg;
class CDnTextureDlgMng;
class CDnTextBoxDlgMng;
class CDnItemAlarmDlg;
class CDnFriendDlg;
class CDnCharStatusDlg;
class CDnInvenTabDlg;
class CDnStoreTabDlg;
class CDnChannelMoveDlg;
class CDnCountDownDlg;
class CDnMissionFailDlg;
class CDnDungeonClearMoveDlg;
class CDnRebirthCaptionDlg;
class CDnRebirthFailDlg;
class CDnPrivateMarketReqDlg;
class CDnPlayerInfoDlg;
class CDnCharDeleteWaitDlg;
class CDnBlindDlg;
#ifdef PRE_ADD_ALTHEIA_NEWCAPTION_TRIGGER
class CDnImageBlindDlg;
#endif // PRE_ADD_ALTHEIA_NEWCAPTION_TRIGGER
class CDnAcceptRequestDlg;
//blondymarry start
class CDnPVPRoomListDlg;
class CDnPVPMakeRoomDlg;
class CDnBaseRoomDlg;
class CDnPVPGameRoomDlg;
class CDnGuildWarRoomDlg;
#ifdef PRE_ADD_PVP_TOURNAMENT
class CDnPVPTournamentRoomDlg;
class CDnPVPTournamentRoomMatchListDlg;
class CDnPVPTournamentGameMatchListDlg;
class CDnPVPTournamentGameMatchListItemDlg;
class CDnPVPTournamentFinalResultDlg;
#endif
class CDnPVPModeEndDlg;
class CDnRespawnGauageDlg;
class CDnPVPBaseHUD;
class CDnPVPGameResultDlg;
class CDnPVPZoneMapDlg;
class CDnGuildWarZoneMapDlg;
class CDnPvpGuildPropInfoDlg;
class CDnPVPLobbyChatTabDlg;
class CDnPvPKilledMeDlg;
class CDnPvPRacingResultDlg;
class CDnStaffrollDlg;
#ifdef PRE_ADD_DOORS
class CDnDoorsMobileAuthButtonDlg;
#endif

class CDnPVPGameResultAllDlg;
class CDnGuildWarResultDlg;

//blondymarry end
class CDnRandomItemDlg;
class CDnMovieDlg;
class CDnMovieProcessDlg;
class CDnDungeonInfoDlg;
class CDnNpcTalkReturnDlg;
class CDnMissionAchieveAlarmDlg;
class CDnMissionGainAlarmDlg;
class CDnProgressDlg;
class CDnProgressMBox;
class CDnSlideCaptionDlg;
class CDnGameMacroMsgOptDlg;
class CDnGameTipDlg;
class CDnGuildCreateDlg;
class CDnGuildYesNoDlg;
class CDnGuildInviteReqDlg;
class CDnDarklairRoundCountDlg;
class CDnDarklairDungeonEnterDlg;
class CDnDarklairInfoDlg;
class CDnDarklairClearReportDlg;
class CDnDarklairClearRankDlg;
class CDnDarklairClearRewardDlg;
class CDnDarkLairRankBoardDlg;
class CDnHelpKeyboardDlg;
class CDnDungeonOpenNoticeDlg;
class CDnNameLinkMng;
class CDnItemChoiceDlg;
class CDnStageClearMoveDlg;
class CDnItemUnSealDlg;
class CDnItemSealDlg;
class CDnCashShopMenuDlg;
class CDnPetalTokenDlg;
class CDnAppellationGainDlg;
class CDnLifeChannelDlg;

class CDnSecurityNUMPADChangeDlg;
class CDnSecurityNUMPADCheckDlg;
class CDnSecurityNUMPADCreateDlg;
class CDnSecurityKBDDlg;
class CDnSecondPassCheck;
class CDnSecurityNUMPADDeleteDlg;

class CDnCharRenameDlg;

class CDnCharSelectDlg;

class CDnCashShopDlg;
class CDnWorldMsgDlg;
class CDnSkillResetDlg;
class CDnStoreConfirmExDlg;
class CDnEventTodayDlg;

#ifdef PRE_ADD_GACHA_JAPAN
class CDnGachaDlg;
class CDnGachaRouletteDlg;
#endif
class CDnContextMenuDlg;
class CDnCostumeMixDlg;
class CDnCostumeDesignMixDlg;
#ifdef PRE_ADD_COSRANDMIX
class CDnCostumeRandomMixDlg;
#endif

class CDnChatRoomDlg;
class CDnChatRoomPassWordDlg;
class CDnCharmItemKeyDlg;
class CDnCharmItemProgressDlg;
class CDnCharmItemResultDlg;
class CDnPackageBoxResultDlg;
class CDnPackageBoxProgressDlg;
class CDnVoiceChatPremiumOptDlg;
class CDnLifeSkillFishingDlg;

class CDnGuildWarPreliminaryStatusDlg;
class CDnGuildWarPreliminaryEventListDlg;
class CDnGuildWarPreliminaryResultDlg;
class CDnGuildWarFinalTournamentListDlg;
class CDnGuildWarSituationMng;

class CDnGuildRenameDlg;

class CDnDurabilityGaugeMng;
class CDnPVPLadderSystemDlg;
class CDnPVPLadderTabDlg;
class CDnPVPLadderRankBoardDlg;
#if defined( PRE_ADD_REVENGE )
class CDnPVPRevengeMessageBoxDlg;
#endif	// #if defined( PRE_ADD_REVENGE )


#ifdef PRE_FIX_ESCAPE
class CDnEscapeDlg;
#endif

#ifdef PRE_ADD_FADE_TRIGGER
class CDnFadeCaptionDlg;
#endif // PRE_ADD_FADE_TRIGGER

class CDnEnchantJewelDlg;
class CDnGuildMarkCreateDlg;
class CDnMODDlgBase;
class CEtTriggerElement;
class CDnPotentialJewelDlg;
class CDnBossAlertDlg;
struct TQuest;
class CDnLifeTooltipDlg;
class CDnLifeConditionDlg;
class CDnLifeSlotDlg;
class CDnHelpAlarmDlg;
class CDnRepUnionMarkTooltipDlg;

class CDnPetNamingDlg;
class CDnPetExtendPeriodDlg;
class CDnPetAddSkillDlg;
class CDnPetExpandSkillSlotDlg;
#ifdef PRE_ADD_PET_EXTEND_PERIOD
class CDnBuyPetExtendPeriodDlg;
#endif // PRE_ADD_PET_EXTEND_PERIOD

#ifdef PRE_ADD_CHANGEJOB_CASHITEM
class CDnChangeJobCashItemDlg;
#endif // #ifdef PRE_ADD_CHANGEJOB_CASHITEM

class CDnGuildWarMarkTooltipDlg;

class CDnGuildRewardMarkTooltipDlg;
class CDnProgressSimpleMessageBox;

#ifdef PRE_ADD_LEVELUP_GUIDE
class CDnLevelUpGuideDlg;
#endif

class CDnMapMoveCashItemDlg;


class CDnLifeUnionMarkTooltipDlg;
class CDnGuildWarFinalWinNoticeDlg;

#if defined(PRE_ADD_REMOVE_PREFIX)
class CDnRemovePrefixDlg;
#endif // PRE_ADD_REMOVE_PREFIX

class CDnHardCoreModeTimeAttackDlg;
class CDnTimeOutFailDlg;

#ifdef PRE_ADD_STAGE_LIMIT_INTERFACE
class CDnDungeonLimitDlg;
#endif
class CDnDungeonSynchroDlg;
class CDnNestInfoDlg;

class CDnGuildLevelUpAlarmDlg;

#ifdef PRE_ADD_EXCHANGE_POTENTIAL
class CDnPotentialTransferDlg;
#endif

#ifdef PRE_ADD_INSTANT_CASH_BUY
class CDnInstantCashShopBuyDlg;
#endif // PRE_ADD_INSTANT_CASH_BUY

class CDnWorldZoneSelectDlg;

#ifdef PRE_ADD_DIRECT_BUY_UPGRADEITEM 
class CDnMarketDirectBuyDlg;
#endif

#ifdef PRE_ADD_CHARSELECT_PREVIEW_CASHITEM
class CDnCharSelectCostumePreviewDlg;
#endif

#ifdef PRE_WORLDCOMBINE_PVP
class CDnWorldPVPRoomStartDlg;
#endif // PRE_WORLDCOMBINE_PVP

#if defined( PRE_ADD_STAGE_CLEAR_ADD_REWARD )
class CDnDungeonExtraRewardDlg;
#endif	// PRE_ADD_STAGE_CLEAR_ADD_REWARD

#ifdef PRE_MOD_POTENTIAL_JEWEL_RENEWAL
class CDnPotentialJewelCleanDlg;
#endif // PRE_MOD_POTENTIAL_JEWEL_RENEWAL

#if defined( PRE_ALTEIAWORLD_EXPLORE )
class CDnAlteaBoardDlg;
class CDnAlteaClearDlg;
#endif	// #if defined( PRE_ALTEIAWORLD_EXPLORE )

#ifdef PRE_ADD_PVPRANK_INFORM
class CDnPvPRankInformDlg;
#endif




#define _INVALID_TIME_VALUE			-1.f
#define _REQUEST_ACCEPT_TOTALTIME	10.f

//blondy values needed to calculate pvp icon 
const int iIconWidth = 23;
const int iIconHeight = 23;
const int iXNum = 5;
const int iYNUM = 5;
//blondy end

class CDnInterface : public CSingleton<CDnInterface>, public CInputReceiver, public CEtUICallback, public CLostDeviceProcess
{
public:
	CDnInterface();
	virtual ~CDnInterface();

	enum InterfaceTypeEnum 
	{
		Title,
		Login,
		Village,
		PVPVillage,
		Game,
		PVPGame,
		DLGame,
		Farm,
		Common,
		InterfaceTypeMax,
	};

	enum emCAPTION_TYPE
	{
		typeCaption1,	// Notice (shown on top); currently also used at PvP game start.
		typeCaption2,	// Map name (second from top), time count.
		typeCaption3,	// Quest progress info and most other system messages. Displayed in a queue of up to 3 items.
		typeCaption4,   // Caption for PVP start message
		typeCaption5,	// Party invite caption

		typeCaption6,   // Caption dedicated to shutdown system.
#ifdef PRE_ADD_NEWCOMEBACK
		typeCaption7,   // Output two Statics - returnee.
#endif // PRE_ADD_NEWCOMEBACK
	};

	enum emLobbyState
	{
		InLobby,
		InGameRoom,		
	};

	enum emCountDownType
	{
		eNumber_Prefix,
		eNumber_Only,
		eProgress_Always,
		eProgress,
		eCaptain,
	};

	enum eCpJudgeType
	{
		CpType_None,
		CpType_FinishAttack,
		CpType_Critical,
		CpType_Stun,
		CpType_SuperArmorAttack,
		CpType_BrokenShield,
		CpType_Genocide,
		CpType_AirCombo,
		CpType_SkillCombo,
		CpType_Rescue,
		CpType_SkillScore,
		CpType_Combo,
		CpType_PropBreak,
		CpType_GetItem,
		CpType_AssistKillMonster,
		CpType_MAX
	};

	enum eGuildMgrBoxType
	{
		GuildMgrBox_Create	= 50001,	// Arbitrarily set so it does not collide with other dialog IDs.
		GuildMgrBox_Dismiss	= 50002,
		GuildMgrBox_Leave	= 50003,
		GuildMgrBox_Point	= 50004,
		GuildMgrBox_LevelUp	= 50005,
		GuildMgrBox_GuildWarEnroll = 50006,

		GuildMgrBox_Amount	= 6,
	};

	enum eContextMenuType	// Arbitrarily set so it does not collide with other dialog IDs.
	{
		CONTEXTM_MIN	= 60000,
		CONTEXTM_PARTY	= CONTEXTM_MIN,
		CONTEXTM_CHAT,
		CONTEXTM_MAX
	};

	enum emSTORE_CONFIRM_TYPE
	{
		STORE_CONFIRM_SELL,
		STORE_CONFIRM_BUY,
		MAIL_WRITE,
		PRIVATE_TRADE,
		STORAGE_FROM_INVENTORY,
		INVENTORY_FROM_STORAGE,
		MARKET_REGISTER,
		NPC_REPUTATION_GIFT,
	};

	enum emPTAUTH
	{
		EKey,
		EkeyResvPass,
		ECardSmall,
		ECradBig,
		ECradNo,
	};

	enum eBrowserPosType
	{
		eBPT_CENTER,
	};

	enum ePVPRoomState
	{
		ePVP_Room,
		eGuildWar_Room,
		eTournament_Room,
		eRoomMax
	};

	enum eMiniGaugeSize
	{
		eMonsterGaugeSize = 2,
		eMonsterPartsGaugeSize = 4,
		eMiniPlayerGaugeSize = 8,
	};

	enum eMiniPartyPlayerGaugeType
	{
		eLeftType,
		eRighType,
	};

#ifdef PRE_MOD_INTEG_SYSTEM_STATE
	enum eAcceptReqType
	{
		eNONE,
		eTRADE,
		ePARTY,
		eFRIEND,
		eMASTER,
		eDUEL,
	};
#endif

	struct SUnionMembershipData
	{
		int				unionType;
		int				itemId;
		__time64_t		expireDate;
		int				leftUseTime;

		SUnionMembershipData() : unionType(-1), itemId(0), expireDate(0), leftUseTime(0) {}
	};

protected:
	InterfaceTypeEnum m_Type;
	CTask *m_pTask;
	std::string m_szTaskName;

	// Note : UI
	CDnMessageBox*			m_pMessageBox;
	CDnMessageBoxTextBox*	m_pMessageBoxTextBox;
	CDnBigMessageBox*		m_pBigMessageBox;
	CDnMiddleMessageBox*	m_pMiddleMessageBox;
	CDnAssertDlg*			m_pAssertDlg;
	CDnAcceptDlg*			m_pAcceptDlg;
	CDnSimpleTooltipDlg*	m_pSimpleTooltipDlg;
	CDnCustomTooltipDlg*    m_pCustomTooltipDlg;
	CDnFadeInOutDlg*		m_pFadeDlg;
	CDnFadeForBlowDlg*		m_pFadeForBlowDlg;
	CDnBlindDlg*			m_pBlindDlg;
	CDnTextureDlgMng*		m_pTextureDlgMng;
	CDnTextBoxDlgMng*		m_pTextBoxDlgMng;
#ifdef PRE_ADD_ALTHEIA_NEWCAPTION_TRIGGER
	CDnImageBlindDlg*		m_ImageBlindDlg;
#endif // PRE_ADD_ALTHEIA_NEWCAPTION_TRIGGER

	CDnSecurityNUMPADChangeDlg*	m_pSecurityChangeDlg;
	CDnSecurityNUMPADCheckDlg*	m_pSecurityCheckDlg;
	CDnSecurityNUMPADCreateDlg*	m_pSecurityCreateDlg;
	CDnSecurityNUMPADDeleteDlg*	m_pSecurityDeleteDlg;

	CDnSecurityKBDDlg *m_pSecurityKBDDlg;
	CDnSecondPassCheck *m_pSecondPassCheck;

#ifdef PRE_ADD_ACTIVEMISSION
	class CDnAcMissionAlarmDlg * m_pActiveMissionDlg;
#endif // PRE_ADD_ACTIVEMISSION



	bool m_bEnableRender;

	// Note : Title
	int m_nCurLogoIndex;
	float m_fLogoDelta;

	std::vector<CEtUIDialog *> m_pVecLogoList;

	// Note : Login
	CDnLoginTask::LoginStateEnum m_State;

	CDnLoginDlg *m_pLoginDlg;
	CDnCharSelectDlg* m_pCharSelectDlg;
	CDnCharCreateDlg *m_pCharCreateDlg;
	CDnCharCreateBackDlg *m_pCharCreateBackDlg;
	CDnCharCreateServerNameDlg *m_pCharCreateServerNameDlg;
	CDnServerListDlg *m_pServerListDlg;
	CDnWaitUserDlg *m_pWaitUserDlg;
	CDnChannelListDlg *m_pChannelListDlg;

#ifdef PRE_MOD_SELECT_CHAR
	CDnCharLoginTitleDlg*			m_pCharLoginTitleDlg;
	CDnCharCreatePlayerInfo*		m_pCharCreatePlayerInfo;
	CDnCharCreateSelectDlg*			m_pCharCreateSelectDlg;
	CDnCharRotateDlg*				m_pCharRotateDlg;
	CDnCharGestureDlg*				m_pCharGestureDlg;
#else // PRE_MOD_SELECT_CHAR
	CDnCharCreateTitleDlg *m_pCharCreateTitleDlg;
	CDnPlayerInfoDlg *m_pSlotInfoDlg[SHOWCHARNUMPERONEPAGE];
	CDnCharDeleteWaitDlg *m_pSlotDeleteWaitDlg[SHOWCHARNUMPERONEPAGE];
	std::vector<CEtUIDialog *> m_pVecCreateInfoList;
#endif // PRE_MOD_SELECT_CHAR

#ifdef PRE_ADD_CHARSELECT_PREVIEW_CASHITEM 
	CDnCharSelectCostumePreviewDlg* m_pCharSelectPreviewCostumeDlg;
#endif// PRE_ADD_CHARSELECT_PREVIEW_CASHITEM




	CDnStaffrollDlg *m_pStaffrollDlg;

#ifdef PRE_ADD_DOORS
	CDnDoorsMobileAuthButtonDlg* m_pDoorsMobileAuthDlg;
#endif
	// Members to be used on the Game and Village side


	CDnMainDlg *m_pMainBar;


	CDnMainMenuDlg *m_pMainMenuDlg;
	CDnChatTabDlg *m_pChatDlg;
	CDnTooltipDlg *m_pTooltipDlg;
	CDnNpcDlg *m_pNpcDlg;
	CDnNoticeDlg *m_pNoticeDlg;
	CDnGaugeDlg *m_pPlayerGaugeDlg;
	CDnGuildWarBossGaugeDlg * m_pGuildWarBossGaugeDlg;
	CDnCaptionDlg_01 *m_pCaptionDlg_01;
	CDnCaptionDlg_02 *m_pCaptionDlg_02;
	CDnCaptionDlg_03 *m_pCaptionDlg_03;
	CDnCaptionDlg_04 *m_pCaptionDlg_04;
	CDnCaptionDlg_05* m_pCaptionDlg_05;
	CDnCaptionDlg_06* m_pCaptionDlg_06;
#ifdef PRE_ADD_NEWCOMEBACK
	class CDnCaptionDlg_07 * m_pCaptionDlg_07; // PRE_ADD_COMEBACK_RENEWAL
#endif // PRE_ADD_NEWCOMEBACK
	CDnItemAlarmDlg *m_pItemAlarmDlg;
	CDnDirectDlg *m_pDirectDlg;
	CDnInspectPlayerDlg	*m_pInspectPlayerDlg;
	CDnPrivateChatDlg *m_pPrivateChatDlg;
	CDnGateQuestionDlg *m_pGateQuestionDlg;
	CDnWorldDataManager *m_pWorldDataMng;
	CDnChannelMoveDlg *m_pChannelMoveDlg;
	CDnPerfHudDlg *m_pPerfHudDlg;
	CDnPrivateMarketReqDlg *m_pPrivateMarketReqDlg;
	CDnAcceptRequestDlg *m_pAcceptRequestDlg;
	CDnProgressDlg* m_pProgressDlg;
	CDnProgressMBox* m_pProgressMBox;
	CDnRandomItemDlg *m_pRandomItemDlg;
	CDnMovieDlg *m_pMovieDlg;
	CDnMovieProcessDlg *m_pMovieProcessDlg;
	CDnMovieProcessDlg *m_pFocusTypeMovieProcessDlg;
	CDnNpcTalkReturnDlg *m_pNpcTalkReturnDlg;
	CDnMissionAchieveAlarmDlg *m_pMissionAchieveAlarmDlg;
	CDnMissionGainAlarmDlg *m_pMissionGainAlarmDlg;
	CDnSlideCaptionDlg *m_pSlideCaptionDlg;
	CDnGameTipDlg *m_pGameTipDlg;		// Bottom slide. For game tips.
	CDnHelpKeyboardDlg *m_pHelpKeyboardDlg;
	CDnDungeonOpenNoticeDlg *m_pDungeonOpenNoticeDlg;
	CDnStoreConfirmExDlg* m_pSplitConfirmExDlg;
	CDnMODDlgBase *m_pMODDlg;
	CDnBossAlertDlg *m_pBossAlertDlg;
	CDnLifeTooltipDlg * m_pLifeTooltipDlg;
	CDnHelpAlarmDlg *m_pHelpAlarmDlg;

	CDnDungeonSynchroDlg* m_pDungeonSynchroDlg;

#ifdef PRE_ADD_GACHA_JAPAN
	CDnGachaDlg* m_pGachaDlg;
	CDnGachaRouletteDlg* m_pGachaItemNameRouletteDlg;
	CDnGachaRouletteDlg* m_pGachaItemStatRouletteDlg;
#endif

#ifdef PRE_ADD_INSTANT_CASH_BUY
	CDnInstantCashShopBuyDlg* m_pDnInstantCashShopBuyDlg;
#endif // PRE_ADD_INSTANT_CASH_BUY

	// Guild : Guild-related windows are requested to show immediately whenever selected in NPC dialogue.
	// If grouped, they cannot be put in CommonList, so they ended up being held individually like this.
	//
	CDnGuildCreateDlg *m_pGuildCreateDlg;
	CDnGuildYesNoDlg *m_pGuildYesNoDlg;
	CDnGuildInviteReqDlg *m_pGuildInviteReqDlg;

	// Cash commodity world chat
	CDnWorldMsgDlg *m_pWorldMsgDlg;

	// Cash commodity skill reset confirmation dialog.
	CDnSkillResetDlg* m_pSkillResetConfirmDlg;

	CDnChatRoomDlg *m_pChatRoomDlg;
	CDnChatRoomPassWordDlg *m_pChatRoomPassWordDlg;

	CDnCharmItemKeyDlg *m_pCharmItemKeyDlg;
	CDnCharmItemProgressDlg *m_pCharmItemProgressDlg;
	CDnCharmItemResultDlg *m_pCharmItemResultDlg;
#if defined( PRE_ADD_EASYGAMECASH )
	CDnPackageBoxResultDlg * m_pPackageBoxResultDlg;
	CDnPackageBoxProgressDlg * m_pPackageBoxProgressDlg;
#endif	// PRE_ADD_EASYGAMECASH

#ifdef PRE_ADD_AUTOUNPACK
	class CDnAutoUnPackDlg * m_pAutoUnPackDlg;
	class CDnAutoUnPackResultDlg * m_pAutoUnPackResultDlg;	
#endif

#ifdef PRE_ADD_CASHREMOVE
	class CDnInvenCashItemRemoveDlg * m_pCashItemRemoveDlg;
#endif

#ifdef PRE_ADD_DIRECT_BUY_UPGRADEITEM 
	CDnMarketDirectBuyDlg* m_pMarketDirectBuyDlg;
#endif


	CDnVoiceChatPremiumOptDlg *m_pVoiceChatPremiumOptDlg;

	CDnGuildRenameDlg* m_pGuildRenameDlg;
	CDnCharRenameDlg *m_pCharRenameDlg;
	CDnGuildLevelUpAlarmDlg *m_pGuildLevelUpAlarmDlg;

#ifdef PRE_ADD_EXCHANGE_POTENTIAL
	CDnPotentialTransferDlg *m_pPotentialTransferDlg;
#endif

#ifdef PRE_PRIVATECHAT_CHANNEL
	CDnPrivateChannelInviteDlg*		m_pDnPrivateChannelInviteDlg;
	CDnPrivateChannelPasswordDlg*	m_pDnPrivateChannelPasswordDlg;
#ifdef PRE_ADD_PRIVATECHAT_CHANNEL
	CDnPrivateChannelPasswordChangeDlg* m_pDnPrivateChannelPasswordChnageDlg;
#endif // PRE_ADD_PRIVATECHAT_CHANNEL
#endif // PRE_PRIVATECHAT_CHANNEL

	// Note : Used only in Game
	//
	CDnDamageMng *m_pDamageMng;
	CDnComboMng *m_pComboMng;
	CDnChainMng *m_pChainMng;
	CDnCpJudgeMng*	m_pCpJudgeMng;
	CDnCpScoreMng*	m_pCpScoreMng;
	CDnCpDownMng*	m_pCpDownMng;
#ifdef PRE_ADD_CRAZYDUC_UI
	CDnScoreMng* m_pScoreMng;
#endif 
	std::vector<eCpJudgeType>	m_CpShowList;
	CDnStickAniDlg *m_pStickAniDlg;
	CDnEnemyGaugeMng *m_pEnemyGaugeMng;
	CDnMissionFailDlg *m_pMissionFailDlg;
	CDnDungeonClearMoveDlg *m_pDungeonMoveDlg;
	CDnRebirthCaptionDlg *m_pRebirthCaptionDlg;
	CDnRebirthFailDlg *m_pRebirthFailDlg;
	CDnDungeonEnterDlg *m_pDungeonEnterDlg;
	CDnDungeonEnterDlg *m_pDungeonNestEnterDlg;
	CDnHardCoreModeTimeAttackDlg *m_pDnHardCoreModeTimeAttackDlg;
	CDnTimeOutFailDlg *m_pTimeOutFailDlg;
#ifdef PRE_ADD_STAGE_LIMIT_INTERFACE
	CDnDungeonLimitDlg *m_pDungeonLimitDlg;
#endif

	CDnEventDungeonEnterDlg *m_pEventDungeonEnterDlg;
	CDnStageClearReportDlg *m_pStageClearReportDlg;
	CDnStageClearDlg *m_pStageClearDlg;
	CDnDungeonInfoDlg *m_pDungeonInfoDlg;
#if defined( PRE_ADD_STAGE_CLEAR_ADD_REWARD )
	CDnDungeonExtraRewardDlg * m_pDungeonExtraRewardDlg;
#endif	// PRE_ADD_STAGE_CLEAR_ADD_REWARD

	//pvp lobby
	ePVPRoomState m_ePVPRoomState;
	CDnPVPRoomListDlg *m_pPVPRoomListDlg;
	CDnPVPGameRoomDlg *m_pPVPGameRoomDlg;
	CDnGuildWarRoomDlg *m_pGuildWarRoomDlg;
#ifdef PRE_ADD_PVP_TOURNAMENT
	CDnPVPTournamentRoomDlg* m_pPVPTournamentRoomDlg;
	CDnPVPTournamentRoomMatchListDlg* m_pPVPTournamentMatchListDlg;
	CDnPVPTournamentGameMatchListDlg* m_pPVPTournamentGMatchListDlg;
	CDnPVPTournamentGameMatchListItemDlg* m_pPVPTournamentGMatchItemDlg;
#endif
	CDnPVPLobbyChatTabDlg	*m_pPVPChatDlg;

#ifdef PRE_WORLDCOMBINE_PVP
	CDnWorldPVPRoomStartDlg* m_pWorldPVPRoomStartDlg;
#endif // PRE_WORLDCOMBINE_PVP
	
	//pvp ladder
	CDnPVPLadderSystemDlg	*m_pPVPLadderSystemDlg;
	CDnPVPLadderTabDlg		*m_pPVPLadderTabDlg;
	CDnPVPLadderRankBoardDlg*m_pPVPLadderRankBoardDlg;

#if defined( PRE_ADD_REVENGE )
	CDnPVPRevengeMessageBoxDlg * m_pPVPRevengeMessageBox;
#endif	// #if defined( PRE_ADD_REVENGE )

#ifdef PRE_FIX_ESCAPE
	CDnEscapeDlg* m_pEscapeDlg;
#endif
#ifdef PRE_ADD_FADE_TRIGGER
	CDnFadeCaptionDlg* m_pFadeCaptionDlg;
#endif // PRE_ADD_FADE_TRIGGER
	//pvp game
	CDnRespawnGauageDlg *m_pRespawnGauageDlg;
	CDnPVPModeEndDlg *m_pPVPModeEndDlg;
	CDnPVPBaseHUD * m_pModeHUD;
	CDnPVPGameResultDlg * m_pPVPGameResultDlg;
	CDnPVPZoneMapDlg * m_pPVPZoneMapDlg;
	CDnGuildWarZoneMapDlg * m_pGuildWarZoneMapDlg;
	CDnPvpGuildPropInfoDlg * m_pGuildPropInfoDlg;
	CDnPvPKilledMeDlg *m_pPvPKilledMeDlg;
	CDnPvPRacingResultDlg * m_pPvPRacingResultDlg;
#ifdef PRE_ADD_PVP_TOURNAMENT
	CDnPVPTournamentFinalResultDlg* m_pPVPTournamentFinalResultDlg;
#endif

	CDnLifeSkillFishingDlg* m_pLifeSkillFishingDlg;

	CDnPVPGameResultAllDlg* m_pPVPGameResultAllDlg;
	CDnGuildWarResultDlg * m_pGuildWarResultDlg;

	// Darklair game
	CDnDarklairDungeonEnterDlg *m_pDarkLairEnterDlg;
	CDnDarklairRoundCountDlg *m_pDarkLairRoundCountDlg;
	CDnDarklairInfoDlg *m_pDarkLairInfoDlg;
	CDnDarklairClearReportDlg *m_pDarkLairClearReportDlg;
	CDnDarklairClearRankDlg *m_pDarkLairClearRankDlg;
	CDnDarklairClearRewardDlg *m_pDarkLairClearRewardDlg;
	CDnDarkLairRankBoardDlg* m_pDarkLairRankBoardDlg;

	// Farm game
	CDnLifeChannelDlg * m_pLifeChannelDlg;
	CDnLifeConditionDlg * m_pLifeConditionDlg;
	CDnLifeSlotDlg * m_pLifeSlotDlg;
	bool m_bOpenFarmChannel;

	// Guild War game
	CDnGuildWarPreliminaryStatusDlg*		m_pGuildWarPreliminaryStatusDlg;		// Guild war progress status
	CDnGuildWarPreliminaryResultDlg*		m_pGuildWarPreliminaryResultDlg;		// Guild war preliminary result
	CDnGuildWarFinalTournamentListDlg*		m_pGuildWarFinalTournamentListDlg;		// Guild war finals bracket

	CDnGuildWarSituationMng * m_pGuildWarSituation;

	CDnPetNamingDlg*		m_pPetNamingDlg;
	CDnPetExtendPeriodDlg*	m_pPetExtendPeriodDlg;
	CDnPetAddSkillDlg*		m_pPetAddSkillDlg;
	CDnPetExpandSkillSlotDlg*	m_pPetExpandSkillSlotDlg;
#ifdef PRE_ADD_PET_EXTEND_PERIOD
	CDnBuyPetExtendPeriodDlg*	m_pBuyPetExtendPeriodDlg;
#endif // PRE_ADD_PET_EXTEND_PERIOD

	CDnGuildWarFinalWinNoticeDlg*	m_pDnGuildWarFinalWinNoticeDlg;

	CDnMapMoveCashItemDlg* m_pMapMoveCashItemDlg;

	EtTextureHandle m_hPVPIconImage[2]; //PvP icon image
	std::vector<EtTextureHandle> m_vecGuildMarkTextureHandle;
#ifdef PRE_ADD_VIP
	EtTextureHandle m_hVIPIconImage;
#endif
	EtTextureHandle m_hPartyMemberMarkerImage;
	EtTextureHandle m_hGuildPreliminariesImage;
	EtTextureHandle m_hStigmaActorMarketImage;
	EtTextureHandle m_hPointMarkImage;
	EtTextureHandle m_hPointMarkTailImage;

	char m_cLastDungeonEnterDialogType;
	CDnProgressSimpleMessageBox* m_pProgressSimpleMsgBox;

	//blondymarry end
	struct PartyPlayerGauge
	{
		CDnGaugeDlg *pGaugeDlg;
		DnActorHandle hActor;
	};

	struct EnemyGauge
	{
		CDnGaugeDlg *pGaugeDlg;
		DnActorHandle hActor;
	};

	std::vector<CEtUIDialog*> m_pVecCommonDialogList;
	std::vector<PartyPlayerGauge> m_pVecPartyPlayerGaugeList;
	std::vector<EnemyGauge> m_pVecEnemyGaugeList;
#ifdef PRE_ADD_IMPROVE_MASTER_OBSERVER
	std::vector<PartyPlayerGauge> m_pVecPartyPlayerRightGageList; // Right-aligned party gauge; currently used in PVP tournament observer mode.
#endif

	std::vector<CDnGaugeDlg*> m_pVecMonsterPartsGuageList; 
	EtVector2 m_vec2MonsterPartsGaugeOffset;

	std::vector<CEtUIDialog*> m_pVecOpenBaseDialogList;
	float m_fPartyPlayerGaugeOffset;
	float m_fEnemyGaugeOffset;
	CDnRenderAvatarNew m_TempAvatar[8];	// Pre-allocate up to the number of party members.
	int m_nCompareInfo[8][4];			// Four parts: Face, Hair, Helmet, Earring.

	// Global CountDown
	bool			m_bEnableCountDown;
	float			m_fCountDownDelta;
	std::wstring	m_szCountDownString;
	std::wstring    m_szPVPRoomPassword;
	emCountDownType m_CountDownType;

	bool m_bIsOpenNpcDlg;
	bool m_bProcessMainProcess;

	int	m_nAccumulationCPCache;

	// Variable created for ease of access.
	CDnGameMacroMsgOptDlg *m_pGameMacroMsgOptDlg;

	CDnNameLinkMng*		m_pNameLinkMng;
	CDnTooltipDlg*		m_pNameLinkToolTipDlg;

	CDnItemChoiceDlg*		m_pItemChoiceDlg;
	CDnStageClearMoveDlg*	m_pStageClearMoveDlg;
	CDnItemUnSealDlg*		m_pItemUnsealDlg;
	CDnItemSealDlg*			m_pItemSealDlg;
	CDnPotentialJewelDlg*	m_pItemPotentialDlg;
#ifdef PRE_MOD_POTENTIAL_JEWEL_RENEWAL
	CDnPotentialJewelCleanDlg* m_pItemPotentialCleanDlg;
	bool m_bIsRemovePotentialDlg;
#endif// PRE_MOD_POTENTIAL_JEWEL_RENEWAL
	CDnPetalTokenDlg*		m_pItemPetalTokenDlg;
	CDnAppellationGainDlg*	m_pItemAppellationGainDlg;

	CDnEnchantJewelDlg *m_pEnchantJewelDlg;
	CDnGuildMarkCreateDlg *m_pGuildMarkCreateDlg;

#ifdef PRE_ADD_CHANGEJOB_CASHITEM
	CDnChangeJobCashItemDlg* m_pChangeJobDlg;
#endif // #ifdef PRE_ADD_CHANGEJOB_CASHITEM

	CDnDurabilityGaugeMng* m_pDurabilityGaugeMng;
	CDnRepUnionMarkTooltipDlg* m_pRepUnionMarkTooltipDlg;

#if defined(PRE_ADD_VIP_FARM)
	CDnLifeUnionMarkTooltipDlg * m_pLifeUnionMarkTooltipDlg;
#endif
	CDnGuildWarMarkTooltipDlg* m_pGuildWarMarkTooltipDlg;
	CDnGuildRewardMarkTooltipDlg* m_pGuildRewardMarkTooltipDlg;


#ifdef PRE_ADD_LEVELUP_GUIDE
	CDnLevelUpGuideDlg* m_pLevelUpGuildeDlg;
#endif

#if defined(PRE_ADD_REMOVE_PREFIX)
	CDnRemovePrefixDlg* m_pRemovePrefixDlg;
#endif // PRE_ADD_REMOVE_PREFIX

#if defined( PRE_ALTEIAWORLD_EXPLORE )
	CDnAlteaBoardDlg * m_pAlteaBoardDlg;
	CDnAlteaClearDlg * m_pAlteaClearDlg;
#endif	// #if defined( PRE_ALTEIAWORLD_EXPLORE )

	CDnNestInfoDlg* m_pNestInfoDlg;
	CDnWorldZoneSelectDlg *m_pWorldZoneSelectDlg;

	enum eFadeState
	{
		eFS_NONE,
		eFS_CASHSHOP_OPEN_FO,
		eFS_CASHSHOP_OPEN_FI,
		eFS_CASHSHOP_CLOSE_FO,
		eFS_CASHSHOP_CLOSE_FI,
	};
	eFadeState	m_FadeState;
	CDnCashShopMenuDlg* m_pCashShopMenuDlg;
	CDnCashShopDlg*		m_pCashShopDlg;

#ifdef PRE_ADD_CHAOSCUBE
	class CDnChaosCubeDlg * m_pChaosCubeDlg;	
	class CDnChaosCubeResultDlg * m_pChaosCubeResultDlg; // Acquired result Dlg.

	class CDnChaosCubeStuffDlg * m_pChaosCubeStuffDlg; // MaterialDlg or ResultDlg.
	class CDnChaosCubeProgressDlg * m_pChaosCubeProgressDlg; // Progress Dlg.
	class CDnChaosGaugeCubeDlg * m_pChaosGaugeCubeDlg;
#endif
#if defined( PRE_ADD_DWC )
	class CDnDwcDlg *m_pDwcDlg;
	class CDnDwcCreateTeamDlg *m_pDwcCreateTeamDlg;
	class CDnDwcInviteReqDlg *m_pDwcInviteReqDlg;
	class CDnDWCCharCreateDlg *m_pDWCCharCreateDlg;
	class CDnPVPDWCTabDlg *m_pPVPDWCTabDlg;
	class CDnPvPRankListItemDwcDlg *m_pPvPRankListItemDwcDlg;
	class CDnDWCMarkTooltipDlg *m_pDWCMarkTooltipDlg;
	class CDnDWCMarkInformer *m_pDWCMarkInformer;
#endif
	// Extended UI & QoL Dialogs
	class CDnMainQuestDlg *m_pMainQuestDlg;
	class CDnJobInformationMainDlg *m_pJobInformationMainDlg;
	class CDnSkillClassInfoDlg *m_pSkillClassInfoDlg;
	class CDnSkillCountMng *m_pSkillCountMng;
	class CDnSubGaugeDlg *m_pSubGaugeDlg;
	class CDnNPCHotKeyInfoDlg *m_pNPCHotKeyInfoDlg;
	class CDnDungeonEnterNotifierDlg *m_pDungeonEnterNotifierDlg;
	class CDnNestNotifierDlg *m_pNestNotifierDlg;
	class CDnQuestNotifierListItemDlg *m_pQuestNotifierListItemDlg;
	class CDnStageShortCutRepairDlg *m_pStageShortCutRepairDlg;
	class CDnPackageBoxResultAutoDlg *m_pPackageBoxResultAutoDlg;
	class CDnPotenCharmItemProgressDlg *m_pPotenCharmItemProgressDlg;
	class CDnSecretCashShopAlarmButtonDlg *m_pSecretCashShopAlarmBtnDlg;
	class CDnSecretCashShopBuyDlg *m_pSecretCashShopBuyDlg;
	class CDnSecretCashShopMessageDlg *m_pSecretCashShopMsgDlg;
	class CDnItemsOwnedAlarmMng *m_pItemsOwnedAlarmMng;
	class CDnAlarmMarkButton *m_pAlarmMarkButton;
	class CDnInputContextMessageBox *m_pInputContextMessageBox;
	class CDnMetaMissionTooltipDlg *m_pMetaMissionTooltipDlg;
	class CDnMissionAchieveEventListBoxDlg *m_pMissionAchieveEventListBoxDlg;
	class CDnPremiumPcBangNotifyToolTipDlg *m_pPremiumPcBangNotifyToolTipDlg;

#ifdef PRE_ADD_CHOICECUBE
	class CDnChoiceCubeDlg * m_pChoiceCubeDlg;
#endif

#ifdef PRE_ADD_BESTFRIEND
	class CDnBFRegistrationDlg * m_pBFRegistrationDlg;			  // Best friend certificate Dlg.
	class CDnIdentifyBestFriendDlg * m_pIdentifyBF;				  // Best friend confirm Dlg.
	class CDnBestFriendProgressDlg * m_pBFProgressDlg;			  // Best friend confirm accept ProgressDlg.
	class CDnBestFriendRewardDlg * m_pBFRewardDlg;				  // Reward item Dlg.
	class CDnBestFriendRewardProgressDlg * m_pBFRewardProgressDlg;// Reward item ProgressDlg.
	class CDnBestFriendMemoDlg * m_pBFMemoDlg;					  // Memo Dlg.
	std::wstring m_strNameBF; // Best friend name - stored here as needed for tooltip handling.
	INT64 m_serialBF;
#endif

#ifdef PRE_MOD_INTEG_SYSTEM_STATE
#else
	enum eAcceptReqType
	{
		eNONE,
		eTRADE,
		ePARTY,
		eFRIEND,
		eMASTER,
		eDUEL,
	};
#endif
	eAcceptReqType	m_CurAcceptReqDlg;

	bool m_bLockMainMenu;

	std::map<int, CDnContextMenuDlg*>	m_pContextMenuDlgList;
	std::multimap<int, CEtUIDialog*>	m_pMsgListenDlgList;

	bool m_bJoypadPushed;
#ifdef PRE_MOD_SYSTEM_STATE
	CDnSystemStateManager m_SystemStateMgr;
#endif

#ifdef PRE_ADD_COMEBACK
	bool m_bComeback;	

#endif
#ifdef PRE_ADD_NEWCOMEBACK
	class CDnComeBackRewardDlg * m_pComeBackRewardDlg;
	class CDnComeBackMsgDlg * m_pComebackMsgDlg;
	bool m_bComebackUser;	
#endif // PRE_ADD_NEWCOMEBACK
#ifdef PRE_WORLDCOMBINE_PARTY
	std::map<int, std::wstring> m_ServerListCache;
#endif

#ifdef PRE_ADD_PVP_RANKING
	class CDnPvPRankDlg * m_pPvPRankDlg;
#endif // PRE_ADD_PVP_RANKING

#ifdef PRE_MOD_BROWSER
	std::wstring m_DisableDlgStringCache;
	float m_DisableDlgDelayTime;
	CDnInternetBrowserManager m_InternetBrowserMgr;
#endif

#ifdef PRE_ADD_PVP_COMBOEXERCISE
	LOCAL_TIME m_TimeSummonMonster; // Monster spawn time limit in combo practice mode.
#endif // PRE_ADD_PVP_COMBOEXERCISE

#ifdef PRE_ADD_STAMPSYSTEM
	class CDnStampEventAlarmDlg * m_pStampEventAlarmDlg;
#endif // PRE_ADD_STAMPSYSTEM

#ifdef PRE_ADD_GAMEQUIT_REWARD
	class CDnGameQuitRewardDlg * m_pGameQuitRewardDlg;
	class CDnGameQuitNextTimeRewardDlg * m_pGameQuitNextTimeRewardDlg;
	class CDnGameQuitNextTimeRewardComboDlg * m_pGameQuitNextTimeRewardComboDlg;
#endif // PRE_ADD_GAMEQUIT_REWARD

#ifdef PRE_ADD_PART_SWAP_RESTRICT_TRIGGER
	bool m_bIsPartSwapRestrict; // Restrict item equipping.
#endif

#ifdef PRE_ADD_RETURN_VILLAGE_SHOWNAME
	int m_nVillage;
#endif 

#ifdef PRE_ADD_START_POPUP_QUEUE
	CDnStartPopupMgr m_StartPopupMgr;
#endif

#ifdef PRE_ADD_PVPRANK_INFORM
	CDnPvPRankInformDlg* m_pPvPRankInformDlg;
#endif


protected:
	void InitializeTitle( CTask *pTask );
	void InitializeLogin( CTask *pTask );
	void InitializeVillage( CTask *pTask );
	void InitializeGame( CTask *pTask );
	void InitializeCommon( CTask *pTask );

	void InitializePVPVillage( CTask *pTask);
	void InitializePVPGame( CTask *pTask );
	void InitializeDLGame( CTask *pTask );
	void InitializeFarmGame( CTask *pTask );

	void ReInitializeTitle();
	void ReInitializeLogin();
	void ReInitializeVillage();
	void ReInitializeGame();
	void ReInitializePVPGame();
	void ReInitializeDLGame();
	void ReInitializePVPVillage();
	void ReInitializeCommon();
	void ReInitializeFarmGame();

	void FinalizeTitle();
	void FinalizeLogin();
	void FinalizeVillage();
	void FinalizeGame();
	void FinalizeCommon();
	void FinalizePVPGame();
	void FinalizePVPVillage();
	void FinalizeDLGame();
	void FinalizeFarmGame();

	bool ProcessTitle( LOCAL_TIME LocalTime, float fDelta );
	bool ProcessLogin( LOCAL_TIME LocalTime, float fDelta );
	bool ProcessVillage( LOCAL_TIME LocalTime, float fDelta );
	bool ProcessGame( LOCAL_TIME LocalTime, float fDelta );
	bool ProcessDefaultGameUI( LOCAL_TIME LocalTime, float fDelta );
	bool ProcessPVPVillage( LOCAL_TIME LocalTime, float fDelta );
	bool ProcessPVPGame( LOCAL_TIME LocalTime, float fDelta );
	bool ProcessDLGame( LOCAL_TIME LocalTime, float fDelta );
	bool ProcessFarmGame( LOCAL_TIME LocalTime, float fDelta );

	void ProcessInput( LOCAL_TIME LocalTime, float fDelta );
	void ProcessPVPInput();

#ifdef PRE_ADD_AIM_ASSIST_BUTTON
	void ProcessAimAssistFindTargetUI( LOCAL_TIME LocalTime, float fDelta );
	void ProcessAimAssistMoveNearUI( LOCAL_TIME LocalTime, float fDelta );
	bool m_bAimKeyState;
	float m_fAimMoveDelay;
	int m_nLockCursorPosX;
	int m_nLockCursorPosY;
#endif

	void SetPVPVillageMenu( bool isPVP  );	
	void InitCommonDlg();
	void InitPVPLobbyDlg(); //Initialize UI used in PVP

	void CreatePlayerGauge();
	void CreateMonsterGauge();
	void AddCommonDialogList( CEtUIDialog *pDialog )			{ m_pVecCommonDialogList.push_back( pDialog ); }
	void AddPartyPlayerGaugeList( CDnGaugeDlg *pGauge , eMiniPartyPlayerGaugeType eType );
	void AddMonsterGaugeList( CDnGaugeDlg *pGauge );

	CEtUIDialog * GetCommonDialog( int ID );
	bool RemoveCommonDialog( int ID ); // Remove from CommonDialogList.

	void ProcessMonsterPartsGauge( DnActorHandle hActor );
	void HideMonsterPartsGauge();
	void CreateMonsterPartsGauge();

	void ProcessPlayerGauge();
	void ProcessPartyPlayerGauge();
	void ProcessMonsterGauge();
	void ProcessAcceptRequestDialog();

#ifdef PRE_ADD_BUFF_ADD_INFORMATION
	void ProcessVillagePartyPlayerGauge();
#endif

#ifdef PRE_SWAP_QUICKSLOT
	bool m_bSwapState;
	void ProcessSwapQuickSlot();
#endif

	void OpenNpcTalkReturnDlg();
public:
	// Common
	void PreInitialize();
	void PostFinalize();
	void Initialize( InterfaceTypeEnum Type, CTask *pTask );
	void ReInitialize();
	void Finalize( InterfaceTypeEnum Type );
	bool Process( LOCAL_TIME LocalTime, float fDelta );
	void PostProcess( LOCAL_TIME LocalTime, float fDelta );
	void JoypadPostProcess();

	InterfaceTypeEnum GetInterfaceType()				{ return m_Type; }
	CDnCashShopDlg*	GetCashShopDlg() { return m_pCashShopDlg; }
	CDnCashShopMenuDlg *GetCashShopMenuDlg() { return m_pCashShopMenuDlg; }

	//#ifndef _FINAL_BUILD
	void _TestUIDump( CTask *pTask );
	//#endif	// #ifndef _FINAL_BUILD

#ifdef PRE_ADD_AIM_ASSIST_BUTTON
	bool GetAimKeyState()								{ return m_bAimKeyState; }
#endif

	CDnDungeonInfoDlg * GetDungeonInfoDlg()				{ return m_pDungeonInfoDlg; }

	CDnFadeInOutDlg *GetFadeDlg()						{ return m_pFadeDlg; }
	CDnLoginDlg *GetLoginDlg()							{ return m_pLoginDlg; }
	CDnServerListDlg *GetServerListDlg()				{ return m_pServerListDlg; }
	CDnCharCreateDlg *GetCharCreateDlg()				{ return m_pCharCreateDlg; }
	CDnTooltipDlg *GetTooltipDialog()					{ return m_pTooltipDlg; }
	CDnTooltipDlg* GetNameLinkToolTipDlg() const		{ return m_pNameLinkToolTipDlg; }
	void CloseNameLinkToolTipDlg();
	CDnSimpleTooltipDlg* GetSimpleTooltipDialog()		{ return m_pSimpleTooltipDlg; }
	CDnChatTabDlg *GetChatTabDialog()						{ return m_pChatDlg; }
	CDnChatTabDlg *GetChatDialog( bool bOtherChatDlg = false );					
	CDnMainDlg *GetMainBarDialog()						{ return m_pMainBar; }
	CDnNpcDlg *GetNpcDialog()							{ return m_pNpcDlg; }
	CDnStageClearDlg *GetStageClearDialog()				{ return m_pStageClearDlg; }
	CDnStageClearReportDlg *GetStageClearReportDialog()	{ return m_pStageClearReportDlg; }
	CDnDungeonEnterDlg *GetDungeonEnterDialog()			{ return m_pDungeonEnterDlg; }
	CDnEventDungeonEnterDlg *GetEventDungeonEnterDialog()	{ return m_pEventDungeonEnterDlg; }
	CDnDungeonEnterDlg *GetCurrentDungeonEnterDialog();
	CDnBlindDlg* GetBlindDialog()						{ return m_pBlindDlg; }
	CDnInspectPlayerDlg* GetInspectDialog() { return m_pInspectPlayerDlg; }
#ifdef PRE_ADD_ALTHEIA_NEWCAPTION_TRIGGER
	CDnImageBlindDlg* GetImageBlindDialog() { return m_ImageBlindDlg; }
#endif // PRE_ADD_ALTHEIA_NEWCAPTION_TRIGGER

	void ShowDungeonSynchroDialog( bool bShow, bool bSynchro );

	CDnMainMenuDlg *GetMainMenuDialog()					{ return m_pMainMenuDlg; }
	CEtUIDialog *GetMainMenuDialog( DWORD dwDialogID );

	void SetGameMacroMsgOptDlg( CDnGameMacroMsgOptDlg *pDlg )	{ m_pGameMacroMsgOptDlg = pDlg; }
	CDnGameMacroMsgOptDlg *GetGameMacroMsgOptDlg()			{ return m_pGameMacroMsgOptDlg; }

	// Darklair
	CDnDarklairClearRewardDlg *GetDarkLairClearRewardDialog() { return m_pDarkLairClearRewardDlg; }

#ifdef PRE_ADD_INSTANT_CASH_BUY
	CDnInstantCashShopBuyDlg* GetInstantCashShopBuyDlg() { return m_pDnInstantCashShopBuyDlg; }
#endif // PRE_ADD_INSTANT_CASH_BUY

#ifdef PRE_MOD_SYSTEM_STATE
	CDnSystemStateManager& GetSystemStateMgr() { return m_SystemStateMgr; }
#endif
#ifdef PRE_PRIVATECHAT_CHANNEL
	CDnPrivateChannelPasswordDlg* GetPrivateChannelPasswordDlg() { return m_pDnPrivateChannelPasswordDlg; }
	CDnPrivateChannelInviteDlg* GetPrivateChannelInviteDlg() { return m_pDnPrivateChannelInviteDlg; }
#ifdef PRE_ADD_PRIVATECHAT_CHANNEL
	CDnPrivateChannelPasswordChangeDlg* GetPrivateChannelPasswordChangeDlg() { return m_pDnPrivateChannelPasswordChnageDlg; }
#endif // PRE_ADD_PRIVATECHAT_CHANNEL
#endif // PRE_PRIVATECHAT_CHANNEL

#ifdef PRE_WORLDCOMBINE_PVP
	void SetWorldPVPStartButtonInfo( bool bShow, UINT uiPvPIndex = -1 );
	void ChangePVPScoreTeam( int nSession, int nTeam );
#endif // PRE_WORLDCOMBINE_PVP
#ifdef PRE_MOD_DARKLAIR_RECONNECT
	CDnDarklairInfoDlg* GetDarklairInfoDlg() { return m_pDarkLairInfoDlg; }
#endif // PRE_MOD_DARKLAIR_RECONNECT

	// Note : UI
	void OpenBaseDialog();
	CDnMessageBox* GetMessageBox() { return m_pMessageBox; }
	CDnMessageBoxTextBox* GetMessageBoxTextBox() { return m_pMessageBoxTextBox; }
	CDnBigMessageBox * GetBigMessageBox() { return m_pBigMessageBox; }
	CDnMiddleMessageBox * GetMiddleMessageBox() { return m_pMiddleMessageBox; }
	void ShowGateQuestionDialog( bool bShow, std::wstring &strGateName = std::wstring(), CEtUICallback *pCallback = NULL );

	void ShowWorldZoneSelectDialog( bool bShow, const BYTE cGateNo = 0, CEtUICallback *pCallback = NULL );
	bool IsShowWorldZoneSelectDialog();
	BYTE GetWorldZoneSelectIndex();
	void SetWorldZoneSelectIndex( const BYTE cSelectIndex );

	bool IsFarm();
	void ShowFarmGateQuestionDialog( bool bShow, SCFarmInfo * pFarmInfo, CEtUICallback *pCallback = NULL );
	void UpdateFarmPlantedInfo( SCFarmPlantedInfo * pPlantedInfo );
	void RefreshLifeStorage( SCFarmWareHouseList * pData );
	void RequestTakeWareHouseItem( int nSlotIndex );
	void OnRecvTakeWareHouseItem(INT64 biUniqueID);
	void SetFarmTooltip( CDnLifeSkillPlantTask::SPlantTooltip & info );
	void ChangeState( int nAreaIndex, int nItemID, int eAreaState, float fTime, float fWater, char * szHarvestIconName );
	void FarmConditionRefresh();
	CDnLifeTooltipDlg * GetLifeTooltipDlg();
	CDnLifeConditionDlg * GetLifeConditionDlg();
	CDnLifeChannelDlg * GetLifeChannelDlg();
	void OpenLifeSlotDialog();
	void CloseLifeSlotDialog();
	void OpenFarmChannel();
	void CloseFarmChannel();

	DWORD DrawTextureWindow( EtTextureHandle hTextureHandle, float fX, float fY, float fDelayTime, DWORD dwSetID = 0, CEtUICallback *pCallback = NULL );
	DWORD DrawTextureWindow( EtTextureHandle hTextureHandle, float fX, float fY, int nPos, float fDelayTime, DWORD dwSetID = 0, CEtUICallback *pCallback = NULL );
#ifdef PRE_ADD_OPEN_QUEST_TEXTURE_DIALOG
	DWORD DrawTextureWindow( EtTextureHandle hTextureHandle, float fX, float fY, int nPos, float fDelayTime, DWORD dwSetID = 0, bool bAutoCloseDialog = true, CEtUICallback *pCallback = NULL );
	DWORD ClearNpcTalkTextureWindow();
#endif
	void CloseTextureWindow( DWORD dwID, bool bSetID = false );
	void ClearTextureWindow();

	DWORD ShowAlarmWindow( int nType, float fX, float fY, float fWidth, float fDelayTime, LPCWSTR wszStr, DWORD dwSetID = 0, CEtUICallback *pCallback = NULL );
	DWORD ShowAlarmWindow( int nType, float fX, float fY, int nPos, int nFormat, float fDelayTime, LPCWSTR wszStr, DWORD dwSetID = 0, CEtUICallback *pCallback = NULL );
	DWORD ShowCountDownAlarmWindow( int nType, float fX, float fY, int nPos, int nFormat, float fDelayTime, int nCountDown, DWORD dwSetID = 0, CEtUICallback *pCallback = NULL );
	void CloseAlarmWindow( DWORD dwID, bool bSetID = false );
	void ClearAlarmWindow();
#ifdef PRE_FIX_QUICKPVP_EXCLUSIVEREQ
	void ClearAcceptReqDlg(bool bReject = false);
#else
	void ClearAcceptReqDlg();
#endif

	// Most system notification messages use typeCaption3!
	void ShowCaptionDialog( emCAPTION_TYPE emCaptionType, LPCWSTR wszMsg, DWORD dwColor = textcolor::WHITE, float fFadeTime = 5.0f );
	void ShowCaptionDialog( emCAPTION_TYPE emCaptionType, int nIndex, DWORD dwColor = textcolor::WHITE, float fFadeTime = 5.0f );
#ifdef PRE_ADD_NEWCOMEBACK
	void ShowCaptionDialog2( emCAPTION_TYPE emCaptionType, std::vector< int > & vecStrIdx, DWORD dwColor = textcolor::WHITE, float fFadeTime = 5.0f ); // PRE_ADD_NEWCOMEBACK
#endif // PRE_ADD_NEWCOMEBACK
	void CloseCaptionDialog( emCAPTION_TYPE emCaptionType );
	void CloseCaptionDialog();
	CDnCaptionDlg_04* GetCaptionDlg_04(){return m_pCaptionDlg_04;}; // Fetch to control the second message. < Using this way for now, will likely need refactoring later. >

	CDnPVPLobbyChatTabDlg* GetPVPLobbyChatTabDlg(){return m_pPVPChatDlg;}

	CDnPVPGameResultAllDlg* GetPVPGameResultAllDlg(){return m_pPVPGameResultAllDlg;}; // BESTUSER can be found in the score.
	CDnPvPRacingResultDlg * GetPvPRacingResultDlg() { return m_pPvPRacingResultDlg; }

	CDnGuildWarResultDlg * GetGuildWarResultDlg()	{return m_pGuildWarResultDlg;}

	void SetPVPRoomPassword(std::wstring Pass){m_szPVPRoomPassword = Pass;}
	std::wstring GetPVPRoomPassword(){return m_szPVPRoomPassword;}
	void AddSlideCaptionMessage( LPCWSTR wszMsg, DWORD dwColor = textcolor::WHITE, float fShowTime = 60.0f );
	void CloseSlideCaptionMessage( bool bForce = true );
	void DelSlideCaptionMessage( LPCWSTR wszMsg );
	void DelSlideCaptionMessage( int nIndex );

	void AddGameTipMessage( LPCWSTR wszMsg, DWORD dwColor = textcolor::WHITE, float fShowTime = 5.0f );	// Unlike slide, shown once and then shown again after a long interval.
	void CloseGameTipMessage();

	void ShowSkillAlarmDialog( LPCWSTR wszMsg, DnSkillHandle hSkill, DWORD dwColor = textcolor::WHITE, float fFadeTime = 5.0f );
	void ShowItemAlarmDialog( LPCWSTR wszMsg, CDnItem *pItem, int nCount, DWORD dwColor = textcolor::WHITE, float fFadeTime = 5.0f );

	void ShowSymbolAlarmDialog( LPCWSTR wszMsg, CDnItem *pItem, int nCount, DWORD dwColor = textcolor::WHITE, float fFadeTime = 5.0f );
	void ShowMissionAlarmDialog( CDnMissionTask::MissionInfoStruct *pInfo, float fFadeTime = 5.f );
	void ShowPlantAlarmDialog( std::vector<CDnItem *> & vPlant, DWORD dwColor = textcolor::WHITE, float fFadeTime = 5.0f );
	void OpenItemChoiceDialog(const TItem& itemInfo, float totalTime, const DWORD& dropItemUniqueID);
	bool IsShowItemChoiceDialog() const;
	void CloseItemChoiceDialog();

	bool OpenItemUnsealDialog(const MIInventoryItem* item, CDnSlotButton *pSlotButton);
	bool CloseItemUnsealDialog();
	CDnItemUnSealDlg* GetItemUnsealDlg() const { return m_pItemUnsealDlg; }
	bool IsShowItemUnsealDialog() const;

	void ShowItemSealDialog( bool bShow, CDnItem *pSealItem = NULL );
	CDnItemSealDlg *GetItemSealDlg() { return m_pItemSealDlg; }
	bool IsShowItemSealDialog() const;

	void ShowItemPotentialDialog( bool bShow, CDnItem *pPotentialItem = NULL );
	CDnPotentialJewelDlg *GetItemPotentialDlg() { return m_pItemPotentialDlg; }

#ifdef PRE_MOD_POTENTIAL_JEWEL_RENEWAL
	CDnPotentialJewelCleanDlg *GetItemPotentialCleanDlg() { return m_pItemPotentialCleanDlg; }
	bool IsRemovePotentialItemDlg() { return m_bIsRemovePotentialDlg; }
#endif // PRE_MOD_POTENTIAL_JEWEL_RENEWAL

	void ShowGuildRenameDlg( bool bShow, char cInvenType=0, BYTE cInvenIndex=0, INT64 biItemSerial=0);
	CDnGuildRenameDlg *GetGuildRenameDlg() { return m_pGuildRenameDlg; }

	void ShowCharRenameDlg( bool bShow, CDnItem *pItem = NULL );
	CDnCharRenameDlg *GetCharRenameDlg() { return m_pCharRenameDlg; }

	void ShowEnchantJewelDialog( bool bShow, CDnItem *pItem = NULL );
	CDnEnchantJewelDlg *GetEnchantJewelDlg() { return m_pEnchantJewelDlg; }

	void ShowGuildMarkCreateDialog( bool bShow, CDnItem *pItem = NULL );
	CDnGuildMarkCreateDlg *GetGuildMarkCreateDlg() { return m_pGuildMarkCreateDlg; }


#ifdef PRE_ADD_CHANGEJOB_CASHITEM
	void ShowChangeJobDialog( bool bShow, CDnItem *pChangeJobDlg = NULL );
	CDnChangeJobCashItemDlg *GetChangJobDlg() { return m_pChangeJobDlg; }
	bool IsShowChangeJobDialog() const;
#endif // #ifdef PRE_ADD_CHANGEJOB_CASHITEM

#ifdef PRE_ADD_EXCHANGE_POTENTIAL
	void ShowItemPotentialTransferDialog( bool bShow );
	CDnPotentialTransferDlg *GetItemPotentialTransferDlg() { return m_pPotentialTransferDlg; }
#endif
#ifdef PRE_ADD_INSTANT_CASH_BUY
	void ShowInstantCashShopBuyDlg( bool bShow, eInstantCashShopBuyType eBuyType = INSTANT_BUY_NONE, CEtUICallback* pParentCallback = NULL );
#endif // PRE_ADD_INSTANT_CASH_BUY

	void ShowHelpAlarmDialog( int nHelpTableID );
	void ShowTimeEventDialog( bool bShow );

	void ShowChatRoomDialog( bool bShow );
	CDnChatRoomDlg *GetChatRoomDlg() { return m_pChatRoomDlg; }
	void ShowChatRoomPassWordDlg( bool bShow, int nID = -1, CEtUICallback *pCall = NULL );
	LPCWSTR GetChatRoomPassWord();

	void ShowCharmItemKeyDialog( bool bShow, CDnItem *pCharmItem = NULL );
	CDnCharmItemKeyDlg *GetCharmItemKeyDlg() { return m_pCharmItemKeyDlg; }
	void ShowCharmItemProgressDialog( bool bShow, char cInvenType = 0, short sInvenIndex = 0, INT64 biInvenSerial = 0, BYTE cKeyInvenIndex = 0, int nKeyItemID = 0, INT64 biKeyItemSerial = 0 );
	void ShowCharmItemResultDialog( bool bShow, int nItemID = 0, int nCount = 0, int nPeriod = 0 );
	void ShowPetalTokenDialog( bool bShow, CDnItem *pItem = NULL );
	void ShowAppellationGainDialog( bool bShow, CDnItem *pItem = NULL );
#if defined( PRE_ADD_EASYGAMECASH )
	void ShowPackageProgressDlg( char cInvenType, BYTE cInvenIndex, INT64 biInvenSerial );
	void ShowPackageBoxDlg( bool bShow );
#endif	// #if defined( PRE_ADD_EASYGAMECASH )

#ifdef PRE_ADD_GETITEM_WITH_RANDOMVALUE
	void SetPackageBoxInfo( char cInvenType, short sInvenIndex, TRangomGiveItemData* pItemlist, int nitemCount );
	void ShowRandomCharmItemProgressDialog( bool bShow, char cInvenType, short sInvenIndex, INT64 biInvenSerial, TRangomGiveItemData* pItemlist, int nitemCount);
#endif

	void ShowVoiceChatPremiumOptDialog( bool bShow, bool bDisableApplyButton = false );

	void ShowStageClearMoveDialog(bool bShow);
	void TerminateStageClearWarpStandBy();

	void ShowMapCaption( int nMapID );
	void FadeDialog( DWORD srcColor, DWORD destColor, float fFadeTime, CEtUICallback *pCallback = NULL, bool bForceApplySrcColor = false );
	void FadeForBlow( float fFadeTime, DWORD dwColor );

	void ShowCutSceneBlindDialog( bool bShow );
	void SetCutSceneCaption( const wchar_t *wszCaption );
	void ClearCutSceneCaption();

	void OpenBlind();
	void CloseBlind( bool bShowBaseDialog = false );
	bool IsOpenBlind();

#ifdef PRE_ADD_ALTHEIA_NEWCAPTION_TRIGGER
	void OpenImageBlind();
	void CloseImageBlind( bool bShowBaseDialog = false);
#endif // PRE_ADD_ALTHEIA_NEWCAPTION_TRIGGER

	bool IsEnableRender() { return m_bEnableRender; }
	void SelectShortestTime_AcceptRequestDlg(eAcceptReqType type, float& shortest_time, float comparee_time);

	// Note : MainBar Dialog
	void OpenStorageDialog( int iRemoteItemID, bool bGuildStorageOpened = false );
#ifdef PRE_ADD_PVPRANK_INFORM
	void OpenStoreDialog(int nType = Shop::Type::Normal, int nShopID = -1);
#else
	void OpenStoreDialog(int nType = Shop::Type::Normal);
#endif
	void OpenSkillStoreDialog();
	void OpenMarketDialog();
	void OpenMailDialog(bool bWrite, const std::wstring& receiver, const std::wstring& title);
	void OpenItemDisjointDialog();
	void OpenPlateMainDialog();
	void OpenItemUpgradeDialog( int nRemoteItemID = 0 );
#ifdef PRE_ADD_EXCHANGE_ENCHANT
	void OpenItemUpgradeExchangeDialog();
#endif
	void OpenItemCompoundDialog( int nCompoundShopID );
	void OpenItemCompound2Dialog( int nCompoundGroupID, int nRemoteItemID );
	void ShowMiniMap( bool bShow );
#ifdef PRE_ADD_COOKING_SYSTEM
	void OpenCookingDialog( bool bClearSelectedRecipe );
	void CloseCookingDialog();
	bool IsGateQuestionDlgShow();
#endif // PRE_ADD_COOKING_SYSTEM
	void OpenGlyphLiftDialog();
	void OpenInvenDialog();

#ifdef PRE_ADD_COMPARETOOLTIP
	// #69087
	bool IsOpenInvenDialog();
#endif // PRE_ADD_COMPARETOOLTIP

	void AddMessageCoin( INT64 nCoin );
	void AddMessageItem( const wchar_t *szItemName, int nCount );
	void AddMessageSkill( const wchar_t *szSkillName );
	void AddMessageExp( int Exp, const wchar_t *szStrArg = NULL );
	void AddMessageText( const wchar_t *szText , DWORD dTextcolor = textcolor::WHITE);

	// Note : Main Menu
	void ToggleButtonMouseCursor( bool bMouseCursor );
	void ToggleButtonBattleMode( bool bBattleMode );
	void CloseAllMainMenuDialog();
	void CloseModalDialog();

	// Note : Related to death handling
	void OpenRebirthCaptionDlg();
	void OpenRebirthFailDlg();

	void CloseRebirthCaptionDlg();
	void CloseRebirthFailDlg();

	bool IsShowRebirthCationDlg();
	bool IsShowRebirthFailDlg();

	void OpenMissionFailDialog( bool bTimeOut, bool bCanWarpWorldZone );
#ifdef PRE_ADD_STAGE_LIMIT_INTERFACE
	void ShowDungeonLimitDlg( bool bShow, int nMaxIndex );
#endif

	void OpenMissionFailMoveDialog();

	void RefreshRebirthInterface();

	// Note : Common Dialog
	void OpenAssert( const wchar_t *szMsg, CEtUICallback *pCallback );

	bool OpenAcceptDialog( LPCWSTR wszMessage, float fTotalTime, float fElapsedTime, int nID = -1, CEtUICallback *pCall = NULL );
	bool OpenAcceptDialog( int nMsgIndex, float fTotalTime, float fElapsedTime, int nID = -1, CEtUICallback *pCall = NULL );
	void CloseAcceptDialog();
	bool IsOpenAcceptDialog();

	bool OpenPrivateMarketReqDialog( LPCWSTR wszMessage, float fTime, int nID = -1, CEtUICallback *pCall = NULL );
	void ClosePrivateMarketReqDialog();
	bool IsOpenPrivateMarketReqDialog();

	// Translucent dialog at top-center of screen for user-to-user request/accept decisions.
	void OpenAcceptRequestDialog( LPCWSTR wszMessage, float fTotalTime, float fRemainTime, bool bAccept, int type, int nID = -1, CEtUICallback *pCall = NULL );
	void CloseAcceptRequestDialog(bool bReject = false);
#ifdef PRE_ADD_QUICK_PVP
	void TimeUpAcceptRequestDialog();
#endif
	bool IsOpenAcceptRequestDialog();
	CEtUIDialog *GetAcceptRequestDialog() { return (CEtUIDialog *)m_pAcceptRequestDlg; }
	void RefreshQuestSummaryInfo();

	void OpenProgressDialog(LPCWSTR wszMessage, float fTotalTime, float fRemainTime, bool bManualTimeCtrl, bool bManualClose, int nID = -1, CEtUICallback *pCall = NULL);
	bool IsOpenProgressDialog();
	void ForceCloseProgressDialog();

	void ProgressMBox( LPCWSTR wszMessage, float fTotalTime, float fRemainTime, bool bManual, int nID = -1, CEtUICallback *pCall = NULL );
	void SetProgressMBoxCallback(CEtUICallback* pCall);
	bool IsOpenProgressMBox();

	// If a compile error occurs here, a class not inheriting CEtUICallback may have been passed to pCall -> can cause a tricky Overloading Compile Error. (During overloading, the compiler seems to match arguments right-to-left while reporting errors left-to-right.)
	void MessageBox( int nMsgIndex, UINT uType = MB_OK, int nID = -1, CEtUICallback *pCall = NULL, bool bTopMost = true, bool bSmartMoveNoButton = false, bool bNoBtn = false, bool bNoOverwrite = false );
	void MessageBox( LPCWSTR pwszMessage, UINT uType = MB_OK, int nID = -1, CEtUICallback *pCall = NULL, bool bTopMost = true, bool bSmartMoveNoButton = false, bool bNoBtn = false, bool bNoOverwrite = false );
	void MessageBoxTextBox( LPCWSTR pwszMessage, UINT uType = MB_OK, int nID = -1, CEtUICallback *pCall = NULL, bool bTopMost = true, bool bSmartMoveNoButton = false, bool bNoBtn = false, bool bNoOverwrite = false );
	void BigMessageBox( int nMsgIndex, UINT uType = MB_OK, int nID = -1, CEtUICallback *pCall = NULL, bool bTopMost = true, bool bSmartMoveNoButton = false, bool bNoBtn = false, bool bNoOverwrite = false );
	void BigMessageBox( LPCWSTR pwszMessage, UINT uType = MB_OK, int nID = -1, CEtUICallback *pCall = NULL, bool bTopMost = true, bool bSmartMoveNoButton = false, bool bNoBtn = false, bool bNoOverwrite = false );
	void MiddleMessageBox( LPCWSTR pwszMessage, UINT uType = MB_OK, int nID = -1, CEtUICallback *pCall = NULL, bool bTopMost = true, bool bSmartMoveNoButton = false, bool bNoBtn = false, bool bNoOverwrite = false );
	void MiddleMessageBox( int nMsgIndex, UINT uType = MB_OK, int nID = -1, CEtUICallback *pCall = NULL, bool bTopMost = true, bool bSmartMoveNoButton = false, bool bNoBtn = false, bool bNoOverwrite = false );
	void GetServerMessage(std::wstring& msg, int nMsgIndex) const;
	void ServerMessageBox( int nMsgIndex, UINT uType = MB_OK, int nID = -1, CEtUICallback *pCall = NULL, bool bTopMost = true, bool bSmartMoveNoButton = false, bool bNoBtn = false, bool bNoOverwrite = false );
	void CloseMessageBox();

	void OpenPrivateChatDialog( LPCWSTR wszPrivateName );
	void OpenGuildChatDialog( LPCWSTR wszGuildName );

	void ShowHelpKeyboardDialog( bool bShow );
	void ShowDungeonOpenNoticeDialog( bool bShow );

	CDnLifeSkillFishingDlg* GetLifeSkillFishingDlg() { return m_pLifeSkillFishingDlg; }
	CDnDarklairDungeonEnterDlg* GetDarklairDungeonEnterDlg() { return m_pDarkLairEnterDlg; }

	void HideCommonDialog();
	void ShowRecordDialog( bool bShow );

	// Note : Npc Dialog
	void OpenNpcDialog( CEtUICallback *pCallback );
	void CloseNpcDialog();

	// Note : Dungeon
	void OpenStageClearDialog();
	void OpenStageClearReportDialog( CEtUICallback *pCallback );
	void OpenDungeonEnterDialog( CDnWorld::DungeonGateStruct *pGateInfo, CEtUICallback *pCallback );
	void SetSelectDungeonInfo( int nMapIndex, int nDifficult );
#if defined( PRE_ADD_STAGE_CLEAR_ADD_REWARD )
	void OpenDungeonExtraRewardDlg( DWORD dwPropIndex );
#endif	// PRE_ADD_STAGE_CLEAR_ADD_REWARD

	// Note : DarkLair
	void OpenDarkLairEnterDialog( CDnWorld::DungeonGateStruct *pGateInfo, CEtUICallback *pCallback );
	void OpenDarkLariRoundCountDialog( int nRound, bool bBoss, bool bFinalRound );
	void OpenDarkLairClearReportDialog( CEtUICallback *pCallback );
	void OpenDarkLairClearRankDialog();
	void OpenDarkLairClearRewardItemDialog();
	void CloseDarkLairClearRewardItemDialog();
	//	CDnStageClearDlg *GetStageClearDialog()				{ return m_pStageClearDlg; }
	void RequestDarkLairRankInfo();
	void OpenDarkRareRankBoardDialog( TDLRankHistoryPartyInfo* pBestInfo, TDLRankHistoryPartyInfo* pHistoryInfo );
#if defined(PRE_ADD_CHALLENGE_DARKLAIR)
	void SetDarkRareRankBoardMapIndex( const int nMapIndex );
#endif	// #if defined(PRE_ADD_CHALLENGE_DARKLAIR)

	void OpenGuildWarPresentCondition();
	void OpenGuildWarPreliminaryResult();
	void OpenGuildWarFinalTournamentList();
	void SetGuildWarPreliminaryResult();
	void SetGuildWarFinalTournamentList();
	void SetGuildWarSkillUI( UINT uiMyPVPState, bool bResetGuildWarSkillList );
	CDnGuildWarPreliminaryResultDlg* GetGuildWarPreliminaryResult() { return m_pGuildWarPreliminaryResultDlg; }
	CDnGuildWarFinalTournamentListDlg* GetGuildWarFinalTournamentList() { return m_pGuildWarFinalTournamentListDlg; }

	//Note : Second Authenticate
	void OpenSecurityKBDDlg(int State);
	bool OpenSecondAuthDlg(int nChannelID);
	void OpenSecurityChangeDlg();
	//AuthType 0 Login 1 LoginCharDel 2 Game
	void OpenSecurityCheckDlg(InterfaceTypeEnum Type  = InterfaceTypeEnum::Game  , int AuthType = 2);
	void OpenSecurityCreateDlg();
	void OpenSecurityPassCheck();
	void OpenSecurityDelete();
	bool IsShowSecurityCheckDlg();
	bool IsShowSecurityChangeDlg();

	void ShowDungeonMoveDlg(bool bShow, bool bMissionFailMode);
	CDnDungeonClearMoveDlg* GetDungeonMoveDlg() const { return m_pDungeonMoveDlg; }
	void CloseStageClearDialog(bool bLeader);

	// Note : Direct Community UI
	void OpenDirectDialog( DWORD dwSessionID, int nUserLevel, const wchar_t *wszJobName, const wchar_t *wszName );
	void SetDirectDialogPermission( UINT nSessionID, char *pCommunityOption, TPARTYID PartyID );
	void CloseDirectDialog();

	// Note : Inspect UI
	void OpenInspectPlayerInfoDialog( DWORD dwSessionID );
	void CloseInspectPlayerInfoDialog();

	// Note : Login
	void ChangeState( CDnLoginTask::LoginStateEnum state, bool bHideOnly = false );

	void ShowSlotInfoDlg( bool bShow );
#ifdef PRE_MOD_SELECT_CHAR
	CDnCharSelectDlg* GetCharSelectDlg() { return m_pCharSelectDlg; }
	void ShowCharCreateServerNameDlg( bool bShow );
	void ShowCharLoginTitleDlg( bool bShow, int nTitleString = 0 );
#ifdef PRE_ADD_NEWCOMEBACK
	void SetComebackUser( bool bComeback );
#endif // PRE_ADD_NEWCOMEBACK

	void ShowCharCreatePlayerInfo( bool bShow, int nJobClassID = 0 );
	void ShowCharCreateSelectDlg( bool bShow );
	void ResetCharSelectDlgButton();
	bool IsShowCharCreatePlayerInfo();
	void ShowCharCreateBackDlg( bool bShow );
	void ShowCharRotateDlg( bool bShow );
	void SetClassHairColor( int nClassIndex );
	void SetCharCreateSelect( int nClassIndex );
	void SetFocusCharIndex(int nClassIndex);
	void ShowCharGestureDlg(bool bShow, bool bCharCreate);
	void ShowCharOldGestureDlg(bool bShow, bool bCharCreate);

#else // PRE_MOD_SELECT_CHAR
	void ShowSlotInfoDlg( bool bShow, BYTE CharIndex, DnActorHandle hActor, LPCTSTR szMapName );
	void ShowSlotDeleteWaitDlg( bool bShow );
	void ShowSlotDeleteWaitDlg( bool bShow, BYTE CharIndex, __time64_t RemainTime );
	void ShowSlotBlockDlg( bool bShow );
	void ShowSlotBlockDlg( bool bShow, BYTE CharIndex );
	void ShowCreateInfoList( bool bShow );
	void ShowCreateInfoList( bool bShow, int nClassIndex );
#endif // PRE_MOD_SELECT_CHAR

#ifdef PRE_ADD_CHARSELECT_PREVIEW_CASHITEM
	void ShowPreviewCostumeDlg(bool bShow = true);
	void ResetPreviewCostumeRadioButton();
#endif // PRE_ADD_CHARSELECT_PREVIEW_CASHITEM

	void EnableCharSelectDlgControl( bool bEnable, const char *sControlName );
	bool IsEnableCharSelectDlgControl( const char *szControlName );
	void ShowServerSelectDlg( bool bShow );
	void SetWaitUserProperty( LPCWSTR  wszServerName, UINT nWaitUserCount, ULONG nEstimateTime );	
	void ShowWaitUserDlg( bool bShow );
	void ShowCharCreateDlg( bool bShow );
	void EnableCharCreateBackDlgControl( bool bEnable );
	void ShowCashShopDlg(bool bShow);
	void ShowChannelList(bool bShow);
	void ShowServerList(bool bShow);
	bool IsShowCashShopDlg() const;

	// Note : Login
#ifndef PRE_MOD_SELECT_CHAR
	std::wstring GetServerName( int nIndex );
	int GetServerIndex( LPCWSTR szServerName );
#endif // PRE_MOD_SELECT_CHAR
	void ClearServerList();
	void AddServerList( int nIndex, LPCWSTR szServerName, float fUserRatio, int nCharCount );
	void AddChannelList( sChannelInfo *pChannelInfo );
	void SetDefaultServerList();
	void SetDefaultChannelList();

#ifdef PRE_WORLDCOMBINE_PARTY
	void AddServerListToCache(int nIndex, LPCWSTR szServerName);
	void GetServerNameByIndex(std::wstring& serverName, int nIndex) const;
#endif

#ifdef PRE_MOD_CREATE_CHAR
	void SetCharCreatePartsName( CDnParts::PartsTypeEnum PartsIndex, std::wstring strPartsName );
#endif // PRE_MOD_CREATE_CHAR
#ifndef PRE_MOD_SELECT_CHAR
	void ShowCharSelectArrowLeft( bool bShow );
	void ShowCharSelectArrowRight( bool bShow );
#endif // PRE_MOD_SELECT_CHAR
	bool IsShowPrivateMarketDlg();

	// Game, Village Member
	void OnDamage( DnActorHandle hActor );
	void ShowEnemyGauge( DnActorHandle hActor, bool bCrossHair );
	void HideEnemyGauge( DnActorHandle hActor );
	void UpdateBossPortrait( int nIndex );
	void ShowBossGauge( DnActorHandle hActor );
	void ShowGuildWarBossGauge( DnActorHandle hActor );
	void HideBossGauge( int nIndex );
#ifdef PRE_ADD_SHOW_MONACTION
	bool IsShowEnemyGauge(DnActorHandle hActor) const;
#endif

#ifdef PRE_ADD_FADE_TRIGGER
	void ShowFadeScreenCaption( int nUIStringIndex, int nDelay );
#endif // PRE_ADD_FADE_TRIGGER

	void ResetPartyListSort();
#ifdef PRE_PARTY_DB
	const WCHAR* GetPartyListSearchWord();
#endif
	void RefreshPartyList();
	void RefreshPartyInfoList();
	void RefreshPartyMemberList();
	void RefreshPartyGateInfo();
	void RefreshPartyGuage();
	void RefreshPartyVoiceButtonMode( UINT nSessionID, bool bAvailable, bool bMute );
	void RefreshPartySpeaking( UINT nSessionID, bool bSpeaking );
	void RefreshGuildWarSituation();
	CDnGuildWarSituationMng * GetGuildWarSituation()	{ return m_pGuildWarSituation; }
	void HideVoiceButtonMode();
	void SwapPartyDialog();
	int GetPartyListStartIndex();
	void SetPartyInfoLeader( int nLeaderIndex );
	void OnPartyJoin();
	void RefreshStageInfoDlg( bool bRefreshInfo = true );
	void MoveOffsetStageInfoDlg( bool bMove );
	void SetPartyCreateDifficulties(const BYTE* pDifficulties);

	void OnPartyJoinFailed();


#ifdef PRE_MOD_REQ_JOIN_PARTY_ANSWER_MSG_APP
	inline CDnDirectDlg* GetDirectDlg()		{ return ( m_pDirectDlg ? m_pDirectDlg : NULL ); }
#endif

	// Note : Village
	void SetChannelID( int nChannelID );
	void ShowEventToday();
	void CloseEventToday();

	// Note : Game
	// Since it's crit and crit resistance, it's awkward to group by type, so pass it like this for now.
	void SetDamage( EtVector3 vPos, int nDamage, bool bCritical, bool bCriticalRes, bool bEnemy, bool bMine, int hasElement, bool bStageLimit );
	void SetRecovery( EtVector3 vPos, int nHP, int nSP, bool bEnemy, bool bMine );
	void SetResist( EtVector3 vPos );
#ifdef PRE_ADD_DECREASE_EFFECT
	void SetDecreaseEffect( EtVector3 vPos );
#endif // PRE_ADD_DECREASE_EFFECT
#ifdef PRE_ADD_CRAZYDUC_UI
	void SetCrazyDucScore( EtVector3& vPos, int nMonsterID );
#endif 
	void SetCombo( int nCombo, int nDelay );
	void SetChain( int nChain );
	void SetCpJudge(eCpJudgeType type, int nValue);
	void SetCpDown(int nValue);
	void SetCpUp( int nValue );	//	todo : merge this with SetCpJudge by kalliste
	void ResetCp();
	int	 GetCp() const { return m_nAccumulationCPCache; }
	SUICoord GetCpScoreDlgCoord() const;
	SUICoord GetCpScoreDlgBaseCoord() const;
	void MakeCpShowList();
	bool IsCpShow(eCpJudgeType type) const;
	void ClearListDialog();
	void ShowStickAniDialog( bool bShow );
	void ShowBossAlertDialog( bool bShow, WCHAR *wszName = NULL, const char *szImageFileName = NULL );

	bool IsShowPartyPlayerGaugeList();
	void ShowPartyPlayerGaugeList( bool bShow );
	void UpdatePartyPlayerGaugePortrait( UINT nSessionID = 0 );
	void ResetPartyPlayerGaugeList();
	void SetPartyPlayerGaugeAttributes( UINT nSessionID, bool bEnable, bool bMaster, bool bSelect );
	void ResetPartyPlayerGaugeAttributes( bool bMaster, bool bSelection );// Extract into a flag if there get to be many.

	//PVP
	void AddLobbyList( sRoomInfo * RoomInfo , UINT uiMaxPage);
	void RefreshRoomList();
	void CloseLobbyDialog();
	void CloseGameRoomDialog();
	void ChageLobbyState( emLobbyState state );
	CDnBaseRoomDlg * GetGameRoomDlg();
	void SetFirstRoom( bool isFirst );	
	void OnRefreshLocalPlayerStatus();
#ifdef PRE_ADD_IMPROVE_MASTER_OBSERVER
	void InsertPvpPlayerGauge( int nIndex, DnActorHandle hActor, eMiniPartyPlayerGaugeType eType );
	void RemovePvpPlayerGauge( int nIndex, eMiniPartyPlayerGaugeType eType );
	void RefreshPvpPlayerGauge();
#endif
#ifdef PRE_ADD_PVP_SHOW_PARTY_GAUGE
	void ShowPvpPartyControl(bool bShow);
#endif

	void SelectPVPRoom( ePVPRoomState eMode );
	CDnGuildWarRoomDlg * GetGuildWarRoomDlg();

	CDnPVPRoomListDlg* GetPvPRoomListDlg(){return m_pPVPRoomListDlg;}
#ifdef PRE_ADD_PVP_VILLAGE_ACCESS
	bool IsShowPVPVillageAccessDlg();
	void ShowPVPVillageAccessDlg(bool bShow);
#endif

	CDnPVPLadderTabDlg* GetPVPLadderTabDlg(){return m_pPVPLadderTabDlg;}
	CDnPVPLadderSystemDlg* GetPvPLadderSystemDlg(){return m_pPVPLadderSystemDlg;}
	void RefreshLadderInfo();

	CDnPVPLadderRankBoardDlg* GetPVPLadderRankBoardDlg(){return m_pPVPLadderRankBoardDlg;}
	void RequestPVPLadderRankInfo();
	void OpenPVPLadderRankBoardDialog(SCPvPLadderRankBoard* pData);

#if defined( PRE_ADD_REVENGE )
	void ShowPVPRevengeDlg( const std::wstring wszRevengeRequestUser );
#endif	// #if defined( PRE_ADD_REVENGE )

#ifdef PRE_FIX_ESCAPE
	void ShowEscapeDlg( bool bShow = true );
#endif

	//pvp game
	void ShowPVPModeEndDialog( byte cGaemeMode , bool bShow , byte cResult , bool FinalRound = false);
	void OpenpRespawnGauageDlg( PCWSTR pwszMessage, float fTime, bool bText = true );
	void ShowRespawnGauageDlg(bool bShow);
	CDnPVPBaseHUD * GetHUD(){return m_pModeHUD;}
	CDnGuildWarZoneMapDlg * GetGuildWarZoneMapDlg()	{ return m_pGuildWarZoneMapDlg; }
	CDnPVPModeEndDlg * GetRespawnModeEnd(){return m_pPVPModeEndDlg;}
	//void ShowActorGauge( DnActorHandle hActor );
	void ShowPlayerGauge( DnActorHandle hActor , bool bShow );
	void AddPVPGameUer( DnActorHandle hUser);
	void SetPVPGameScore( int nSessionID, int nKOCount, int nKObyCount,int nBossKOCount,UINT uiKOP, UINT uiAssistP, UINT uiTotalP, UINT uiOccupation = 0 );
	void SetPvPGameOccupationScore( int nSessionID, UINT uiOccupation, UINT uiOccupationSteal );
	void SetPvPRacingScore( int nSessionID, DWORD dwLapTime );

	void RemovePVPGameUer( DnActorHandle hUser);
	void SetPVPGameUerState( DnActorHandle hUser , int iState );
	bool IsPVP();
	bool IsPVPLobby();
	bool IsLadderMode();

	void RestartRound(  );
	void TogglePVPMapDlg();
	void OpenFinalResultDlg();
	void ToggleShowAllPVPPlayerInMap( bool bShow );
#ifdef PRE_MOD_PVP_LADDER_XP
	void SetPVPXP( UINT nSessionID, UINT uiAddXPScore, UINT uiTotalXPScore, UINT uiAddMedalScore, UINT uiAddLadderXPScore );
#else // PRE_MOD_PVP_LADDER_XP
	void SetPVPXP( UINT nSessionID, UINT uiAddXPScore, UINT uiTotalXPScore, UINT uiAddMedalScore );
#endif // PRE_MOD_PVP_LADDER_XP

	void ShowPvPKilledMeDlg( bool bShow, WCHAR *wszName , int nKillCount = 0 );
	void ShowOccupationWarnning( DnActorHandle hActor, int nStringID, bool bMapAlarm, bool bReaction, bool bMessage, bool bTeamCheck, int nSoundID = -1 );

	// Note : Guild
	void ShowGuildMgrBox( int nGuildMgrNo, bool bShow = true );
	void ShowGuildCreateDlg( bool bShow = true );
	void ShowGuildDismissDlg( bool bShow = true );
	void ShowGuildLeaveDlg( bool bShow = true );
	void ShowGuildWarEnrollDlg( bool bShow = true );
	void ShowGuildWarFinalWinNoticeDlg( bool bShow, const std::wstring strMsg );
	bool IsShowGuildMgrBox();
	CDnGuildCreateDlg *GetGuildCreateDlg() { return m_pGuildCreateDlg; }

	void OpenGuildInviteReqDlg( LPCWSTR wszGuildName, LPCWSTR wszCharacterName, float fTime, int nID = -1, CEtUICallback *pCall = NULL );
	void CloseGuildInviteReqDlg( bool bSendReject = false );
	bool IsShowGuildInviteReqDlg();
	CEtUIDialog *GetGuildInviteReqDlg() { return (CEtUIDialog *)m_pGuildInviteReqDlg; }
	void ShowGuildLevelUpAlarmDlg( bool bShow, int nLevel, float fFadeTime = 3.0f );

	CDnPetNamingDlg* GetPetNamingDlg() { return m_pPetNamingDlg; }
	CDnPetExtendPeriodDlg* GetPetExtendPeriodDlg() { return m_pPetExtendPeriodDlg; }
#ifdef PRE_ADD_PET_EXTEND_PERIOD
	CDnBuyPetExtendPeriodDlg* GetBuyPetExtendPeriodDlg() { return m_pBuyPetExtendPeriodDlg; }
#endif // PRE_ADD_PET_EXTEND_PERIOD
	void ShowPetExtendPeriodDlg( CDnItem* pItem, bool bShow );
	CDnPetAddSkillDlg* GetPetAddSkillDlg() { return m_pPetAddSkillDlg; }
	void ShowPetAddSkillDlg( CDnItem* pItem, bool bShow );
	CDnPetExpandSkillSlotDlg* GetPetExpandSkillSlotDlg() { return m_pPetExpandSkillSlotDlg; }
	void ShowPetExpandSkillSlotDlg( CDnItem* pItem, bool bShow );
#ifdef PRE_ADD_PET_EXTEND_PERIOD
	void ShowBuyPetExtendPeriodItemDlg( bool bShow, INT64 nPetSerialID );
#endif // PRE_ADD_PET_EXTEND_PERIOD
	void RefreshPetFoodCount();
	// Note : Item
	void SetCharStatusSlotEvent();

	bool IsEmptySlot( DWORD dwDialogID, int nSubSlotIndex = 0 );
	int GetEmptySlot( DWORD dwDialogID, int nSubSlotIndex = 0 );

	// Note : RandomItem
	void OpenRandomItemDlg( char cInvenType, BYTE cInvenIndex, INT64 biInvenSerial );
	void OpenRandomItemResultDlg( char cInvenType, int nItemID, char cLevel, int nItemCount );

	// Note : Skill
	void ChangeSkillPage(int nPage);
	void RefreshSkillDialog();
#ifdef PRE_MOD_SKILLRESETITEM
	void OpenSkillResetConfirmDlg(CDnItem* pItem, int iRewardSP);
#else
	void OpenSkillResetConfirmDlg( INT64 biItemSerial, int iStartJobDegree, int iEndJobDegree, int iRewardSP );
#endif
	CDnSkillResetDlg* GetSkillResetConfirmDlg() { return m_pSkillResetConfirmDlg; }
	void CloseSkillResetConfirmDlg();

	// Master Dialog
	void RefreshMasterListDialog();
	void RefreshMasterSimpleInfo();
	void RefreshMasterCharacterInfo();
	void RefreshMasterClassmate();
	void RefreshPupilList();
	void RefreshMyMasterInfo();
	void RefreshClassmateInfo();
	void RefreshPupilLeave();
	void RefreshRespectPoint();
	void ResetKeepMasterInfo();

	//Plate Dlg
	void RefreshPlateDialog();
	void RefreshGlyphCover();

#if defined(PRE_ADD_TALISMAN_SYSTEM)
	void RefreshTalismanDialog();
	void RefreshTalismanOpenSlot();
	void RefreshTalismanCashSlot();
#endif

	//FarmWareHouse Dlg
	void ShowLifeStorageDialog( bool bShow );

	// Movie Control Dialog
	void OpenMovieDlg( const char *szFileName, float fTimer = -1.f );
	void CloseMovieDlg();

	// Common cutscene movie window.
	void OpenMovieProcessDlg( const char *szMovieFileName, float fTimer, LPCWSTR wszStaticMsg, int nID = -1, CEtUICallback *pCall = NULL, bool bShowButton = true );
	void CloseMovieProcessDlg( bool bDisableFlag = false );	// Since there is cancel, there is also close.
	void OpenMovieAlarmDlg( const char *szMovieFileName, float fTimer, int nItemID, int nEnchantLevel = 0, char cOption = 0, int nFailMsgID = 0 );	// Alarms do not need a callback since they take no input.

	// FocustType Movie Process Dialog
	void OpenFocusTypeMovieProcessDlg( const char *szMovieFileName, float fTimer, LPCWSTR wszStaticMsg, int nID = -1, CEtUICallback *pCall = NULL );
	void CloseFocusTypeMovieProcessDlg( bool bDisableFlag = false );

	// Chat
	void ShowChatDialog();
	void AddChatMessage( eChatType eType, LPCWSTR wszFromCharName, LPCWSTR wszChatMsg, bool bShowCaption = true, bool isAppend = false, bool hasColor = false, DWORD colorValue = 0xffffffff, DWORD bgColorValue = 0 );
	void SendChatMessage( eChatType type, const std::wstring& fromCharName, std::wstring& chatMsg);
	void DelPrivateName( LPCWSTR wszPrivateName );
	bool IsFocusEditBox();

#ifdef PRE_ADD_GACHA_JAPAN
	// Gacha
	void ShowGachaDialog( int nGachaIndex );
	void CloseGachaDialog( void );
	bool IsShowGachaShopDlg( void );
	void OnRecvGachaRes_JP( const SCRefreshCashInven* pPacket );
#endif

	void ShowCostumeMixDlg(bool bShow, bool bCloseInvenTab);
	bool IsShowCostumeMixDlg() const;
	CDnCostumeMixDlg* GetCostumeMixDlg() const;
	void ShowCostumeDesignMixDlg(bool bShow, bool bCloseInvenTab);
	bool IsShowCostumeDesignMixDlg() const;
	CDnCostumeDesignMixDlg* GetCostumeDesignMixDlg() const;
#ifdef PRE_ADD_COSRANDMIX
	void ShowCostumeRandomMixDlg(bool bShow, bool bCloseInvenTab);
	bool IsShowCostumeRandomMixDlg() const;
	CDnCostumeRandomMixDlg* GetCostumeRandomMixDlg() const;
	#ifdef PRE_MOD_COSRANDMIX_NPC_CLOSE
	void ResetCostumeRandomMixDlg();
	#endif // PRE_MOD_COSRANDMIX_NPC_CLOSE
#endif
	// Notice-related message
	void AddNoticeMessage(const WCHAR * pMsg, int nShowSec);

	// World system message (notifying enhancement, mission success, etc.)
	//void AddWorldSystemMessage( char cType, LPCWSTR wszFromCharName, int nID, int nValue = 0 );
	void AddWorldSystemMessage( SCWorldSystemMsg *pPacket );

	// Notify message using the index sent by server
	void AddServerMessage( char cType, char cStringType, int nMsgIndex, char cCaptionType, BYTE cFadeTime );

	//PVP-related UI
	void OpenCreateRoom();

	// Cash related
	// World chat
	void OpenWorldMsgDialog( int nType, INT64 biItemSerial );
	bool IsOpenWorldMsgDialog() const;
	CDnWorldMsgDlg* GetWorldMsgDialog();
	void CloseWorldMsgDialog();

	// UI key settings
	void ApplyUIHotKey();
	void SetMainUIHotKey( int nUIWrappingKeyIndex, BYTE cVK );
	void SetRebirthCaptionHotKey( BYTE cVK );
#ifdef _USE_VOICECHAT
	void SetToggleMicHotKey( BYTE cVK );
#endif

	// CInputReceiver
	void OnInputReceive( int nReceiverState, LOCAL_TIME LocalTime = -1 );

	// CEtUICallback 
	virtual void OnUICallbackProc( int nID, UINT nCommand, CEtUIControl *pControl, UINT uiMsg = 0 );

	// Global CountDown
	// "After X seconds" is automatically prepended to the message. Keep this in mind when setting UI strings.
	// If the string needs to be displayed without 'after N seconds',
	// Use AddChatMessage-CHATTYPE_NOTICE.
	void BeginCountDown( int nSec, LPCWSTR wszString, emCountDownType type = eNumber_Prefix );
	void EndCountDown();
	void ProcessCountDown( float fDelta );

	void UpdateMyPortrait();
	void UpdateTargetPortrait(DnActorHandle hActor);
	void BackupMyPortrait();

	EtTextureHandle CloneMyPortrait();
	CDnGaugeDlg* GetPlayerGauge() { return m_pPlayerGaugeDlg;}
	void RefreshPlayerDurabilityIcon();

	void OpenNotifyArrowDialog(EtVector3 vTarget, LOCAL_TIME nTime, bool bImmediately );
	void CloseNotifyArrowDialog();

	virtual void OnLostDevice();
	virtual void OnResetDevice();

	static void _stdcall OnAutoCursorCallback( bool bShow );

	void CloseNpcTalkReturnDlg();
#ifdef PRE_MOD_ITEM_COMPOUND_DIALOG_RENEWAL
	void ShowNpcTalkReturnDlgButton( bool bShow );
#endif 
#ifdef PRE_ADD_QUEST_BACK_DIALOG_BUTTON
	void ShowQuestReturnButton(bool bIsShow);
#endif

	//Related to PvP rank 
	void SetPvPClassIcon( CEtUITextureControl * pControl, BYTE pvpLevel ); // Set Colosseum rank icon.	
	static int GeticonWidth( ){return iIconWidth;};
	static int GeticonHeight( ){return iIconHeight;};
	static bool ConvertPVPGradeToUV( int iGrade , int &iU , int &iV )
	{
#ifdef PRE_MOD_PVPRANK
		if( iGrade < 1 ) return false;
		iGrade = iGrade - 1;
#else
		if( iGrade < 2 ) return false;
		iGrade = iGrade - 2;
#endif
		if( iGrade < 0 )
			return false;

		int iX = iGrade/iXNum;
		int iLinex = iGrade%iXNum;
		iU = iLinex * iIconWidth;
		iV = iX * iIconHeight;

		return true;		
	};

	EtTextureHandle GetPVPIconTex() { return m_hPVPIconImage[0]; } 
	EtTextureHandle GetPVPIconBigTex() { return m_hPVPIconImage[1]; } 
	EtTextureHandle GetGuildMarkIconTex( int nIndex );
#ifdef PRE_ADD_VIP
	EtTextureHandle GetVIPIconTex() { return m_hVIPIconImage; } 
#endif
	EtTextureHandle GetPartyMemberMarkerTex() { return m_hPartyMemberMarkerImage; }
	EtTextureHandle GetGuildPreliminariesTex() { return m_hGuildPreliminariesImage; }
	EtTextureHandle GetStigmaActorMarketTex() { return m_hStigmaActorMarketImage; }
	EtTextureHandle GetPointMarkTex() { return m_hPointMarkImage; }
	EtTextureHandle GetPointMarkTailTex() { return m_hPointMarkTailImage; }
	CDnNameLinkMng*	GetNameLinkMng() const { return m_pNameLinkMng; };

	void OpenStoreConfirmExDlg(CDnSlotButton* pFromSlot, emSTORE_CONFIRM_TYPE type, CEtUICallback* pCallback);
	void EnableStoreConfirmExDlg(bool bEnable);
	void CloseStoreConfirmExDlg();
	CDnStoreConfirmExDlg* GetStoreConfirmExDlg() const { return m_pSplitConfirmExDlg; }
	void LockMainMenu( bool bLock ) { m_bLockMainMenu = bLock; }
	bool IsLockMainMenu() { return m_bLockMainMenu; }

#ifdef PRE_ADD_VIP
	CDnVIPDataMgr*			GetLocalPlayerVIPDataMgr();
	const CDnVIPDataMgr*	GetLocalPlayerVIPDataMgr() const;
#endif

	CDnContextMenuDlg*		GetContextMenuDlg(eContextMenuType type);
	void					RegisterMsgListenDlg(int protocol, CEtUIDialog* pDlg);
	void					HandleMsgListenDlg(int protocol, char* pData);
	void					DisableCashShopMenuDlg(bool bDisable);

#ifdef PRE_MOD_ITEM_COMPOUND_DIALOG_RENEWAL
	void ForceShowCashShopMenuDlg(bool bShow );
#endif 

	CDnNpcTalkReturnDlg* GetNpcTalkReturnDlg( void ) { return m_pNpcTalkReturnDlg; };

	bool SetNameLinkChat(const CDnItem& item);

	void InitializeMODDialog( int nModTableID );
	void FinalizeMODDialog();
	void ShowMODDialog( bool bShow );
#ifdef PRE_ADD_MONSTER_PARTS_UI_TRIGGER
	void SetLinkMODValue( CEtTriggerElement *pElement, int nIndex, int nDefineValueIndex, int nGaugeType = 0, int nPartsIndex = 0);
#else
	void SetLinkMODValue( CEtTriggerElement *pElement, int nIndex, int nDefineValueIndex );
#endif

	DWORD GetCharNameColor(const WCHAR* pName) const;

	void ShowDurabilityGauge(DnActorHandle hActor, int nStateBlowID, float fDurability, bool bShow);

#if defined(PRE_ADD_23829)
	void Show2ndPassCreateButton(bool bSecondAuthNotifyShow);
#endif

	void OpenBrowser(std::string& url, float fPageWidth, float fPageHeight, eBrowserPosType posType, eGeneralBrowserType type);
#ifdef PRE_ADD_GUILD_EASYSYSTEM
	ULONG ProcIDFromWnd(HWND hwnd);
	HWND GetWinHandle(ULONG pid);
	void OpenSimpleBrowser(const std::wstring& wUrl);
#endif
#ifdef PRE_MOD_BROWSER
	void OnWndProcForInternetBrowser(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	void OnCloseBrowser(eBrowserType type);
#else
	void OnCloseBrowser(eGeneralBrowserType type);
#endif

	void RefreshRepUnionMembershipMark(const std::vector<SUnionMembershipData>& membershipList);
	void ShowRepUnionMarkTooltip(int tableIdx, float fMouseX, float fMouseY);
	void CloseRepUnionMarkTooltip();

#if defined(PRE_ADD_VIP_FARM)
	void ShowLifeUnionMarkTooltip( float fMouseX, float fMouseY );
	void CloseLifeUnionMarkTooltip();
#endif
	void RefreshGuildWarMark(bool bShow, bool bNew);
	void ShowGuildWarMarkTooltip(float fMouseX, float fMouseY);
	void CloseGuildWarMarkTooltip();

#if defined(PRE_ADD_WEEKLYEVENT)
	void RefreshWeeklyEventMark(bool bShow, bool bNew);
	void ShowWeeklyEventMarkTooltip(float fMouseX, float fMouseY);
	void CloseWeeklyEventMarkTooltip();
#endif

	void RefreshGuildRewardMark(bool bShow, bool bNew);
	bool IsMarkedGuildReward();


	void ShowGuildRewardMarkTooltip(float fMouseX, float fMouseY);
	void CloseGuildRewardMarkTooltip();

	void ShowPlayerGuildInfo(bool bShow);

	void UpdateGuildRewardInfo();

	bool IsShowChannelDlg() const;
	bool IsFading() const;
	void ProgressSimpleMsgBox(LPCWSTR pwszMessage, float heartBeatMsgSec, bool bLoadingAni, int nID = -1, CEtUICallback *pCall = NULL, float aniSecForRound = -1.f);
	void CloseProgressSimpleMsgBox();
	CDnProgressSimpleMessageBox* GetProgressSimpleMsgBox() const;
#ifdef PRE_ADD_LEVELUP_GUIDE
	void ShowLevelUpGuide(bool bShow, int nNewLevel);
#endif

#if defined(PRE_ADD_REMOVE_PREFIX)
	CDnRemovePrefixDlg *GetRemovePrefixDlg() { return m_pRemovePrefixDlg; }
	void ShowRemovePrefixDialog( bool bShow, CDnItem *pItem = NULL );
#endif // PRE_ADD_REMOVE_PREFIX

	void ShowHardCoreModeTimeAttackDlg( bool bShow );
	void SetHardCoreModeRemainTime( int nOriginTime, int nRemainTime );
	void ResetHardCoreModeTimer();
	void SetHardCoreModeHurryUpTime( int nTime );

	CDnNestInfoDlg* GetNestInfoDlg() { return m_pNestInfoDlg; }

	void SetLastTryUseMapMoveCashItem( CDnItem* pUsedItem );
	void ShowMapMoveCashItemDlg( bool bShow, const WarpVillage::WarpVillageInfo* pVillageList, int iListCount );
	void ResetMapMoveOKBtn( void );
	bool IsShowMapMoveCashItemDlg( void );

#ifdef PRE_ADD_DONATION
	void OpenContributionDialog( bool bShow );
	bool IsOpenDonation();
#endif

#ifdef PRE_ADD_CHAOSCUBE
	void OpenChaosCubeDialog( bool bShow, CDnItem * pItem );
	class CDnChaosCubeDlg * GetChaosCubeDlg(){
		return m_pChaosCubeDlg;
	}
	bool IsOpenChaosCubeDlg();
	void UpdateChaosCubeDlg();
	void OpenChaosCubeProgressDlg( bool bShow, struct SCChaosCubeRequest * pData );
	void OpenResultDlg( bool bShow, int nItemID, int nCount );
	void MixComplete(); // Mix complete.
	void RecvResultItemComplet(); // Result item received.
	void RecvMixStart( bool bShow ); // Enable/disable mix button.

	void OpenStuffDlg( bool bShow, int nItemID, int nCount );
	void OpenProgressDlg( bool bShow, int nItemID, int nCount );	

	class CDnChaosCubeStuffDlg * GetChaosCubeStuffDlg(){
		return m_pChaosCubeStuffDlg;
	}

	void OpenChaosGaugeCubeDialog( bool bShow, CDnItem * pItem );
	class CDnChaosGaugeCubeDlg * GetChaosGaugeCubeDlg(){
		return m_pChaosGaugeCubeDlg;
	}
	bool IsOpenChaosGaugeCubeDlg();
	void UpdateChaosGaugeCubeDlg();

#endif
#if defined( PRE_ADD_DWC )
	class CDnDwcDlg *GetDwcDlg() const { return m_pDwcDlg; }
	class CDnDwcCreateTeamDlg *GetDwcCreateTeamDlg() const { return m_pDwcCreateTeamDlg; }
	class CDnDwcInviteReqDlg *GetDwcInviteReqDlg() const { return m_pDwcInviteReqDlg; }
	class CDnDWCCharCreateDlg *GetDWCCharCreateDlg() const { return m_pDWCCharCreateDlg; }
	class CDnPVPDWCTabDlg *GetPVPDWCTabDlg() const { return m_pPVPDWCTabDlg; }
	class CDnDWCMarkTooltipDlg *GetDWCMarkTooltipDlg() const { return m_pDWCMarkTooltipDlg; }
	class CDnDWCMarkInformer *GetDWCMarkInformer() const { return m_pDWCMarkInformer; }
	void OpenDwcDlg( bool bShow );
	void OpenDwcCreateTeamDlg( bool bShow );
	void OpenDWCCharCreateDlg( bool bShow );
#endif
	class CDnMainQuestDlg *GetMainQuestDlg() const { return m_pMainQuestDlg; }
	class CDnJobInformationMainDlg *GetJobInformationMainDlg() const { return m_pJobInformationMainDlg; }
	class CDnSkillClassInfoDlg *GetSkillClassInfoDlg() const { return m_pSkillClassInfoDlg; }
	class CDnSkillCountMng *GetSkillCountMng() const { return m_pSkillCountMng; }
	class CDnSubGaugeDlg *GetSubGaugeDlg() const { return m_pSubGaugeDlg; }
	class CDnNPCHotKeyInfoDlg *GetNPCHotKeyInfoDlg() const { return m_pNPCHotKeyInfoDlg; }
	class CDnDungeonEnterNotifierDlg *GetDungeonEnterNotifierDlg() const { return m_pDungeonEnterNotifierDlg; }
	class CDnNestNotifierDlg *GetNestNotifierDlg() const { return m_pNestNotifierDlg; }
	class CDnStageShortCutRepairDlg *GetStageShortCutRepairDlg() const { return m_pStageShortCutRepairDlg; }
	class CDnPackageBoxResultAutoDlg *GetPackageBoxResultAutoDlg() const { return m_pPackageBoxResultAutoDlg; }
	class CDnPotenCharmItemProgressDlg *GetPotenCharmItemProgressDlg() const { return m_pPotenCharmItemProgressDlg; }
	class CDnSecretCashShopAlarmButtonDlg *GetSecretCashShopAlarmBtnDlg() const { return m_pSecretCashShopAlarmBtnDlg; }
	class CDnSecretCashShopBuyDlg *GetSecretCashShopBuyDlg() const { return m_pSecretCashShopBuyDlg; }
	class CDnSecretCashShopMessageDlg *GetSecretCashShopMsgDlg() const { return m_pSecretCashShopMsgDlg; }
	class CDnItemsOwnedAlarmMng *GetItemsOwnedAlarmMng() const { return m_pItemsOwnedAlarmMng; }
	class CDnInputContextMessageBox *GetInputContextMessageBox() const { return m_pInputContextMessageBox; }
	class CDnMetaMissionTooltipDlg *GetMetaMissionTooltipDlg() const { return m_pMetaMissionTooltipDlg; }
	class CDnMissionAchieveEventListBoxDlg *GetMissionAchieveEventListBoxDlg() const { return m_pMissionAchieveEventListBoxDlg; }
	class CDnPremiumPcBangNotifyToolTipDlg *GetPremiumPcBangNotifyToolTipDlg() const { return m_pPremiumPcBangNotifyToolTipDlg; }


#ifdef PRE_ADD_CHOICECUBE
	bool IsOpenChoiceCubeDlg();
	void UpdateChoiceCubeDlg();
	void OpenChoiceCubeDialog( bool bShow, CDnItem * pItem );	
	void OpenChoiceCubeProgressDlg( bool bShow, SCChaosCubeRequest * pData );
	class CDnChoiceCubeDlg * GetChoiceCubeDlg(){ 
		return m_pChoiceCubeDlg;
	}
	void RecvChoiceResultItemComplet();
	void RecvChoiceMixStart( bool bShow );
#endif


#ifdef PRE_ADD_AUTOUNPACK

	class CDnAutoUnPackDlg * GetAutoUnPackDlg(){
		return m_pAutoUnPackDlg;
	}
	bool IsOpenAutoUnPackDlg();

	// nTime : open delay time % per item.
	void OpenAutoUnPack( INT64 sn, bool bShow, int nTime=0.0f );

	// Start auto-open.
	void UnPacking();

#endif
	bool IsShowGateReady();

#ifdef PRE_ADD_CASHREMOVE

	void OpenCashItemRemoveDialog( bool bShow, MIInventoryItem * pItem ); // Remove cash item.
	void OpenCashItemRestoreDialog( bool bShow, MIInventoryItem * pItem );// Restore cash item.

	void RemoveCashItemAccept(); // Accept cash item deletion.
	void RemoveCashItemCancel(); // Cancel cash item deletion.	

#endif


#ifdef PRE_ADD_BESTFRIEND // Best friend system.

	bool IsOpenBFRegistrationDlg();
	INT64 GetBFserial(); // Best friend certificate item Serial..
	void OpenBFRegistrationDlg( bool bShow, INT64 serial ); // Best friend certificate.

	void OpenIdentifyBF( bool bShow ); // Best friend confirm Dlg.
	void SetSearchBF( struct BestFriend::SCSearch * pData ); // Best friend confirm info.

	void OpenBFProgressDlg( bool bShow ); // Best friend accept ProgressDlg.
	void SetBFProgressData( struct BestFriend::SCRegistReq * pData, bool bOpen=true );

	void SetBFData( struct TBestFriendInfo & info ); // Best friend info.

	void OpenBFRewardDlg( bool bShow, INT64 giftSerial, int typeParam1 );       // Reward item Dlg.	
	void OpenBFRewardProgressDlg( bool bShow, INT64 giftSerial ); // Reward item ProgressDlg.
	void SendRewardComplet( int nItemID ); // Reward item grant complete.

	void OpenBFMemo( bool bShow ); // Memo Dlg.
	void SetMemoBF( struct BestFriend::SCEditMemo * sMemo ); // Receive memo.

	const wchar_t * GetNameBF(); // Best friend name.
	INT64 GetSerialBF(){ // Best friend ring serial.
		return m_serialBF;
	}

	void BrokeupOrCancelBF( bool bCancel, WCHAR * strName ); // Break best friend or cancel break.

	void DestroyBF(); // Dissolve best friend.

#endif


	// Is it equipped?
	bool IsEquipped( CDnItem * pItem, bool bCash );

#ifdef PRE_MOD_BROWSER
	void DisableAllDlgs(bool bEnable, const std::wstring& text, float delayTime = 0.f);
	void ProcessDisableDlgs(float fElapsedTime);
#endif
#ifdef PRE_ADD_DOORS
	void OnSetDoorsAuthMobileMsgBox(const SCDoorsGetAuthKey& data);
	void OnSetDoorsCancelAuthMobilMsgBox(const SCDoorsCancelAuth& data);
	void OnSetDoorsAuthFlag(const SCDoorsGetAuthFlag& data);
#endif

#ifdef PRE_ADD_COMBINEDSHOP_PERIOD
	bool IsOpenStore(); // Shop Dlg.
#endif // PRE_ADD_COMBINEDSHOP_PERIOD
#if defined(PRE_ADD_SERVER_WAREHOUSE)
	bool IsOpenStorage(); // Storage Dlg.
#endif

#if defined(PRE_ADD_MAILBOX_OPEN) || defined(PRE_ADD_68286)
	void OpenMailDialogByShortCutKey();
#endif //PRE_ADD_MAILBOX_OPEN

#ifdef PRE_MOD_PVPOBSERVER	
	bool IsEventRoom();
	void ResetPvpObserverJoinFlag();
#endif // PRE_MOD_PVPOBSERVER

#ifdef PRE_ADD_PVP_TOURNAMENT
	void SwitchPVPRoomDlg();
	void UpdatePVPTournamentGameMatchUserList();
#endif

#if defined(PRE_ADD_TOTAL_LEVEL_SKILL)
	void UpdateTotalLevelSkill();
	void AddTotalLevelSkill(int nSlotIndex, DnSkillHandle hSkill);
	void RemoveTotalLevelSkill(int nSlotIndex);

	void ActivateTotalLevelSkillSlot(int nSlotIndex, bool bActivate);
	void ActivateTotalLevelSkillCashSlot(int nSlotIndex, bool bActivate, __time64_t tExpireDate);
#endif // PRE_ADD_TOTAL_LEVEL_SKILL

	bool IsSkipInteraction();


#ifdef PRE_ADD_PVP_RANKING
	void OpenPvPRankDlg();
	void ClosePvPRankDlg();
	bool IsShowPvPRankDlg();
	class CDnPvPRankDlg *GetPvPRankDlg() const { return m_pPvPRankDlg; }
	
	void SetInfoMyRankColosseum( struct TPvPRankingDetail * pInfo );    // My info - Colosseum.
	void SetInfoMyRankLadder( struct TPvPLadderRankingDetail * pInfo ); // My info - Ladder.

	void SetInfoColosseum( struct TPvPRankingDetail * pInfo );    // Character info - Colosseum.
	void SetInfoLadder( struct TPvPLadderRankingDetail * pInfo ); // Character info - Ladder.

	void SetListColosseum( struct SCPvPRankList * pInfo );   // List - Colosseum.
	void SetListLadder( struct SCPvPLadderRankList * pInfo );// List - Ladder.

	void OpenRightAlramDlg( bool bShow ); // Right-side screen notifier windows - missions, quests, crafting, etc..
#endif // PRE_ADD_PVP_RANKING


#ifdef PRE_ADD_DIRECT_BUY_UPGRADEITEM
	void SetDirectBuyUpgradeItemDlg(SCMarketMiniList* pPacket);
	CDnMarketDirectBuyDlg* GetDirectBuyUpgradeItem() { return m_pMarketDirectBuyDlg; }
#endif


#ifdef PRE_ADD_NEWCOMEBACK
	void OpenComebackRewardDlg( bool bShow );	
	void SelectedCharIndex( int nIndex );
	void SetComebackLocalUser( bool bComebackUser ){
		m_bComebackUser = bComebackUser;
	}
	bool GetComebackLocalUser(){
		return m_bComebackUser;
	}
	void OpenComebackMsgDlg( bool bShow ); // Welcome message Dlg shown when a returning user logs in for the first time.
#endif // PRE_ADD_NEWCOMEBACK


#if defined(PRE_ADD_68286)
	void SetDisableChatTabMsgproc(bool isDisable);
	void OpenCashShopRecvGiftWindow();
#endif // PRE_ADD_68286

#if defined(PRE_ADD_68286)
	bool IsAlarmIconClick(POINT& MousePoint);	
#endif // PRE_ADD_68286

#ifdef PRE_ADD_ACTIVEMISSION
	void ShowActiveAlarmDlg( bool bShow, std::wstring & str, float fadeTime=1.0f );
	void RefreshActiveMissionEventMark( int acMissionID, std::wstring & str, bool bShow, bool bNew);
	void ShowActiveMissionEventMarkTooltip( std::wstring & str, bool bShow, float fMouseX=0.0f, float fMouseY=0.0f );
	void CloseActiveMissionEventMarkTooltip();	
#endif // PRE_ADD_ACTIVEMISSION

#ifdef PRE_ADD_QUESTCOUNT_IN_STAGELIST
	CDnWorldZoneSelectDlg* GetWorldZoneSelectDlg() { return m_pWorldZoneSelectDlg ? m_pWorldZoneSelectDlg : NULL; }
#endif

#ifdef PRE_ADD_CASHSHOP_ACTOZ
	void ShowConfirmRefund( CDnItem * pItem );
#endif // PRE_ADD_CASHSHOP_ACTOZ
	
#ifdef PRE_ADD_STAMPSYSTEM	
	void NPCSearch( int mapID, int npcID );	
	void ChangeWorldMap();
	void ShowStampDlg();

	void AddStamp( bool bOne=false, bool bForce=false ); // Complete one achievement.
	void ShowStampAlarmDlg( bool bShow );	

	class CDnStampEventDlg * GetStampEventDlg();	
	void ShowDailyQuestTab(); // Open the daily quest tab in the quest window.
#endif // PRE_ADD_STAMPSYSTEM

#if defined( PRE_ALTEIAWORLD_EXPLORE )
	void Initialize_AlteaBoard();
	void Show_AlteaBoard( bool bShow );
	CDnAlteaBoardDlg * GetAlteaBoard() { return m_pAlteaBoardDlg; }
	void SetAlteaDiceIcon();
	void ShowAlteaClearDlg( bool bClear );
#endif	// #if defined( PRE_ALTEIAWORLD_EXPLORE )

#ifdef PRE_ADD_GAMEQUIT_REWARD
	bool ShowGameQuitRewardDlg( bool bShow, int type ); // Immediate item grant Dlg.
	void ShowGameQuitNextTimeRewardDlg( bool bShow, int type, bool bCharSelect=false, int nLevel=0, int nClass=-1 ); // Dlg for items receivable later.

	void ShowGameQuitRewardComboDlg( bool bShow ); // New account game exit reward character selection Dlg.	
	void SelectedGameQuitRewardCharIndex( int charIndex ); // Index of character selected in character selection screen.
	void ShowNewbieGameQuitRewardMessage();	
	void ResetCharacterListGameQuitRewardComboDlg(); // Initialize character list.
	void RecvReward();
#endif // PRE_ADD_GAMEQUIT_REWARD

#if defined( PRE_ADD_PART_SWAP_RESTRICT_TRIGGER )
	void SetPartSwapRestrict(bool bSwitch) { m_bIsPartSwapRestrict = bSwitch; }
	bool IsPartSwapRestrict() { return m_bIsPartSwapRestrict; }

#endif // PRE_ADD_PART_SWAP_RESTRICT_TRIGGER 

#ifdef PRE_ADD_RETURN_VILLAGE_SHOWNAME
	void SetReturnVillage( int nVillage )	{ m_nVillage = nVillage; }
#endif 
#ifdef PRE_ADD_START_POPUP_QUEUE
	CDnStartPopupMgr& GetStartPopupMgr() { return m_StartPopupMgr; }
#endif

#ifdef PRE_ADD_PVPRANK_INFORM
	void ShowPVPRankInformDlg(bool bShow);
#endif

#ifdef PRE_ADD_CASHSHOPBANNER_RENEWAL
	void ShowIngameBanner( bool bShow );
#endif // PRE_ADD_CASHSHOPBANNER_RENEWAL


public:








};

#define GetInterface()	CDnInterface::GetInstance()