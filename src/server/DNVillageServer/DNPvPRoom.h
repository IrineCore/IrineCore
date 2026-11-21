
#pragma once

class CDNUserSendManager;
class CDNUserSession;

class CDNPvPRoom
{
private:

	// User info
	typedef struct tagPvPUserInfo
	{
		UINT	uiAccountDBID;
		UINT	uiSessionID;
		int		iJobID;
		BYTE	cLevel;
		BYTE	cPvPLevel;
		USHORT	usTeam;
		char cSlotIndex;
		UINT	uiUserState;
		WCHAR	wszCharName[NAMELENMAX];

		tagPvPUserInfo():uiAccountDBID(0),uiSessionID(0),iJobID(0),cLevel(0),cPvPLevel(0),usTeam(PvPCommon::Team::Max),uiUserState(PvPCommon::UserState::None)
		{
			cSlotIndex = 0;
			memset( wszCharName, 0, sizeof(WCHAR)*NAMELENMAX );
		}
	}SPvPUserInfo;

public:

	CDNPvPRoom();
	~CDNPvPRoom();

	bool			bIsEmpty();
	bool			bIsEmptySlot() const;
	UINT			GetIndex() const { return m_uiIndex; }
	int				GetEventRoomIndex(){ return m_nEventRoomIndex; }
	TEvent*			GetEventPtr(){ return &m_EventData; }
	UINT			GetMapIndex(){ return m_uiMapIndex; }
	UINT			GetMaxUser(){ return m_uiMaxUser; }
	UINT			GetCurUser(){ return static_cast<UINT>(m_mUserInfo.size()); }
	UINT			GetObserverUser();
	UINT			GetGameMode(){ return m_uiGameMode; }
	UINT			GetGameModeTableID(){ return m_uiGameModeTableID; }
	const WCHAR*	GetRoomName(){ return m_wstrRoomName.c_str(); }
	void			SetRoomState( const UINT uiRoomState );
	UINT			GetRoomState(){ return m_uiRoomState; }
	bool IsGuildWarSystem() { return m_bIsGuildWarSystem; }
#ifdef PRE_ADD_COLOSSEUM_BEGINNER
	BYTE GetPvPChannelType() {return static_cast<BYTE>(m_eRoomType);}
#endif		//#ifdef PRE_ADD_COLOSSEUM_BEGINNER
	bool IsExtendObserver() { return m_bExtendObserverCount; }

	int				Create( const MAVIPVP_CREATEROOM* pPacket );
	void			ModifyRoom( const MAVIPVP_MODIFYROOM* pPacket );
	int				LeaveUser( const UINT uiLeaveAccountDBID, PvPCommon::LeaveType::eLeaveType Type, const short nRetCode=ERROR_NONE );
	void			JoinUser( const MAVIPVP_JOINROOM* pPacket );
	void			ChangeTeam( const MAVIPVP_CHANGETEAM* pPacket );
	void			ChangeUserState( const MAVIPVP_CHANGEUSERSTATE* pPacket );
	WCHAR*			MakeRoomInfoList( CDNUserSendManager* pUserSendManager, PvPCommon::RoomInfoList* pDest, int iCountOffset, WCHAR* pwOffset );
	bool			bMakeJoinRoom( SCPVP_JOINROOM* pDest, int& riSize );
	void			StartPvP( const MAVIPVP_START* pPacket );
	void			StartPvPMsg( const MAVIPVP_STARTMSG* pPacket );

