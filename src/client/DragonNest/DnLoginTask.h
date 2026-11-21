#pragma once

#include "Task.h"
#include "InputReceiver.h"
#include "LoginClientSession.h"
#include "EtMatrixEx.h"
#include "DnParts.h"
#include "MAPartsBody.h"
#include "DnHangCrashTask.h"



#ifdef PRE_MOD_SELECT_CHAR


class CDnWorld;


class CDnLoginTask : public CTask, public CLoginClientSession, public CInputReceiver, public CEtUICallback, public CDnHangMonitorObject
{
public:
	CDnLoginTask();
	virtual ~CDnLoginTask();

	enum LoginStateEnum 
	{
		Logo,
		RequestIDPass,
		CharSelect,
		CharCreate_SelectClass,
		CharCreate_ChangeParts,
		CharDelete,
		StartGame,
		ServerList,
		ChannelList,
	};
#ifdef PRE_ADD_RELOGIN_PROCESS
	std::wstring m_strConnectID;
	std::wstring m_strConnectPassword;
#endif // PRE_ADD_RELOGIN_PROCESS

protected:
	enum UICallbackEnum 
	{
		ConnectLoginFailed = 110000,
		CheckVersionFailed,
		CheckLoginFailed,
		CharListFailed,
		CharCreateFailed,
		CharDeleteFailed,
		GameInfoFailed,
		VillageInfoFailed,
		DisconnectTCP,
		SkipTutorial,
		CharReviveFailed,
	};

	enum 
	{
		HAIR_COLOR_COUNT = 4,
		SKIN_COLOR_COUNT = 4,
		EYE_COLOR_COUNT = 4,
	};

	enum
	{
		CAM_IDLE = 0,
		CAM_SELECT,
		CAM_DESELECT,
		CAM_FADEOUT,
	};

	struct DefaultPartsStruct 
	{
		std::vector<int> nVecList[EQUIPMAX];
		int nSelectPartsIndex[EQUIPMAX];
		DWORD dwColor[MAPartsBody::PartsColorAmount];
		int nWeapon[2];
		std::vector<DnPartsHandle> hVecParts[EQUIPMAX];
		DnWeaponHandle hWeapon[2];
#ifdef PRE_FIX_WEAPON_DUPLICATE_RENDER
		DnWeaponHandle hDeafaultWeapon[2];
#endif // PRE_FIX_WEAPON_DUPLICATE_RENDER
	};

	struct ClassPropCamInfo
	{
		int					nPropIndex;
		std::vector<string> vecCamList;
		std::vector<int>	vecSoundList;

		~ClassPropCamInfo() { vecCamList.clear(); vecSoundList.clear(); }
	};

	DnCameraHandle					m_hCamera;
#ifndef _FINAL_BUILD
	DnCameraHandle					m_hDebugCamera;
#endif // _FINAL_BUILD
	SCCharList						m_CharList;					// For pre-receiving character list packet

	int								m_nCurrentMapIndex;			// Map index
	std::vector<int>				m_vecTitleMapIndex;			// Title map index
	std::vector<TCharListData>		m_vecCharListData;			// Full character list data
	DnActorHandle					m_hSlotActorHandle;			// Handle to use for selected character or character creation
	DnEtcHandle						m_hCommonEffectHandle;		// Effect handle used when a character is selected
	EtOutlineHandle					m_hSelectOutline;			// Selection outline handle
	std::vector<EtOutlineHandle>	m_hVecSelectCharOutline;	// For a single flash effect when selecting a character class
	int								m_nSlotActorMapID;			// Selected character map ID
	std::wstring					m_wstrCreateCharName;		// Name to save when receiving character creation complete packet (used to navigate to created character page)

	int								m_nCharSlotCount;			// Number of character slots
	int								m_nCharSlotMaxCount;		// Maximum number of character slots
	int								m_nSelectCharIndex;			// Selected character index
	int								m_nSelectedServerIndex;		// Selected server index
	int								m_nSelectedChannelIndex;	// Selected channel index

