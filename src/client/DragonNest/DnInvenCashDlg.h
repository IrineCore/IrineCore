#pragma once

#include "DnCustomControlCommon.h"

class MIInventoryItem;
class CDnInvenSlotDlg;

class CDnInvenCashDlg : public CEtUIDialog
{
public:
	void SetItem( MIInventoryItem *pItem );
	void ResetSlot( int nSlotIndex );
	void SetSlotType( ITEM_SLOT_TYPE slotType );

	void ReleaseNewGain();
	int GetRegisteredItemCount();

	std::vector<CDnInvenSlotDlg *>& GetInvenSlotDlgList() { return m_pVecInvenSlotDlg; }

protected:
	enum
	{
		NUM_PAGE_PER_GROUP = 5,
	};

	CEtUIButton *m_pButtonPageGroupPrev;
	CEtUIButton *m_pButtonPageGroupNext;
	CEtUIButton *m_pButtonPagePrev;
	CEtUIButton *m_pButtonPageNext;
	CEtUIButton *m_pButtonPage[NUM_PAGE_PER_GROUP];

	std::vector<CDnInvenSlotDlg *> m_pVecInvenSlotDlg;

#ifdef PRE_ADD_CASHINVENTAB
	
	enum ETABTYPE
	{		
		ETABTYPE_COSTUME = 0,   // Costume.
		ETABTYPE_CONSUMABLES,   // Consumable.
		ETABTYPE_FUNCTIONALITY, // Functional.
		ETABTYPE_ETC,			// Etc.
		ETABTYPE_MAX
	};

	// Classify by type.
	struct STabInfo
	{
		std::vector<CDnInvenSlotDlg *> vecInvenSlotDlg;
		int crrPage;
		int maxItem;
		STabInfo():crrPage(0),maxItem(0){}		
		~STabInfo();
	};
	std::vector< STabInfo * > m_pVecTypeInvenSlotDlg;		
	int m_crrTabType; // Current tab type.

	std::map< int, int > m_mapType; // map< ItemType, TabType >

	int m_newItemTabIndex; // Tab index where the item was added.


	class CDnInvenCashTabDlg * m_pInvenCashTabDlg; // CashTab.

#endif // PRE_ADD_CASHINVENTAB

	int m_nCurPage;
	int m_nMaxPage;

#ifdef PRE_ADD_CASHINVENTAB
	void CheckSlotIndex( int nSlotIndex, bool bInsert, MIInventoryItem *pItem );
#else
	void CheckSlotIndex( int nSlotIndex, bool bInsert );
#endif // PRE_ADD_CASHINVENTAB

	void RefreshPageControl( bool bCheckMaxPage = false );
	void RefreshCashInvenSlot();

	void NextPage();
	void PrevPage();
	void NextGroupPage();
	void PrevGroupPage();


#ifdef PRE_ADD_CASHINVENTAB

	void RefreshCashInvenTypeSlot();	
	int GetCrrMaxPage();

#endif // PRE_ADD_CASHINVENTAB



public:
	CDnInvenCashDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnInvenCashDlg(void);

	virtual void Initialize( bool bShow );
	virtual void InitialUpdate();
	virtual void Show( bool bShow );
	virtual void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg = 0 );
	virtual bool MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );

	virtual void MoveDialog( float fX, float fY );

#ifdef PRE_ADD_LEVELUP_REWARD_NOTIFIER
	void SetFocusBlink( int nSlotIndex, int nBlinkTime, int nEndTime );
	void RefreshInvenPageFromSlotIndex( int nSlotIndex );
#endif

#ifdef PRE_ADD_CASHINVENTAB

	void ChangeTab( int idx );

	//  Display New mark on the tab that received the item.
	void AddNewItem();

	void SetAllignTypeCashTab( UIAllignHoriType Hori, UIAllignVertType Vert );

	class CDnItem * GetCashInvenItemBySN( INT64 SN );

#endif // PRE_ADD_CASHINVENTAB

};
