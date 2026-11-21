#pragma once

#if defined(PRE_MOD_ITEM_COMPOUND_DIALOG_RENEWAL)

#include "DnCustomDlg.h"
#include "DnSmartMoveCursor.h"

class CDnItem;

class CDnCompoundSelectMaterial : public CDnCustomDlg
{
public:
	enum {
		SLOT_PER_PAGE = 5,
	};

	enum eSelectMode {
		MODE_NORMAL  = 0,
		MODE_SUFFIX  = 1,
		MODE_UPGRADE = 2,
	};

public:
	CDnCompoundSelectMaterial( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnCompoundSelectMaterial();

protected:
	CEtUIStatic *m_pStaticTitle;
	CEtUIStatic *m_pStaticPage;
	CEtUIStatic *m_pStaticBg;

	CDnItemSlotButton *m_pTargetSlot;
	CDnItemSlotButton *m_pMaterialSlots[SLOT_PER_PAGE];
	CEtUIStatic *m_pSelectFrames[SLOT_PER_PAGE];

	CEtUIButton *m_pBtnOK;
	CEtUIButton *m_pBtnPrior;
	CEtUIButton *m_pBtnNext;

	CDnItem *m_pTargetItem;
	std::vector<CDnItem*> m_vecMaterialItems;

	int m_nSelectedIndex;
	int m_nMaxPage;
	int m_nCurPage;
	int m_nSelectMode;

	CDnSmartMoveCursor m_SmartMove;

	void UpdateTitle();
	void UpdatePageText();
	void RefreshMaterialSlots();
	void UpdateSelectFrame();
	void ClearMaterials();

public:
	virtual void Initialize( bool bShow );
	virtual void InitialUpdate();
	virtual void Show( bool bShow );
	virtual bool MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
	virtual void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg = 0 );

	void SetSelectMode( int nMode ) { m_nSelectMode = nMode; }
	void SetTargetItem( CDnItem *pItem );
	void AddMaterialItem( CDnItem *pItem );
	CDnItem *GetSelectedItem() const;
	INT64 GetSelectedSerialID() const;
};

#endif // PRE_MOD_ITEM_COMPOUND_DIALOG_RENEWAL
