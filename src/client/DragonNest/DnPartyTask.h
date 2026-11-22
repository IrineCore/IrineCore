#pragma once

#include "Task.h"
//#include "ClientTcpSession.h"
//#include "ClientUdpSession.h"
#include "MessageListener.h"
#include "DNProtocol.h"
#include "DNPacket.h"
#include "DnWorld.h"
#include "DnPartyData.h"
#ifdef PRE_MOD_INTEG_SYSTEM_STATE
#include "DnAcceptRequestInfo.h"
#include "DnAcceptRequestInviteInfo.h"
#include "DnAcceptRequestDataManager.h"
#endif

#define INVALID_GATE_INDEX -1

struct TItemInfo;
class CDnPartyTask : public CTask, public CTaskListener, public CEtUICallback, public CSingleton<CDnPartyTask>
{
public:
	CDnPartyTask();
	virtual ~CDnPartyTask();	

	enum PartyRoleEnum
	{
		SINGLE,
		LEADER,
		MEMBER,
	};

	enum eErrorPartyStartGame
	{
		ENABLE,
		FATIGUE,
	};

	enum ePartyState
	{
		NONE,
		NORMAL,
		STAGE_CLEAR_WARP_STANDBY,
	};

	enum ePartySoundType
	{
		CREATE,
		ENTER,
		KICK,
		EXILE,
		LOOTCHANGE,
		REQUESTREADY,
		SOUNDMAX
	};

	enum ePartyProcessState
	{
		PPS_NONE			= 0x0000,
		PPS_ACCEPT_INVITE	= 0x0001,
	};

#ifdef PRE_ADD_NAMEDITEM_SYSTEM
	// In village, party member buff info cannot be distinguished by actor info.
	// Party member buff info must be managed separately only when in a party to display it / Used for named items.
	struct VillagePartyEffectSkillInfo
	{
		int nItemID;
		int nSkillID;
		float fRemainTime;
		bool bEternity;
	};
#endif

	struct PartyStruct : public SPartyMemberInfo
	{
		DnActorHandle hActor;
		int	nEnteredGateIndex;
		bool bSync;
		bool bPartyMaster;
		bool bGMTrace;
		USHORT usTeam;
		bool bSyncHPSP;
		INT64 iSyncHP;
		int	iSyncSP;
		BYTE cSyncBattleMode;
		bool bCompleteBreakInto;

		int nDefaultPartsIndex[4];
		char cViewCashEquipBitmap[VIEWCASHEQUIPMAX_BITSIZE];
		std::vector<TItemInfo> VecParts;
		TItemInfo Weapon[2];
		std::vector<TItemInfo> VecCashParts;
		TItemInfo CashWeapon[2];
		std::vector<TItemInfo> VecGlyph;
#if defined(PRE_ADD_TALISMAN_SYSTEM)
		std::vector<TItemInfo> VecTalisman;
#endif	// #if defined(PRE_ADD_TALISMAN_SYSTEM)

		TVehicleCompact VehicleInfo;
		TVehicleCompact PetInfo;

		std::vector<int> nVecJobHistoryList;
		std::vector<TSkill> VecSkill;
#ifdef PRE_ADD_NAMEDITEM_SYSTEM
		std::vector<VillagePartyEffectSkillInfo> vecEffestSkillData;
#endif
		EtVector3 vPos;

		// Temporary storage for selected title; used only during creation.
		int nSelectAppellation;
		int nSelectCoverAppellation;
#if defined(PRE_ADD_VIP)
		bool bVIP;
#endif	// #if defined(PRE_ADD_VIP)
		char cAccountLevel;

		// Guild info
		TGuildSelfView GuildSelfView;
		bool bAbsence;

#ifdef PRE_MOD_SYNCPACKET
		std::vector <std::pair<BYTE, BYTE>> VerifyPartyStruct;
#endif		//#ifdef PRE_MOD_SYNCPACKET

