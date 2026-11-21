#pragma once
#include "EtUIListBox.h"
//#include "EtUIQuestTreeItem.h"

// An extended listbox whose items are regular UI elements.
class CEtUIListBoxEx : public CEtUIListBox
{
public:
	CEtUIListBoxEx( CEtUIDialog *pParent );
	virtual ~CEtUIListBoxEx(void);

protected:
	void CheckListBoxItem();

	SUICoord m_SplitSelectCoord[9];
	SUICoord m_SplitSelectUVCoord[9];

	bool m_bRenderScrollBar;
	bool m_bRenderSelectBar;
	bool m_bReturnHandleMouseProcessed;

public:
	//bool SetElementUI( const char *szFileName );

	template< typename T >
	T *AddItem(int nID = -1);

	template< typename T >
	T *InsertItem(int nID = 0);

	/*template<typename T >
	void SwapItem(int nleft, int nRight);*/

	void RemoveItem( int nIndex );
	void RemoveAllItems();

	template< typename T >
	T *GetItem( int nIndex ) 
	{
		if( m_vecItems.size() == 0 || nIndex >= (int)m_vecItems.size() )
			return NULL;

		return (T*)m_vecItems[nIndex]->pData;
	}

	template<typename T>
	T*	FindItem(int nID)
	{
		std::vector<SListBoxItem*>::iterator iter = m_vecItems.begin();
		for (; iter != m_vecItems.end(); ++iter)
		{
			SListBoxItem* pItem = *iter;
			if (pItem)
			{
				T* pTmp = (T*)pItem->pData;
				if (pTmp)
				{
					int dlgID = pTmp->GetDialogID();
					if (dlgID < 0 || dlgID != nID)
						continue;
					else
						return pTmp;
				}
			}
		}

		return NULL;
	}

	// Unlike normal listbox, extended listbox may skip rendering the scrollbar.
	// Seems too rarely used a flag to add as a property in the tool.
	void SetRenderScrollBar( bool bRender = true ) { m_bRenderScrollBar = bRender; }

	// Unlike normal listbox, extended listbox may skip rendering the selection bar.
	void SetRenderSelectBar( bool bRender = true ) { m_bRenderSelectBar = bRender; }

	// Whether to return after processing item dialog in HandleMouse. Default is false (breaks).
	// Returning like this causes many issues with scrollbar handling. Probably should remove it.
	void SetReturnHandleMouseProcessed( bool bReturn = false ) { m_bReturnHandleMouseProcessed = bReturn; }

public:
	virtual void Initialize( SUIControlProperty *pProperty );
	virtual bool HandleMouse( UINT uMsg, float fX, float fY, WPARAM wParam, LPARAM lParam );
	virtual void Render( float fElapsedTime );

	virtual void UpdateRects();
};

// There are two ways to use a UI file as a list box item.
// One approach assigns a child window to each item,
// The other way is to create a window always in Show(false) state and manage it directly.
//
// Looking at each characteristic,
// When handled as a child window, input processing is done automatically,
// Just call Show(true/false) at the right time (when item is visible while scrolling) and right position (listbox item position).
// However, as the biggest drawback,
// Because dialog MsgProc is used as-is, inputs handled in the child dialog do not reach the list box.
// So messages need to be sent separately to the listbox control,
// Disassembling the dialog to handle it as an exception seems too messy.
// Another major drawback is that dialog rendering order cannot be easily changed,
// Because it draws its own window dialog, then controls, then child dialogs, the list item selection image gets drawn underneath the child window.
// For this reason, the method below was chosen.
//
// If created and managed as an invisible window,
// Seems awkward since input handling and rendering must be controlled directly, but
// Advantage is not having to write messy code in the dialog .cpp. (Special handling is done in the specialized EtUIListBoxEx class.)
//

//#define _LISTITEM_CHILD

template< typename T >
T *CEtUIListBoxEx::AddItem(int nID)
{
	// Uses SListBoxItem, but only uses pData.
	SListBoxItem *pNewItem;

	pNewItem = new SListBoxItem;

#ifdef _LISTITEM_CHILD
	T *pData = new T( UI_TYPE_CHILD, m_pParent nID);
#else
	T *pData = new T( UI_TYPE_SELF, NULL, nID );
#endif
	pData->Initialize( true );
	pData->SetElementDialog( true );
	pNewItem->pData = pData;
	wcscpy_s( pNewItem->strText, 256, L"" );
	pNewItem->nValue = 0;
	SetRect( &pNewItem->rcActive, 0, 0, 0, 0 );
	pNewItem->bSelected = false;
	pNewItem->dwColor = 0xFFFFFFFF;

	m_vecItems.push_back( pNewItem );
	m_ScrollBar.SetTrackRange( 0, ( int )m_vecItems.size() );

	CheckListBoxItem();

	return pData;
}

template< typename T >
T * CEtUIListBoxEx::InsertItem(int nID)
{
	// Uses SListBoxItem, but only uses pData.
	SListBoxItem *pNewItem;

	pNewItem = new SListBoxItem;

#ifdef _LISTITEM_CHILD
	T *pData = new T( UI_TYPE_CHILD, m_pParent nID);
#else
	T *pData = new T( UI_TYPE_SELF, NULL, nID );
#endif
	pData->Initialize( true );
	pData->SetElementDialog( true );
	pNewItem->pData = pData;
	wcscpy_s( pNewItem->strText, 256, L"" );
	pNewItem->nValue = 0;
	SetRect( &pNewItem->rcActive, 0, 0, 0, 0 );
	pNewItem->bSelected = false;
	pNewItem->dwColor = 0xFFFFFFFF;

	m_vecItems.insert( m_vecItems.begin() + nID, pNewItem );
	m_ScrollBar.SetTrackRange( 0, ( int )m_vecItems.size() );

	CheckListBoxItem();

	return pData;
}
