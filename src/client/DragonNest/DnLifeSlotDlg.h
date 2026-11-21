#pragma once

#include "DnCustomDlg.h"


class CDnItem;

class CDnLifeSlotDlg : public CDnCustomDlg
{
public:
	CDnLifeSlotDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnLifeSlotDlg();

private :
	enum{
		MAX_SEED_SLOT = 16,
	};

	CEtUIButton * m_pSeedButton;
	CEtUIButton * m_pPrevButton;
	CEtUIButton * m_pNextButton;
	CEtUIStatic * m_pPageStatic;
	CEtUIStatic * m_pEnableStatic;
	CEtUIStatic * m_pDisableStatic;
	CEtUIStatic * m_pNumberStatic;
	CEtUICheckBox * m_pCheckCash;

	std::vector<CDnItemSlotButton *> m_vSeedSlot;
	CDnItemSlotButton * m_pCashItemSlot;

	int m_nPage;
	int m_nSelectSlotIndex;

	CDnItem * m_pSelectItem;
	CDnItem * m_pCashItem;
	std::vector<CDnItem *>	m_vInvenSeed;
	std::vector<int>		m_vInvenIndex;

	int BoostCount();
	bool IsPage( int nPage );
	bool IsUseSeedItem( int nItemID );
	void SetGrowBoost( bool bIs, int nGetCount, int nNeedCount );

public :

	void RefreshPage();

public:
	virtual void Initialize( bool bShow );
	virtual void InitialUpdate();
	virtual void InitCustomControl( CEtUIControl *pControl );
	virtual void Show( bool bShow );
	virtual void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg = 0 );
	//virtual void Process( float fElapsedTime );
	//virtual void OnUICallbackProc( int nID, UINT nCommand, CEtUIControl *pControl, UINT uiMsg = 0 );
	//virtual bool MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
};