	std::vector<DnActorHandle>		m_hVecCreateActorList;			// Character creation window actor list
	std::vector<DefaultPartsStruct> m_VecCreateDefaultPartsList;	// Default parts list for character creation screen actor
	std::vector<DefaultPartsStruct>	m_VecPreviewCostumePartsList;	// Character creation window preview parts (Lv.30 default gear, guild costume, seed gear)
#ifdef PRE_ADD_CHARSELECT_PREVIEW_CASHITEM
	std::vector<DefaultPartsStruct>	m_vecCharSelectDlgPreviewCostumePartsList;  // Character selection screen preview parts (only has Lv. 30 default equipment)
#endif
	
	EtOutlineHandle					m_hOutline;						// Outline handle
	MatrixEx						m_CreateOffset;					// Character creation screen actor position (placed on map as invisible prop)
	
	std::map<int, ClassPropCamInfo>	m_mapPropCamListByClass;		// Per-character camera animation list

	int								m_nSelectClassIndex;		// Selected job index
	bool							m_bCompleteCharList;		// Whether character list load is complete
	float							m_fChangePartsFadeInTime;	// Delay margin before character creation fade-in starts

	LoginStateEnum					m_State;					// Login task state
	LoginStateEnum					m_PrevState;				// Previous state
	CDnWorld*						m_pWorld;					// World

	int								m_nLoginTitleMapIndex;		// Title map index
	int								m_nLoginCharMapIndex;		// Character selection & creation map index
	int								m_nLoginTitlePropIndex;		// Title main prop index

	float							m_fOldDOFNearBlurSize;
	float							m_fOldDOFFarBlurSize;
	float							m_fOriginalFarStart;
	float							m_fOriginalFarEnd;
	float							m_fOriginalFocusDist;

	float							m_fShowOutlineTime;			// Time to show and hide full outline
	bool							m_bRequestWait;

	int								m_nTitleSoundIndex;			// Title BGM
	int								m_nSelectSoundIndex;
	EtSoundChannelHandle			m_hTitleSoundChannel;		// Title BGM Channel
#ifdef PRE_ADD_RELOGIN_PROCESS
	int								m_nReLoginCount;
#endif // PRE_ADD_RELOGIN_PROCESS
	std::string						m_strTitleIdleCamera;			// Title default camera
	std::string						m_strTitleFadeOutCamera;		// Title fade-out camera
	std::string						m_strSelectCamera;				// Character selection camera
	std::string						m_strCharCreateSelectCamera;	// Character creation class selection camera
	std::string						m_strCharCreateStartCamera;		// Character creation start camera
	int								m_nCharCreateStartSoundIndex;	// Character creation start sound index
	std::string						m_strMakingCamera;				// Creation camera
	std::string						m_strMakingCameraFadeIn;		// Creation camera fade in

	bool							m_bStartSelectMove;				// Character selection movement
	bool							m_bStartCharSelect;				// Show UI after the opening animation of character class selection
	bool							m_bShowBlackDragonRadialBlur;	// Black Dragon roar camera effect
	bool							m_bCharSelectToServerList;		// Go from character selection screen to server selection screen (handled as exception due to tangled fade-in state)
	bool							m_bLoginInitParts;
#if defined( PRE_ADD_DWC )
	bool							m_bDWCCharCreateMode;
	BYTE							m_cDWCJobCode1;
	BYTE							m_cDWCJobCode2;
	BYTE							m_cDWCLevel;
	int								m_nDWCExp;
	int								m_nDWCMapID;
	int								m_nDWCGold;
	int								m_nDWCSkillPoint;
#endif // PRE_ADD_DWC
protected:
	void InitSelectCharacter();
	void ShowSelectCharacter( bool bShow );
	void ShowSelectCharOutline( bool bShow, float fSpeed );
	void InitCreateCharacter();
	void ShowCreateCharacter( bool bShow, int nClassID = -1 );
	int GetCreateSelectClass();
	void SetSelectClassOutLine( int nSelectIndex );

	void InitShowPartsList();
	void ResetShowPartsList();
	void ShowDefaultPartsList( bool bFirstRandom = false );
	void DetachAllParts();

	void SetRequestWait( bool bWait );
	bool LoadTitle();
	bool LoadWorld( int nMapIndex );
	void PlayDefaultCamera();

