#pragma once
#include "DnCustomDlg.h"
#include "DnDataManager.h"
#include "DnItem.h"

class CDnNpcQuestRecompenseDlg : public CDnCustomDlg
{
	struct SRecompenseItemSlot
	{
		CDnItemSlotButton *pItemButton;
		CEtUIStatic *pStaticSelected;

		SRecompenseItemSlot()
			: pItemButton(NULL)
			, pStaticSelected(NULL)
		{
		}

		void ResetSlot()
		{
			CDnItem *pItem = (CDnItem*)pItemButton->GetItem();
			SAFE_DELETE( pItem );
			pItemButton->ResetSlot();
			pStaticSelected->Show( false );
		}

		void SetSlot( CDnItem *pItem )
		{
			pItemButton->SetItem(pItem, CDnSlotButton::ITEM_ORIGINAL_COUNT);
			pItemButton->Show( true );
		}

		CDnItem* GetSlot()
		{
			if ( pItemButton )
				return static_cast<CDnItem*>(pItemButton->GetItem());

			return NULL;
		}

		void SetCheck( bool bCheck )
		{
			pStaticSelected->Show( bCheck );
		}

		bool IsCheckSlot()
		{
			return pStaticSelected->IsShow();
		}
	};

	enum
	{
		RECOMPENSE_ITEMSLOT_SIZE = 6,
	};

public:
	CDnNpcQuestRecompenseDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL  );
	virtual ~CDnNpcQuestRecompenseDlg(void);

protected:
	DWORD m_nSetItemCount;
	DWORD m_nSetAddItemCount;

	CEtUIStatic *m_pStaticComment;
	CEtUIStatic *m_pMoneyGold;
	CEtUIStatic *m_pMoneySilver;
	CEtUIStatic *m_pMoneyBronze;
	CEtUIStatic *m_pMoneyExp;
	CEtUIStatic *m_pMoneyExpBack;
	CEtUIStatic *m_pMoneyBack;

	CEtUIStatic *m_pItemBack;

	std::vector<std::pair< SUICoord, CEtUIStatic* > >  m_vecMoneyAndExp;

	std::vector<SRecompenseItemSlot> m_vecRecompenseItemInfo;

	CEtUIStatic * m_pStaticAdd;
	CEtUIStatic * m_pStaticAddBoard;
	CEtUIStatic * m_pStaticOnlyAdd;
	CEtUIStatic * m_pStaticOnlyAddBoard;

	std::vector<SRecompenseItemSlot> m_vecAddRecompenseItemInfo;
	std::vector<SRecompenseItemSlot> m_vecOnlyAddRecompenseItemInfo;

	CEtUIStatic *m_pBoardFTG;
	CEtUIStatic *m_pTextFTG;

	int m_nTypeRecompense;		// Note : 1:All rewards, 2:Selectable reward
	int m_nAvailableCount;		// Note : Selectable count
	bool m_bRecompense;			// Note : Check reward
	int m_nRecompenseID;

	SUICoord m_ExpCoord;
	SUICoord m_ExpBackCoord;

	int m_nRewardCashItemStartSlotIndex;	// Note : Starting index for cash item rewards among the 6 slots

protected:
	void ResetAllData();
	void ResetSlotCheck();
	int GetSlotCheckCount();

	void AddRecompenseItemSlot( const char * szButtonName, const char * szStaticName, const int nSize, std::vector<SRecompenseItemSlot> & vecRecompenseItemInfo );

	void SetPositionCoinUI();
	void SetCoinText( const UINT uiCoin );
	void SetExpText( const int nExp );
	void SetCommentText( const UINT nStringIndex );

	void SetItemOption( const int nTableID, const int nCount, OUT TItemInfo & itemInfo );
	bool GetNormalItemInfo( const int nItemID, const int nItemCount, OUT TItemInfo & itemInfo );
	CDnItem * CreateNormalItem( TItemInfo & itemInfo );

	bool GetCashItemInfo( const int nCashItemSN, OUT TItemInfo & itemInfo );
	CDnItem * CreateCashItem( const int nCashItemSN, TItemInfo & itemInfo );

	bool SetItemSlot( CDnItem * pItem );

	void InitAddRecompenseUI();
	bool SetAddItemSlot( CDnItem * pItem, std::vector<SRecompenseItemSlot> & vecItemSlotButton );
	void SetFTGText( const int nFTG );

public:
	void SetRecompense( const TQuestRecompense &questRecompense, const bool bRecompense );
	void SetLevelCapRecompense( const TQuestRecompense & questRecompese, const TQuestLevelCapRecompense & questLevelCapRecompense );

	int GetRecompenseID() { return m_nRecompenseID; }
	bool IsCompleteCheck();
	bool IsRecompense() { return m_bRecompense; }
	int GetAvailableCount() { return m_nAvailableCount; }
	void GetRecompenseItemArray( bool itemArray[], bool cashitemArray[] );

public:
	virtual void Initialize(bool bShow);
	virtual void InitialUpdate();
	virtual void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg = 0 );
	virtual void Show( bool bShow );

#ifdef PRE_ADD_QUEST_BACK_DIALOG_BUTTON
	void ResetRecompenseDlg() { ResetAllData(); this->Show(false); }
#endif
};
