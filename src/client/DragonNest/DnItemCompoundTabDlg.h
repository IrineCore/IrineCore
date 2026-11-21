#pragma once
#include "DnCustomDlg.h"
#include "DnItemTask.h"
#include "DnWeapon.h"
#include "DnItemTask.h"
#include "DnInterfaceString.h"

class CDnItem;
class CDnItemSlotButton;

class CDnItemCompoundPopupDlg;
class CDnItemCompoundMixDlg;

// Although it has tabs, each tab is not a separate dialog, so it does not inherit from TabDialog.
class CDnItemCompoundTabDlg : public CDnCustomDlg, public CEtUICallback
{
	struct SCompoundListItem
	{
		int m_nCompoundID;
		CDnItem *m_pItem;
		CDnItemSlotButton *m_pItemSlotButton;
		CEtUIStatic *m_pStaticBase;
		CEtUIStatic *m_pStaticName;
		CEtUIStatic *m_pStaticLevelText;	// Text displaying level
		CEtUIStatic *m_pStaticLevel;
		CEtUIStatic *m_pStaticType;

		SCompoundListItem()
			: m_pItem(NULL)
			, m_pItemSlotButton(NULL)
			, m_pStaticBase(NULL)
			, m_pStaticName(NULL)
			, m_pStaticLevelText(NULL)
			, m_pStaticLevel(NULL)
			, m_pStaticType(NULL)
			, m_nCompoundID(0)
		{
		}

		~SCompoundListItem()
		{
			SAFE_DELETE( m_pItem );
		}

		void SetInfo( int nCompoundID, int nItemID )
		{
			m_nCompoundID = nCompoundID;

			if( nItemID > 0 )
			{
				SAFE_DELETE( m_pItem );

				TItemInfo itemInfo;
				if( CDnItem::MakeItemInfo( nItemID, 1, itemInfo ) == false ) return;
				m_pItem = GetItemTask().CreateItem( itemInfo );

				// Reset durability. Since max value is unknown above, set to max value here like this.
				if( m_pItem->GetItemType() == ITEMTYPE_WEAPON )
				{
					CDnWeapon *pWeapon = dynamic_cast<CDnWeapon *>(m_pItem);
					pWeapon->SetDurability( pWeapon->GetMaxDurability() );
				}
				else if( m_pItem->GetItemType() == ITEMTYPE_PARTS )
				{
					CDnParts *pParts = dynamic_cast<CDnParts *>(m_pItem);
					pParts->SetDurability( pParts->GetMaxDurability() );
				}
				m_pItemSlotButton->SetItem(m_pItem, CDnSlotButton::ITEM_ORIGINAL_COUNT);

				m_pStaticBase->Show( true );
				m_pStaticName->SetText( m_pItem->GetName() );
				m_pStaticLevelText->Show( true );
				m_pStaticLevel->SetIntToText( m_pItem->GetLevelLimit() );

				// Item sub-type. Set only when a sub-type exists.
				{
					WCHAR szTemp[64];
					if( m_pItem->GetItemType() == ITEMTYPE_WEAPON )
					{
						CDnWeapon *pWeapon = dynamic_cast<CDnWeapon *>(m_pItem);
						if( pWeapon )
							swprintf_s( szTemp, _countof(szTemp), L"%s : %s", GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7009 ), DN_INTERFACE::STRING::ITEM::GetEquipString( pWeapon->GetEquipType() ) );
						m_pStaticType->SetText( szTemp );
					}
					else if( m_pItem->GetItemType() == ITEMTYPE_PARTS )
					{
						CDnParts *pParts = dynamic_cast<CDnParts *>(m_pItem);
						if( pParts )
							swprintf_s( szTemp, _countof(szTemp), L"%s : %s", GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7009 ), DN_INTERFACE::STRING::ITEM::GetPartsString( pParts->GetPartsType() ) );
						m_pStaticType->SetText( szTemp );
					}
				}
			}
		}

		void Clear()
		{
			m_nCompoundID = 0;
			SAFE_DELETE( m_pItem );
			m_pItemSlotButton->ResetSlot();
			m_pStaticBase->Show( false );
			m_pStaticName->SetText(L"");
			m_pStaticLevelText->Show( false );
			m_pStaticLevel->SetText(L"");
			m_pStaticType->SetText(L"");
		}

		bool IsInsideItem( float fX, float fY )
		{
			SUICoord uiCoordsBase;
			m_pStaticBase->GetUICoord(uiCoordsBase);
			return uiCoordsBase.IsInside(fX, fY);
		}
	};

	enum
	{
		STUFF_VIEW_DIALOG,
		COMPOUND_CONFIRM_DIALOG,
		COMPOUND_MOVIE_DIALOG,
		NUM_MAX_COMPOUND_ITEM = 6,
		NUM_NEED_ITEM = 5,
	};