		PartyStruct()
		{
			nEnteredGateIndex	= -1;
			bSync				= false;
			bPartyMaster		= false;
			bGMTrace			= false;
			usTeam				= 0;
			bSyncHPSP			= false;
			iSyncHP				= 0;
			iSyncSP				= 0;
			cSyncBattleMode		= 0;
			memset( Weapon, 0, sizeof(TItemInfo)*2 );
			memset( CashWeapon, 0, sizeof(TItemInfo)*2 );
			memset( wszCharacterName, 0, sizeof(wszCharacterName) );
			memset( nDefaultPartsIndex, 0, sizeof(nDefaultPartsIndex) );
			vPos = EtVector3( 0.f, 0.f, 0.f );
			wFatigue			= 0;
			nHairID				= 0;
			nFaceID				= 0;
			nHelmetID			= 0;
			dwHairColor			= 0;
			dwEyeColor			= 0;
			dwSkinColor			= 0;
			nSelectAppellation  = -1;
			nSelectCoverAppellation = -1;
			bCompleteBreakInto = false;
			memset( cViewCashEquipBitmap, 0, sizeof(cViewCashEquipBitmap) );
			bAbsence = false;
#if defined(PRE_ADD_VIP)
			bVIP = false;
#endif	// #if defined(PRE_ADD_VIP)
			cMemberIndex = 0;
			uiPvPUserState = 0;
#ifdef PRE_WORLDCOMBINE_PARTY
			nWorldSetID = Party::Constants::INVALID_SERVERID;
#endif
#ifdef PRE_ADD_NEWCOMEBACK
			bCheckComebackAppellation = false;
#endif // PRE_ADD_NEWCOMEBACK
			cAccountLevel = 0;
			memset(&PetInfo, 0, sizeof(PetInfo));
			memset(&VehicleInfo, 0, sizeof(VehicleInfo));

#ifdef PRE_MOD_SYNCPACKET
			VerifyPartyStruct.push_back(std::make_pair(static_cast<BYTE>(SC_ROOM), static_cast<BYTE>(eRoom::SC_SYNC_MEMBERHPSP)));
			VerifyPartyStruct.push_back(std::make_pair(static_cast<BYTE>(SC_ROOM), static_cast<BYTE>(eRoom::SC_SYNC_MEMBERBATTLEMODE)));
			VerifyPartyStruct.push_back(std::make_pair(static_cast<BYTE>(SC_PARTY), static_cast<BYTE>(eParty::SC_DEFAULTPARTSDATA)));
			VerifyPartyStruct.push_back(std::make_pair(static_cast<BYTE>(SC_PARTY), static_cast<BYTE>(eParty::SC_WEAPONORDERDATA)));
			VerifyPartyStruct.push_back(std::make_pair(static_cast<BYTE>(SC_PARTY), static_cast<BYTE>(eParty::SC_EQUIPDATA)));
			VerifyPartyStruct.push_back(std::make_pair(static_cast<BYTE>(SC_PARTY), static_cast<BYTE>(eParty::SC_EQUIPCASHDATA)));
			VerifyPartyStruct.push_back(std::make_pair(static_cast<BYTE>(SC_PARTY), static_cast<BYTE>(eParty::SC_GLYPHDATA)));
#if defined(PRE_ADD_TALISMAN_SYSTEM)
			VerifyPartyStruct.push_back(std::make_pair(static_cast<BYTE>(SC_PARTY), static_cast<BYTE>(eParty::SC_TALISMANDATA)));
#endif	// #if defined(PRE_ADD_TALISMAN_SYSTEM)
			VerifyPartyStruct.push_back(std::make_pair(static_cast<BYTE>(SC_PARTY), static_cast<BYTE>(eParty::SC_VEHICLEEQUIPDATA)));
			VerifyPartyStruct.push_back(std::make_pair(static_cast<BYTE>(SC_PARTY), static_cast<BYTE>(eParty::SC_PETEQUIPDATA)));
			VerifyPartyStruct.push_back(std::make_pair(static_cast<BYTE>(SC_PARTY), static_cast<BYTE>(eParty::SC_SKILLDATA)));
			VerifyPartyStruct.push_back(std::make_pair(static_cast<BYTE>(SC_PARTY), static_cast<BYTE>(eParty::SC_ETCDATA)));
#endif		//#ifdef PRE_MOD_SYNCPACKET
		}

		void Assign( SPartyMemberInfo &e )
		{
			nSessionID	= e.nSessionID;
			nExp		= e.nExp;
			cLevel		= e.cLevel;
			cPvPLevel	= e.cPvPLevel;
			//wJob		= e.wJob;
			cClassID	= e.cClassID;
			nMapIndex	= e.nMapIndex;
			nFaceID		= e.nFaceID;
			nHairID		= e.nHairID;
			nHelmetID	= e.nHelmetID;
			nEarringID	= e.nEarringID;
			dwHairColor = e.dwHairColor;
			dwEyeColor = e.dwEyeColor;
			dwSkinColor = e.dwSkinColor;
			wFatigue	= e.wFatigue;
			wStdRebirthCoin = e.wStdRebirthCoin;
			wCashRebirthCoin = e.wCashRebirthCoin;
			cMemberIndex = e.cMemberIndex;
			uiPvPUserState = e.uiPvPUserState;
			//cVoiceAvailable = e.cVoiceAvailable;
			_wcscpy( wszCharacterName, _countof(wszCharacterName), e.wszCharacterName, (int)wcslen(e.wszCharacterName) );
#ifdef PRE_WORLDCOMBINE_PARTY
			nWorldSetID = e.nWorldSetID;
#endif
#ifdef PRE_ADD_NEWCOMEBACK
			bCheckComebackAppellation = e.bCheckComebackAppellation;
#endif // PRE_ADD_NEWCOMEBACK
			//if( std::find( nVecJobHistoryList.begin(), nVecJobHistoryList.end(), e.wJob ) == nVecJobHistoryList.end() )
			memcpy( cJobArray, e.cJobArray, sizeof(cJobArray) );
			for( UINT i=0 ; i<JOBMAX ; ++i )
			{
				if( cJobArray[i] > 0 )
					nVecJobHistoryList.push_back( cJobArray[i] );
			}
		}

#ifdef PRE_MOD_SYNCPACKET
		void OnRecv(int nMain, int nSub)
		{
			std::vector <std::pair<BYTE, BYTE>>::iterator ii;
			for (ii = VerifyPartyStruct.begin(); ii != VerifyPartyStruct.end(); ii++)
			{
				if ((*ii).first == static_cast<BYTE>(nMain) && (*ii).second == static_cast<BYTE>(nSub))
				{
					VerifyPartyStruct.erase(ii);
					return;
				}
			}
		}

