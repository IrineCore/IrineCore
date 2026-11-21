#pragma once
#include "DnPVPMakeRoomDlg.h"
#include "DnPassWordInputDlg.h"
#include "DnJobIconStatic.h"
#include "DnCustomDlg.h"
#include "DnPVPPopupDlg.h"
#include "DnPVPLadderInviteConfirmDlg.h"
#ifdef PRE_ADD_PVP_HELP_MESSAGE
#include "DnPVPLobbyMapImageDlg.h"
#endif

const float fRefreshTime = 2.0f;
const float fSendRefreshTime = PvPCommon::Common::RoomListRefreshGapTime;


namespace PVPRoomList
{
	enum GameString
	{
		EXIT = 120111,
		READY_ROOM = 120028,
		RESERVATION_ROOM = 120029,
		PLAYING_ROOM = 120030,
		EXIT_DIRECT_FROM_VILLAGE = 3841,
		GUILDWAR_SYSTEM_ROOM_OBSERVER_NOT_ENTER = 126308,	// UISTRING : Cannot spectate Guild War.
		GUILDWAR_SYSTEM_ROOM_NOT_MEMBER_ENTER = 126309 ,	// UISTRING : You belong to a guild that cannot participate in this guild war.
	};

	const int MAX_SLOTNUM = 10;
}

class CDnPVPRoomListDlg : public CDnCustomDlg, public CEtUICallback
{
public:					 
	CDnPVPRoomListDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL , bool bAutoCursor = false );
	virtual ~CDnPVPRoomListDlg(void);

public:
	enum ChildWinID
	{
		ROOMMAKE_DIALOG,
		PASSWORDINPUT_DIALOG,
		EXIT_DIALOG,
#ifdef PRE_PVP_GAMBLEROOM
		CONFIRM_JOIN_GAMBLEROOM,
#endif // PRE_PVP_GAMBLEROOM
#ifdef PRE_FIX_PVP_LOBBY_RENEW
		CONFIRM_NO_BREAKINTO,
#endif
	};

	enum RoomState
	{
		CanJoin,
		Full,		
	};

	enum GameModeState
	{
		DeathMatch,
		TeamDeathMatch,		
	};

	enum SortButtonState
	{
		SortTypeNone,
		SortTypeAccend,
		SortTypeDescend
	};

	enum Page
	{
		MaxPage = 256,
	};

	enum eWaitUser
	{
		TypeLobby = 0,
		TypeFriend = 1,
		TypeGuild = 2,
		RequestTimeLimit_Friend = 60,
		RequestTimeLimit_Guild = 60, // Allowed only once per minute.
	};

	struct SRoomInfo
	{
		int iRoomIndex;
		wchar_t szRoomName[256];		
		byte byCur_PlayerNum;
		byte byMax_PlayerNum;
		wchar_t szMapName[256];
		byte cRoomState;
		bool isInGameJoin;
		byte cGameMode;
		UINT uiObjective;
		UINT uiMapIndex;
		bool bDropItem;
		byte byMin_PlayerLevel;
		byte byMax_PlayerLevel;
		int nEventID;
		UINT nGuildDBID[PvPCommon::TeamIndex::Max];
#ifdef PRE_MOD_PVPOBSERVER
		bool bEventRoom;
#endif // PRE_MOD_PVPOBSERVER
#ifdef PRE_WORLDCOMBINE_PVP
		int nWorldPvPRoomType;
#endif // PRE_WORLDCOMBINE_PVP
#ifdef PRE_PVP_GAMBLEROOM
		BYTE cGambleType;
		int nGamblePrice;
#endif // PRE_PVP_GAMBLEROOM
		SRoomInfo()
			: iRoomIndex(-1)
			, byCur_PlayerNum(0)
			, byMax_PlayerNum(0)
			, cRoomState(255)
			, isInGameJoin(false)
			, cGameMode(0)
			, uiObjective(0)
			, uiMapIndex(0)
			, bDropItem(false)
			, byMin_PlayerLevel(0)
			, byMax_PlayerLevel(100)
			, nEventID(0)
#ifdef PRE_MOD_PVPOBSERVER
			, bEventRoom(false)
#endif // PRE_MOD_PVPOBSERVER
#ifdef PRE_WORLDCOMBINE_PVP
			, nWorldPvPRoomType( WorldPvPMissionRoom::Common::NoneType )
#endif // PRE_WORLDCOMBINE_PVP
#ifdef PRE_PVP_GAMBLEROOM
			, cGambleType( PvPGambleRoom::NoneType )
			, nGamblePrice( 0 )
#endif // PRE_PVP_GAMBLEROOM
		{
			SecureZeroMemory(szRoomName,sizeof(szRoomName));
			SecureZeroMemory(szMapName,sizeof(szMapName));
			SecureZeroMemory(nGuildDBID,sizeof(nGuildDBID));
		}
	};

	struct sSlot
	{
		CEtUIStatic *pStaticState;
		CEtUIStatic *pStaticRoomNum;
		CEtUIStatic *pRoomPlayerNum;
		CEtUIStatic *pRoomName;
		CEtUIStatic *pRoomSelect;
		CEtUIStatic *pUsePassWord;
		CEtUIStatic *pGameMode;
		CEtUIStatic *pInMapName;
		CEtUIStatic *pPlayerMinMaxLevel;
#ifdef PRE_PVP_GAMBLEROOM
		CEtUIStatic *pGamblePetal;
		CEtUIStatic *pGambleGold;
#endif // PRE_PVP_GAMBLEROOM
		sSlot()
			: pStaticState(NULL)
			, pStaticRoomNum(NULL)
			, pRoomPlayerNum(NULL)
			, pRoomName(NULL)
			, pRoomSelect(NULL)
			, pUsePassWord(NULL)
			, pGameMode(NULL)
			, pInMapName(NULL)
			, pPlayerMinMaxLevel(NULL)
#ifdef PRE_PVP_GAMBLEROOM
			, pGamblePetal( NULL )
			, pGambleGold( NULL )
#endif // PRE_PVP_GAMBLEROOM
		{
		}
	};

	struct sWaitUserSlot // Required list: name, rank, class, level, click wrapper = 5 items
	{
		CEtUIStatic *pStaticName;
		CDnJobIconStatic* pStaticJobIcon;
		CEtUITextureControl *pStaticRankIcon;
		CEtUIStatic *pStaticLevel;
		CEtUIStatic *pStaticDummy;

		sWaitUserSlot()
			: pStaticName(NULL)
			, pStaticJobIcon(NULL)
			, pStaticRankIcon(NULL)
			, pStaticLevel(NULL)
			, pStaticDummy(NULL)
		{}
	};