public:
	CDnItemCompoundTabDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnItemCompoundTabDlg(void);

protected:
	CEtUIRadioButton *m_pTabButtonWeapon;
	CEtUIRadioButton *m_pTabButtonParts;
	CEtUIRadioButton *m_pTabButtonAccessory;
	CEtUIRadioButton *m_pTabButtonNormal;

	// Does not inherit from tab dialog so cannot use GetCurrentTabID; store it directly.
	int m_nCurTab;

	CEtUIComboBox *m_pComboBoxJob;
	CEtUIComboBox *m_pComboBoxType;

	CEtUIButton *m_pButtonOK;
	CEtUIButton *m_pButtonCancel;
	bool m_bOK;

	SCompoundListItem m_sCompoundItem[NUM_MAX_COMPOUND_ITEM];
	CEtUIStatic *m_pStaticSelect;
	int m_nCurSelect;

	CEtUIButton *m_pButtonPagePrev;
	CEtUIButton *m_pButtonPageNext;
	CEtUIStatic *m_pStaticPage;
	int m_nCurPage;
	int m_nMaxPage;

	// Data related
	int m_nCurCompoundListID;
	std::vector<int> m_vecCompoundID;

	// Vector holding only the items to display in the current list
	std::vector<int> m_vecCompoundIDinList;

	CDnItemCompoundMixDlg *m_pCompoundMixDlg;

	// Material item slot
	CDnItem *m_pNeedItem[NUM_NEED_ITEM];
	CDnItemSlotButton *m_pNeedItemSlotButton[NUM_NEED_ITEM];

	CEtUIStatic *m_pGold;
	CEtUIStatic *m_pSilver;
	CEtUIStatic *m_pBronze;
	DWORD m_dwColorGold;
	DWORD m_dwColorSilver;
	DWORD m_dwColorBronze;

	// Set material slot
	void SetNeedItemSlot( int nSlotIndex, int nItemID, int nNeedItemCount, int nCurItemCount );

	int FindInsideItem( float fX, float fY );
	void UpdateSelectBar();

	void ResetList();

	void CheckUsableTab();

	void UpdateComboBoxType();
	void UpdateCompoundList();
	void UpdatePage();
	void UpdateCompoundSlot();
	void UpdateCompoundableSlot();

	void UpdateList();

	void PrevPage();
	void NextPage();

	void ShowConfirmDlg( int nCompoundID, bool bOK );

	// Check if combination is possible.
	void CheckCompound();
	bool CheckCompound( int nCompoundID );

	// Required item struct to send to server
	vector<CDnItemTask::ItemCompoundInfo> m_vNeedItemInfo;
public:
	// Get NPC's CompountItemShopID and set up the craft list.
	void SetCompoundListID( int nListID );


	void OnRecvItemCompoundOpen();
	void OnRecvItemCompound();

public:
	virtual void InitialUpdate();
	virtual void Initialize( bool bShow );
	virtual void Show( bool bShow );
	virtual void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg = 0 );
	virtual bool MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
	virtual void OnUICallbackProc( int nID, UINT nCommand, CEtUIControl *pControl, UINT uiMsg = 0 );
};