	void GetOriginalDOFInfo();
	void RestoreOriginalDOFInfo();
	void SetDOFInfo( float fDOFStart, float fDOFEnd, float fDOFFocusDistance, float fNearBlurSize, float fFarBlurSize );
	static bool SortByServerIndex( const TCharListData& a, const TCharListData& b );
#ifdef PRE_CHARLIST_SORTING
	static bool SortByCreate( const TCharListData& a, const TCharListData& b );
	static bool SortByName( const TCharListData& a, const TCharListData& b );
	static bool SortByLevel( const TCharListData& a, const TCharListData& b );
	static bool SortByJob( const TCharListData& a, const TCharListData& b );
	static bool SortByLatelyLogin( const TCharListData& a, const TCharListData& b );
#endif // PRE_CHARLIST_SORTING

public:
	void PreInitialize( bool bReload = false );
	bool Initialize( LoginStateEnum State );

	virtual void Process( LOCAL_TIME LocalTime, float fDelta );

	// CInputReceiver
	virtual void OnInputReceive( int nReceiverState, LOCAL_TIME LocalTime = -1 );

	// Client Session
	virtual void OnConnectTcp();
	virtual void OnDisconnectTcp( bool bValidDisconnect );
	// Tcp
	virtual void OnRecvLoginCheckVersion( SCCheckVersion *pPacket );
	virtual void OnRecvLoginCheckLogin( SCCheckLogin *pPacket );
	virtual void OnRecvLoginCharList( SCCharList *pPacket );
	void _OnRecvLoginCharList( SCCharList *pPacket );
	virtual void OnRecvLoginWaitUser( SCWaitUser *pPacket );
	virtual void OnRecvLoginBlock( SCCHeckBlock *pPacket );
	virtual void OnRecvLoginCharCreate( SCCreateChar *pPacket );
	virtual void OnRecvLoginCharDelete( SCDeleteChar *pPacket );
	virtual void OnRecvLoginServerList( SCServerList *pPacket );
	virtual void OnRecvLoginChannelList( SCChannelList *pPacket );
	virtual void OnRecvLoginBackButton();
	virtual void OnRecvLoginReviveChar( SCReviveChar *pPacket );
	

	virtual void OnRecvSystemGameInfo( SCGameInfo *pPacket );
	virtual void OnRecvSystemVillageInfo( SCVillageInfo *pPacket );

	// CEtUICallback
	virtual void OnUICallbackProc( int nID, UINT nCommand, CEtUIControl *pControl, UINT uiMsg = 0 );

	// Local
	bool IsEmptySelectActorSlot();
	int GetSelectActorSlot() { return m_nSelectCharIndex; }
	void ChangeState( LoginStateEnum State, bool bAutoSendMessage = true );

	void CreateCharacter( TCHAR* szName, int nSelectServerIndex );
#if defined( PRE_ADD_DWC )
	void StartDWCCharCreate( int nClassID, int nJobCode1, int nJobCode2, int nLevel, int nExp, int nMapID, int nGold, int nSkillPoint );
	void ResetDWCCharCreateMode();
	bool IsDWCCharCreateMode() const { return m_bDWCCharCreateMode; }
#endif // PRE_ADD_DWC
	void DeleteCharacter();
	bool CheckSelectCharDeleteWait();
	bool IsSelectClass() { return m_nSelectClassIndex != -1 ? 1 : 0; }
	void SetAuthPassword( int nSeed, int nValue[] ) 
	{
		m_nAuthPasswordSeed = 1234;
		memcpy( m_nAuthPasswordValue, nValue, sizeof(m_nAuthPasswordValue) );
	}

	int m_nAuthPasswordSeed;
	int m_nAuthPasswordValue[4];

	void SetCharSelect( int nSelectIndex );
	void ReviveCharacter( int nSlotIndex );
	void SelectServer( int nServerIndex );
	void SelectChannel( int nChannelIndex );
	int  GetSelectServerIndex(){return m_nSelectedServerIndex;}

