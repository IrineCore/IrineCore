#pragma once
#include "EtUITabDialog.h"
#include "DnInven.h"
#include "DnInventoryDlg.h"
#include "DnCustomControlCommon.h"
#include "DnSmartMoveCursor.h"

class MIInventoryItem;
class CEtUIRadioButton;
class CDnSlotButton;
class CDnInvenDlg;
class CDnInvenSymbolDlg;
class CDnPrivateMarketDlg;
class CDnQuickSlotButton;
class CDnInvenGestureDlg;
class CDnInvenCashDlg;
class CDnInvenPetDlg;

class CDnInvenTabDlg : public CEtUITabDialog, public CEtUICallback, public CDnInventoryDlg
{
public:
	enum
	{
		PRIVATE_MARKET_DIALOG,
	};
	enum eDragCtrlType
	{
		RELEASE,
		RENDER,
	};

	enum eInvenTabType
	{
		eCOMMON		= 0x0001,
		eCASH		= 0x0002,
		eQUEST		= 0x0004,
		eGESTURE	= 0x0008,

#ifdef PRE_ADD_CASHREMOVE
		ePET        = 0x0010,
		eCASHREMOVE = 0x0020,
#else
		ePET        = 0x0010,
#endif
	};

public:
	CDnInvenTabDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL  );
	virtual ~CDnInvenTabDlg(void);

protected:
	CEtUIRadioButton *m_pCommonTabButton;
	CEtUIRadioButton *m_pCashTabButton;
	CEtUIRadioButton *m_pQuestTabButton;
	CEtUIRadioButton *m_pGestureTabButton;
	CEtUIRadioButton *m_pPetTabButton;

	CEtUIStatic *m_pCommonTabNewStatic;
	CEtUIStatic *m_pCashTabNewStatic;
	CEtUIStatic *m_pPetTabNewStatic;

	CDnInvenPetDlg  *m_pInvenPetDlg;

	CDnInvenDlg *m_pInvenCommonDlg;
	CDnInvenCashDlg *m_pInvenCashDlg;
	CDnInvenDlg *m_pInvenQuestDlg;
	CDnInvenGestureDlg *m_pInvenGestureDlg;

#ifdef PRE_ADD_CASHREMOVE
	CEtUIRadioButton * m_pInvenStandByCashRemoveBtn;
	class CDnInvenStandByCashRemoveDlg * m_pInvenStandByCashRemoveDlg;
#endif

	CDnSlotButton *m_pDragButton;
	CEtUIButton *m_pTrashButton;
	CEtUIStatic *m_pStaticGold;
	CEtUIStatic *m_pStaticSilver;
	CEtUIStatic *m_pStaticBronze;

	CEtUIButton *m_pSortButton;

	int m_nTrashSoundIndex;

	CDnSlotButton *m_pTrashItemButton;
	CDnPrivateMarketDlg *m_pPrivateMarketDlg;

	CDnSmartMoveCursorEx m_SmartMoveEx;

#if defined(PRE_ADD_AUTOUNPACK) || defined(PRE_ADD_CASHREMOVE)
	eInvenTabType m_EnableInvenType; // Currently active inventory type.
#endif

public:
	virtual void SetItem( MIInventoryItem *pItem );
	virtual void ResetSlot( MIInventoryItem *pItem );

	virtual void SetUseItemCnt( DWORD dwItemCnt );
	virtual void SetUseQuestItemCnt( DWORD dwItemCnt );

	virtual bool IsEmptySlot();
	virtual int GetEmptySlot();

#if defined(PRE_ADD_AUTOUNPACK) || defined(PRE_ADD_CASHREMOVE)
	eInvenTabType GetEnableInvenType(){
		return m_EnableInvenType; // Currently active inventory type.
	}
	void SetInvenTabType( int tabID ); // Currently active inventory type.
#endif

public:	
	void ShowPrivateMarketDialog( bool bShow, bool bCancel );
	bool IsShowPrivateMarketDialog();
	void AddPrivateMarketChatMe( const wchar_t *wszChat );
	void AddPrivateMarketChatYou( const wchar_t *wszChat );
	void SetPrivateMarketName( const wchar_t *wszMe, const wchar_t *wszYou );

	bool IsYourName( const wchar_t *wszName );
	void ShowTab( int nTabID );
	void ShowTabNew( int nTabID );

	// Handle sending items to the trade window via right-click
	void AddItemToPrivateMarketTradeList(CDnQuickSlotButton* pPressedButton, bool bItemSplit);
	void SetDragCtrl(eDragCtrlType type, bool bParam = false);
	void ShowRadioButton( bool bShow );
	void ForceOpenNormalInvenTab( bool bEnable );
	void EnableInvenTab(int type);

#ifdef PRE_ADD_SECONDARY_SKILL
	void EnableSortButton();
#endif // PRE_ADD_SECONDARY_SKILL

	// Check empty slot count in normal inventory
	int GetEmptySlotCount();

	// When left-clicking and dragging from inventory to another window, that slot is stored in InvenSlotDlg's PressedButton.
	// If item move was ignored due to an error, the slot's split counter must be forcibly reset; this function handles that.
	void DisablePressedButtonSplitMode();

	// This function replaces GetInvenDlg(ST_INVENTORY_CASH), and
	// Did not want to create this, but added it to avoid making a separate cash inventory when opening inventory in the Cash Shop.
	CDnInvenCashDlg *GetCashInvenDlg() { return m_pInvenCashDlg; }


#ifdef PRE_ADD_CASHREMOVE
	void RemoveCashItemAccept(); // Accept cash item deletion.
	void RemoveCashItemCancel(); // Cancel cash item deletion.		
#endif

#ifdef PRE_ADD_CASHINVENTAB
	class CDnItem * GetCashInvenItemBySN( INT64 SN );
#endif // PRE_ADD_CASHINVENTAB

public:
	virtual void InitialUpdate();
	virtual void Initialize( bool bShow );
	virtual void Show( bool bShow );
	virtual void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg = 0 );
	virtual void OnUICallbackProc( int nID, UINT nCommand, CEtUIControl *pControl, UINT uiMsg = 0 );
	virtual void Process( float fElapsedTime );

	CDnInvenDlg *GetInvenDlg( ITEM_SLOT_TYPE Type );

	CEtUIRadioButton * GetCommonTab()	{ return m_pCommonTabButton; }
	CEtUIRadioButton * GetQuestTab()	{ return m_pQuestTabButton; }
	CEtUIRadioButton * GetCashTab()		{ return m_pCashTabButton; }
	CEtUIRadioButton * GetGestureTab()	{ return m_pGestureTabButton; }
	CEtUIRadioButton * GetPetTab()	{ return m_pPetTabButton; }

};