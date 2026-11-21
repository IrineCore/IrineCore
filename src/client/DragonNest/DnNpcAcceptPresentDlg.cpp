
#include "StdAfx.h"
#include "DnNpcAcceptPresentDlg.h"
#include "TaskManager.h"
#include "DnItemTask.h"
#include "DnInterface.h"
#include "DnCashShopTask.h"
#include "DnTableDB.h"

#ifdef PRE_ADD_NPC_REPUTATION_SYSTEM

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

const int REPUTE_PRESENT_MONEY_ITEM_ID = 400194;

CDnNpcAcceptPresentDlg::CDnNpcAcceptPresentDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback  )
: CDnCustomDlg( dialogType, pParentDialog, nID, pCallback  )
, m_pMoneyGold(NULL)
, m_pMoneySilver(NULL)
, m_pStaticComment(NULL)
, m_iAvailableCount( 0 ) 
, m_iSelectedSlotIndex( -1 )
, m_pItemBG(NULL)
, m_pMoneyBack(NULL)
, m_pMoneyBronze(NULL)
, m_pMoneyExp(NULL)
, m_pMoneyExpBack(NULL)
{
}

CDnNpcAcceptPresentDlg::~CDnNpcAcceptPresentDlg(void)
{
	DeleteAllItem();
}

void CDnNpcAcceptPresentDlg::Initialize(bool bShow)
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "NpcQuestRecompenseDlg.ui" ).c_str(), bShow );
}

void CDnNpcAcceptPresentDlg::InitialUpdate()
{
	m_pMoneyGold = GetControl<CEtUIStatic>("ID_MONEY_GOLD");
	m_pMoneySilver = GetControl<CEtUIStatic>("ID_MONEY_SILVER");
	m_pMoneyBronze = GetControl<CEtUIStatic>("ID_MONEY_BRONZE");
	m_pMoneyExp = GetControl<CEtUIStatic>("ID_MONEY_EXP");
	m_pMoneyExpBack = GetControl<CEtUIStatic>("ID_STATIC1");
	m_pMoneyBack = GetControl<CEtUIStatic>("ID_STATIC2");
	m_pStaticComment = GetControl<CEtUIStatic>("ID_STATIC_COMMENT");

	m_vlMoneyAndExp.push_back( std::make_pair( m_pMoneyGold->GetUICoord(), m_pMoneyGold) );
	m_vlMoneyAndExp.push_back( std::make_pair( m_pMoneySilver->GetUICoord(), m_pMoneySilver) );
	m_vlMoneyAndExp.push_back( std::make_pair( m_pMoneyBronze->GetUICoord(), m_pMoneyBronze) );
	m_vlMoneyAndExp.push_back( std::make_pair( m_pMoneyExp->GetUICoord(), m_pMoneyExp) );
	m_vlMoneyAndExp.push_back( std::make_pair( m_pMoneyExpBack->GetUICoord(), m_pMoneyExpBack) );
	m_vlMoneyAndExp.push_back( std::make_pair( m_pMoneyBack->GetUICoord(), m_pMoneyBack) );
	m_vlMoneyAndExp.push_back( std::make_pair( m_pStaticComment->GetUICoord(), m_pStaticComment) );

	m_pItemBG = GetControl<CEtUIStatic>("ID_STATIC0");

	SAcceptPresentItemSlot AcceptItemSlot;
	char szButton[ 32 ] = { 0 };
	char szStatic[ 32 ] = { 0 };

	for( int i = 0; i < MAX_ACCEPT_NPC_PRESENT_VARIATION_COUNT; i++ )
	{
		sprintf_s( szButton, 32, "ID_BUTTON_ITEM_%02d", i );
		sprintf_s( szStatic, 32, "ID_STATIC_ITEM_%02d", i );

		AcceptItemSlot.pItemButton = GetControl<CDnItemSlotButton>(szButton);
		AcceptItemSlot.pStaticSelected = GetControl<CEtUIStatic>(szStatic);
		AcceptItemSlot.pItemButton->SetSlotType( ST_NPC_PRESENT );

		AcceptItemSlot.ResetSlot();
		m_vlAcceptPresentItemInfo.push_back( AcceptItemSlot );
	}

	// Hide EXP-related UI since it is not used.
	m_pMoneyExp->Show( false );
	m_pMoneyExpBack->Show( false );
}

void CDnNpcAcceptPresentDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );

	if( nCommand == EVENT_BUTTON_CLICKED )
	{
		if( strstr( pControl->GetControlName(), "ID_BUTTON_ITEM" ) )
		{
			// Note : Display the selected item.
			//

			// Since only one can be selected, deselect the rest.
			ResetSlotCheck();

			CDnItemSlotButton* pBtnItemSlot = static_cast<CDnItemSlotButton*>( pControl );

			// Skip currently empty item slots
			if( pBtnItemSlot->GetItem() )
			{
				if( pBtnItemSlot->IsEnable() )
				{
					// Cannot select items not in inventory except gold items.
					if( REPUTE_PRESENT_MONEY_ITEM_ID == pBtnItemSlot->GetItem()->GetClassID() )
					{
						CDnItemSlotButton *pButton = static_cast<CDnItemSlotButton*>(pControl);
						int nIndex = pButton->GetButtonID();
						int iCoin = m_vlAcceptPresentItemInfo.at( nIndex ).iCoin;

						if( GetItemTask().GetCoin() < iCoin )
						{
							//GetInterface().MessageBox( L"Cannot send gift due to insufficient coins. (UIString needed)", MB_OK );
							GetInterface().MessageBox( GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 3207), MB_OK );
						}
						else
						{
							int iGold = iCoin/10000;
							int iSilver = (iCoin%10000)/100;
							int iBronze = iCoin%100;

							m_pMoneyGold->SetIntToText( iGold );
							m_pMoneySilver->SetIntToText( iSilver );
							m_pMoneyBronze->SetIntToText( iBronze );

							// Set selected index.
							m_iSelectedSlotIndex = nIndex;
							m_vlAcceptPresentItemInfo[ nIndex ].SetCheck( true );
						}
					}
					else
					{
						CDnItemSlotButton *pButton = static_cast<CDnItemSlotButton*>( pControl );

						if( NULL == GetItemTask().GetCharInventory().FindItem( pButton->GetItem()->GetClassID() ) )
						{
							//GetInterface().MessageBox( L"Cannot send gift because you do not own this item. (UIString needed)", MB_OK );
							GetInterface().MessageBox( GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 3206), MB_OK );
						}
						else
						{
							int nIndex = pButton->GetButtonID();

							// Set selected index.
							m_iSelectedSlotIndex = nIndex;

							CDnItem* pItem = m_vlAcceptPresentItemInfo[ nIndex ].GetSlot();
							if ( pItem )
								m_vlAcceptPresentItemInfo[ nIndex ].SetCheck( true );
						}
					}
				}
			}

			return;
		}
	}

	CEtUIDialog::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}

void CDnNpcAcceptPresentDlg::Show( bool bShow ) 
{
	if( m_bShow == bShow )
		return;

	if( bShow )
	{
	}
	else
	{
		m_pMoneyGold->ClearText();
		m_pMoneySilver->ClearText();
		m_pStaticComment->ClearText();

		m_iAvailableCount = 0;

		// If all items are cleared when hidden, the gift table item ID to send to the server also gets reset.
		// Since DeleteAllItem() is called in SetPresent anyway, do not call it here.
		// m_iSelectedSlotIndex = -1; is also inside DeleteAllItem().
		//DeleteAllItem();
	}

	CEtUIDialog::Show( bShow );
}

void CDnNpcAcceptPresentDlg::DeleteAllItem()
{
	m_iSelectedSlotIndex = -1;
	for( int i=0; i<(int)m_vlAcceptPresentItemInfo.size(); i++ )
	{
		m_vlAcceptPresentItemInfo[i].ResetSlot();
	}
}

