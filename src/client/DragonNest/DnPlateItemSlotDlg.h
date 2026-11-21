#pragma once
#include "DnCustomDlg.h"
#include "DnItemTask.h"

class CDnItem;

class CDnPlateItemSlotDlg : public CDnCustomDlg
{
public:
	CDnPlateItemSlotDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnPlateItemSlotDlg(void);

protected:

	int m_nMaxItemSlot;

	CDnItem *m_pItem[5];
	CDnItemSlotButton *m_pItemSlotButton[5];
	CEtUIStatic *m_pItemSlotEffect[5];

public:
	// Has a separate Init function. Argument is a value between 1 and 5. Call once after creation to initialize.
	void Init( int nMaxItemSlot );

	// Return true if item type and count conditions are met.
	bool SetCompoundInfo(int *pItemID, int *pItemCount);

	// Configure individual slot
	void SetItemSlot( int nSlotIndex, int nItemID, int nNeedItemCount, int nCurItemCount );
public:
	virtual void InitialUpdate();
	virtual void Show( bool bShow );
};