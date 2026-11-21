#pragma once
#include "EtUIDialog.h"
#include "DnSmartMoveCursor.h"


#ifdef PRE_MOD_PVP_ROOM_CREATE

namespace PVPMakeGame
{
	enum ErrorCodeString
	{
		RoomNameIsNULL = 100048,
		RoomNameIsWrong = 121006,
		PasswordShort = 121008,
	};
}

class CDnPVPMakeRoomDlg : public CEtUIDialog, public CEtUICallback
{
public:
	CDnPVPMakeRoomDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnPVPMakeRoomDlg(void);

	enum eMakeRoomCommon
	{
		MapData_PlayerMAX = 5,
		MapData_ModeMAX = 10,
	};

protected:
	CEtUIEditBox *m_pEditBoxMinLevel;
	CEtUIEditBox *m_pEditBoxMaxLevel;
	CEtUIEditBox *m_pEditBoxRoomName;
	CEtUIEditBox *m_pEditBoxPassWord;

	CEtUIComboBox *m_pComboBoxGameMode;			// Value: Cell_ID of PvPGameModeTable.
	CEtUIComboBox *m_pComboBoxMapSelection;		// Value: TableMapID
	CEtUIComboBox *m_pComboBoxPlayerNum;
	CEtUIComboBox *m_pComboBoxVictoryCondition;
	
	CEtUICheckBox *m_pCheckBoxDropItem;
	CEtUICheckBox *m_pCheckBoxInGameJoin;
	CEtUICheckBox *m_pCheckBoxShowHP; 
	CEtUICheckBox *m_pCheckBoxRandomTeamMode;
	CEtUICheckBox *m_pCheckBoxRevision;
	CEtUICheckBox *m_pCheckBoxRandomOrder;
	CEtUIStatic	  *m_pStaticRandomOrder;
#ifdef PRE_MOD_PVPOBSERVER
	CEtUICheckBox* m_pCheckBoxObserver;
#endif // PRE_MOD_PVPOBSERVER
#ifdef PRE_WORLDCOMBINE_PVP
	CEtUIStatic*   m_pStaticWorldCombineRoom;
	CEtUICheckBox* m_pCheckBoxWorldCombineRoom;
#endif // PRE_WORLDCOMBINE_PVP
	CEtUIButton   *m_pExitButton;

	CEtUITextureControl *m_pMiniMap;		// Map image
	EtTextureHandle m_hMiniMapImage;		// Image to be displayed next to the map.
	
	CEtUITextureControl *m_pGameModeImage;	// Game mode image
	EtTextureHandle m_hGameModeImage;		// Image to display for game mode.

	int  m_SelectedIndex;
	int  m_SelectedMapIndex;
	int  m_SelectedPlayerNum;
	int  m_SelectedModeIndex;
	int  m_nGameModeTableID;
	int  m_nWinCondition;
	int  m_nPlayTimeSec;
	bool m_IsInGameJoin;
	bool m_IsFirstInputRoomName;
	bool m_bDropItem;
	bool m_bShowHp;
	bool m_bRevision;
	bool m_bRandomTeamMode;
	bool m_bRandomOrder;
	int  m_nCurrSelectModeVectorIndex;
#ifdef PRE_MOD_PVPOBSERVER
	bool m_bObserverAccess;
#endif // PRE_MOD_PVPOBSERVER

	struct sPvPMapData 
	{
		int  nMapTableID;			 // MapTableID
		int  nMapNameStringID;		 // MapNameUIstring
		int  nDefaultNumOfPlayerOpt; // DefaultNumOfPlayers
		bool bIsAllowBreak;			 // Allow_Breakin_PlayingGame (whether break-in is allowed on this map)
		std::string strMapImageName; // MapImage
		int  nNumOfPlayerOpt[MapData_PlayerMAX]; // NumOfPlayersOption1~5

		sPvPMapData()
			: nMapTableID(0)
			, nMapNameStringID(0)
			, nDefaultNumOfPlayerOpt(0)
			, bIsAllowBreak(false)
		{
			SecureZeroMemory(nNumOfPlayerOpt, sizeof(nNumOfPlayerOpt));
		}
	};
	struct sPvPModeData
	{
		int nTableID;					// ID
		int nGameMode;					// GamemodeID
		int nGameModeStringID;			// GameModeUIString
		int nLadderType;				// LadderType
		int nNumOfPlayers_Min;			// NumOfPlayers_Min;
		int nNumOfPlayers_Max;			// NumOfPlayers_Max;
		std::string strModeImageName;	// _ModeImg
		std::vector<sPvPMapData> vAssignMapDataList; // List of maps used in the current mode.
		bool bIsReleaseShow;

