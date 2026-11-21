#pragma once

//PvP related - to be implemented
#include "DNUser.h"

class CDNPvP
{
public:
	
	CDNPvP();
	~CDNPvP();

	UINT		GetIndex() const { return m_uiIndex; }
	BYTE		GetEventRoomIndex() const { return m_cEventRoomIndex; }
	TEvent*		GetEventDataPtr(){ return &m_EventData; }
	BYTE		GetVillageID() const { return m_cVillageID; }
	USHORT		GetVillageChannelID() const { return m_unVillageChannelID; }
	UINT		GetVillageMapIndex() const { return m_uiVillageMapIndex; }
	int			GetRandomSeed() const { return m_iRandomSeed; }
	UINT		GetCaptainAccountDBID() const { return m_uiCaptainAccountDBID; }
#if defined(PRE_ADD_QUICK_PVP)
	void		SetCaptainAccountDBID(UINT uiCaptainAccountDBID) { m_uiCaptainAccountDBID =uiCaptainAccountDBID; }
	void		AddListAccountDBID(UINT uiAddAccountDBID);
#endif
	UINT		GetMapIndex() const { return m_uiMapIndex; }
	UINT		GetRoomState() const { return m_uiRoomState; }
	int			GetGameServerID() const { return m_iGameServerID; }
	int			GetGameServerRoomIndex() const { return m_iGameRoomID; }
	UINT		GetGameModeTableID() const { return m_uiGameModeTableID; }
	UINT		GetGameModeWinCondition() const { return m_uiWinCondition; }
	UINT		GetGameModePlayTimeSec() const { return m_uiPlayTimeSec; }
	USHORT		GetGameModeCheck() const { return m_unGameModeCheck; }
	INT64		GetSNMain() const { return m_biSNMain; }
	int			GetSNSub() const { return m_iSNSub; }
	USHORT		GetRoomOptionBit() const { return m_unRoomOptionBit; }
	bool		bIsBreakIntoRoom() const { return (m_unRoomOptionBit&PvPCommon::RoomOption::BreakInto) ? true : false; }
	bool		bIsDropItemRoom() const { return (m_unRoomOptionBit&PvPCommon::RoomOption::DropItem) ? true : false; }
#ifdef PRE_MOD_PVPOBSERVER
	bool bIsAllowObserver() const { return (m_unRoomOptionBit&PvPCommon::RoomOption::AllowObserver) ? true : false; }
#endif		//#ifdef PRE_MOD_PVPOBSERVER
	UINT		GetMaxUser() const { return m_uiMaxUser; }
	void		GetVoiceChannelID(UINT *pVoiceChannel);
	UINT		GetGameMode(){ return m_uiGameMode; }
	bool GetIsGuildWarSystem() const { return m_bIsGuildWarSystem; }
	UINT GetGuildDBIDbyIdx(int nIdx) const;
	bool		IsMode( UINT uiMode ){ return (m_uiGameMode==uiMode); }
	
	void		Update( const UINT uiCurTick );

	void		AddBanUser( UINT uiSessionID );
	void		ClearBanUser();
	bool		bIsEmpty( bool bExceptObserver=false );
	bool		bIsCaptain( const UINT uiAccountDBID ){ return (m_uiCaptainAccountDBID==uiAccountDBID); }
	bool bIsGuildWarGrade(UINT nAccountDBID, int nGrade);
	bool		bIsCheckUser( const UINT uiAccountDBID );
	bool		bIsBanUser( const UINT uiSessionID );
	bool		bIsReady( CDNUser* pUser );
	bool		bIsAllowLevel( const BYTE cUserLevel );
	bool		bCreate( const BYTE cVillageID, const UINT uiPvPIndex, const VIMAPVP_CREATEROOM* pPacket, const INT64 biCreateRoomCharacterDBID, bool bExtendObserver );
	void		ModifyRoom( CDNVillageConnection* pVillageCon, const VIMAPVP_MODIFYROOM* pPacket );
	bool		bLeave( CDNVillageConnection* pVillageCon, CDNUser* pLeaveUser, const bool bDisconnectVillageServer=false, PvPCommon::LeaveType::eLeaveType Type=PvPCommon::LeaveType::Normal );
	bool		bLeaveAll( CDNVillageConnection* pVillageCon, PvPCommon::LeaveType::eLeaveType Type=PvPCommon::LeaveType::Normal );
#ifdef PRE_MOD_PVPOBSERVER
	bool LeaveObserver(CDNVillageConnection* pVillageCon, PvPCommon::LeaveType::eLeaveType Type=PvPCommon::LeaveType::Normal);
#endif		//#ifdef PRE_MOD_PVPOBSERVER
	bool		bIsPWRoom() const { return !m_wstrRoomPW.empty(); }
	short		Join( CDNVillageConnection* pVillageCon, CDNUser* pJoinUser, const UINT uiUserState, bool bIsObserver=false, UINT nGuildDBID = 0 );
	
