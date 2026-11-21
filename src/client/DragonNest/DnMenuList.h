#pragma once

#include "EtUIControl.h"

class CDnMenuListItem;

// Usage is similar to Tree.
// Usage example is in DnCashShopMenuListDlg (Cash Shop category).
class CDnMenuList : public CEtUIControl
{

private:

	//CEtUIScrollBar m_ScrollBar;

	std::vector< CDnMenuListItem * > m_vMainMenu;
	CDnMenuListItem * m_pSelectedMainItem;
	CDnMenuListItem * m_pSelectedSubItem;
	CDnMenuListItem * m_pSelectedItem; // - If only MainItem is selected, MainItem,
									   //   If only SubItem is selected, SubItem,
									   //   If both are selected, SubItem.


	SUICoord m_ViewArea; // Visible area of MenuItem (Clipping area).

	CDnMenuListItem * m_pOverItem; // MouserOver.

	// Menu list scroll buttons.
	struct ScorllButton
	{
		SUICoord m_BtnCoord;
		SUICoord m_BtnOverCoord;

		bool bOver;
		bool bSelect;
		ScorllButton() : bOver(false), bSelect(false){}
	};
	ScorllButton m_ScrollBtn[2];
	EtVector2 m_BtnPressOffset;
	

	int m_topMainItemIndex; // Topmost visible MainItem Index;

	bool m_bAutoSelectSubItem; // Whether the first SubItem is auto-selected when MainItem is selected.
	
	bool m_bShowScrollBtn; // Whether to display scroll buttons.

public:

	CDnMenuList( CEtUIDialog * pParent );
	virtual ~CDnMenuList();


	virtual void Initialize( SUIControlProperty *pProperty );
	virtual bool HandleMouse( UINT uMsg, float fX, float fY, WPARAM wParam, LPARAM lParam );
	virtual void Render( float fElapsedTime );
	virtual void Show( bool bShow );
	virtual void OnChangeResolution();
	virtual bool CanHaveFocus() { return true; }

private:

	void RenderBase( CDnMenuListItem *pItem, int eleIdx );
	void RenderSelect( CDnMenuListItem *pItem, int eleIdx );
	void RenderOver( CDnMenuListItem *pItem, int eleIdx );
	void RenderText( CDnMenuListItem *pItem, int eleIdx );
	void RenderButton();

	
	// Set positions of Items.
	void UpdatePosition();

	// Mouse handling.
	bool SelectItem( float fX, float fY );
	bool OverItem( float fX, float fY );

	bool SelectButton( float fX, float fY );
	bool OverButton( float fX, float fY );
	void PressItem( float fX, float fY );
	void Scroll( int nScroll );

public:

	CDnMenuListItem * AddItem( LPCWSTR szText, DWORD dwColor=textcolor::WHITE  ); // Add MainItem.

	CDnMenuListItem * AddChildItem( CDnMenuListItem * pMenuItem, LPCWSTR szText, DWORD dwColor=textcolor::WHITE ); // Add SubItem to MainItem.

	// Remove.
	bool DeleteItem( CDnMenuListItem * pMenuItem );
	void DeleteAllItems();

	int GetMainTabSize(){
		return (int)m_vMainMenu.size();
	}
	CDnMenuListItem * GetMainTab( int idx ){
		if( idx < (int)m_vMainMenu.size() )
			return m_vMainMenu[ idx ];
		return NULL;
	}	

	// Auto-select the first SubTab when MainTab is selected.
	void AutoSelectToSubItem( bool bAuto ){
		m_bAutoSelectSubItem = bAuto;
	}
	bool IsAutoSelectToSubItem(){
		return m_bAutoSelectSubItem;
	}

	// Selected Item.
	CDnMenuListItem * GetSelectedMainItem(){
		return m_pSelectedMainItem;
	}
	CDnMenuListItem * GetSelectedSubItem(){
		return m_pSelectedSubItem;
	}
	CDnMenuListItem * GetSelectedItem(){
		return m_pSelectedItem; 
	}

	// Make the item selected.
	void SetSelectItem( CDnMenuListItem * pSelectedItem );
	
	void SelectByIndex( int idx ); // Select the corresponding index among the main items.

	void Reset(); // Deselect.
	void ResetnUpdatePos(); // Deselect & adjust position.
	void ResetItem( CDnMenuListItem * pItem );

};