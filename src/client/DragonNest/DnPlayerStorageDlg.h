#pragma once

#include "EtUITabDialog.h"
#include "DnInven.h"
#include "DnInventoryDlg.h"

class CDnStorageDlg;
class CDnSlotButton;
class MIInventoryItem;
class CDnMoneyInputDlg;
class CDnStoreConfirmExDlg;
class CDnQuickSlotButton;
class CDnItem;

class CDnPlayerStorageDlg : public CEtUIDialog, public CDnInventoryDlg, public CEtUICallback
{
public:
	CDnPlayerStorageDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnPlayerStorageDlg(void);

protected:
	CDnStorageDlg *m_pStoreCommonDlg;

	CEtUIStatic *m_pStaticGold;
	CEtUIStatic *m_pStaticSilver;
	CEtUIStatic *m_pStaticBronze;

	CEtUIButton *m_pButtonMoneyIn;	// Deposit
	CEtUIButton *m_pButtonMoneyOut;	// Withdraw

	enum emMONEY_CONFIRM_TYPE
	{
		MONEY_CONFIRM_IN,
		MONEY_CONFIRM_OUT,
	};

	enum
	{
		MONEY_INPUT_DIALOG,
		ITEM_ATTACH_CONFIRM_DIALOG
	};

	emMONEY_CONFIRM_TYPE m_emMoneyType;

	CDnMoneyInputDlg *m_pMoneyInputDlg;
	CDnStoreConfirmExDlg* m_pSplitConfirmExDlg;

	int m_iRemoteItemID;

public:
	virtual void SetItem( MIInventoryItem *pItem );
	virtual void ResetSlot( MIInventoryItem *pItem );

	virtual void SetUseItemCnt( DWORD dwItemCnt );

	virtual bool IsEmptySlot();
	virtual int GetEmptySlot();

	void MoveItemToStorage(CDnQuickSlotButton* pFromSlot, bool bItemSplit);
	void RequestMoveItem(eItemMoveType moveType, CDnItem* pItem, int itemCount);

public:
	virtual void InitialUpdate();
	virtual void Initialize( bool bShow );
	virtual void Show( bool bShow );
	virtual void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg = 0 );
	virtual void Process( float fElapsedTime );
	virtual void OnUICallbackProc( int nID, UINT nCommand, CEtUIControl *pControl, UINT uiMsg = 0 );

	void SetRemoteItemID( int iRemoteItemID );

#if defined( PRE_PERIOD_INVENTORY )
	void EnablePeriodStorage( const bool bEnable, const __time64_t tTime = 0 );
#endif	//	#if defined( PRE_PERIOD_INVENTORY )
};
