#include "Stdafx.h"

#include "DnItemTask.h"

#include "DnInterface.h"
#include "DnInvenCashItemRemoveDlg.h"

#ifdef PRE_ADD_CASHREMOVE




CDnInvenCashItemRemoveDlg::CDnInvenCashItemRemoveDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback, bool bAutoCursor )
: CDnCustomDlg( dialogType, pParentDialog, nID, pCallback, bAutoCursor )
, m_pSlotButton(NULL)
, m_DlgType( CDnInvenCashItemRemoveDlg::DLGTYPE_REMOVE )
{

}


//CDnInvenCashItemRemoveDlg::


void CDnInvenCashItemRemoveDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName("InvenCashDeleteDlg.ui").c_str(), bShow );
}


void CDnInvenCashItemRemoveDlg::InitialUpdate()
{
	m_pSlotButton = GetControl<CDnItemSlotButton>( "ID_ITEM" );		
}


void CDnInvenCashItemRemoveDlg::Show( bool bShow )
{
	if( m_bShow == bShow )
		return;

	CEtUIDialog::Show( bShow );

	if( bShow )
	{
		CEtUIStatic * pStatic = NULL;

		// Removal window.
		if( m_DlgType == DLGTYPE::DLGTYPE_REMOVE )
		{
			pStatic = GetControl<CEtUIStatic>( "ID_TEXT_TITLE" );
			pStatic->SetText( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 4881 ) ); // "Delete Cash Item"

			pStatic = GetControl<CEtUIStatic>( "ID_TEXT_ASK0" );
			pStatic->SetText( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 4882 ) ); // "Do you want to delete the selected cash item?"

			pStatic = GetControl<CEtUIStatic>( "ID_TEXT_ASK1" );
			pStatic->SetText( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 4883 ) ); // "If you agree, the cash item can be restored ~."			
		}

		// Restore window.
		else if( m_DlgType == DLGTYPE::DLGTYPE_RESTORE )
		{
			pStatic = GetControl<CEtUIStatic>( "ID_TEXT_TITLE" );
			pStatic->SetText( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 4888 ) ); // "Restore Cash Item"

			pStatic = GetControl<CEtUIStatic>( "ID_TEXT_ASK0" );
			pStatic->SetText( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 4889 ) ); // "Do you want to restore the selected cash item?"

			pStatic = GetControl<CEtUIStatic>( "ID_TEXT_ASK1" );
			pStatic->SetText( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 4890 ) ); // "If you agree, the cash item will be restored based on ~."			
		}
	}
	else
	{
		// Removal window.
		if( m_DlgType == DLGTYPE::DLGTYPE_REMOVE )
			GetInterface().RemoveCashItemCancel();
		//// Restore window.
		//else if( m_DlgType == DLGTYPE::DLGTYPE_RESTORE )
		//	GetInterface().RestoreCashItemCancel;
	}

}


void CDnInvenCashItemRemoveDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );
	
	if( nCommand == EVENT_BUTTON_CLICKED )
	{
		if( IsCmdControl( "ID_OK" ) )
		{
			// Removal window.
			if( m_DlgType == DLGTYPE::DLGTYPE_REMOVE )
				GetInterface().RemoveCashItemAccept(); // Accept removal.

			// Restore window.
			else if( m_DlgType == DLGTYPE::DLGTYPE_RESTORE )
				RestoreCashItemAccept(); // Accept restore.
			
			Show( false );			
		}

		else if( IsCmdControl( "ID_CANCEL" ) || IsCmdControl( "ID_CLOSE" ) )
		{
			Show( false );			
		}
	}

	CEtUIDialog::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}


void CDnInvenCashItemRemoveDlg::SetItemSlot( MIInventoryItem * pItem )
{
	// #65680 Tooltip does not appear on mouseover of item in cash item delete confirmation window.
	// - When dragging a cash item to delete it, tooltip handling hides while dragging is still in progress, so forcibly Release it.
	if( EtInterface::drag::IsValid() )		
	{		
		drag::Command(UI_DRAG_CMD_CANCEL);
		drag::ReleaseControl();		
	}

	m_pSlotButton->SetItem( pItem, pItem->GetOverlapCount() );
}



// Set Dlg type.
void CDnInvenCashItemRemoveDlg::SetDlgType( DLGTYPE type )
{
	m_DlgType = type;
}


// Accept item restoration.
void CDnInvenCashItemRemoveDlg::RestoreCashItemAccept()
{
	CDnItem * pItem = static_cast< CDnItem * >( m_pSlotButton->GetItem() );
	if( pItem )
	{			
		GetItemTask().RequestRemoveCashItem( pItem->GetSerialID(), true );
	}	
}

#endif