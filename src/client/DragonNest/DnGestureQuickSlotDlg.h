#pragma once

#include "DnCustomDlg.h"
#include "DnCustomControlCommon.h"

class CDnSlotButton;
class CDnLifeSkillButton;
class MIInventoryItem;
class CDnItem;

// LifeSkill quickslot dialog (skills usable only in Normal_Stand)
class CDnLifeSkillQuickSlotDlg : public CDnCustomDlg
{
public:
	CDnLifeSkillQuickSlotDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnLifeSkillQuickSlotDlg(void);

protected:
	std::vector< CDnLifeSkillButton* > m_vecSlotButton;

protected:
	bool SetSlot( CDnSlotButton *pDragButton, CDnSlotButton *pPressedButton );
	// Because gesture button and secondary skill button can be shown simultaneously, item type must be passed additionally like this.
	int FindLifeSkillSlotItem( int nLifeSkillID, MIInventoryItem::InvenItemTypeEnum ItemType );
	int FindVehicleSlotItem( INT64 nItemSerialID, MIInventoryItem::InvenItemTypeEnum ItemType );

	// Unlike default quickslots, there is only one dialog, so it can be handled directly without creating it in MainDlg.
	void ChangeQuickSlotButton( CDnSlotButton *pDragButton, CDnSlotButton *pPressedButton );
	bool SetSlot( int nIndex, int nLifeSkillID, MIInventoryItem *pItem, MIInventoryItem::InvenItemTypeEnum ItemType );

	// Sound when placing a skill into a slot and when picking it up
	int m_nSkillLaydownSoundIndex;
	int m_nSkillDragSoundIndex;

	// These are the approaches considered when first implementing this.
	// Method 1.
	// Create one quickslot button and keep it just for rendering
	//
	// Method 2.
	// Full integration. Integrate all separated LifeSkillButton instances into QuickSlotButton, then update and unify UI custom control settings.
	// => Too much to tear apart, including all the UI. If quickslots are ever revamped later, it would be completely double work.
	//
	// Method 3.
	// Extend LifeSkillButton so only mounts and pets can be registered.
	// Awkward to implement. Would end up holding all ItemSlotButton textures too, which is duplicate code, so exclude this.
	//
	// So the chosen option is method 1.
	// The issue is CreateControl cannot link a template if none exists, so it must be pre-placed in the UI and linked for use.
	// And after equipping, it disappears from inventory, so in the current structure it is automatically removed from the quick slot.
	// To resolve this, also store DnItem.
	struct LifeSkillEx
	{
		CDnQuickSlotButton *pSlotButton;
		CDnItem *pItem;
	};
	std::map<CDnLifeSkillButton*, LifeSkillEx> m_mapMatchedButton;
	float m_fVehicleCoolTime;
	int m_nGestureSkillIndex;

public:
	bool InitSlot( int nIndex, int nLifeSkillID, MIInventoryItem *pItem, MIInventoryItem::InvenItemTypeEnum ItemType );
	void ResetSlot( int nIndex, bool bSendPacket = true );
	bool ResetSecondarySkillQuickSlot( int nSecondarySkillID );
	void EnableQuickSlot(bool bEnable);
	void RefreshVehicleItem();

public:
	virtual void Initialize(bool bShow);
	virtual void InitCustomControl( CEtUIControl *pControl );
	virtual void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg = 0 );
	virtual void Process( float fElapsedTime );
};