		sPvPModeData()
			: nTableID(0)
			, nGameMode(0)
			, nGameModeStringID(0)
			, nLadderType(0)
			, nNumOfPlayers_Min(0)
			, nNumOfPlayers_Max(0)
			, bIsReleaseShow(false)
		{}
	};
	std::vector<sPvPModeData> m_vGameModeList;
	
	// Smart move
	CDnSmartMoveCursor m_SmartMove;

	void MakePvPGameModeData();
	void MakeMapImage( UINT index );
	void MakeGameModeImage( UINT index );
	
	void MakeMapDropDownList( UINT nVecIndex );	// Create Map ComboBox
	void MakeGameModeDropDownList();			// Create GameMode ComboBox
	void MakeGameModeDropDownList( UINT index );// Create GameMode ComboBox
	void MakeDropPlayDownList( UINT index );	// Create player count ComboBox
	void MakeVicConDropDownList();				// Create victory condition ComboBox
	void SetGameModeButtonState();				// Set other button states.

	void GetGamemodeInfo( int nIndex );
	const wchar_t* MakeDefaultRoomName();
	virtual void SendMakeRoom();

public:
	void SetMapAndModeDropDownList( UINT nCurMapIndex, UINT nCurModeIndex, LPCWSTR wszItemName );
	void SetMaxPlayerDownList( byte cMaxUserNum );
	void SetInGameJoin( bool IsJoin  );
	void SetGameWinObjectNum( LPCWSTR wszItemName  );
	void SetDropItem( bool bDropItem );
	void SetShowHp( bool bDropItem );
	void SetRevision( bool bRevision );
	void SetRandomOrder( bool bRandomOrder );
	void SetRandomTeamMode( bool bRandomTeamMode );
	void InitCheckBox();
	void SetPlayerLevel(int nLevelMin, int nLevelMax); // Rotha: Set PVP level limit
#ifdef PRE_MOD_PVPOBSERVER
	void SetObserver( bool b );
#endif // PRE_MOD_PVPOBSERVER

	virtual bool IsEditRoom() { return false; }
	virtual void InitialUpdate();
	virtual void Initialize( bool bShow );
	virtual void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg = 0 );
	virtual void Show( bool bShow );
#ifdef PRE_ADD_PVP_HELP_MESSAGE
	virtual bool MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
#endif

#ifdef PRE_ADD_PVP_TOURNAMENT
	int GetGameModeData(int nGameModeTableID, const char* pLableString) const;
#endif
	void Shuffle();
};





#else // PRE_MOD_PVP_ROOM_CREATE


namespace PVPMakeGame
{
	enum ErrorCodeString
	{
		RoomNameIsNULL = 100048,
		RoomNameIsWrong = 121006,
		PasswordShort = 121008,
	};
}

class CDnPVPMakeRoomDlg : public CEtUIDialog, public CEtUICallback
{

public:
	CDnPVPMakeRoomDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnPVPMakeRoomDlg(void);

	enum eMakeRoomCommon
	{
		MapData_PlayerMAX = 5,
		MapData_ModeMAX = 10,

	};
protected:

	CEtUIEditBox *m_pEditBoxMinLevel;
	CEtUIEditBox *m_pEditBoxMaxLevel;
	CEtUIEditBox *m_pEditBoxRoomName;
	CEtUIEditBox *m_pEditBoxPassWord;

	CEtUIComboBox *m_pComboBoxMapSelection;
	CEtUIComboBox *m_pComboBoxPlayerNum;
	CEtUIComboBox *m_pComboBoxGameMode;
	CEtUIComboBox *m_pComboBoxVictoryCondition;

