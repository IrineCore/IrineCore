#pragma once
#include "DnCustomDlg.h"
#include "DnSkill.h"
#include "DnActor.h"
#include "DnTradeTask.h"
#include "DnInterfaceString.h"
#include "DnItemTask.h"
#include "DnSkillTask.h"

class CDnSkillBuyDlg;

class CDnSkillStoreDlg : public CDnCustomDlg, public CEtUICallback
{
	struct SSkillStoreSlot
	{
		CDnSkillSlotButton *m_pSkillButton;

		CDnItemSlotButton *m_pItemButton;
		CDnItem *m_pNeedItem;
		CEtUIStatic *m_pNeedItemCount;

		CEtUIStatic *m_pSkillBase;
		CEtUIStatic *m_pSkillName;
		CEtUIStatic *m_pSkillLevel;
		CEtUIStatic *m_pSkillMoney;
		CEtUIStatic *m_pSkillSP;

		int m_nSlotIndex; // Unique index of shop skill (used in CDnTradeSkill)
		bool m_bEnable;

		SSkillStoreSlot()
			: m_pSkillBase(NULL)
			, m_pSkillButton(NULL)
			, m_pItemButton(NULL)
			, m_pNeedItem(NULL)
			, m_pNeedItemCount(NULL)
			, m_pSkillName(NULL)
			, m_pSkillLevel(NULL)
			, m_pSkillMoney(NULL)
			, m_bEnable(false)
			, m_nSlotIndex(-1)
			, m_pSkillSP(NULL)
		{
		}

		~SSkillStoreSlot()
		{
			SAFE_DELETE( m_pNeedItem );
		}

		void InitSlot()
		{
			m_pSkillBase->SetTextureColor( 0x4CFFFFFF, 0 );

			m_pSkillButton->ResetSlot();
			m_pItemButton->ResetSlot();
			m_pSkillName->ClearText();
			m_pNeedItemCount->ClearText();
			m_pSkillLevel->ClearText();
			m_pSkillMoney->ClearText();
			m_pSkillSP->ClearText();
			m_bEnable = true;
			m_nSlotIndex = -1;
		};

		bool IsInside( float fX, float fY )
		{
			SUICoord uiCoords, uiCoorde;

			m_pSkillButton->GetUICoord(uiCoords);
			m_pSkillLevel->GetUICoord(uiCoorde);
			uiCoords.fWidth = uiCoorde.Right() - uiCoords.fX;

			return uiCoords.IsInside( fX, fY );
		}

		void SetSlot( int nSlotIndex, CDnSkill *pSkillItem )
		{
		}
	};

	enum
	{
		ITEM_SIZE_X = 1,
		ITEM_SIZE_Y = 6,
		ITEM_SIZE = 6,

		SKILL_BUY_DIALOG,
	};

public:
	CDnSkillStoreDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallBack = NULL );
	virtual ~CDnSkillStoreDlg(void);

protected:

	struct SSkillItemInfo
	{
		int nSlotIndex;		// Note : Actual index held in the task
		CDnSkill *pSkill;

		SSkillItemInfo()
			: nSlotIndex(-1)
			, pSkill(NULL)
		{
		}
	};

// 	std::vector<SSkillStoreSlot> m_vecSkillSlot;
// 	std::vector<SSkillItemInfo> m_vecSkillItem;
// 
// 	CEtUIStatic *m_pSkillPage;
// 	CEtUIButton *m_pButtonPagePrev;
// 	CEtUIButton *m_pButtonPageNext;
// 	CEtUIStatic *m_pSelectBar;
// 	CEtUIButton *m_pButtonBuy;
// 
// 	int m_nMaxPage;
// 	int m_nCurrentPage;
// 	int m_nSkillSlotIndex;

	// CDnSkillBuyDlg *m_pSkillBuyDlg;

protected:
	void InitSkillInfo();
	void InitSkillSlotList();

	void UpdateSkillPage();
	void UpdateMaxPageNum();
	void UpdatePageButton();
	void UpdateSelectBar( int nSlotIndex );

	void NextPage();
	void PrevPage();

	int FindInsideItem( float fX, float fY );

public:
	void SetItem( int nSlotIndex, MIInventoryItem *pItem ) { return; }
	void ResetAllItem() { return; }

	void RefreshDialog() { return; }
	void UpdateSkillISlot() { return; }

	bool IsEmptySlotDialog() { return false; }

public:
	virtual void Initialize(bool bShow);
//	virtual void InitialUpdate();
//	virtual void InitCustomControl( CEtUIControl *pControl );
// 	virtual void Show( bool bShow );
// 	virtual void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg = 0 );
// 	virtual bool MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
// 	virtual void OnUICallbackProc( int nID, UINT nCommand, CEtUIControl *pControl, UINT uiMsg = 0 );
};
