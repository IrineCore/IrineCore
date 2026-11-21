#include "Stdafx.h"

#ifdef PRE_ADD_AUTOUNPACK

#include "ItemSendPacket.h"

#include "DnItemTask.h"

#include "DnInterface.h"
#include "DnMainMenuDlg.h"
#include "DnAutoUnPackDlg.h"

#include "DnAutoUnPackResultDlg.h"


CDnAutoUnPackResultDlg::CDnAutoUnPackResultDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback, bool bAutoCursor )
: CDnCustomDlg( dialogType, pParentDialog, nID, pCallback, bAutoCursor ),
m_fOrgProgressTime(0.0f), m_fProgressTime(0.0f), m_pAutoUnPackDlg(NULL), m_pItemSlotBtn( NULL ), m_pResultProgress( NULL ), m_pStaticItemName( NULL ), m_pResultItem( NULL )
{
	
}

CDnAutoUnPackResultDlg::~CDnAutoUnPackResultDlg()
{
	SAFE_DELETE(m_pResultItem);
}

void CDnAutoUnPackResultDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName("InvenBoxOpenResultDlg.ui").c_str(), bShow );
}


void CDnAutoUnPackResultDlg::InitialUpdate()
{
	m_pItemSlotBtn = GetControl<CDnItemSlotButton>("ID_BT_ITEM");
	m_pResultProgress = GetControl<CEtUIProgressBar>("ID_PROGRESSBAR1");
	m_pResultProgress->SetReverse( true );
	m_pStaticItemName = GetControl<CEtUIStatic>("ID_TEXT_ITEMNAME");
}


void CDnAutoUnPackResultDlg::Show( bool bShow )
{
	if( m_bShow == bShow )
		return;
	
	CEtUIDialog::Show( bShow );
	
	if( bShow )
	{
		m_bUnpacking = true;

		// Auto-open window.
		if( m_pAutoUnPackDlg == NULL )
		{ 			
			m_pAutoUnPackDlg = GetInterface().GetAutoUnPackDlg();
		}
	
		m_pResultProgress->Show( true );
		m_fOrgProgressTime = m_fProgressTime = 1.15f;
	}	
	else
	{
		m_bUnpacking = false;
	}
}


void CDnAutoUnPackResultDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );

	if( nCommand == EVENT_BUTTON_CLICKED )
	{
		if( IsCmdControl("ID_CANCEL") )
			m_fProgressTime = 0.0f;

		if( IsCmdControl( "ID_STOP" ) )
		{
			// Stop auto-open.
			if( GetInterface().IsOpenAutoUnPackDlg() )
				m_bUnpacking = false;				
		}
	}

	CEtUIDialog::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}


void CDnAutoUnPackResultDlg::Process( float fElapsedTime )
{
	CEtUIDialog::Process( fElapsedTime );

	if( !IsShow() )
		return;

	// State where "Stop Opening" was pressed. 
	if( !m_bUnpacking )
	{
		// Restore previous state.
		if( m_pAutoUnPackDlg )
		{
			m_pAutoUnPackDlg->PreStateRestore();
			Show( false );
		}
	}

	// Progress status.
	else
	{
		m_fProgressTime -= fElapsedTime;
		if( m_fProgressTime < 0.0f )
		{
			m_pResultProgress->Enable( false );
			Show( false );

			// Prepare to open item.
			if( m_pAutoUnPackDlg )
				m_pAutoUnPackDlg->ReadyUnPack();
		}
		else
			m_pResultProgress->SetProgress( (1.0f - (m_fProgressTime / m_fOrgProgressTime)) * 100.0f );
	}
}


// Display result item.
void CDnAutoUnPackResultDlg::SetResultItem( int nItemID, int nCount, int nPeriod, INT64 nGold )
{
	TItemInfo itemInfo;	
	if( CDnItem::MakeItemInfo( nItemID, 1, itemInfo ) == false ) 
		return;

	if( m_pResultItem )
	{
		SAFE_DELETE(m_pResultItem);
		m_pItemSlotBtn->ResetSlot();
	}

	m_pResultItem = GetItemTask().CreateItem( itemInfo );

	if( m_pResultItem )
	{
		std::wstring strText;	

		// Gold acquired - do not output units with 0 (e.g., 1 gold 0 silver 1 copper -> 1 gold 1 copper).
		if( nGold > 0 )
		{	
			wchar_t str[256] = {0,};

			// Gold.
			INT64 coin = nGold / 10000;
			if( coin > 0 )
			{
				swprintf_s( str, _countof(str), GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 1766 ), coin );
				strText.assign( str );
			}

			// Silver.
			coin = (nGold % 10000) / 100;
			if( coin > 0 )
			{
				strText.append( L" " );
				swprintf_s( str, _countof(str), GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 1781 ), coin );			
				strText.append( str );
			}

			// Copper.
			coin = nGold % 100;
			if( coin > 0 )
			{
				strText.append( L" " );
				swprintf_s( str, _countof(str), GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 1783 ), coin );			
				strText.append( str );
			}

			m_pItemSlotBtn->SetItem( m_pResultItem, 0 );
		}

		// Acquire item.
		else
		{
			strText.assign( m_pResultItem->GetName() );
						
			if( nPeriod <= 0 )
				m_pResultItem->SetEternityItem( true );	

			m_pItemSlotBtn->SetItem( m_pResultItem, nCount );		
		}	

		m_pStaticItemName->SetText( strText );
	}
}
#endif