	CEtUICheckBox *m_pCheckBoxDropItem;
	CEtUICheckBox *m_pCheckBoxInGameJoin;
	CEtUICheckBox *m_pCheckBoxShowHP; 
	CEtUICheckBox *m_pCheckBoxRandomTeamMode;
	CEtUICheckBox *m_pCheckBoxRevision;
	CEtUICheckBox *m_pCheckBoxRandomOrder;
	CEtUIStatic *m_pStaticRandomOrder;

#ifdef PRE_MOD_PVPOBSERVER
	CEtUICheckBox * m_pCheckBoxObserver;
#endif // PRE_MOD_PVPOBSERVER
#ifdef PRE_WORLDCOMBINE_PVP
	CEtUIStatic* m_pStaticWorldCombineRoom;
	CEtUICheckBox* m_pCheckBoxWorldCombineRoom;
#endif // PRE_WORLDCOMBINE_PVP

	CEtUIButton *m_pExitButton;

	CEtUITextureControl *m_pMiniMap;
	EtTextureHandle m_hMiniMapImage; //Image to be displayed next to the map

	int m_SelectedIndex;
	int m_SelectedMapIndex;
	int m_SelectedPlayerNum;
	int m_SelectedModeIndex;
	int m_nGameModeTableID;
	int m_nWinCondition;
	int m_nPlayTimeSec;
	bool m_IsInGameJoin;
	bool m_IsFirstInputRoomName;
	bool m_bDropItem;
	bool m_bShowHp;
	bool m_bRevision;
	bool m_bRandomTeamMode;
	bool m_bRandomOrder;

#ifdef PRE_MOD_PVPOBSERVER
	bool m_bObserverAccess;
#endif // PRE_MOD_PVPOBSERVER


	struct sPvpMapData{
		int bPlayerNum[MapData_PlayerMAX];
		int nGameMode[MapData_ModeMAX];
		int bRecommandedPlayerNum;
		WCHAR szMapName[256];
		UINT nMapIndex;
		UINT nUIStringIndex;
		bool isInGameJoin;
		std::string szMapImage;

		sPvpMapData()
			: bRecommandedPlayerNum(0)
			, nMapIndex(0)
			, nUIStringIndex(0)
			,isInGameJoin(false)

		{
			SecureZeroMemory(szMapName,sizeof(szMapName));
			SecureZeroMemory(bPlayerNum,sizeof(bPlayerNum));
			SecureZeroMemory(nGameMode,sizeof(nGameMode));
		}
	};

	struct sPVPGameMode{
		int nCondition;
		int nTime;

		sPVPGameMode()
			:nCondition(0)
			,nTime(0)
		{
		}
	};

	// Smart move
	CDnSmartMoveCursor m_SmartMove;

	std::vector<sPvpMapData> vMapData;	
	void RefreshRoomList();
	void MakePvPMapData();
	void MakeDropDownList();
	void MakeDropPlayDownList( UINT index );
	void MakeMapImage( UINT index );
	const wchar_t* MakeDefaultRoomName();
	void MakeGameModeDropDownList( UINT index );
	void MakeVicConDropDownList( );
	void GetGamemodeInfo( int nIndex );
	void SetGameModeButtonState();
	virtual void SendMakeRoom();

#ifdef PRE_ADD_PVP_HELP_MESSAGE
	virtual bool MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
#endif

public:

	void SetMapDropDownList( UINT nCurMapIndex, LPCWSTR wszItemName , byte cCurMaxUserNum );
	void SetGameModeDownList( UINT nCurMapIndex , LPCWSTR wszItemName  );
	void SetMaxPlayerDownList( byte cMaxUserNum );
	void SetInGameJoin( bool IsJoin  );
	void SetGameWinObjectNum( LPCWSTR wszItemName  );
	void SetDropItem( bool bDropItem );
	void SetShowHp( bool bDropItem );
	void SetRevision( bool bRevision );
	void SetRandomOrder( bool bRandomOrder );
	void SetRandomTeamMode( bool bRandomTeamMode );
	void InitCheckBox();
	void SetPlayerLevel(int nLevelMin, int nLevelMax); // Rotha: Set PVP level limit
#ifdef PRE_MOD_PVPOBSERVER
	void SetObserver( bool b );
#endif // PRE_MOD_PVPOBSERVER

	virtual bool IsEditRoom() { return false; }
	virtual void InitialUpdate();
	virtual void Initialize( bool bShow );
	virtual void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg = 0 );
	virtual void Show( bool bShow );

	void Shuffle();
#ifdef PRE_ADD_PVP_TOURNAMENT
	int GetGameModeData(int nGameModeTableID, const char* pLableString) const;
#endif
};


#endif // PRE_MOD_PVP_ROOM_CREATE