		bool IsCompleteStruct()
		{
			return VerifyPartyStruct.size() == 0 ? true : false;
		}
#endif		//#ifdef PRE_MOD_SYNCPACKET

	};

protected:
	PartyRoleEnum m_PartyRole;
	PartyRoleEnum m_PrevPartyRole;
	ePartyState		m_PartyState;
	//emPartyRouletteLevel m_emPartyRouletteLevel;
	ePartyItemLootRule m_emPartyRouletteLevel;
	eItemRank m_emPartyRouletteItemRank;
	TPARTYID m_PartyID;
	tstring m_szPartyName;
#ifdef PRE_PARTY_DB
	int m_iPartyPassword;
	bool m_bAllowWorldZoneMapList;
#else
	tstring m_szPartyPassword;
#endif
	int m_nLocalActorPartyIndex;
	int m_nMaxPartyMemberCount;
	int m_nMinUserLevel;
#if defined( PRE_PARTY_DB )
#else
	int m_nMaxUserLevel;
#endif // #if defined( PRE_PARTY_DB )
	UINT m_nMasterSessionID;

	int m_nTargetStageIdx;
#ifdef PRE_PARTY_DB
	TDUNGEONDIFFICULTY m_TargetStageDifficulty;
#else
	int m_nTargetStageDifficulty;
#endif
	bool m_bJobDice;
	std::vector<int> m_nStageIdxListForSort;
	bool m_bAdvanceSearching;
	int m_nStageDifficultyForSort;

	bool m_bSingleToPartyByGMTrace;

	int m_nEnteredGateIndex;
	bool m_bRequestEnteredGate;
	int m_nRequestEnteredGateIndex;
	std::vector<PartyStruct> m_VecPartyList;
	std::list<PartyStruct> m_BreakIntoPartyList;
	CSyncLock m_BreakIntoPartyListLock;

	// When joining a party in another channel, store party index in this variable and change channel.
	int m_nPartyIndexToJoin;
#ifdef PRE_PARTY_DB
	std::vector<SPartyTargetMapInfo> m_PartyWholeTargetMapCache;
	std::map<int, int> m_PartyWholeTargetMapLevelLimitCache;
#else
	WCHAR m_wszPartyPasswordToJoin[PARTYPASSWORDMAX];
#endif

#ifdef PRE_WORLDCOMBINE_PARTY
	std::map<int, SWorldCombinePartyData> m_WorldCombineTableDataCache;
	int m_nCurWorldCombinePartyTableIndex;
#endif

	bool m_bSyncComplete;
	int m_nStartingMemberCount;

	PartyStruct m_LocalData;
	TPvPGroup m_PvPInfo;

	LadderSystem::SC_SCOREINFO m_sPVPLadderInfo;
	TPvPGhoulScores m_PvPGhoulInfo;

#ifdef PRE_MOD_INTEG_SYSTEM_STATE
	CDnAcceptRequestDataManager<CDnPartyAcceptRequestAskInfo> m_AcceptRequestAskInfo;
	CDnAcceptRequestDataManager<CDnPartyAcceptRequestInviteInfo> m_AcceptRequestInviteInfo;
	CDnPartyAcceptRequestAskInfo m_AskInfoCache;
	CDnPartyAcceptRequestInviteInfo m_inviteInfo;
#else
	struct SInviteInfo
	{
		TPARTYID m_PartyID;
		tstring m_szInviterName;
		tstring m_szPartyName;
		tstring m_szPassword;
		float	m_fAcceptTime;
		int		m_nMaxUserCount;
		int		m_nCurUserCount;
		int		m_nAvrLevel;
		int		m_nGameServerID;
		int		m_nGameMapIdx;

		SInviteInfo()
			: m_PartyID(0)
			, m_szInviterName(_T(""))
			, m_szPartyName(_T(""))
			, m_szPassword(_T(""))
			, m_fAcceptTime(10.0f)
			, m_nGameServerID(0)
			, m_nGameMapIdx(0)
			, m_nMaxUserCount(0)
			, m_nCurUserCount(0)
			, m_nAvrLevel(0)
		{
		}

		void Clear()
		{
			m_PartyID		= 0;
			m_szInviterName		= _T("");
			m_szPartyName		= _T("");
			m_szPassword		= _T("");
			m_fAcceptTime		= 10.f;
			m_nGameServerID		= 0;
			m_nGameMapIdx		= -1;
		}

		const SInviteInfo& MakeDenyInfo(TPARTYID PartyIdx, const std::wstring& inviterName, int nGameMapIdx, int nGameServerId)
		{
			Clear();
			m_nGameMapIdx = nGameMapIdx;
			m_nGameServerID = nGameServerId;
			m_PartyID = PartyIdx;
			m_szInviterName = inviterName;
			return *this;
		}
	};

	struct SAskInfo
	{
		std::wstring szApplicantName;
		int			 jobId;
		int			 level;
		float		 fAcceptTime;

		SAskInfo() : fAcceptTime(10.f), jobId(-1), level(0) {}
		void Clear() { fAcceptTime = 10.f; jobId = -1; level = -1; }
	};
	std::list<SAskInfo> m_listAskInfo;
	SAskInfo m_AskInfoCache;

	// Currently active (visible on screen) invite info
	SInviteInfo m_inviteInfo;

	typedef std::list<SInviteInfo>		LIST_INVITEINFO;
	typedef LIST_INVITEINFO::iterator	LIST_INVITEINFO_ITER;

	LIST_INVITEINFO m_listInviteInfo;
#endif // PRE_MOD_INTEG_SYSTEM_STATE

	int m_nKeepPartyBonusRate;
	int m_nBestFriendBonusRate;

protected:
	void OnRecvPartyMessage( int nSubCmd, char *pData, int nSize );
	void OnRecvRoomMessage( int nSubCmd, char *pData, int nSize );
	void OnRecvSystemMessage( int nSubCmd, char *pData, int nSize );
	void OnRecvCharMessage( int nSubCmd, char *pData, int nSize );

protected:
	void InitParty();
	int ChangePartyMaster( UINT nSessionID );