	void SetCharColor( MAPartsBody::PartsColorEnum colorType, int nColorIndex, int nClassIndex = -1);
	void RotateCreateParts( CDnParts::PartsTypeEnum PartsIndex, bool bAdd );

	bool IsCompleteCharList() { return m_bCompleteCharList; }
	bool IsRequestWait() { return m_bRequestWait; }

	int GetCurrentSelectLevel();
	LoginStateEnum GetState() { return m_State; }

	virtual bool OnCloseTask();

	void DeSelectCharacter();
	void StartCharacterChangeParts();
	void SelectMoveCharacter( int nSelectIndex );

	INT64 GetSelectCharacterDBID( int nSelectIndex );
#ifdef PRE_MOD_CREATE_CHAR
	void SetCharCreateBasePartsName();	// Set initial default part names in UI on character creation
#endif // PRE_MOD_CREATE_CHAR
	void CharacterAddRotateYaw( float fAngle );
	void CharCreateGestureAction( const char* pGestureActionName, bool bCharCreate );
	void ShowPartsList( int nPartsListIndex );

#ifdef PRE_ADD_DOORS
	void OnRecvDoorsAuthKey(SCDoorsGetAuthKey * pPacket);
	void OnRecvDoorsCancelAuth(SCDoorsCancelAuth * pPacket);
	void OnRecvDoorsAuthFlag(SCDoorsGetAuthFlag* pPacket);
#endif		//#ifdef PRE_ADD_DOORS

#ifdef PRE_ADD_CHARSELECT_PREVIEW_CASHITEM
	enum {
		PREVIEW_CASH_COSTUME_RESET = 0 ,
		PREVIEW_CASH_COSTUME_ATTACH = 1,
	};

	void ResetPreviewCashCostume();
	void ShowPreviewCashCostume(int nRadioBtnIndex);
	void AttachPreviewCashCostume(int nRadioBtnIndex);
	void DetachPreviewCashCostume();

	void InitPreviewCashCostumeList();
	void ResetPreviewCashCostumeList();
#endif
	void SetSelectedCharacterDark(int JobID);
	LoginStateEnum	GetLastState(){ return m_PrevState; }
#ifdef PRE_CHARLIST_SORTING
	enum eCharacterListSortCommand
	{
		CHARACTERLIST_SORT_BY_NONE = 0,
		CHARACTERLIST_SORT_BY_SERVER = 1,
		CHARACTERLIST_SORT_BY_CREATE,
		CHARACTERLIST_SORT_BY_NAME,
		CHARACTERLIST_SORT_BY_LEVEL,
		CHARACTERLIST_SORT_BY_JOB,
		CHARACTERLIST_SORT_BY_LATELYLOGIN,
	};
	eCharacterListSortCommand	m_eCharacterListSortCommand;
	std::vector<TCharListData>	m_vecOriginCharListData;			// Full character list data (backup for initial list)
	void SortCharacterList( eCharacterListSortCommand sortCommand );
	int GetCharacterListSortCommand() { return m_eCharacterListSortCommand; }
#endif // PRE_CHARLIST_SORTING

};


#else // PRE_MOD_SELECT_CHAR


#define SHOWCHARNUMPERONEPAGE	4		// Number of characters displayed per page

class CDnWorld;

class CDnLoginTask : public CTask, public CLoginClientSession, public CInputReceiver, public CEtUICallback, public CDnHangMonitorObject
{
public:
	CDnLoginTask();
	virtual ~CDnLoginTask();

	enum LoginStateEnum {
		Logo,
		RequestIDPass,
		CharSelect,
		CharCreate_SelectClass,
		CharCreate_ChangeParts,
		CharDelete,
		StartGame,
		ServerList,
		ChannelList,
	};
#ifdef PRE_ADD_RELOGIN_PROCESS
	std::wstring m_strConnectID;
	std::wstring m_strConnectPassword;
#endif // PRE_ADD_RELOGIN_PROCESS
	
protected:
	enum UICallbackEnum {
		ConnectLoginFailed = 110000,
		CheckVersionFailed,
		CheckLoginFailed,
		CharListFailed,
		CharCreateFailed,
		CharDeleteFailed,
		GameInfoFailed,
		VillageInfoFailed,
		DisconnectTCP,
		SkipTutorial,
		CharReviveFailed,
		ComeBack,
	};

