#pragma once

#ifdef PRE_ADD_CHAOSCUBE

#include "DnCustomDlg.h"

class CDnItem;
class CDnStoreConfirmExDlg;
class CDnChaosGaugeCubeStuffConfirmDlg;
class CDnChaosGaugeCubeResultDlg;
class CDnChaosCubeProgressDlg;
class CDnChaosGaugeCubeCompleteDlg;

class CDnChaosGaugeCubeDlg : public CDnCustomDlg, public CEtUICallback
{
public:
	enum {
		DIALOG_SPLIT_ITEM = 0,
		DIALOG_STUFF_CONFIRM = 1,
		DIALOG_RESULT_PREVIEW = 2,
		DIALOG_PROGRESS = 3,
		DIALOG_COMPLETE = 4,
	};

	struct SStuffInfo {
		int nItemID;
		int nGaugeValue;
		int nRowID;
		SStuffInfo() : nItemID( 0 ), nGaugeValue( 1 ), nRowID( -1 ) {}
	};

public:
	CDnChaosGaugeCubeDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnChaosGaugeCubeDlg();

protected:
	CEtUIStatic *m_pStaticTitle;
	CEtUIStatic *m_pStaticDesc;
	CEtUIProgressBar *m_pProgressGauge;
	CEtUIListBoxEx *m_pListBoxStuff;
	CEtUIButton *m_pBtnOK;
	CEtUIButton *m_pBtnCancel;
	CEtUIButton *m_pBtnPreview;
	CEtUIStatic *m_pStaticEffect;
	CEtUIStatic *m_pStaticGaugeCount;

	CDnStoreConfirmExDlg *m_pSplitDlg;
	CDnChaosGaugeCubeStuffConfirmDlg *m_pStuffConfirmDlg;
	CDnChaosGaugeCubeResultDlg *m_pResultPreviewDlg;
	CDnChaosCubeProgressDlg *m_pProgressDlg;
	CDnChaosGaugeCubeCompleteDlg *m_pCompleteDlg;

	INT64 m_biCubeSerial;
	char m_cCubeInvenType;
	short m_sCubeSlotIndex;
	int m_nChaosNum;
	int m_nMaxGauge;
	int m_nCurrentGauge;
	int m_nTargetGauge;
	float m_fGaugeAnimProgress;

	std::map<int, SStuffInfo> m_mapStuffTable;

	void ResetCubeState();
	void RefreshStuffList();
	void RefreshGaugeUI();
	CDnItem *FindInventoryItem( int nItemID ) const;
	int GetOwnedItemCount( int nItemID ) const;

public:
	virtual void Initialize( bool bShow );
	virtual void InitialUpdate();
	virtual void Show( bool bShow );
	virtual void Process( float fElapsedTime );
	virtual void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg = 0 );
	virtual void OnUICallbackProc( int nID, UINT nCommand, CEtUIControl *pControl, UINT uiMsg = 0 );

	void SetChaosGaugeCubeItem( CDnItem *pItem );
	void OnAddStuffItem( CDnItem *pStuffItem );
	void OnRecvAddStuff( int nChaosGaugeType, int nCurrentGauge );
	void OnRecvComplete( int nChaosGaugeType, int nResultItemID, int nItemCount );
	void UpdateStuffList();
	int GetChaosNum() const { return m_nChaosNum; }
};

#endif // PRE_ADD_CHAOSCUBE