	tstring GetPermitGateErrorString( int nGateIndex );

	static bool __stdcall OnLoadRecvBreakIntoCallback( void *pThis, void *pParam, int nSize, LOCAL_TIME LocalTime );
	static int __stdcall OnLoadRecvBreakIntoUniqueID( void *pParam, int nSize );
	static bool __stdcall OnCheckLoadingPacket( void *pThis, UINT nUniqueID, int nMainCmd, int nSubCmd, void *pParam, int nSize );

	void	OnPartyMemberOut(UINT betrayerSessionID, char kickKind);

	void CalcDungeonOffsetNeedItem( CDnWorld::DungeonGateStruct *pStruct, int &nResultItemID, int &nResultItemCount );

public:
	bool Initialize();
	void ResetGateIndex();
	void OnInitializeVillageStage();
	void FinalizeGameTask();
	virtual void Process( LOCAL_TIME LocalTime, float fDelta );

	// Request cancellation can also be called externally.
#ifdef PRE_MOD_INTEG_SYSTEM_STATE
	void ClearInviteInfoList(bool bTerminate);
#else
	void ClearInviteInfoList();
#endif

	float GetShortestAcceptTime();
	void OpenAcceptRequestDialog();

	PartyRoleEnum GetPartyRole() { return m_PartyRole; }
	PartyRoleEnum GetPrevPartyRole() { return m_PrevPartyRole; }
	ePartyState		GetPartyState() const { return m_PartyState; }
	void			SetPartyState(ePartyState state);
	ePartyItemLootRule GetPartyLootRule() const { return m_emPartyRouletteLevel; }
	const wchar_t* GetPartyLootRuleString(ePartyItemLootRule rule) const;
	bool			IsJobDice() const { return m_bJobDice; }
	const wchar_t*	GetPartyLootItemRankString(eItemRank rank) const;

	std::vector<int>& GetPartyStageIdxListForSort() { return m_nStageIdxListForSort; }
	bool SetPartyStageIdxForSort(int idx);
	bool IsAdvancedSearching() const		{ return m_bAdvanceSearching; }
	void SetAdvancedSearching(bool bSet);
	int GetPartyStageDifficultyForSort() const { return m_nStageDifficultyForSort; }
	void SetPartyStageDifficultyForSort(int difficulty);
	void ReqPartyDungeonDifficultyInfo(int mapIdx);
	void ReqPartyAbsence(bool bAbsence);

	int GetPartySoundIndex(ePartySoundType type) const { return m_PartySoundIndex[type]; }
	void SetReversionItemSharing(bool bSharing) { m_bReversionItemSharing = bSharing; }
	eItemRank GetPartyLootItemRank() const { return m_emPartyRouletteItemRank; }
#ifdef PRE_PARTY_DB
	bool IsPartyAllowWorldZoneMapList() const { return m_bAllowWorldZoneMapList; }
#endif
	DWORD GetPartyCount();
	DWORD GetPartyCountExcepGM() const;

	DWORD GetObserverCount();
	int GetStartingMemberCount() { return m_nStartingMemberCount; } // Stores the party member count when first exiting to the world zone.
	int GetLocalActorIndex() { return m_nLocalActorPartyIndex; }
	PartyStruct *GetPartyData( DWORD dwIndex );
	PartyStruct *GetPartyDataFromSessionID( UINT nSessionID, bool bFindBreakIntoList = false );
	PartyStruct *GetPartyDataFromMemberName( const WCHAR* pName );
	const TCHAR *GetPartyName() const { return m_szPartyName.c_str(); }
#ifdef PRE_PARTY_DB
	int GetPartyPassword() const { return m_iPartyPassword; }
#else
	const TCHAR *GetPartyPassword() const { return m_szPartyPassword.c_str(); }
#endif
	int GetPartyTargetStageIdx() const { return m_nTargetStageIdx; }
#ifdef PRE_PARTY_DB
	TDUNGEONDIFFICULTY GetPartyTargetStageDifficulty() const { return m_TargetStageDifficulty; }
#else
	int GetPartyTargetStageDifficulty() const { return m_nTargetStageDifficulty; }
#endif

	void SetPartyTargetStageIdx(int idx);
	void SetPartyTargetStageDifficulty(int difficulty);

#ifdef PRE_PARTY_DB
	const std::vector<SPartyTargetMapInfo>& MakeWholePartyTargetStageList();
	const std::map<int, int>& MakeWholePartyTargetStageLevelLimitList();

	void MakePartyTargetStageList(int currentMapIndex, std::vector<SPartyTargetMapInfo>& stageNameList, bool bClearList = true);
	void MakePartyTargetStageLevelLimitList(std::map<int, int>& stageLevelLimitList, std::vector<SPartyTargetMapInfo>& stageNameList, bool bClearList = true);
	void SetPartyPassword(int iPassword) { m_iPartyPassword = iPassword; }
	bool IsSecretParty() const;
#else
	void SetPartyPassword( const TCHAR *szPassword ) { m_szPartyPassword = szPassword; }
	bool IsSecretParty() { return !m_szPartyPassword.empty(); }
	void MakePartyTargetStageList(int currentMapIndex, std::vector<SPartyTargetMapInfo>& stageNameList, std::map<int, int>& stageLevelLimitList);
#endif