	//
	void			SendAllCancelStage( short cRetCode );
	void			SendAllRoomID( GameTaskType::eType GameTaskType, BYTE cReqGameIDType, USHORT wGameID, int nRoomID );
	//void			SendAllLocalSymbolData();
	void			SendAllReadyToGame( ULONG nIP, USHORT nPort, USHORT nTcpPort );
	void			SendChat( eChatType eType, int cLen, const WCHAR *pwszCharacterName, const WCHAR *pwszChatMsg, int nRet = ERROR_NONE );
	void			SendMemberIndex(USHORT nTeam, BYTE cCount, const TSwapMemberIndex * pIndex, int nRetCode);
#if defined(PRE_ADD_PVP_TOURNAMENT)
	void			SendPvPSwapTounamentIndex(char cSourceIndex, char cDestIndex);
	void			SetPvPShuffleIndex(MAVIPVPShuffleTournamentIndex* pPacket);
#endif // #if defined(PRE_ADD_PVP_TOURNAMENT)

#if defined( PRE_WORLDCOMBINE_PVP )	
	int				GetWorldPvPRoomType( ) { return m_nWorldPvPRoomType; }
	void			SetWorldPvPRoomMemberInfo( BYTE cCurUserCount ) { m_cWorldPvPRoomCurUserCount=cCurUserCount; }
	int				GetWorldPvPRoomDBIndex() { return m_nWorldPvPRoomDBIndex; }
	void			SetWorldPvPRoomDBIndex( int nWorldPvPRoomDBIndex ) { m_nWorldPvPRoomDBIndex=nWorldPvPRoomDBIndex; }
	void			SetWorldPvPRoomPw(int nPassWord) {m_nWorldPvPRoomPw = nPassWord;}
	int				GetWorldPvPRoomPw() {return m_nWorldPvPRoomPw;}
	bool			CheckLevel( BYTE cLevel );
#endif

#if defined( PRE_PVP_GAMBLEROOM )
	int			CheckGambleRoomJoin( CDNUserSession* pUserSession );
	BYTE			GetGambleRoomType() { return m_cGambleType; }
#endif

	//
	void			OnMasterServerDisconnected();

private:

	void			_MakeRoomInfo( PvPCommon::RoomInfo* pDest );
	void			_SendRoomState();
	void			_SendJoinUser( CDNUserSession* pUserObj, const SPvPUserInfo& sPvPUserInfo );
	void			_SendChangeTeam( const short nRetCode, const UINT uiSessionID, const USHORT usTeam, char cTeamSlotIndex );
	void			_SendUserState( const UINT uiSessionID, const UINT uiUserState );
	void			_BroadCast( int iMainCmd, int iSubCmd, char* pData, int iLen, CDNUserSession* pExceptUserObj=NULL );
	void			_UpdatePvPLevel( CDNUserSession* pUserSession, std::map<UINT,SPvPUserInfo>::iterator itor );

private:

	// VillageServer info
	USHORT				m_unVillageChannelID;
	// GameServer info
	int					m_iGameServerID;
	int					m_iGameRoomID;

	// Room info
	UINT				m_uiIndex;
	int					m_nEventRoomIndex;
	TEvent				m_EventData;
	UINT				m_uiMaxUser;
	UINT				m_uiMapIndex;
	UINT				m_uiRoomState;
	std::wstring		m_wstrRoomName;
	std::wstring		m_wstrRoomPW;
	UINT				m_uiGameMode;
	UINT				m_uiWinCondition;
	UINT				m_uiPlayTimeSec;
	UINT				m_uiGameModeTableID;
	BYTE				m_cMinLevel;
	BYTE				m_cMaxLevel;
	USHORT				m_unRoomOptionBit;
	bool m_bIsGuildWarSystem;
	UINT m_nGuildWarDBID[PvPCommon::TeamIndex::Max];
#ifdef PRE_ADD_COLOSSEUM_BEGINNER
	PvPCommon::RoomType::eRoomType m_eRoomType;
#endif		//#ifdef PRE_ADD_COLOSSEUM_BEGINNER
	bool m_bExtendObserverCount;
#if defined( PRE_WORLDCOMBINE_PVP )
	int		m_nWorldPvPRoomType;
	BYTE	m_cWorldPvPRoomCurUserCount;
	int		m_nWorldPvPRoomDBIndex;
	int		m_nWorldPvPRoomPw;
#endif
#if defined( PRE_PVP_GAMBLEROOM )
	BYTE    m_cGambleType;
	int		m_nGamblePrice;
#endif // #if defined( PRE_PVP_GAMBLEROOM )

	//VoiceChat
	UINT m_nVoiceChannelID[PvPCommon::TeamIndex::Max];

	std::map<UINT,SPvPUserInfo>	m_mUserInfo;
};
