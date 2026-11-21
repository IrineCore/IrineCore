#pragma once


#ifdef PRE_ADD_CHAOSCUBE

#include "DnCustomDlg.h"

//-----------------------------------------------
// Chaos Cube main window.

class CDnChaosCubeStuffDlg : public CDnCustomDlg
{

private:

	CEtUIStatic * m_pStaticTitle; // Title bar.
	CEtUIStatic * m_pStaticHelp;  // Help.
	CEtUIStatic * m_pStaticPage;  // Page/MaxPage.

	CEtUIButton * m_pBtnPrev;     // Previous.
	CEtUIButton * m_pBtnNext;     // Next.
	
	int m_DlgMode; // CDnChaosCubeDlg::EChildDlgMode. Material window or result window.
	int m_nChaosNum; // 

	std::vector< CDnItem * > m_vItems; // Item list.
	int m_nStuffItemID; // Material item ID.

	std::map< int, INT64 > m_mapGold; // Result gold amount.

	// Handle Page.
	const int m_CountPerPage; // Number of slots per page.
	int m_crrPage;			  // Current page.
	int m_maxPage;			  // Total pages.
	

	std::vector< CDnItemSlotButton * > m_vSlotBtns; // Item slots.
	std::vector< CEtUIStatic * > m_vSlotStatics;    //Item slot Static.

public:

	CDnChaosCubeStuffDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL, bool bAutoCursor = false );
	virtual ~CDnChaosCubeStuffDlg(){
		ReleaseDlg();
	}

	void ReleaseDlg();

	// Override - CEtUIDialog //
	virtual void Initialize( bool bShow );
	virtual void InitialUpdate();
	virtual void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl * pControl, UINT uMsg = 0 );
	virtual bool MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
	virtual void Show( bool bShow );

public:

	// Material window or Result window.
	//void SetDlgMode( int mode, int nItemID, int nChaosNum, bool bSelf );
	void SetDlgMode( int mode, int nChaosNum, bool bSelf );

private:

	// Load item list.
	//void LoadItems( int nChaosNum, int nItemID, bool bSelf );	
	void LoadItems( int nChaosNum, bool bSelf );	

	// Handle item list paging.
	void RefreshList();

};

#endif