	void SetDefaultPartsInfo( SCDefaultPartsData* pPacket );
	void SetWeaponOrderInfo( SCWeaponOrderData* pPacket );
#ifdef PRE_MOD_SYNCPACKET
	void SetEquipInfo( SCEquipData* pPacket );
	void SetCashEquipInfo( SCCashEquipData* pPacket );
	void SetGlyphEquipInfo( SCGlyphEquipData* pPacket );
#if defined(PRE_ADD_TALISMAN_SYSTEM)
	void SetTalismanEquipInfo( SCTalismanEquipData* pPacket );
#endif
	void SetVehicleEquipInfo( SCVehicleEquipData* pPacket );
	void SetPetEquipInfo( SCVehicleEquipData* pPacket );
	void SetSkillInfo( SCSkillData* pPacket );
#else		//#ifdef PRE_MOD_SYNCPACKET
	void SetEquipInfo( TPartyMemberEquip *pEquipInfo, SCEquipData* pPacket );
	void SetCashEquipInfo( TPartyMemberCashEquip *pEquipInfo, SCCashEquipData* pPacket );
	void SetGlyphEquipInfo( TPartyMemberGlyphEquip *pEquipInfo, SCGlyphEquipData* pPacket );
#if defined(PRE_ADD_TALISMAN_SYSTEM)
	void SetTalismanEquipInfo( TPartyMemberTalismanEquip *pEquipInfo, SCTalismanEquipData* pPacket );
#endif
	void SetVehicleEquipInfo( TVehicleCompact *pEquipInfo, SCVehicleEquipData* pPacket );
	void SetPetEquipInfo( TVehicleCompact *pEquipInfo, SCVehicleEquipData* pPacket );
	void SetSkillInfo( TPartyMemberSkill *pSkillInfo, SCSkillData* pPacket );
#endif //#ifdef PRE_MOD_SYNCPACKET

	void SetEtcInfo( SCEtcData* pPacket );

	TPARTYID GetPartyIndex() { return m_PartyID; }
	void SetLocalData( DnActorHandle hActor );
	void LockPartyReqPartyMemberInfo(bool bLock);
#ifdef PRE_PARTY_DB
	void LockPartyReqPartyList(bool bLock);	
#endif

	void SetPartyIndexToJoin( int nPartyIndex ) { m_nPartyIndexToJoin = nPartyIndex; }
	void JoinReservedParty();

	PartyStruct *GetLocalData() { return &m_LocalData; }
	void ResetSync();
	bool IsSyncComplete() { return m_bSyncComplete; }
	void SetSyncComplete( bool bValue ) { m_bSyncComplete = bValue; }
	void SetWaitingPartyFindDlgOpen(bool bSet) { m_bWaitingPartyFindDlgOpen = bSet; }

	void RequestSyncWaitMsg();
	void UpdatePartyMemberInfo();

	bool IsPartyMemberAlive();
	bool IsPartyMember( UINT sessionID );
	bool IsPartyMember( const wchar_t* name);
	bool					IsPartyMemberEnableStartGame(UINT sessionID) const;
	eErrorPartyStartGame	IsPartyEnableStartGame( char cGateIndex ) const;
	bool					IsPartyFull() const;
	bool					IsWaitingPartyFindDlgOpen() const { return m_bWaitingPartyFindDlgOpen; }

#ifdef PRE_PARTY_DB
	void ReqCreateParty(SPartyCreateParam& param);
#else
	void ReqCreateParty(BYTE cPartyType, BYTE cPartyMemberMax, int nUserLvLimitMin, int nUserLvLimitMax, const WCHAR * pPartyName, const WCHAR * pPass, int nMapIdx, int nDifficulty, BYTE cIsPartyDice);
#endif
#ifdef PRE_MOD_INTEG_SYSTEM_STATE
	void ReqDenyPartyInvite(const CDnPartyAcceptRequestInviteInfo& info, bool bIsOpenBlind = false);
	void ReqJoinPartyByInvite(const CDnPartyAcceptRequestInviteInfo& info);
#else
	void ReqDenyPartyInvite(const SInviteInfo& info, bool bIsOpenBlind = false);
	void ReqJoinPartyByInvite(const SInviteInfo& info);
#endif
	void OnPartyMemberNewAdded(ePartyRefreshSubject subject, const PartyStruct& memberInfo);
	bool IsPartyMemberInviting() const { return m_bPartyMemberInviting; }
	bool IsValidGameMapIdx(int mapIdx) const;
#ifdef PRE_PARTY_DB
	void ReqJoinParty( TPARTYID PartyIndex, int iPassword);
#else
	void ReqJoinParty( TPARTYID PartyIndex, const TCHAR *szPartyPassword );
#endif
#ifdef PRE_FIX_GAMEQUIT_TO_VILLAGE
	void ReqOutParty(bool bIntendedDisconnect = false);
#else
	void ReqOutParty();
#endif
	void ReqInviteParty( const WCHAR *szPlayerName );
	void ReqAskParty(const WCHAR* szPlayerName);

#ifdef PRE_MOD_INTEG_SYSTEM_STATE
	void AnswerPartyAsking(bool bAccept, const std::wstring& applicantName);
#else
	void AnswerPartyAsking(bool bAccept, const SAskInfo& info);
#endif
#ifdef PRE_PARTY_DB
	void ReqPartyMemberInfo( TPARTYID nPartyIdx );
#else
	void ReqPartyMemberInfo( UINT nPartyIdx );
#endif
	void ReqPartyMemberKick( int nMemberIndex, int nKickType );			// nKickType : 0 normal, 1 permanent
	void ReqPartyMemberKick( const WCHAR *szPlayerName, int nKickType );
	void ReqPartyLeaderSwap( int nNewLeaderIndex );
	void ReqPartyLeaderSwapWithSessionID( UINT nSessionID );
#ifdef PRE_PARTY_DB
	void ReqPartyInfoModify(BYTE cPartyMemberMax, TPARTYITEMLOOTRULE cLootRule, TITEMRANK cLootRank, int nUserLvLimitMin, const WCHAR * pPartyName, int iPassword, int mapIdx, TDUNGEONDIFFICULTY difficulty, bool bAllowWorldZoneMapList);
#else
	void ReqPartyInfoModify(BYTE cPartyMemberMax, BYTE cLootRule, BYTE cLootRank, int nUserLvLimitMin, int nUserLvLimitMax, const WCHAR * pPartyName, const WCHAR * pPass, int mapIdx, int difficulty);
#endif
	void ReqPartyRequestReady();
	void ReqPartySwapMemberIndex(const std::vector<SRaidReGroupInfo>& members);
	void ReqPartyItemLootRuleModify(ePartyItemLootRule lootRule);
	void ReqPartyItemLootRankModify(eItemRank lootRank);
#ifdef PRE_PARTY_DB
	void ReqPartyAllowWorldZoneMapList(bool bAllow);
#endif
	void ReqPartyJoinGetReversionItem(bool bJoin);
	void ReqPartyJobDiceModified(bool bJobDice);
#ifdef PRE_PARTY_DB
	void ReqPartyListInfo(int currentPage, const std::wstring& searchWord);
	void ResetPartyList();
#else
	void ReqPartyListInfo(int currentPage, bool bSort);	
#endif