protected:
#ifdef PRE_ADD_COLOSSEUM_BEGINNER
	PvPCommon::RoomType::eRoomType m_eSelectedPVPGradeChannel;
#endif // #ifdef PRE_ADD_COLOSSEUM_BEGINNER

	sSlot m_sSlot[PVPRoomList::MAX_SLOTNUM];
	std::vector<SRoomInfo> m_RoomInfoList;
	std::vector<sWaitUserSlot> vWaitUserSlot;

	CDnPVPMakeRoomDlg *m_pPVPMakeRoomDlg;
	CDnPassWordInputDlg *m_pPassWordInputDlg;	

#ifdef PRE_MOD_PVPOBSERVER
	CDnPassWordInputEventDlg * m_pPassWordInputEventDlg;
	bool m_bShowPasswordDlg;
#endif // PRE_MOD_PVPOBSERVER

	CDnPVPPopupDlg *m_pPVPPopupDlg;
	CDnPVPLadderInviteConfirmDlg *m_pLadderInviteConfirmDlg;

	CEtUIComboBox *m_pMapCombo;
	CEtUIComboBox *m_pModeCombo;

	CEtUIRadioButton *m_pRButtonLobby;
	CEtUIRadioButton *m_pRButtonFriend;
	CEtUIRadioButton *m_pRButtonGuild;

	CEtUIButton *m_pButtonRefreshUser;
	CEtUIButton *m_pButtonUserPrePage;
	CEtUIButton *m_pButtonUserNextPage;

	CEtUIButton *m_pReFreshButton;
	CEtUIButton *m_pQuickEnterButton;
	CEtUIButton *m_pRoomNumberSort;
	CEtUIButton *m_pMaxPlayerSort; 

	CEtUIButton *m_pJoin;
	CEtUIButton *m_pPrePage;
	CEtUIButton *m_pNextPage;

	CEtUIStatic *m_pUserSelect;
	CEtUIStatic *m_pUserPageIndex; 
	
	CEtUIStatic *m_pSelector;
	CEtUIStatic *m_pRoomNumberSortAc;
	CEtUIStatic *m_pRoomNumberSortDc;
	CEtUIStatic *m_pMaxPlayerSortAc;
	CEtUIStatic *m_pMaxPlayerSortDc;

	CEtUIStatic *m_pPageIndex;
	CEtUIStatic *m_pMapName;
	CEtUIStatic *m_pModeName;

	CEtUIButton *m_pObserverJoin;
	bool m_bObserverJoin;
	bool m_isFirstRoomListPacket; //Whether room list was received for the first time
#ifdef PRE_ADD_PVP_VILLAGE_ACCESS
	bool m_bVillageAccessMode;
