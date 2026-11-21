#pragma once
#include "EtUIDialog.h"
#include "DnCustomDlg.h"
#if defined(PRE_ADD_CASHSHOP_CREDIT)
#include "DnCashShopDefine.h"
#endif


#ifdef PRE_ADD_INSTANT_CASH_BUY

#define INSTANT_CASHSHOP_BUY_DIALOG		0x1001

class CDnInstantCashShopBuyMessageBox;


class CDnInstantCashShopBuyDlg : public CDnCustomDlg, public CEtUICallback
{
public:
	CDnInstantCashShopBuyDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnInstantCashShopBuyDlg();

public:
	virtual void Initialize( bool bShow );
	virtual void InitialUpdate();
	virtual void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg = 0 );
	virtual void OnUICallbackProc( int nID, UINT nCommand, CEtUIControl *pControl, UINT uiMsg = 0 );
	virtual bool MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );

public:
	bool SetBuyType( eInstantCashShopBuyType eBuyType, CEtUICallback* pParentCallback );
	void OnRecvInstantBuyResult( SCCashShopBuy* pPacket );

protected:
	void ResetData();
	void ResetUIList();
	void SetItemPage( int nCurrentPage );
	void SetSelectedItemInfo( int nSelectedItemIndex );
	void UpdateBuyInfo();
	void EnableUI( bool bEnable );
	void PrevPage();
	void NextPage();

public:
	enum
	{
		MESSAGEBOX_CONFIRM = 0,
		MESSAGEBOX_REQUEST_BUY,
		MESSAGEBOX_RECIEVE_RESULT,
#ifdef PRE_ADD_CASHSHOP_CREDIT
		MESSAGEBOX_CREDIT_ERROR,
#endif // PRE_ADD_CASHSHOP_CREDIT
	};

protected:
	CDnItemSlotButton*	m_pItemSlotBtn0;
	CDnItemSlotButton*	m_pItemSlotBtn1;
	CDnItem*			m_pItem0;
	CDnItem*			m_pItem1;

	CEtUIStatic*	m_pStaticSelect0;
	CEtUIStatic*	m_pStaticSelect1;
	CEtUIStatic*	m_pStaticItemName0;
	CEtUIStatic*	m_pStaticItemName1;
	CEtUIStatic*	m_pStaticItemPeriod0;
	CEtUIStatic*	m_pStaticItemPeriod1;
	CEtUIStatic*	m_pStaticItemCount0;
	CEtUIStatic*	m_pStaticItemCount1;
	CEtUIStatic*	m_pStaticReserve0;
	CEtUIStatic*	m_pStaticReserve1;
	CEtUIStatic*	m_pStaticCash0;
	CEtUIStatic*	m_pStaticCash1;

	CEtUIStatic*	m_pStaticPage;

	CEtUIStatic*	m_pStaticSelectItem;
	CEtUIStatic*	m_pStaticSelectItemCount;
	CEtUIStatic*	m_pStaticSelectItemPeriod;
	CEtUIStatic*	m_pStaticSelectCash;
	CEtUIStatic*	m_pStaticSelectReserve;
	CEtUIStatic*	m_pStaticSelectRemain;
#ifdef PRE_ADD_CASH_REFUND
	CEtUICheckBox*	m_pCheckBoxMoveCashInven;
#endif // PRE_ADD_CASH_REFUND
	CEtUICheckBox*	m_pCheckBoxBuyReserve;
#ifdef PRE_ADD_CASHSHOP_CREDIT
	CEtUICheckBox*	m_pCheckBoxBuyCash;
	CEtUICheckBox*	m_pCheckBoxBuyCredit;
#endif // PRE_ADD_CASHSHOP_CREDIT

	CEtUIButton*	m_pPrevButton;
	CEtUIButton*	m_pNextButton;
	CEtUIButton*	m_pOKButton;

	std::vector<CASHITEM_SN>	m_vecItemSNList;

	int							m_nTotalPage;
	int							m_nCurrentPage;
	int							m_nSelectedItemIndex;

	CEtUICallback*						m_pParentCallback;
	CDnInstantCashShopBuyMessageBox*	m_pInstantCashShopBuyMessageBox;
	eInstantCashShopBuyType				m_eBuyType;

	bool								m_bBuying;
	int									m_nCloseWindow;
#if defined(PRE_ADD_CASHSHOP_CREDIT)
	eBuyMode							m_eBuyMode;
#endif
};



class CDnInstantCashShopBuyMessageBox : public CEtUIDialog
{
public:
	CDnInstantCashShopBuyMessageBox( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnInstantCashShopBuyMessageBox(void);

	void SetMsgBox( int nID, CEtUICallback* pCallBack );

	virtual void InitialUpdate();
	virtual void Initialize( bool bShow );

protected:
	virtual void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg );
};

#endif // PRE_ADD_INSTANT_CASH_BUY