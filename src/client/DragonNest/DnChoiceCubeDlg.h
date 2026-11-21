#pragma once


#ifdef PRE_ADD_CHOICECUBE

#include "DnCustomDlg.h"

//-----------------------------------------------
// Chaos Cube main window.

class CDnChoiceCubeDlg : public CDnCustomDlg, public CEtUICallback
{

private:

	// Controls ------------

	//--- Material slots ---//
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
	

	//--- Result slots ---//
	CDnItemSlotButton * m_pSlotBtnSelectedResult; // Selected result slot.
	CEtUIStatic * m_pStaticLightResult; // Sparkle.
	CEtUIStatic * m_pStaticPageResult;  // Page/MaxPage.

	std::vector< CDnItemSlotButton * > m_vSlotBtnMaterialsResult; // Slots.
	std::vector< CEtUIStatic * > m_vStaticSelectResult; // Slot selection Static.
	
	CEtUIStatic * m_pWheelRegionStuff; // Material slot wheel area.
	CEtUIStatic * m_pWheelRegionResult; // Result slot wheel area.

	std::vector< CEtUIStatic * > m_vPeriodItems; // Static control for displaying time-limited duration in result slot.
	

	// -----------------------


	class CDnItem * m_pItemChaos; // Choice item for opening UI.
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




	// Result --------------------------------------------//	
	struct SResultItem
	{
		CDnItem * pItem;
		int nPeriod; // Whether time-limited.
		INT64 nGold; // Gold.
		SResultItem() : pItem(NULL), nPeriod(0), nGold(0) {}
	};
	std::vector< SResultItem > m_vResultItems;     // Result items.
	int m_crrPageResult;			  // Current page.
	int m_maxPageResult;			  // Total pages.
		
	// Selected result slot index.
	int m_SelectedSlotIndexResult;
	float m_fAlphaColorResult; // Alpha value.
	short m_signLightResult;   // Sign for selected material slot glow effect.

public:



public:

	CDnChoiceCubeDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL, bool bAutoCursor = false );
	virtual ~CDnChoiceCubeDlg(){
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

	// Choice item.
	void SetChaoseItem( class CDnItem * pItem );
	class CDnItem * GetChoiceItem(){
		return m_pItemChaos; // Choice item for opening UI.
	}

	void UpdateItemList(); // Refresh item list.

	void MixComplete(); // Mix complete.

	void RecvResultItemComplet(); // Result item received.

	void RecvMixStart( bool bEnableBtn ); // Mix start response.

	int GetResultItemID();

	int GetPeriodItemByID( int nItemID ); // Whether item has an expiration period (return value: duration).

private:

	// Search for and register materials when Dlg opens.
	void OpenDlg();

	// Clean up when Dlg closes.
	void CloseDlg();

	// Register material in material slot.
	void RefreshStuffItemSlot();

	// Select material item.
	void SelectionStuffItem( const char * strCtlName );

	// Register result into result slot.
	void RefreshResultItemSlot();

	// Select result item.
	void SelectionResultItem( const char * strCtlName );

	// Mix.
	void MixStart();

	// Compare if material item and result item are the same.
	bool CompareStuffnResult(); 
};


#endif