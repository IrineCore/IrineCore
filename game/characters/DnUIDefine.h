#pragma once

#include "DnBaseRoomDlg.h" // todo by kalliste : Move CDnBaseRoomDlg::SlotState inside SMatchListInfo here as well.

enum eCharmItemSystemMsgType
{
	eTYPE_NORMAL,
	eTYPE_TREASURE,
};

namespace PvPTournamentUIDef
{
	enum eMatchModeByCount
	{
		eMODE_4,
		eMODE_8,
		eMODE_16,
		eMODE_32,
		eMODE_ONLYBYGM = eMODE_32,
		eMODE_MAX,
	};

	enum eConstance
	{
		INVALID_MEMBERCOUNT = -1,
		MAX_MATCH_MEMBERCOUNT = 32,
		MAX_MATCH_SIDE_COUNT = (MAX_MATCH_MEMBERCOUNT / 2),
		MAX_WINNERS_FOR_FINAL_RESULT_DLG = 4,
		LOSERS_FIRST_RANK = 4,
	};

	enum ePVPTournamentError
	{
		ePTERR_NONE = 0,
		ePTERR_CANT_KICK_SELF = 20000,
		ePTERR_CANT_KICK_NO_SELECT,
		ePTERR_CANT_CHANGEMASTER_SELF,
		ePTERR_CANT_CHANGEMASTER_NO_SELECT,
	};

	enum eUserState
	{
		Normal_S = 0,
		MINE_S = 1,
		KILLED_S = 2,
	};

	enum eMedalIconSize
	{
		PVP_MEDAL_ICON_XSIZE = 23,
		PVP_MEDAL_ICON_YSIZE = 23,
	};

	struct SBasicRoomInfo
	{
		UINT uiMapIndex;
		UINT uiGameModeID;
		UINT uiRoomState;
		UINT uiWinCondition;
		UINT uiRoomIndex;
		int nEventRoomID;

		BYTE cMaxLevel;
		BYTE cMinLevel;
		BYTE cMaxPlayerNum;

		bool bBreakIntoFlag;
		bool bDropItem;
		bool bRandomTeam;

		bool bShowHp;
		bool bRevision;

		std::wstring roomName;

#ifdef PRE_WORLDCOMBINE_PVP
		int nWorldPvPRoomType;
#endif // PRE_WORLDCOMBINE_PVP
#ifdef PRE_PVP_GAMBLEROOM
		BYTE cGambleType;
		int nGamblePrice;
#endif // PRE_PVP_GAMBLEROOM
		void Reset();
		void Set(const PvPCommon::RoomInfo& info);
	};

	struct SMatchUserInfo
	{
		int commonIndex;
		int matchListUIIndex;
		CDnBaseRoomDlg::SlotState slotState;
		int  nJobID;
		BYTE cLevel;
		BYTE cPVPLevel;
		UINT uiUserState;
		UINT uiSessionID;
		bool bIsNeedUpdate;
		bool bShow;
		std::wstring playerName;
#if defined( PRE_ADD_REVENGE )
		UINT uiRevengeUserID;
#endif	// #if defined( PRE_ADD_REVENGE )

		SMatchUserInfo();
		bool IsEmpty() const { return (matchListUIIndex == -1); }
	};

	struct SGameMatchUserInfo
	{
		bool bCurrentMatchUser;
		int step;
		bool bWin;
		std::wstring charName;
		int jobId;
		UINT sessionId;
		SGameMatchUserInfo();
	};

	struct SFinalReportUserInfo
	{
		int nSessionID;
		int nLevel;
		int nState;
		byte cJobClassID;
		byte cPVPlevel;

		std::wstring wszUserName;
		TGuildSelfView GuildSelfView;

#ifdef PRE_ADD_IMPROVE_MASTER_OBSERVER
		DnActorHandle hActor;
#endif
		UINT uiKOCount;		// Kill
		UINT uiKObyCount;	// Death
		UINT uiKOP;			// Seems to be kill score.
		UINT uiAssistP;		// Assist score
		UINT uiTotalScore;	// Total sum
		UINT uiXP;			// EXP
		UINT uiTotalXP;
		UINT uiMedal;		// Medal count
		UINT uiBossKOCount;	// Death
#if defined( PRE_ADD_REVENGE )
		UINT eRevengeUser;
#endif	// #if defined( PRE_ADD_REVENGE )
		int tournamentStep;
		int winnersRank;

		SFinalReportUserInfo();
	};
}

namespace StorageUIDef
{
	enum eStorageTabType
	{
		eTAB_PLAYER = 1,
		eTAB_GUILD,
		eTAB_WORLDSERVER_NORMAL,
		eTAB_WORLDSERVER_CASH,
		eTAB_MAX,
	};

	enum eCommon
	{
		eChargeCoinItemID = 1073764750,
	};

	enum eError
	{
		eERR_NONE = 0,
		eERR_NOETERNITY = 4082,
		eERR_BELONG = 3836,
		eERR_NODRAG = 1632,
		eERR_SERVERWARE = 1616,
	};
}

namespace PvPRankInfoUIDef
{
	enum eCommon
	{
		eMAX_REWARD_ITEM_COUNT = 2,
		eMIN_PVPRANK_SHOPID = 5501,	// temp by kalliste : Not ideal, but hard to set a separate type to distinguish Colosseum Elite Shop, so apply this for now. Needs future fix.
		eMAX_PVPRANK_SHOPID = 5506
	};

	struct SRankRewardItemUnit
	{
		ITEMCLSID id;
		int count;

		SRankRewardItemUnit() { id = ITEMCLSID_NONE; count = 0; }
	};

	struct SRankUnit
	{
		int level;
		DWORD lineColor;
		std::wstring name;
		int needExp;
		std::vector<SRankRewardItemUnit> rewards;

		SRankUnit() { level = 0; needExp = -1; lineColor = 0; }
		void Clear() { level = 0; name.clear(); rewards.clear(); needExp = -1; lineColor = 0; }
	};
}