	enum {
		HAIR_COLOR_COUNT = 4,
		SKIN_COLOR_COUNT = 4,
		EYE_COLOR_COUNT = 4,
	};

	struct DefaultPartsStruct {
		std::vector<int> nVecList[EQUIPMAX];
		int nSelectPartsIndex[EQUIPMAX];
		DWORD dwColor[MAPartsBody::PartsColorAmount];
		int nWeapon[2];
		std::vector<DnPartsHandle> hVecParts[EQUIPMAX];
		DnWeaponHandle hWeapon[2];
	};

	DnCameraHandle m_hCamera;

	std::vector<TCharListData> m_vecCharListData;	// Full character list data
	int m_nTotalCharSelectPage;						// Number of character selection pages
	int m_nCurrentCharSelectPage;					// Current character selection page
	int m_nTotalCharSlotCount;						// Total character slot count
	int m_nEmptyCharSlotCount;						// Number of empty character slots
	int m_nDeleteCharPage;							// Save current page when character is deleted

	DnActorHandle m_hSlotActorHandle[SHOWCHARNUMPERONEPAGE];
	int m_nSlotActorMapID[SHOWCHARNUMPERONEPAGE];
	EtVector3 m_vSlotActorPos[SHOWCHARNUMPERONEPAGE];
	EtVector3 m_vSlotActorLook[SHOWCHARNUMPERONEPAGE];
	bool m_bSlotDeleteWait[SHOWCHARNUMPERONEPAGE];
	bool m_bSlotOpen[SHOWCHARNUMPERONEPAGE];	
	__time64_t m_SlotDelteWaitRemainTime[SHOWCHARNUMPERONEPAGE];
	
	std::wstring m_wstrCreateCharName;				// Name to save when receiving character creation complete packet (used to navigate to created character page)

	int m_nCharSlotCount;
	int m_nSelectIndex;
	int m_nSelectedServerIndex;
	int m_nSelectedChannelIndex;

	std::vector< DnActorHandle > m_hVecCreateActorList;
	std::vector< DefaultPartsStruct > m_VecCreateDefaultPartsList;	
	std::vector<EtOutlineHandle> m_vecOutlineSlot;
	std::vector<EtOutlineHandle> m_vecOutlineCreate;

	MatrixEx m_TreeOffset;

	int m_nSelectClassIndex;

//	EtParticleHandle m_hParticle;
//	int m_nParticleDataIndex;
//	MatrixEx m_ParticleCross;

	float m_fCameraDelta;
	float m_fCameraYaw;
//	bool m_bRecvCharList;
	bool m_bProcessCamera;
	bool m_bShowHideActor;
	bool m_bCompleteCharList;

	LoginStateEnum		m_State;
	LoginStateEnum		m_PrevState;
	CDnWorld			*m_pWorld;	

	SCCharList			m_CharList;
	
	float				m_fOldDOFNearBlurSize;
	float				m_fOldDOFFarBlurSize;	
	bool				m_bRequestWait;
#ifdef PRE_ADD_RELOGIN_PROCESS
	int					m_nReLoginCount;
#endif // PRE_ADD_RELOGIN_PROCESS
	
protected:
	void CalcCharSlotPosition();
	bool SelectActorSlot( int nIndex );
	void ShowSlotInfoDlg();
	void ShowSlotInfoDlg( int nSlotIndex );
	void ProcessDeleteWait( LOCAL_TIME LocalTime, float fDelta );

	void ShowCreateCharacter( bool bShow );
	int GetCreateSelectClass();

	void SetRequestWait( bool bWait );
	void SetCharSelectPage( int nSelectPage );	// Set character selection page

public:
	void PreInitialize();
	// Task 
	bool Initialize( LoginStateEnum State );

	void ReduceFarDOFBlurSize();	// Hardcode reducing DOF Far BlurSize specifically for the character selection screen
	void RestoreFarDOFBlurSize();			// (Prevents excessive blur on the character's head area) 
	
