#pragma once
#include "DnCustomDlg.h"
#include "DnNPCActor.h"
#include "DnWorld.h"
#include "DnSmartMoveCursor.h"

class CDnZoneMapDlg : public CDnCustomDlg
{
public:
	enum 
	{
		CURRENT_ZONE = -1,
		PREV_ZONE = -2,
	};

	struct SZoneNPCButtonInfo
	{
		CDnZoneNPCButton *pButton;
		int nNpcID;
	};

protected:
	enum emZoneGateIndex
	{
		ZONE_GATE_VIALLAGE,
		ZONE_GATE_DUNGEON,
		ZONE_GATE_WORLD,
		ZONE_GATE_CLOSE,
		ZONE_GATE_UNKNOWN,
		ZONE_GATE_AMOUNT,
	};

	struct SZonePartyButtonInfo
	{
		CEtUIButton *pButton;
		int nSessionID;
	};

	struct SDungeonInfo
	{
		CEtUIStatic *pStatic;
		CEtUIStatic *pStaticRecommend;
#ifdef PRE_ADD_QUESTCOUNT_IN_STAGELIST
		CEtUIStatic *pStaticBackground;
		CEtUIStatic	*pStaticQuestMark;
		CEtUIStatic *pStaticQuestCount;
#endif
		SUICoord Coord;
#ifdef PRE_ADD_QUESTCOUNT_IN_STAGELIST
		SUICoord    QuestMarkCoord;
		SUICoord	QuestCountCoord;
#endif
	};

public:
	CDnZoneMapDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL  );
	virtual ~CDnZoneMapDlg(void);

protected:
	CEtUIStatic *m_pStaticMapName;
	CEtUIStatic *m_pStaticPlayer;
	CEtUIStatic *m_pStaticPlayerUp;

	float m_fElapsedTime;
	int m_nSetMapIndex;
	int m_nCurMapIndex;
	bool m_bCurWorld;

	CDnZoneGateButton	*m_pShowGateButton;

	CEtUITextureControl *m_pTextureMinimap;
	EtTextureHandle m_hMinimap;

	CEtUIButton *m_pButtonToWorld;
	CEtUIButton *m_pButtonCancelCheck;

	CEtUIButton *m_pButtonParty[CLASSKINDMAX];
	CEtUIButton *m_pButtonGuild;
	//blondy
	CEtUIButton *m_pButtonEnemy;		// Enemy button
	CEtUIButton *m_pButtonGhost;		// Ghost button
	//blondy end
	CEtUIButton *m_pButtonRoot;
	CEtUIButton *m_pButtonLeaf;
	CEtUIButton *m_pButtonFruit;
	CEtUIButton *m_pButtonCereals;
	CEtUIButton *m_pButtonFungus;
	CEtUIButton *m_pButtonEmptyArea;
	std::vector<CDnFarmAreaButton *> m_vecButtonFarm;

	CEtUIButton *m_pButtonQuestTrace;

	std::list<SZonePartyButtonInfo> m_listButtonParty;

	CDnZoneNPCButton *m_pButtonZoneNPC[ CDnNPCActor::typeAmount ];

	std::vector<SZoneNPCButtonInfo> m_vecButtonNPC;

	CDnZoneGateButton *m_pButtonGate[ZONE_GATE_AMOUNT];
	std::vector<CDnZoneGateButton*> m_vecButtonGate[ZONE_GATE_AMOUNT];

	CEtUIStatic *m_pStaticDungeonTitle;
	
	std::vector< SDungeonInfo > m_vecDungeonName;

	CDnSmartMoveCursorEx m_SmartMoveEx;

	int m_nTraceQuest;
	SUICoord m_sUICoord;

protected:
	virtual void ProcessParty( float fElapsedTime );
	
	
	void ProcessPlayer( float fElapsedTime );	
	void ProcessNPC( float fElapsedTime );	
	void DeletePartyControl();

	void CreateNpcControl( int nMapIndex );
	void CreateNpcControl();
	void DeleteNpcControl();
	
	void CreateGateControl( int nMapIndex );
	void CreateGateControl();
	void DeleteGateControl();

	void CreateFarmControl();
	void DeleteFarmControl();

	void InitializeNpcControl();

	void CreateQuestTrace();
	void ProcessQuestTrace();

	bool IsCreateNPC( int nID );
	void HideStageList();

protected:
	void ResetButtonPin();
	void ResetGateButtonPin();
	void ResetNPCButtonPin();
	void ResetEmptyAreaButtonPin();

	virtual void CreatePartyControl();
public:
	void SetMapIndex( int nMapIndex );
	void RefreshNPCActor();
	void RefreshPartyActor();
	void RefreshFarmControl();
	bool IsChanged( int nMapIndex );
	void GetQuestTrace( int nQuestID )	{ m_nTraceQuest = nQuestID; }

#ifdef PRE_ADD_QUESTCOUNT_IN_STAGELIST
	int	GetEnbleQuestCountWithMapIndex( int nStageIndex );
#endif 

public:
	virtual void Initialize(bool bShow);
	virtual void InitialUpdate();
	virtual void InitCustomControl( CEtUIControl *pControl );
	virtual void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg = 0 );
	virtual void Show( bool bShow );
	virtual void Process( float fElapsedTime );
	virtual bool MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
};