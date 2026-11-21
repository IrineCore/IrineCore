#pragma once
#include "DnCustomDlg.h"

class MIInventoryItem;
class CDnInvenSepDlg;
class CDnStoreConfirmDlg;
class CDnMailDlg;
class CDnItem;
class CDnInvenTabDlg;
class CDnAskLookChgDlg;
#ifdef PRE_ADD_INVEN_EXTENSION
class CDnInvenExtensionDlg;
#endif
class CDnInvenSlotDlg : public CDnCustomDlg, public CEtUICallback
{
	enum
	{
		INVEN_SPLIT_DIALOG,
		INVEN_HAIRDYE_DIALOG,
		INVEN_REMOTE_REPAIR_DIALOG,
		INVEN_PET_ADDEXP_DIALOG,
#ifdef PRE_ADD_INVEN_EXTENSION
		INVEN_EXTENSION_DIALOG,
#endif // PRE_ADD_INVEN_EXTENSION

#if defined( PRE_PERIOD_INVENTORY )
		INVEN_PERIOD_INVEN,
		INVEN_PERIOD_STORAGE,
#endif	// #if defined( PRE_PERIOD_INVENTORY )
#if defined(PRE_ADD_TALISMAN_SYSTEM)
		INVEN_TALISMAN_DETACH_DIALOG,
#endif
	};

public:
	CDnInvenSlotDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL  );
	virtual ~CDnInvenSlotDlg(void);

protected:
	std::vector< CDnQuickSlotButton* > m_vecSlotButton;
	CDnInvenSepDlg *m_pInvenSepDlg;
#ifdef PRE_ADD_INVEN_EXTENSION
	CDnInvenExtensionDlg *m_pInvenExtensionDlg;
#endif
	CDnAskLookChgDlg	*m_pAskLookChgDlg;
	CDnQuickSlotButton	*m_pPressedButton;
	ITEM_SLOT_TYPE		m_SlotType;
	int					m_SlotStartIndex;
#ifdef PRE_ADD_INVEN_EXTENSION
	int					m_nUseItemCnt;
#endif
#if defined(PRE_ADD_TALISMAN_SYSTEM)
	int					m_nTalismanSrcSlot;
	int					m_nTalismanDestSlot;
	INT64				m_biTalismanSerial;
	int					m_nTalismanCount;
#endif

protected:
	bool RequestMoveItem( CDnSlotButton *pDragButton, CDnQuickSlotButton *pPressedButton );

public:
	void SetItem( MIInventoryItem *pItem );
	void SetItemByIndex( MIInventoryItem *pItem, int index );
	void ResetSlot( int nSlotIndex );
	void SetUseItemCnt( DWORD dwItemCnt );
	int	 GetSlotStartIndex();
	void SetSlotType( ITEM_SLOT_TYPE slotType );
	CDnItem * GetSlotItem( int nSlotIndex );
#if defined(PRE_ADD_AUTOUNPACK) || defined(PRE_ADD_CASHREMOVE)
	const std::vector< CDnQuickSlotButton* > & GetSlotButtons() {
		return m_vecSlotButton;
	}

	ITEM_SLOT_TYPE GetSlotType() {
		return m_SlotType;
	}
#endif

	bool IsEmptySlot();
	int  GetEmptySlot();
	int  GetEmptySlotCount();

	void ReleaseNewGain();
	int  GetRegisteredSlotCount();

	void DisablePressedButtonSplitMode() { if( m_pPressedButton ) m_pPressedButton->DisableSplitMode(true); }

public:
	virtual void Initialize(bool bShow, int pageNum, int slotCount, const char* pResFileName = "InvenItemSlotDlg.ui");
	virtual void InitialUpdate();
	virtual void InitCustomControl( CEtUIControl *pControl );
	virtual void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg = 0 );
	//virtual void Process( float fElapsedTime );
	virtual void OnUICallbackProc( int nID, UINT nCommand, CEtUIControl *pControl, UINT uiMsg = 0 );
#ifdef PRE_ADD_INVEN_EXTENSION
	virtual void Show( bool bShow );
	virtual void MoveDialog( float fX, float fY );
	void CheckShowExtensionDlg( bool bShow );
#endif

private:
	bool IsOnMailMode(CDnMailDlg* pMailDlg) const;
	void HandleItemClick(CDnItem* pItem, CDnMailDlg* pMailDlg, CDnInvenTabDlg* pInvenDlg);
#ifdef PRE_ADD_COSRANDMIX
	bool ReadyCostumeMix(CDnItem* pItem);
#endif

#ifdef PRE_FIX_COSMIX_NORESPONSE_ON_ERROR
	bool IsOnCostumeMixMode(const CDnItem& item) const;
	void CancelCostumeMixReady(const CDnItem& item);
#endif

#ifdef PRE_ADD_EQUIPLOCK
	bool CheckEquipItemLock(const CDnItem& item) const;
#endif
};