	bool IsPartyLevelToEnterGate( int nLevel, CDnWorld::DungeonGateStruct *pStruct, int *nRequireMinLevel = NULL, int *nRequireMaxLevel = NULL );
	bool IsPartyCountToEnterGate( int nCount, CDnWorld::DungeonGateStruct *pStruct, int *nRequireMinCount = NULL, int *nRequireMaxCount = NULL );
#ifdef PRE_PARTY_DB
	bool IsPartyInfoModified(int newMaxPartyCount, int newMinLevel, const std::wstring& newPartyName, int iNewPassword, int newTargetStageIdx, TDUNGEONDIFFICULTY newTargetStageDifficulty) const;
	bool IsPartyAllowWorldZoneMapListModified(bool bAllow) const;
#else
	bool IsPartyInfoModified(int newMaxPartyCount, int newMinLevel, int newMaxLevel, const std::wstring& newPartyName, const std::wstring& newPassword, int newTargetStageIdx, int newTargetStageDifficulty) const;
#endif
	bool IsPartyLootRuleModified(ePartyItemLootRule newRule) const;
	bool IsPartyLootRankModified(eItemRank newRank) const;
	bool IsPartyTargetStageIdxModified(int newStageIdx) const;
	bool IsPartyTargetStageDifficultyModified(int newDifficulty) const;
	bool IsPartyInfoEmpty() const;
	bool IsWaitingGate() const					{ return m_bWaitGate; }
	bool IsLocalActorEnterGateReady() const		{ return (m_nEnteredGateIndex != INVALID_GATE_INDEX); }
	bool IsLockedReqPartyMemberInfo() const		{ return m_bLockReqPartyMemberInfo; }
#ifdef PRE_PARTY_DB
	bool IsLockedReqPartyList() const { return m_bLockReqPartyList; }
#endif
	bool IsSingleToPartyByGMTrace() const		{ return m_bSingleToPartyByGMTrace; }
	int GetEnteredGateIndex() { return m_nEnteredGateIndex; }
	bool IsRequestEnteredGate() { return m_bRequestEnteredGate; }
	bool IsGMTracing();

#ifdef PRE_PARTY_DB
	int	GetMaxPasswordLength() const;
#else
	int	GetMaxPasswordLength() const { return PARTYPASSWORDMAX - 1; }
#endif

	void GetPartyTargetStageString(std::wstring& result);
	void GetPartyTargetStageDifficultyString(std::wstring& result);
	bool HasPartyMemberJobDicePriority(int diceValue) const;

	void RequestInitParty(){InitParty();}// Initialize party from outside 
#ifdef PRE_ADD_BEGINNERGUILD
	void RefreshKeepPartyInfo( int nKeepPartyBonusRate, int nBestFriendBonusRate, bool bLimitKeepPartyBonus, int nBeginnerGuildBonusRate );
#else
	void RefreshKeepPartyInfo( int nKeepPartyBonusRate, int nBestFriendBonusRate, bool bLimitKeepPartyBonus );
#endif


	void SetPvPInfo( TPvPGroup *pInfo ) { m_PvPInfo = *pInfo; }
	TPvPGroup *GetPvPInfo() { return &m_PvPInfo; }

	void SetPVPLadderInfo(LadderSystem::SC_SCOREINFO *pData);
	LadderSystem::SC_SCOREINFO* GetPVPLadderInfo(){return &m_sPVPLadderInfo;}

	void SetPvPGhoulInfo(TPvPGhoulScores *pInfo){ m_PvPGhoulInfo = *pInfo;}
	TPvPGhoulScores *GetPvPGhoulInfo(){ return &m_PvPGhoulInfo;}

#if defined( PRE_ADD_NAMEDITEM_SYSTEM )
	void ProcessEffectSkill( float fElapsedTime );
	void AddEffectSkillInfo( UINT nSessionID , VillagePartyEffectSkillInfo *pInfo );
	void RemoveEffectSkillInfo( UINT nSessionID , int nEffectSkillIndex );
#endif

public:
	int GetMaxPartyCount() const		{ return m_nMaxPartyMemberCount; }
	int GetMinUserLevel() const		{ return m_nMinUserLevel; }
#if defined( PRE_PARTY_DB )
#else
	int GetMaxUserLevel() const		{ return m_nMaxUserLevel; }
#endif // #if defined( PRE_PARTY_DB )
	bool IsUserLevelLimit() const;
	bool IsLocalActorMaster() const;
	UINT GetMasterSessionID() const	{ return m_nMasterSessionID; }
	int GetRequestEnteredGateIndex() const { return m_nRequestEnteredGateIndex; }
	bool IsPvpGame();

#ifdef PRE_WORLDCOMBINE_PARTY
	void GetCurrentWorldCombinePartyData(SWorldCombinePartyData& data) const;
#endif

public:
	// ClientSession
	virtual void OnDisconnectTcp( bool bValidDisconnect );
	virtual void OnDisconnectUdp( bool bValidDisconnect );