void CDnNpcAcceptPresentDlg::SetPresent( int nNpcID )
{
	DeleteAllItem();

	CDnItemTask* pItemTask = static_cast<CDnItemTask*>( CTaskManager::GetInstance().GetTask("ItemTask") );
	if( !pItemTask ) 
		return;

	// Search the NPC favorability table and display what gifts this NPC accepts.
	DNTableFileFormat*  pNpcReputationTable = GetDNTable( CDnTableDB::TREPUTE );
	DNTableFileFormat*  pPresentTable = GetDNTable( CDnTableDB::TPRESENT );
	vector<int> vlNpcReputeInfo;
	pNpcReputationTable->GetItemIDListFromField( "_NpcID", nNpcID, vlNpcReputeInfo );

	_ASSERT( 1 == vlNpcReputeInfo.size() );

	// If not found in the table, it crashes here.
	int iReputeTableID = vlNpcReputeInfo.front();

	// Currently the maximum number of gifts is 6
	bool bItemExist = false;
	char acBuffer[ 256 ] = { 0 };
	for( int i = 0; i < MAX_ACCEPT_NPC_PRESENT_VARIATION_COUNT; ++i )
	{
		sprintf_s( acBuffer, "_PresentID%d", i+1 );
		int iPresentTableItemID = pNpcReputationTable->GetFieldFromLablePtr( iReputeTableID, acBuffer )->GetInteger();

		if( 0 == iPresentTableItemID )
			continue;

		++m_iAvailableCount;

		m_vlAcceptPresentItemInfo.at( i ).SetPresentID( iPresentTableItemID );

		// Distinguish whether it is an item or a coin.
		// Display items regardless of whether they are normal items or cash items.
		int iPresentType = pPresentTable->GetFieldFromLablePtr( iPresentTableItemID, "_PresentType" )->GetInteger();
		if( 0 == iPresentType ||
			1 == iPresentType )
		{
			int iPresentItemID = pPresentTable->GetFieldFromLablePtr( iPresentTableItemID, "_PresentTypeID" )->GetInteger();
			int iAcceptCount = pPresentTable->GetFieldFromLablePtr( iPresentTableItemID, "_Count" )->GetInteger();

			TItemInfo ItemInfo;
			bool bSuccess = CDnItem::MakeItemInfo( iPresentItemID, iAcceptCount, ItemInfo );
			_ASSERT( bSuccess );
			CDnItem* pItem = pItemTask->CreateItem( ItemInfo );
			if( pItem )
			{
				_ASSERT( i < (int)m_vlAcceptPresentItemInfo.size() );

				m_vlAcceptPresentItemInfo.at( i ).SetSlot( pItem );
				bItemExist = true;
				m_vlAcceptPresentItemInfo.at( i ).pItemButton->SetRenderCount(false);

				// When pressing Send Gift, prevent gifting if the player doesn't own the item.
				// Items not actually owned cannot be selected. Display indicator.
				if( 0 == iPresentType )
				{
					std::vector<CDnItem *> invenItemList;
					int count = CDnItemTask::GetInstance().GetCharInventory().FindItemList(iPresentItemID, -1, invenItemList);
					if (count <= 0)
					{
						m_vlAcceptPresentItemInfo.at( i ).pItemButton->SetRegist( true );
					}
					else
					{
						CDnItemSlotButton* pBtn = m_vlAcceptPresentItemInfo.at( i ).pItemButton;
						if (pBtn)
						{
							m_vlAcceptPresentItemInfo.at( i ).SetSlot( pItem, count );
							pBtn->SetRegist(false);
							pBtn->SetRenderCount(true);
						}
					}
				}
				else
				if( 1 == iPresentType )
				{
					std::vector<CDnItem *> invenItemList;
					int count = CDnItemTask::GetInstance().GetCashInventory().FindItemList(iPresentItemID, -1, invenItemList);
					if (count <= 0)
					{
						m_vlAcceptPresentItemInfo.at( i ).pItemButton->SetRegist( true );
					}
					else
					{
						CDnItemSlotButton* pBtn = m_vlAcceptPresentItemInfo.at( i ).pItemButton;
						if (pBtn)
						{
							m_vlAcceptPresentItemInfo.at( i ).SetSlot( pItem, count );
							pBtn->SetRegist(false);
							pBtn->SetRenderCount(true);
						}
					}
				}
			}
		}
		else
		if( 2 == iPresentType )		// Gift type is 'money'.
		{
			int iCoin = pPresentTable->GetFieldFromLablePtr( iPresentTableItemID, "_PresentTypeID" )->GetInteger();

			//  Display gold amount only when a gold item is selected.
			//int iGold = iCoin/10000;
			//int iSilver = (iCoin%10000)/100;
			//int iBronze = iCoin%100;

			//m_pMoneyGold->SetIntToText( iGold );
			//m_pMoneySilver->SetIntToText( iSilver );
			//m_pMoneyBronze->SetIntToText( iBronze );

			m_vlAcceptPresentItemInfo.at( i ).SetCoin( iCoin );

			// If the gift is money type, display a dummy item for showing money. Doesn't matter if the player doesn't own it.
			// Process with a unique item ID. The money display item ID must not change.
			TItemInfo ItemInfo;
			bool bSuccess = CDnItem::MakeItemInfo( REPUTE_PRESENT_MONEY_ITEM_ID, 1, ItemInfo );
			_ASSERT( bSuccess );
			CDnItem* pItem = pItemTask->CreateItem( ItemInfo );
			m_vlAcceptPresentItemInfo.at( i ).SetSlot( pItem );

			// When pressing Send Gift, prevent gifting if the player doesn't own the item.
			//// Enable only when actually having enough money
			//if( iCoin < GetItemTask().GetCoin() )
			//	m_vlAcceptPresentItemInfo.at( i ).pItemButton->Enable( false );
		}
	}

	if( false == bItemExist )
	{
		m_pItemBG->Show( false );

		for each( std::pair< SUICoord, CEtUIStatic* > e in m_vlMoneyAndExp ) 
		{
			SUICoord Coord = e.first;
			Coord.fY += 0.05f;
			e.second->SetUICoord( Coord );
		}
	}
	else
	{
		m_pItemBG->Show( true );
		
		for each( std::pair< SUICoord, CEtUIStatic* > e in m_vlMoneyAndExp ) 
		{
			e.second->SetUICoord( e.first );
		}
	}
}

