#include "Stdafx.h"


#ifdef PRE_ADD_DONATION


#include "DnContributionTask.h"

#include "DnContributionRewardListItemDlg.h"
#include "DnContributionRewardDlg.h"


CDnContributionRewardDlg::CDnContributionRewardDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback, bool bAutoCursor )
: CEtUIDialog( dialogType, pParentDialog, nID, pCallback, bAutoCursor )
, m_crrType( ETypeReward::NONE )
, m_bFirstOpen( true )
, m_pListRewardMonth( NULL )
, m_pListRewardDay( NULL )
, m_pRadioMonth( NULL )
, m_pRadioDay( NULL )
{
	
}


// Override - CEtUIDIalog.	
void CDnContributionRewardDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "DonationRewardDlg.ui" ).c_str(), bShow );
}


void CDnContributionRewardDlg::InitialUpdate()
{
	m_pRadioMonth = GetControl<CEtUIRadioButton>( "ID_RBT_MOMTH" );	
	m_pRadioDay = GetControl<CEtUIRadioButton>( "ID_RBT_DAY" );
	
	m_pListRewardMonth = GetControl<CEtUIListBoxEx>( "ID_LISTBOXEX_LIST" ); // Month.		
	m_pListRewardDay = static_cast<CEtUIListBoxEx *>( CreateControl( m_pListRewardMonth->GetProperty() ) ); // Day.
}


void CDnContributionRewardDlg::Show( bool bShow )
{	
	if( m_bShow == bShow )
		return;

	CEtUIDialog::Show( bShow );

	if( bShow )
	{
		m_pRadioDay->SetChecked( false );  // #49576
		m_pRadioMonth->SetChecked( true ); // Check monthly reward on Open.			

		// First time Dlg opens.
		if( m_bFirstOpen )
		{
			m_bFirstOpen = false;	
			SetRewardList(); // Set ranking reward list in ListBox.
		}

		RefreshList( ETypeReward::MONTH );
	}
	else
	{
		m_crrType = ETypeReward::NONE;				
	}
}


void CDnContributionRewardDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );

	if( nCommand == EVENT_RADIOBUTTON_CHANGED )
	{
		// Monthly reward.
		if( IsCmdControl( "ID_RBT_MOMTH") && m_pRadioMonth->IsChecked() )
			RefreshList( ETypeReward::MONTH );

		// Daily reward.
		else if( IsCmdControl( "ID_RBT_DAY") && m_pRadioDay->IsChecked() )
			RefreshList( ETypeReward::DAY);
	}

	else if( nCommand == EVENT_BUTTON_CLICKED )
	{
		if( IsCmdControl( "ID_BUTTON_CLOSE" ) || IsCmdControl( "ID_BT_CLOSE" ) )
			Show( false );
	}
	

	CEtUIDialog::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}


void CDnContributionRewardDlg::SetRewardList()
{
	CDnContributionTask & rTask = CDnContributionTask::GetInstance();

	// Month.
	const std::vector< SContributionRewardItems > & vecMonth = rTask.GetRewardItemsMonth();
	if( !vecMonth.empty() )
	{
		int size = (int)vecMonth.size();
		for( int i=1; i<size; ++i ) // Rank starts from 1 since rankings are 1-based.
		{		
			CDnContributionRewardListItemDlg * pItem = m_pListRewardMonth->AddItem< CDnContributionRewardListItemDlg >();		
			pItem->SetData( vecMonth[ i ] );
		}
	}	

	// Day.
	const std::vector< SContributionRewardItems > & vecDay = rTask.GetRewardItemsDay();
	if( !vecDay.empty() )
	{
		int size = (int)vecDay.size();
		for( int i=1; i<size; ++i ) // Rank starts from 1 since rankings are 1-based.
		{		
			CDnContributionRewardListItemDlg * pItem = m_pListRewardDay->AddItem< CDnContributionRewardListItemDlg >();		
			pItem->SetData( vecDay[ i ] );
		}
	}	
	
}


void CDnContributionRewardDlg::RefreshList( ETypeReward type )
{	
	if( m_crrType == type )
		return;
	m_crrType = type;

	// Monthly.
	if( type == ETypeReward::MONTH )
	{
		m_pListRewardMonth->Show( true );
		m_pListRewardDay->Show( false );	
	}

	// Daily.
	else if( type == ETypeReward::DAY )
	{
		m_pListRewardDay->Show( true );
		m_pListRewardMonth->Show( false );
	}
}

#endif