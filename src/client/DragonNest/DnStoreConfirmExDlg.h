#pragma once
#include "DnCustomDlg.h"
#include "DnStoreTabDlg.h"
#include "DnSmartMoveCursor.h"
#include "DnInterface.h"

class CDnItem;
#ifdef PRE_ADD_NPC_REPUTATION_SYSTEM
class CDnRepStoreBenefitTooltipDlg;
#endif // #ifdef PRE_ADD_NPC_REPUTATION_SYSTEM

class CDnStoreConfirmExDlg : public CDnCustomDlg
{
public:
	CDnStoreConfirmExDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL  );
	virtual ~CDnStoreConfirmExDlg(void);

protected:
	CDnItemSlotButton *m_pItemSlotButton;

	CEtUIStatic *m_pItemName;
	CEtUIStatic *m_pItemCount;
	CEtUIStatic *m_pItemAmount1;
	CEtUIStatic *m_pItemAmount2;
	CEtUIStatic *m_pItemText;
	CEtUIEditBox *m_pEditBoxItemCount;
#ifdef PRE_ADD_NPC_REPUTATION_SYSTEM
	CEtUIStatic *m_pReputationBenefit;
	CDnRepStoreBenefitTooltipDlg* m_pStoreBenefitTooltip;
	int m_iNowStoreBenefit;
#endif // #ifdef PRE_ADD_NPC_REPUTATION_SYSTEM
#ifdef PRE_MOD_SELL_SEALEDITEM
	CEtUIStatic *m_pStaticSellSealedItem;
#endif

	// Named exactly as created by the UI designer.
	CEtUIButton *m_pItemUp;
	CEtUIButton *m_pItemDown;

	CDnSlotButton*	m_pFromSlotBtnCache;

	CEtUIButton *m_pButtonMin;
	CEtUIButton *m_pButtonMax;
	CEtUIButton* m_pOKButton;
	CEtUIButton* m_pCancelButton;

	// Smart move
	CDnSmartMoveCursor m_SmartMove;

	CDnInterface::emSTORE_CONFIRM_TYPE m_emConfirmType;

protected:
	void UpdateAmount();
	void SetCombinedBuy( CDnItem * pItem, int nTabID, int nCount );

public:
	void SetItem(CDnSlotButton* pFromSlot, CDnInterface::emSTORE_CONFIRM_TYPE type);
	void SetItem(CDnItem* pItem, CDnInterface::emSTORE_CONFIRM_TYPE type, int slotCount);
	void EnalbleButtons(bool bEnable);
	CDnSlotButton*	GetFromSlot() const { return m_pFromSlotBtnCache; }
	void Reset();
	CDnItem *GetItem() { return (CDnItem*)m_pItemSlotButton->GetItem(); }

	int GetItemCount();

	CDnInterface::emSTORE_CONFIRM_TYPE GetConfirmType() { return m_emConfirmType; }
	void SetRepurchaseBuy(const CDnItem& item);

public:
	virtual void Initialize( bool bShow );
	virtual void InitialUpdate();
	virtual void InitCustomControl( CEtUIControl *pControl );
	virtual void Show( bool bShow );
	virtual void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg = 0 );
	virtual bool MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
};