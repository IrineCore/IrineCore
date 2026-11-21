#include "Stdafx.h"

#ifdef PRE_ADD_BESTFRIEND

#include "DnItem.h"
#include "DnItemTask.h"

#include "DnTableDB.h"
#include "DNTableFile.h"

#include "DnInterface.h"

#include "DnBestFriendRewardDlg.h"


#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif


CDnBestFriendRewardDlg::CDnBestFriendRewardDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback, bool bAutoCursor )
 : CDnCustomDlg( dialogType, pParentDialog, nID, pCallback, bAutoCursor )
 ,m_bFirst(true)
 ,m_selectionIndex(-1)
 ,m_giftSerial(-1)
 ,m_itemID(-1)
 ,m_typeParam1(-1)
{

}

void CDnBestFriendRewardDlg::ReleaseDlg()
{
	int size = (int)m_vecItems.size();
	for( int i=0; i<size; ++i )
		SAFE_DELETE( m_vecItems[ i ] );
	m_vecItems.clear();
}


void CDnBestFriendRewardDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName("BFItemDlg.ui").c_str(), bShow );
}


void CDnBestFriendRewardDlg::InitialUpdate()
{
	const int size = 5;
	m_vecItems.resize( size );
	char str[32] = {0,};
	for( int i=0; i<size; ++i )
	{
		SItem * pItem = new SItem;
		
		sprintf_s( str, "ID_TEXT_ITEM%d", i );
		pItem->pStatic = GetControl<CEtUIStatic>( str );

		sprintf_s( str, "ID_STATIC_SELECT%d", i );
		pItem->pStaticSelect = GetControl<CEtUIStatic>( str );
		pItem->pStaticSelect->Show( false );

		sprintf_s( str, "ID_BT_ITEM%d", i );
		pItem->pItemSlot = GetControl<CDnItemSlotButton>( str );

		m_vecItems[ i ] = pItem;
	}
}


void CDnBestFriendRewardDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );

	if( nCommand == EVENT_BUTTON_CLICKED )
	{
		if( strstr( pControl->GetControlName(), "ID_BT_ITEM" ) )			
		{		
			std::string str = GetCmdControlName();
			std::string strIdx = str.substr( str.size()-1, str.size() );
			int idx = atoi( strIdx.c_str() );

			if( idx < (int)m_vecItems.size() )
			{
				m_selectionIndex = idx;

				for( int i=0; i<(int)m_vecItems.size(); ++i )
				{
					if( i == idx )
						m_vecItems[ i ]->pStaticSelect->Show( true );
					else
						m_vecItems[ i ]->pStaticSelect->Show( false );
				}
			}
		}

		else if( GetCmdControlName() == "ID_BT_OK" )
		{
			if( m_selectionIndex != -1 && m_giftSerial != -1 )
			{	
				m_itemID = m_vecItems[m_selectionIndex]->pItemSlot->GetItemID();
				GetItemTask().RequestRewardItemBF( m_giftSerial, m_itemID );
				Show( false );
			}
		}

		else if( GetCmdControlName() == "ID_BT_CANCEL" ||
				 GetCmdControlName() == "ID_CANCEL" )
		{
			Show( false );
		}
	}


	CEtUIDialog::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}


void CDnBestFriendRewardDlg::Show( bool bShow )
{
	if( m_bShow == bShow )
		return;

	CEtUIDialog::Show( bShow );

	if( bShow )
	{
		if( m_bFirst )
		{
			m_bFirst = false;
			
			int idx = 0;
			DNTableFileFormat * pTable = GetDNTable( CDnTableDB::TableEnum::TITEM );
			if( pTable )
			{
				int size = pTable->GetItemCount();
				for( int i=0; i<size; ++i )
				{
					int itemID = pTable->GetItemID( i );
					DNTableCell * pCell = pTable->GetFieldFromLablePtr( itemID, "_Type" );
					if( pCell && eItemTypeEnum::ITEMTYPE_MENUBOX == pCell->GetInteger() )
					{	
						pCell = pTable->GetFieldFromLablePtr( itemID, "_TypeParam1" );		
						if( pCell && pCell->GetInteger() == m_typeParam1 )
						{
							CDnItem* pItem = NULL;
							TItemInfo itemInfo;
							if( CDnItem::MakeItemInfo( itemID, 1, itemInfo ) )						
								pItem = GetItemTask().CreateItem( itemInfo );

							if( pItem )
							{							
								m_vecItems[ idx ]->pItemSlot->SetItem( pItem, 1 );							
								m_vecItems[ idx ]->pStatic->SetText( pItem->GetName() );
								++idx;

								if( idx >= (int)m_vecItems.size() )
									break;
							}
						}
					}
				}
			}
		}// First.

	}// Show

	else
	{
		// Deselect.
		for( int i=0; i<(int)m_vecItems.size(); ++i )
				m_vecItems[ i ]->pStaticSelect->Show( false );
	}
}



// Reward item grant complete.
void CDnBestFriendRewardDlg::SendRewardComplet( int nItemID )
{
	m_selectionIndex = m_itemID = -1;
	m_giftSerial = -1;

	if( nItemID > 0 )
	{
		CDnItem * pItem = CDnItem::CreateItem( nItemID, 0 );
		if( pItem )
		{
			const wchar_t * strBF = GetInterface().GetNameBF();
			if( strBF )
			{
				wchar_t strName[256] = {0,};
				swprintf_s( strName, 256, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 1000043811 ), strBF ); // "Best Friend Ring with %s"
				GetInterface().ShowItemAlarmDialog( strName, pItem, 1, textcolor::WHITE, 3.0f );
				SAFE_DELETE( pItem );
			}
		}
	}
}


#endif // PRE_ADD_BESTFRIEND