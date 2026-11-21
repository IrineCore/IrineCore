#pragma once

#ifdef PRE_ADD_CHAOSCUBE

#include "DnCustomDlg.h"

class CDnItem;

class CDnChaosGaugeCubeResultDlg : public CDnCustomDlg
{
public:
	enum {
		ITEM_PER_PAGE = 5,
	};

	struct SResultItemEntry {
		int nRowID;
		CDnItem *pItem;
		int nPeriod;
		SResultItemEntry() : nRowID( -1 ), pItem( NULL ), nPeriod( 0 ) {}
	};

public:
	CDnChaosGaugeCubeResultDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnChaosGaugeCubeResultDlg();

protected:
	CEtUIStatic *m_pStaticPage;
	CEtUIButton *m_pBtnPrior;
	CEtUIButton *m_pBtnNext;

	CDnItemSlotButton *m_pSlotButtons[ITEM_PER_PAGE];
	CEtUIStatic *m_pStaticNames[ITEM_PER_PAGE];

	std::vector<SResultItemEntry> m_vecItems;
	std::map<int, INT64> m_mapGold;

	int m_nMaxPage;
	int m_nCurPage;

	void ClearSlots();
	void ClearItems();
	void RefreshPage();

public:
	virtual void Initialize( bool bShow );
	virtual void InitialUpdate();
	virtual void InitCustomControl( CEtUIControl *pControl );
	virtual void Show( bool bShow );
	virtual bool MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
	virtual void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg = 0 );

	void LoadResultItems( int nChaosNum );
};

#endif // PRE_ADD_CHAOSCUBE