void CDnNpcAcceptPresentDlg::ResetSlotCheck()
{
	m_iSelectedSlotIndex = -1;
	for( int i=0; i<(int)m_vlAcceptPresentItemInfo.size(); i++ )
	{
		m_vlAcceptPresentItemInfo[i].SetCheck( false );
	}

	// Remove displayed gold.
	m_pMoneyGold->ClearText();
	m_pMoneySilver->ClearText();
	m_pMoneyBronze->ClearText();
}

int CDnNpcAcceptPresentDlg::GetSelectedPresentID( void )
{
	int iResult = 0;

	if( -1 != m_iSelectedSlotIndex )
	{
		const SAcceptPresentItemSlot& AcceptPresentSlot = m_vlAcceptPresentItemInfo.at( m_iSelectedSlotIndex );
		iResult = AcceptPresentSlot.iPresentID;
	}

	return iResult;
}

//int CDnNpcAcceptPresentDlg::GetSlotCheckCount()
//{
//	int nRetCount(0);
//
//	for( int i=0; i<(int)m_vlAcceptPresentItemInfo.size(); i++ )
//	{
//		if( m_vlAcceptPresentItemInfo[i].IsCheckSlot() )
//		{
//			nRetCount++;
//		}
//	}
//
//	return nRetCount;
//}

CDnSlotButton* CDnNpcAcceptPresentDlg::GetSelectedPresentSlotButton() const
{
	int iResult = 0;

	if (m_iSelectedSlotIndex != -1)
	{
		const SAcceptPresentItemSlot& AcceptPresentSlot = m_vlAcceptPresentItemInfo.at(m_iSelectedSlotIndex);
		return AcceptPresentSlot.pItemButton;
	}

	return NULL;
}
#endif // #ifdef PRE_ADD_NPC_REPUTATION_SYSTEM