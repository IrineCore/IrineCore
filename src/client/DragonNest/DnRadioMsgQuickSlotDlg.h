#pragma once

#include "DnCustomDlg.h"
#include "DnCustomControlCommon.h"

class CDnSlotButton;
class CDnRadioMsgButton;
class MIInventoryItem;

// Radio message quickslot dialog
class CDnRadioMsgQuickSlotDlg : public CDnCustomDlg
{
public:
	CDnRadioMsgQuickSlotDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnRadioMsgQuickSlotDlg(void);

protected:
	std::vector< CDnRadioMsgButton* > m_vecSlotButton;

protected:
	bool SetSlot( CDnSlotButton *pDragButton, CDnSlotButton *pPressedButton );
	void ResetSlot( int nIndex );
	int FindRadioMsgSlotItem( int nRadioMsgID );

	// Unlike default quickslots, there is only one dialog, so it can be handled directly without creating it in MainDlg.
	void ChangeQuickSlotButton( CDnSlotButton *pDragButton, CDnSlotButton *pPressedButton );
	bool SetSlot( int nIndex, int nRadioMsgID );

	// Sound when placing a skill into a slot and when picking it up
	int m_nSkillLaydownSoundIndex;
	int m_nSkillDragSoundIndex;

public:
	bool InitSlot( int nIndex, int nRadioMsgID );

public:
	virtual void Initialize(bool bShow);
	virtual void InitCustomControl( CEtUIControl *pControl );
	virtual void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg = 0 );
};