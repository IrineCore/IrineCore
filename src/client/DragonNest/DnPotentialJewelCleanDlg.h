#pragma once
#include "DnCustomDlg.h"
#include "DnItem.h"

class CDnItem;
class CDnQuickSlotButton;

#ifdef PRE_MOD_POTENTIAL_JEWEL_RENEWAL

// Separated the Cube of Destruction dialog.
class CDnPotentialJewelCleanDlg : public CDnCustomDlg, public CEtUICallback
{
	enum
	{
		ITEM_CLEAN_CATEGORY_WEAPON = 17,
		ITEM_CLEAN_CATEGORY_ARMOR,
		ITEM_CLEAN_CATEGORY_ACCESSORY,
	};
public:
	CDnPotentialJewelCleanDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnPotentialJewelCleanDlg();

protected:
	CDnItem *m_pItem;
	CDnItemSlotButton *m_pItemSlot;
	CEtUIStatic *m_pStaticText;
	CEtUIButton*		m_pButtonPotentialApply;
	CEtUIButton *m_pButtonApply;
	CDnQuickSlotButton *m_pButtonQuickSlot;
	int m_nSoundIdx;
	INT64				m_nPotentialID;
	CDnItem *m_pPotentialItem;
	char				m_cPrevPotentialIndex;
public:
	void SetPotentialItem( CDnQuickSlotButton *pPressedButton );
	bool IsPotentialableItem( CDnSlotButton *pDragButton );
	void CheckPotentialItem();
	void SetPotentialItem( CDnItem *pItem );
	void OnRecvPotentialItem( int nResult, int nSlotIndex );
	void			SetPotentialItemID(CDnItem* pItem);

public:
	virtual void Initialize( bool bShow );
	virtual void InitialUpdate();
	virtual void Show( bool bShow );
	virtual void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg = 0 );
};

#endif // PRE_MOD_POTENTIAL_JEWEL_RENEWAL