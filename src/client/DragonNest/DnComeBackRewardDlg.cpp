#include "Stdafx.h"


#ifdef PRE_ADD_NEWCOMEBACK


#include "DnTableDB.h"

#include "TaskManager.h"

#include "DnInterface.h"
#include "DnCharSelectDlg.h"
#include "DnCharSelectListDlg.h"

#include "DnComeBackRewardDlg.h"



CDnComeBackRewardDlg::CDnComeBackRewardDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback, bool bAutoCursor )
:CDnCustomDlg( dialogType, pParentDialog, nID, pCallback, bAutoCursor )
, m_pComboCharacter( NULL )
, m_pStaticGuild(NULL)
, m_crrEnableRadioIndex(0)
{
	m_level = 1;	
	m_SelectedCharIndex = -1; // Index of character selected in character selection screen.
}

void CDnComeBackRewardDlg::ReleaseDlg()
{
	m_vecStaticName.clear();

	// Reset ItemSlots.
	int nItemSlotSize = (int)m_vecItemSlot.size();
	for( int i=0; i<nItemSlotSize; ++i )
	{
		CDnItemSlotButton * pSlot = m_vecItemSlot[ i ];		
		CDnItem * pItem = (CDnItem *)( pSlot->GetItem() );
		SAFE_DELETE( pItem );
		pSlot->ResetSlot();
	}

	m_vecItemSlot.clear();	
	m_vecRadioBtn.clear();
	m_vecJob.clear();
}

// Override - CEtUIDialog //
void CDnComeBackRewardDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName("RewardList.ui").c_str(), bShow );
}

void CDnComeBackRewardDlg::InitialUpdate()
{
	m_pComboCharacter = GetControl<CEtUIComboBox>("ID_COMBOBOX_NAME");	

	m_pStaticGuild = GetControl<CEtUIStatic>("ID_STATIC8");

	// Reward box text.
	char str[256]={0,};
	m_vecStaticName.resize( 3 );
	for( int i=0; i<3; ++i )
	{
		sprintf_s( str, 256, "ID_TEXT_NAME%d", i );
		m_vecStaticName[ i ] = GetControl<CEtUIStatic>( str );
	}
	
	// Reward item slot.
	m_vecItemSlot.resize( 21 );
	for( int i=0; i<21; ++i )
	{
		sprintf_s( str, 256, "ID_BT_ITEM%d", i );
		m_vecItemSlot[ i ] = GetControl<CDnItemSlotButton>( str );		
	}
	
	// Reward box radio.
	m_vecRadioBtn.resize( 3 );
	for( int i=0; i<3; ++i )
	{
		sprintf_s( str, 256, "ID_RBT_BOX%d", i );
		m_vecRadioBtn[ i ] = GetControl<CEtUIRadioButton>( str );		
	}
}

void CDnComeBackRewardDlg::Show( bool bShow )
{
	if( m_bShow == bShow )
		return;

	if( bShow )
	{
		BuildComboBox(); // Set up character combo box.		

		// Reset reward item slot. 
		m_crrEnableRadioIndex = -1;
		RefreshItemSlot( 0 );
	}
	else
	{
		Reset();
	}

	CEtUIDialog::Show( bShow );
}


void CDnComeBackRewardDlg::OnUICallbackProc( int nID, UINT nCommand, CEtUIControl *pControl, UINT uiMsg )
{
	SetCmdControlName( pControl->GetControlName() );

	if( nCommand == EVENT_BUTTON_CLICKED )
	{
		switch( nID )
		{
		case 4952:
			{
				if( IsCmdControl("ID_OK") )		
					GetInterface().OpenSecurityCheckDlg( CDnInterface::InterfaceTypeEnum::Login , 0 );						
			}
			break;
		}		
	}
}

void CDnComeBackRewardDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl * pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );

	// Button.
	if( nCommand == EVENT_BUTTON_CLICKED )
	{	
		// Close.
		if( IsCmdControl("ID_BT_CLOSE") || IsCmdControl("ID_BUTTON_CANAEL" ) )
			Show( false );

		// Confirm.
		if( IsCmdControl("ID_BT_OK") )
		{
			/*CDnCharSelectDlg * pDlg = GetInterface().GetCharSelectDlg();
			if( pDlg )
			{
				CDnCharSelectListDlg * pDlgList = pDlg->GetCharSelectListDlg();
				if( pDlgList )
				{
					int idx = pDlgList->GetSelectCharIndex();
					std::vector<TCharListData> & vecCharList = pDlgList->GetCharListData();
					if( !vecCharList.empty() && (int)vecCharList.size() > idx )
					{						
						GetInterface().MessageBox( FormatW( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 4952 ), vecCharList[ idx ].wszCharacterName ).c_str()
, MB_OKCANCEL, 4952, this ); // "Are you sure you want to receive it on this character? (Cannot be modified after selection. Please choose carefully.)"
					}

				}
			}*/

#ifdef PRE_MOD_SELECT_CHAR
			CDnCharSelectDlg * pDlg = GetInterface().GetCharSelectDlg();
			if( pDlg )
			{
				SComboBoxItem* pItem = m_pComboCharacter->GetSelectedItem();
				CDnCharSelectListDlg * pDlgList = pDlg->GetCharSelectListDlg();
				if( pDlgList && pItem )
				{
					int idx = m_pComboCharacter->GetSelectedIndex();
					std::vector<TCharListData> & vecCharList = pDlgList->GetCharListData();
					if( !vecCharList.empty() && (int)vecCharList.size() > idx )
					{							
						GetInterface().MessageBox( FormatW( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 4952 ), vecCharList[ idx ].wszCharacterName ).c_str()
							, MB_OKCANCEL, 4952, this ); // "Are you sure you want to receive it on this character? (Cannot be changed once selected. Please choose carefully.)"																		
					}

				}
			}
#endif	// #ifdef PRE_MOD_SELECT_CHAR			
			
			this->Show( false );
		}
	}

	// Radio.
	else if( nCommand == EVENT_RADIOBUTTON_CHANGED )
	{
		int nIndex = -1;
		if( strstr( pControl->GetControlName(), "ID_RBT_BOX" ) != NULL )
		{
			if( sscanf( pControl->GetControlName() + strlen("ID_RBT_BOX"), "%d", &nIndex) == 1 )
			{
				RefreshItemSlot( nIndex );
			}
		}
	}

	// ComboBox.
	else if( nCommand == EVENT_COMBOBOX_SELECTION_CHANGED )
	{
		if( IsCmdControl("ID_COMBOBOX_NAME") )
		{
			SComboBoxItem* pItem = m_pComboCharacter->GetSelectedItem();
			if( pItem )
			{
				m_level = pItem->nValue;
				m_crrEnableRadioIndex = 0;
				RefreshItemSlot( m_crrEnableRadioIndex, true );

				// Change character selection.
				CDnLoginTask *pLoginTask = (CDnLoginTask *)CTaskManager::GetInstance().GetTask( "LoginTask" );
				if( !pLoginTask ) return;
				pLoginTask->SetCharSelect( m_pComboCharacter->GetSelectedIndex() );
			}
		}
	}


	CEtUIDialog::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );

}


