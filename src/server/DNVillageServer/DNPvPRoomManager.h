
#pragma once

class CDNPvPRoom;
class CDNUserSession;
#if defined( PRE_WORLDCOMBINE_PVP )
class CDNDBConnection;
#endif //#if defined( PRE_WORLDCOMBINE_PVP )

class CDNPvPRoomManager:public CSingleton<CDNPvPRoomManager>
{
public:

	CDNPvPRoomManager();
	virtual ~CDNPvPRoomManager();

	bool		bIsExist( const USHORT unVillageChannelID, const UINT uiPvPIndex );
	CDNPvPRoom*	GetPvPRoom( const USHORT unVillageChannelID, const UINT uiPvPIndex );
	short 		CreatePvPRoom( CDNUserSession* pUserObj, const MAVIPVP_CREATEROOM* pPacket );
	bool		bDestroyPvPRoom( const USHORT unVillageChannelID, const UINT uiPvPIndex );
#if defined(PRE_ADD_PVP_VILLAGE_ACCESS)
	void		SendPvPRoomList( CDNUserSession* pUserObj, const MAVIPVP_ROOMLIST* pPacket, BYTE cVillageID=0, UINT uiAccountDBID=0 );
#else
	void		SendPvPRoomList( CDNUserSession* pUserObj, const MAVIPVP_ROOMLIST* pPacket);
#endif
	void		SendPvPWaitUserList( CDNUserSession* pUserSession, const MAVIPVP_WAITUSERLIST* pPacket );
	void		OnDisconnected( const UINT uiConnectionKey );
	
	void		DoUpdate( DWORD dwCurTick );
#if defined( PRE_WORLDCOMBINE_PVP )
	CDNDBConnection* GetDBConnection();
	int			GetWorldPvPRoomChannelID() { return m_nWorldPvPChannelID; }	
	void		OnRecvWorldPvPRoomList( const TAGetListWorldPvPRoom *pPacket );
	void		CreateWorldPvPRoom();
	void		DelWorldPvPRoom( int nRoomIndex );
	TWorldPvPRoomDBData* GetWorldPvPRoom( UINT uiPvPIndex );
	int			GetGMWorldPvPRoomIndex() { return m_nGMWorldPvPRoomIndex++; }
	void		SetApplyWorldPvPRoom( bool bFlag ) { m_bApplyWorldPvPRoom = bFlag; }
#endif
	// Function for unittest
#ifdef PRE_ADD_COLOSSEUM_BEGINNER
	std::list<CDNPvPRoom*>	GetList( const UINT uiChannel )
	{
		std::vector<std::list<CDNPvPRoom*>> veclist = m_listPvPRoom[uiChannel];
		return veclist[PvPCommon::RoomType::beginner];
	}
#else		//#ifdef PRE_ADD_COLOSSEUM_BEGINNER
	std::list<CDNPvPRoom*>	GetList( const UINT uiChannel ){ return m_listPvPRoom[uiChannel]; }
#endif		//#ifdef PRE_ADD_COLOSSEUM_BEGINNER

#if defined( PRE_PVP_GAMBLEROOM )
	void SetGambleRoomData( int nRoomIndex, bool bRegulation, BYTE cGambleType, int nPrice, bool bFlag );
	void SetCreateGambleRoomFromDolis( bool bFlag );	
	void CreateGambleRoom( int nTotalGambleRoomCount );
	void DelGambleRoom();
	int GetPvPLobbyChannelID();
#endif

private:

	DWORD m_dwUpdateTick;

	std::map<std::pair<USHORT,UINT>,CDNPvPRoom*>	m_mPvPRoom;			// first:(VillageChannelID,PvPIndex)
#ifdef PRE_ADD_COLOSSEUM_BEGINNER
	std::map<USHORT,std::vector<std::list<CDNPvPRoom*>>> m_listPvPRoom;		// frist:VillageChannelID
#else		//#ifdef PRE_ADD_COLOSSEUM_BEGINNER
	std::map<USHORT,std::list<CDNPvPRoom*>>			m_listPvPRoom;		// frist:VillageChannelID
#endif		//#ifdef PRE_ADD_COLOSSEUM_BEGINNER
	std::map<USHORT,std::list<CDNPvPRoom*>>			m_listPvPEventRoom;	// frist:EventID

#if defined( PRE_WORLDCOMBINE_PVP )
	int m_nWorldPvPChannelID;
	DWORD m_dwGetWorldPvPRoomTick;
	CDNDBConnection* m_pDBCon;
	std::map<int, TWorldPvPRoomDBData>				m_mapWorldPvPRoom;
	int												m_nGMWorldPvPRoomIndex;	
	std::list<int>									m_listWorldPvpReqRoom;	
	bool											m_bApplyWorldPvPRoom;
#endif

#if defined( PRE_PVP_GAMBLEROOM )
	bool m_bCreateGambleRoomFromDolis;	// Doris setting value
	TPvPGambleRoomData m_GambleRoomData;
	int m_nPvPLobbyChannelID;
#endif // #if defined( PRE_PVP_GAMBLEROOM )
	long m_nMasterDisconnected;
	void MasterDisconnectProcess();
};
