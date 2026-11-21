#pragma once
#include "DnCustomDlg.h"
#include "DnItemTask.h"

class CDnItem;

class CDnPlateListDlg : public CDnCustomDlg
{
	struct SPlateListItem
	{
		CDnItem *m_pItem;
		CDnItemSlotButton *m_pItemSlotButton;
		CEtUIStatic *m_pStaticName;
		CEtUIStatic *m_pStaticBase;

		SPlateListItem()
			: m_pItem(NULL)
			, m_pItemSlotButton(NULL)
			, m_pStaticName(NULL)
			, m_pStaticBase(NULL)
		{
		}

		~SPlateListItem()
		{
			SAFE_DELETE( m_pItem );
		}

		void SetInfo( int nItemID )
		{
			if( nItemID > 0 )
			{
				SAFE_DELETE( m_pItem );

				TItemInfo itemInfo;
				if( CDnItem::MakeItemInfo( nItemID, 1, itemInfo ) == false ) return;
				m_pItem = GetItemTask().CreateItem( itemInfo );
				ASSERT(m_pItem&&"Plate crafting result item ID is invalid.");
				m_pItemSlotButton->SetItem(m_pItem, CDnSlotButton::ITEM_ORIGINAL_COUNT);

				m_pStaticName->SetText( m_pItem->GetName() );
				m_pStaticBase->Show( true );
			}
			else
			{
				// Display random item
				SAFE_DELETE( m_pItem );
				m_pStaticName->SetText( L"???" );
				m_pStaticBase->Show( true );
			}
		}

		void Clear()
		{
			SAFE_DELETE( m_pItem );
			m_pItemSlotButton->ResetSlot();
			m_pStaticName->SetText(L"");
			m_pStaticBase->Show( false );
		}

		bool IsInsideItem( float fX, float fY )
		{
			SUICoord uiCoords;
			m_pStaticBase->GetUICoord(uiCoords);
			return uiCoords.IsInside(fX, fY);
		}
	};

public:
	CDnPlateListDlg( UI_DIALOG_TYPE dialogType = UI_TYPE_FOCUS, CEtUIDialog *pParentDialog = NULL, int nID = -1, CEtUICallback *pCallback = NULL );
	virtual ~CDnPlateListDlg(void);

protected:

	enum{
		MAX_PLATE_NUM = 4
	};

	CEtUIStatic *m_pStaticSelect;
	SPlateListItem m_sPlateItem[MAX_PLATE_NUM];
	int m_nCurSelect;

	int FindInsideItem( float fX, float fY );
	void UpdateSelectBar();

public:
	void RefreshPlateList();
	int GetSelectedCompoundID();

public:
	virtual void Initialize( bool bShow );
	virtual void InitialUpdate();
	virtual void Show( bool bShow );
	virtual void ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg = 0 );
	virtual bool MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
};