	virtual void Process( LOCAL_TIME LocalTime, float fDelta );

	// CInputReceiver
	virtual void OnInputReceive( int nReceiverState, LOCAL_TIME LocalTime = -1 );

	// Client Session
	virtual void OnConnectTcp();
	virtual void OnDisconnectTcp( bool bValidDisconnect );
	// Tcp
	virtual void OnRecvLoginCheckVersion( SCCheckVersion *pPacket );
	virtual void OnRecvLoginCheckLogin( SCCheckLogin *pPacket );
	virtual void OnRecvLoginCharList( SCCharList *pPacket );
	void _OnRecvLoginCharList( SCCharList *pPacket );
	virtual void OnRecvLoginWaitUser( SCWaitUser *pPacket );
	virtual void OnRecvLoginBlock( SCCHeckBlock *pPacket );
	virtual void OnRecvLoginCharCreate( SCCreateChar *pPacket );
	virtual void OnRecvLoginCharDelete( SCDeleteChar *pPacket );
	virtual void OnRecvLoginServerList( SCServerList *pPacket );
	virtual void OnRecvLoginChannelList( SCChannelList *pPacket );
	virtual void OnRecvLoginBackButton();
	virtual void OnRecvLoginReviveChar( SCReviveChar *pPacket );

	virtual void OnRecvSystemGameInfo( SCGameInfo *pPacket );
	virtual void OnRecvSystemVillageInfo( SCVillageInfo *pPacket );

	// CEtUICallback
	virtual void OnUICallbackProc( int nID, UINT nCommand, CEtUIControl *pControl, UINT uiMsg = 0 );

	// Local
	bool IsEmptySelectActorSlot();
	int GetSelectActorSlot() { return m_nSelectIndex; }
	void SetSelectActorSlot( int nValue ) { m_nSelectIndex = nValue; }
	void ChangeState( LoginStateEnum State, bool bAutoSendMessage = true );
	
	void CreateCharacter( TCHAR *szName );
	void DeleteCharacter();
	void SetAuthPassword( int nSeed, int nValue[] ) {
		m_nAuthPasswordSeed = nSeed;
		memcpy( m_nAuthPasswordValue, nValue, sizeof(m_nAuthPasswordValue) );
	}
	int m_nAuthPasswordSeed;
	int m_nAuthPasswordValue[4];
	void ReviveCharacter( int nSlotIndex );
	void GetCharSelectMousePos( std::vector<EtVector2> &vecPos, float fScreenWidthRatio, float fScreenHeightRatio );
	
	void SelectServer( int nServerIndex );
	void SelectChannel( int nChannelIndex );
	int  GetServerIndex(){return m_nSelectedServerIndex;}
	void ChangeCreateClass( int nClassID );
	void SetCharColor( MAPartsBody::PartsColorEnum colorType, int nColorIndex, int nClassIndex = -1);
	void RotateCreateParts( CDnParts::PartsTypeEnum PartsIndex, bool bAdd );
	void TurnCharacter();

	bool IsCompleteCharList() { return m_bCompleteCharList; }
	bool IsRequestWait() { return m_bRequestWait; }

	int GetCurrentSelectLevel();
	LoginStateEnum GetState() { return m_State; }

	CEtOutlineFilter *GetOutlineFilter();

	virtual bool OnCloseTask();

#ifdef PRE_MOD_CREATE_CHAR
	void SetCharCreateBasePartsName();	// Set initial default part names in UI on character creation
#endif // PRE_MOD_CREATE_CHAR
	void NextCharListPage();			// Move to the next character page in the character selection screen
	void PrevCharListPage();			// Move to the previous character page in the character selection screen
	int GetSelectExtendActorSlot();

#ifdef PRE_ADD_DOORS
	void OnRecvDoorsAuthKey(SCDoorsGetAuthKey * pPacket);
	void OnRecvDoorsCancelAuth(SCDoorsCancelAuth * pPacket);
	void OnRecvDoorsAuthFlag(SCDoorsGetAuthFlag* pPacket);
#endif		//#ifdef PRE_ADD_DOORS
};


#endif // PRE_MOD_SELECT_CHAR