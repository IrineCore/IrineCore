#pragma once
#include "DnCustomDlg.h"
#include "DnParts.h"

#ifdef PRE_ADD_GACHA_JAPAN

//#ifdef PRE_ADD_KALI
//const int GACHA_NUM_JOB_BTN = 6;
//#else
//const int GACHA_NUM_JOB_BTN = 5;
//#endif

const int GACHA_NUM_JOB_BTN = 6;

class CDnGachaPreview;
class CDnGachaRouletteDlg;
class CDnGachaBuyConfirmDlg;
class CDnGachaResultDlg;
class CDnItem;

// Gachapon exclusive to Japan version 
class CDnGachaDlg : public CDnCustomDlg,
					public CEtUICallback
{
private:
	enum
	{
		PREVIEW_DLG,
		GACHA_BUY_CONFIRM_DLG,
		GACHA_RESULT_DLG,
	};

	// Gachapon state.
	enum
	{
		WAITING,
		ROULETTING,
		STOPPING,
		END,
	};

	int m_iGachaIndex;

	CDnGachaPreview* m_pGachaPreviewDlg;
	CDnGachaBuyConfirmDlg* m_pGachaBuyConfirm;
	CDnGachaResultDlg* m_pGachaResultDlg;
	CEtUIComboBox* m_pComboPartsSelect;
	CEtUIRadioButton* m_apRadioBtnJob[ GACHA_NUM_JOB_BTN ];
	CEtUIButton* m_pBtnOpenCashShop;
	CEtUIStatic* m_pStaticCoin;
	CEtUIButton* m_pBtnBuyGacha;
	CEtUIButton* m_pBtnClose;

	CEtUIStatic* m_pStaticItemNameArea;
	CEtUIStatic* m_pStaticItemStatArea;

	// Like SkillTreeContentDlg, hidden by default and rendered directly when this dialog renders.
	CDnGachaRouletteDlg* m_pNameRouletteDlg;
	CDnGachaRouletteDlg* m_pStatRouletteDlg;
	int m_iRouletteCount;
	deque<CDnItem*> m_dqItemBuffer;

	int m_iSelectedJobBtnID;
	bool m_bInitialized;
	bool m_bProcessRoulette;

	struct S_PART_GACHA_DROPITEMS_INFO
	{
		vector<int> avlItemsByPart[ CDnParts::CashPartsTypeEnum_Amount ];
	};

	// Collect all item IDs that can drop from the currently opened Gachapon.
	map<int, S_PART_GACHA_DROPITEMS_INFO> m_mapItemsByJob;
	//int m_aiDropTableIDs[ CDnParts::CashPartsTypeEnum_Amount ];
	int m_iSelectedPart;

	// Item received from server as result
	CDnItem* m_pResultItem;
	float m_fStopAccel;
	int m_iLeftStopSelCount;
	float m_fRouletteSpeed;
	float m_fLeftRouletteDistance;
	//bool m_bAllowStop;
	float m_fStopTimeStamp;
	float m_fRouletteStopTimeGap;
	int m_iResultItemStaticIndex;			// Roulette index where the result item will be set.

	float m_fRouletteElapsedTime;

	int m_iPrevRouletteCount;

	// The sequence of steps up to item delivery is complete.
	//bool m_bGachaCompleted;

	// Current gachapon state
	int m_eGachaStatus;

	// Collect item IDs currently spinning in the roulette and spin randomly.
	vector<int> m_vlMixedSelectedPartsItems;

	// Sound related.
	int m_iRouletteChangeSound;
	int m_iRouletteResultSound;

protected:
	void _GatherGachaItems( void );
	void _InitializeComboBox( void );
	void _UpdateJobSelection( void );
	void _OnSelectPart( void );				// Part selected. Start spinning gacha.
	void _UpdateRouletteItemNameInfo( void );
	void _UpdateRouletteItemStatInfo( void );
	void _RenderRouletteDialog( float fDelta, CEtUIControl* pAreaControl, CDnGachaRouletteDlg* pRouletteDlg );
	void _MakeItemStatString( CDnState* pItemState, /*IN OUT*/ wstring& strItemStat );

public:
	CDnGachaDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnGachaDlg(void);

	// Gachapon index registered to the NPC received from server
	void SetGachaIndex( int iGachaIndex ) { m_iGachaIndex = iGachaIndex; };

	void Initialize( bool bShow );
	void InitialUpdate( void );
	void Reset( void );
	void Show( bool bShow );
	void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg = 0 );
	void OnUICallbackProc( int nID, UINT nCommand, CEtUIControl *pControl, UINT uiMsg = 0 );
	void Process( float fDelta );
	void Render( float fDelta );

	void UpdateGachaCoin( void );

	// Called from CDnGachaRouletteDlg.
	
	// Keep calling while roulette is spinning.
	void ProcessRoulette( bool bProcessRoulette ) { m_bProcessRoulette = bProcessRoulette; };

	// Called from Gacha Preview to rename.
	void OnChangeItem( int iItemID );
	
	//////////////////////////////////////////////////////////////////////////

	// Dialog acting as roulette is set.
	void SetRouletteDlg( CDnGachaRouletteDlg* pNameRouletteDlg, CDnGachaRouletteDlg* pStatRouletteDlg )
	{ 
		m_pNameRouletteDlg = pNameRouletteDlg; 
		m_pStatRouletteDlg = pStatRouletteDlg;
	};

	void OnChangeResolution();

	// Gachapon item result received from server
	void OnRecvGachaResult( const SCRefreshCashInven* pPacket );
};
#endif //PRE_ADD_GACHA_JAPAN