void CDnComeBackRewardDlg::RefreshItemSlot( int nIndex, bool bForce )
{
	if( !bForce && m_crrEnableRadioIndex == nIndex )
		return;

	m_crrEnableRadioIndex = nIndex;

	// Reset ItemSlots.
	int nItemSlotSize = (int)m_vecItemSlot.size();
	for( int i=0; i<nItemSlotSize; ++i )
	{
		CDnItemSlotButton * pSlot = m_vecItemSlot[ i ];		
		CDnItem * pItem = (CDnItem *)( pSlot->GetItem() );
		SAFE_DELETE( pItem );
		pSlot->ResetSlot();
	}
	
	// Initialize reward text.
	for( int i=0; i<(int)m_vecStaticName.size(); ++i )
		m_vecStaticName[ i ]->Show( false );

	// Initialize reward box radio button.	
	for( int i=0; i<(int)m_vecRadioBtn.size(); ++i )
		m_vecRadioBtn[ i ]->Enable( false );

	/*
	if( m_level > 60 )
		m_level = 60;
	if( m_level < 1 )
		m_level = 1;
	*/

	wchar_t buf[256]={0,};
	char str[256]={0,};
	
	// LevelupEventTable.
	DNTableFileFormat * pTable = GetDNTable( CDnTableDB::TLEVELUPEVENT ); 
	if( !pTable )
		return;
	int size = pTable->GetItemCount();

	// MailTable.
	DNTableFileFormat * pTableMail = GetDNTable( CDnTableDB::TMAIL ); 
	if( !pTableMail )
		return;
	int sizeMail = pTableMail->GetItemCount();

	// CashCommodity.
	DNTableFileFormat * pTableCashCommodity = GetDNTable( CDnTableDB::TCASHCOMMODITY ); 
	if( !pTableCashCommodity )
		return;
	int sizeCashCommodity = pTableCashCommodity->GetItemCount();

	// ItemTable.
	DNTableFileFormat * pTableItem = GetDNTable( CDnTableDB::TITEM ); 
	if( !pTableItem )
		return;
	int sizeItem = pTableItem->GetItemCount();

	// CharmItemTable.
	DNTableFileFormat * pTableCharmItem = GetDNTable( CDnTableDB::TCHARMITEM ); 
	if( !pTableCharmItem )
		return;
	int sizeCharmItem = pTableCharmItem->GetItemCount();


	DNTableCell * pCell = NULL;

	int nItemSlotIndex = 0;


	// LevelupEventTable //	
	for( int i=0; i<size; ++i )
	{
		int nID = pTable->GetItemID( i );

		pCell = pTable->GetFieldFromLablePtr( nID, "_EventType" );
		if( !pCell || pCell->GetInteger() != 5 )
			continue;
		
		pCell = pTable->GetFieldFromLablePtr( nID, "_PCLevel" );
		if( !pCell )
			continue;

		if( pCell->GetInteger() != m_level )
			continue;
		
		pCell = pTable->GetFieldFromLablePtr( nID, "_CashMailID" );	
		if( !pCell)
			continue;


		// MailTable //
		for( int imail=0; imail<sizeMail; ++imail )
		{
			int nIDmail = pTableMail->GetItemID( imail );
			if( nIDmail != pCell->GetInteger() )
				continue;

			for( int k=1; k<4; ++k )
			{
				sprintf_s( str, 256, "_MailPresentItem%d", k );
				pCell = pTableMail->GetFieldFromLablePtr( nIDmail, str );	
				if( !pCell )
					continue;
				int nMailPresentItem = pCell->GetInteger();


				// CashCommodityTable //
				for( int iCashComm=0; iCashComm<sizeCashCommodity; ++iCashComm )
				{
					int nIDCashComm = pTableCashCommodity->GetItemID( iCashComm );
					pCell = pTableCashCommodity->GetFieldFromLablePtr( nIDCashComm, "_SN" );

					if( nMailPresentItem == pCell->GetInteger() )
					{
						// Reward box name.
						pCell = pTableCashCommodity->GetFieldFromLablePtr( nIDCashComm, "_NameID" );
						if( !pCell )
							continue;
						int idxK = k - 1;
						m_vecStaticName[ idxK ]->SetText( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, pCell->GetInteger() ) );
						m_vecStaticName[ idxK ]->Show( true );
						m_vecRadioBtn[ idxK ]->Enable( true );
	

						if( m_crrEnableRadioIndex != (k-1) )
							continue;

						// ItemTable //
						pCell = pTableCashCommodity->GetFieldFromLablePtr( nIDCashComm, "_ItemID01" );
						if( !pCell )
							continue;
						
						int nTypeParam = -1;
						for( int iitem=0; iitem<sizeItem; ++iitem )
						{
							int nIDitem = pTableItem->GetItemID( iitem );
							if( nIDitem != pCell->GetInteger() )
								continue;

							pCell = pTableItem->GetFieldFromLablePtr( nIDitem, "_TypeParam1" );
							if( !pCell || ( pCell && pCell->GetInteger() == 0 ) )
								continue;
							
							nTypeParam = pCell->GetInteger();
							break;
						} // ItemTable //

						if( nTypeParam == -1 )
							continue;

						// CharmItemTable //
						for( int icharm=0; icharm<sizeCharmItem; ++icharm )
						{
							int nIDcharm = pTableCharmItem->GetItemID( icharm );
							pCell = pTableCharmItem->GetFieldFromLablePtr( nIDcharm, "_CharmNum" );
							if( !pCell )
								continue;

							if( nTypeParam != pCell->GetInteger() )
								continue;

							pCell = pTableCharmItem->GetFieldFromLablePtr( nIDcharm, "_ItemID" );
							if( !pCell )
								continue;	
							int nRewardItemID = pCell->GetInteger();

							int nRewardItemCount = 1;
							pCell = pTableCharmItem->GetFieldFromLablePtr( nIDcharm, "_Count" );
							if( pCell )
								nRewardItemCount = pCell->GetInteger() < 1 ? 1 : pCell->GetInteger();


							// ItemTable - Job filtering.
							if( nItemSlotIndex >= (int)m_vecItemSlot.size() )
								break;

							pCell = pTableItem->GetFieldFromLablePtr( nRewardItemID, "_NeedJobClass" );
							if( !pCell )
								continue;

							int needJob = atoi( pCell->GetString() );
							if( needJob != 0 && needJob != m_vecJob[ m_pComboCharacter->GetSelectedIndex() ] )
								continue;

							CDnItem * pItem = CDnItem::CreateItem( nRewardItemID, nRewardItemCount );
							pItem->SetOverlapCount( nRewardItemCount );
							m_vecItemSlot[ nItemSlotIndex ]->SetItem( pItem, nRewardItemCount );
							++nItemSlotIndex;


							/*// ItemTable - Class filtering.
							for( int iitem=0; iitem<sizeItem; ++iitem )
							{
								if( nItemSlotIndex >= (int)m_vecItemSlot.size() )
									break;

								int nIDitem = pTableItem->GetItemID( iitem );

								pCell = pTableItem->GetFieldFromLablePtr( nIDitem, "_NeedJobClass" );
								if( !pCell )
									continue;

								int needJob = atoi( pCell->GetString() );
								if( needJob != 0 && needJob != m_vecJob[ m_pComboCharacter->GetSelectedIndex() ] )
									continue;

								CDnItem * pItem = CDnItem::CreateItem( nRewardItemID, nRewardItemCount );
								pItem->SetOverlapCount( nRewardItemCount );
								m_vecItemSlot[ nItemSlotIndex ]->SetItem( pItem, nRewardItemCount );
								++nItemSlotIndex;
							} 
							// ItemTable //*/
																
						}// CharmItemTable //

						break;
					}

				}// CashCommodityTable //

			}			

		}// MailTable //

		
		
	}// LevelupEventTable //	
}