	virtual void OnDispatchMessage( int nMainCmd, int nSubCmd, char *pData, int nSize );

	virtual void OnRecvPartyCreateParty( SCCreateParty *pPacket );
	virtual void OnRecvPartyRefresh( SCRefreshParty *pPacket );
	virtual void OnRecvPartyJoinParty( SCJoinParty *pPacket );
	virtual void OnRecvPartyOutParty( SCPartyOut *pPacket );
	virtual void OnRecvPartyRefreshGateInfo( SCRefreshGateInfo *pPacket );
	virtual void OnRecvPartyChangeLeader( char *pPacket );
	virtual void OnRecvPartyInviteMsg( char * pPacket );
	virtual void OnRecvPartyInviteFail(char * pPacket);
	virtual void OnRecvPartyInviteDenied(SCPartyInviteDenied * pPacket);
	virtual void OnRecvPartyMemberKick( SCPartyKickedMember *pPacket );
	virtual void OnRecvPartyLeaderSwap( SCSwapPartyLeader *pPacket );
	virtual void OnRecvPartyModifyResult(SCPartyInfoModify * pPacket);
	virtual void OnRecvVerifyJoinParty(SCVerifyJoinParty * pPacket);

	//virtual void OnRecvPartyDelMember( SCDelPartyMember *pPacket );
	virtual void OnRecvPartyReadyGate( SCGateInfo *pPacket );
	virtual void OnRecvPartyStageStart( SCStartStage *pPacket );
	virtual void OnRecvPartyStageCancel( SCCancelStage *pPacket );
	virtual void OnRecvPartyVillageStart( char *pPacket );
	virtual void OnRecvPartyStageStartDirect( SCStartStageDirect *pPacket );

	virtual void OnRecvPartySkillData( SCSkillData *pPacket );
	virtual void OnRecvPartyEquipData(SCEquipData * pPacket);
	virtual void OnRecvPartyCashEquipData( SCCashEquipData *pPacket );
	virtual void OnRecvPartyDefaultPartsData( SCDefaultPartsData *pPacket );
	virtual void OnRecvPartyEtcData( SCEtcData *pPacket );
	virtual void OnRecvPartyWeaponOrderData( SCWeaponOrderData *pPacket );
	virtual void OnRecvPartyGlyphEquipData( SCGlyphEquipData *pPacket );
#if defined(PRE_ADD_TALISMAN_SYSTEM)
	virtual void OnRecvPartyTalismanEquipData( SCTalismanEquipData *pPacket );
#endif

	virtual void OnRecvPartyVehicleEquipData( SCVehicleEquipData *pPacket );
	virtual void OnRecvPartyPetEquipData( SCVehicleEquipData* pPacket );

	virtual void OnRecvPartyGuildData( TPartyMemberGuild *pPacket );
	virtual void OnRecvPartyJoinGetReversionItem(SCPartyJoinGetReversionItemInfo* pData);
	virtual void OnRecvPartyRollGetReversionItem(SCRollDiceForGetReverseItem* pPacket);
	virtual void OnRecvPartyResultGetReversionItem(SCResultGetReverseItem* pPacket);

	virtual void OnRecvPartyReadyRequest();
	virtual void OnRecvPartyMemberAbsent(SCMemberAbsent * pPacket);
	virtual void OnRecvVerifyDungeonRequest(SCVerifyDungeon * pPacket);
	virtual void OnRecvPartyMemberInfo(ePartyType type);
	virtual void OnRecvPartyRefreshRebirthCoin(SCRefreshRebirthCoin * pPacket);
	virtual void OnRecvPartyKeepBonus( SCPartyBonusValue *pPacket );
	virtual void OnRecvPartyInviteNotice(SCPartyInviteNotice* pPacket);
	virtual void OnRecvPartyAskJoinResult(SCPartyAskJoinResult * pPacket);
	virtual void OnRecvPartyAskJoinToLeader(SCPartyAskJoinToLeader * pPacket);
	virtual void OnRecvPartyAskJoinDecisionInfo(SCPartyAskJoinDecisionInfo * pPacket);
	virtual void OnRecvPartyUpdateFieldUI(SCPartyUpdateFieldUI * pPacket);
	virtual void OnRecvPartyMemberPart(SCPartyMemberPart *pData);

#ifdef PRE_MOD_REQ_JOIN_PARTY_ANSWER_MSG_APP
	virtual void OnRecvPartyAskInfo(SCPartyInfo *pPacket);
#endif 

	virtual void OnRecvSystemVillageInfo( SCVillageInfo *pPacket );
	virtual void OnRecvSystemCountDown(SCCountDownMsg * pPacket);