	bool		bIsEmptySlot( bool bIsObserver=false );
	bool		bIsEmptyTeamSlot( const USHORT usTeam );
	bool		bIsCheckPW( const WCHAR* pwszPW );
	CDNUser*	GetUserBySessionID( const UINT uiSessionID );
	CDNUser*	GetUserByAccountDBID( const UINT uiAccountDBID );
	void		ChangeCaptain( CDNVillageConnection* pVillageCon, const UINT uiCaptainAccountDBID );
	bool		bIsGroupCaptain( UINT uiAccountDBID );
	CDNUser*	GetGroupCaptain( USHORT nTeam );
	void		ChangeGroupCaptain( UINT uiGroupCaptainAccountDBID, USHORT nTeam );
	void		CheckAndSetGroupCaptain();
	void		BanPvPRoom( CDNVillageConnection* pVillageCon, const VIMAPVP_BAN* pPacket );
	void		ReadyUser( CDNVillageConnection* pVillageCon, const VIMAPVP_READY* pPacket );
	short		StartPvP( CDNVillageConnection* pVillageCon, const VIMAPVP_START* pPacket );

	int SetPvPGuildWarMemberGrade(USHORT nTeam, bool bAsign, USHORT nType, UINT nSessionID);
	bool SetPvPMemberIndex(int nTeam, UINT nAccountDBID);
	bool SwapPvPMemberIndex(USHORT nTeam, BYTE cCount, const TSwapMemberIndex * pIndex);

	int GetUserCountByTeam(USHORT nTeam);
	int GetUserCountByState(USHORT nTeam, USHORT nState);
	bool GetAndSetRemainTeamIndex(int nTeam, UINT nAccountDBID, char &cIndex);
	char GetPvPTeamIndex(CDNUser * pUser);

	UINT SelectNewGuildWarCaptain(USHORT nTeam, UINT nExceptSessionID = 0);

	void		SetRoomState( const UINT uiRoomState );
	void		AddPvPUserState( CDNUser* pUser, const UINT uiAddUserSate );
	void		SetPvPUserState( CDNUser* pUser, const UINT uiUserState );
	//
	void		SetAllGameID( USHORT wGameID, int nRandomSeed, int nMapIdx, BYTE cGateNum );
	void		SetAllRoomID( CDNVillageConnection* pVillageCon, CDNGameConnection* pGameCon, int nRoomID, int nServerIdx );
	void		GetMembersByAccountDBID( std::vector<UINT>& rvAccountDBID );
	// 
	bool		bDisconnectServer( const int iConnectionKey );

	UINT		FindNewCaptain();
	UINT		FindNewGroupCaptain( UINT uiTeam );
	//VoiceChannel
	void		SetVoiceChannelID(UINT *pChannelID);
	bool GetGuildWarInfo(UINT * pGuildDBID, int * pScore);
	INT64		GetCreateRoomCharacterDBID(){ return m_biCreateRoomCharacterDBID;};
#ifdef PRE_ADD_COLOSSEUM_BEGINNER
	PvPCommon::RoomType::eRoomType GetRoomChannelType() const { return m_eRoomType; }
#endif		//#ifdef PRE_ADD_COLOSSEUM_BEGINNER
#ifdef PRE_MOD_PVPRANK
	bool bIsExtendObserver() const { return m_bExtendObserver; }
#endif		//#ifdef PRE_MOD_PVPRANK

#if defined(PRE_ADD_PVP_TOURNAMENT)
	UINT GetPvPTournamentIndex(int nIndex) { return m_vTournamentAccoutDBID[nIndex];}
	bool SwapPvPTournamentIndex(char cSourceIndex, char cDestIndex);
	char FindTournamentIndex(UINT uiAccountDBID);
	char GetTournamentIndex(UINT uiAccountDBID);
	bool DelTournamentIndex(UINT uiAccountDBID);
	UINT GetPvPTournamentAccountDBID(char cIndex);
	void SetTournamentShuffleIndex();
	UINT* GetPvPTournamentIndexAll() { return &m_vTournamentAccoutDBID[0]; } 
#endif

#if defined( PRE_PVP_GAMBLEROOM )
	BYTE GetGambleRoomType() { return m_cGambleType; }
	int GetGambleRoomPrice() { return m_nGamblePrice;}
#endif

private:

	short		_StartBreakInto( const VIMAPVP_START* pPacket );
	void		_RequestGameRoom();
	USHORT		_SelectRandomTeam();
	void		_GetReadyUserCount( UINT& ruiATeam, UINT& ruiBTeam );
	void		_GetTeamUserCount( UINT& ruiATeam, UINT& ruiBTeam );
	bool _InitGuildWar(BYTE cCount, const VIMAPVP_CREATEROOM * pPacket);
	USHORT _SelectTeamGuild(UINT nGuildDBID);		
	void _GetJoinUserGrade(USHORT nTeam, UINT &uiUserState);
	int _GetUserCountByTeamIndex(PvPCommon::Team::TPvPTeamIndex &Team);
	bool _CheckReadyGuildWarMaster();

	BYTE				m_cEventRoomIndex;
	TEvent				m_EventData;
	bool				m_bIsCanStartEventRoom;
	UINT				m_uiMinUser;
	// Village info for creating PvP room
	BYTE				m_cVillageID;
	USHORT				m_unVillageChannelID;
	UINT				m_uiVillageMapIndex;
	// GameServer info for creating PvP room
	int					m_iGameServerID;
	int					m_iGameRoomID;
	// PvP room info
	UINT				m_uiIndex;
	UINT				m_uiCaptainAccountDBID;
	UINT				m_uiRoomState;
	UINT				m_uiMapIndex;
	UINT				m_uiMaxUser;
	std::wstring		m_wstrRoomName;
	std::wstring		m_wstrRoomPW;
	int					m_iRandomSeed;
	USHORT				m_unGameModeCheck;
	UINT				m_uiGameMode;
	UINT				m_uiWinCondition;
	UINT				m_uiPlayTimeSec;
	UINT				m_uiGameModeTableID;
	BYTE				m_cMinLevel;
	BYTE				m_cMaxLevel;
	USHORT				m_unRoomOptionBit;

	std::vector <PvPCommon::Team::TPvPTeamIndex> m_vTeamSlotIndex;	
	std::vector <PvPCommon::Team::TPvPGuildWarInfo> m_vGuildDBID;
#if defined(PRE_ADD_PVP_TOURNAMENT)	
	UINT m_vTournamentAccoutDBID[PvPCommon::Common::PvPTournamentUserMax];
#endif 
	std::list<UINT>		m_listAccountDBID;

	// S/N
	INT64				m_biSNMain;
	INT32				m_iSNSub;

	// Etc
	UINT				m_uiStartMsgCount;		// Game start message
	UINT				m_uiStartMsgTick;

	// Expulsion-related
	std::map<UINT,DWORD>	m_mBanUser;

	//VoiceChannel
	UINT m_nVoiceChannelID[PvPCommon::TeamIndex::Max];
	bool m_bIsGuildWarSystem;		//Whether created by the guild war system
	INT64		m_biCreateRoomCharacterDBID;
#ifdef PRE_ADD_COLOSSEUM_BEGINNER
	PvPCommon::RoomType::eRoomType m_eRoomType;
#endif		//#ifdef PRE_ADD_COLOSSEUM_BEGINNER
	DWORD m_dwPlayingStateCheckTick;
#ifdef PRE_MOD_PVPOBSERVER
	bool m_bExtendObserver;
#endif		//#ifdef PRE_MOD_PVPOBSERVER
#if defined( PRE_PVP_GAMBLEROOM )	
	BYTE m_cGambleType;
	int m_nGamblePrice;
#endif
};
