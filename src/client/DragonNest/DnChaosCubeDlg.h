#pragma once


#ifdef PRE_ADD_CHAOSCUBE

#include "DnCustomDlg.h"

//-----------------------------------------------
// Chaos Cube main window.

class CDnChaosCubeDlg : public CDnCustomDlg, public CEtUICallback
{

private:

	// Controls ------------
	CDnItemSlotButton * m_pSlotBtnSelected; // Selected material slot.
	std::vector< CDnItemSlotButton * > m_vSlotBtnMaterials; // Material slots.
	std::vector< CEtUIStatic * > m_vStaticSelect; // Material slot selection Static.

	CEtUIStatic * m_pStaticLight; // Sparkle.
	CEtUIStatic * m_pStaticTitle; // Title bar.
	CEtUIStatic * m_pStaticHelp;  // Help.
	CEtUIStatic * m_pStaticPage;  // Page/MaxPage.

	CEtUIButton * m_pBtnPrev;     // Previous.
	CEtUIButton * m_pBtnNext;     // Next.
	CEtUIButton * m_pBtnMaterial; // Material.
	CEtUIButton * m_pBtnProduct;  // Result.
	CEtUIButton * m_pBtnConfirm;  // Confirm.
	// -----------------------


	//------------------------
	// Child Dlg
	//class CDnChaosCubeStuffDlg * m_pChildDlg; // Material Dlg or Result Dlg.
	//class CDnChaosCubeProgressDlg * m_pChaosCubeProgressDlg; // Progress Dlg.
	//class CDnChaosCubeResultDlg * m_pChaosCubeResultDlg; // Acquired result Dlg.
	//------------------------


	class CDnItem * m_pItemChaos; // Chaos item for opening UI.
	INT64 m_nChaosSerial;
	
	std::map< int, std::vector< CDnItem * > > m_mapStuffItems; // <ItemID, inventory slot items> Material items found in inventory.
															   // When items with the same ID are split across multiple slots, consumed counts can differ per slot, so store them separately.

	std::vector< int > m_vStuffItemIDs;     // Material items.
	std::map< int, int > m_vNeedStuffCount; // <ItemID, count> Material count needed for mix.

	// Handle Page.
	const int m_CountPerPage; // Number of slots per page.
	int m_crrPage;			  // Current page.
	int m_maxPage;			  // Total pages.

	// Highlight selected material.
	float m_fAlphaColor; // Alpha value.
	short m_signLight;   // Sign for selected material slot glow effect.

	bool m_bSelfItemList; // Whether the same item as the material item is loaded.

	// Selected material slot index.
	int m_SelectedSlotIndex;

public:

	enum EChildDlgMode
	{
		NONE = 0,
		STUFF = 1,   // Material window. - Chaos Cube
		PRODUCT = 2, // Result window - Chaos Cube

		STUFF_CHOICE = 3, // Material window - Choice Cube
		PRODUCT_CHOICE = 4, // Result window - Choice Cube
	};

public:

	CDnChaosCubeDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL, bool bAutoCursor = false );
	virtual ~CDnChaosCubeDlg(){
		ReleaseDlg();
	}
	
	void ReleaseDlg();

	// Override - CEtUIDialog //
	virtual void Initialize( bool bShow );
	virtual void InitialUpdate();
	virtual void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl * pControl, UINT uMsg = 0 );
	virtual void Process( float fElapsedTime );
	virtual bool MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
	virtual void Show( bool bShow );

	virtual void OnUICallbackProc( int nID, UINT nCommand, CEtUIControl *pControl, UINT uiMsg );

public:

	// Chaos item.
	void SetChaoseItem( class CDnItem * pItem );
	class CDnItem * GetChaosItem(){
		return m_pItemChaos; // Chaos item for opening UI.
	}

	// Open ProgressDlg.
	//void OpenProgressDlg( bool bShow, struct SCChaosCubeRequest * pData );

	// Open ResultDlg;
	//void OpenResultDlg( bool bShow, int nItemID, int nCount );

	void UpdateItemList(); // Refresh item list.

	void MixComplete(); // Mix complete.

	void RecvResultItemComplet(); // Result item received.

	void RecvMixStart( bool bEnableBtn ); // Mix start response.

private:

	// Search for and register materials when Dlg opens.
	void OpenDlg();

	// Clean up when Dlg closes.
	void CloseDlg();

	// Register material in material slot.
	void RefreshStuffItemSlot();

	// Select material item.
	void SelectionStuffItem( const char * strCtlName );

	// Mix.
	void MixStart();
	
};


#endif