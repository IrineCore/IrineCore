#pragma once
#include "DnCustomDlg.h"

class CDnItem;
class CDnItemDisjointResultDlg;
class CDnItemDisjointDlg : public CDnCustomDlg
{
	enum
	{
		ITEM_DISJOINT_ITEMSLOT_MAX = 4,
	};

public:
	CDnItemDisjointDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnItemDisjointDlg(void);

protected:
	std::vector<CDnItem*> m_vecItem;
	std::vector<CDnItemSlotButton*> m_vecSlotButton;
	std::map<int, CEtUIRadioButton*> m_mapRadioButton;
	CEtUIListBoxEx *m_pListBoxEx;
	CDnItemDisjointResultDlg *m_pDisjointResultDlg;

	CEtUIStatic *m_pStaticGold;
	CEtUIStatic *m_pStaticSilver;
	CEtUIStatic *m_pStaticBronze;
	DWORD m_dwColorGold;
	DWORD m_dwColorSilver;
	DWORD m_dwColorBronze;

	CEtUIButton *m_pButtonDisjoint;
	CEtUIButton *m_pButtonCancel;


	std::vector<int> m_pItemCacheList;
	// Same as items placed in the trade window
	// Remember items placed in disassemble slots so they appear disabled in inventory window.
	std::vector<CDnQuickSlotButton*> m_vecQuickSlotButton;

	bool CheckItemDuplication( CDnItem *pItem ) const;
	bool IsEmptySlot();
	int GetEmptySlot();

	// Evaluate price using the registered item and...
	void CheckItemList() const;

	bool IsDisjoitableItem(CDnSlotButton* pDragButton, CDnItem* pItem) const;

	void Clear(bool bAll);
	void SelectSlot( int nSlotIndex );
	void RefreshExpectItemList( int nSlotIndex );
public:
	void RemoveItemFromCache(int nItemID, int nItemLevel);

	// Direct registration on right-click in inventory.
	void AddItemToList( CDnQuickSlotButton *pPressedButton );
	void ShowDisjointResult( const std::vector<TItem> &vecItems );
	CDnItemDisjointResultDlg *GetDisjointResultDlg() const { return m_pDisjointResultDlg; }
public:
	virtual void Initialize( bool bShow );
	virtual void InitialUpdate();
	virtual void Show( bool bShow );
	virtual void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg = 0 );
};