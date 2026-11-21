#include "StdAfx.h"
#include "DnItemDisjointExpectItemInfoDlg.h"
#include "DnItemTask.h"
#include "DnItem.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnItemDisjointExpectItemInfoDlg::CDnItemDisjointExpectItemInfoDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
: CDnCustomDlg( dialogType, pParentDialog, nID, pCallback )
{
	memset( m_pExpectItems, 0, sizeof(m_pExpectItems) );
	memset( m_pSlotButtons, 0, sizeof(m_pSlotButtons) );
}

CDnItemDisjointExpectItemInfoDlg::~CDnItemDisjointExpectItemInfoDlg()
{
	ClearItems();
}

void CDnItemDisjointExpectItemInfoDlg::ClearItems()
{
	for( int i = 0; i < EXPECT_SLOT_PER_ROW; ++i )
	{
		SAFE_DELETE( m_pExpectItems[i] );
		if( m_pSlotButtons[i] )
		{
			m_pSlotButtons[i]->ResetSlot();
			m_pSlotButtons[i]->Show( false );
		}
	}
}

void CDnItemDisjointExpectItemInfoDlg::Initialize( bool bShow )
{
	CDnCustomDlg::Initialize( CEtResourceMng::GetInstance().GetFullName( "ItemDisjointSlotListDlg.ui" ).c_str(), bShow );
}

void CDnItemDisjointExpectItemInfoDlg::InitialUpdate()
{
	char szControlName[32] = {0,};
	for( int i = 0; i < EXPECT_SLOT_PER_ROW; ++i )
	{
		sprintf_s( szControlName, _countof(szControlName), "ID_BT_ITEM%d", i );
		m_pSlotButtons[i] = GetControl<CDnItemSlotButton>( szControlName );
		if( m_pSlotButtons[i] )
		{
			m_pSlotButtons[i]->Show( false );
			m_pSlotButtons[i]->SetSlotType( ST_ITEM_DISJOINT );
		}
	}
}

void CDnItemDisjointExpectItemInfoDlg::SetExpectItems( const std::vector<int> &vecItemIDs, int nStartIndex )
{
	ClearItems();

	if( !CDnItemTask::IsActive() )
		return;

	int nTotal = (int)vecItemIDs.size();
	for( int i = 0; i < EXPECT_SLOT_PER_ROW; ++i )
	{
		int nIdx = nStartIndex + i;
		if( nIdx >= nTotal || !m_pSlotButtons[i] )
			break;

		TItemInfo itemInfo;
		if( CDnItem::MakeItemInfo( vecItemIDs[nIdx], 1, itemInfo ) )
		{
			m_pExpectItems[i] = GetItemTask().CreateItem( itemInfo );
			if( m_pExpectItems[i] )
			{
				m_pSlotButtons[i]->SetItem( m_pExpectItems[i], 1 );
				m_pSlotButtons[i]->Show( true );
			}
		}
	}
}
