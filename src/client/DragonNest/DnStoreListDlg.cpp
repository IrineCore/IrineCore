#include "StdAfx.h"
#include "DnStoreListDlg.h"
#include "DnStoreSlotDlg.h"
#include "DnInven.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif 

CDnStoreListDlg::CDnStoreListDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
	: CEtUITabDialog( dialogType, pParentDialog, nID, pCallback )
	, m_StoreListDlgType(eNormal)
{
}

CDnStoreListDlg::~CDnStoreListDlg(void)
{
}

void CDnStoreListDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "StoreDlg.ui" ).c_str(), bShow );
}

void CDnStoreListDlg::InitialUpdate()
{
	CDnStoreSlotDlg *pStoreSlotDlg(NULL);
	CEtUIRadioButton *pTabButton(NULL);

	pTabButton = GetControl<CEtUIRadioButton>("ID_TAB_BASIC01");
	pStoreSlotDlg = new CDnStoreSlotDlg( UI_TYPE_CHILD, this, PAGE_01 );
	pStoreSlotDlg->Initialize( false );
	AddTabDialog( pTabButton, pStoreSlotDlg );
	m_vecTabButton.push_back(pTabButton);

	pTabButton = GetControl<CEtUIRadioButton>("ID_TAB_BASIC02");
	pStoreSlotDlg = new CDnStoreSlotDlg( UI_TYPE_CHILD, this, PAGE_02 );
	pStoreSlotDlg->Initialize( false );
	AddTabDialog( pTabButton, pStoreSlotDlg );
	m_vecTabButton.push_back(pTabButton);

	pTabButton = GetControl<CEtUIRadioButton>("ID_TAB_BASIC03");
	pStoreSlotDlg = new CDnStoreSlotDlg( UI_TYPE_CHILD, this, PAGE_03 );
	pStoreSlotDlg->Initialize( false );
	AddTabDialog( pTabButton, pStoreSlotDlg );
	m_vecTabButton.push_back(pTabButton);

	pTabButton = GetControl<CEtUIRadioButton>("ID_TAB_BASIC04");
	pStoreSlotDlg = new CDnStoreSlotDlg( UI_TYPE_CHILD, this, PAGE_04 );
	pStoreSlotDlg->Initialize( false );
	AddTabDialog( pTabButton, pStoreSlotDlg );
	m_vecTabButton.push_back(pTabButton);

	pTabButton = GetControl<CEtUIRadioButton>("ID_TAB_BASIC05");
	pStoreSlotDlg = new CDnStoreSlotDlg( UI_TYPE_CHILD, this, PAGE_05 );
	pStoreSlotDlg->Initialize( false );
	AddTabDialog( pTabButton, pStoreSlotDlg );
	m_vecTabButton.push_back(pTabButton);

	SetCheckedTab( m_vecTabButton[PAGE_01]->GetTabID() );
}

void CDnStoreListDlg::Show( bool bShow ) 
{
	if( m_bShow == bShow )
		return;

	if( bShow )
	{
		CDnStoreSlotDlg *pStoreSlotDlg(NULL);
		for( int i=0; i<m_groupTabDialog.GetDialogCount(); ++i )
		{
			pStoreSlotDlg = static_cast<CDnStoreSlotDlg*>(m_groupTabDialog.GetDialogFromIndex(i));
			if( !pStoreSlotDlg->IsEmptySlotDialog() )
			{
				m_vecTabButton[i]->Enable( true );
			}
		}

		SetCheckedTab( m_vecTabButton[PAGE_01]->GetTabID() );
	}
	else
	{
		for( int i=0; i<(int)m_vecTabButton.size(); i++ )
		{
			m_vecTabButton[i]->Enable( false );
		}
	}

	CEtUITabDialog::Show( bShow );
	//CDnMouseCursor::GetInstance().ShowCursor( bShow );
}

void CDnStoreListDlg::SetItem( MIInventoryItem *pItem )
{
	ASSERT(pItem&&"CDnStoreDlg::SetItem");
	if( !pItem ) return;

	int nStoreSlotIndex = pItem->GetSlotIndex() / emINVEN::ITEM_MAX;
	CDnStoreSlotDlg *pStoreSlotDlg(NULL);
	for( int i=0; i<m_groupTabDialog.GetDialogCount(); ++i )
	{
		if( i == nStoreSlotIndex ) {
			pStoreSlotDlg = static_cast<CDnStoreSlotDlg*>(m_groupTabDialog.GetDialogFromIndex(i));
			pStoreSlotDlg->SetItem( pItem );
			return;
		}
	}

	CDebugSet::ToLogFile( "CDnStoreListDlg::SetItem, no empty slot. This shouldn't happen!" );
}

void CDnStoreListDlg::ResetAllSlot()
{
	CDnStoreSlotDlg *pStoreSlotDlg(NULL);

	for( int i=0; i<m_groupTabDialog.GetDialogCount(); ++i )
	{
		pStoreSlotDlg = static_cast<CDnStoreSlotDlg*>(m_groupTabDialog.GetDialogFromIndex(i));
		pStoreSlotDlg->ResetAllSlot();
	}
}

bool CDnStoreListDlg::IsEmptyListDialog()
{
	CDnStoreSlotDlg *pStoreSlotDlg(NULL);
	for( int i=0; i<m_groupTabDialog.GetDialogCount(); ++i )
	{
		pStoreSlotDlg = static_cast<CDnStoreSlotDlg*>(m_groupTabDialog.GetDialogFromIndex(i));
		if( !pStoreSlotDlg->IsEmptySlotDialog() )
		{
			return false;
		}
	}

	return true;
}

void CDnStoreListDlg::ResetSlot(BYTE cSlotIndex)
{
	CDnStoreSlotDlg *pStoreSlotDlg(NULL);
	for( int i=0; i<m_groupTabDialog.GetDialogCount(); ++i )
	{
		pStoreSlotDlg = static_cast<CDnStoreSlotDlg*>(m_groupTabDialog.GetDialogFromIndex(i));
		if (pStoreSlotDlg && !pStoreSlotDlg->IsEmptySlotDialog())
		{
			pStoreSlotDlg->ResetSlot(cSlotIndex);
			pStoreSlotDlg->ResetRepurchaseSlot(cSlotIndex);
		}
	}
}