	virtual void OnRecvRoomSyncWait( SCSyncWait *pPacket );
	virtual void OnRecvRoomSyncStart( SCSyncStart *pPacket ); 
	virtual void OnRecvRefreshMember(SCRefreshMember * pPacket);
	virtual void OnRecvRoomMemberInfo( SCROOM_SYNC_MEMBERINFO* pPacket );
	virtual void OnRecvRoomMemberTeam( SCROOM_SYNC_MEMBERTEAM* pPacket );
	virtual void OnRecvRoomMemberBreakInto( SCROOM_SYNC_MEMBER_BREAKINTO* pPacket, int nSize );
	virtual void OnRecvRoomMemberHPSP( SCROOM_SYNC_MEMBERHPSP* pPacket );
	virtual void OnRecvRoomMemberBattleMode( SCROOM_SYNC_MEMBERBATTLEMODE* pPacket );
	virtual void OnRecvRoomDropItemList( SCROOM_SYNC_DROPITEMLIST* pPacket );

	virtual void OnRecvCharEntered( SCEnter *pPacket );
	virtual void OnRecvPartySwapMemberIndex(SCPartySwapMemberIndex* pPacket);
	virtual void OnRecvPartyLastDugeonInfo(SCPartyCheckLastDungeonInfo* pPacket);
	virtual void OnRecvPartyConfirmLastDungeonInfo(SCPartyConfirmLastDungeonInfo* pPacket);
	virtual void OnRecvSelectStage( SCSelectStage * pPacket );

	// CEtUICallback
	virtual void OnUICallbackProc( int nID, UINT nCommand, CEtUIControl *pControl, UINT uiMsg = 0 );

	void AddPartyList( PartyStruct &Struct );
	void AddBreakIntoPartyList( PartyStruct& Struct );
	void UpdateBreakIntoPartyList();
	void DeleteBreakIntoPartyList( UINT iSessionID );

	void LockBreakIntoPartyList( bool bLock );
	DWORD GetBreakIntoPartyCountWithoutSync() 
	{ 
		return (DWORD)m_BreakIntoPartyList.size(); 
	}
	PartyStruct *GetBreakIntoPartyDataWithoutSync( DWORD dwIndex ) 
	{ 
		if( dwIndex < 0 || dwIndex >= m_BreakIntoPartyList.size() ) 
			return NULL; 
		
		std::list<PartyStruct>::iterator itor = m_BreakIntoPartyList.begin();
		std::advance( itor, dwIndex );
		
		return &(*itor);
	}
	bool IsPartyAcceptRequesting() const { return m_bPartyAcceptRequesting; }
	void SetPartyListResetSort(bool bSet)	{ m_bResetSort = bSet; }
	bool GetPartyListResetSort() const		{ return m_bResetSort; }

	bool DelegatePartyLeader(bool bExcludeMe);

	void		SetPartyType(ePartyType type) { m_PartyType = type; }
	ePartyType	GetPartyType() const	{ return m_PartyType; }
	bool		IsPartyType(ePartyType type) const;

	void		OnCancelStage();
	bool		IsPartyProcessState(ePartyProcessState state) const { return (m_PartyProcessState & state) ? true : false; }

#ifdef PRE_WORLDCOMBINE_PARTY
	void AddWorldCombinePartyTableDataCache(int index, SWorldCombinePartyData& data);
	void GetWorldCombinePartyTableData(int index, SWorldCombinePartyData& data) const;
	bool IsPartyTypeNeedRaidPartyTypeUI() const;
#endif

private:
	void EnableWaitGateState(bool bEnable);
	void SetLocalActorEnterGateIdx(int idx, bool bForceMsgOff);

	bool m_bWaitGate;
	bool m_bPartyAcceptRequesting;
	bool m_bWaitingPartyFindDlgOpen;

	bool m_bReversionItemSharing;
	bool m_bNoWarpBeforeItemSharing;

	bool m_bLockReqPartyMemberInfo;
#ifdef PRE_PARTY_DB
	bool m_bLockReqPartyList;
#endif
	int m_PartySoundIndex[SOUNDMAX];
	bool m_bResetSort;

	bool m_bPartyMemberInviting;

	ePartyType m_PartyType;
	std::vector<SPartyMemberData>	m_PartyMemberDataList;
	int m_PartyProcessState;
	int m_PartyReconnectCheckCounter;

protected:
	typedef std::map<int, SCGetPlayerCustomEventUI> NESTINFO_LIST;
	NESTINFO_LIST m_NestInfoList;

	void AddNestInfo(int nSessionID, SCGetPlayerCustomEventUI* pInfo);
	void RemoveNestInfo(int nSessionID);
	SCGetPlayerCustomEventUI* GetNestInfo(int nSessionID);
public:
	void InitNestInfoList() { m_NestInfoList.clear(); }

	void RequestNestInfo(int nSessionID);
	void OnRecvPlayerCustomEventUI( int nSubCmd, char *pData, int nSize );
	void OnRecvPlayerCustomEventUI(SCGetPlayerCustomEventUI* pPacket);

#if defined (PRE_PARTY_DB) && defined (_WORK)
	float m_fPartyListRefreshTime;
	void SetPartyListRefreshTime(float fTime);
	float GetPartyListRefreshTime() const  { return m_fPartyListRefreshTime; }
#endif

#ifdef PRE_FIX_PARTYINDEX_DUMP
	void SetLocalActorPartyIndex();
#endif

#ifdef PRE_MOD_REQ_JOIN_PARTY_ANSWER_MSG_APP
	void ReqPartyInfoWithPlayerID( const TPARTYID PlayerID );
	void ReqPartyInfoWithPlayerName( const WCHAR* pPlayerName );
#endif


};

#define GetPartyTask()	CDnPartyTask::GetInstance()