void CDnComeBackRewardDlg::BuildComboBox()
{
	m_vecJob.clear();

	// Set combo box - character name.
	if( m_pComboCharacter && m_pComboCharacter->GetItemCount() < 1 )
	{
		CDnCharSelectDlg * pDlg = GetInterface().GetCharSelectDlg();
		if( pDlg )
		{
			CDnCharSelectListDlg * pDlgList = pDlg->GetCharSelectListDlg();
			if( pDlgList )
			{
				wchar_t buf[256] = {0,};
				const wchar_t * pStr = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 4957 ); // "Level %d %s"

				std::vector<TCharListData> & vecCharList = pDlgList->GetCharListData();
				int size = (int)vecCharList.size();

				m_vecJob.resize( size );

				for( int i=0; i<size; ++i )
				{
					swprintf_s( buf, 256, pStr, vecCharList[ i ].cLevel, vecCharList[ i ].wszCharacterName );					
					m_pComboCharacter->AddItem( buf, NULL, vecCharList[ i ].cLevel );					

					// Base class group.
					int job = vecCharList[ i ].cJob;
					int temp = 0;
					while( ( temp = CommonUtil::GetParentJob( job ) ) > 0 )
						job = temp;
					m_vecJob[ i ] = job;
				}

			}
		}
		int level = 0;
		if( m_pComboCharacter->GetItemValue( 0, level ) )
			m_level = level;
	}


	if( m_SelectedCharIndex == -1 || m_SelectedCharIndex >= (int)m_pComboCharacter->GetItemCount() )
		return;

	m_pComboCharacter->SetSelectedByIndex( m_SelectedCharIndex );

	int nValue = 0;
	if( m_pComboCharacter->GetSelectedValue( nValue ) )
		m_level = nValue;
		
}


// Set reward item to the character selected in the character selection screen.
void CDnComeBackRewardDlg::SetSelectedChar( int nIndex )
{
	m_SelectedCharIndex = nIndex; // Index of character selected in character selection screen.

	if( !m_pComboCharacter )
		return;

	if( nIndex >= (int)m_pComboCharacter->GetItemCount() )
		return;

	m_pComboCharacter->SetSelectedByIndex( nIndex );

}


void CDnComeBackRewardDlg::Reset()
{
	m_SelectedCharIndex = -1;
	m_pComboCharacter->RemoveAllItems();

	int size = 0;
	size = (int)m_vecStaticName.size();
	for( int i=0; i<size; ++i )
		m_vecStaticName[ i ]->SetText( L"" );
	
	size = (int)m_vecItemSlot.size();
	for( int i=0; i<size; ++i )
	{
		CDnItemSlotButton * pSlot = m_vecItemSlot[ i ];		
		CDnItem * pItem = (CDnItem *)( pSlot->GetItem() );
		SAFE_DELETE( pItem );
		pSlot->ResetSlot();
	}

}

#endif // PRE_ADD_NEWCOMEBACK