#endif

	std::wstring m_wszDefaultMapName;
	std::wstring m_wszDefaultModeName;
	
	UINT m_uiBackUpRoomIndex;
	UINT m_uiCurPage;
	UINT m_uiMaxPage;

	BYTE m_cRoomSortType;
	SortButtonState m_emRoomNumAscend;
	SortButtonState m_emMaxPlayerAscend;

	int m_iDataIndex;
	int m_nUserType; // Guild, friend, standby list 
	int m_nUserPageIndex;
	int m_nUserMaxPage;
	int m_nRefreshUserCount;
	int m_nSelectedRoomIndex;

	float m_fRequestGuildTimeLimit;
	float m_fRequestFriendTimeLimit;
	float m_fRefreshTime;

#ifdef PRE_ADD_PVP_HELP_MESSAGE
	CDnPVPLobbyMapImageDlg* m_pLobbyImagePopUpDlg;
#endif
	
public:
	virtual void InitialUpdate();
	virtual void Initialize( bool bShow );
	virtual void OnUICallbackProc( int nID, UINT nCommand, CEtUIControl *pControl, UINT uiMsg = 0 );
	virtual void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg = 0 );
	virtual void Process( float fElapsedTime );
	virtual bool MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
	//virtual void Render( float fElapsedTime );
	virtual void Show( bool bShow );

	void AddLobbyList( sRoomInfo * RoomInfo , UINT uiMaxPage );
	void RefreshRoomList();
	void Reset();
	void SetFirstRoom( bool FirstPacket ){m_isFirstRoomListPacket = FirstPacket;};
	void SendExitLobby();
	void OpenExitDlg();
	void ClearvecRoomInfo(){m_RoomInfoList.clear();};

	void InitializeWaitUserList();
	void ClearWaitUserList(bool bShow = false);
	void SetWaitUserList(PvPCommon::WaitUserList::Repository sUserList , UINT uiMaxPage);
	void SetFriendList(int nPageNumber);
	void SetGuildList(int nPageNumber);
	void SetRefreshUserCount(int nCount);
	int GetCurrentRefreshUserType(){return m_nUserType;}
	void ShowLadderInviteConfirmDlg(LadderSystem::SC_INVITE_CONFIRM_REQ *pData);

#ifdef PRE_ADD_PVP_VILLAGE_ACCESS
	void EnableVillageAccessMode(bool bTrue);
#endif

#ifdef PRE_ADD_COLOSSEUM_BEGINNER
	void SetSelectedGradeChannel( PvPCommon::RoomType::eRoomType ePVPGradeChannel ) { m_eSelectedPVPGradeChannel = ePVPGradeChannel; };
	PvPCommon::RoomType::eRoomType GetSelectedGradeChannel( void ) { return m_eSelectedPVPGradeChannel; };
	void RefreshRoomByChangeGradeChannel( void ) { m_fRefreshTime = PvPCommon::Common::RoomListRefreshGapTime; };
#endif // #ifdef PRE_ADD_COLOSSEUM_BEGINNER

#ifdef PRE_MOD_PVPOBSERVER
	void ResetObserverJoinFlag(){
		m_bObserverJoin = false;
	}
#endif // PRE_MOD_PVPOBSERVER

protected:

	void ClearRoomInfo();
	void SetRoomInfo(sRoomInfo *RoomInfo);
	void SetRoomListColor(DWORD dwColor,int nRoomIndex);
	void UpdateRoomInfo( );

	void SendJoinRoom( int iRoomindex, bool bObserver = false );

	void SendRefresh( UINT Page);
	void SendRefreshWaitUserList(UINT Page);
	void UpdateSelectBar();
	void MakeMapList();
	void MakeModeList();

	bool IsReadyRoom( byte cRoomState ){ return cRoomState == PvPCommon::RoomState::None || cRoomState == PvPCommon::RoomState::Password; }
	bool IsPasssWordRoom( byte cRoomState ){ return cRoomState&PvPCommon::RoomState::Password?true:false; }
	bool IsSyncingRoom(  byte cRoomState ){ return cRoomState&PvPCommon::RoomState::Syncing?true:false; }
	bool IsStartingRoom( byte cRoomState ){ return cRoomState&PvPCommon::RoomState::Starting?true:false; }
	bool IsPlayingRoom(byte cRoomState ){ return cRoomState&PvPCommon::RoomState::Playing?true:false; }

	int FindInsideSlot( float fX, float fY );

	bool IsGuildWarRoomEnter( const int iRoomIndex, const bool bObserver );
#ifdef PRE_PVP_GAMBLEROOM
	void CheckJoinGambleRoom();
#endif // PRE_PVP_GAMBLEROOM
#ifdef PRE_FIX_PVP_LOBBY_RENEW
	bool CheckJoinRoom(const SRoomInfo& info);
#endif
};
