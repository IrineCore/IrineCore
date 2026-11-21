#pragma once

#include "DnCustomDlg.h"
#include "DnCustomControlCommon.h"

class CDnSlotButton;
class CDnQuickSlotButton;
class MIInventoryItem;

class CDnQuickSlotDlg : public CDnCustomDlg
{
public:
	CDnQuickSlotDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnQuickSlotDlg(void);

protected:
	std::vector< CDnQuickSlotButton* > m_vecSlotButton;

protected:
	bool SetSlot( CDnSlotButton *pDragButton, CDnSlotButton *pPressedButton );
	void ResetSlot( int nIndex );
	int FindQuickSlotItem( MIInventoryItem *pItem );

	// Sound when placing a skill into a slot and when picking it up
	int m_nSkillLaydownSoundIndex;
	int m_nSkillDragSoundIndex;
	

public:
	bool InitSlot( int nIndex, MIInventoryItem *pItem );
	bool SetSlot( int nIndex, MIInventoryItem *pItem );
	bool SetEventSlot( MIInventoryItem *pItem ); // Register in event slot <consumable slot remembered only on client>
	int FindEmptyQuickSlotIndex();
	int FindSkillQuickSlotIndex( int nSkillID );

	void ResetAllSlot();
	void OnRefreshSlot();
	void EnableQuickSlot(bool bEnable);
	void GetItemList(std::vector<int> &vecList); // Get the item registered in the quickslot.
	void ResetSlotID(int nSlotID) { this->ResetSlot(nSlotID); };
public:
	virtual void Initialize(bool bShow);
	virtual void InitCustomControl( CEtUIControl *pControl );
	virtual void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg = 0 );
	virtual void Process( float fElapsedTime );
};